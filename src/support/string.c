#include "internal/pkg_support.h"
#include <stdlib.h>
#include <string.h>

char *pkg_strdup_internal(const char *value) {
    size_t len;
    char *copy;
    if (value == NULL) return NULL;
    len = strlen(value);
    if (len == SIZE_MAX) return NULL;
    copy = malloc(len + 1U);
    if (copy == NULL) return NULL;
    memcpy(copy, value, len + 1U);
    return copy;
}
