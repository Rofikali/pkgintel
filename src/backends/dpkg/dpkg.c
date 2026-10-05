#include "core/pkg_internal.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int path_join(char *out, size_t out_size, const char *root, const char *relative) {
    int written;
    if (out == NULL || out_size == 0U || root == NULL || relative == NULL) return -1;
    written = snprintf(out, out_size, "%s%s", root, relative);
    return (written < 0 || (size_t)written >= out_size) ? -1 : 0;
}

static char *trim_newline(char *value) {
    size_t len;
    if (value == NULL) return NULL;
    len = strlen(value);
    while (len > 0U && (value[len - 1U] == '\n' || value[len - 1U] == '\r')) {
        value[--len] = '\0';
    }
    return value;
}

static int parse_u64_decimal(const char *value, uint64_t *out) {
    unsigned long long parsed;
    char *end = NULL;
    if (value == NULL || out == NULL || *value == '\0') return -1;
    errno = 0;
    parsed = strtoull(value, &end, 10);
    if (errno != 0 || end == value || *end != '\0') return -1;
    *out = (uint64_t)parsed;
    return 0;
}

static int append_package(pkg_scan_result *result, const pkg_scan_options *options,
                          const char *name, const char *version, const char *architecture,
                          uint64_t installed_size) {
    pkg_package_record *grown;
    size_t next_count;
    if (result == NULL || name == NULL || version == NULL || architecture == NULL) return -1;
    if (options != NULL && options->max_packages != 0U &&
        result->package_count >= (size_t)options->max_packages) return 1;
    if (result->package_count == SIZE_MAX) return -1;
    next_count = result->package_count + 1U;
    grown = realloc(result->packages, next_count * sizeof(*grown));
    if (grown == NULL) return -1;
    result->packages = grown;
    grown[result->package_count].name = pkg_strdup_internal(name);
    grown[result->package_count].version = pkg_strdup_internal(version);
    grown[result->package_count].architecture = pkg_strdup_internal(architecture);
    grown[result->package_count].installed_size = installed_size;
    grown[result->package_count].file_count = 0U;
    grown[result->package_count].missing_file_count = 0U;
    if (grown[result->package_count].name == NULL ||
        grown[result->package_count].version == NULL ||
        grown[result->package_count].architecture == NULL) {
        free(grown[result->package_count].name);
        free(grown[result->package_count].version);
        free(grown[result->package_count].architecture);
        grown[result->package_count].name = NULL;
        grown[result->package_count].version = NULL;
        grown[result->package_count].architecture = NULL;
        return -1;
    }
    result->package_count = next_count;
    return 0;
}

static int is_installed_status(const char *status) {
    const char *last_space;
    if (status == NULL) return 0;
    last_space = strrchr(status, ' ');
    return last_space != NULL && strcmp(last_space + 1, "installed") == 0;
}

pkg_status pkg_dpkg_scan(pkg_context *context, pkg_target *target,
                         const pkg_scan_options *options, pkg_scan_result *result) {
    char status_path[4096];
    FILE *file;
    char *line = NULL;
    size_t capacity = 0U;
    char *name = NULL;
    char *version = NULL;
    char *architecture = NULL;
    char *status = NULL;
    uint64_t installed_size = 0U;
    int parse_error = 0;
    int truncated = 0;

    if (context == NULL || target == NULL || result == NULL) return PKG_ERR_INVALID_ARGUMENT;
    if (path_join(status_path, sizeof(status_path), target->root, "/var/lib/dpkg/status") != 0)
        return PKG_ERR_RESOURCE_LIMIT;

    file = fopen(status_path, "rb");
    if (file == NULL) {
        if (errno == ENOENT) return PKG_ERR_NOT_FOUND;
        if (errno == EACCES || errno == EPERM) return PKG_ERR_PERMISSION;
        return PKG_ERR_IO;
    }

    while (getline(&line, &capacity, file) >= 0) {
        char *colon;
        char *key;
        char *value;
        trim_newline(line);
        if (line[0] == '\0') {
            if (name != NULL && version != NULL && architecture != NULL && is_installed_status(status)) {
                int append_result = append_package(result, options, name, version, architecture, installed_size);
                if (append_result < 0) { parse_error = 1; break; }
                if (append_result > 0) { truncated = 1; break; }
            }
            free(name); name = NULL;
            free(version); version = NULL;
            free(architecture); architecture = NULL;
            free(status); status = NULL;
            installed_size = 0U;
            continue;
        }
        colon = strchr(line, ':');
        if (colon == NULL || colon == line) { parse_error = 1; continue; }
        *colon = '\0';
        key = line;
        value = colon + 1;
        while (*value == ' ' || *value == '\t') ++value;

        if (strcmp(key, "Package") == 0) {
            free(name); name = pkg_strdup_internal(value);
        } else if (strcmp(key, "Version") == 0) {
            free(version); version = pkg_strdup_internal(value);
        } else if (strcmp(key, "Architecture") == 0) {
            free(architecture); architecture = pkg_strdup_internal(value);
        } else if (strcmp(key, "Status") == 0) {
            free(status); status = pkg_strdup_internal(value);
        } else if (strcmp(key, "Installed-Size") == 0) {
            uint64_t parsed;
            if (parse_u64_decimal(value, &parsed) != 0) parse_error = 1;
            else if (parsed > UINT64_MAX / 1024U) parse_error = 1;
            else installed_size = parsed * 1024U;
        }
    }

    if (ferror(file) != 0 && !parse_error) parse_error = 1;
    if (feof(file) != 0 && name != NULL && version != NULL && architecture != NULL && is_installed_status(status)) {
        int append_result = append_package(result, options, name, version, architecture, installed_size);
        if (append_result < 0) parse_error = 1;
        if (append_result > 0) truncated = 1;
    }

    free(name);
    free(version);
    free(architecture);
    free(status);
    free(line);
    (void)fclose(file);

    if (parse_error) return PKG_ERR_PARSE;
    if (truncated) return PKG_ERR_RESOURCE_LIMIT;
    return PKG_OK;
}
