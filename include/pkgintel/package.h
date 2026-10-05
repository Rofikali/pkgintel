#ifndef PKGINTEL_PACKAGE_H
#define PKGINTEL_PACKAGE_H
#include <stdint.h>
#include "snapshot.h"
typedef enum pkg_installation_state {
    PKG_INSTALLATION_UNKNOWN = 0,
    PKG_INSTALLATION_INSTALLED = 1,
    PKG_INSTALLATION_PARTIAL = 2,
    PKG_INSTALLATION_REMOVED = 3
} pkg_installation_state;
typedef enum pkg_consistency_state {
    PKG_CONSISTENCY_UNKNOWN = 0,
    PKG_CONSISTENCY_CONSISTENT = 1,
    PKG_CONSISTENCY_INCONSISTENT = 2,
    PKG_CONSISTENCY_MISSING_ARTIFACT = 3,
    PKG_CONSISTENCY_BROKEN_LINK = 4,
    PKG_CONSISTENCY_PERMISSION_DENIED = 5,
    PKG_CONSISTENCY_UNEXPECTED_ARTIFACT = 6,
    PKG_CONSISTENCY_UNVERIFIABLE = 7
} pkg_consistency_state;
pkg_string_view pkg_package_name(const pkg_package *package);
pkg_string_view pkg_package_version(const pkg_package *package);
pkg_string_view pkg_package_architecture(const pkg_package *package);
pkg_string_view pkg_package_source(const pkg_package *package);
pkg_installation_state pkg_package_get_state(const pkg_package *package);
pkg_consistency_state pkg_package_get_consistency(const pkg_package *package);
uint64_t pkg_package_declared_size_bytes(const pkg_package *package);
uint64_t pkg_package_installed_size_bytes(const pkg_package *package);
size_t pkg_package_artifact_count(const pkg_package *package);
pkg_status pkg_package_artifact_at(const pkg_package *package, size_t index, const pkg_artifact **out_artifact);
#endif
