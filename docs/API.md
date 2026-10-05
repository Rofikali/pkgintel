# Public API Contract (v0.1)

The public API is intentionally small. `pkg_context`, `pkg_target`, and `pkg_scan_result` are opaque.

## Ownership

- `pkg_context_create` returns an owned context; caller destroys it with `pkg_context_destroy`.
- `pkg_target_create_*` returns an owned target; caller destroys it with `pkg_target_destroy`.
- `pkg_scan` returns an owned result through `out_result`; caller destroys it with `pkg_scan_result_destroy`.
- Accessor-returned strings are borrowed, immutable, and valid while their owning object remains alive and unmodified.
- No returned pointer may be freed by the caller.

## Error model

Functions return `pkg_status`. `PKG_OK` is zero. NULL output parameters are rejected. Invalid object relationships return `PKG_ERR_STATE`.

## ABI policy

v0.1 is **not yet declared ABI-stable**. Before v1.0 the project must add:

- symbol visibility/versioning;
- ABI compatibility tests;
- supported compiler/platform matrix;
- struct layout compatibility policy;
- semantic versioning policy;
- deprecation policy.

The public ABI uses fixed-width integers where sizes cross the boundary and does not expose internal allocation types.

## Threading

Independent contexts may be used concurrently. A context is not guaranteed safe for concurrent mutation. v0.1 does not expose mutable shared state or a parallel scan API.

## Strings

Linux filesystem paths are arbitrary byte sequences and are not guaranteed to be UTF-8. The future JSON serializer must define an explicit lossless representation for invalid UTF-8. Public APIs must document whether a string is filesystem bytes or UTF-8 text.
