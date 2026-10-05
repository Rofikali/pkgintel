#ifndef PKGINTEL_STATUS_H
#define PKGINTEL_STATUS_H
#include <stddef.h>
#include <stdint.h>
typedef enum pkg_status {
    PKG_STATUS_OK = 0,
    PKG_STATUS_INVALID_ARGUMENT = 1,
    PKG_STATUS_OUT_OF_MEMORY = 2,
    PKG_STATUS_IO_ERROR = 3,
    PKG_STATUS_PERMISSION = 4,
    PKG_STATUS_NOT_FOUND = 5,
    PKG_STATUS_UNSUPPORTED = 6,
    PKG_STATUS_LIMIT_EXCEEDED = 7,
    PKG_STATUS_CORRUPT_DATA = 8,
    PKG_STATUS_CANCELLED = 9,
    PKG_STATUS_INTERNAL_ERROR = 10
} pkg_status;
typedef struct pkg_bytes { const void *data; size_t size; } pkg_bytes;
typedef struct pkg_string_view { const char *data; size_t size; } pkg_string_view;
typedef struct pkg_path { const unsigned char *data; size_t size; } pkg_path;
#endif
