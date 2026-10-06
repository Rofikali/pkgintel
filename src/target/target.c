#include "internal/pkg_internal.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/openat2.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static pkg_status target_create(pkg_context *context, pkg_target_type type, const char *root, pkg_target **out_target) {
    pkg_target *target;
    if (context == NULL || root == NULL || out_target == NULL || root[0] == '\0') return PKG_ERR_INVALID_ARGUMENT;
    *out_target = NULL;
    target = calloc(1, sizeof(*target));
    if (target == NULL) return PKG_ERR_INTERNAL;
    target->root = pkg_strdup_internal(root);
    if (target->root == NULL) { free(target); return PKG_ERR_INTERNAL; }
    target->context = context;
    target->type = type;
    target->root_fd = -1;
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
    how.resolve = RESOLVE_IN_ROOT | RESOLVE_NO_MAGICLINKS;
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
    how.resolve = RESOLVE_IN_ROOT | RESOLVE_NO_MAGICLINKS;
    fd = (int)syscall(SYS_openat2, target->root_fd, relative, &how, sizeof(how));
    if (fd < 0) return -1;
    if (fstat(fd, st) != 0) { int saved_errno = errno; (void)close(fd); errno = saved_errno; return -1; }
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
