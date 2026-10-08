#include "pkgintel/pkgintel.h"
#include "internal/json.h"

#include <stdio.h>
#include <string.h>

static int command_scan(int json_mode) {
    pkg_context *context = NULL;
    pkg_target *target = NULL;
    pkg_scan_result *result = NULL;
    pkg_status status;

    status = pkg_context_create(NULL, &context);
    if (status != PKG_OK) {
        fprintf(stderr, "pkgintel: context: %s\n", pkg_status_string(status));
        return 1;
    }

    status = pkg_target_create_local(context, &target);
    if (status != PKG_OK) {
        fprintf(stderr, "pkgintel: target: %s\n", pkg_status_string(status));
        pkg_context_destroy(context);
        return 1;
    }

    status = pkg_scan(context, target, NULL, &result);
    if (status != PKG_OK && status != PKG_ERR_RESOURCE_LIMIT) {
        fprintf(stderr, "pkgintel: scan: %s\n", pkg_status_string(status));
        pkg_target_destroy(target);
        pkg_context_destroy(context);
        return 1;
    }

    if (json_mode) {
        pkg_status write_status = pkg_json_write(stdout, result, status);
        if (write_status != PKG_OK) {
            fprintf(stderr, "pkgintel: json: %s\n", pkg_status_string(write_status));
            pkg_scan_result_destroy(result);
            pkg_target_destroy(target);
            pkg_context_destroy(context);
            return 1;
        }
    } else {
        size_t i;
        printf("Target: %s\n", pkg_scan_result_target_root(result));
        printf("Packages: %zu\n", pkg_scan_result_package_count(result));
        if (status == PKG_ERR_RESOURCE_LIMIT) printf("Warning: resource limit reached\n");

        for (i = 0U; i < pkg_scan_result_package_count(result) && i < 20U; ++i) {
            printf("  %s %s [%s] %llu bytes\n",
                   pkg_scan_result_package_name(result, i),
                   pkg_scan_result_package_version(result, i),
                   pkg_scan_result_package_architecture(result, i),
                   (unsigned long long)pkg_scan_result_package_installed_size(result, i));
        }
        if (pkg_scan_result_package_count(result) > 20U) printf("  ...\n");
    }

    pkg_scan_result_destroy(result);
    pkg_target_destroy(target);
    pkg_context_destroy(context);
    return status == PKG_ERR_RESOURCE_LIMIT ? 3 : 0;
}

static void usage(const char *program) {
    fprintf(stderr, "usage: %s scan [--json]\n", program);
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "scan") == 0) return command_scan(0);
    if (argc == 3 && strcmp(argv[1], "scan") == 0 && strcmp(argv[2], "--json") == 0)
        return command_scan(1);
    usage(argv[0]);
    return 2;
}
