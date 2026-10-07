
#define _GNU_SOURCE
#include "pkgintel/pkgintel.h"
#include "fixtures.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

extern void *__real_malloc(size_t size);
extern void *__real_calloc(size_t count, size_t size);
extern void *__real_realloc(void *ptr, size_t size);

typedef enum test_alloc_kind {
    TEST_ALLOC_MALLOC = 0,
    TEST_ALLOC_CALLOC,
    TEST_ALLOC_REALLOC
} test_alloc_kind;

static size_t test_malloc_calls;
static size_t test_calloc_calls;
static size_t test_realloc_calls;
static test_alloc_kind test_fail_kind = TEST_ALLOC_MALLOC;
static size_t test_fail_at;

void *__wrap_malloc(size_t size) {
    ++test_malloc_calls;
    if (test_fail_at != 0U && test_fail_kind == TEST_ALLOC_MALLOC && test_malloc_calls == test_fail_at) return NULL;
    return __real_malloc(size);
}

void *__wrap_calloc(size_t count, size_t size) {
    ++test_calloc_calls;
    if (test_fail_at != 0U && test_fail_kind == TEST_ALLOC_CALLOC && test_calloc_calls == test_fail_at) return NULL;
    return __real_calloc(count, size);
}

void *__wrap_realloc(void *ptr, size_t size) {
    ++test_realloc_calls;
    if (test_fail_at != 0U && test_fail_kind == TEST_ALLOC_REALLOC && test_realloc_calls == test_fail_at) return NULL;
    return __real_realloc(ptr, size);
}

static void reset_allocator_observation(void) {
    test_malloc_calls = 0U;
    test_calloc_calls = 0U;
    test_realloc_calls = 0U;
    test_fail_at = 0U;
}

static size_t allocation_count(test_alloc_kind kind) {
    switch (kind) {
        case TEST_ALLOC_MALLOC: return test_malloc_calls;
        case TEST_ALLOC_CALLOC: return test_calloc_calls;
        case TEST_ALLOC_REALLOC: return test_realloc_calls;
    }
    return 0U;
}

static void assert_scan_survives_allocator_failure(pkg_context *context, pkg_target *target,
                                                   test_alloc_kind kind, size_t fail_at) {
    pkg_scan_result *result = NULL;
    reset_allocator_observation();
    test_fail_kind = kind;
    test_fail_at = fail_at;
    assert(pkg_scan(context, target, NULL, &result) == PKG_ERR_INTERNAL);
    assert(result == NULL);
    test_fail_at = 0U;
}

int test_snapshot_resource_budgets(void) {
    char root[512];
    pkg_context *context = NULL;
    pkg_target *target = NULL;
    pkg_scan_result *result = NULL;

    make_state_fixture(root, sizeof(root));
    assert(pkg_context_create(NULL, &context) == PKG_OK);
    assert(pkg_target_create_rootfs(context, root, &target) == PKG_OK);

    /* Establish deterministic allocation counts for the public scan path. */
    reset_allocator_observation();
    assert(pkg_scan(context, target, NULL, &result) == PKG_OK);
    assert(result != NULL);
    assert(pkg_scan_result_package_count(result) == 5U);
    const size_t malloc_calls = test_malloc_calls;
    const size_t calloc_calls = test_calloc_calls;
    const size_t realloc_calls = test_realloc_calls;
    assert(malloc_calls > 0U);
    assert(calloc_calls > 0U);
    assert(realloc_calls > 0U);
    pkg_scan_result_destroy(result);
    result = NULL;

    /*
     * Fail every observed allocation site, one at a time. The production ABI
     * is unchanged: failure injection exists only in this test executable via
     * the linker --wrap facility.
     */
    const size_t baseline_counts[] = { malloc_calls, calloc_calls, realloc_calls };
    for (test_alloc_kind kind = TEST_ALLOC_MALLOC; kind <= TEST_ALLOC_REALLOC; ++kind) {
        const size_t calls = baseline_counts[kind];
        for (size_t fail_at = 1U; fail_at <= calls; ++fail_at)
            assert_scan_survives_allocator_failure(context, target, kind, fail_at);
    }

    test_fail_at = 0U;
    reset_allocator_observation();
    pkg_target_destroy(target);
    pkg_context_destroy(context);
    remove_state_fixture(root);
    return 0;
}
