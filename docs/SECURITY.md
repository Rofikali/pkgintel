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

The implementation must also consider:

- symlink substitution;
- `..` components;
- mount crossings;
- bind mounts;
- magic links;
- `/proc` and `/sys` exposure;
- special files;
- filesystem races.

## Symlink and special-file policy

Symlinks are observed but not recursively followed by default. FIFOs, sockets, device nodes, and other special files must not be opened merely for inspection. Regular-file reads must be explicit and bounded.

## Parser security

Package metadata and filesystem metadata are untrusted input in the current slice. Future APT and ELF parsers will inherit the same boundary.

Every offset, length, count, multiplication, addition, and allocation must be range-checked before use.

Malformed input must produce a diagnostic or controlled error, never an out-of-bounds access, integer overflow, use-after-free, or process crash.

## Resource exhaustion

Resource limits are part of the security contract, not merely performance tuning.

### Implemented in the current v0.1 slice

- package count;
- per-package non-empty package-file record count;
- bounded dpkg status record materialization;
- transactional artifact and diagnostic snapshot mutation.

### Reserved / not yet enforced

- aggregate filesystem file count;
- aggregate directory count;
- recursion depth;
- bytes read;
- individual filesystem file-size limits;
- ELF input size;
- diagnostic/artifact aggregate budgets;
- wall-clock scan time;
- cache-entry limits;
- general allocation/descriptors budgets.

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
