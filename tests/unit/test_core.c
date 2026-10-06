#define _GNU_SOURCE
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
    char path[512]; int written; FILE *file;
    written = snprintf(root, root_size, "/tmp/pkgintel-test-XXXXXX"); assert(written > 0 && (size_t)written < root_size); assert(mkdtemp(root) != NULL);
    written = snprintf(path, sizeof(path), "%s/var", root); assert(written > 0 && (size_t)written < sizeof(path)); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/var/lib", root); assert(written > 0 && (size_t)written < sizeof(path)); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/var/lib/dpkg", root); assert(written > 0 && (size_t)written < sizeof(path)); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/var/lib/dpkg/status", root); assert(written > 0 && (size_t)written < sizeof(path));
    file = fopen(path, "wb"); assert(file != NULL);
    assert(fputs("Package: fixture-pkg\nVersion: 1.2.3\nArchitecture: amd64\nStatus: install ok installed\nInstalled-Size: 10\n\nPackage: removed-pkg\nVersion: 9.9\nArchitecture: amd64\nStatus: deinstall ok config-files\nInstalled-Size: 999\n", file) >= 0); assert(fclose(file) == 0);
    written = snprintf(path, sizeof(path), "%s/var/lib/dpkg/info", root); assert(written > 0 && (size_t)written < sizeof(path)); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/var/lib/dpkg/info/fixture-pkg.list", root); file = fopen(path, "wb"); assert(file != NULL);
    assert(fputs("/usr/bin/present\n/usr/bin/missing\n/usr/bin/link\n/usr/bin/broken\n/usr/bin/adir\n/usr/bin/fifo\n/usr/bin/present\n/restricted/secret\n../escape\n\n", file) >= 0); assert(fclose(file) == 0);
    written = snprintf(path, sizeof(path), "%s/usr", root); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/usr/bin", root); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/usr/bin/present", root); file = fopen(path, "wb"); assert(file != NULL); assert(fputs("x", file) == 1); assert(fclose(file) == 0);
    written = snprintf(path, sizeof(path), "%s/usr/bin/link", root); assert(symlink("/usr/bin/present", path) == 0);
    written = snprintf(path, sizeof(path), "%s/usr/bin/broken", root); assert(symlink("/usr/bin/nope", path) == 0);
    written = snprintf(path, sizeof(path), "%s/usr/bin/adir", root); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/usr/bin/fifo", root); assert(mkfifo(path, 0600) == 0);
    written = snprintf(path, sizeof(path), "%s/restricted", root); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/restricted/secret", root); file = fopen(path, "wb"); assert(file != NULL); assert(fwrite("secret", 1U, 6U, file) == 6U); assert(fclose(file) == 0);
    written = snprintf(path, sizeof(path), "%s/restricted", root); assert(chmod(path, 0000) == 0);
}

static void remove_fixture(const char *root) {
    char path[512];
    assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg/status", root) > 0); assert(unlink(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg/info/fixture-pkg.list", root) > 0); assert(unlink(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg/info", root) > 0); assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/usr/bin/fifo", root) > 0); assert(unlink(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/usr/bin/adir", root) > 0); assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/usr/bin/broken", root) > 0); assert(unlink(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/usr/bin/link", root) > 0); assert(unlink(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/usr/bin/present", root) > 0); assert(unlink(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/usr/bin", root) > 0); assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/usr", root) > 0); assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/restricted", root) > 0); assert(chmod(path, 0700) == 0); assert(snprintf(path, sizeof(path), "%s/restricted/secret", root) > 0); assert(unlink(path) == 0); assert(snprintf(path, sizeof(path), "%s/restricted", root) > 0); assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg", root) > 0); assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var/lib", root) > 0); assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var", root) > 0); assert(rmdir(path) == 0); assert(rmdir(root) == 0);
}

int main(void) {
    char fixture[256]; pkg_context *context = NULL; pkg_target *target = NULL;
    pkg_scan_result *result = NULL, *limited_result = NULL, *second_result = NULL;
    pkg_scan_options options = PKG_SCAN_OPTIONS_INIT; options.max_packages = 10U; options.max_package_files = 100U;
    assert(pkg_context_create(NULL, &context) == PKG_OK); assert(context != NULL);
    make_fixture(fixture, sizeof(fixture));
    assert(pkg_target_create_rootfs(context, fixture, &target) == PKG_OK); assert(target != NULL);
    assert(pkg_scan(context, target, &options, &result) == PKG_OK); assert(result != NULL);
    assert(strcmp(pkg_scan_result_target_root(result), fixture) == 0); assert(pkg_scan_result_package_count(result) == 1U);
    assert(pkg_scan(context, target, &options, &second_result) == PKG_OK); assert(pkg_scan_result_package_count(second_result) == pkg_scan_result_package_count(result));
    assert(strcmp(pkg_scan_result_package_name(second_result, 0U), pkg_scan_result_package_name(result, 0U)) == 0); assert(pkg_snapshot_artifact_count(second_result) == pkg_snapshot_artifact_count(result));
    pkg_scan_result_destroy(second_result); second_result = NULL;
    assert(strcmp(pkg_scan_result_package_name(result, 0U), "fixture-pkg") == 0); assert(strcmp(pkg_scan_result_package_version(result, 0U), "1.2.3") == 0); assert(strcmp(pkg_scan_result_package_architecture(result, 0U), "amd64") == 0);
    assert(pkg_scan_result_package_installed_size(result, 0U) == 10240U); assert(pkg_scan_result_package_file_count(result, 0U) == 9U); assert(pkg_scan_result_package_missing_file_count(result, 0U) == 1U); assert(pkg_scan_result_package_invalid_path_count(result, 0U) == 1U); assert(pkg_snapshot_artifact_count(result) == 9U);
    { const pkg_artifact *artifact = NULL; assert(pkg_snapshot_artifact_at(result, 1U, &artifact) == PKG_OK); assert(pkg_artifact_get_state(artifact) == PKG_ARTIFACT_MISSING); assert(pkg_snapshot_artifact_at(result, 2U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_SYMLINK); assert(pkg_artifact_get_state(artifact) == PKG_ARTIFACT_PRESENT); assert(pkg_snapshot_artifact_at(result, 3U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_SYMLINK); assert(pkg_artifact_get_state(artifact) == PKG_ARTIFACT_BROKEN_LINK); assert(pkg_snapshot_artifact_at(result, 4U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_DIRECTORY); assert(pkg_snapshot_artifact_at(result, 5U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_OTHER); assert(pkg_snapshot_artifact_at(result, 6U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_REGULAR); assert(pkg_snapshot_artifact_at(result, 7U, &artifact) == PKG_OK); if (geteuid() != 0) assert(pkg_artifact_get_state(artifact) == PKG_ARTIFACT_PERMISSION_DENIED); assert(pkg_snapshot_artifact_at(result, 8U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_UNKNOWN); assert(pkg_artifact_get_state(artifact) == PKG_ARTIFACT_UNVERIFIABLE); }
    { const pkg_package *package = NULL; const pkg_artifact *artifact = NULL; pkg_path path_view; assert(pkg_snapshot_package_at(result, 0U, &package) == PKG_OK); assert(pkg_package_artifact_count(package) == 9U); assert(pkg_package_artifact_at(package, 0U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_REGULAR); path_view = pkg_artifact_path(artifact); assert(path_view.size == strlen("/usr/bin/present")); assert(pkg_package_artifact_at(package, 3U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_SYMLINK); }
    assert(pkg_scan_result_diagnostic_count(result) == 1U);
    { const pkg_diagnostic *diagnostic = NULL; pkg_string_view code; assert(pkg_snapshot_diagnostic_at(result, 0U, &diagnostic) == PKG_OK); code = pkg_diagnostic_code(diagnostic); assert(code.size == strlen("PKG_DPKG_FILELIST_MALFORMED")); assert(memcmp(code.data, "PKG_DPKG_FILELIST_MALFORMED", code.size) == 0); }
    pkg_scan_result_destroy(result); pkg_target_destroy(target);
    assert(pkg_target_create_rootfs(context, fixture, &target) == PKG_OK);
    { pkg_scan_options limited = PKG_SCAN_OPTIONS_INIT; limited.max_packages = 10U; limited.max_package_files = 1U; assert(pkg_scan(context, target, &limited, &limited_result) == PKG_ERR_RESOURCE_LIMIT); assert(limited_result != NULL); assert(pkg_scan_result_package_count(limited_result) == 1U); assert(pkg_scan_result_package_file_count(limited_result, 0U) == 1U); assert(pkg_snapshot_artifact_count(limited_result) == 1U); }
    pkg_scan_result_destroy(limited_result); pkg_target_destroy(target);
    assert(pkg_target_create_rootfs(context, fixture, &target) == PKG_OK);
    { pkg_scan_options exact_limit = PKG_SCAN_OPTIONS_INIT; const pkg_artifact *artifact = NULL; exact_limit.max_packages = 10U; exact_limit.max_package_files = 9U; assert(pkg_scan(context, target, &exact_limit, &limited_result) == PKG_OK); assert(limited_result != NULL); assert(pkg_scan_result_package_file_count(limited_result, 0U) == 9U); assert(pkg_snapshot_artifact_count(limited_result) == 9U); assert(pkg_snapshot_artifact_at(limited_result, 8U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_UNKNOWN); assert(pkg_artifact_get_state(artifact) == PKG_ARTIFACT_UNVERIFIABLE); }
    pkg_scan_result_destroy(limited_result); pkg_target_destroy(target);
    assert(pkg_target_create_rootfs(context, fixture, &target) == PKG_OK);
    {
        char path[512];
        FILE *file;
        char oversized[65539];
        memset(oversized, 'x', sizeof(oversized));
        oversized[sizeof(oversized) - 2U] = '\n';
        oversized[sizeof(oversized) - 1U] = '\0';
        assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg/info/fixture-pkg.list", fixture) > 0);
        file = fopen(path, "wb"); assert(file != NULL);
        assert(fwrite(oversized, 1U, sizeof(oversized) - 1U, file) == sizeof(oversized) - 1U);
        assert(fclose(file) == 0);
        assert(pkg_scan(context, target, NULL, &limited_result) == PKG_ERR_RESOURCE_LIMIT);
        assert(limited_result != NULL);
        assert(pkg_scan_result_package_count(limited_result) == 1U);
        assert(pkg_scan_result_package_file_count(limited_result, 0U) == 0U);
        assert(pkg_snapshot_artifact_count(limited_result) == 0U);
        pkg_scan_result_destroy(limited_result); limited_result = NULL;
        file = fopen(path, "wb"); assert(file != NULL);
        assert(fputs("/usr/bin/present\n", file) >= 0);
        assert(fclose(file) == 0);

        assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg/status", fixture) > 0);
        file = fopen(path, "wb"); assert(file != NULL);
        assert(fputs("Package: fixture-pkg\nVersion: 1.2.3\nArchitecture: amd64\nStatus: install ok installed\nInstalled-Size: 10\n\n", file) >= 0);
        assert(fwrite(oversized, 1U, sizeof(oversized) - 1U, file) == sizeof(oversized) - 1U);
        assert(pkg_scan(context, target, NULL, &limited_result) == PKG_ERR_RESOURCE_LIMIT);
        assert(limited_result != NULL);
        assert(pkg_scan_result_package_count(limited_result) == 1U);
        assert(pkg_snapshot_artifact_count(limited_result) == 0U);
        pkg_scan_result_destroy(limited_result); limited_result = NULL;
        assert(fclose(file) == 0);
    }
    {
        pkg_scan_options unsupported = PKG_SCAN_OPTIONS_INIT;
        unsupported.max_files = 1U;
        assert(pkg_scan(context, target, &unsupported, &limited_result) == PKG_ERR_UNSUPPORTED);
        assert(limited_result == NULL);
        unsupported = (pkg_scan_options)PKG_SCAN_OPTIONS_INIT;
        unsupported.max_directories = 1U;
        assert(pkg_scan(context, target, &unsupported, &limited_result) == PKG_ERR_UNSUPPORTED);
        assert(limited_result == NULL);
        unsupported = (pkg_scan_options)PKG_SCAN_OPTIONS_INIT;
        unsupported.max_file_bytes = 1U;
        assert(pkg_scan(context, target, &unsupported, &limited_result) == PKG_ERR_UNSUPPORTED);
        assert(limited_result == NULL);
        unsupported = (pkg_scan_options)PKG_SCAN_OPTIONS_INIT;
        unsupported.max_total_bytes = 1U;
        assert(pkg_scan(context, target, &unsupported, &limited_result) == PKG_ERR_UNSUPPORTED);
        assert(limited_result == NULL);
        unsupported = (pkg_scan_options)PKG_SCAN_OPTIONS_INIT;
        unsupported.max_duration_ms = 1U;
        assert(pkg_scan(context, target, &unsupported, &limited_result) == PKG_ERR_UNSUPPORTED);
        assert(limited_result == NULL);
    }
    pkg_target_destroy(target);
    remove_fixture(fixture);
    assert(pkg_target_create_rootfs(context, "/definitely/nonexistent/pkgintel", &target) == PKG_OK); assert(pkg_scan(context, target, NULL, &result) == PKG_ERR_NOT_FOUND); assert(result == NULL); pkg_target_destroy(target);
    pkg_context_destroy(context); assert(strcmp(pkg_status_string(PKG_ERR_PARSE), "corrupt data") == 0); return 0;
}
