#define _GNU_SOURCE
#include "pkgintel/pkgintel.h"
#include "internal/pkg_target.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>

static int make_dir(const char *path) { return mkdir(path, 0700) == 0 ? 0 : -1; }

static int write_file(const char *path) {
    FILE *file = fopen(path, "wb");
    if (file == NULL) return -1;
    if (fputs("outside\n", file) < 0) { (void)fclose(file); return -1; }
    return fclose(file) == 0 ? 0 : -1;
}

static int run_bind_mount_test(void) {
    char root[] = "/tmp/pkgintel-security-root-XXXXXX";
    char outside[] = "/tmp/pkgintel-security-outside-XXXXXX";
    char source[512], mountpoint[512];
    pkg_context *context = NULL;
    pkg_target *target = NULL;
    struct stat st;
    int fd = -1, status = 1;

    if (mkdtemp(root) == NULL || mkdtemp(outside) == NULL) return 1;
    if (snprintf(source, sizeof(source), "%s/source", outside) <= 0 ||
        snprintf(mountpoint, sizeof(mountpoint), "%s/mnt", root) <= 0) goto cleanup;
    if (make_dir(source) != 0 || make_dir(mountpoint) != 0) goto cleanup;
    {
        char payload[512];
        if (snprintf(payload, sizeof(payload), "%s/payload", source) <= 0 ||
            write_file(payload) != 0) goto cleanup;
    }

    if (mount(source, mountpoint, NULL, MS_BIND, NULL) != 0) {
        if (errno == EPERM || errno == EACCES || errno == ENOSPC) {
            fprintf(stderr, "SKIP: bind mount unavailable: %s\n", strerror(errno));
            status = 77;
        } else {
            fprintf(stderr, "FAIL: bind mount setup: %s\n", strerror(errno));
        }
        goto cleanup;
    }

    if (pkg_context_create(NULL, &context) != PKG_OK ||
        pkg_target_create_rootfs(context, root, &target) != PKG_OK ||
        pkg_target_open_root(target) != PKG_OK) goto cleanup;

    errno = 0;
    fd = pkg_target_open_path(target, "/mnt/payload", O_RDONLY);
    if (fd >= 0) { (void)close(fd); fprintf(stderr, "FAIL: bind mount crossed\n"); goto cleanup; }
    if (errno != EXDEV) { fprintf(stderr, "FAIL: bind open errno=%d (%s), expected EXDEV\n", errno, strerror(errno)); goto cleanup; }

    errno = 0;
    if (pkg_target_lstat_path(target, "/mnt/payload", &st) == 0) {
        fprintf(stderr, "FAIL: bind mount crossed during lstat\n");
        goto cleanup;
    }
    if (errno != EXDEV) { fprintf(stderr, "FAIL: bind lstat errno=%d (%s), expected EXDEV\n", errno, strerror(errno)); goto cleanup; }

    status = 0;
cleanup:
    if (target != NULL) pkg_target_destroy(target);
    if (context != NULL) pkg_context_destroy(context);
    (void)umount2(mountpoint, MNT_DETACH);
    {
        char p[512];
        if (snprintf(p, sizeof(p), "%s/source/payload", outside) > 0) (void)unlink(p);
        if (snprintf(p, sizeof(p), "%s/source", outside) > 0) (void)rmdir(p);
        (void)rmdir(mountpoint);
        (void)rmdir(root);
        (void)rmdir(outside);
    }
    return status;
}

static int run_magic_link_test(void) {
    char root[] = "/tmp/pkgintel-security-proc-XXXXXX";
    char proc_mount[512];
    pkg_context *context = NULL;
    pkg_target *target = NULL;
    struct stat st;
    char link_target[256];
    int fd = -1, status = 1;

    if (mkdtemp(root) == NULL) return 1;
    if (snprintf(proc_mount, sizeof(proc_mount), "%s/proc", root) <= 0 ||
        make_dir(proc_mount) != 0) goto cleanup;

    if (mount("proc", proc_mount, "proc", MS_NOSUID | MS_NODEV | MS_NOEXEC, NULL) != 0) {
        if (errno == EPERM || errno == EACCES) {
            fprintf(stderr, "SKIP: procfs mount unavailable: %s\n", strerror(errno));
            status = 77;
        } else {
            fprintf(stderr, "FAIL: procfs setup: %s\n", strerror(errno));
        }
        goto cleanup;
    }

    if (lstat("/proc/self/fd/0", &st) != 0 || !S_ISLNK(st.st_mode) ||
        readlink("/proc/self/fd/0", link_target, sizeof(link_target)) <= 0) {
        fprintf(stderr, "FAIL: genuine procfs magic-link fixture unavailable\n");
        goto cleanup;
    }

    if (pkg_context_create(NULL, &context) != PKG_OK ||
        pkg_target_create_rootfs(context, root, &target) != PKG_OK ||
        pkg_target_open_root(target) != PKG_OK) goto cleanup;

    errno = 0;
    fd = pkg_target_open_path(target, "/proc/self/fd/0", O_RDONLY);
    if (fd >= 0) { (void)close(fd); fprintf(stderr, "FAIL: procfs magic link was followed\n"); goto cleanup; }
    if (errno != EXDEV && errno != ELOOP) { fprintf(stderr, "FAIL: magic-link open errno=%d (%s)\n", errno, strerror(errno)); goto cleanup; }

    errno = 0;
    if (pkg_target_lstat_path(target, "/proc/self/fd/0", &st) == 0) {
        fprintf(stderr, "FAIL: procfs magic link was observed\n");
        goto cleanup;
    }
    if (errno != EXDEV && errno != ELOOP) { fprintf(stderr, "FAIL: magic-link lstat errno=%d (%s)\n", errno, strerror(errno)); goto cleanup; }

    status = 0;
cleanup:
    if (target != NULL) pkg_target_destroy(target);
    if (context != NULL) pkg_context_destroy(context);
    (void)umount2(proc_mount, MNT_DETACH);
    (void)rmdir(proc_mount);
    (void)rmdir(root);
    return status;
}

int main(void) {
    int status;
    if (geteuid() != 0) { fprintf(stderr, "SKIP: security mount tests require root\n"); return 77; }
    status = run_bind_mount_test();
    if (status != 0) return status;
    status = run_magic_link_test();
    if (status != 0) return status;
    puts("PASS: genuine mount-boundary and procfs magic-link containment tests");
    return 0;
}
