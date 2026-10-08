# ADR-0038: Target Root and Mount Containment

- Status: Accepted
- Date: 2026-10-07

## Context

A rootfs target is a security boundary. `RESOLVE_IN_ROOT` prevents ordinary absolute-path and `..` traversal from escaping the directory descriptor, but mount-point and bind-mount crossings can still expose objects outside the intended filesystem image. The target root itself can also be a symlink if opened without `O_NOFOLLOW`, making the caller's boundary ambiguous.

## Decision

For Linux target-relative filesystem operations:

1. Open the target root directory with `O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC`.
2. Resolve target-relative paths with `openat2()` using:
   - `RESOLVE_IN_ROOT`
   - `RESOLVE_NO_MAGICLINKS`
   - `RESOLVE_NO_XDEV`
3. Treat the supplied target root as a real directory boundary, not a symlink alias.
4. Observe symlinks with `O_PATH | O_NOFOLLOW`; do not recursively follow them for artifact identity.
5. Keep special files out of normal content reads.

## Security rationale

This gives three independent containment properties:

- lexical traversal cannot escape the root;
- magic links cannot redirect resolution through special kernel paths;
- mount and bind-mount crossings cannot silently change the filesystem boundary.

Rejecting a symlinked target root avoids silently changing what the caller intended to inspect.

## Consequences

A rootfs path that is itself a symlink is rejected by the target-opening operation. Target-relative access that would cross a mount point is rejected rather than silently traversing into another filesystem.

This is intentionally fail-closed. Callers that need a different mount policy must receive an explicit future API decision rather than inheriting a weaker default.

## Test gate

The security suite must cover at least:

- absolute paths;
- `..` traversal;
- in-target symlinks;
- broken symlinks;
- symlinks targeting outside the root;
- magic-link behavior;
- symlinked target roots;
- special files;
- permission-denied paths;
- mount/bind-mount crossing where the test environment permits creating them.

A successful scan alone is not evidence that target containment is correct; hostile fixtures must verify the boundary.
