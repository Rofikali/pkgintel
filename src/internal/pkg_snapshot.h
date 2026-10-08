#ifndef PKGINTEL_INTERNAL_SNAPSHOT_H
#define PKGINTEL_INTERNAL_SNAPSHOT_H

#include "pkg_model.h"
#include "pkgintel/pkgintel.h"
#include <sys/stat.h>

int pkg_snapshot_add_artifact(struct pkg_snapshot *snapshot, const unsigned char *path,
                              size_t path_size, pkg_artifact_kind kind,
                              pkg_artifact_state state, const struct stat *st);
int pkg_snapshot_add_diagnostic(struct pkg_snapshot *snapshot, pkg_status status,
                                pkg_diagnostic_severity severity,
                                pkg_evidence_source source,
                                const char *code, const char *message);

#endif
