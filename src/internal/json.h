#ifndef PKGINTEL_INTERNAL_JSON_H
#define PKGINTEL_INTERNAL_JSON_H

#include "pkgintel/status.h"
#include <stdio.h>

struct pkg_snapshot;

pkg_status pkg_json_write(FILE *output, const struct pkg_snapshot *snapshot, pkg_status scan_status);

#endif
