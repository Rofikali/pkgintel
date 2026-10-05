#ifndef PKGINTEL_CONTEXT_H
#define PKGINTEL_CONTEXT_H
#include <stdint.h>
#include "status.h"
typedef struct pkg_context pkg_context;
typedef struct pkg_context_options {
 uint32_t struct_size; uint32_t flags;
 uint64_t max_files; uint64_t max_directories; uint64_t max_depth; uint64_t max_bytes; uint64_t max_elf_bytes;
} pkg_context_options;
#define PKG_CONTEXT_OPTIONS_INIT { (uint32_t)sizeof(pkg_context_options),0u,0u,0u,0u,0u,0u }
pkg_status pkg_context_create(const pkg_context_options *options,pkg_context **out_context);
void pkg_context_destroy(pkg_context *context);
#endif
