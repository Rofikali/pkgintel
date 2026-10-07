#ifndef PKGINTEL_SCAN_H
#define PKGINTEL_SCAN_H
#include "export.h"
#include <stdint.h>
#include "target.h"
#include "snapshot.h"
typedef struct pkg_scan_options {
 uint32_t struct_size; uint32_t flags;
 uint64_t max_files; uint64_t max_directories; uint64_t max_file_bytes; uint64_t max_total_bytes; uint64_t max_duration_ms;
 uint64_t max_packages; uint64_t max_package_files;
} pkg_scan_options;
/* Reserved for future slices; nonzero use is rejected with PKG_ERR_UNSUPPORTED in v0.1. */
#define PKG_SCAN_INCLUDE_ELF          (1u<<0)
#define PKG_SCAN_INCLUDE_CACHES       (1u<<1)
#define PKG_SCAN_INCLUDE_CAPABILITIES (1u<<2)
/* Implemented in v0.1: correlate installed packages with selected package-file records. */
#define PKG_SCAN_CORRELATE_FILES      (1u<<3)
#define PKG_SCAN_OPTIONS_INIT { (uint32_t)sizeof(pkg_scan_options),0u,0u,0u,0u,0u,0u,0u,0u }
PKGINTEL_API pkg_status pkg_scan(pkg_context *context,pkg_target *target,const pkg_scan_options *options,pkg_snapshot **out_snapshot);
#endif
