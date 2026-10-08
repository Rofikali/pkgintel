#define _GNU_SOURCE

#include <pkgintel/pkgintel.h>

#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static pkg_context *g_context;
static pkg_target *g_target;
static char g_root[] = "/tmp/pkgintel-fuzz-XXXXXX";

int LLVMFuzzerInitialize(int *argc, char ***argv);
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

static void die(const char *message) { perror(message); abort(); }

static void write_bytes(const char *path, const uint8_t *data, size_t size) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
    size_t offset = 0U;
    if (fd < 0) die("open");
    while (offset < size) {
        ssize_t written = write(fd, data + offset, size - offset);
        if (written <= 0) { (void)close(fd); die("write"); }
        offset += (size_t)written;
    }
    if (close(fd) != 0) die("close");
}

static void append_bytes(const char *path, const uint8_t *data, size_t size) {
    int fd = open(path, O_WRONLY | O_APPEND | O_CLOEXEC);
    size_t offset = 0U;
    if (fd < 0) die("open");
    while (offset < size) {
        ssize_t written = write(fd, data + offset, size - offset);
        if (written <= 0) { (void)close(fd); die("write"); }
        offset += (size_t)written;
    }
    if (close(fd) != 0) die("close");
}

static void make_dir(const char *path) {
    if (mkdir(path, 0700) != 0) die("mkdir");
}

static void init_fixture(void) {
    char path[PATH_MAX];
    if (mkdtemp(g_root) == NULL) die("mkdtemp");
    if (snprintf(path, sizeof(path), "%s/var", g_root) >= (int)sizeof(path)) abort();
    make_dir(path);
    if (snprintf(path, sizeof(path), "%s/var/lib", g_root) >= (int)sizeof(path)) abort();
    make_dir(path);
    if (snprintf(path, sizeof(path), "%s/var/lib/dpkg", g_root) >= (int)sizeof(path)) abort();
    make_dir(path);
    if (snprintf(path, sizeof(path), "%s/var/lib/dpkg/info", g_root) >= (int)sizeof(path)) abort();
    make_dir(path);
    if (pkg_context_create(NULL, &g_context) != PKG_OK) abort();
    if (pkg_target_create_rootfs(g_context, g_root, &g_target) != PKG_OK) abort();
}

static void destroy_fixture(void) {
    if (g_target != NULL) pkg_target_destroy(g_target);
    if (g_context != NULL) pkg_context_destroy(g_context);
}

int LLVMFuzzerInitialize(int *argc, char ***argv) {
    (void)argc;
    (void)argv;
    init_fixture();
    atexit(destroy_fixture);
    return 0;
}

static void run_scan(void) {
    pkg_scan_options options = PKG_SCAN_OPTIONS_INIT;
    pkg_snapshot *snapshot = NULL;
    options.flags = PKG_SCAN_CORRELATE_FILES;
    options.max_packages = 8U;
    options.max_package_files = 256U;
    pkg_status status = pkg_scan(g_context, g_target, &options, &snapshot);
    if (snapshot != NULL) pkg_snapshot_destroy(snapshot);
    if (status == PKG_ERR_INTERNAL) abort();
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    char path[PATH_MAX];
    static const char trailer[] = "\n\nPackage: fuzzpkg\nVersion: 1\nArchitecture: amd64\nStatus: install ok installed\nInstalled-Size: 1\n";
    if (size > 131072U) return 0;
    if (snprintf(path, sizeof(path), "%s/var/lib/dpkg/status", g_root) >= (int)sizeof(path)) abort();
    write_bytes(path, data, size);
    append_bytes(path, (const uint8_t *)trailer, sizeof(trailer) - 1U);
    run_scan();
    return 0;
}
