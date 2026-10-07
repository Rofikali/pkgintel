# Security Model

`pkgintel` is a read-only system inspection engine. Its attack surface is the target environment itself: package databases, filenames, filesystem metadata, and binary formats must all be treated as potentially hostile input.

## Security invariants

1. **No mutation:** scanning must not install, remove, rewrite, chmod, chown, rename, unlink, or otherwise modify target content.
2. **No execution:** discovered programs are data, never commands. No shell, `system()`, `popen()`, implicit interpreter, or executable probing is permitted in normal discovery.
3. **Target containment:** a rootfs target is a security boundary. Target-relative operations must not escape that boundary.
4. **Least privilege:** root is not a normal requirement. Permission denial is evidence, not a reason to request unnecessary privilege.
5. **Fail closed on security decisions:** ambiguous or unverifiable security-sensitive observations must not be reported as verified facts.
6. **Bounded work:** implemented untrusted-input paths are subject to explicit file/record, parser-size, package-count, and allocation limits. Aggregate filesystem/depth/time controls remain planned.
7. **No unbounded allocation:** counts and lengths are validated before multiplication, addition, allocation, or indexing.
8. **No unsafe path races:** when an operation depends on object identity, use descriptor-relative access and post-open verification rather than `stat(path)` followed by `open(path)`.
9. **No trust in encoding:** Linux paths are arbitrary bytes. UTF-8 conversion is an output concern and must be lossless or explicitly escaped.
10. **Evidence provenance:** important conclusions retain source and confidence so consumers can distinguish package metadata from filesystem observations and heuristics.

## Target boundary

`pkg_target` owns a root directory descriptor after validation. The Linux implementation uses `openat2()` with an explicit resolution policy where supported by the current platform.

A plain string concatenation of target root + untrusted path is not sufficient for security-sensitive operations.

The implementation must enforce the following boundary conditions:

- symlink substitution must not escape the target;
- `..` components must remain confined to the target root;
- mount-point and bind-mount crossings are rejected for target-relative operations;
- magic links are rejected;
- the target root itself must be an actual directory, not a symlink;
- `/proc` and `/sys` exposure must not weaken the target boundary;
- special files must not be opened merely for inspection;
- filesystem races must be handled with descriptor-relative operations and appropriate post-open verification.

## Symlink and special-file policy

Symlinks are observed but not recursively followed by default. FIFOs, sockets, device nodes, and other special files must not be opened merely for inspection. Regular-file reads must be explicit and bounded.

## Parser security

Package metadata and filesystem metadata are untrusted input in the current slice. Future APT and ELF parsers will inherit the same boundary.

Every offset, length, count, multiplication, addition, and allocation must be range-checked before use.

### Dpkg record boundary

The v0.1 dpkg parser uses a hard maximum of **65,536 bytes of record content before the LF delimiter**. A 65,536-byte record is accepted; the next byte of record content is rejected as `PKG_ERR_RESOURCE_LIMIT`. EOF without a final LF is accepted as a complete record. CRLF input is accepted, with the CR participating in the bounded record content and removed during normalization. This boundary applies before semantic field interpretation and therefore limits both valid and malformed records.

Malformed input must produce a diagnostic or controlled error, never an out-of-bounds access, integer overflow, use-after-free, or process crash.

## Resource exhaustion

Resource limits are part of the security contract, not merely performance tuning.

### Implemented in the current v0.1 slice

- package count;
- per-package non-empty package-file record count;
- bounded dpkg status and package-file record materialization;
- aggregate snapshot artifact ceiling of 1,000,000 records;
- aggregate diagnostic ceiling of 100,000 records;
- transactional artifact and diagnostic snapshot mutation.

### Reserved / not yet enforced

- aggregate filesystem file count;
- aggregate directory count;
- recursion depth;
- bytes read;
- individual filesystem file-size limits;
- ELF input size;
- wall-clock scan time;
- cache-entry limits;
- general descriptor budgets and total non-snapshot allocation budgets.

These controls must not be presented as enforced until accounting and tests exist. Public option fields for unsupported limits are rejected with `PKG_ERR_UNSUPPORTED` rather than silently ignored. Reserved scan feature flags for ELF, cache, and capability analysis are likewise rejected with `PKG_ERR_UNSUPPORTED` in v0.1; the engine never accepts a feature request and silently ignores it.

A limit reached during scanning must be distinguishable from a clean complete scan.

## Privileges and environment

The CLI must not automatically escalate privileges. It must not inherit dangerous environment behavior such as executing commands based on `PATH` discovery. The engine should avoid dependence on the caller's shell, locale, current working directory, or mutable environment variables for security decisions.

## Error and information policy

Diagnostics should identify the failing subsystem and stable error class without leaking secrets unnecessarily. Raw environment paths are not to be treated as safe log identifiers. Future telemetry must use redaction and bounded fields.

## TOCTOU

Avoid:

```text
stat(path)
...
open(path)
```

when the decision depends on both observations referring to the same object. Prefer descriptor-relative operations and verification after opening.

## Fuzzing

Before production-grade release, isolated fuzz targets are required for:

- dpkg status parsing;
- dpkg file-list parsing;
- APT metadata parsing;
- ELF parsing;
- path normalization/resolution;
- JSON serialization.

Fuzzing must run with sanitizers and resource limits. Only the first two parser targets correspond to currently implemented parser code.

## Security quality gates

A release candidate requires:

- ASan clean;
- UBSan clean;
- fuzz targets operational;
- hostile fixtures passing;
- no shell/executable execution path in discovery;
- no known target-boundary escape;
- documented privilege requirements;
- documented **implemented** resource limits;
- review of every parser handling untrusted bytes.

A future feature must not weaken these guarantees merely to expose an earlier public API.

## Dpkg status-state handling

The `Status:` field is treated as untrusted structured input. Its three tokens are interpreted independently: desired action is not used as the installation-state classifier, the error flag can force a broken/reinstallation-required package into `PARTIAL`, and the actual state token determines installed/removed/transitional classification.

Unknown or malformed status combinations are represented as `PKG_INSTALLATION_UNKNOWN` and accompanied by a diagnostic. They are never silently promoted to `INSTALLED`. This is important because a parser that mistakes a desired action or an unfamiliar state for a healthy installation would turn malformed metadata into a false security conclusion.

The current public state mapping is intentionally narrower than dpkg's complete state vocabulary. Extending it is a semantic/API change and requires tests, documentation, and ABI review together.

## Performance-security interaction

Performance optimizations must not weaken the security boundary. In particular, hand-written assembly is not part of v0.1. Any future architecture-specific optimization must pass the evidence gate in ADR-0040 and retain the same bounds, ownership, error, and containment guarantees as the portable implementation.
