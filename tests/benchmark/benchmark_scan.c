#define _GNU_SOURCE
#include "pkgintel/pkgintel.h"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

static double seconds_since(const struct timespec *start, const struct timespec *end) {
    return (double)(end->tv_sec - start->tv_sec) +
           (double)(end->tv_nsec - start->tv_nsec) / 1000000000.0;
}

static int write_fixture(const char *root, size_t packages, size_t files_per_package) {
    char path[1024];
    FILE *status = NULL;
    if (snprintf(path, sizeof(path), "%s/var/lib/dpkg/status", root) < 0) return -1;
    status = fopen(path, "wb");
    if (status == NULL) return -1;
    for (size_t p = 0; p < packages; ++p) {
        if (fprintf(status,
                    "Package: bench-%06zu\nVersion: 1.0\nArchitecture: amd64\n"
                    "Status: install ok installed\nInstalled-Size: 1\n\n", p) < 0) {
            fclose(status);
            return -1;
        }
        if (snprintf(path, sizeof(path), "%s/var/lib/dpkg/info/bench-%06zu.list", root, p) < 0) {
            fclose(status);
            return -1;
        }
        FILE *list = fopen(path, "wb");
        if (list == NULL) {
            fclose(status);
            return -1;
        }
        for (size_t f = 0; f < files_per_package; ++f) {
            if (fprintf(list, "/bench/missing-%06zu-%06zu\n", p, f) < 0) {
                fclose(list);
                fclose(status);
                return -1;
            }
        }
        if (fclose(list) != 0) {
            fclose(status);
            return -1;
        }
    }
    return fclose(status) == 0 ? 0 : -1;
}

static int make_tree(char *root, size_t root_size, size_t packages, size_t files_per_package) {
    char command[1024];
    int n = snprintf(root, root_size, "/tmp/pkgintel-perf-XXXXXX");
    if (n < 0 || (size_t)n >= root_size || mkdtemp(root) == NULL) return -1;
    if (snprintf(command, sizeof(command), "mkdir -p '%s/var/lib/dpkg/info'", root) < 0) return -1;
    if (system(command) != 0) return -1;
    return write_fixture(root, packages, files_per_package);
}

static void remove_tree(const char *root, size_t packages) {
    char path[1024];
    for (size_t p = 0; p < packages; ++p) {
        if (snprintf(path, sizeof(path), "%s/var/lib/dpkg/info/bench-%06zu.list", root, p) > 0) {
            (void)unlink(path);
        }
    }
    if (snprintf(path, sizeof(path), "%s/var/lib/dpkg/status", root) > 0) (void)unlink(path);
    if (snprintf(path, sizeof(path), "%s/var/lib/dpkg/info", root) > 0) (void)rmdir(path);
    if (snprintf(path, sizeof(path), "%s/var/lib/dpkg", root) > 0) (void)rmdir(path);
    if (snprintf(path, sizeof(path), "%s/var/lib", root) > 0) (void)rmdir(path);
    if (snprintf(path, sizeof(path), "%s/var", root) > 0) (void)rmdir(path);
    (void)rmdir(root);
}

int main(int argc, char **argv) {
    size_t packages = 100U;
    size_t files_per_package = 100U;
    size_t iterations = 10U;
    char root[256];
    pkg_context *context = NULL;
    pkg_target *target = NULL;
    struct timespec wall_start, wall_end;
    struct rusage usage_start, usage_end;
    uint64_t total_artifacts = 0U;
    uint64_t total_packages = 0U;

    if (argc > 1 && (packages = (size_t)strtoull(argv[1], NULL, 10)) == 0U) return 2;
    if (argc > 2 && (files_per_package = (size_t)strtoull(argv[2], NULL, 10)) == 0U) return 2;
    if (argc > 3 && (iterations = (size_t)strtoull(argv[3], NULL, 10)) == 0U) return 2;

    if (make_tree(root, sizeof(root), packages, files_per_package) != 0) {
        perror("benchmark fixture");
        return 1;
    }
    if (pkg_context_create(NULL, &context) != PKG_OK ||
        pkg_target_create_rootfs(context, root, &target) != PKG_OK) {
        remove_tree(root, packages);
        pkg_context_destroy(context);
        return 1;
    }

    if (clock_gettime(CLOCK_MONOTONIC, &wall_start) != 0 ||
        getrusage(RUSAGE_SELF, &usage_start) != 0) {
        pkg_target_destroy(target);
        pkg_context_destroy(context);
        remove_tree(root, packages);
        return 1;
    }

    for (size_t i = 0; i < iterations; ++i) {
        pkg_scan_result *result = NULL;
        pkg_status status = pkg_scan(context, target, NULL, &result);
        if (status != PKG_OK || result == NULL) {
            fprintf(stderr, "scan failed at iteration %zu: %s\n", i, pkg_status_string(status));
            pkg_target_destroy(target);
            pkg_context_destroy(context);
            remove_tree(root, packages);
            return 1;
        }
        total_packages += (uint64_t)pkg_scan_result_package_count(result);
        total_artifacts += (uint64_t)pkg_snapshot_artifact_count(result);
        pkg_scan_result_destroy(result);
    }

    (void)getrusage(RUSAGE_SELF, &usage_end);
    (void)clock_gettime(CLOCK_MONOTONIC, &wall_end);

    double wall = seconds_since(&wall_start, &wall_end);
    double user = seconds_since(&usage_start.ru_utime, &usage_end.ru_utime);
    double sys = seconds_since(&usage_start.ru_stime, &usage_end.ru_stime);
    long peak_rss_kib = usage_end.ru_maxrss;

    printf("packages=%zu files_per_package=%zu iterations=%zu\n", packages, files_per_package, iterations);
    printf("observed_packages=%" PRIu64 " observed_artifacts=%" PRIu64 "\n", total_packages, total_artifacts);
    printf("wall_seconds=%.9f user_seconds=%.9f system_seconds=%.9f peak_rss_kib=%ld\n",
           wall, user, sys, peak_rss_kib);
    printf("artifacts_per_second=%.3f\n",
           wall > 0.0 ? (double)total_artifacts / wall : 0.0);

    pkg_target_destroy(target);
    pkg_context_destroy(context);
    remove_tree(root, packages);
    return 0;
}
