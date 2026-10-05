#include "pkgintel/pkgintel.h"

#include <assert.h>
#include <string.h>

int main(void) {
    pkg_context *context = NULL;
    pkg_target *target = NULL;
    pkg_scan_result *result = NULL;

    assert(pkg_context_create(NULL, &context) == PKG_OK);
    assert(context != NULL);

    assert(pkg_target_create_rootfs(context, "/tmp/pkgintel-fixture", &target) == PKG_OK);
    assert(target != NULL);

    assert(pkg_scan(context, target, NULL, &result) == PKG_OK);
    assert(result != NULL);
    assert(strcmp(pkg_scan_result_target_root(result), "/tmp/pkgintel-fixture") == 0);
    assert(pkg_scan_result_package_count(result) == 0);

    pkg_scan_result_destroy(result);
    pkg_target_destroy(target);
    pkg_context_destroy(context);

    assert(strcmp(pkg_status_string(PKG_ERR_PARSE), "parse error") == 0);
    return 0;
}
