#include "pkgintel/pkgintel.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

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
    written = snprintf(path, sizeof(path), "%s/var/lib/dpkg/info", root);\n    assert(mkdir(path, 0700) == 0);\n    written = snprintf(path, sizeof(path), "%s/var/lib/dpkg/info/fixture-pkg.list", root);\n    file = fopen(path, "wb");\n    assert(file != NULL);\n    assert(fputs("/usr/bin/present\n/usr/bin/missing\n/usr/bin/link\n/usr/bin/broken\n../escape\n", file) >= 0);\n    assert(fclose(file) == 0);\n    written = snprintf(path, sizeof(path), "%s/usr", root); assert(mkdir(path, 0700) == 0);\n    written = snprintf(path, sizeof(path), "%s/usr/bin", root); assert(mkdir(path, 0700) == 0);\n    written = snprintf(path, sizeof(path), "%s/usr/bin/present", root); file = fopen(path, "wb"); assert(file != NULL); assert(fputs("x", file) == 1); assert(fclose(file) == 0);\n    written = snprintf(path, sizeof(path), "%s/usr/bin/link", root); assert(symlink("/usr/bin/present", path) == 0);\n    written = snprintf(path, sizeof(path), "%s/usr/bin/broken", root); assert(symlink("/usr/bin/nope", path) == 0);
}

static void remove_fixture(const char *root) {
    char path[512];
    assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg/status", root) > 0);
    assert(unlink(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg/info/fixture-pkg.list", root) > 0); assert(unlink(path) == 0);\n    assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg/info", root) > 0); assert(rmdir(path) == 0);\n    assert(snprintf(path, sizeof(path), "%s/usr/bin/broken", root) > 0); assert(unlink(path) == 0);\n    assert(snprintf(path, sizeof(path), "%s/usr/bin/link", root) > 0); assert(unlink(path) == 0);\n    assert(snprintf(path, sizeof(path), "%s/usr/bin/present", root) > 0); assert(unlink(path) == 0);\n    assert(snprintf(path, sizeof(path), "%s/usr/bin", root) > 0); assert(rmdir(path) == 0);\n    assert(snprintf(path, sizeof(path), "%s/usr", root) > 0); assert(rmdir(path) == 0);
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
    pkg_scan_options options = PKG_SCAN_OPTIONS_INIT;
    options.max_packages = 10U;\n    options.max_package_files = 100U;

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
    assert(pkg_scan_result_package_file_count(result, 0U) == 5U);
    assert(pkg_scan_result_package_missing_file_count(result, 0U) == 1U);
    assert(pkg_scan_result_package_invalid_path_count(result, 0U) == 1U);
    assert(pkg_snapshot_artifact_count(result) == 5U);
    { const pkg_package *package = NULL; const pkg_artifact *artifact = NULL; pkg_path path_view;
      assert(pkg_snapshot_package_at(result, 0U, &package) == PKG_OK);
      assert(pkg_package_artifact_count(package) == 5U);
      assert(pkg_package_artifact_at(package, 0U, &artifact) == PKG_OK);
      assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_REGULAR);
      path_view = pkg_artifact_path(artifact); assert(path_view.size == strlen("/usr/bin/present"));
      assert(pkg_package_artifact_at(package, 3U, &artifact) == PKG_OK);
      assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_SYMLINK);
    }
    assert(pkg_scan_result_diagnostic_count(result) == 0U);

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
