#define _GNU_SOURCE
#include "alloc_stats.h"
#include <malloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static pkg_bench_alloc_stats stats;
static uint64_t live_bytes;

static uint64_t usable_bytes(void *ptr) {
    return ptr == NULL ? 0U : (uint64_t)malloc_usable_size(ptr);
}

static void add_live(uint64_t bytes) {
    if (UINT64_MAX - live_bytes < bytes) live_bytes = UINT64_MAX;
    else live_bytes += bytes;
    if (live_bytes > stats.peak_live_bytes) stats.peak_live_bytes = live_bytes;
}

static void subtract_live(uint64_t bytes) {
    live_bytes = bytes > live_bytes ? 0U : live_bytes - bytes;
}

void *pkg_bench_malloc_class(size_t size, unsigned int alloc_class) {
    void *ptr = malloc(size);
    ++stats.malloc_calls;
    ++stats.by_class[alloc_class].malloc_calls;
    if (UINT64_MAX - stats.malloc_bytes_requested < (uint64_t)size)
        stats.malloc_bytes_requested = UINT64_MAX;
    else
        stats.malloc_bytes_requested += (uint64_t)size;
    if (UINT64_MAX - stats.by_class[alloc_class].malloc_bytes_requested < (uint64_t)size)
        stats.by_class[alloc_class].malloc_bytes_requested = UINT64_MAX;
    else
        stats.by_class[alloc_class].malloc_bytes_requested += (uint64_t)size;
    if (ptr != NULL) add_live(usable_bytes(ptr));
    return ptr;
}

void *pkg_bench_calloc_class(size_t count, size_t size, unsigned int alloc_class) {
    void *ptr = calloc(count, size);
    uint64_t requested = count != 0U && size > SIZE_MAX / count ? UINT64_MAX : (uint64_t)(count * size);
    ++stats.calloc_calls;
    ++stats.by_class[alloc_class].calloc_calls;
    if (UINT64_MAX - stats.calloc_bytes_requested < requested)
        stats.calloc_bytes_requested = UINT64_MAX;
    else
        stats.calloc_bytes_requested += requested;
    if (UINT64_MAX - stats.by_class[alloc_class].calloc_bytes_requested < requested)
        stats.by_class[alloc_class].calloc_bytes_requested = UINT64_MAX;
    else
        stats.by_class[alloc_class].calloc_bytes_requested += requested;
    if (ptr != NULL) add_live(usable_bytes(ptr));
    return ptr;
}

void *pkg_bench_realloc_class(void *ptr, size_t size, unsigned int alloc_class) {
    uint64_t old_live = usable_bytes(ptr);
    void *grown = realloc(ptr, size);
    uint64_t new_live;
    ++stats.realloc_calls;
    ++stats.by_class[alloc_class].realloc_calls;
    if (UINT64_MAX - stats.realloc_bytes_requested < (uint64_t)size)
        stats.realloc_bytes_requested = UINT64_MAX;
    else
        stats.realloc_bytes_requested += (uint64_t)size;
    if (UINT64_MAX - stats.by_class[alloc_class].realloc_bytes_requested < (uint64_t)size)
        stats.by_class[alloc_class].realloc_bytes_requested = UINT64_MAX;
    else
        stats.by_class[alloc_class].realloc_bytes_requested += (uint64_t)size;
    if (grown == NULL) return NULL;

    new_live = usable_bytes(grown);
    if (old_live < new_live) {
        uint64_t delta = new_live - old_live;
        if (UINT64_MAX - stats.realloc_bytes_grown < delta) stats.realloc_bytes_grown = UINT64_MAX;
        else stats.realloc_bytes_grown += delta;
        subtract_live(old_live);
        add_live(new_live);
    } else if (old_live > new_live) {
        uint64_t delta = old_live - new_live;
        if (UINT64_MAX - stats.realloc_bytes_shrunk < delta) stats.realloc_bytes_shrunk = UINT64_MAX;
        else stats.realloc_bytes_shrunk += delta;
        subtract_live(delta);
    }
    return grown;
}

void pkg_bench_free_class(void *ptr, unsigned int alloc_class) {
    uint64_t bytes = usable_bytes(ptr);
    ++stats.free_calls;
    ++stats.by_class[alloc_class].free_calls;
    subtract_live(bytes);
    free(ptr);
}

void pkg_bench_alloc_stats_reset(void) {
    memset(&stats, 0, sizeof(stats));
    live_bytes = 0U;
}

void pkg_bench_alloc_stats_get(pkg_bench_alloc_stats *out) {
    if (out == NULL) return;
    *out = stats;
    out->final_live_bytes = live_bytes;
}
