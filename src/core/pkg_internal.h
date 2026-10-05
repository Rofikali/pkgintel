#ifndef PKGINTEL_INTERNAL_H
#define PKGINTEL_INTERNAL_H

#include "pkgintel/pkgintel.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>

typedef struct pkg_package_record {
    char *name;
    char *version;
    char *architecture;
    uint64_t installed_size;
    uint64_t file_count;
    uint64_t missing_file_count;
    uint64_t invalid_path_count;
} pkg_package_record;

struct pkg_context {
    pkg_context_options options;
};

struct pkg_target {
    pkg_context *context;
    pkg_target_type type;
    char *root;
    int root_fd;
};

struct pkg_scan_result {
    char *target_root;
    pkg_package_record *packages;
    size_t package_count;
    uint64_t diagnostic_count;
};

char *pkg_strdup_internal(const char *value);
pkg_status pkg_dpkg_scan(pkg_context *context, pkg_target *target,
                         const pkg_scan_options *options, pkg_scan_result *result);
pkg_status pkg_target_open_root(pkg_target *target);
int pkg_target_open_path(const pkg_target *target, const char *path, int flags);
int pkg_target_lstat_path(const pkg_target *target, const char *path, struct stat *st);

#endif
