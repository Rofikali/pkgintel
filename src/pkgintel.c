#include "pkgintel/pkgintel.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct pkg_context {
    pkg_context_options options;
};

struct pkg_target {
    pkg_context *context;
    pkg_target_type type;
    char *root;
};

typedef struct pkg_package_record {
    char *name;
    char *version;
    char *architecture;
    uint64_t installed_size;
} pkg_package_record;

struct pkg_scan_result {
    char *target_root;
    pkg_package_record *packages;
    size_t package_count;
};

static char *pkg_strdup(const char *value) {
    size_t len;
    char *copy;
    if (value == NULL) return NULL;
    len = strlen(value);
    if (len == SIZE_MAX) return NULL;
    copy = malloc(len + 1U);
    if (copy == NULL) return NULL;
    memcpy(copy, value, len + 1U);
    return copy;
}

const char *pkg_status_string(pkg_status status) {
    switch (status) {
        case PKG_OK: return "ok";
        case PKG_ERR_INVALID_ARGUMENT: return "invalid argument";
        case PKG_ERR_NOT_FOUND: return "not found";
        case PKG_ERR_PERMISSION: return "permission denied";
        case PKG_ERR_IO: return "I/O error";
        case PKG_ERR_PARSE: return "parse error";
        case PKG_ERR_UNSUPPORTED: return "unsupported";
        case PKG_ERR_CORRUPT: return "corrupt data";
        case PKG_ERR_RESOURCE_LIMIT: return "resource limit";
        case PKG_ERR_STATE: return "invalid state";
        case PKG_ERR_INTERNAL: return "internal error";
        default: return "unknown status";
    }
}

pkg_status pkg_context_create(const pkg_context_options *options, pkg_context **out_context) {
    pkg_context *context;
    if (out_context == NULL) return PKG_ERR_INVALID_ARGUMENT;
    *out_context = NULL;
    context = calloc(1, sizeof(*context));
    if (context == NULL) return PKG_ERR_INTERNAL;
    if (options != NULL) context->options = *options;
    if (context->options.max_files == 0) context->options.max_files = 1000000;
    if (context->options.max_directories == 0) context->options.max_directories = 100000;
    if (context->options.max_depth == 0) context->options.max_depth = 64;
    if (context->options.max_bytes == 0) context->options.max_bytes = UINT64_C(4) * 1024U * 1024U * 1024U;
    if (context->options.max_elf_bytes == 0) context->options.max_elf_bytes = UINT64_C(256) * 1024U * 1024U;
    *out_context = context;
    return PKG_OK;
}

void pkg_context_destroy(pkg_context *context) { free(context); }

static pkg_status target_create(pkg_context *context, pkg_target_type type,
                                const char *root, pkg_target **out_target) {
    pkg_target *target;
    if (context == NULL || root == NULL || out_target == NULL || root[0] == '\0')
        return PKG_ERR_INVALID_ARGUMENT;
    *out_target = NULL;
    target = calloc(1, sizeof(*target));
    if (target == NULL) return PKG_ERR_INTERNAL;
    target->root = pkg_strdup(root);
    if (target->root == NULL) {
        free(target);
        return PKG_ERR_INTERNAL;
    }
    target->context = context;
    target->type = type;
    *out_target = target;
    return PKG_OK;
}

pkg_status pkg_target_create_local(pkg_context *context, pkg_target **out_target) {
    return target_create(context, PKG_TARGET_LOCAL, "/", out_target);
}

pkg_status pkg_target_create_rootfs(pkg_context *context, const char *root, pkg_target **out_target) {
    return target_create(context, PKG_TARGET_ROOTFS, root, out_target);
}

void pkg_target_destroy(pkg_target *target) {
    if (target == NULL) return;
    free(target->root);
    free(target);
}

pkg_status pkg_scan(pkg_context *context, pkg_target *target,
                    const pkg_scan_options *options, pkg_scan_result **out_result) {
    pkg_scan_result *result;
    (void)options;
    if (context == NULL || target == NULL || out_result == NULL) return PKG_ERR_INVALID_ARGUMENT;
    if (target->context != context) return PKG_ERR_STATE;
    *out_result = NULL;
    result = calloc(1, sizeof(*result));
    if (result == NULL) return PKG_ERR_INTERNAL;
    result->target_root = pkg_strdup(target->root);
    if (result->target_root == NULL) {
        free(result);
        return PKG_ERR_INTERNAL;
    }
    /* v0.1 vertical slice: result construction is intentionally minimal.
       Backend enumeration is implemented in the next milestone. */
    *out_result = result;
    return PKG_OK;
}

void pkg_scan_result_destroy(pkg_scan_result *result) {
    size_t i;
    if (result == NULL) return;
    for (i = 0; i < result->package_count; ++i) {
        free(result->packages[i].name);
        free(result->packages[i].version);
        free(result->packages[i].architecture);
    }
    free(result->packages);
    free(result->target_root);
    free(result);
}

size_t pkg_scan_result_package_count(const pkg_scan_result *result) {
    return result == NULL ? 0U : result->package_count;
}

const char *pkg_scan_result_target_root(const pkg_scan_result *result) {
    return result == NULL ? NULL : result->target_root;
}

const char *pkg_scan_result_package_name(const pkg_scan_result *result, size_t index) {
    return result != NULL && index < result->package_count ? result->packages[index].name : NULL;
}

const char *pkg_scan_result_package_version(const pkg_scan_result *result, size_t index) {
    return result != NULL && index < result->package_count ? result->packages[index].version : NULL;
}

const char *pkg_scan_result_package_architecture(const pkg_scan_result *result, size_t index) {
    return result != NULL && index < result->package_count ? result->packages[index].architecture : NULL;
}

uint64_t pkg_scan_result_package_installed_size(const pkg_scan_result *result, size_t index) {
    return result != NULL && index < result->package_count ? result->packages[index].installed_size : 0;
}
