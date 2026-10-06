#include "internal/pkg_internal.h"
#include <stdlib.h>

void pkg_snapshot_destroy(pkg_snapshot *result) {
    size_t i;
    if (result == NULL) return;
    for (i = 0; i < result->package_count; ++i) { free(result->packages[i].name); free(result->packages[i].version); free(result->packages[i].architecture); }
    free(result->packages);
    for (i = 0; i < result->artifact_count; ++i) free(result->artifacts[i].path);
    free(result->artifacts);
    for (i = 0; i < result->diagnostic_count; ++i) { free(result->diagnostics[i].code); free(result->diagnostics[i].message); }
    free(result->diagnostics);
    free(result->target_root);
    free(result);
}

void pkg_scan_result_destroy(pkg_scan_result *result) { pkg_snapshot_destroy(result); }

size_t pkg_scan_result_package_count(const pkg_scan_result *result) { return result == NULL ? 0U : result->package_count; }

const char *pkg_scan_result_target_root(const pkg_scan_result *result) { return result == NULL ? NULL : result->target_root; }

const char *pkg_scan_result_package_name(const pkg_scan_result *result, size_t index) { return result != NULL && index < result->package_count ? result->packages[index].name : NULL; }

const char *pkg_scan_result_package_version(const pkg_scan_result *result, size_t index) { return result != NULL && index < result->package_count ? result->packages[index].version : NULL; }

const char *pkg_scan_result_package_architecture(const pkg_scan_result *result, size_t index) { return result != NULL && index < result->package_count ? result->packages[index].architecture : NULL; }

uint64_t pkg_scan_result_package_installed_size(const pkg_scan_result *result, size_t index) { return result != NULL && index < result->package_count ? result->packages[index].installed_size : 0U; }

uint64_t pkg_scan_result_package_file_count(const pkg_scan_result *result, size_t index) { return result != NULL && index < result->package_count ? result->packages[index].file_count : 0U; }

uint64_t pkg_scan_result_package_missing_file_count(const pkg_scan_result *result, size_t index) { return result != NULL && index < result->package_count ? result->packages[index].missing_file_count : 0U; }

uint64_t pkg_scan_result_package_invalid_path_count(const pkg_scan_result *result, size_t index) { return result != NULL && index < result->package_count ? result->packages[index].invalid_path_count : 0U; }

uint64_t pkg_scan_result_diagnostic_count(const pkg_scan_result *result) { return result == NULL ? 0U : result->diagnostic_count; }

size_t pkg_snapshot_package_count(const pkg_snapshot *snapshot) { return snapshot == NULL ? 0U : snapshot->package_count; }

pkg_status pkg_snapshot_package_at(const pkg_snapshot *snapshot, size_t index, const pkg_package **out_package) { if (out_package == NULL) return PKG_ERR_INVALID_ARGUMENT; *out_package = NULL; if (snapshot == NULL || index >= snapshot->package_count) return PKG_ERR_NOT_FOUND; *out_package = (const pkg_package *)&snapshot->packages[index]; return PKG_OK; }

size_t pkg_snapshot_artifact_count(const pkg_snapshot *snapshot) { return snapshot == NULL ? 0U : snapshot->artifact_count; }

pkg_status pkg_snapshot_artifact_at(const pkg_snapshot *snapshot, size_t index, const pkg_artifact **out_artifact) { if (out_artifact == NULL) return PKG_ERR_INVALID_ARGUMENT; *out_artifact = NULL; if (snapshot == NULL || index >= snapshot->artifact_count) return PKG_ERR_NOT_FOUND; *out_artifact = (const pkg_artifact *)&snapshot->artifacts[index]; return PKG_OK; }

size_t pkg_snapshot_cache_count(const pkg_snapshot *snapshot) { (void)snapshot; return 0U; }

pkg_status pkg_snapshot_cache_at(const pkg_snapshot *snapshot, size_t index, const pkg_cache **out_cache) { (void)snapshot; (void)index; if (out_cache == NULL) return PKG_ERR_INVALID_ARGUMENT; *out_cache = NULL; return PKG_ERR_NOT_FOUND; }

size_t pkg_snapshot_capability_count(const pkg_snapshot *snapshot) { (void)snapshot; return 0U; }

pkg_status pkg_snapshot_capability_at(const pkg_snapshot *snapshot, size_t index, const pkg_capability **out_capability) { (void)snapshot; (void)index; if (out_capability == NULL) return PKG_ERR_INVALID_ARGUMENT; *out_capability = NULL; return PKG_ERR_NOT_FOUND; }

size_t pkg_snapshot_diagnostic_count(const pkg_snapshot *snapshot) { return snapshot == NULL ? 0U : snapshot->diagnostic_count; }

pkg_status pkg_snapshot_diagnostic_at(const pkg_snapshot *snapshot, size_t index, const pkg_diagnostic **out_diagnostic) { if (out_diagnostic == NULL) return PKG_ERR_INVALID_ARGUMENT; *out_diagnostic = NULL; if (snapshot == NULL || index >= snapshot->diagnostic_count) return PKG_ERR_NOT_FOUND; *out_diagnostic = (const pkg_diagnostic *)&snapshot->diagnostics[index]; return PKG_OK; }
