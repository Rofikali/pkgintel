#include "internal/pkg_model.h"
#include "internal/pkg_snapshot.h"
#include "internal/pkg_support.h"
#include <stdlib.h>
#include <string.h>

int pkg_snapshot_add_diagnostic(pkg_snapshot *snapshot, pkg_status status, pkg_diagnostic_severity severity, pkg_evidence_source source, const char *code, const char *message) {
    pkg_diagnostic_record *grown;
    char *owned_code;
    char *owned_message;
    size_t n;

    if (snapshot == NULL || code == NULL || message == NULL) return -1;
    if (snapshot->max_diagnostics != 0U && snapshot->diagnostic_count >= snapshot->max_diagnostics) return -2;
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
    n = snapshot->diagnostic_count + 1U;
    if (n > SIZE_MAX / sizeof(*grown)) {
        free(owned_code);
        free(owned_message);
        return -1;
    }
    grown = realloc(snapshot->diagnostics, n * sizeof(*grown));
    if (grown == NULL) {
        free(owned_code);
        free(owned_message);
        return -1;
    }

    snapshot->diagnostics = grown;
    grown[n - 1U] = (pkg_diagnostic_record){
        .code = owned_code,
        .message = owned_message,
        .status = status,
        .severity = severity,
        .evidence_source = source
    };
    snapshot->diagnostic_count = n;
    return 0;
}

pkg_diagnostic_severity pkg_diagnostic_get_severity(const pkg_diagnostic *diagnostic) { const pkg_diagnostic_record *d=(const pkg_diagnostic_record *)diagnostic; return d==NULL?PKG_DIAGNOSTIC_INFO:d->severity; }

pkg_status pkg_diagnostic_get_status(const pkg_diagnostic *diagnostic) { const pkg_diagnostic_record *d=(const pkg_diagnostic_record *)diagnostic; return d==NULL?PKG_ERR_INVALID_ARGUMENT:d->status; }

pkg_string_view pkg_diagnostic_code(const pkg_diagnostic *diagnostic) { const pkg_diagnostic_record *d=(const pkg_diagnostic_record *)diagnostic; return d==NULL||d->code==NULL?(pkg_string_view){NULL,0U}:(pkg_string_view){d->code,strlen(d->code)}; }

pkg_string_view pkg_diagnostic_message(const pkg_diagnostic *diagnostic) { const pkg_diagnostic_record *d=(const pkg_diagnostic_record *)diagnostic; return d==NULL||d->message==NULL?(pkg_string_view){NULL,0U}:(pkg_string_view){d->message,strlen(d->message)}; }

pkg_evidence_source pkg_diagnostic_get_evidence_source(const pkg_diagnostic *diagnostic) { const pkg_diagnostic_record *d=(const pkg_diagnostic_record *)diagnostic; return d==NULL?PKG_EVIDENCE_UNKNOWN:d->evidence_source; }
