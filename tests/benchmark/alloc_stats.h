#ifndef PKGINTEL_BENCH_ALLOC_STATS_H
#define PKGINTEL_BENCH_ALLOC_STATS_H

#include <stddef.h>
#include <stdint.h>

#ifndef PKGINTEL_BENCH_ALLOC_CLASS
#define PKGINTEL_BENCH_ALLOC_CLASS 0
#endif

enum {
    PKG_BENCH_ALLOC_UNKNOWN = 0,
    PKG_BENCH_ALLOC_CONTEXT,
    PKG_BENCH_ALLOC_SCAN,
    PKG_BENCH_ALLOC_SNAPSHOT,
    PKG_BENCH_ALLOC_PACKAGE,
    PKG_BENCH_ALLOC_ARTIFACT,
    PKG_BENCH_ALLOC_DIAGNOSTIC,
    PKG_BENCH_ALLOC_TARGET,
    PKG_BENCH_ALLOC_DPKG,
    PKG_BENCH_ALLOC_SUPPORT,
    PKG_BENCH_ALLOC_CLASS_COUNT
};

typedef struct pkg_bench_alloc_class_stats {
    uint64_t malloc_calls;
    uint64_t calloc_calls;
    uint64_t realloc_calls;
    uint64_t free_calls;
    uint64_t malloc_bytes_requested;
    uint64_t calloc_bytes_requested;
    uint64_t realloc_bytes_requested;
} pkg_bench_alloc_class_stats;

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
    pkg_bench_alloc_class_stats by_class[PKG_BENCH_ALLOC_CLASS_COUNT];
} pkg_bench_alloc_stats;

void *pkg_bench_malloc_class(size_t size, unsigned int alloc_class);
void *pkg_bench_calloc_class(size_t count, size_t size, unsigned int alloc_class);
void *pkg_bench_realloc_class(void *ptr, size_t size, unsigned int alloc_class);
void pkg_bench_free_class(void *ptr, unsigned int alloc_class);
void pkg_bench_alloc_stats_reset(void);
void pkg_bench_alloc_stats_get(pkg_bench_alloc_stats *out);

#endif

