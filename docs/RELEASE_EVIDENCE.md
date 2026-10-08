# Release Evidence Ledger

## Purpose

This is the canonical evidence ledger for pkgintel release-gate work. It prevents repeated verification, stale evidence reuse, branch confusion, and unsupported security/performance claims.

Every significant release-gate result must identify the exact source SHA and the evidence class. A result from an ancestor commit is inherited only after a production-relevant diff proves that the evidence remains applicable.

## Current release candidate

- Repository: `Rofikali/pkgintel`
- Release line: `codex/p0-module-architecture`
- Current SHA: `946a7fc91c689dfbe408f254b81f20145ce9fa24`
- Remote synchronization: local and origin branch heads match at this SHA.
- PR: #1, base `main`, not merged.
- Release decision: **NOT READY TO MERGE**.

## Branch anti-duplication record

The current repository has three relevant branches:

```
main
  |
  v
codex/p0-foundation
  |
  v
codex/p0-module-architecture  <-- current cumulative P0/release line
```

Known heads at this checkpoint:

| Branch | SHA | Responsibility |
|---|---|---|
| `main` | `003ac31c9f6b12e71a13d8958002a5263717ffe8` | historical/base line |
| `codex/p0-foundation` | `9a3ddd9f3dc0faa12eeec3bda30800df045f2518` | foundational work |
| `codex/p0-module-architecture` | `15c4ee667ab74cc849eb021789ecec5ed5e10492` | cumulative P0/release line |

Before implementing anything, identify whether the requested work already exists on an ancestor or the current cumulative branch. Missing evidence is an evidence problem, not permission to create duplicate implementation.

## P9.2 — Performance evidence

### Baseline

- Evidence commit: `5c36a768a58b5b85fbb8f41acbe6ebcffddc54e1`
- Commit: `docs: record repeated performance baseline evidence`
- Current HEAD: `15c4ee667ab74cc849eb021789ecec5ed5e10492`

### Applicability check

The exact comparison `5c36a76..HEAD` shows only:

- `AGENTS.md`
- `docs/BRANCH_PROVENANCE.md`
- `docs/ENGINEERING_OPERATING_MODEL.md`

changed after the performance evidence commit.

There were **no** changes to:

- `src/`
- `include/`
- `tests/`
- `CMakeLists.txt`
- compiler/build configuration
- benchmark implementation
- public API/ABI
- production runtime implementation

Therefore the repeated performance evidence remains applicable to the current source.

**Gate: PASS.**

### Recorded repeated results

| Workload | Mean artifacts/s | Min | Max | CV |
|---|---:|---:|---:|---:|
| 100 × 100 | 792,452 | 684,997 | 860,689 | 11.88% |
| 1,000 × 100 | 871,385 | 795,557 | 924,986 | 7.75% |
| 5,000 × 20 | 710,472 | 682,927 | 725,758 | 3.36% |
| Allocation 1,000 × 100 | 829,423 | 808,654 | 847,314 | 2.35% |

Evidence interpretation: repeated runs were reasonably consistent, larger workloads were more stable, allocation counts were repeatable, and `final_live_bytes=0`. No evidence currently justifies claiming that arena/slab allocation or SIMD/assembly is a bottleneck. Optimization remains evidence-gated.

## P9.3 — Compiler/configuration matrix

Required configurations:

1. GCC Debug
2. GCC Release
3. Clang Debug
4. Clang Release
5. Clang ASan/UBSan

The normal compiler matrix uses the CI-equivalent hardening baseline:

```
-D_FORTIFY_SOURCE=3
-fstack-protector-strong
-fno-common
```

Each configuration should use a fresh build directory and verify, as applicable:

- configure;
- build;
- CTest;
- install;
- exported public ABI;
- private-symbol non-leakage;
- installed consumer.

### GCC Debug — PASS

Source SHA:

```
15c4ee667ab74cc849eb021789ecec5ed5e10492
```

Environment:

- Ubuntu 24.04.5 LTS userland;
- Docker Desktop/WSL2-backed kernel `5.15.167.4-microsoft-standard-WSL2`;
- x86_64;
- GCC 13.3.0;
- CMake 3.28.3;
- Ninja 1.11.1.

Fresh build:

```
build-p9-gcc-debug
```

Configuration:

```
CMAKE_BUILD_TYPE=Debug
PKGINTEL_BUILD_TESTS=ON
PKGINTEL_BUILD_CLI=ON
CFLAGS='-D_FORTIFY_SOURCE=3 -fstack-protector-strong -fno-common'
```

Observed evidence:

- configure: PASS;
- build: PASS, 49/49 build steps;
- `pkgintel.unit.core`: PASS;
- `pkgintel.security.mounts`: SKIPPED because the normal development container lacks the privilege/capability required by the genuine mount test;
- install: PASS;
- exported ABI audit: PASS;
- private implementation symbol audit: PASS;
- installed consumer build/run: PASS.

The GCC Debug compiler/build/install/API gate is **PASS**. The privileged filesystem security gate remains separate and is not satisfied by the skipped test.

## Real OS/platform verification policy

Authoritative developer verification topology:

```
Windows 11 host
  -> Docker Desktop
  -> Ubuntu 24.04
  -> pkgintel verification environment
```

The assistant must not claim to have executed commands inside this environment. For kernel/VFS/namespace/capability-dependent claims, provide exact commands for the developer to run. The returned terminal output is the runtime evidence.

The normal development container is intentionally unprivileged. Genuine mount-boundary/procfs magic-link verification must use the dedicated capable security runtime documented by the repository. `sudo` inside a restricted container is not equivalent to a privileged verification container.

Evidence states are explicit:

- **PASS** — required property was actually exercised and passed;
- **SKIP_UNAVAILABLE** — environment could not exercise the property;
- **FAIL** — the property was exercised and failed.

`SKIP_UNAVAILABLE` never becomes PASS by interpretation.

## Role-specific release review

### Staff/Principal Software Engineering

For every gate, review:

- branch/commit provenance;
- architecture/module boundaries;
- ownership/lifetime;
- API/ABI compatibility;
- complexity/resource bounds;
- portability;
- test coverage;
- release/reversibility;
- maintainability.

### Principal Security Engineering

For security-sensitive gates, review:

- attacker-controlled input;
- trust boundaries;
- filesystem/VFS semantics;
- privilege/capability assumptions;
- fail-closed behavior;
- resource exhaustion;
- memory/integer safety;
- evidence strength;
- whether the environment actually exercised the claimed primitive.

### CA/Finance

Apply when the decision has material economic impact:

- engineering time;
- infrastructure/tooling cost;
- operational burden;
- maintenance cost;
- vendor dependency;
- risk-adjusted cost;
- opportunity cost;
- ROI/cash-flow implications.

Do not invent financial precision where the repository has no reliable cost data.

### MBA/Management/Product

Apply when deciding scope or release sequencing:

- customer/business value;
- priority;
- dependency ordering;
- delivery risk;
- operational ownership;
- support burden;
- roadmap impact;
- reversibility.

These dimensions are decision-specific, not mandatory ceremony for every code change.

## Current gate ledger

| Gate | Status |
|---|---|
| P9.2 performance provenance/reproducibility | **PASS** |
| P9.3 GCC Debug | **PASS** |
| P9.3 GCC Release | **PASS** |
| P9.3 Clang Debug | **PASS** |
| P9.3 Clang Release | **PASS** |
| P9.3 Clang ASan/UBSan | PENDING |
| Genuine privileged filesystem security runtime | **PASS** |
| Final API/ABI review | PENDING |
| Final release/security verification | PENDING |
| P0 sign-off | PENDING |

## Current evidence checkpoint — 2026-10-08

The current release-candidate source checkpoint is `946a7fc91c689dfbe408f254b81f20145ce9fa24`. The intervening production-relevant change after the previously recorded compiler evidence was documentation-only: dedicated security-verification runtime documentation. No production implementation, public ABI/API, tests, build configuration, or benchmark implementation changed.

### P9.3 GCC Debug — PASS

Fresh `build-p9-gcc-debug` evidence passed configure, build, CTest, install, public ABI audit, private-symbol audit, and installed-consumer build/run. The normal unprivileged security.mounts test was skipped and is not promoted to PASS.

### P9.3 GCC Release — PASS

Fresh `build-p9-gcc-release` evidence passed configure, build, CTest, install, public ABI audit, private-symbol audit, and installed-consumer build/run. The normal unprivileged security.mounts test was skipped and remains separate from the privileged security gate.

### P9.3 Clang Debug — PASS

Fresh `build-p9-clang-debug` evidence with Clang 18.1.3 passed configure, build, CTest, install, public ABI audit, private-symbol audit, and installed-consumer build/run. The normal unprivileged security.mounts test was skipped and is not promoted to PASS.

### P9.3 Clang Release — PASS

Fresh `build-p9-clang-release` evidence with Clang 18.1.3 passed configure, build, CTest, install, public ABI audit, private-symbol audit, and installed-consumer build/run. The normal unprivileged security.mounts test was skipped and remains separate from the privileged security gate.

### Privileged Linux filesystem security — PASS

The dedicated privileged security runtime was qualified in the Windows 11 → Docker Desktop → Ubuntu 24.04 workflow. Real capability, namespace, identity, and bind-mount evidence was observed; strict `pkgintel.security.mounts` passed 1/1 and the complete security-runtime CTest suite passed 2/2. This evidence is separate from the normal development-container skip.

### Evidence inheritance decision

The four completed P9.3 compiler gates remain applicable at the current source checkpoint because the intervening source delta was documentation-only. They must not be rerun merely because the repository SHA advanced.

## Anti-repetition rule

Do not rerun a completed gate simply because time has passed.

Rerun when at least one evidence applicability condition changes:

- production-relevant source changed;
- public ABI/API changed;
- compiler/toolchain changed;
- build flags/configuration changed;
- target OS/kernel/platform changed materially;
- security boundary or runtime capability changed;
- benchmark workload/method changed;
- prior evidence is incomplete or invalid;
- the release policy explicitly requires a fresh run.

Otherwise, compare provenance and inherit the valid evidence with an explicit record.

## Evidence record template

Future entries should use:

```
Gate:
Property:
Branch:
Source SHA:
Evidence baseline SHA:
Production-relevant delta:
Environment:
Toolchain:
Configuration:
Command:
Observed result:
Evidence class:
Security assumptions/limitations:
Business/operational impact:
Decision:
Next action:
```

The goal is simple: **know what exists, know which branch contains it, know which SHA proved it, and never perform or claim the same work twice without a reason.**
