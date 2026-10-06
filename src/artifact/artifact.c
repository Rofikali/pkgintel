#include "internal/pkg_internal.h"
#include <stdlib.h>
#include <string.h>

int pkg_snapshot_add_artifact(pkg_snapshot *snapshot, const unsigned char *path, size_t path_size, pkg_artifact_kind kind, pkg_artifact_state state, const struct stat *st) {
    pkg_artifact_record *grown;
    size_t n;
    if (snapshot == NULL || path == NULL || path_size == 0U || path_size > SIZE_MAX - 1U) return -1;
    n = snapshot->artifact_count + 1U;
    if (n < snapshot->artifact_count || n > SIZE_MAX / sizeof(*grown)) return -1;
    grown = realloc(snapshot->artifacts, n * sizeof(*grown));
    if (grown == NULL) return -1;
    snapshot->artifacts = grown;
    memset(&grown[n - 1U], 0, sizeof(grown[n - 1U]));
    grown[n - 1U].path = malloc(path_size + 1U);
    if (grown[n - 1U].path == NULL) return -1;
    memcpy(grown[n - 1U].path, path, path_size);
    grown[n - 1U].path[path_size] = 0;
    grown[n - 1U].path_size = path_size;
    grown[n - 1U].kind = kind;
    grown[n - 1U].state = state;
    if (st != NULL) {
        grown[n - 1U].logical_size = S_ISREG(st->st_mode) ? (uint64_t)st->st_size : 0U;
        if (st->st_blocks >= 0 && (uint64_t)st->st_blocks <= UINT64_MAX / UINT64_C(512)) { grown[n - 1U].allocated_size = (uint64_t)st->st_blocks * UINT64_C(512); grown[n - 1U].allocated_size_valid = true; }
    }
    snapshot->artifact_count = n;
    return 0;
}

pkg_path pkg_artifact_path(const pkg_artifact *artifact) { const pkg_artifact_record *a=(const pkg_artifact_record *)artifact; return a==NULL?(pkg_path){NULL,0U}:(pkg_path){a->path,a->path_size}; }

pkg_artifact_kind pkg_artifact_get_kind(const pkg_artifact *artifact) { const pkg_artifact_record *a=(const pkg_artifact_record *)artifact; return a==NULL?PKG_ARTIFACT_UNKNOWN:a->kind; }

pkg_artifact_state pkg_artifact_get_state(const pkg_artifact *artifact) { const pkg_artifact_record *a=(const pkg_artifact_record *)artifact; return a==NULL?PKG_ARTIFACT_STATE_UNKNOWN:a->state; }

uint64_t pkg_artifact_logical_size_bytes(const pkg_artifact *artifact) { const pkg_artifact_record *a=(const pkg_artifact_record *)artifact; return a==NULL?0U:a->logical_size; }

bool pkg_artifact_logical_size_available(const pkg_artifact *artifact) { const pkg_artifact_record *a=(const pkg_artifact_record *)artifact; return a!=NULL&&a->kind==PKG_ARTIFACT_REGULAR; }

uint64_t pkg_artifact_allocated_size_bytes(const pkg_artifact *artifact) { const pkg_artifact_record *a=(const pkg_artifact_record *)artifact; return a==NULL?0U:a->allocated_size; }

bool pkg_artifact_allocated_size_available(const pkg_artifact *artifact) { const pkg_artifact_record *a=(const pkg_artifact_record *)artifact; return a!=NULL&&a->allocated_size_valid; }
