#include "internal/pkg_model.h"
#include "internal/pkg_snapshot.h"
#include <stdlib.h>
#include <string.h>

static int grow_artifacts(pkg_snapshot *snapshot, size_t required) {
    size_t capacity;
    size_t new_capacity;
    pkg_artifact_record *grown;
    if (required <= snapshot->artifact_capacity) return 0;
    capacity = snapshot->artifact_capacity == 0U ? 8U : snapshot->artifact_capacity;
    new_capacity = capacity;
    while (new_capacity < required) {
        if (new_capacity > SIZE_MAX / 2U) {
            new_capacity = required;
            break;
        }
        new_capacity *= 2U;
    }
    if (snapshot->max_artifacts != 0U && new_capacity > snapshot->max_artifacts) new_capacity = snapshot->max_artifacts;
    if (new_capacity < required || new_capacity > SIZE_MAX / sizeof(*grown)) return -1;
    grown = realloc(snapshot->artifacts, new_capacity * sizeof(*grown));
    if (grown == NULL) return -1;
    snapshot->artifacts = grown;
    snapshot->artifact_capacity = new_capacity;
    return 0;
}

int pkg_snapshot_add_artifact(pkg_snapshot *snapshot, const unsigned char *path, size_t path_size, pkg_artifact_kind kind, pkg_artifact_state state, const struct stat *st) {
    pkg_artifact_record *record;
    unsigned char *owned_path;
    size_t index;

    if (snapshot == NULL || path == NULL || path_size == 0U || path_size > SIZE_MAX - 1U) return -1;
    if (snapshot->max_artifacts != 0U && snapshot->artifact_count >= snapshot->max_artifacts) return -2;
    owned_path = malloc(path_size + 1U);
    if (owned_path == NULL) return -1;
    memcpy(owned_path, path, path_size);
    owned_path[path_size] = 0;
    if (snapshot->artifact_count == SIZE_MAX) {
        free(owned_path);
        return -1;
    }
    index = snapshot->artifact_count;
    if (grow_artifacts(snapshot, index + 1U) != 0) {
        free(owned_path);
        return -1;
    }
    record = &snapshot->artifacts[index];
    *record = (pkg_artifact_record){ .path = owned_path, .path_size = path_size, .kind = kind, .state = state };
    if (st != NULL) {
        record->logical_size = S_ISREG(st->st_mode) ? (uint64_t)st->st_size : 0U;
        if (st->st_blocks >= 0 && (uint64_t)st->st_blocks <= UINT64_MAX / UINT64_C(512)) {
            record->allocated_size = (uint64_t)st->st_blocks * UINT64_C(512);
            record->allocated_size_valid = true;
        }
    }
    snapshot->artifact_count = index + 1U;
    return 0;
}

pkg_path pkg_artifact_path(const pkg_artifact *artifact) { const pkg_artifact_record *a=(const pkg_artifact_record *)artifact; return a==NULL?(pkg_path){NULL,0U}:(pkg_path){a->path,a->path_size}; }
pkg_artifact_kind pkg_artifact_get_kind(const pkg_artifact *artifact) { const pkg_artifact_record *a=(const pkg_artifact_record *)artifact; return a==NULL?PKG_ARTIFACT_UNKNOWN:a->kind; }
pkg_artifact_state pkg_artifact_get_state(const pkg_artifact *artifact) { const pkg_artifact_record *a=(const pkg_artifact_record *)artifact; return a==NULL?PKG_ARTIFACT_STATE_UNKNOWN:a->state; }
uint64_t pkg_artifact_logical_size_bytes(const pkg_artifact *artifact) { const pkg_artifact_record *a=(const pkg_artifact_record *)artifact; return a==NULL?0U:a->logical_size; }
bool pkg_artifact_logical_size_available(const pkg_artifact *artifact) { const pkg_artifact_record *a=(const pkg_artifact_record *)artifact; return a!=NULL&&a->kind==PKG_ARTIFACT_REGULAR&&a->state==PKG_ARTIFACT_PRESENT; }
uint64_t pkg_artifact_allocated_size_bytes(const pkg_artifact *artifact) { const pkg_artifact_record *a=(const pkg_artifact_record *)artifact; return a==NULL?0U:a->allocated_size; }
bool pkg_artifact_allocated_size_available(const pkg_artifact *artifact) { const pkg_artifact_record *a=(const pkg_artifact_record *)artifact; return a!=NULL&&a->allocated_size_valid&&a->state==PKG_ARTIFACT_PRESENT; }
