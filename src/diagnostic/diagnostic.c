#include "internal/pkg_model.h"
#include "internal/pkg_support.h"
#include <stdlib.h>
#include <string.h>

int pkg_snapshot_add_diagnostic(pkg_snapshot *snapshot, pkg_status status, pkg_diagnostic_severity severity, pkg_evidence_source source, const char *code, const char *message) {
    pkg_diagnostic_record *grown;
    size_t n;
    if (snapshot == NULL || code == NULL || message == NULL) return -1;
    n = snapshot->diagnostic_count + 1U;
    if (n < snapshot->diagnostic_count || n > SIZE_MAX / sizeof(*grown)) return -1;
    grown = realloc(snapshot->diagnostics, n * sizeof(*grown));
    if (grown == NULL) return -1;
    snapshot->diagnostics = grown;
    memset(&grown[n - 1U], 0, sizeof(grown[n - 1U]));
    grown[n - 1U].code = pkg_strdup_internal(code);
    grown[n - 1U].message = pkg_strdup_internal(message);
    if (grown[n - 1U].code == NULL || grown[n - 1U].message == NULL) { free(grown[n - 1U].code); free(grown[n - 1U].message); return -1; }
    grown[n - 1U].status = status;
    grown[n - 1U].severity = severity;
    grown[n - 1U].evidence_source = source;
    snapshot->diagnostic_count = n;
    return 0;
}

pkg_diagnostic_severity pkg_diagnostic_get_severity(const pkg_diagnostic *diagnostic) { const pkg_diagnostic_record *d=(const pkg_diagnostic_record *)diagnostic; return d==NULL?PKG_DIAGNOSTIC_INFO:d->severity; }

pkg_status pkg_diagnostic_get_status(const pkg_diagnostic *diagnostic) { const pkg_diagnostic_record *d=(const pkg_diagnostic_record *)diagnostic; return d==NULL?PKG_ERR_INVALID_ARGUMENT:d->status; }

pkg_string_view pkg_diagnostic_code(const pkg_diagnostic *diagnostic) { const pkg_diagnostic_record *d=(const pkg_diagnostic_record *)diagnostic; return d==NULL||d->code==NULL?(pkg_string_view){NULL,0U}:(pkg_string_view){d->code,strlen(d->code)}; }

pkg_string_view pkg_diagnostic_message(const pkg_diagnostic *diagnostic) { const pkg_diagnostic_record *d=(const pkg_diagnostic_record *)diagnostic; return d==NULL||d->message==NULL?(pkg_string_view){NULL,0U}:(pkg_string_view){d->message,strlen(d->message)}; }

pkg_evidence_source pkg_diagnostic_get_evidence_source(const pkg_diagnostic *diagnostic) { const pkg_diagnostic_record *d=(const pkg_diagnostic_record *)diagnostic; return d==NULL?PKG_EVIDENCE_UNKNOWN:d->evidence_source; }
