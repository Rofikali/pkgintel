#define _GNU_SOURCE
#include "pkgintel/pkgintel.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "fixtures.h"

int test_scan_behaviour(void) {
    char fixture[256];
    pkg_context *context = NULL;
    pkg_target *target = NULL;
    pkg_scan_result *result = NULL, *limited_result = NULL, *second_result = NULL;
    pkg_scan_options options = PKG_SCAN_OPTIONS_INIT;
    options.max_packages = 10U;
    options.max_package_files = 100U;

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
        assert(fclose(file) == 0);
        file = NULL;
        assert(pkg_scan(context, target, NULL, &limited_result) == PKG_ERR_RESOURCE_LIMIT);
        assert(limited_result != NULL);
        assert(pkg_scan_result_package_count(limited_result) == 1U);
        assert(pkg_snapshot_artifact_count(limited_result) == 0U);
        pkg_scan_result_destroy(limited_result); limited_result = NULL;
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

    {
        char multi_fixture[256];
        pkg_scan_result *multi_result = NULL;
        const pkg_package *package = NULL;
        const pkg_artifact *artifact = NULL;
        pkg_path path_view;

        make_multi_package_fixture(multi_fixture, sizeof(multi_fixture));
        assert(pkg_target_create_rootfs(context, multi_fixture, &target) == PKG_OK);
        assert(pkg_scan(context, target, NULL, &multi_result) == PKG_OK);
        assert(multi_result != NULL);
        assert(pkg_scan_result_package_count(multi_result) == 2U);

        /* Dpkg discovery order is zeta-pkg, alpha-pkg; public order is deterministic. */
        assert(strcmp(pkg_scan_result_package_name(multi_result, 0U), "alpha-pkg") == 0);
        assert(strcmp(pkg_scan_result_package_name(multi_result, 1U), "zeta-pkg") == 0);

        assert(pkg_snapshot_package_at(multi_result, 0U, &package) == PKG_OK);
        assert(pkg_package_artifact_count(package) == 1U);
        assert(pkg_package_artifact_at(package, 0U, &artifact) == PKG_OK);
        path_view = pkg_artifact_path(artifact);
        assert(path_view.size == strlen("/alpha/artifact"));
        assert(memcmp(path_view.data, "/alpha/artifact", path_view.size) == 0);

        assert(pkg_snapshot_package_at(multi_result, 1U, &package) == PKG_OK);
        assert(pkg_package_artifact_count(package) == 1U);
        assert(pkg_package_artifact_at(package, 0U, &artifact) == PKG_OK);
        path_view = pkg_artifact_path(artifact);
        assert(path_view.size == strlen("/zeta/artifact"));
        assert(memcmp(path_view.data, "/zeta/artifact", path_view.size) == 0);

        pkg_scan_result_destroy(multi_result);
        pkg_target_destroy(target);
        remove_multi_package_fixture(multi_fixture);
    }
    assert(pkg_target_create_rootfs(context, "/definitely/nonexistent/pkgintel", &target) == PKG_OK); assert(pkg_scan(context, target, NULL, &result) == PKG_ERR_NOT_FOUND); assert(result == NULL); pkg_target_destroy(target);
    pkg_context_destroy(context); assert(strcmp(pkg_status_string(PKG_ERR_PARSE), "corrupt data") == 0); return 0;
}
