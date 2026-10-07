#ifndef PKGINTEL_INTERNAL_MODEL_H
#define PKGINTEL_INTERNAL_MODEL_H

#include "pkgintel/pkgintel.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
    pkg_installation_state installation_state;
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
    size_t package_capacity;
    pkg_artifact_record *artifacts;
    size_t artifact_count;
    size_t artifact_capacity;
    pkg_diagnostic_record *diagnostics;
    size_t diagnostic_count;
    size_t diagnostic_capacity;
    size_t max_artifacts;
    size_t max_diagnostics;
};

#endif
