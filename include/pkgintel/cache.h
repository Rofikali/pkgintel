#ifndef PKGINTEL_CACHE_H
#define PKGINTEL_CACHE_H
#include <stdint.h>
#include "snapshot.h"
typedef enum pkg_cache_entry_state { PKG_CACHE_ENTRY_UNKNOWN=0, PKG_CACHE_ENTRY_PRESENT=1, PKG_CACHE_ENTRY_INCOMPLETE=2, PKG_CACHE_ENTRY_CORRUPT=3 } pkg_cache_entry_state;
pkg_string_view pkg_cache_backend(const pkg_cache *cache);
pkg_path pkg_cache_path(const pkg_cache *cache);
pkg_cache_entry_state pkg_cache_get_state(const pkg_cache *cache);
uint64_t pkg_cache_size_bytes(const pkg_cache *cache);
#endif
