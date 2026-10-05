#ifndef PKGINTEL_DIAGNOSTIC_H
#define PKGINTEL_DIAGNOSTIC_H
#include "snapshot.h"
typedef enum pkg_diagnostic_severity {
    PKG_DIAGNOSTIC_INFO = 0,
    PKG_DIAGNOSTIC_NOTICE = 1,
    PKG_DIAGNOSTIC_WARNING = 2,
    PKG_DIAGNOSTIC_ERROR = 3,
    PKG_DIAGNOSTIC_FATAL = 4
} pkg_diagnostic_severity;
typedef enum pkg_evidence_source {
    PKG_EVIDENCE_UNKNOWN = 0,
    PKG_EVIDENCE_DPKG = 1,
    PKG_EVIDENCE_APT = 2,
    PKG_EVIDENCE_FILESYSTEM = 3,
    PKG_EVIDENCE_ELF = 4,
    PKG_EVIDENCE_PATH = 5,
    PKG_EVIDENCE_HEURISTIC = 6
} pkg_evidence_source;
pkg_diagnostic_severity pkg_diagnostic_get_severity(const pkg_diagnostic *diagnostic);
pkg_status pkg_diagnostic_get_status(const pkg_diagnostic *diagnostic);
pkg_string_view pkg_diagnostic_code(const pkg_diagnostic *diagnostic);
pkg_string_view pkg_diagnostic_message(const pkg_diagnostic *diagnostic);
pkg_evidence_source pkg_diagnostic_get_evidence_source(const pkg_diagnostic *diagnostic);
#endif
