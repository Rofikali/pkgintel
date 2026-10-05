#include "pkgintel/pkgintel.h"

#include <stdio.h>
#include <string.h>

static int command_scan(void) {
    pkg_context *context = NULL;
    pkg_target *target = NULL;
    pkg_scan_result *result = NULL;
    pkg_status status;

    status = pkg_context_create(NULL, &context);
    if (status != PKG_OK) {
        fprintf(stderr, "pkgintel: context: %s\n", pkg_status_string(status));
        return 1;
    }

    status = pkg_target_create_local(context, &target);
    if (status != PKG_OK) {
        fprintf(stderr, "pkgintel: target: %s\n", pkg_status_string(status));
        pkg_context_destroy(context);
        return 1;
    }

    status = pkg_scan(context, target, NULL, &result);
    if (status != PKG_OK) {
        fprintf(stderr, "pkgintel: scan: %s\n", pkg_status_string(status));
        pkg_target_destroy(target);
        pkg_context_destroy(context);
        return 1;
    }

    printf("Target: %s\n", pkg_scan_result_target_root(result));
    printf("Packages: %zu\n", pkg_scan_result_package_count(result));

    pkg_scan_result_destroy(result);
    pkg_target_destroy(target);
    pkg_context_destroy(context);
    return 0;
}

static void usage(const char *program) {
    fprintf(stderr, "usage: %s scan\n", program);
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "scan") == 0) return command_scan();
    usage(argv[0]);
    return 2;
}
