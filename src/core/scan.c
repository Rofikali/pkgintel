#include "internal/pkg_model.h"
#include "internal/pkg_backend.h"
#include "internal/pkg_support.h"
#include "internal/pkg_target.h"
#include <stdlib.h>
#include <string.h>

pkg_status pkg_scan(pkg_context *context, pkg_target *target, const pkg_scan_options *options, pkg_snapshot **out_result) {
    pkg_snapshot *result;
    pkg_status status;
    pkg_scan_options defaults = { .struct_size = (uint32_t)sizeof(pkg_scan_options), .flags = PKG_SCAN_CORRELATE_FILES, .max_packages = 1000000U, .max_package_files = 1000000U };
    if (context == NULL || target == NULL || out_result == NULL) return PKG_ERR_INVALID_ARGUMENT;
    if (target->context != context) return PKG_ERR_STATE;
    if (target->root == NULL || target->root[0] == '\0') return PKG_ERR_STATE;
    *out_result = NULL;
    result = calloc(1, sizeof(*result));
    if (result == NULL) return PKG_ERR_INTERNAL;
    result->target_root = pkg_strdup_internal(target->root);
    if (result->target_root == NULL) { free(result); return PKG_ERR_INTERNAL; }
    status = pkg_target_open_root(target);
    if (status != PKG_OK) { free(result->target_root); free(result); return status; }
    {
        pkg_scan_options normalized = defaults;
        size_t supplied;
        if (options != NULL) {
            if (options->struct_size < sizeof(uint32_t) * 2U) { pkg_snapshot_destroy(result); return PKG_ERR_INVALID_ARGUMENT; }
            supplied = options->struct_size;
            if (supplied > sizeof(normalized)) supplied = sizeof(normalized);
            memcpy(&normalized, options, supplied);
            if ((normalized.flags & ~(PKG_SCAN_INCLUDE_ELF | PKG_SCAN_INCLUDE_CACHES | PKG_SCAN_INCLUDE_CAPABILITIES | PKG_SCAN_CORRELATE_FILES)) != 0U) { pkg_snapshot_destroy(result); return PKG_ERR_INVALID_ARGUMENT; }
            if (normalized.max_files != 0U || normalized.max_directories != 0U ||
                normalized.max_file_bytes != 0U || normalized.max_total_bytes != 0U ||
                normalized.max_duration_ms != 0U) {
                pkg_snapshot_destroy(result);
                return PKG_ERR_UNSUPPORTED;
            }
        }
        status = pkg_dpkg_scan(context, target, &normalized, result);
    }
    if (status != PKG_OK && status != PKG_ERR_RESOURCE_LIMIT) { pkg_snapshot_destroy(result); return status; }
    *out_result = result;
    return status;
}
