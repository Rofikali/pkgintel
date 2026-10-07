#include "internal/pkg_model.h"
#include "internal/pkg_snapshot.h"
#include "internal/pkg_support.h"
#include <stdlib.h>
#include <string.h>

static int grow_diagnostics(pkg_snapshot *snapshot, size_t required) {
    size_t capacity = snapshot->diagnostic_capacity == 0U ? 8U : snapshot->diagnostic_capacity;
    size_t new_capacity = capacity;
    pkg_diagnostic_record *grown;
    while (new_capacity < required) {
        if (new_capacity > SIZE_MAX / 2U) {
            new_capacity = required;
            break;
        }
        new_capacity *= 2U;
    }
    if (snapshot->max_diagnostics != 0U && new_capacity > snapshot->max_diagnostics) new_capacity = snapshot->max_diagnostics;
    if (new_capacity < required || new_capacity > SIZE_MAX / sizeof(*grown)) return -1;
    grown = realloc(snapshot->diagnostics, new_capacity * sizeof(*grown));
    if (grown == NULL) return -1;
    snapshot->diagnostics = grown;
    snapshot->diagnostic_capacity = new_capacity;
    return 0;
}

int pkg_snapshot_add_diagnostic(pkg_snapshot *snapshot, pkg_status status, pkg_diagnostic_severity severity, pkg_evidence_source source, const char *code, const char *message) {
    char *owned_code;
    char *owned_message;
    size_t index;
    size_t code_bytes;
    size_t message_bytes;
    size_t total_bytes;
    if (snapshot == NULL || code == NULL || message == NULL) return -1;
    if (snapshot->max_diagnostics != 0U && snapshot->diagnostic_count >= snapshot->max_diagnostics) return -2;
    code_bytes = strlen(code) + 1U;
    message_bytes = strlen(message) + 1U;
    if (code_bytes > SIZE_MAX - message_bytes) return -1;
    total_bytes = code_bytes + message_bytes;
    if (snapshot->max_string_bytes != 0U && (total_bytes > snapshot->max_string_bytes || snapshot->string_bytes > snapshot->max_string_bytes - total_bytes)) return -2;
    owned_code = pkg_strdup_internal(code);
    if (owned_code == NULL) return -1;
    owned_message = pkg_strdup_internal(message);
    if (owned_message == NULL) {
        free(owned_code);
        return -1;
    }
    if (snapshot->diagnostic_count == SIZE_MAX) {
        free(owned_code);
        free(owned_message);
        return -1;
    }
    index = snapshot->diagnostic_count;
    if (grow_diagnostics(snapshot, index + 1U) != 0) {
        free(owned_code);
        free(owned_message);
        return -1;
    }
    snapshot->diagnostics[index] = (pkg_diagnostic_record){
        .code = owned_code, .message = owned_message,
        .status = status, .severity = severity, .evidence_source = source
    };
    snapshot->diagnostic_count = index + 1U;
    snapshot->string_bytes += total_bytes;
    return 0;
}

pkg_diagnostic_severity pkg_diagnostic_get_severity(const pkg_diagnostic *diagnostic) { const pkg_diagnostic_record *d=(const pkg_diagnostic_record *)diagnostic; return d==NULL?PKG_DIAGNOSTIC_INFO:d->severity; }
pkg_status pkg_diagnostic_get_status(const pkg_diagnostic *diagnostic) { const pkg_diagnostic_record *d=(const pkg_diagnostic_record *)diagnostic; return d==NULL?PKG_ERR_INVALID_ARGUMENT:d->status; }
pkg_string_view pkg_diagnostic_code(const pkg_diagnostic *diagnostic) { const pkg_diagnostic_record *d=(const pkg_diagnostic_record *)diagnostic; return d==NULL||d->code==NULL?(pkg_string_view){NULL,0U}:(pkg_string_view){d->code,strlen(d->code)}; }
pkg_string_view pkg_diagnostic_message(const pkg_diagnostic *diagnostic) { const pkg_diagnostic_record *d=(const pkg_diagnostic_record *)diagnostic; return d==NULL||d->message==NULL?(pkg_string_view){NULL,0U}:(pkg_string_view){d->message,strlen(d->message)}; }
pkg_evidence_source pkg_diagnostic_get_evidence_source(const pkg_diagnostic *diagnostic) { const pkg_diagnostic_record *d=(const pkg_diagnostic_record *)diagnostic; return d==NULL?PKG_EVIDENCE_UNKNOWN:d->evidence_source; }
