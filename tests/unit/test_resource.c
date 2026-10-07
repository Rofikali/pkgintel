#include "internal/pkg_model.h"
#include "internal/pkg_snapshot.h"
#include "pkgintel/pkgintel.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

int test_snapshot_resource_budgets(void) {
    pkg_snapshot *snapshot = calloc(1, sizeof(*snapshot));
    assert(snapshot != NULL);
    const unsigned char path[] = "/usr/bin/example";

    snapshot->max_artifacts = 1U;
    assert(pkg_snapshot_add_artifact(snapshot, path, sizeof(path) - 1U,
        PKG_ARTIFACT_REGULAR, PKG_ARTIFACT_PRESENT, NULL) == 0);
    assert(snapshot->artifact_count == 1U);
    assert(pkg_snapshot_add_artifact(snapshot, path, sizeof(path) - 1U,
        PKG_ARTIFACT_REGULAR, PKG_ARTIFACT_PRESENT, NULL) == -2);
    assert(snapshot->artifact_count == 1U);

    snapshot->max_diagnostics = 1U;
    assert(pkg_snapshot_add_diagnostic(snapshot, PKG_ERR_PARSE, PKG_DIAGNOSTIC_WARNING,
        PKG_EVIDENCE_DPKG, "TEST", "diagnostic") == 0);
    assert(snapshot->diagnostic_count == 1U);
    assert(pkg_snapshot_add_diagnostic(&snapshot, PKG_ERR_PARSE, PKG_DIAGNOSTIC_WARNING,
        PKG_EVIDENCE_DPKG, "TEST2", "diagnostic") == -2);
    assert(snapshot->diagnostic_count == 1U);

    pkg_snapshot_destroy(snapshot);
    return 0;
}
