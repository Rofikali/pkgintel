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
    {
        char root_link[512];
        assert(snprintf(root_link, sizeof(root_link), "%s-root-link", fixture) > 0);
        assert(symlink(fixture, root_link) == 0);
        assert(pkg_target_create_rootfs(context, root_link, &target) == PKG_OK);
        assert(target != NULL);
        assert(pkg_scan(context, target, NULL, &limited_result) != PKG_OK);
        assert(limited_result == NULL);
        pkg_target_destroy(target); target = NULL;
        assert(unlink(root_link) == 0);
    }
    assert(pkg_target_create_rootfs(context, fixture, &target) == PKG_OK); assert(target != NULL);
    assert(pkg_scan(context, target, &options, &result) == PKG_OK); assert(result != NULL);
    assert(strcmp(pkg_scan_result_target_root(result), fixture) == 0); assert(pkg_scan_result_package_count(result) == 2U);
    assert(pkg_scan(context, target, &options, &second_result) == PKG_OK); assert(pkg_scan_result_package_count(second_result) == pkg_scan_result_package_count(result));
    assert(strcmp(pkg_scan_result_package_name(second_result, 0U), pkg_scan_result_package_name(result, 0U)) == 0); assert(pkg_snapshot_artifact_count(second_result) == pkg_snapshot_artifact_count(result));
    pkg_scan_result_destroy(second_result); second_result = NULL;
    assert(strcmp(pkg_scan_result_package_name(result, 0U), "fixture-pkg") == 0); assert(strcmp(pkg_scan_result_package_version(result, 0U), "1.2.3") == 0); assert(strcmp(pkg_scan_result_package_architecture(result, 0U), "amd64") == 0);
    assert(pkg_scan_result_package_installed_size(result, 0U) == 10240U); assert(pkg_scan_result_package_file_count(result, 0U) == 10U); assert(pkg_scan_result_package_missing_file_count(result, 0U) == 1U); assert(pkg_scan_result_package_invalid_path_count(result, 0U) == 1U); assert(pkg_snapshot_artifact_count(result) == 10U);
    assert(strcmp(pkg_scan_result_package_name(result, 1U), "removed-pkg") == 0);
    { const pkg_package *removed = NULL; assert(pkg_snapshot_package_at(result, 1U, &removed) == PKG_OK); assert(pkg_package_get_state(removed) == PKG_INSTALLATION_REMOVED); assert(pkg_package_artifact_count(removed) == 0U); }
    { const pkg_artifact *artifact = NULL; assert(pkg_snapshot_artifact_at(result, 1U, &artifact) == PKG_OK); assert(pkg_artifact_get_state(artifact) == PKG_ARTIFACT_MISSING); assert(pkg_snapshot_artifact_at(result, 2U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_SYMLINK); assert(pkg_artifact_get_state(artifact) == PKG_ARTIFACT_PRESENT); assert(pkg_snapshot_artifact_at(result, 3U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_SYMLINK); assert(pkg_artifact_get_state(artifact) == PKG_ARTIFACT_BROKEN_LINK); assert(pkg_snapshot_artifact_at(result, 4U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_DIRECTORY); assert(pkg_snapshot_artifact_at(result, 5U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_OTHER); assert(pkg_snapshot_artifact_at(result, 6U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_REGULAR); assert(pkg_snapshot_artifact_at(result, 7U, &artifact) == PKG_OK); if (geteuid() != 0) assert(pkg_artifact_get_state(artifact) == PKG_ARTIFACT_PERMISSION_DENIED); assert(pkg_snapshot_artifact_at(result, 8U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_UNKNOWN); assert(pkg_artifact_get_state(artifact) == PKG_ARTIFACT_UNVERIFIABLE); assert(pkg_snapshot_artifact_at(result, 9U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_SYMLINK); assert(pkg_artifact_get_state(artifact) == PKG_ARTIFACT_BROKEN_LINK); }
    { const pkg_package *package = NULL; const pkg_artifact *artifact = NULL; pkg_path path_view; assert(pkg_snapshot_package_at(result, 0U, &package) == PKG_OK); assert(pkg_package_artifact_count(package) == 10U); assert(pkg_package_artifact_at(package, 0U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_REGULAR); path_view = pkg_artifact_path(artifact); assert(path_view.size == strlen("/usr/bin/present")); assert(pkg_package_artifact_at(package, 3U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_SYMLINK); }
    assert(pkg_scan_result_diagnostic_count(result) == 2U);
    { const pkg_diagnostic *diagnostic = NULL; pkg_string_view code; assert(pkg_snapshot_diagnostic_at(result, 0U, &diagnostic) == PKG_OK); code = pkg_diagnostic_code(diagnostic); assert(code.size == strlen("PKG_DPKG_FILELIST_MALFORMED")); assert(memcmp(code.data, "PKG_DPKG_FILELIST_MALFORMED", code.size) == 0); }
    pkg_scan_result_destroy(result); pkg_target_destroy(target);
    assert(pkg_target_create_rootfs(context, fixture, &target) == PKG_OK);
    { pkg_scan_options limited = PKG_SCAN_OPTIONS_INIT; limited.max_packages = 10U; limited.max_package_files = 1U; assert(pkg_scan(context, target, &limited, &limited_result) == PKG_ERR_RESOURCE_LIMIT); assert(limited_result != NULL); assert(pkg_scan_result_package_count(limited_result) == 2U); assert(pkg_scan_result_package_file_count(limited_result, 0U) == 1U); assert(pkg_snapshot_artifact_count(limited_result) == 1U); }
    pkg_scan_result_destroy(limited_result); pkg_target_destroy(target);
    assert(pkg_target_create_rootfs(context, fixture, &target) == PKG_OK);
    { pkg_scan_options exact_limit = PKG_SCAN_OPTIONS_INIT; const pkg_artifact *artifact = NULL; exact_limit.max_packages = 10U; exact_limit.max_package_files = 10U; assert(pkg_scan(context, target, &exact_limit, &limited_result) == PKG_OK); assert(limited_result != NULL); assert(pkg_scan_result_package_file_count(limited_result, 0U) == 10U); assert(pkg_snapshot_artifact_count(limited_result) == 10U); assert(pkg_snapshot_artifact_at(limited_result, 8U, &artifact) == PKG_OK); assert(pkg_artifact_get_kind(artifact) == PKG_ARTIFACT_UNKNOWN); assert(pkg_artifact_get_state(artifact) == PKG_ARTIFACT_UNVERIFIABLE); }
    pkg_scan_result_destroy(limited_result); pkg_target_destroy(target);
    assert(pkg_target_create_rootfs(context, fixture, &target) == PKG_OK);
    {
        char path[512];
        FILE *file;
        char *record;
        const size_t boundaries[] = {65535U, 65536U, 65537U};
        for (size_t i = 0U; i < sizeof(boundaries) / sizeof(boundaries[0]); ++i) {
            size_t length = boundaries[i];
            record = malloc(length + 2U);
            assert(record != NULL);
            record[0] = '/';
            memset(record + 1U, 'x', length - 1U);
            record[length] = '\n';
            record[length + 1U] = '\0';
            assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg/info/fixture-pkg.list", fixture) > 0);
            file = fopen(path, "wb"); assert(file != NULL);
            assert(fwrite(record, 1U, length + 1U, file) == length + 1U);
            assert(fclose(file) == 0);
            free(record);
            assert(pkg_scan(context, target, NULL, &limited_result) ==
                   (length <= 65536U ? PKG_OK : PKG_ERR_RESOURCE_LIMIT));
            assert(limited_result != NULL);
            assert(pkg_scan_result_package_count(limited_result) == 2U);
            assert(pkg_scan_result_package_file_count(limited_result, 0U) ==
                   (length <= 65536U ? 1U : 0U));
            pkg_scan_result_destroy(limited_result);
            limited_result = NULL;
        }
        /* A record without a final LF is still a complete EOF-terminated record. */
        assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg/info/fixture-pkg.list", fixture) > 0);
        file = fopen(path, "wb"); assert(file != NULL);
        assert(fputs("/usr/bin/present", file) >= 0);
        assert(fclose(file) == 0);
        assert(pkg_scan(context, target, NULL, &limited_result) == PKG_OK);
        assert(limited_result != NULL);
        assert(pkg_scan_result_package_file_count(limited_result, 0U) == 1U);
        pkg_scan_result_destroy(limited_result); limited_result = NULL;

        /* Restore the normal fixture before the following resource tests. */
        file = fopen(path, "wb"); assert(file != NULL);
        assert(fputs("/usr/bin/present\n", file) >= 0);
        assert(fclose(file) == 0);
    }
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
        assert(pkg_scan_result_package_count(limited_result) == 2U);
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
        unsupported = (pkg_scan_options)PKG_SCAN_OPTIONS_INIT;
        unsupported.flags = PKG_SCAN_INCLUDE_ELF;
        assert(pkg_scan(context, target, &unsupported, &limited_result) == PKG_ERR_UNSUPPORTED);
        assert(limited_result == NULL);
        unsupported = (pkg_scan_options)PKG_SCAN_OPTIONS_INIT;
        unsupported.flags = PKG_SCAN_INCLUDE_CACHES;
        assert(pkg_scan(context, target, &unsupported, &limited_result) == PKG_ERR_UNSUPPORTED);
        assert(limited_result == NULL);
        unsupported = (pkg_scan_options)PKG_SCAN_OPTIONS_INIT;
        unsupported.flags = PKG_SCAN_INCLUDE_CAPABILITIES;
        assert(pkg_scan(context, target, &unsupported, &limited_result) == PKG_ERR_UNSUPPORTED);
        assert(limited_result == NULL);
        unsupported = (pkg_scan_options)PKG_SCAN_OPTIONS_INIT;
        unsupported.flags = PKG_SCAN_INCLUDE_ELF | PKG_SCAN_INCLUDE_CACHES | PKG_SCAN_INCLUDE_CAPABILITIES;
        assert(pkg_scan(context, target, &unsupported, &limited_result) == PKG_ERR_UNSUPPORTED);
        assert(limited_result == NULL);
    }
    pkg_target_destroy(target);
    remove_fixture(fixture);

    {
        char state_fixture[256];
        pkg_scan_result *state_result = NULL;
        make_state_fixture(state_fixture, sizeof(state_fixture));
        assert(pkg_target_create_rootfs(context, state_fixture, &target) == PKG_OK);
        assert(pkg_scan(context, target, NULL, &state_result) == PKG_OK);
        assert(state_result != NULL);
        assert(pkg_scan_result_package_count(state_result) == 5U);
        /* Ordering is name/architecture/version, independent of dpkg's discovery order. */
        assert(strcmp(pkg_scan_result_package_name(state_result, 0U), "broken-pkg") == 0);
        assert(strcmp(pkg_scan_result_package_name(state_result, 1U), "installed-pkg") == 0);
        assert(strcmp(pkg_scan_result_package_name(state_result, 2U), "partial-pkg") == 0);
        assert(strcmp(pkg_scan_result_package_name(state_result, 3U), "removed-pkg") == 0);
        assert(strcmp(pkg_scan_result_package_name(state_result, 4U), "unknown-pkg") == 0);
        {
            const pkg_package *package = NULL;
            assert(pkg_snapshot_package_at(state_result, 0U, &package) == PKG_OK);
            assert(pkg_package_get_state(package) == PKG_INSTALLATION_PARTIAL);
            assert(pkg_snapshot_package_at(state_result, 1U, &package) == PKG_OK);
            assert(pkg_package_get_state(package) == PKG_INSTALLATION_INSTALLED);
            assert(pkg_snapshot_package_at(state_result, 2U, &package) == PKG_OK);
            assert(pkg_package_get_state(package) == PKG_INSTALLATION_PARTIAL);
            assert(pkg_snapshot_package_at(state_result, 3U, &package) == PKG_OK);
            assert(pkg_package_get_state(package) == PKG_INSTALLATION_REMOVED);
            assert(pkg_snapshot_package_at(state_result, 4U, &package) == PKG_OK);
            assert(pkg_package_get_state(package) == PKG_INSTALLATION_UNKNOWN);
        }
        /* Selection intent ("hold") does not change the installation state. */
        assert(pkg_scan_result_diagnostic_count(state_result) == 6U);
        {
            size_t diagnostic_count = pkg_scan_result_diagnostic_count(state_result);
            int found_unknown_status = 0;
            for (size_t i = 0U; i < diagnostic_count; ++i) {
                const pkg_diagnostic *diagnostic = NULL;
                pkg_string_view code;
                assert(pkg_snapshot_diagnostic_at(state_result, i, &diagnostic) == PKG_OK);
                code = pkg_diagnostic_code(diagnostic);
                if (code.size == strlen("PKG_DPKG_STATUS_UNKNOWN") &&
                    memcmp(code.data, "PKG_DPKG_STATUS_UNKNOWN", code.size) == 0) {
                    found_unknown_status = 1;
                }
            }
            assert(found_unknown_status == 1);
        }
        pkg_scan_result_destroy(state_result);
        pkg_target_destroy(target);
        remove_state_fixture(state_fixture);
    }

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
