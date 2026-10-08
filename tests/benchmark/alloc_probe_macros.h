#ifndef PKGINTEL_BENCH_ALLOC_MACROS_H
#define PKGINTEL_BENCH_ALLOC_MACROS_H

#include "alloc_stats.h"

#define malloc(size) pkg_bench_malloc_class((size), PKGINTEL_BENCH_ALLOC_CLASS)
#define calloc(count, size) pkg_bench_calloc_class((count), (size), PKGINTEL_BENCH_ALLOC_CLASS)
#define realloc(ptr, size) pkg_bench_realloc_class((ptr), (size), PKGINTEL_BENCH_ALLOC_CLASS)
#define free(ptr) pkg_bench_free_class((ptr), PKGINTEL_BENCH_ALLOC_CLASS)

#endif
