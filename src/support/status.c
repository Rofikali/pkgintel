#include "pkgintel/status.h"

const char *pkg_status_string(pkg_status status) {
    switch (status) {
        case PKG_STATUS_OK: return "ok";
        case PKG_STATUS_INVALID_ARGUMENT: return "invalid argument";
        case PKG_STATUS_OUT_OF_MEMORY: return "out of memory";
        case PKG_STATUS_IO_ERROR: return "I/O error";
        case PKG_STATUS_PERMISSION: return "permission denied";
        case PKG_STATUS_NOT_FOUND: return "not found";
        case PKG_STATUS_UNSUPPORTED: return "unsupported";
        case PKG_STATUS_LIMIT_EXCEEDED: return "resource limit";
        case PKG_STATUS_CORRUPT_DATA: return "corrupt data";
        case PKG_STATUS_CANCELLED: return "cancelled";
        case PKG_STATUS_INTERNAL_ERROR: return "internal error";
        default: return "unknown status";
    }
}
