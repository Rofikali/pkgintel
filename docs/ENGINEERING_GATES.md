# Engineering Gates

This document is the release discipline for pkgintel. A feature is not complete because it compiles or works on one developer machine.

## Gate 0 — Business/product and domain truth

Before implementation, record the user/customer problem, measurable outcome, acceptance criteria, non-goals, operating/economic constraints, and failure impact. Then define the domain entities, states, evidence, ownership, and invariants.

## Gate 1 — HLD / LLD design

Before implementation:

- system boundaries and responsibilities are explicit;
- public/private boundary is explicit;
- low-level interfaces define ownership/lifetime and failure semantics;
- resource limits and state transitions are identified;
- target-boundary implications are reviewed.

## Gate 2 — Mathematical/resource and security/evidence model

Before implementation:

- arithmetic and resource bounds are stated;
- complexity and capacity assumptions are explicit;
- threat model and trust assumptions are reviewed;
- evidence strength required for each security claim is defined;
- fail-closed behavior is specified.

## Gate 3 — Build

Required configurations:

- GCC + Debug;
- GCC + Release;
- Clang + Debug;
- Clang + Release;
- Clang + ASan/UBSan.

No new warning should be accepted casually.

## Gate 4 — Correctness

Every feature gets deterministic unit tests and, where applicable, real Ubuntu/Debian integration fixtures.

Tests must cover:

- happy path;
- missing data;
- malformed data;
- permission failure;
- resource limit;
- empty input;
- boundary values;
- ownership/lifetime behavior.

## Gate 5 — Security

For system-facing code:

- no uncontrolled process execution;
- no target-root escape;
- no unbounded parser allocation;
- integer overflow checked;
- symlink policy tested;
- special-file policy tested;
- hostile input fixture added;
- fuzz target considered or added.

For filesystem confinement claims that depend on Linux kernel namespace/VFS behavior, the verification level must match the claim:

- ordinary CI fixtures prove deterministic application behavior without requiring privilege;
- genuine mount-boundary tests must use a real mount or bind mount, not a simulated directory fixture;
- genuine magic-link tests must use a real procfs-style magic link, not an ordinary symlink;
- privileged verification has an explicit three-state result: **PASS**, **SKIP_UNAVAILABLE**, or **FAIL**;
- **SKIP_UNAVAILABLE is not PASS** and cannot satisfy the release/security gate;
- a skipped privileged environment is an infrastructure-qualification gap, while a failed genuine security test is a security defect;
- the target-relative observation path and metadata/lstat path must both be verified when both implement the security boundary.

## Gate 6 — Performance

Measure before optimizing. Record:

- wall time;
- CPU time;
- peak RSS;
- files examined;
- bytes read;
- package count;
- diagnostics count.

Performance claims require a reproducible fixture and environment.

## Gate 7 — API/ABI

Public C API changes require:

- ownership review;
- error semantics review;
- thread-safety review;
- symbol visibility review;
- compatibility impact review.

v0.1 is not ABI-stable. ABI stability is a release milestone, not an assumption.

## Gate 8 — Documentation

Behavior changes require documentation updates. At minimum, update the relevant architecture, API, security, and CLI contracts.

## Gate 9 — Real runtime and release

Where a claim depends on the real Linux kernel/VFS/namespace/capability/platform, execute the actual runtime verification after implementation, tests, and documentation. Record the host/container image, kernel, UID, capabilities, namespaces, exact command, and result.

### Required privileged verification environment

For pkgintel's genuine mount-boundary and procfs magic-link tests, the verification environment must be capable of performing the Linux mount operations used by the test. The intended developer/release topology is:

~~~
Windows 11 host
  -> Docker Desktop
  -> privileged Ubuntu 24.04 verification container
  -> pkgintel
~~~

The normal development container must not be made privileged merely for convenience. Privilege is an explicit property of the dedicated security-verification environment. On Docker Desktop, the preferred first attempt is a dedicated container with `--privileged`; a narrower `CAP_SYS_ADMIN` plus an appropriate seccomp policy may be used only if the complete test still passes and the resulting capability set is recorded.

A `sudo` shell inside an already restricted container is not equivalent to a privileged container. UID 0 can still lack the kernel capabilities or syscall permissions required for mount operations. Therefore, if the test reports `Operation not permitted`, inspect the container's effective/permitted capabilities, seccomp policy, user namespace, and mount namespace rather than weakening the test.

The genuine security gate must be executed in that capable environment. Do not replace real mounts with ordinary directories or ordinary symlinks, and do not change the test to turn an unavailable mount operation into a successful assertion.

A release candidate requires all applicable gates to pass and a written record of known limitations. `SKIP_UNAVAILABLE` is evidence that the environment was insufficient; it is never evidence that the security property passed.
