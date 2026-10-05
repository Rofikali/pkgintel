#ifndef PKGINTEL_INTERNAL_H
#define PKGINTEL_INTERNAL_H

#include "pkgintel/pkgintel.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>

typedef struct pkg_artifact_record {
    unsigned char *path;
    size_t path_size;
    pkg_artifact_kind kind;
    pkg_artifact_state state;
    uint64_t logical_size;
    uint64_t allocated_size;
    bool allocated_size_valid;
} pkg_artifact_record;

struct pkg_snapshot;
typedef struct pkg_package_record {
    char *name;
    char *version;
    char *architecture;
    uint64_t installed_size;
    uint64_t file_count;
    uint64_t missing_file_count;
    uint64_t invalid_path_count;
    size_t artifact_start;
    size_t artifact_count;
    struct pkg_snapshot *owner_snapshot;
} pkg_package_record;

typedef struct pkg_diagnostic_record {
    char *code;
    char *message;
    pkg_status status;
    pkg_diagnostic_severity severity;
    pkg_evidence_source evidence_source;
} pkg_diagnostic_record;

struct pkg_context { pkg_context_options options; };
struct pkg_target { pkg_context *context; pkg_target_type type; char *root; int root_fd; };

struct pkg_snapshot {
    char *target_root;
    pkg_package_record *packages;
    size_t package_count;
    pkg_artifact_record *artifacts;
    size_t artifact_count;
    pkg_diagnostic_record *diagnostics;
    size_t diagnostic_count;
};

typedef struct pkg_scan_result pkg_scan_result;

char *pkg_strdup_internal(const char *value);
pkg_status pkg_dpkg_scan(pkg_context *context, pkg_target *target,
                         const pkg_scan_options *options, struct pkg_snapshot *result);
pkg_status pkg_target_open_root(pkg_target *target);
int pkg_target_open_path(const pkg_target *target, const char *path, int flags);
int pkg_target_lstat_path(const pkg_target *target, const char *path, struct stat *st);
int pkg_snapshot_add_artifact(struct pkg_snapshot *snapshot, const unsigned char *path,
                              size_t path_size, pkg_artifact_kind kind,
                              pkg_artifact_state state, const struct stat *st);
int pkg_snapshot_add_diagnostic(struct pkg_snapshot *snapshot, pkg_status status,
                                pkg_diagnostic_severity severity,
                                pkg_evidence_source source,
                                const char *code, const char *message);

#endif
