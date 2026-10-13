#include "internal/pkg_model.h"
#include "internal/pkg_support.h"
#include "internal/pkg_target.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/openat2.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#ifdef PKGINTEL_BENCHMARK_ALLOC_STATS
#include "alloc_probe_macros.h"
#endif

static pkg_status target_open_error_status(int error_number) {
    if (error_number == EACCES || error_number == EPERM) return PKG_ERR_PERMISSION;
    if (error_number == ENOENT) return PKG_ERR_NOT_FOUND;
    return PKG_ERR_IO;
}

static pkg_status target_open_root_fd(const char *root, int *out_fd) {
    size_t length;
    char *open_path;
    int fd;
    int saved_errno;

    if (root == NULL || out_fd == NULL) return PKG_ERR_INVALID_ARGUMENT;
    *out_fd = -1;
    length = strlen(root);
    open_path = malloc(length + 1U);
    if (open_path == NULL) return PKG_ERR_INTERNAL;
    memcpy(open_path, root, length + 1U);

    /*
     * O_NOFOLLOW protects only the final pathname component. A trailing slash
     * or a terminal "/." can make a symlink appear as an earlier component,
     * bypassing the intended symlink-root rejection. Remove only these
     * semantically redundant suffixes before opening; retain the caller's
     * original spelling in target->root for diagnostics.
     */
    for (;;) {
        while (length > 1U && open_path[length - 1U] == '/') {
            open_path[--length] = '\0';
        }
        if (length >= 2U && open_path[length - 1U] == '.' &&
            open_path[length - 2U] == '/') {
            if (length == 2U && open_path[0] == '/') {
                open_path[1] = '\0';
                length = 1U;
                break;
            }
            length -= 2U;
            open_path[length] = '\0';
            continue;
        }
        break;
    }

    fd = open(open_path, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    saved_errno = errno;
    free(open_path);
    if (fd < 0) return target_open_error_status(saved_errno);
    *out_fd = fd;
    return PKG_OK;
}

static pkg_status target_create(pkg_context *context, pkg_target_type type, const char *root, pkg_target **out_target) {
    pkg_target *target;
    pkg_status status;
    if (out_target == NULL) return PKG_ERR_INVALID_ARGUMENT;
    *out_target = NULL;
    if (context == NULL || root == NULL || root[0] == '\0') return PKG_ERR_INVALID_ARGUMENT;
    target = calloc(1, sizeof(*target));
    if (target == NULL) return PKG_ERR_INTERNAL;
    target->root_fd = -1;
    target->root = pkg_strdup_internal(root);
    if (target->root == NULL) { free(target); return PKG_ERR_INTERNAL; }
    /*
     * Pin the caller-selected directory now, not at first scan. Relative paths
     * must not be reinterpreted if the process working directory later changes,
     * and renames/replacements of the pathname must not retarget this object.
     */
    status = target_open_root_fd(target->root, &target->root_fd);
    if (status != PKG_OK) {
        free(target->root);
        free(target);
        return status;
    }
    target->context = context;
    target->type = type;
    *out_target = target;
    return PKG_OK;
}

int pkg_target_open_path(const pkg_target *target, const char *path, int flags) {
    const char *relative;
    struct open_how how = {0};
    if (target == NULL || target->root_fd < 0 || path == NULL || path[0] != '/') { errno = EINVAL; return -1; }
    relative = path + 1U;
    if (*relative == '\0') { errno = EINVAL; return -1; }
    how.flags = (unsigned long long)(flags | O_CLOEXEC);
    how.resolve = RESOLVE_IN_ROOT | RESOLVE_NO_MAGICLINKS | RESOLVE_NO_XDEV;
    return (int)syscall(SYS_openat2, target->root_fd, relative, &how, sizeof(how));
}

int pkg_target_lstat_path(const pkg_target *target, const char *path, struct stat *st) {
    const char *relative;
    struct open_how how = {0};
    int fd;
    if (target == NULL || target->root_fd < 0 || path == NULL || path[0] != '/' || st == NULL) { errno = EINVAL; return -1; }
    relative = path + 1U;
    if (*relative == '\0') { errno = EINVAL; return -1; }
    how.flags = (unsigned long long)(O_PATH | O_NOFOLLOW | O_CLOEXEC);
    how.resolve = RESOLVE_IN_ROOT | RESOLVE_NO_MAGICLINKS | RESOLVE_NO_XDEV;
    fd = (int)syscall(SYS_openat2, target->root_fd, relative, &how, sizeof(how));
    if (fd < 0) return -1;
    if (fstat(fd, st) != 0) { int saved_errno = errno; (void)close(fd); errno = saved_errno; return -1; }
    (void)close(fd);
    return 0;
}

pkg_status pkg_target_open_root(pkg_target *target) {
    if (target == NULL || target->root == NULL) return PKG_ERR_INVALID_ARGUMENT;
    if (target->root_fd >= 0) return PKG_OK;
    return target_open_root_fd(target->root, &target->root_fd);
}

pkg_status pkg_target_create_local(pkg_context *context, pkg_target **out_target) { return target_create(context, PKG_TARGET_LOCAL, "/", out_target); }

pkg_status pkg_target_create_rootfs(pkg_context *context, const char *root, pkg_target **out_target) { return target_create(context, PKG_TARGET_ROOTFS, root, out_target); }

void pkg_target_destroy(pkg_target *target) { if (target == NULL) return; if (target->root_fd >= 0) (void)close(target->root_fd); free(target->root); free(target); }

pkg_status pkg_target_local_create(pkg_context *context, pkg_target **out_target) { return pkg_target_create_local(context, out_target); }

pkg_status pkg_target_rootfs_create(pkg_context *context, pkg_path root, pkg_target **out_target) {
    char *text; pkg_status status;
    if (root.data == NULL || root.size == 0U || root.size > SIZE_MAX - 1U || memchr(root.data, '\0', root.size) != NULL) return PKG_ERR_INVALID_ARGUMENT;
    text = malloc(root.size + 1U); if (text == NULL) return PKG_ERR_INTERNAL;
    memcpy(text, root.data, root.size); text[root.size] = '\0';
    status = pkg_target_create_rootfs(context, text, out_target); free(text); return status;
}
