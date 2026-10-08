#define _GNU_SOURCE
#include "pkgintel/pkgintel.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>

static int make_dir(const char *path) {
    return mkdir(path, 0700) == 0 ? 0 : -1;
}

static int write_text(const char *path, const char *text) {
    FILE *file = fopen(path, "wb");
    if (file == NULL) return -1;
    if (fputs(text, file) < 0) {
        (void)fclose(file);
        return -1;
    }
    return fclose(file) == 0 ? 0 : -1;
}

static int make_dpkg_fixture(const char *root, const char *path) {
    char var[512], lib[512], dpkg[512], info[512], status[512], list[512];
    if (snprintf(var, sizeof(var), "%s/var", root) <= 0 ||
        snprintf(lib, sizeof(lib), "%s/var/lib", root) <= 0 ||
        snprintf(dpkg, sizeof(dpkg), "%s/var/lib/dpkg", root) <= 0 ||
        snprintf(info, sizeof(info), "%s/var/lib/dpkg/info", root) <= 0 ||
        snprintf(status, sizeof(status), "%s/var/lib/dpkg/status", root) <= 0 ||
        snprintf(list, sizeof(list), "%s/var/lib/dpkg/info/security-test.list", root) <= 0)
        return -1;
    if (make_dir(var) != 0 || make_dir(lib) != 0 || make_dir(dpkg) != 0 || make_dir(info) != 0)
        return -1;
    if (write_text(status,
                   "Package: security-test\n"
                   "Version: 1.0\n"
                   "Architecture: amd64\n"
                   "Status: install ok installed\n"
                   "Installed-Size: 1\n\n") != 0)
        return -1;
    return write_text(list, path);
}

static void remove_dpkg_fixture(const char *root) {
    char path[512];
    if (snprintf(path, sizeof(path), "%s/var/lib/dpkg/info/security-test.list", root) > 0) (void)unlink(path);
    if (snprintf(path, sizeof(path), "%s/var/lib/dpkg/status", root) > 0) (void)unlink(path);
    if (snprintf(path, sizeof(path), "%s/var/lib/dpkg/info", root) > 0) (void)rmdir(path);
    if (snprintf(path, sizeof(path), "%s/var/lib/dpkg", root) > 0) (void)rmdir(path);
    if (snprintf(path, sizeof(path), "%s/var/lib", root) > 0) (void)rmdir(path);
    if (snprintf(path, sizeof(path), "%s/var", root) > 0) (void)rmdir(path);
}

static int assert_unverifiable_scan(const char *root) {
    pkg_context *context = NULL;
    pkg_target *target = NULL;
    pkg_scan_result *result = NULL;
    pkg_scan_options options = PKG_SCAN_OPTIONS_INIT;
    const pkg_artifact *artifact = NULL;

    options.flags = PKG_SCAN_CORRELATE_FILES;
    if (pkg_context_create(NULL, &context) != PKG_OK ||
        pkg_target_create_rootfs(context, root, &target) != PKG_OK)
        goto fail;

    if (pkg_scan(context, target, &options, &result) != PKG_OK || result == NULL)
        goto fail;
    if (pkg_scan_result_package_count(result) != 1U ||
        pkg_scan_result_package_file_count(result, 0U) != 1U ||
        pkg_snapshot_artifact_count(result) != 1U)
        goto fail;
    if (pkg_snapshot_artifact_at(result, 0U, &artifact) != PKG_OK ||
        artifact == NULL ||
        pkg_artifact_get_state(artifact) != PKG_ARTIFACT_UNVERIFIABLE)
        goto fail;

    pkg_scan_result_destroy(result);
    pkg_target_destroy(target);
    pkg_context_destroy(context);
    return 0;

fail:
    if (result != NULL) pkg_scan_result_destroy(result);
    if (target != NULL) pkg_target_destroy(target);
    if (context != NULL) pkg_context_destroy(context);
    return -1;
}

static int run_bind_mount_test(void) {
    char root[] = "/tmp/pkgintel-security-root-XXXXXX";
    char outside[] = "/tmp/pkgintel-security-outside-XXXXXX";
    char source[512], mountpoint[512], payload[512], list_path[512];
    int mounted = 0;
    int status = 1;

    if (mkdtemp(root) == NULL || mkdtemp(outside) == NULL) return 1;
    if (snprintf(source, sizeof(source), "%s/source", outside) <= 0 ||
        snprintf(mountpoint, sizeof(mountpoint), "%s/mnt", root) <= 0 ||
        snprintf(payload, sizeof(payload), "%s/payload", source) <= 0 ||
        snprintf(list_path, sizeof(list_path), "/mnt/payload\n") <= 0)
        goto cleanup;
    if (make_dir(source) != 0 || make_dir(mountpoint) != 0 ||
        write_text(payload, "outside\n") != 0)
        goto cleanup;
    if (make_dpkg_fixture(root, list_path) != 0) goto cleanup;

    if (mount(source, mountpoint, NULL, MS_BIND, NULL) != 0) {
        if (errno == EPERM || errno == EACCES || errno == ENOSPC) {
            fprintf(stderr, "SKIP: bind mount unavailable: %s\n", strerror(errno));
            status = 77;
        } else {
            fprintf(stderr, "FAIL: bind mount setup: %s\n", strerror(errno));
        }
        goto cleanup;
    }
    mounted = 1;

    if (assert_unverifiable_scan(root) != 0) {
        fprintf(stderr, "FAIL: public scan crossed bind-mount boundary\n");
        goto cleanup;
    }

    status = 0;
cleanup:
    if (mounted) (void)umount2(mountpoint, MNT_DETACH);
    remove_dpkg_fixture(root);
    (void)unlink(payload);
    (void)rmdir(source);
    (void)rmdir(mountpoint);
    (void)rmdir(root);
    (void)rmdir(outside);
    return status;
}

static int run_magic_link_test(void) {
    char root[] = "/tmp/pkgintel-security-proc-XXXXXX";
    char proc_mount[512], list_path[512];
    int mounted = 0;
    int status = 1;

    if (mkdtemp(root) == NULL) return 1;
    if (snprintf(proc_mount, sizeof(proc_mount), "%s/proc", root) <= 0 ||
        snprintf(list_path, sizeof(list_path), "/proc/self/fd/0\n") <= 0 ||
        make_dir(proc_mount) != 0)
        goto cleanup;
    if (make_dpkg_fixture(root, list_path) != 0) goto cleanup;

    if (mount("proc", proc_mount, "proc", MS_NOSUID | MS_NODEV | MS_NOEXEC, NULL) != 0) {
        if (errno == EPERM || errno == EACCES) {
            fprintf(stderr, "SKIP: procfs mount unavailable: %s\n", strerror(errno));
            status = 77;
        } else {
            fprintf(stderr, "FAIL: procfs setup: %s\n", strerror(errno));
        }
        goto cleanup;
    }
    mounted = 1;

    if (assert_unverifiable_scan(root) != 0) {
        fprintf(stderr, "FAIL: public scan crossed procfs magic-link boundary\n");
        goto cleanup;
    }

    status = 0;
cleanup:
    if (mounted) (void)umount2(proc_mount, MNT_DETACH);
    remove_dpkg_fixture(root);
    (void)rmdir(proc_mount);
    (void)rmdir(root);
    return status;
}

int main(void) {
    int status;
    if (geteuid() != 0) {
        fprintf(stderr, "SKIP: security mount tests require root\n");
        return 77;
    }
    status = run_bind_mount_test();
    if (status != 0) return status;
    status = run_magic_link_test();
    if (status != 0) return status;
    puts("PASS: genuine mount-boundary and procfs magic-link containment tests");
    return 0;
}
