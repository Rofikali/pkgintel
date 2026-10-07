#include "internal/pkg_model.h"
#include "internal/pkg_support.h"
#include "internal/pkg_target.h"
#include "internal/pkg_snapshot.h"
#include "internal/pkg_backend.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PKG_DPKG_MAX_RECORD_BYTES UINT64_C(65536)

static int add_artifact_checked(pkg_snapshot *result, const unsigned char *path, size_t path_size,
                                  pkg_artifact_kind kind, pkg_artifact_state state, const struct stat *st) {
    int rc = pkg_snapshot_add_artifact(result, path, path_size, kind, state, st);
    return rc == -2 ? 2 : rc;
}

static int add_diagnostic_checked(pkg_snapshot *result, pkg_status status, pkg_diagnostic_severity severity,
                                  pkg_evidence_source source, const char *code, const char *message) {
    int rc = pkg_snapshot_add_diagnostic(result, status, severity, source, code, message);
    return rc == -2 ? 2 : rc;
}


/*
 * Read one metadata record without allowing the input to grow an attacker-sized
 * heap buffer. The returned record excludes the line terminator and is always
 * NUL-terminated. A return value of -2 means the record exceeded the hard byte
 * bound; the caller must treat that as a resource-limit event.
 */
static int read_bounded_record(FILE *file, char *buffer, size_t buffer_size) {
    size_t length = 0U;
    int ch;
    if (file == NULL || buffer == NULL || buffer_size < 2U) return -1;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            buffer[length] = '\0';
            return 1;
        }
        if (length + 1U >= buffer_size) return -2;
        buffer[length++] = (char)ch;
    }
    if (ferror(file) != 0) return -1;
    if (length == 0U) return 0;
    buffer[length] = '\0';
    return 1;
}

static char *trim_newline(char *value) {
    size_t len;
    if (value == NULL) return NULL;
    len = strlen(value);
    while (len > 0U && (value[len - 1U] == '\n' || value[len - 1U] == '\r')) value[--len] = '\0';
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

static pkg_installation_state installation_state_from_dpkg_status(const char *status) {
    char want[32], eflag[32], state[32], extra[2];
    int fields;
    if (status == NULL) return PKG_INSTALLATION_UNKNOWN;

    fields = sscanf(status, "%31s %31s %31s %1s", want, eflag, state, extra);
    if (fields != 3) return PKG_INSTALLATION_UNKNOWN;
    (void)want;

    if (strcmp(eflag, "reinstreq") == 0) return PKG_INSTALLATION_PARTIAL;
    if (strcmp(state, "installed") == 0) return PKG_INSTALLATION_INSTALLED;
    if (strcmp(state, "not-installed") == 0 || strcmp(state, "config-files") == 0) return PKG_INSTALLATION_REMOVED;
    if (strcmp(state, "half-installed") == 0 ||
        strcmp(state, "unpacked") == 0 ||
        strcmp(state, "half-configured") == 0 ||
        strcmp(state, "triggers-awaited") == 0 ||
        strcmp(state, "triggers-pending") == 0) return PKG_INSTALLATION_PARTIAL;
    return PKG_INSTALLATION_UNKNOWN;
}

static int append_package(pkg_snapshot *result, const pkg_scan_options *options,
                          const char *name, const char *version, const char *architecture,
                          pkg_installation_state installation_state, uint64_t installed_size) {
    pkg_package_record *grown;
    size_t next_count;
    if (result == NULL || name == NULL || version == NULL || architecture == NULL) return -1;
    if (options != NULL && options->max_packages != 0U && result->package_count >= (size_t)options->max_packages) return 1;
    if (result->package_count > SIZE_MAX / sizeof(*grown) - 1U) return -1;
    next_count = result->package_count + 1U;
    grown = realloc(result->packages, next_count * sizeof(*grown));
    if (grown == NULL) return -1;
    result->packages = grown;
    grown[result->package_count].name = pkg_strdup_internal(name);
    grown[result->package_count].version = pkg_strdup_internal(version);
    grown[result->package_count].architecture = pkg_strdup_internal(architecture);
    grown[result->package_count].installation_state = installation_state;
    grown[result->package_count].installed_size = installed_size;
    grown[result->package_count].owner_snapshot = result;
    grown[result->package_count].file_count = 0U;
    grown[result->package_count].missing_file_count = 0U;
    grown[result->package_count].invalid_path_count = 0U;
    grown[result->package_count].artifact_start = 0U;
    grown[result->package_count].artifact_count = 0U;
    if (grown[result->package_count].name == NULL || grown[result->package_count].version == NULL || grown[result->package_count].architecture == NULL) {
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

static int package_record_compare(const void *left, const void *right) {
    const pkg_package_record *a = (const pkg_package_record *)left;
    const pkg_package_record *b = (const pkg_package_record *)right;
    int cmp = strcmp(a->name, b->name);
    if (cmp != 0) return cmp;
    cmp = strcmp(a->architecture, b->architecture);
    if (cmp != 0) return cmp;
    return strcmp(a->version, b->version);
}

static int package_file_list(pkg_target *target, pkg_snapshot *result, pkg_package_record *package,
                             const pkg_scan_options *options) {
    char path[4096];
    int written, fd;
    FILE *file;
    char line[PKG_DPKG_MAX_RECORD_BYTES + 1U];
    int read_rc;
    uint64_t count = 0U, missing = 0U, invalid = 0U;
    int malformed = 0;
    if (target == NULL || result == NULL || package == NULL) return -1;
    written = snprintf(path, sizeof(path), "/var/lib/dpkg/info/%s.list", package->name);
    if (written < 0 || (size_t)written >= sizeof(path)) return -1;
    fd = pkg_target_open_path(target, path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) return errno == ENOENT ? 1 : -1;
    file = fdopen(fd, "rb");
    if (file == NULL) { (void)close(fd); return -1; }

    while ((read_rc = read_bounded_record(file, line, sizeof(line))) > 0) {
        char *entry;
        struct stat st;
        int add_rc;
        trim_newline(line);
        entry = line;
        if (*entry == '\0') continue;
        if (options != NULL && options->max_package_files != 0U && count >= options->max_package_files) {
            package->file_count = count;
            package->missing_file_count = missing;
            package->invalid_path_count = invalid;
            (void)fclose(file);
            return 2;
        }
        ++count; /* Every non-empty record, including malformed paths, consumes budget. */
        if (entry[0] != '/') {
            ++invalid;
            malformed = 1;
            add_rc = pkg_snapshot_add_artifact(result, (const unsigned char *)entry, strlen(entry), PKG_ARTIFACT_UNKNOWN, PKG_ARTIFACT_UNVERIFIABLE, NULL);
            if (add_rc == 2) { (void)fclose(file); return 2; }
            if (add_rc != 0) { (void)fclose(file); return -1; }
            continue;
        }
        if (pkg_target_lstat_path(target, entry, &st) != 0) {
            pkg_artifact_state artifact_state = PKG_ARTIFACT_UNVERIFIABLE;
            if (errno == ENOENT) { ++missing; artifact_state = PKG_ARTIFACT_MISSING; }
            else if (errno == EACCES || errno == EPERM) { artifact_state = PKG_ARTIFACT_PERMISSION_DENIED; }
            else if (errno == EXDEV || errno == ELOOP || errno == EINVAL) { ++invalid; artifact_state = PKG_ARTIFACT_UNVERIFIABLE; }
            else { ++missing; artifact_state = PKG_ARTIFACT_UNVERIFIABLE; }
            add_rc = pkg_snapshot_add_artifact(result, (const unsigned char *)entry, strlen(entry), PKG_ARTIFACT_UNKNOWN, artifact_state, NULL);
        } else {
            pkg_artifact_kind kind = PKG_ARTIFACT_OTHER;
            pkg_artifact_state artifact_state = PKG_ARTIFACT_PRESENT;
            if (S_ISREG(st.st_mode)) kind = PKG_ARTIFACT_REGULAR;
            else if (S_ISDIR(st.st_mode)) kind = PKG_ARTIFACT_DIRECTORY;
            else if (S_ISLNK(st.st_mode)) {
                int target_fd = pkg_target_open_path(target, entry, O_PATH);
                kind = PKG_ARTIFACT_SYMLINK;
                if (target_fd >= 0) {
                    (void)close(target_fd);
                } else if (errno == ENOENT) {
                    artifact_state = PKG_ARTIFACT_BROKEN_LINK;
                } else if (errno == EACCES || errno == EPERM) {
                    artifact_state = PKG_ARTIFACT_PERMISSION_DENIED;
                } else if (errno == EXDEV || errno == ELOOP || errno == EINVAL) {
                    ++invalid;
                    artifact_state = PKG_ARTIFACT_UNVERIFIABLE;
                } else {
                    artifact_state = PKG_ARTIFACT_UNVERIFIABLE;
                }
            } else if (S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode) || S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode)) {
                kind = PKG_ARTIFACT_OTHER;
            }
            add_rc = pkg_snapshot_add_artifact(result, (const unsigned char *)entry, strlen(entry), kind, artifact_state, &st);
        }
        if (add_rc != 0) { (void)fclose(file); return -1; }
    }
    if (read_rc == -2) {
        package->file_count = count;
        package->missing_file_count = missing;
        package->invalid_path_count = invalid;
        (void)fclose(file);
        return 2;
    }
    if (read_rc < 0) {
        (void)fclose(file);
        return -1;
    }
    (void)fclose(file);
    package->file_count = count;
    package->missing_file_count = missing;
    package->invalid_path_count = invalid;
    if (malformed && pkg_snapshot_add_diagnostic(result, PKG_ERR_PARSE, PKG_DIAGNOSTIC_WARNING, PKG_EVIDENCE_DPKG,
        "PKG_DPKG_FILELIST_MALFORMED", "package file list contains malformed entries") != 0) return -1;
    return 0;
}

static pkg_status correlate_package_files(pkg_target *target, pkg_snapshot *result, const pkg_scan_options *options) {
    size_t i;
    int limited = 0;
    if (target == NULL || result == NULL) return PKG_ERR_INVALID_ARGUMENT;
    for (i = 0U; i < result->package_count; ++i) {
        size_t before = result->artifact_count;
        int rc = package_file_list(target, result, &result->packages[i], options);
        result->packages[i].artifact_start = before;
        result->packages[i].artifact_count = result->artifact_count - before;
        if (rc == 2) { limited = 1; continue; }
        if (rc < 0 && pkg_snapshot_add_diagnostic(result, PKG_ERR_IO, PKG_DIAGNOSTIC_WARNING, PKG_EVIDENCE_DPKG,
            "PKG_DPKG_FILELIST_READ_FAILED", "package file list could not be read") != 0) return PKG_ERR_INTERNAL;
        if (rc == 1 && pkg_snapshot_add_diagnostic(result, PKG_ERR_NOT_FOUND, PKG_DIAGNOSTIC_WARNING, PKG_EVIDENCE_DPKG,
            "PKG_DPKG_FILELIST_MISSING", "package file list is missing") != 0) return PKG_ERR_INTERNAL;
    }
    return limited ? PKG_ERR_RESOURCE_LIMIT : PKG_OK;
}

pkg_status pkg_dpkg_scan(pkg_context *context, pkg_target *target, const pkg_scan_options *options, pkg_snapshot *result) {
    int fd;
    FILE *file;
    char line[PKG_DPKG_MAX_RECORD_BYTES + 1U];
    int read_rc;
    char *name = NULL, *version = NULL, *architecture = NULL, *status = NULL;
    uint64_t installed_size = 0U;
    int parse_error = 0, truncated = 0;
    if (context == NULL || target == NULL || result == NULL || target->root_fd < 0) return PKG_ERR_INVALID_ARGUMENT;
    fd = pkg_target_open_path(target, "/var/lib/dpkg/status", O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        if (errno == ENOENT) return PKG_ERR_NOT_FOUND;
        if (errno == EACCES || errno == EPERM) return PKG_ERR_PERMISSION;
        return PKG_ERR_IO;
    }
    file = fdopen(fd, "rb");
    if (file == NULL) { (void)close(fd); return PKG_ERR_IO; }
    while ((read_rc = read_bounded_record(file, line, sizeof(line))) > 0) {
        if (line[0] == '\0') {
            if (name != NULL && version != NULL && architecture != NULL) {
                pkg_installation_state installation_state = installation_state_from_dpkg_status(status);
                int rc = append_package(result, options, name, version, architecture, installation_state, installed_size);
                if (rc == 1) { truncated = 1; break; }
                if (rc != 0) { parse_error = 1; break; }
                if (installation_state == PKG_INSTALLATION_UNKNOWN &&
                    pkg_snapshot_add_diagnostic(result, PKG_ERR_PARSE, PKG_DIAGNOSTIC_WARNING, PKG_EVIDENCE_DPKG,
                        "PKG_DPKG_STATUS_UNKNOWN", "package status is missing or unrecognized") != 0) {
                    parse_error = 1;
                    break;
                }
            }
            free(name); free(version); free(architecture); free(status);
            name = NULL; version = NULL; architecture = NULL; status = NULL; installed_size = 0U;
            continue;
        }
        if (strncmp(line, "Package: ", 9U) == 0) { free(name); name = pkg_strdup_internal(trim_newline(line + 9U)); }
        else if (strncmp(line, "Version: ", 9U) == 0) { free(version); version = pkg_strdup_internal(trim_newline(line + 9U)); }
        else if (strncmp(line, "Architecture: ", 14U) == 0) { free(architecture); architecture = pkg_strdup_internal(trim_newline(line + 14U)); }
        else if (strncmp(line, "Status: ", 8U) == 0) { free(status); status = pkg_strdup_internal(trim_newline(line + 8U)); }
        else if (strncmp(line, "Installed-Size: ", 16U) == 0) {
            uint64_t kib = 0U;
            if (parse_u64_decimal(trim_newline(line + 16U), &kib) != 0 || kib > UINT64_MAX / UINT64_C(1024)) parse_error = 1;
            else installed_size = kib * UINT64_C(1024);
        }
    }
    if (truncated == 0 && parse_error == 0 &&
        name != NULL && version != NULL && architecture != NULL) {
        pkg_installation_state installation_state = installation_state_from_dpkg_status(status);
        int rc = append_package(result, options, name, version, architecture, installation_state, installed_size);
        if (rc == 1) truncated = 1;
        else if (rc != 0) parse_error = 1;
        if (parse_error == 0 && installation_state == PKG_INSTALLATION_UNKNOWN) {
            if (pkg_snapshot_add_diagnostic(result, PKG_ERR_PARSE, PKG_DIAGNOSTIC_WARNING, PKG_EVIDENCE_DPKG,
                "PKG_DPKG_STATUS_UNKNOWN", "package status is missing or unrecognized") != 0) parse_error = 1;
        }
    }
    if (read_rc == -2) truncated = 1;
    else if (read_rc < 0) parse_error = 1;
    free(name); free(version); free(architecture); free(status);
    if (fclose(file) != 0 && parse_error == 0) parse_error = 1;
    if (parse_error != 0) return PKG_ERR_PARSE;
    if (truncated != 0) return PKG_ERR_RESOURCE_LIMIT;
    {
        /* Keep package ordering deterministic before generating package-owned artifact ranges. */
        if (result->package_count > 1U) qsort(result->packages, result->package_count, sizeof(result->packages[0]), package_record_compare);
        pkg_status correlation = correlate_package_files(target, result, options);
        if (correlation != PKG_OK && correlation != PKG_ERR_RESOURCE_LIMIT) return correlation;
        return correlation;
    }
}
