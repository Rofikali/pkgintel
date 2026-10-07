#ifndef PKGINTEL_CAPABILITY_H
#define PKGINTEL_CAPABILITY_H
#include "export.h"
#include "snapshot.h"
typedef enum pkg_capability_kind { PKG_CAPABILITY_UNKNOWN=0, PKG_CAPABILITY_COMPILER=1, PKG_CAPABILITY_FORMATTER=2, PKG_CAPABILITY_LINKER=3, PKG_CAPABILITY_DEBUGGER=4, PKG_CAPABILITY_BUILD_TOOL=5 } pkg_capability_kind;
PKGINTEL_API pkg_string_view pkg_capability_name(const pkg_capability *capability);
PKGINTEL_API pkg_capability_kind pkg_capability_get_kind(const pkg_capability *capability);
PKGINTEL_API pkg_path pkg_capability_executable_path(const pkg_capability *capability);
#endif
