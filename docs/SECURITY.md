# Security Model

`pkgintel` is a read-only system inspection engine. Its attack surface is the target environment itself: package databases, filenames, filesystem metadata, caches, and binary formats must all be treated as potentially hostile input.

## Security invariants

1. **No mutation:** scanning must not install, remove, rewrite, chmod, chown, rename, unlink, or otherwise modify target content.
2. **No execution:** discovered programs are data, never commands. No shell, `system()`, `popen()`, implicit interpreter, or executable probing is permitted in normal discovery.
3. **Target containment:** a rootfs target is a security boundary. Target-relative operations must not escape that boundary.
4. **Least privilege:** root is not a normal requirement. Permission denial is evidence, not a reason to request unnecessary privilege.
5. **Fail closed on security decisions:** ambiguous or unverifiable security-sensitive observations must not be reported as verified facts.
6. **Bounded work:** every untrusted input path is subject to file, directory, depth, byte, parser-size, package-count, and time limits.
7. **No unbounded allocation:** counts and lengths are validated before multiplication, addition, allocation, or indexing.
8. **No unsafe path races:** when an operation depends on object identity, use descriptor-relative access and post-open verification rather than `stat(path)` followed by `open(path)`.
9. **No trust in encoding:** Linux paths are arbitrary bytes. UTF-8 conversion is an output concern and must be lossless or explicitly escaped.
10. **Evidence provenance:** important conclusions retain source and confidence so consumers can distinguish package metadata from filesystem observations and heuristics.

## Target boundary

`pkg_target` owns a root directory descriptor after validation. Future filesystem implementations should prefer `openat2()` with an explicit resolution policy where available, with a carefully designed fallback on older kernels.

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

Package metadata, APT metadata, and ELF structures are untrusted input. Every offset, length, count, multiplication, addition, and allocation must be range-checked before use.

Malformed input must produce a diagnostic or controlled error, never an out-of-bounds access, integer overflow, use-after-free, or process crash.

## Resource exhaustion

Limits are part of the security contract, not merely performance tuning. The engine must bound:

- package count;
- package-file count;
- filesystem files/directories;
- recursion depth;
- bytes read;
- individual metadata/file size;
- ELF input size;
- cache entries;
- wall-clock scan time.

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

Fuzzing must run with sanitizers and resource limits.

## Security quality gates

A release candidate requires:

- ASan clean;
- UBSan clean;
- fuzz targets operational;
- hostile fixtures passing;
- no shell/executable execution path in discovery;
- no known target-boundary escape;
- documented privilege requirements;
- documented resource limits;
- review of every parser handling untrusted bytes.
