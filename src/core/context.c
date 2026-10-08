#include "internal/pkg_model.h"
#include <stdlib.h>
#include <string.h>
#ifdef PKGINTEL_BENCHMARK_ALLOC_STATS
#include "alloc_probe_macros.h"
#endif

pkg_status pkg_context_create(const pkg_context_options *options, pkg_context **out_context) {
    pkg_context_options normalized = PKG_CONTEXT_OPTIONS_INIT;
    size_t supplied;
    pkg_context *context;
    if (out_context == NULL) return PKG_ERR_INVALID_ARGUMENT;
    *out_context = NULL;
    if (options != NULL) {
        if (options->struct_size < sizeof(uint32_t) * 2U) return PKG_ERR_INVALID_ARGUMENT;
        supplied = options->struct_size;
        if (supplied > sizeof(normalized)) supplied = sizeof(normalized);
        memcpy(&normalized, options, supplied);
        if (normalized.flags != 0U) return PKG_ERR_INVALID_ARGUMENT;
        if (normalized.max_files != 0U || normalized.max_directories != 0U ||
            normalized.max_depth != 0U || normalized.max_bytes != 0U ||
            normalized.max_elf_bytes != 0U) {
            return PKG_ERR_UNSUPPORTED;
        }
    }
    context = calloc(1, sizeof(*context));
    if (context == NULL) return PKG_ERR_INTERNAL;
    context->options = normalized;
    *out_context = context;
    return PKG_OK;
}

void pkg_context_destroy(pkg_context *context) { free(context); }
