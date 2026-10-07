#ifndef PKGINTEL_BENCH_ALLOC_STATS_H
#define PKGINTEL_BENCH_ALLOC_STATS_H

#include <stddef.h>
#include <stdint.h>

typedef struct pkg_bench_alloc_stats {
    uint64_t malloc_calls;
    uint64_t calloc_calls;
    uint64_t realloc_calls;
    uint64_t free_calls;
    uint64_t malloc_bytes_requested;
    uint64_t calloc_bytes_requested;
    uint64_t realloc_bytes_requested;
    uint64_t realloc_bytes_grown;
    uint64_t realloc_bytes_shrunk;
    uint64_t peak_live_bytes;
    uint64_t final_live_bytes;
} pkg_bench_alloc_stats;

void *pkg_bench_malloc(size_t size);
void *pkg_bench_calloc(size_t count, size_t size);
void *pkg_bench_realloc(void *ptr, size_t size);
void pkg_bench_free(void *ptr);

void pkg_bench_alloc_stats_reset(void);
void pkg_bench_alloc_stats_get(pkg_bench_alloc_stats *out);

#endif
