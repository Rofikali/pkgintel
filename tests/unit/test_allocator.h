#ifndef PKGINTEL_TEST_ALLOCATOR_H
#define PKGINTEL_TEST_ALLOCATOR_H

#include <stddef.h>

void *pkg_test_malloc(size_t size);
void *pkg_test_calloc(size_t count, size_t size);
void *pkg_test_realloc(void *ptr, size_t size);

#endif
