#ifndef PKGINTEL_TARGET_H
#define PKGINTEL_TARGET_H
#include "context.h"
typedef struct pkg_target pkg_target;
typedef enum pkg_target_type { PKG_TARGET_LOCAL=0, PKG_TARGET_ROOTFS=1 } pkg_target_type;
pkg_status pkg_target_local_create(pkg_context *context,pkg_target **out_target);
pkg_status pkg_target_rootfs_create(pkg_context *context,pkg_path root,pkg_target **out_target);
void pkg_target_destroy(pkg_target *target);
/* Legacy constructors remain available until the v0.x API transition is complete. */
pkg_status pkg_target_create_local(pkg_context *context,pkg_target **out_target);
pkg_status pkg_target_create_rootfs(pkg_context *context,const char *root,pkg_target **out_target);
#endif
