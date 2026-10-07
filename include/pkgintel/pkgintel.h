#ifndef PKGINTEL_PKGINTEL_H
#define PKGINTEL_PKGINTEL_H
#include "version.h"
#include "export.h"
#include "status.h"
#include "context.h"
#include "target.h"
#include "scan.h"
#include "snapshot.h"
#include "package.h"
#include "artifact.h"
#include "diagnostic.h"
#ifdef __cplusplus
extern "C" {
#endif
PKGINTEL_API const char *pkg_status_string(pkg_status status);
#ifdef __cplusplus
}
#endif
#endif
