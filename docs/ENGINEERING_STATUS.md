
### Dedicated security-verification runtime created

A dedicated Docker Compose security-verification service is now part of the repository so privileged kernel/VFS evidence does not require making the normal development service privileged.

- Normal service: `pkgintel`, non-root developer runtime, existing `SYS_PTRACE` capability and unconfined seccomp policy.
- Security service: `pkgintel-security`, Compose `security` profile, container name `pkgintel-security-verify`, `privileged: true`, `user: root`.
- The security service reuses the existing Compose service definition through `extends`, avoiding duplicated build, volume, environment, and working-directory configuration.
- `build-security/` is now ignored as generated verification-build state.
- The service has been successfully created and started through the repository's Compose configuration on the Windows 11 -> Docker Desktop workflow.
- At the latest checkpoint, the interactive shell inside `pkgintel-security-verify` is confirmed to be UID 0. Capability/seccomp and genuine mount probes are intentionally the next evidence step; the latest privileged runtime is fully qualified: UID 0, broad effective capabilities, identity UID mapping, a real mount namespace, and a successful real bind-mount probe were observed.
- The unchanged pkgintel genuine security test passed in strict mode: `pkgintel.security.mounts` 1/1.
- The complete security-runtime CTest suite passed: `pkgintel.unit.core` and `pkgintel.security.mounts`, 2/2.
- This closes the P7 hostile-filesystem runtime verification gate for the currently implemented security surface.

This separation is intentional: normal development remains least-privileged, while privileged authority is granted only to the explicit security-verification runtime.

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
- Developer Debug configure with `PKGINTEL_BUILD_TESTS=ON`: PASS
- Developer Debug build: PASS
- Developer CTest unit suite: PASS (1/1 executed test passed; privileged security test was unavailable)
- CTest test-name audit: `-R scan` is not a valid selector for the current registered test name; the scan coverage is part of `pkgintel.unit.core`.
- Working tree clean at the verification checkpoint: PASS
- Release CMake configuration: PASS
- Release build: PASS
- Unit CTest suite: PASS
- ASAN + UBSAN build: PASS
- ASAN + UBSAN unit suite: PASS
- Dpkg invalid-package-name regression: PASS

### Not yet satisfied

- Fuzzing safety execution gate: PASS (both Clang/libFuzzer targets completed bounded ASan/UBSan runs without sanitizer findings)
- Fuzzing coverage/effectiveness gate: pending re-run after explicit SanitizerCoverage instrumentation fix; the prior runs emitted `no interesting inputs` and retained a one-entry corpus, so they are not accepted as coverage evidence
- Current performance evidence: pending current reproducible benchmark run
- Full compiler/configuration matrix: pending audit against the release gate
- Final P0 sign-off: pending all applicable gates

The earlier unprivileged runtime qualification remains historical evidence that the normal developer container cannot exercise the genuine mount test. The dedicated security runtime now supplies the required capable environment, and strict verification has passed there.

The ordinary developer CTest suite intentionally retains the three-state SKIP behavior so an unprivileged workstation does not falsely claim the security property.
A privileged security test returning 77 means the environment cannot provide the kernel capability required by the test. It is not a security pass.

## Genuine filesystem security test

**P7 status: PASS for the implemented hostile-filesystem verification surface.**

The test is intentionally exposed through the public pkgintel API. It attempts to exercise:

1. a real bind mount crossing;
2. a real procfs mount;
3. a procfs-style magic-link path;
4. the normal target-relative scan path.

The ordinary developer container previously reported:

```
SKIP: bind mount unavailable: Operation not permitted
```

That environment remains intentionally unprivileged and is not the release-security runtime. The dedicated `pkgintel-security` service provides the required kernel capability without changing the normal development service.

The repository now has a strict qualification mode:

```bash
cmake -S . -B build-security \\
  -DCMAKE_BUILD_TYPE=Debug \\
  -DPKGINTEL_BUILD_TESTS=ON \\
  -DPKGINTEL_REQUIRE_PRIVILEGED_SECURITY_TESTS=ON
cmake --build build-security -j"$(nproc)"
sudo ctest --test-dir build-security -R 'pkgintel[.]security[.]mounts' --output-on-failure
```

In the intended Ubuntu 24.04 verification environment, the command must PASS. If the kernel/container denies the mount operation, strict mode must FAIL rather than silently skip. The CI workflow contains a dedicated privileged security-runtime job for this gate.

### Current runtime qualification evidence

The dedicated privileged verification runtime was executed from the Windows 11 -> Docker Desktop -> Ubuntu 24.04 workflow. The build used `PKGINTEL_REQUIRE_PRIVILEGED_SECURITY_TESTS=ON`, and the genuine test was run without changing or weakening the test source.

Observed environment:

- UID/GID: `0/0`.
- `CapPrm`, `CapEff`, and `CapBnd`: `000001ffffffffff`.
- Seccomp mode 2 with one active filter.
- Identity UID mapping: `0 0 4294967295`.
- A dedicated mount namespace was present.
- An independent real `mount --bind` probe returned `mount_rc=0` and successfully read the mounted file.

Actual pkgintel evidence:

```
Test #2: pkgintel.security.mounts ......... Passed
1/1 test passed

Test #1: pkgintel.unit.core ............... Passed
Test #2: pkgintel.security.mounts ......... Passed
2/2 tests passed
100% tests passed
```

Interpretation: the environment-capability gap is closed for this verification runtime. The genuine hostile-filesystem test now has real kernel/VFS evidence, and the full security-runtime CTest suite passes. No fake mount, test relaxation, or silent skip was used.


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

1. Re-run the fuzz targets after the SanitizerCoverage instrumentation correction and require real `cov:`/`ft:` evidence plus corpus behavior.
2. Run the current reproducible performance benchmark and record results.
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


## Current Principal/Staff Engineer review checkpoint

The current review has completed source-level inspection of the implemented P0 vertical slice. The principal findings are:

### Confirmed strong areas

- target-root descriptor anchoring and Linux openat2 containment design;
- RESOLVE_IN_ROOT, RESOLVE_NO_MAGICLINKS, and RESOLVE_NO_XDEV policy;
- O_PATH/O_NOFOLLOW artifact identity observation;
- bounded dpkg record materialization;
- deterministic package ordering;
- snapshot ownership and package-to-artifact mapping;
- aggregate resource ceilings and transactional mutation;
- genuine hostile-filesystem test design;
- opaque public-object architecture;
- separation of package identity, installation state, and filesystem evidence.

### Current blockers

1. **Consistency semantics were corrected.**
   Package consistency now requires complete file correlation and is derived from artifact evidence. NOT_REQUESTED and INCOMPLETE correlation return UNKNOWN rather than silently claiming CONSISTENT.

2. **Broken-link and permission-denied evidence now participate in consistency.**
   A single failure class maps to its specific consistency state; mixed failure classes map to INCONSISTENT while individual artifact evidence remains available.

3. **Unexpected errno no longer implies missing evidence.**
   Only definitive ENOENT increments missing-file accounting. Other observation failures are represented as unverifiable/invalid evidence as appropriate.

4. **Correlation completeness is explicit internal state.**
   Each package now tracks NOT_REQUESTED, COMPLETE, or INCOMPLETE correlation. This prevents zero artifacts from being confused with successful zero-record correlation.

5. **Privileged filesystem evidence remains environment-gated.**
   Genuine bind-mount and procfs magic-link tests must produce PASS in a capable Linux environment before their release gate is satisfied. SKIP_UNAVAILABLE is not PASS.

6. **Root identity timing must remain an explicit contract.**
   The current design resolves the configured root path when scanning opens the target root and then anchors operations to the resulting descriptor. This timing should be documented as intentional rather than accidental.

### Immediate engineering sequence

~~~
P5 consistency contract
    -> explicit correlation completeness
    -> evidence-derived consistency
    -> regression tests
    -> API/security documentation review
    -> P8 ABI contract audit
    -> P9 release/security gates
~~~

P5 implementation is now source-complete at the contract level. Automated verification is still required in the real build environment before the gate can be marked PASS.

Do not start unrelated refactors while these semantic gates are open.

## Agent handoff requirement

A future agent must refresh the branch head before acting. The branch may move after this document is written.

The repository-level onboarding contract is:

- `AGENTS.md`
- `docs/AGENT_ONBOARDING.md`
- `docs/ENGINEERING_WORKFLOW.md`
- `docs/BRANCH_PROVENANCE.md`

These documents are the canonical handoff mechanism and must remain synchronized with major workflow or branch changes.
