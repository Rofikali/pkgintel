#ifndef PKGINTEL_PKGINTEL_H
#define PKGINTEL_PKGINTEL_H
#include "version.h"
#if defined(_WIN32) && defined(PKGINTEL_BUILDING_LIBRARY)
#define PKGINTEL_API __declspec(dllexport)
#elif defined(_WIN32)
#define PKGINTEL_API __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
#define PKGINTEL_API __attribute__((visibility("default")))
#else
#define PKGINTEL_API
#endif
#include "status.h"
#include "context.h"
#include "target.h"
#include "scan.h"
#include "snapshot.h"
#include "package.h"
#include "artifact.h"
#include "cache.h"
#include "capability.h"
#include "diagnostic.h"
#ifdef __cplusplus
extern "C" {
#endif
PKGINTEL_API const char *pkg_status_string(pkg_status status);
#ifdef __cplusplus
}
#endif
#endif
