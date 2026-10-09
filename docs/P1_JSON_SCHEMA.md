# P1 JSON Schema v1 — pkgintel.scan

## Status

**Frozen for P1 implementation.** This is the machine-readable contract for pkgintel scan --json.

The schema version is independent of the native C SONAME/API version and the CLI version.

## Document shape

```json
{
  "schema": {"name": "pkgintel.scan", "version": 1},
  "producer": {"name": "pkgintel", "version": "0.1.0"},
  "scan": {"status": "complete"},
  "packages": [],
  "diagnostics": []
}
```

## Field contract

### schema
- name: fixed string pkgintel.scan.
- version: integer 1 for this contract.

### producer
- name: fixed string pkgintel.
- version: semantic product/library version emitted by the build.

### scan
status is the operation outcome supplied by pkg_scan:

| Value | Meaning |
|---|---|
| complete | pkg_scan returned PKG_OK. |
| resource_limit | pkg_scan returned PKG_ERR_RESOURCE_LIMIT; the snapshot is usable but observation was bounded/truncated. |

Other scan failures do not produce a JSON document because the CLI reports the failure on stderr and exits non-zero.

### packages
Each package contains:
- name: lossless byte string object.
- version: lossless byte string object.
- architecture: lossless byte string object.
- installation_state: unknown, installed, partial, or removed.
- consistency: unknown, consistent, inconsistent, missing_artifact, broken_link, permission_denied, unexpected_artifact, or unverifiable.
- installed_size_bytes: unsigned integer.
- file_count: number of correlated artifact records in this package.
- artifacts: ordered array of artifact observations owned by this package.

The package array is canonically ordered by the scan implementation: package name, architecture, version. The serializer does not sort packages again.

### artifacts
Each artifact contains:
- path: lossless byte string object.
- kind: unknown, regular, directory, symlink, or other.
- state: unknown, present, missing, broken_link, permission_denied, or unverifiable.
- logical_size_bytes: unsigned integer.
- logical_size_available: boolean.
- allocated_size_bytes: unsigned integer.
- allocated_size_available: boolean.

Artifact ordering is the immutable snapshot order produced by package-file correlation. The serializer does not re-resolve or re-open the path.

### diagnostics
Each diagnostic contains:
- code: lossless byte string object.
- message: lossless byte string object.
- status: stable status identifier.
- severity: info, notice, warning, error, or fatal.
- evidence_source: unknown, dpkg, apt, filesystem, elf, path, or heuristic.

Diagnostics preserve snapshot emission order.

## Lossless byte strings
All values represented by the C API as byte-oriented strings are encoded as:

```json
{"encoding":"base64","data":"..."}
```

data uses standard RFC 4648 Base64 with the standard alphabet and required = padding.

This applies to package names, versions, architectures, artifact paths, diagnostic codes, and diagnostic messages.

This deliberate v1 choice avoids locale-dependent conversion and prevents invalid/non-UTF-8 Linux bytes from being replaced or emitted as invalid JSON.

## Status identifiers
| C status | JSON identifier |
|---|---|
| PKG_OK | ok |
| PKG_ERR_INVALID_ARGUMENT | invalid_argument |
| PKG_STATUS_INTERNAL_ERROR | internal_error |
| PKG_ERR_IO | io_error |
| PKG_ERR_PERMISSION | permission |
| PKG_ERR_NOT_FOUND | not_found |
| PKG_ERR_UNSUPPORTED | unsupported |
| PKG_ERR_RESOURCE_LIMIT | resource_limit |
| PKG_ERR_PARSE | corrupt_data |
| PKG_STATUS_CANCELLED | cancelled |
| PKG_ERR_INTERNAL | internal_error (legacy alias) |

Unknown internal enum values are treated as serializer failure rather than silently relabeled.

## Determinism
For an identical immutable snapshot and scan outcome, serialization is byte-for-byte deterministic.
Object member order is fixed by the implementation. Array order is fixed by the domain/snapshot contract described above.
No hash-table iteration or unordered collection participates in serialization.

## Resource model
The serializer uses a streaming writer and does not build an intermediate JSON DOM.
Additional serializer memory is O(1), excluding the standard-library FILE buffering.
Base64 output expansion follows the RFC 4648 padded representation: 4 * ceil(N / 3) output characters for N input bytes.
The implementation never performs unchecked size multiplication to allocate an output-sized buffer.

## Failure semantics
Success means a complete JSON document was written and flushed.
A write/flush/encoding/internal failure returns non-success. The output stream may contain a partial document, but the CLI exits non-zero and must not claim the output is a complete JSON result.
The serializer does not own or mutate the snapshot.

## Compatibility rules
- Adding an optional object field is additive-compatible.
- Removing or renaming a field is breaking.
- Changing a field's JSON type is breaking.
- Changing enum identifiers is breaking.
- Changing byte-string encoding is breaking.
- Reordering object members is not semantically breaking, but v1 promises deterministic bytes, so intentional order changes require a schema/contract review.
- Changing array ordering is a contract change and requires review.
- Breaking changes require a new schema major version.
- The native SONAME/API version and schema version evolve independently.

## Security boundary
The serializer does not execute discovered values, interpret shell syntax, resolve or reopen target paths, follow target links, or modify the target. It serializes only the already-materialized snapshot.
JSON generation follows RFC 8259 and Base64 follows RFC 4648.