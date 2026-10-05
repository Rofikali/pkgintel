#include "core/pkg_internal.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
    if (errno != 0 || end == value || *end != '\0' || parsed > UINT64_MAX) return -1;
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
    if (result->package_count > SIZE_MAX / sizeof(*grown) - 1U) return -1;
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

static int package_file_list(pkg_target *target, pkg_snapshot *result, pkg_package_record *package,
                             const pkg_scan_options *options) {
    char path[4096];
    int written;
    int fd;
    FILE *file;
    char *line = NULL;
    size_t capacity = 0U;
    uint64_t count = 0U;
    uint64_t missing = 0U;
    uint64_t invalid = 0U;

    if (target == NULL || result == NULL || package == NULL) return -1;
    written = snprintf(path, sizeof(path), "/var/lib/dpkg/info/%s.list", package->name);
    if (written < 0 || (size_t)written >= sizeof(path)) return -1;

    fd = pkg_target_open_path(target, path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        if (errno == ENOENT) return 1;
        return -1;
    }
    file = fdopen(fd, "rb");
    if (file == NULL) {
        (void)close(fd);
        return -1;
    }

    while (getline(&line, &capacity, file) >= 0) {
        char *entry;
        struct stat st;
        trim_newline(line);
        entry = line;
        if (*entry == '\0') continue;
        if (entry[0] != '/') {
            ++invalid;
            ++count;
            (void)pkg_snapshot_add_artifact(result, entry, PKG_ARTIFACT_UNKNOWN, PKG_ARTIFACT_UNVERIFIABLE, NULL);
            continue;
        }
        if (options != NULL && options->max_package_files != 0U &&
            count >= options->max_package_files) {
            free(line);
            (void)fclose(file);
            return 2;
        }
        if (pkg_target_lstat_path(target, entry, &st) != 0) {
            pkg_artifact_state artifact_state = PKG_ARTIFACT_UNVERIFIABLE;
            if (errno == ENOENT) { ++missing; artifact_state = PKG_ARTIFACT_MISSING; }
            else if (errno == EACCES || errno == EPERM) { ++missing; artifact_state = PKG_ARTIFACT_PERMISSION_DENIED; }
            else if (errno == EXDEV || errno == ELOOP || errno == EINVAL) { ++invalid; artifact_state = PKG_ARTIFACT_UNVERIFIABLE; }
            else { ++missing; artifact_state = PKG_ARTIFACT_UNVERIFIABLE; }
            (void)pkg_snapshot_add_artifact(result, entry, PKG_ARTIFACT_UNKNOWN, artifact_state, NULL);
        } else {
            pkg_artifact_kind kind = PKG_ARTIFACT_OTHER;
            if (S_ISREG(st.st_mode)) kind = PKG_ARTIFACT_REGULAR;
            else if (S_ISDIR(st.st_mode)) kind = PKG_ARTIFACT_DIRECTORY;
            else if (S_ISLNK(st.st_mode)) kind = PKG_ARTIFACT_SYMLINK;
            (void)pkg_snapshot_add_artifact(result, entry, kind, PKG_ARTIFACT_PRESENT, &st);
        }
        ++count;
    }

    if (ferror(file) != 0) {
        free(line);
        (void)fclose(file);
        return -1;
    }

    free(line);
    (void)fclose(file);
    ((pkg_package_record *)package)->file_count = count;
    ((pkg_package_record *)package)->missing_file_count = missing;
    ((pkg_package_record *)package)->invalid_path_count = invalid;
    return 0;
}

static pkg_status correlate_package_files(pkg_target *target, pkg_snapshot *result,
                                          const pkg_scan_options *options) {
    size_t i;
    int limited = 0;
    if (target == NULL || result == NULL) return PKG_ERR_INVALID_ARGUMENT;
    for (i = 0U; i < result->package_count; ++i) {
        size_t before = result->artifact_count;
        int rc = package_file_list(target, result, &result->packages[i], options);
        result->packages[i].artifact_start = before;
        result->packages[i].artifact_count = result->artifact_count - before;
        if (rc == 2) {
            limited = 1;
            continue;
        }
        if (rc < 0) {
            (void)pkg_snapshot_add_diagnostic(result, PKG_ERR_IO, PKG_DIAGNOSTIC_WARNING, PKG_EVIDENCE_DPKG, "PKG_DPKG_FILELIST_READ_FAILED", "package file list could not be read");
        }
        if (rc == 1) {
            (void)pkg_snapshot_add_diagnostic(result, PKG_ERR_NOT_FOUND, PKG_DIAGNOSTIC_WARNING, PKG_EVIDENCE_DPKG, "PKG_DPKG_FILELIST_MISSING", "package file list is missing");
        }
    }
    return limited ? PKG_ERR_RESOURCE_LIMIT : PKG_OK;
}

pkg_status pkg_dpkg_scan(pkg_context *context, pkg_target *target,
                         const pkg_scan_options *options, pkg_scan_result *result) {
    int fd;
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

    if (context == NULL || target == NULL || result == NULL || target->root_fd < 0)
        return PKG_ERR_INVALID_ARGUMENT;

    fd = pkg_target_open_path(target, "/var/lib/dpkg/status", O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        if (errno == ENOENT) return PKG_ERR_NOT_FOUND;
        if (errno == EACCES || errno == EPERM) return PKG_ERR_PERMISSION;
        return PKG_ERR_IO;
    }
    file = fdopen(fd, "rb");
    if (file == NULL) {
        (void)close(fd);
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
            if (parse_u64_decimal(value, &parsed) != 0 || parsed > UINT64_MAX / 1024U) parse_error = 1;
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

    {
        pkg_status correlation = correlate_package_files(target, result, options);
        if (correlation != PKG_OK) return correlation;
    }
    return PKG_OK;
}
