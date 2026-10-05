#ifndef PKGINTEL_PKGINTEL_H
#define PKGINTEL_PKGINTEL_H

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32) && defined(PKGINTEL_BUILDING_LIBRARY)
#  define PKGINTEL_API __declspec(dllexport)
#elif defined(_WIN32)
#  define PKGINTEL_API __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
#  define PKGINTEL_API __attribute__((visibility("default")))
#else
#  define PKGINTEL_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pkg_context pkg_context;
typedef struct pkg_target pkg_target;
typedef struct pkg_scan_result pkg_scan_result;

typedef enum pkg_status {
    PKG_OK = 0,
    PKG_ERR_INVALID_ARGUMENT,
    PKG_ERR_NOT_FOUND,
    PKG_ERR_PERMISSION,
    PKG_ERR_IO,
    PKG_ERR_PARSE,
    PKG_ERR_UNSUPPORTED,
    PKG_ERR_CORRUPT,
    PKG_ERR_RESOURCE_LIMIT,
    PKG_ERR_STATE,
    PKG_ERR_INTERNAL
} pkg_status;

typedef enum pkg_target_type {
    PKG_TARGET_LOCAL = 0,
    PKG_TARGET_ROOTFS
} pkg_target_type;

typedef enum pkg_scan_mode {
    PKG_SCAN_FAST = 0,
    PKG_SCAN_NORMAL,
    PKG_SCAN_DEEP
} pkg_scan_mode;

typedef struct pkg_context_options {
    uint64_t max_files;
    uint64_t max_directories;
    uint64_t max_depth;
    uint64_t max_bytes;
    uint64_t max_elf_bytes;
} pkg_context_options;

typedef struct pkg_scan_options {
    pkg_scan_mode mode;
    uint64_t max_packages;
    uint64_t max_package_files;
} pkg_scan_options;

PKGINTEL_API pkg_status pkg_context_create(const pkg_context_options *options, pkg_context **out_context);
PKGINTEL_API void pkg_context_destroy(pkg_context *context);

PKGINTEL_API pkg_status pkg_target_create_local(pkg_context *context, pkg_target **out_target);
PKGINTEL_API pkg_status pkg_target_create_rootfs(pkg_context *context, const char *root, pkg_target **out_target);
PKGINTEL_API void pkg_target_destroy(pkg_target *target);

PKGINTEL_API pkg_status pkg_scan(pkg_context *context, pkg_target *target,
                                  const pkg_scan_options *options, pkg_scan_result **out_result);
PKGINTEL_API void pkg_scan_result_destroy(pkg_scan_result *result);

PKGINTEL_API size_t pkg_scan_result_package_count(const pkg_scan_result *result);
PKGINTEL_API const char *pkg_scan_result_target_root(const pkg_scan_result *result);
PKGINTEL_API const char *pkg_scan_result_package_name(const pkg_scan_result *result, size_t index);
PKGINTEL_API const char *pkg_scan_result_package_version(const pkg_scan_result *result, size_t index);
PKGINTEL_API const char *pkg_scan_result_package_architecture(const pkg_scan_result *result, size_t index);
PKGINTEL_API uint64_t pkg_scan_result_package_installed_size(const pkg_scan_result *result, size_t index);

PKGINTEL_API const char *pkg_status_string(pkg_status status);

#ifdef __cplusplus
}
#endif

#endif
