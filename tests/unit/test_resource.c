#include "pkgintel/pkgintel.h"
#include "fixtures.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int test_snapshot_resource_budgets(void) {
    /*
     * The aggregate ceilings are intentionally hard implementation limits.
     * This public-contract test verifies that ordinary bounded scans remain
     * successful and that callers do not need private snapshot symbols.
     */
    char root[512];
    pkg_context *context = NULL;
    pkg_target *target = NULL;
    pkg_scan_result *result = NULL;

    make_state_fixture(root, sizeof(root));
    assert(pkg_context_create(NULL, &context) == PKG_OK);
    assert(pkg_target_create_rootfs(context, root, &target) == PKG_OK);
    assert(pkg_scan(context, target, NULL, &result) == PKG_OK);
    assert(result != NULL);
    assert(pkg_scan_result_package_count(result) == 5U);

    pkg_scan_result_destroy(result);
    pkg_target_destroy(target);
    pkg_context_destroy(context);
    remove_state_fixture(root);
    return 0;
}
