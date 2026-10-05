#include "pkgintel/pkgintel.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void make_fixture(char *root, size_t root_size) {
    char path[512];
    int written;
    FILE *file;

    written = snprintf(root, root_size, "/tmp/pkgintel-test-XXXXXX");
    assert(written > 0 && (size_t)written < root_size);
    assert(mkdtemp(root) != NULL);
    written = snprintf(path, sizeof(path), "%s/var", root);
    assert(written > 0 && (size_t)written < sizeof(path));
    assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/var/lib", root);
    assert(written > 0 && (size_t)written < sizeof(path));
    assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/var/lib/dpkg", root);
    assert(written > 0 && (size_t)written < sizeof(path));
    assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/var/lib/dpkg/status", root);
    assert(written > 0 && (size_t)written < sizeof(path));
    file = fopen(path, "wb");
    assert(file != NULL);
    assert(fputs("Package: fixture-pkg\nVersion: 1.2.3\nArchitecture: amd64\nStatus: install ok installed\nInstalled-Size: 10\n\nPackage: removed-pkg\nVersion: 9.9\nArchitecture: amd64\nStatus: deinstall ok config-files\nInstalled-Size: 999\n", file) >= 0);
    assert(fclose(file) == 0);
}

static void remove_fixture(const char *root) {
    char path[512];
    assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg/status", root) > 0);
    assert(unlink(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg", root) > 0);
    assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var/lib", root) > 0);
    assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var", root) > 0);
    assert(rmdir(path) == 0);
    assert(rmdir(root) == 0);
}

int main(void) {
    char fixture[256];
    pkg_context *context = NULL;
    pkg_target *target = NULL;
    pkg_scan_result *result = NULL;
    pkg_scan_options options = { PKG_SCAN_NORMAL, 10U, 100U };

    assert(pkg_context_create(NULL, &context) == PKG_OK);
    assert(context != NULL);

    make_fixture(fixture, sizeof(fixture));
    assert(pkg_target_create_rootfs(context, fixture, &target) == PKG_OK);
    assert(target != NULL);
    assert(pkg_scan(context, target, &options, &result) == PKG_OK);
    assert(result != NULL);
    assert(strcmp(pkg_scan_result_target_root(result), fixture) == 0);
    assert(pkg_scan_result_package_count(result) == 1U);
    assert(strcmp(pkg_scan_result_package_name(result, 0U), "fixture-pkg") == 0);
    assert(strcmp(pkg_scan_result_package_version(result, 0U), "1.2.3") == 0);
    assert(strcmp(pkg_scan_result_package_architecture(result, 0U), "amd64") == 0);
    assert(pkg_scan_result_package_installed_size(result, 0U) == 10240U);
    assert(pkg_scan_result_package_file_count(result, 0U) == 0U);
    assert(pkg_scan_result_package_missing_file_count(result, 0U) == 0U);
    assert(pkg_scan_result_diagnostic_count(result) == 1U);

    pkg_scan_result_destroy(result);
    pkg_target_destroy(target);
    remove_fixture(fixture);

    assert(pkg_target_create_rootfs(context, "/definitely/nonexistent/pkgintel", &target) == PKG_OK);
    assert(pkg_scan(context, target, NULL, &result) == PKG_ERR_NOT_FOUND);
    assert(result == NULL);
    pkg_target_destroy(target);

    pkg_context_destroy(context);
    assert(strcmp(pkg_status_string(PKG_ERR_PARSE), "parse error") == 0);
    return 0;
}
