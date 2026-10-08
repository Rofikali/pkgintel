#ifndef PKGINTEL_PACKAGE_H
#define PKGINTEL_PACKAGE_H
#include "export.h"
#include <stdint.h>
#include "snapshot.h"

/*
 * Installation state is derived from the three-part dpkg Status field:
 * desired action, error flag, and actual package state. v0.1 ignores desired
 * action for installation-state classification. installed -> INSTALLED,
 * transitional states -> PARTIAL, not-installed/config-files -> REMOVED,
 * reinstreq -> PARTIAL, and unknown/unrecognized values -> UNKNOWN.
 * This is separate from filesystem consistency.
 */
typedef enum pkg_installation_state {
 PKG_INSTALLATION_UNKNOWN=0,
 PKG_INSTALLATION_INSTALLED=1,
 PKG_INSTALLATION_PARTIAL=2,
 PKG_INSTALLATION_REMOVED=3
} pkg_installation_state;

/*
 * Consistency describes observed package-file evidence, not whether dpkg
 * considers the installation complete. A PARTIAL or REMOVED package is not
 * automatically reported as INCONSISTENT merely from its installation state.
 *
 * CONSISTENT is returned only when file correlation was requested, completed
 * for this package, and every observed artifact is PRESENT. If correlation
 * was not requested or could not be completed, the result is UNKNOWN.
 * A single failure class maps to its specific state. Mixed failure classes
 * map to INCONSISTENT. UNEXPECTED_ARTIFACT is reserved for a future scan
 * capability that enumerates unowned filesystem artifacts.
 */
typedef enum pkg_consistency_state {
 PKG_CONSISTENCY_UNKNOWN=0,
 PKG_CONSISTENCY_CONSISTENT=1,
 PKG_CONSISTENCY_INCONSISTENT=2,
 PKG_CONSISTENCY_MISSING_ARTIFACT=3,
 PKG_CONSISTENCY_BROKEN_LINK=4,
 PKG_CONSISTENCY_PERMISSION_DENIED=5,
 PKG_CONSISTENCY_UNEXPECTED_ARTIFACT=6,
 PKG_CONSISTENCY_UNVERIFIABLE=7
} pkg_consistency_state;

PKGINTEL_API pkg_string_view pkg_package_name(const pkg_package *package);
PKGINTEL_API pkg_string_view pkg_package_version(const pkg_package *package);
PKGINTEL_API pkg_string_view pkg_package_architecture(const pkg_package *package);
PKGINTEL_API pkg_installation_state pkg_package_get_state(const pkg_package *package);
PKGINTEL_API pkg_consistency_state pkg_package_get_consistency(const pkg_package *package);
PKGINTEL_API uint64_t pkg_package_installed_size_bytes(const pkg_package *package);
PKGINTEL_API size_t pkg_package_artifact_count(const pkg_package *package);
PKGINTEL_API pkg_status pkg_package_artifact_at(const pkg_package *package, size_t index, const pkg_artifact **out_artifact);
#endif
