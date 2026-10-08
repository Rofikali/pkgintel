# Engineering Status

This document is the entry point for understanding what has already been implemented, what has been verified, and what remains before a v0.1 release candidate.

## Current branch model

The repository currently uses three relevant branches:

```
origin/main
    |
    v
origin/codex/p0-foundation
    |
    v
origin/codex/p0-module-architecture  <-- current P0 integration branch
```

At the current P0 checkpoint:

- `origin/main` is an ancestor of `origin/codex/p0-foundation`.
- `origin/codex/p0-foundation` is an ancestor of `origin/codex/p0-module-architecture`.
- Therefore there are no foundation-only commits missing from the module-architecture branch.
- New P0 work must first be checked against the existing branch history to avoid reimplementing controls that already exist.

## What is already implemented

### Public API / ABI

- Public C API has explicit export/visibility handling.
- Installed-package consumer coverage exists.
- Unsupported feature flags are rejected rather than silently ignored.
- Snapshot mutation follows transactional commit semantics.
- v0.1 is not declared ABI-stable.

### Dpkg parser and semantics

- Dpkg status records have an explicit bounded record contract.
- Installation state is modeled separately from desired action.
- Unknown/malformed state combinations produce unknown state plus diagnostics.
- Package-file correlation is optional for explicit scan options.
- Package names are validated before they influence target-internal Dpkg paths.
- Invalid package names are reported diagnostically and skipped for correlation.
- Aggregate correlation resource exhaustion stops the scan with a resource-limit status.

### Resource governance

Implemented controls include:

- package count bounds;
- per-package package-file bounds;
- bounded Dpkg record materialization;
- aggregate artifact ceiling;
- aggregate diagnostic ceiling;
- aggregate owned-string accounting;
- bounded snapshot-array growth;
- transactional resource-budget mutation;
- allocator fault-injection coverage through the public scan contract.

Do not add another resource-control implementation without first auditing the existing budget primitives, wrappers, tests, and ADRs.

### Filesystem security

Implemented design controls include:

- target-root validation;
- root symlink rejection;
- target mount containment;
- absolute-path containment checks;
- symlink/magic-link policy;
- descriptor-relative target observation;
- controlled special-file handling.

The repository also contains genuine Linux filesystem security tests for bind mounts and procfs-style magic links.

## Verification evidence

The following evidence has been produced on Ubuntu 24.04 in the Docker development environment.

### Verified

- Git branch synchronization: PASS
- Working tree clean at the verification checkpoint: PASS
- Release CMake configuration: PASS
- Release build: PASS
- Unit CTest suite: PASS
- ASAN + UBSAN build: PASS
- ASAN + UBSAN unit suite: PASS
- Dpkg invalid-package-name regression: PASS

### Not yet satisfied

- Genuine privileged mount-boundary runtime verification: **SKIP_UNAVAILABLE**
- Fuzzing release gate: pending current operational evidence
- Current performance evidence: pending current reproducible benchmark run
- Full compiler/configuration matrix: pending audit against the release gate
- Final P0 sign-off: pending all applicable gates

A privileged security test returning 77 means the environment cannot provide the kernel capability required by the test. It is not a security pass.

## Genuine filesystem security test

The test is intentionally exposed through the public pkgintel API. It attempts to exercise:

1. a real bind mount crossing;
2. a real procfs mount;
3. a procfs-style magic-link path;
4. the normal target-relative scan path.

The Docker environment currently reports:

```
SKIP: bind mount unavailable: Operation not permitted
```

Therefore this environment is insufficient for the genuine mount-boundary release gate.

The correct next step is to run the same test in a Linux environment where the required mount capability is intentionally granted, or in a dedicated privileged CI/security runner. Do not weaken the production security policy merely to make the test pass in an unprivileged container.

## Evidence classification

Use these categories for every P0 claim:

| Evidence | Meaning |
|---|---|
| Source | The implementation exists in code. |
| Unit/integration | Deterministic application behavior is tested. |
| Sanitizer/fuzz | Memory/undefined behavior or malformed-input robustness has evidence. |
| Real runtime | The behavior has been verified against the actual Linux/OS primitive. |
| Benchmark | Resource/performance claim is measured in a reproducible environment. |
| Production-like | The complete release configuration and operational boundary has been exercised. |

A lower evidence level must not be silently promoted to a higher one.

## Change discipline for future engineers

Before modifying code:

1. Read this document and the relevant architecture/security/API documents.
2. Inspect branch ancestry and recent commits.
3. Search for an existing implementation, test, or ADR.
4. Determine whether the gap is implementation debt or evidence debt.
5. Prefer the smallest change that closes the actual gap.
6. Preserve the public/private boundary.
7. Add or update documentation when behavior, contracts, security claims, or verification procedures change.
8. Run the narrowest relevant test first, then the applicable release gates.
9. Record real-environment limitations explicitly.
10. Never convert a skipped security test into a pass by changing the test merely to satisfy CI.

## Current next sequence

1. Qualify the genuine filesystem security environment and obtain PASS evidence.
2. Audit and operationalize the required fuzz targets for the implemented parsers.
3. Run the current reproducible performance benchmark and record results.
4. Audit the required compiler/configuration matrix.
5. Review API/ABI, architecture, resource, and security evidence together.
6. Produce a written P0 sign-off or an explicit list of remaining blockers.

## Related documents

- `docs/ENGINEERING_PRINCIPLES.md`
- `docs/ENGINEERING_GATES.md`
- `docs/ARCHITECTURE.md`
- `docs/API_CONTRACT.md`
- `docs/SECURITY.md`
- `docs/PERFORMANCE.md`
- `docs/HLD.md`
- `docs/LLD.md`
- `docs/ADR/`

## Rule

> If a future engineer cannot determine what is implemented, what is merely planned, what has actually been verified, and what remains blocked by reading the repository documentation, the engineering documentation is incomplete.
