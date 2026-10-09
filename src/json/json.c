#include "internal/json.h"

#include "pkgintel/artifact.h"
#include "pkgintel/diagnostic.h"
#include "pkgintel/package.h"
#include "pkgintel/snapshot.h"
#include "pkgintel/version.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int putc_checked(FILE *output, int ch) {
    return fputc(ch, output) == EOF ? -1 : 0;
}

static int write_bytes(FILE *output, const void *data, size_t size) {
    if (size == 0U) return 0;
    return fwrite(data, 1U, size, output) == size ? 0 : -1;
}

static int write_literal(FILE *output, const char *literal) {
    return write_bytes(output, literal, strlen(literal));
}

static int write_u64(FILE *output, uint64_t value) {
    char buffer[32];
    int written = snprintf(buffer, sizeof(buffer), "%llu", (unsigned long long)value);
    if (written < 0 || (size_t)written >= sizeof(buffer)) return -1;
    return write_bytes(output, buffer, (size_t)written);
}

/*
 * JSON string values are UTF-8 text. pkgintel's public byte-oriented values
 * are therefore represented as an explicit RFC 4648 standard base64 object.
 * This keeps the v1 contract lossless for arbitrary Linux/package metadata
 * bytes and avoids locale-dependent transcoding.
 */
static int write_base64_value(FILE *output, const unsigned char *data, size_t size) {
    static const char alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t i = 0U;

    if (data == NULL && size != 0U) return -1;
    if (write_literal(output, "{\"encoding\":\"base64\",\"data\":\"") != 0) return -1;

    while (size - i >= 3U) {
        uint32_t value = ((uint32_t)data[i] << 16U) |
                         ((uint32_t)data[i + 1U] << 8U) |
                         (uint32_t)data[i + 2U];
        char encoded[4];
        encoded[0] = alphabet[(value >> 18U) & 0x3FU];
        encoded[1] = alphabet[(value >> 12U) & 0x3FU];
        encoded[2] = alphabet[(value >> 6U) & 0x3FU];
        encoded[3] = alphabet[value & 0x3FU];
        if (write_bytes(output, encoded, sizeof(encoded)) != 0) return -1;
        i += 3U;
    }

    if (i < size) {
        size_t remaining = size - i;
        uint32_t value = (uint32_t)data[i] << 16U;
        char encoded[4];
        encoded[0] = alphabet[(value >> 18U) & 0x3FU];
        if (remaining == 2U) {
            value |= (uint32_t)data[i + 1U] << 8U;
            encoded[1] = alphabet[(value >> 12U) & 0x3FU];
            encoded[2] = alphabet[(value >> 6U) & 0x3FU];
            encoded[3] = '=';
        } else {
            encoded[1] = alphabet[(value >> 12U) & 0x3FU];
            encoded[2] = '=';
            encoded[3] = '=';
        }
        if (write_bytes(output, encoded, sizeof(encoded)) != 0) return -1;
    }

    return write_literal(output, "\"}");
}

static int write_string_view(FILE *output, pkg_string_view value) {
    return write_base64_value(output, (const unsigned char *)value.data, value.size);
}

static int write_path(FILE *output, pkg_path value) {
    return write_base64_value(output, value.data, value.size);
}

static const char *scan_status_name(pkg_status status) {
    switch (status) {
        case PKG_OK: return "complete";
        case PKG_ERR_RESOURCE_LIMIT: return "resource_limit";
        default: return NULL;
    }
}

static const char *status_name(pkg_status status) {
    switch (status) {
        case PKG_OK: return "ok";
        case PKG_ERR_INVALID_ARGUMENT: return "invalid_argument";
        case PKG_ERR_IO: return "io_error";
        case PKG_ERR_PERMISSION: return "permission";
        case PKG_ERR_NOT_FOUND: return "not_found";
        case PKG_ERR_UNSUPPORTED: return "unsupported";
        case PKG_ERR_RESOURCE_LIMIT: return "resource_limit";
        case PKG_ERR_PARSE: return "corrupt_data";
        case PKG_STATUS_CANCELLED: return "cancelled";
        case PKG_ERR_INTERNAL: return "internal_error";
        default: return NULL;
    }
}

static const char *installation_state_name(pkg_installation_state state) {
    switch (state) {
        case PKG_INSTALLATION_UNKNOWN: return "unknown";
        case PKG_INSTALLATION_INSTALLED: return "installed";
        case PKG_INSTALLATION_PARTIAL: return "partial";
        case PKG_INSTALLATION_REMOVED: return "removed";
        default: return NULL;
    }
}

static const char *consistency_state_name(pkg_consistency_state state) {
    switch (state) {
        case PKG_CONSISTENCY_UNKNOWN: return "unknown";
        case PKG_CONSISTENCY_CONSISTENT: return "consistent";
        case PKG_CONSISTENCY_INCONSISTENT: return "inconsistent";
        case PKG_CONSISTENCY_MISSING_ARTIFACT: return "missing_artifact";
        case PKG_CONSISTENCY_BROKEN_LINK: return "broken_link";
        case PKG_CONSISTENCY_PERMISSION_DENIED: return "permission_denied";
        case PKG_CONSISTENCY_UNEXPECTED_ARTIFACT: return "unexpected_artifact";
        case PKG_CONSISTENCY_UNVERIFIABLE: return "unverifiable";
        default: return NULL;
    }
}

static const char *artifact_kind_name(pkg_artifact_kind kind) {
    switch (kind) {
        case PKG_ARTIFACT_UNKNOWN: return "unknown";
        case PKG_ARTIFACT_REGULAR: return "regular";
        case PKG_ARTIFACT_DIRECTORY: return "directory";
        case PKG_ARTIFACT_SYMLINK: return "symlink";
        case PKG_ARTIFACT_OTHER: return "other";
        default: return NULL;
    }
}

static const char *artifact_state_name(pkg_artifact_state state) {
    switch (state) {
        case PKG_ARTIFACT_STATE_UNKNOWN: return "unknown";
        case PKG_ARTIFACT_PRESENT: return "present";
        case PKG_ARTIFACT_MISSING: return "missing";
        case PKG_ARTIFACT_BROKEN_LINK: return "broken_link";
        case PKG_ARTIFACT_PERMISSION_DENIED: return "permission_denied";
        case PKG_ARTIFACT_UNVERIFIABLE: return "unverifiable";
        default: return NULL;
    }
}

static const char *severity_name(pkg_diagnostic_severity severity) {
    switch (severity) {
        case PKG_DIAGNOSTIC_INFO: return "info";
        case PKG_DIAGNOSTIC_NOTICE: return "notice";
        case PKG_DIAGNOSTIC_WARNING: return "warning";
        case PKG_DIAGNOSTIC_ERROR: return "error";
        case PKG_DIAGNOSTIC_FATAL: return "fatal";
        default: return NULL;
    }
}

static const char *evidence_source_name(pkg_evidence_source source) {
    switch (source) {
        case PKG_EVIDENCE_UNKNOWN: return "unknown";
        case PKG_EVIDENCE_DPKG: return "dpkg";
        case PKG_EVIDENCE_APT: return "apt";
        case PKG_EVIDENCE_FILESYSTEM: return "filesystem";
        case PKG_EVIDENCE_ELF: return "elf";
        case PKG_EVIDENCE_PATH: return "path";
        case PKG_EVIDENCE_HEURISTIC: return "heuristic";
        default: return NULL;
    }
}

static int write_artifact(FILE *output, const pkg_artifact *artifact) {
    const char *kind;
    const char *state;
    pkg_path path;
    uint64_t logical_size;
    uint64_t allocated_size;

    if (artifact == NULL) return -1;
    kind = artifact_kind_name(pkg_artifact_get_kind(artifact));
    state = artifact_state_name(pkg_artifact_get_state(artifact));
    if (kind == NULL || state == NULL) return -1;
    path = pkg_artifact_path(artifact);
    logical_size = pkg_artifact_logical_size_bytes(artifact);
    allocated_size = pkg_artifact_allocated_size_bytes(artifact);

    if (write_literal(output, "{\n          \"path\":") != 0 ||
        write_path(output, path) != 0 ||
        write_literal(output, ",\n          \"kind\":\"") != 0 ||
        write_literal(output, kind) != 0 ||
        write_literal(output, "\",\n          \"state\":\"") != 0 ||
        write_literal(output, state) != 0 ||
        write_literal(output, "\",\n          \"logical_size_bytes\":") != 0 ||
        write_u64(output, logical_size) != 0 ||
        write_literal(output, ",\n          \"logical_size_available\":") != 0 ||
        write_literal(output, pkg_artifact_logical_size_available(artifact) ? "true" : "false") != 0 ||
        write_literal(output, ",\n          \"allocated_size_bytes\":") != 0 ||
        write_u64(output, allocated_size) != 0 ||
        write_literal(output, ",\n          \"allocated_size_available\":") != 0 ||
        write_literal(output, pkg_artifact_allocated_size_available(artifact) ? "true" : "false") != 0 ||
        write_literal(output, "\n        }") != 0) return -1;
    return 0;
}

static int write_package(FILE *output, const pkg_package *package) {
    const char *installation;
    const char *consistency;
    size_t artifact_count;
    size_t i;
    pkg_string_view value;

    if (package == NULL) return -1;
    installation = installation_state_name(pkg_package_get_state(package));
    consistency = consistency_state_name(pkg_package_get_consistency(package));
    if (installation == NULL || consistency == NULL) return -1;

    if (write_literal(output, "    {\n      \"name\":") != 0) return -1;
    value = pkg_package_name(package);
    if (write_string_view(output, value) != 0) return -1;
    if (write_literal(output, ",\n      \"version\":") != 0) return -1;
    value = pkg_package_version(package);
    if (write_string_view(output, value) != 0) return -1;
    if (write_literal(output, ",\n      \"architecture\":") != 0) return -1;
    value = pkg_package_architecture(package);
    if (write_string_view(output, value) != 0) return -1;
    if (write_literal(output, ",\n      \"installation_state\":\"") != 0 ||
        write_literal(output, installation) != 0 ||
        write_literal(output, "\",\n      \"consistency\":\"") != 0 ||
        write_literal(output, consistency) != 0 ||
        write_literal(output, "\",\n      \"installed_size_bytes\":") != 0 ||
        write_u64(output, pkg_package_installed_size_bytes(package)) != 0 ||
        write_literal(output, ",\n      \"file_count\":") != 0 ||
        write_u64(output, (uint64_t)pkg_package_artifact_count(package)) != 0 ||
        write_literal(output, ",\n      \"artifacts\": [") != 0) return -1;

    artifact_count = pkg_package_artifact_count(package);
    for (i = 0U; i < artifact_count; ++i) {
        const pkg_artifact *artifact = NULL;
        if (pkg_package_artifact_at(package, i, &artifact) != PKG_OK) return -1;
        if (i != 0U && putc_checked(output, ',') != 0) return -1;
        if (write_literal(output, "\n") != 0 || write_artifact(output, artifact) != 0) return -1;
    }
    if (artifact_count != 0U && write_literal(output, "\n      ") != 0) return -1;
    return write_literal(output, "]\n    }");
}

static int write_diagnostic(FILE *output, const pkg_diagnostic *diagnostic) {
    const char *status;
    const char *severity;
    const char *source;
    pkg_string_view code;
    pkg_string_view message;

    if (diagnostic == NULL) return -1;
    status = status_name(pkg_diagnostic_get_status(diagnostic));
    severity = severity_name(pkg_diagnostic_get_severity(diagnostic));
    source = evidence_source_name(pkg_diagnostic_get_evidence_source(diagnostic));
    if (status == NULL || severity == NULL || source == NULL) return -1;
    code = pkg_diagnostic_code(diagnostic);
    message = pkg_diagnostic_message(diagnostic);

    if (write_literal(output, "    {\n      \"code\":") != 0 ||
        write_string_view(output, code) != 0 ||
        write_literal(output, ",\n      \"message\":") != 0 ||
        write_string_view(output, message) != 0 ||
        write_literal(output, ",\n      \"status\":\"") != 0 ||
        write_literal(output, status) != 0 ||
        write_literal(output, "\",\n      \"severity\":\"") != 0 ||
        write_literal(output, severity) != 0 ||
        write_literal(output, "\",\n      \"evidence_source\":\"") != 0 ||
        write_literal(output, source) != 0 ||
        write_literal(output, "\n    }") != 0) return -1;
    return 0;
}

pkg_status pkg_json_write(FILE *output, const struct pkg_snapshot *snapshot, pkg_status scan_status) {
    const char *scan_name;
    size_t package_count;
    size_t diagnostic_count;
    size_t i;

    if (output == NULL || snapshot == NULL) return PKG_ERR_INVALID_ARGUMENT;
    scan_name = scan_status_name(scan_status);
    if (scan_name == NULL) return PKG_ERR_INVALID_ARGUMENT;

    if (write_literal(output,
        "{\n"
        "  \"schema\": {\"name\": \"pkgintel.scan\", \"version\": 1},\n"
        "  \"producer\": {\"name\": \"pkgintel\", \"version\": ") != 0) return PKG_ERR_INTERNAL;
    {
        char version[32];
        int written = snprintf(version, sizeof(version), "%u.%u.%u",
                               PKGINTEL_VERSION_MAJOR, PKGINTEL_VERSION_MINOR, PKGINTEL_VERSION_PATCH);
        if (written < 0 || (size_t)written >= sizeof(version)) return PKG_ERR_INTERNAL;
        if (putc_checked(output, '"') != 0 ||
            write_bytes(output, version, (size_t)written) != 0 ||
            write_literal(output, "\"},\n") != 0) return PKG_ERR_INTERNAL;
    }

    if (write_literal(output, "  \"scan\": {\"status\": \"") != 0 ||
        write_literal(output, scan_name) != 0 ||
        write_literal(output, "\"},\n  \"packages\": [") != 0) return PKG_ERR_INTERNAL;

    package_count = pkg_snapshot_package_count(snapshot);
    for (i = 0U; i < package_count; ++i) {
        const pkg_package *package = NULL;
        if (pkg_snapshot_package_at(snapshot, i, &package) != PKG_OK) return PKG_ERR_INTERNAL;
        if (i != 0U && putc_checked(output, ',') != 0) return PKG_ERR_INTERNAL;
        if (write_literal(output, "\n") != 0 || write_package(output, package) != 0) return PKG_ERR_INTERNAL;
    }
    if (package_count != 0U && write_literal(output, "\n  ") != 0) return PKG_ERR_INTERNAL;
    if (write_literal(output, "],\n  \"diagnostics\": [") != 0) return PKG_ERR_INTERNAL;

    diagnostic_count = pkg_snapshot_diagnostic_count(snapshot);
    for (i = 0U; i < diagnostic_count; ++i) {
        const pkg_diagnostic *diagnostic = NULL;
        if (pkg_snapshot_diagnostic_at(snapshot, i, &diagnostic) != PKG_OK) return PKG_ERR_INTERNAL;
        if (i != 0U && putc_checked(output, ',') != 0) return PKG_ERR_INTERNAL;
        if (write_literal(output, "\n") != 0 || write_diagnostic(output, diagnostic) != 0) return PKG_ERR_INTERNAL;
    }
    if (diagnostic_count != 0U && write_literal(output, "\n  ") != 0) return PKG_ERR_INTERNAL;
    if (write_literal(output, "]\n}\n") != 0) return PKG_ERR_INTERNAL;
    if (fflush(output) != 0) return PKG_ERR_IO;
    return PKG_OK;
}
