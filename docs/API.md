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

`PKG_ERR_RESOURCE_LIMIT` is not equivalent to a complete successful scan. A caller must inspect the returned status before treating a result as complete.

Permission failures and unavailable package metadata may become diagnostics in later result versions rather than silently disappearing.

## Scan result semantics

`pkg_scan_result_package_installed_size()` exposes the package manager's declared installed size converted from dpkg's KiB unit to bytes. It is **not** the sum of filesystem allocation and is not dependency-closure size.

`pkg_scan_result_package_file_count()` and `pkg_scan_result_package_missing_file_count()` are reserved for the package/file-correlation phase. They currently report zero until that phase is implemented.

## Resource limits

Limits are explicit inputs to scanning. Zero means use the context default. Reaching a limit produces `PKG_ERR_RESOURCE_LIMIT` and must never be represented as a clean complete scan.

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
