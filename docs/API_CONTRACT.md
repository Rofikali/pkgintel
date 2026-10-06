# pkgintel Public C API Contract

Status: v0.1 design baseline. This document is the contract to review before ABI freeze.

## 1. Boundary

The public API is opaque and read-only. The intended flow is:

    context -> target -> scan -> immutable snapshot -> records

Callers do not access internal structs.

## 2. Versioning

Product version and API version are independent. Product 0.1.0 is represented by
PKGINTEL_VERSION_*; API 0.1 is represented by PKGINTEL_API_*.

SONAME remains 0 while the project is pre-1.0. No claim of stable ABI is made yet.

## 3. Status

pkg_status uses explicit numeric values. Values are never renumbered.

A successful scan may still contain warnings, missing artifacts, or partial evidence.
The function return value describes operation status; snapshot contents describe
observation quality.

## 4. Extensible options

Public option structs use struct_size and flags.

- A NULL options pointer means implementation defaults.
- struct_size smaller than the documented minimum is invalid.
- Implementations read only fields covered by struct_size.
- Unknown trailing fields are ignored.
- Unknown flags are rejected with PKG_STATUS_INVALID_ARGUMENT.
- Every future field must document its minimum struct_size.

## 5. Ownership and lifetime

Create functions return owned opaque objects through output parameters.
On failure, output objects are NULL.

Destroy functions accept NULL and do nothing.

Snapshot records and string/path views are borrowed. They remain valid until the
owning snapshot is destroyed. They are not NUL-termination guarantees.

Snapshots are immutable after creation and are intended to support concurrent
read-only access.

## 6. Bytes, text, and paths

Filesystem paths are arbitrary Linux byte sequences. pkg_path therefore carries
an explicit byte pointer and length and is not required to be UTF-8.

pkg_string_view represents text and is length-delimited. Consumers must not assume
NUL termination unless a specific accessor documents it.

NUL bytes are rejected in API path inputs because Linux path components cannot
contain NUL.

## 7. Domain model

Package metadata, local installation state, and filesystem artifacts are separate
concepts. The long-term model is:

    Package -> Installation -> Artifact observations

An installation state is not a package identity.

## 8. Filesystem safety

A target root path is not itself a security boundary. The implementation uses a
root descriptor and descriptor-relative resolution. Traversal outside the target,
magic-link traversal, and unsafe path resolution must fail closed.

Symlink observation must not require following the symlink. Broken, inaccessible,
and missing artifacts are distinct observations.

## 9. Resource bounds

Scans are bounded. Limits are part of the security contract and their unit and scope
must be explicit.

For the current dpkg backend:

- `max_packages` is a per-scan limit on installed package records consumed.
- `max_package_files` is a per-package limit on non-empty package-file records consumed.
- Every consumed non-empty package-file record consumes one budget unit, including a
  malformed path record that cannot be resolved.
- Empty records do not consume the package-file budget.
- A limit is exceeded only when the scanner would consume one more record than the
  configured maximum. Therefore a file containing exactly N non-empty records can
  complete successfully with `max_package_files == N`.
- Exceeding a limit produces `PKG_ERR_RESOURCE_LIMIT` and preserves the partial
  snapshot accumulated before the refused record.

Zero-valued limits use implementation defaults where documented. Per-package limits
are not a substitute for future scan-wide aggregate byte, time, descriptor, or
artifact budgets.

## 10. Diagnostics

Diagnostic codes are stable machine-readable identifiers. Human messages are for
display and must never be parsed by consumers.

Diagnostic severity does not replace operation status.

## 11. Threading

Independent contexts/targets/snapshots may be used concurrently. Immutable snapshots
may be read concurrently. Mutable objects are not implicitly synchronized.

## 12. Compatibility

During the 0.x transition, legacy PKG_OK/PKG_ERR_* names remain aliases for the
new PKG_STATUS_* values. Legacy scan-result accessors remain available while the
snapshot API is introduced.

The public contract must not be declared ABI-stable until visibility, layout,
symbol-versioning/SONAME policy, ABI compatibility tests, and compiler/platform
coverage have been completed.

## 13. Current implementation boundary

The modular API is being introduced before all record types are populated.
Artifact/cache/capability/diagnostic accessors are currently placeholders where
their underlying data model is not yet implemented. They must not be described
as feature-complete.

Next implementation gate: build the real immutable snapshot/artifact model and
security fixtures before claiming the v0.1 API is frozen.
