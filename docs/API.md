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

`pkg_scan_result_package_file_count()` reports the number of non-empty package-file records consumed for the package. `pkg_scan_result_package_missing_file_count()` reports records whose absolute paths could not be found. `pkg_scan_result_package_invalid_path_count()` reports malformed or otherwise unverifiable path records counted by the current dpkg correlation policy.

## Resource limits

Limits are explicit inputs to scanning. For the current dpkg backend, `max_package_files` is a **per-package** limit on **non-empty package-file records**, not on successfully resolved artifacts. Every non-empty record consumes one unit, including malformed records; empty records do not consume a unit.

`max_packages` is a **per-scan** limit on installed package records consumed.

A configured maximum is not itself an error. `PKG_ERR_RESOURCE_LIMIT` means the scanner refused to consume the next record because doing so would exceed an implemented budget. The returned snapshot remains valid and contains observations accumulated before the refused record.

The scan-option fields `max_files`, `max_directories`, `max_file_bytes`, `max_total_bytes`, and `max_duration_ms` are reserved for future aggregate resource controls. They are not enforced in v0.1; non-zero values are rejected with `PKG_ERR_UNSUPPORTED` so the API never silently presents an unenforced security control.

The context-option limits `max_files`, `max_directories`, `max_depth`, `max_bytes`, and `max_elf_bytes` are likewise reserved in v0.1. Non-zero values are rejected with `PKG_ERR_UNSUPPORTED` because these controls are not yet enforced.

Package-file record length is a separate resource dimension. The implementation must bound record materialization independently of `max_package_files`.

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
