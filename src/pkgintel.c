#include "core/pkg_internal.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/openat2.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *pkg_strdup_internal(const char *value) {
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
    if (context->options.max_files == 0U) context->options.max_files = UINT64_C(1000000);
    if (context->options.max_directories == 0U) context->options.max_directories = UINT64_C(100000);
    if (context->options.max_depth == 0U) context->options.max_depth = UINT64_C(64);
    if (context->options.max_bytes == 0U) context->options.max_bytes = UINT64_C(4) * UINT64_C(1024) * UINT64_C(1024) * UINT64_C(1024);
    if (context->options.max_elf_bytes == 0U) context->options.max_elf_bytes = UINT64_C(256) * UINT64_C(1024) * UINT64_C(1024);
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
    target->root = pkg_strdup_internal(root);
    if (target->root == NULL) {
        free(target);
        return PKG_ERR_INTERNAL;
    }
    target->context = context;
    target->type = type;
    target->root_fd = -1;
    *out_target = target;
    return PKG_OK;
}

int pkg_target_open_path(const pkg_target *target, const char *path, int flags) {
    const char *relative;
    struct open_how how = {0};

    if (target == NULL || target->root_fd < 0 || path == NULL || path[0] != '/') {
        errno = EINVAL;
        return -1;
    }
    relative = path + 1U;
    if (*relative == '\0') {
        errno = EINVAL;
        return -1;
    }

    how.flags = (uint64_t)(flags | O_CLOEXEC);
    how.resolve = RESOLVE_BENEATH | RESOLVE_NO_MAGICLINKS;
    return (int)syscall(SYS_openat2, target->root_fd, relative, &how, sizeof(how));
}

int pkg_target_lstat_path(const pkg_target *target, const char *path, struct stat *st) {
    const char *relative;
    int fd;

    if (target == NULL || target->root_fd < 0 || path == NULL || path[0] != '/' || st == NULL) {
        errno = EINVAL;
        return -1;
    }
    relative = path + 1U;
    if (*relative == '\0') {
        errno = EINVAL;
        return -1;
    }

    fd = pkg_target_open_path(target, path, O_PATH | O_NOFOLLOW);
    if (fd < 0) return -1;
    if (fstat(fd, st) != 0) {
        int saved_errno = errno;
        (void)close(fd);
        errno = saved_errno;
        return -1;
    }
    (void)close(fd);
    return 0;
}

pkg_status pkg_target_open_root(pkg_target *target) {
    if (target == NULL || target->root == NULL) return PKG_ERR_INVALID_ARGUMENT;
    if (target->root_fd >= 0) return PKG_OK;
    target->root_fd = open(target->root, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (target->root_fd < 0) {
        if (errno == EACCES || errno == EPERM) return PKG_ERR_PERMISSION;
        if (errno == ENOENT) return PKG_ERR_NOT_FOUND;
        return PKG_ERR_IO;
    }
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
    if (target->root_fd >= 0) (void)close(target->root_fd);
    free(target->root);
    free(target);
}

pkg_status pkg_scan(pkg_context *context, pkg_target *target,
                    const pkg_scan_options *options, pkg_scan_result **out_result) {
    pkg_scan_result *result;
    pkg_status status;
    pkg_scan_options defaults = {
        .mode = PKG_SCAN_NORMAL,
        .max_packages = context != NULL ? context->options.max_files : 0U,
        .max_package_files = context != NULL ? context->options.max_files : 0U
    };

    if (context == NULL || target == NULL || out_result == NULL) return PKG_ERR_INVALID_ARGUMENT;
    if (target->context != context) return PKG_ERR_STATE;
    if (target->root == NULL || target->root[0] == '\0') return PKG_ERR_STATE;
    *out_result = NULL;

    result = calloc(1, sizeof(*result));
    if (result == NULL) return PKG_ERR_INTERNAL;
    result->target_root = pkg_strdup_internal(target->root);
    if (result->target_root == NULL) {
        free(result);
        return PKG_ERR_INTERNAL;
    }

    status = pkg_target_open_root(target);
    if (status != PKG_OK) {
        free(result->target_root);
        free(result);
        return status;
    }

    if (options == NULL) options = &defaults;
    status = pkg_dpkg_scan(context, target, options, result);
    if (status != PKG_OK && status != PKG_ERR_RESOURCE_LIMIT) {
        pkg_scan_result_destroy(result);
        return status;
    }

    *out_result = result;
    return status;
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
    return result != NULL && index < result->package_count ? result->packages[index].installed_size : 0U;
}

uint64_t pkg_scan_result_package_file_count(const pkg_scan_result *result, size_t index) {
    return result != NULL && index < result->package_count ? result->packages[index].file_count : 0U;
}

uint64_t pkg_scan_result_package_missing_file_count(const pkg_scan_result *result, size_t index) {
    return result != NULL && index < result->package_count ? result->packages[index].missing_file_count : 0U;
}

uint64_t pkg_scan_result_package_invalid_path_count(const pkg_scan_result *result, size_t index) {
    return result != NULL && index < result->package_count ? result->packages[index].invalid_path_count : 0U;
}

uint64_t pkg_scan_result_diagnostic_count(const pkg_scan_result *result) {
    return result == NULL ? 0U : result->diagnostic_count;
}


pkg_status pkg_target_local_create(pkg_context *context, pkg_target **out_target) {
    return pkg_target_create_local(context, out_target);
}

pkg_status pkg_target_rootfs_create(pkg_context *context, pkg_path root, pkg_target **out_target) {
    char *text;
    pkg_status status;
    if (root.data == NULL || root.size == 0U || root.size > SIZE_MAX - 1U) return PKG_ERR_INVALID_ARGUMENT;
    if (memchr(root.data, '\\0', root.size) != NULL) return PKG_ERR_INVALID_ARGUMENT;
    text = malloc(root.size + 1U);
    if (text == NULL) return PKG_ERR_INTERNAL;
    memcpy(text, root.data, root.size);
    text[root.size] = '\\0';
    status = pkg_target_create_rootfs(context, text, out_target);
    free(text);
    return status;
}

void pkg_snapshot_destroy(pkg_snapshot *snapshot) {
    pkg_scan_result_destroy(snapshot);
}

size_t pkg_snapshot_package_count(const pkg_snapshot *snapshot) {
    return pkg_scan_result_package_count(snapshot);
}

pkg_status pkg_snapshot_package_at(const pkg_snapshot *snapshot, size_t index, const pkg_package **out_package) {
    if (out_package == NULL) return PKG_ERR_INVALID_ARGUMENT;
    *out_package = NULL;
    if (snapshot == NULL || index >= snapshot->package_count) return PKG_ERR_NOT_FOUND;
    *out_package = (const pkg_package *)&snapshot->packages[index];
    return PKG_OK;
}

size_t pkg_snapshot_artifact_count(const pkg_snapshot *snapshot) { (void)snapshot; return 0U; }
pkg_status pkg_snapshot_artifact_at(const pkg_snapshot *snapshot, size_t index, const pkg_artifact **out_artifact) {
    (void)snapshot; (void)index; if (out_artifact == NULL) return PKG_ERR_INVALID_ARGUMENT; *out_artifact = NULL; return PKG_ERR_NOT_FOUND;
}
size_t pkg_snapshot_cache_count(const pkg_snapshot *snapshot) { (void)snapshot; return 0U; }
pkg_status pkg_snapshot_cache_at(const pkg_snapshot *snapshot, size_t index, const pkg_cache **out_cache) {
    (void)snapshot; (void)index; if (out_cache == NULL) return PKG_ERR_INVALID_ARGUMENT; *out_cache = NULL; return PKG_ERR_NOT_FOUND;
}
size_t pkg_snapshot_capability_count(const pkg_snapshot *snapshot) { (void)snapshot; return 0U; }
pkg_status pkg_snapshot_capability_at(const pkg_snapshot *snapshot, size_t index, const pkg_capability **out_capability) {
    (void)snapshot; (void)index; if (out_capability == NULL) return PKG_ERR_INVALID_ARGUMENT; *out_capability = NULL; return PKG_ERR_NOT_FOUND;
}
size_t pkg_snapshot_diagnostic_count(const pkg_snapshot *snapshot) { return snapshot == NULL ? 0U : (size_t)snapshot->diagnostic_count; }
pkg_status pkg_snapshot_diagnostic_at(const pkg_snapshot *snapshot, size_t index, const pkg_diagnostic **out_diagnostic) {
    (void)snapshot; (void)index; if (out_diagnostic == NULL) return PKG_ERR_INVALID_ARGUMENT; *out_diagnostic = NULL; return PKG_ERR_NOT_FOUND;
}

pkg_string_view pkg_package_name(const pkg_package *package) {
    const pkg_package_record *p = (const pkg_package_record *)package;
    return (pkg_string_view){ p != NULL ? p->name : NULL, p != NULL && p->name != NULL ? strlen(p->name) : 0U };
}
pkg_string_view pkg_package_version(const pkg_package *package) {
    const pkg_package_record *p = (const pkg_package_record *)package;
    return (pkg_string_view){ p != NULL ? p->version : NULL, p != NULL && p->version != NULL ? strlen(p->version) : 0U };
}
pkg_string_view pkg_package_architecture(const pkg_package *package) {
    const pkg_package_record *p = (const pkg_package_record *)package;
    return (pkg_string_view){ p != NULL ? p->architecture : NULL, p != NULL && p->architecture != NULL ? strlen(p->architecture) : 0U };
}
pkg_string_view pkg_package_source(const pkg_package *package) { (void)package; return (pkg_string_view){NULL,0U}; }
pkg_installation_state pkg_package_get_state(const pkg_package *package) { return package != NULL ? PKG_INSTALLATION_INSTALLED : PKG_INSTALLATION_UNKNOWN; }
pkg_consistency_state pkg_package_get_consistency(const pkg_package *package) {
    const pkg_package_record *p = (const pkg_package_record *)package;
    if (p == NULL) return PKG_CONSISTENCY_UNKNOWN;
    if (p->invalid_path_count != 0U || p->missing_file_count != 0U) return PKG_CONSISTENCY_INCONSISTENT;
    return PKG_CONSISTENCY_CONSISTENT;
}
uint64_t pkg_package_declared_size_bytes(const pkg_package *package) {
    return pkg_package_installed_size_bytes(package);
}
uint64_t pkg_package_installed_size_bytes(const pkg_package *package) {
    const pkg_package_record *p = (const pkg_package_record *)package;
    return p != NULL ? p->installed_size : 0U;
}
size_t pkg_package_artifact_count(const pkg_package *package) {
    const pkg_package_record *p = (const pkg_package_record *)package;
    return p != NULL ? (size_t)p->file_count : 0U;
}
pkg_status pkg_package_artifact_at(const pkg_package *package, size_t index, const pkg_artifact **out_artifact) {
    (void)package; (void)index; if (out_artifact == NULL) return PKG_ERR_INVALID_ARGUMENT; *out_artifact = NULL; return PKG_ERR_NOT_FOUND;
}
