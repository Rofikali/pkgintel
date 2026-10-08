#include "internal/pkg_model.h"
#include <stdint.h>
#include <string.h>
#ifdef PKGINTEL_BENCHMARK_ALLOC_STATS
#include "alloc_probe_macros.h"
#endif

pkg_string_view pkg_package_name(const pkg_package *package) { const pkg_package_record *p = (const pkg_package_record *)package; return p == NULL || p->name == NULL ? (pkg_string_view){NULL,0U} : (pkg_string_view){p->name,strlen(p->name)}; }

pkg_string_view pkg_package_version(const pkg_package *package) { const pkg_package_record *p = (const pkg_package_record *)package; return p == NULL || p->version == NULL ? (pkg_string_view){NULL,0U} : (pkg_string_view){p->version,strlen(p->version)}; }

pkg_string_view pkg_package_architecture(const pkg_package *package) { const pkg_package_record *p = (const pkg_package_record *)package; return p == NULL || p->architecture == NULL ? (pkg_string_view){NULL,0U} : (pkg_string_view){p->architecture,strlen(p->architecture)}; }

pkg_installation_state pkg_package_get_state(const pkg_package *package) {
    const pkg_package_record *p = (const pkg_package_record *)package;
    return p == NULL ? PKG_INSTALLATION_UNKNOWN : p->installation_state;
}

pkg_consistency_state pkg_package_get_consistency(const pkg_package *package) {
    const pkg_package_record *p = (const pkg_package_record *)package;
    size_t i;
    pkg_artifact_state first_bad = PKG_ARTIFACT_STATE_UNKNOWN;
    bool has_bad = false;
    if (p == NULL) return PKG_CONSISTENCY_UNKNOWN;
    if (p->correlation_state != PKG_CORRELATION_COMPLETE || p->owner_snapshot == NULL) return PKG_CONSISTENCY_UNKNOWN;
    if (p->artifact_start > p->owner_snapshot->artifact_count ||
        p->artifact_count > p->owner_snapshot->artifact_count - p->artifact_start) return PKG_CONSISTENCY_UNVERIFIABLE;

    for (i = 0U; i < p->artifact_count; ++i) {
        const pkg_artifact_record *artifact = &p->owner_snapshot->artifacts[p->artifact_start + i];
        if (artifact->state == PKG_ARTIFACT_PRESENT) continue;
        if (!has_bad) {
            first_bad = artifact->state;
            has_bad = true;
        } else if (first_bad != artifact->state) {
            return PKG_CONSISTENCY_INCONSISTENT;
        }
    }

    if (!has_bad) return PKG_CONSISTENCY_CONSISTENT;

    switch (first_bad) {
        case PKG_ARTIFACT_STATE_UNKNOWN:
            return PKG_CONSISTENCY_UNVERIFIABLE;
        case PKG_ARTIFACT_MISSING:
            return PKG_CONSISTENCY_MISSING_ARTIFACT;
        case PKG_ARTIFACT_BROKEN_LINK:
            return PKG_CONSISTENCY_BROKEN_LINK;
        case PKG_ARTIFACT_PERMISSION_DENIED:
            return PKG_CONSISTENCY_PERMISSION_DENIED;
        case PKG_ARTIFACT_UNVERIFIABLE:
        default:
            return PKG_CONSISTENCY_UNVERIFIABLE;
    }
}

uint64_t pkg_package_installed_size_bytes(const pkg_package *package) { const pkg_package_record *p=(const pkg_package_record *)package; return p==NULL?0U:p->installed_size; }

size_t pkg_package_artifact_count(const pkg_package *package) { const pkg_package_record *p=(const pkg_package_record *)package; return p==NULL?0U:p->artifact_count; }

pkg_status pkg_package_artifact_at(const pkg_package *package,size_t index,const pkg_artifact **out_artifact) {
    const pkg_package_record *p=(const pkg_package_record *)package;
    size_t artifact_index;
    if(out_artifact==NULL)return PKG_ERR_INVALID_ARGUMENT;
    *out_artifact=NULL;
    if(p==NULL||p->owner_snapshot==NULL||index>=p->artifact_count)return PKG_ERR_NOT_FOUND;
    if(p->artifact_start > SIZE_MAX - index)return PKG_ERR_INTERNAL;
    artifact_index = p->artifact_start + index;
    if(artifact_index >= p->owner_snapshot->artifact_count)return PKG_ERR_INTERNAL;
    *out_artifact=(const pkg_artifact *)&p->owner_snapshot->artifacts[artifact_index];
    return PKG_OK;
}
