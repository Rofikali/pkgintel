#ifndef PKGINTEL_INTERNAL_BACKEND_H
#define PKGINTEL_INTERNAL_BACKEND_H

#include "pkgintel/pkgintel.h"

pkg_status pkg_dpkg_scan(pkg_context *context, pkg_target *target,
                         const pkg_scan_options *options, struct pkg_snapshot *result);

#endif
