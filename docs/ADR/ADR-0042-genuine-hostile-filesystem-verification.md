# ADR-0042: Genuine Hostile-Filesystem Security Verification

## Status

Accepted

## Date

2026-10-07

## Context

The target boundary is designed to prevent a scan from resolving paths outside its declared root or across filesystem boundaries. The implementation uses Linux openat2() with RESOLVE_IN_ROOT, RESOLVE_NO_MAGICLINKS, and RESOLVE_NO_XDEV.

Existing unit fixtures already cover ordinary symlinks, broken symlinks, parent traversal, restricted paths, missing paths, and non-directory target roots. Those fixtures are valuable, but they do not prove behavior against a genuine mount boundary or a genuine Linux procfs magic link.

An ordinary symlink is not a procfs magic link. A directory fixture that merely resembles a mount is not a mount boundary. Therefore those cases must not be described as equivalent security evidence.

## Decision

Add a separate privileged Linux security verification target for genuine hostile filesystem objects.

The verification must:

1. create an isolated temporary target root;
2. create a real bind-mounted directory below that root and verify that both target-relative open and metadata observation are denied at the mount boundary;
3. create a real procfs fixture containing /proc/self/fd/*, verify that it is a genuine magic-link object, and verify that target-relative open and metadata observation cannot follow it;
4. remove every temporary mount during normal cleanup;
5. report three states explicitly: PASS, SKIP_UNAVAILABLE, and FAIL;
6. keep the privileged verification separate from ordinary unprivileged CI.

The release/security gate must not convert SKIP_UNAVAILABLE into PASS. A release candidate requires a successful privileged Linux security job, or an explicitly documented environment qualification outside CI. Ordinary CI may continue to run the non-privileged hostile fixtures.

## Important qualification

The production resolver intentionally combines NO_MAGICLINKS with NO_XDEV. A procfs path below the target therefore has two independent reasons it may be denied: crossing into procfs is forbidden, and magic-link resolution is forbidden.

The genuine procfs test proves the externally observable invariant: a procfs magic link cannot be followed or used to escape the target. It does not claim to isolate which individual resolve flag produced the kernel errno.

Isolating individual kernel resolution flags would require a separate syscall experiment and would not, by itself, be evidence about the production resolver.

## Current verification record

The first strict runtime qualification was performed in the intended Windows 11 -> Docker Desktop -> Ubuntu 24.04 workflow. The application was built with `PKGINTEL_REQUIRE_PRIVILEGED_SECURITY_TESTS=ON` and the genuine security test was executed through `sudo`.

The environment reported Ubuntu 24.04.5 LTS on WSL2 kernel `5.15.167.4-microsoft-standard-WSL2`. Root had an effective capability set reported as `0xA80C25FB`, but the container remained under seccomp mode 2 with one active filter. A direct `sudo mount --bind` probe failed with permission denied (exit status 32), and the pkgintel test consequently reported `bind mount unavailable: Operation not permitted`. Strict CTest converted that unavailable result into a test failure.

This is the expected evidence behavior: UID 0 inside a restricted container is not treated as equivalent to a mount-capable verification environment. The release gate remains unsatisfied. The next verification must use a dedicated capable Ubuntu 24.04 container and repeat the unchanged genuine test. A privileged Docker container is the preferred first qualification; capability minimization is a later hardening exercise, not a reason to weaken this test.

## Alternatives rejected

### Fake mount fixture

Rejected. A directory or symlink does not exercise the kernel mount-boundary check.

### Ordinary symlink as a magic-link substitute

Rejected. Linux procfs magic links have kernel-specific resolution semantics.

### Silent skip

Rejected. Silent skips create false security confidence.

### Making normal unit tests privileged

Rejected. Privileged tests have different environmental assumptions and should not make the entire unit suite dependent on mount privileges.

## Consequences

Positive:

- the security gate has evidence from genuine kernel objects;
- ordinary CI remains deterministic and unprivileged;
- environmental limitations become visible instead of hidden;
- the distinction between implementation coverage and environment coverage is documented.

Negative:

- one additional Linux security job is required for release confidence;
- privileged CI can be unavailable on some runners;
- mount cleanup must be defensive because failed cleanup can contaminate a runner.

## Gate interpretation

Until the genuine privileged verification is exercised successfully:

**Security gate: PARTIAL.**

The existing hostile fixtures remain valid evidence for their specific cases, but they must not be generalized into a claim that mount and procfs magic-link containment has been fully verified.
