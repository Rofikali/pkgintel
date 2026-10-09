#define _GNU_SOURCE
#include "pkgintel/pkgintel.h"
#include "internal/json.h"
#include "fixtures.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static char *read_stream(FILE *file, size_t *out_size) {
    long length;
    char *buffer;
    assert(file != NULL);
    assert(fflush(file) == 0);
    assert(fseek(file, 0L, SEEK_END) == 0);
    length = ftell(file);
    assert(length >= 0);
    assert(fseek(file, 0L, SEEK_SET) == 0);
    buffer = malloc((size_t)length + 1U);
    assert(buffer != NULL);
    assert(fread(buffer, 1U, (size_t)length, file) == (size_t)length);
    buffer[(size_t)length] = '\0';
    if (out_size != NULL) *out_size = (size_t)length;
    return buffer;
}

static void assert_contains(const char *text, const char *needle) {
    assert(text != NULL);
    assert(needle != NULL);
    assert(strstr(text, needle) != NULL);
}

int test_json_behaviour(void) {
    char fixture[256];
    pkg_context *context = NULL;
    pkg_target *target = NULL;
    pkg_scan_result *result = NULL;
    pkg_scan_result *limited = NULL;
    FILE *first;
    FILE *second;
    char *first_text;
    char *second_text;
    size_t first_size;
    size_t second_size;

    assert(pkg_context_create(NULL, &context) == PKG_OK);
    make_fixture(fixture, sizeof(fixture));
    assert(pkg_target_create_rootfs(context, fixture, &target) == PKG_OK);

    assert(pkg_scan(context, target, NULL, &result) == PKG_OK);
    assert(result != NULL);

    first = tmpfile();
    second = tmpfile();
    assert(first != NULL && second != NULL);
    assert(pkg_json_write(first, result, PKG_OK) == PKG_OK);
    assert(pkg_json_write(second, result, PKG_OK) == PKG_OK);
    first_text = read_stream(first, &first_size);
    second_text = read_stream(second, &second_size);

    assert(first_size == second_size);
    assert(memcmp(first_text, second_text, first_size) == 0);
    assert_contains(first_text, "\"schema\": {\"name\": \"pkgintel.scan\", \"version\": 1}");
    assert_contains(first_text, "\"status\": \"complete\"");
    assert_contains(first_text, "\"encoding\":\"base64\",\"data\":\"Zml4dHVyZS1wa2c=");
    assert_contains(first_text, "\"data\":\"L3Vzci9iaW4vcHJlc2VudA==");
    assert_contains(first_text, "\"installation_state\":\"installed\"");
    assert_contains(first_text, "\"consistency\":\"inconsistent\"");
    assert_contains(first_text, "\"diagnostics\": [");
    assert(strstr(first_text, fixture) == NULL);

    free(first_text);
    free(second_text);
    fclose(first);
    fclose(second);

    {
        pkg_scan_options limited_options = PKG_SCAN_OPTIONS_INIT;
        limited_options.flags = PKG_SCAN_CORRELATE_FILES;
        limited_options.max_packages = 10U;
        limited_options.max_package_files = 1U;
        assert(pkg_scan(context, target, &limited_options, &limited) == PKG_ERR_RESOURCE_LIMIT);
        assert(limited != NULL);
        first = tmpfile();
        assert(first != NULL);
        assert(pkg_json_write(first, limited, PKG_ERR_RESOURCE_LIMIT) == PKG_OK);
        first_text = read_stream(first, &first_size);
        assert_contains(first_text, "\"status\": \"resource_limit\"");
        free(first_text);
        fclose(first);
        pkg_scan_result_destroy(limited);
        limited = NULL;
    }

    pkg_scan_result_destroy(result);
    result = NULL;
    pkg_target_destroy(target);
    target = NULL;
    remove_fixture(fixture);

    {
        char raw_fixture[256];
        FILE *status_file;
        char status_path[512];
        assert(mkdtemp(raw_fixture) != NULL);
        assert(snprintf(status_path, sizeof(status_path), "%s/var", raw_fixture) > 0);
        assert(mkdir(status_path, 0700) == 0);
        assert(snprintf(status_path, sizeof(status_path), "%s/var/lib", raw_fixture) > 0);
        assert(mkdir(status_path, 0700) == 0);
        assert(snprintf(status_path, sizeof(status_path), "%s/var/lib/dpkg", raw_fixture) > 0);
        assert(mkdir(status_path, 0700) == 0);
        assert(snprintf(status_path, sizeof(status_path), "%s/var/lib/dpkg/status", raw_fixture) > 0);
        status_file = fopen(status_path, "wb");
        assert(status_file != NULL);
        assert(fputs("Package: bad-pkg\nVersion: 1.0\nArchitecture: amd64\nStatus: install ok installed\nInstalled-Size: 1\n", status_file) >= 0);
        assert(fclose(status_file) == 0);
        assert(snprintf(status_path, sizeof(status_path), "%s/var/lib/dpkg/info", raw_fixture) > 0);
        assert(mkdir(status_path, 0700) == 0);
        assert(snprintf(status_path, sizeof(status_path), "%s/var/lib/dpkg/info/bad-pkg.list", raw_fixture) > 0);
        status_file = fopen(status_path, "wb");
        assert(status_file != NULL);
        assert(fwrite("/usr/bin/bad-", 1U, strlen("/usr/bin/bad-"), status_file) == strlen("/usr/bin/bad-"));
        assert(fwrite("\xff", 1U, 1U, status_file) == 1U);
        assert(fwrite("\n", 1U, 1U, status_file) == 1U);
        assert(fclose(status_file) == 0);

        assert(pkg_target_create_rootfs(context, raw_fixture, &target) == PKG_OK);
        {
            pkg_scan_options correlate = PKG_SCAN_OPTIONS_INIT;
            correlate.flags = PKG_SCAN_CORRELATE_FILES;
            assert(pkg_scan(context, target, &correlate, &result) == PKG_OK);
        }
        first = tmpfile();
        assert(first != NULL);
        assert(pkg_json_write(first, result, PKG_OK) == PKG_OK);
        first_text = read_stream(first, &first_size);
        assert_contains(first_text, "L3Vzci9iaW4vYmFkLf8=");
        free(first_text);
        fclose(first);
        pkg_scan_result_destroy(result);
        pkg_target_destroy(target);
        result = NULL;
        target = NULL;
        assert(snprintf(status_path, sizeof(status_path), "%s/var/lib/dpkg/info/bad-pkg.list", raw_fixture) > 0);
        assert(unlink(status_path) == 0);
        assert(snprintf(status_path, sizeof(status_path), "%s/var/lib/dpkg/status", raw_fixture) > 0);
        assert(unlink(status_path) == 0);
        /* Remove the small fixture tree explicitly; no target data is retained. */
        assert(snprintf(status_path, sizeof(status_path), "%s/var/lib/dpkg/info", raw_fixture) > 0);
        assert(rmdir(status_path) == 0);
        assert(snprintf(status_path, sizeof(status_path), "%s/var/lib/dpkg", raw_fixture) > 0);
        assert(rmdir(status_path) == 0);
        assert(snprintf(status_path, sizeof(status_path), "%s/var/lib", raw_fixture) > 0);
        assert(rmdir(status_path) == 0);
        assert(snprintf(status_path, sizeof(status_path), "%s/var", raw_fixture) > 0);
        assert(rmdir(status_path) == 0);
        assert(rmdir(raw_fixture) == 0);
    }

    pkg_context_destroy(context);
    return 0;
}
