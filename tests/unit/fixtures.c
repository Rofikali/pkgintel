#define _GNU_SOURCE
#include "fixtures.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void make_fixture(char *root, size_t root_size) {
    char path[512]; int written; FILE *file;
    written = snprintf(root, root_size, "/tmp/pkgintel-test-XXXXXX"); assert(written > 0 && (size_t)written < root_size); assert(mkdtemp(root) != NULL);
    written = snprintf(path, sizeof(path), "%s/var", root); assert(written > 0 && (size_t)written < sizeof(path)); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/var/lib", root); assert(written > 0 && (size_t)written < sizeof(path)); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/var/lib/dpkg", root); assert(written > 0 && (size_t)written < sizeof(path)); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/var/lib/dpkg/status", root); assert(written > 0 && (size_t)written < sizeof(path));
    file = fopen(path, "wb"); assert(file != NULL);
    assert(fputs("Package: fixture-pkg\nVersion: 1.2.3\nArchitecture: amd64\nStatus: install ok installed\nInstalled-Size: 10\n\nPackage: removed-pkg\nVersion: 9.9\nArchitecture: amd64\nStatus: deinstall ok config-files\nInstalled-Size: 999\n", file) >= 0); assert(fclose(file) == 0);
    written = snprintf(path, sizeof(path), "%s/var/lib/dpkg/info", root); assert(written > 0 && (size_t)written < sizeof(path)); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/var/lib/dpkg/info/fixture-pkg.list", root); file = fopen(path, "wb"); assert(file != NULL);
    assert(fputs("/usr/bin/present\n/usr/bin/missing\n/usr/bin/link\n/usr/bin/broken\n/usr/bin/adir\n/usr/bin/fifo\n/usr/bin/present\n/restricted/secret\n../escape\n\n", file) >= 0); assert(fclose(file) == 0);
    written = snprintf(path, sizeof(path), "%s/usr", root); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/usr/bin", root); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/usr/bin/present", root); file = fopen(path, "wb"); assert(file != NULL); assert(fputs("x", file) == 1); assert(fclose(file) == 0);
    written = snprintf(path, sizeof(path), "%s/usr/bin/link", root); assert(symlink("/usr/bin/present", path) == 0);
    written = snprintf(path, sizeof(path), "%s/usr/bin/broken", root); assert(symlink("/usr/bin/nope", path) == 0);
    written = snprintf(path, sizeof(path), "%s/usr/bin/adir", root); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/usr/bin/fifo", root); assert(mkfifo(path, 0600) == 0);
    written = snprintf(path, sizeof(path), "%s/restricted", root); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/restricted/secret", root); file = fopen(path, "wb"); assert(file != NULL); assert(fwrite("secret", 1U, 6U, file) == 6U); assert(fclose(file) == 0);
    written = snprintf(path, sizeof(path), "%s/restricted", root); assert(chmod(path, 0000) == 0);
}

void remove_fixture(const char *root) {
    char path[512];
    assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg/status", root) > 0); assert(unlink(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg/info/fixture-pkg.list", root) > 0); assert(unlink(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg/info", root) > 0); assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/usr/bin/fifo", root) > 0); assert(unlink(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/usr/bin/adir", root) > 0); assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/usr/bin/broken", root) > 0); assert(unlink(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/usr/bin/link", root) > 0); assert(unlink(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/usr/bin/present", root) > 0); assert(unlink(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/usr/bin", root) > 0); assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/usr", root) > 0); assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/restricted", root) > 0); assert(chmod(path, 0700) == 0); assert(snprintf(path, sizeof(path), "%s/restricted/secret", root) > 0); assert(unlink(path) == 0); assert(snprintf(path, sizeof(path), "%s/restricted", root) > 0); assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg", root) > 0); assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var/lib", root) > 0); assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var", root) > 0); assert(rmdir(path) == 0); assert(rmdir(root) == 0);
}


void make_multi_package_fixture(char *root, size_t root_size) {
    char path[512];
    FILE *file;
    int written;
    written = snprintf(root, root_size, "/tmp/pkgintel-multi-test-XXXXXX");
    assert(written > 0 && (size_t)written < root_size);
    assert(mkdtemp(root) != NULL);

    written = snprintf(path, sizeof(path), "%s/var", root);
    assert(written > 0 && (size_t)written < sizeof(path)); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/var/lib", root);
    assert(written > 0 && (size_t)written < sizeof(path)); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/var/lib/dpkg", root);
    assert(written > 0 && (size_t)written < sizeof(path)); assert(mkdir(path, 0700) == 0);
    written = snprintf(path, sizeof(path), "%s/var/lib/dpkg/info", root);
    assert(written > 0 && (size_t)written < sizeof(path)); assert(mkdir(path, 0700) == 0);

    written = snprintf(path, sizeof(path), "%s/var/lib/dpkg/status", root);
    assert(written > 0 && (size_t)written < sizeof(path));
    file = fopen(path, "wb"); assert(file != NULL);
    assert(fputs(
        "Package: zeta-pkg\nVersion: 1.0\nArchitecture: amd64\nStatus: install ok installed\nInstalled-Size: 1\n\n"
        "Package: alpha-pkg\nVersion: 2.0\nArchitecture: amd64\nStatus: install ok installed\nInstalled-Size: 2\n",
        file) >= 0);
    assert(fclose(file) == 0);

    written = snprintf(path, sizeof(path), "%s/var/lib/dpkg/info/zeta-pkg.list", root);
    assert(written > 0 && (size_t)written < sizeof(path));
    file = fopen(path, "wb"); assert(file != NULL);
    assert(fputs("/zeta/artifact\n", file) >= 0); assert(fclose(file) == 0);

    written = snprintf(path, sizeof(path), "%s/var/lib/dpkg/info/alpha-pkg.list", root);
    assert(written > 0 && (size_t)written < sizeof(path));
    file = fopen(path, "wb"); assert(file != NULL);
    assert(fputs("/alpha/artifact\n", file) >= 0); assert(fclose(file) == 0);
}

void remove_multi_package_fixture(const char *root) {
    char path[512];
    assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg/info/zeta-pkg.list", root) > 0); assert(unlink(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg/info/alpha-pkg.list", root) > 0); assert(unlink(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg/info", root) > 0); assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg/status", root) > 0); assert(unlink(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var/lib/dpkg", root) > 0); assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var/lib", root) > 0); assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof(path), "%s/var", root) > 0); assert(rmdir(path) == 0);
    assert(rmdir(root) == 0);
}
