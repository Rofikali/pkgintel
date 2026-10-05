# Security Model

pkgintel treats package metadata, filesystem metadata, cache metadata, filenames, and ELF files as potentially hostile input.

## Mandatory rules

- Read-only operation by default.
- Never invoke discovered executables for identification.
- Never use a shell for normal discovery.
- Never assume filenames are UTF-8.
- Do not follow symlinks recursively by default.
- Bound files, directories, recursion depth, bytes, ELF input size, and scan time.
- Check every parser offset/count/size calculation before arithmetic or memory access.
- Convert permission failures into diagnostics where possible instead of silently dropping evidence.
- Prefer descriptor-relative filesystem operations for operations where path races matter.

## Target boundary

A rootfs target is a security boundary. A future implementation must use `openat2()` with an appropriate resolution policy where available, and a carefully designed fallback on systems without it. A plain string concatenation of target root + untrusted path is not considered a sufficient security design.

## TOCTOU

The design must avoid the pattern:

```text
stat(path)
...
open(path)
```

when a security decision depends on the two observations referring to the same object. Descriptor-relative operations and post-open verification are preferred.

## Privileges

Root is not a normal requirement. Permission-denied results are first-class observations. The project must not encourage running the scanner with unnecessary privileges.

## Fuzzing

ELF parsing and package/cache metadata parsing must have isolated fuzz targets before they are considered production-grade.
