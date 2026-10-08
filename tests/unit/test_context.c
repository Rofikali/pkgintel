#define _GNU_SOURCE
#include "pkgintel/pkgintel.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int test_context_behaviour(void) {
    pkg_context *context = NULL;
    {
        pkg_context_options unsupported = PKG_CONTEXT_OPTIONS_INIT;
        unsupported.max_files = 1U;
        assert(pkg_context_create(&unsupported, &context) == PKG_ERR_UNSUPPORTED);
        assert(context == NULL);
        unsupported = (pkg_context_options)PKG_CONTEXT_OPTIONS_INIT;
        unsupported.max_directories = 1U;
        assert(pkg_context_create(&unsupported, &context) == PKG_ERR_UNSUPPORTED);
        assert(context == NULL);
        unsupported = (pkg_context_options)PKG_CONTEXT_OPTIONS_INIT;
        unsupported.max_depth = 1U;
        assert(pkg_context_create(&unsupported, &context) == PKG_ERR_UNSUPPORTED);
        assert(context == NULL);
        unsupported = (pkg_context_options)PKG_CONTEXT_OPTIONS_INIT;
        unsupported.max_bytes = 1U;
        assert(pkg_context_create(&unsupported, &context) == PKG_ERR_UNSUPPORTED);
        assert(context == NULL);
        unsupported = (pkg_context_options)PKG_CONTEXT_OPTIONS_INIT;
        unsupported.max_elf_bytes = 1U;
        assert(pkg_context_create(&unsupported, &context) == PKG_ERR_UNSUPPORTED);
        assert(context == NULL);
        unsupported = (pkg_context_options)PKG_CONTEXT_OPTIONS_INIT;
        unsupported.struct_size = sizeof(uint32_t);
        assert(pkg_context_create(&unsupported, &context) == PKG_ERR_INVALID_ARGUMENT);
        assert(context == NULL);
    }    assert(pkg_context_create(NULL, &context) == PKG_OK);
    assert(context != NULL);
    pkg_context_destroy(context);
    return 0;
}
