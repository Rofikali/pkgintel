#ifndef PKGINTEL_ARTIFACT_H
#define PKGINTEL_ARTIFACT_H
#include <stdint.h>
#include "snapshot.h"
typedef enum pkg_artifact_kind { PKG_ARTIFACT_UNKNOWN=0, PKG_ARTIFACT_REGULAR=1, PKG_ARTIFACT_DIRECTORY=2, PKG_ARTIFACT_SYMLINK=3, PKG_ARTIFACT_OTHER=4 } pkg_artifact_kind;
typedef enum pkg_artifact_state { PKG_ARTIFACT_STATE_UNKNOWN=0, PKG_ARTIFACT_PRESENT=1, PKG_ARTIFACT_MISSING=2, PKG_ARTIFACT_BROKEN_LINK=3, PKG_ARTIFACT_PERMISSION_DENIED=4, PKG_ARTIFACT_UNVERIFIABLE=5 } pkg_artifact_state;
pkg_path pkg_artifact_path(const pkg_artifact *artifact);
pkg_artifact_kind pkg_artifact_get_kind(const pkg_artifact *artifact);
pkg_artifact_state pkg_artifact_get_state(const pkg_artifact *artifact);
uint64_t pkg_artifact_logical_size_bytes(const pkg_artifact *artifact);
uint64_t pkg_artifact_allocated_size_bytes(const pkg_artifact *artifact);
#endif
