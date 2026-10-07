#ifndef PKGINTEL_SNAPSHOT_H
#define PKGINTEL_SNAPSHOT_H
#include "export.h"
#include <stddef.h>
#include "status.h"
typedef struct pkg_snapshot pkg_snapshot;
typedef pkg_snapshot pkg_scan_result;
typedef struct pkg_package pkg_package;
typedef struct pkg_artifact pkg_artifact;
typedef struct pkg_cache pkg_cache;
typedef struct pkg_capability pkg_capability;
typedef struct pkg_diagnostic pkg_diagnostic;
PKGINTEL_API void pkg_snapshot_destroy(pkg_snapshot *snapshot);
PKGINTEL_API size_t pkg_snapshot_package_count(const pkg_snapshot *snapshot);
PKGINTEL_API pkg_status pkg_snapshot_package_at(const pkg_snapshot *snapshot, size_t index, const pkg_package **out_package);
PKGINTEL_API size_t pkg_snapshot_artifact_count(const pkg_snapshot *snapshot);
PKGINTEL_API pkg_status pkg_snapshot_artifact_at(const pkg_snapshot *snapshot, size_t index, const pkg_artifact **out_artifact);
PKGINTEL_API size_t pkg_snapshot_cache_count(const pkg_snapshot *snapshot);
PKGINTEL_API pkg_status pkg_snapshot_cache_at(const pkg_snapshot *snapshot, size_t index, const pkg_cache **out_cache);
PKGINTEL_API size_t pkg_snapshot_capability_count(const pkg_snapshot *snapshot);
PKGINTEL_API pkg_status pkg_snapshot_capability_at(const pkg_snapshot *snapshot, size_t index, const pkg_capability **out_capability);
PKGINTEL_API size_t pkg_snapshot_diagnostic_count(const pkg_snapshot *snapshot);
PKGINTEL_API pkg_status pkg_snapshot_diagnostic_at(const pkg_snapshot *snapshot, size_t index, const pkg_diagnostic **out_diagnostic);
/* 0.x compatibility accessors. */
PKGINTEL_API void pkg_scan_result_destroy(pkg_scan_result *result);
PKGINTEL_API size_t pkg_scan_result_package_count(const pkg_scan_result *result);
PKGINTEL_API const char *pkg_scan_result_target_root(const pkg_scan_result *result);
PKGINTEL_API const char *pkg_scan_result_package_name(const pkg_scan_result *result, size_t index);
PKGINTEL_API const char *pkg_scan_result_package_version(const pkg_scan_result *result, size_t index);
PKGINTEL_API const char *pkg_scan_result_package_architecture(const pkg_scan_result *result, size_t index);
PKGINTEL_API uint64_t pkg_scan_result_package_installed_size(const pkg_scan_result *result, size_t index);
PKGINTEL_API uint64_t pkg_scan_result_package_file_count(const pkg_scan_result *result, size_t index);
PKGINTEL_API uint64_t pkg_scan_result_package_missing_file_count(const pkg_scan_result *result, size_t index);
PKGINTEL_API uint64_t pkg_scan_result_package_invalid_path_count(const pkg_scan_result *result, size_t index);
PKGINTEL_API uint64_t pkg_scan_result_diagnostic_count(const pkg_scan_result *result);
#endif
