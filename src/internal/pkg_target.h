#ifndef PKGINTEL_INTERNAL_TARGET_H
#define PKGINTEL_INTERNAL_TARGET_H

#include "pkgintel/pkgintel.h"
#include <sys/stat.h>

pkg_status pkg_target_open_root(pkg_target *target);
int pkg_target_open_path(const pkg_target *target, const char *path, int flags);
int pkg_target_lstat_path(const pkg_target *target, const char *path, struct stat *st);

#endif
