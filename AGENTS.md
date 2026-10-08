# pkgintel Agent Engineering Contract

## Purpose

This is the repository-level operating contract for humans, coding agents, reviewers, and future maintainers.

pkgintel is a security-sensitive systems project. Work must meet the applicable Staff/Principal Software Engineer, Principal Security Engineer, CA/Finance, and MBA/Management standards described below.

The objective is not maximum code or maximum abstraction. The objective is a system whose behavior, security claims, costs, interfaces, and verification evidence are explainable and reproducible.

## Current working branch

~~~
main   <-- canonical P0 integration line
~~~

Historical P0 branches:
- `codex/p0-module-architecture` — cumulative P0 integration branch, merged by PR #1.
- `codex/p0-foundation` — ancestor of the cumulative P0 branch; no separate merge into `main` was required.

Current source of truth: `main` at the post-merge P0 reconciliation line. New implementation work should branch from current `main` unless an explicit workflow decision says otherwise.

Do not repeat work already present on an ancestor branch. Before changing code:

1. identify the current branch and commit;
2. inspect ancestry and recent commits;
3. search for an existing implementation, test, ADR, or documented decision;
4. determine whether the problem is implementation debt, contract debt, evidence debt, or documentation debt;
5. make the smallest change that closes the actual gap.

## Engineering roles

### Staff/Principal Software Engineering

Review architecture, module boundaries, ownership, lifetime, ABI/API contracts, failure semantics, complexity, performance, maintainability, portability, testing, release engineering, and change reversibility.

### Principal Security Engineering

Review trust boundaries, attacker-controlled bytes, filesystem races, path traversal, symlinks, magic links, mount boundaries, privilege assumptions, resource exhaustion, information disclosure, memory/integer safety, fail-open/fail-closed behavior, and the strength of evidence behind every security claim.

### CA / Finance

For decisions with material cost or business consequences, evaluate unit economics, CAPEX/OPEX, engineering and operational cost, maintenance burden, vendor dependency, risk exposure, ROI, cash-flow impact, and opportunity cost.

### MBA / Management / Product

Evaluate scope, priorities, dependencies, roadmap, delivery risk, technical debt, operational ownership, support burden, stakeholder impact, decision records, and whether the chosen design delivers the required business value without unnecessary complexity.

Use the role that is relevant to the decision; do not force irrelevant analysis into every line of code.

## Engineering skill contract

Every substantial engineering task must use the capability areas in `SKILLS.md`. The repository explicitly treats the following as first-class Staff/Principal Engineer skills:

- Systems/C and Linux engineering.
- HLD and LLD design.
- SOLID translated to C.
- Design-pattern reasoning without pattern-driven overengineering.
- Algorithms, data structures, complexity, and determinism.
- Mathematics: checked arithmetic, resource bounds, complexity, probability/statistics, and measurement.
- Security engineering and evidence strength.
- API/ABI/FFI engineering.
- Testing, sanitizers, fuzzing, benchmarks, and real-runtime verification.
- Performance and reliability engineering.
- Package/evidence domain modeling.
- CA/Finance analysis for material cost and economic decisions.
- MBA/Management/Product analysis for scope, delivery, operations, risk, and value.
- Documentation, ADRs, knowledge transfer, and reversible decision-making.

These are not separate approval gates on every line of code. Apply the relevant depth to the problem. A change that crosses architecture, security, ABI, resource, or business boundaries must explicitly reason across those dimensions.

### Design hierarchy

Use this order for non-trivial work:

1. **Business/product truth** — customer/user problem, product outcome, acceptance criteria, economics, operational constraints, legal/compliance constraints, and what must *not* be built.
2. **Domain model** — define the real entities, states, evidence, ownership, and invariants before choosing implementation structures.
3. **HLD** — system boundaries, responsibilities, trust boundaries, dependencies, and major data/control flows.
4. **LLD** — concrete interfaces, ownership/lifetime, error semantics, state transitions, and module contracts.
5. **Mathematical/resource invariants** — bounds, checked arithmetic, complexity, capacity, quotas, and conservation/resource-accounting rules.
6. **Security/evidence model** — threat model, trust assumptions, attacker capabilities, evidence strength, and fail-closed behavior.
7. **Algorithm/data-structure choice** — select only after the model and invariants are explicit; justify complexity and determinism.
8. **Design-pattern vocabulary**, only if useful for communicating an already-justified design.
9. **Implementation** — smallest change that preserves the contracts.
10. **Tests** — deterministic correctness, malformed-input, resource, ownership, and security-boundary tests as applicable.
11. **ADR/API documentation** — record durable decisions and public contract changes.
12. **Real runtime verification** — exercise the actual OS/kernel/platform/security primitive when the claim depends on it; record the exact environment and result.

This order is a reasoning dependency, not a bureaucracy requirement. Small changes may collapse several steps, but they must not silently skip the reasoning that determines correctness.

Do not start with a pattern, data structure, optimization, abstraction, or implementation technique before establishing the problem and invariant.

### Canonical skill references

- `SKILLS.md` — capability contract and review questions.
- `docs/ENGINEERING_OPERATING_MODEL.md` — role, provenance, evidence, branch, release, and management operating contract.
- `docs/ENGINEERING_PRINCIPLES.md` — engineering principles and SOLID/design reasoning.
- `docs/HLD.md` — high-level architecture.
- `docs/LLD.md` — low-level contracts and invariants.
- `docs/MATHEMATICS.md` — quantitative and arithmetic reasoning.
- `docs/ALGORITHMS.md` — algorithm/data-structure decisions.
- `docs/PERFORMANCE.md` — measurement and performance policy.
- `docs/SECURITY.md` — security model and evidence requirements.
- `docs/API_CONTRACT.md` — public API/ABI contract.
- `docs/ADR/README.md` — architectural decision history.
- `docs/RELEASE_EVIDENCE.md` — canonical release-gate evidence ledger, including exact source SHA, branch provenance, runtime environment, verification commands/results, evidence class, and remaining gaps.
- `docs/PR_MERGE_RELEASE_POLICY.md` — canonical pull-request, review, merge, exact-SHA approval, post-merge, and release-decision policy.

Before substantial work, read `SKILLS.md` plus the canonical documents relevant to the change.

## Evidence hierarchy

Never promote weak evidence into a stronger claim.

Use explicit labels:

- Source verified — implementation exists in the inspected source.
- Unit/integration verified — deterministic automated test proves the behavior.
- Sanitizer/fuzz verified — memory/undefined-behavior or malformed-input evidence exists.
- Real runtime verified — the actual OS/platform primitive was exercised.
- Benchmark verified — performance/resource claim has reproducible measurement.
- Production-like verified — release configuration and operational boundary were exercised.

A skipped test is not a pass. A simulated filesystem fixture is not evidence of a real mount boundary.

## Real Linux verification environment

The intended OS verification environment is:

~~~
Windows 11 host
  -> Docker Desktop
  -> Ubuntu 24.04
  -> pkgintel build/test environment
~~~

When kernel/VFS/namespace/privilege behavior cannot be proved from an unprivileged environment, provide exact Ubuntu 24.04 commands for the developer to run.

Do not weaken a security test merely because the current container lacks a required capability.

For genuine Linux mount/VFS verification, qualify the runtime before declaring the gate complete. The intended verification path is Windows 11 -> Docker Desktop -> Ubuntu 24.04 in a dedicated privileged security container. A `sudo` shell inside a restricted container is insufficient if `mount(2)` remains unavailable. Inspect capabilities/seccomp/namespaces and move the test to a capable verification container rather than converting the security test into a skip or simulated filesystem test.

## Change discipline

For every non-trivial change record:

1. Problem and violated invariant.
2. Existing implementation/history.
3. Alternatives considered.
4. Selected design and why.
5. Security consequences.
6. Ownership/lifetime consequences.
7. Resource and complexity bounds.
8. API/ABI consequences.
9. Tests added or changed.
10. Documentation/ADR impact.
11. Operational and business consequences where material.
12. Verification result and environment.

Prefer:

~~~
validate -> reserve -> construct -> commit
~~~

over partially mutating state and repairing it after failure.

## Security truthfulness

Never confuse:

~~~
could not observe  !=  does not exist
no correlation  !=  consistent
permission denied != present
broken link      != present
partial dpkg state != filesystem inconsistency
SKIP             != PASS
~~~

A security-sensitive conclusion must be supported by evidence at least as strong as the conclusion.

## Public API / ABI

Public headers under include/pkgintel/ are contracts. Internal headers under src/internal/ are implementation contracts.

Opaque public objects must document ownership, lifetime, borrowing, invalidation, thread-safety, status/error semantics, versioning, and resource-limit behavior.

Do not expose internal struct layout merely to make implementation easier.

## Testing

Every feature should cover applicable:

- happy path;
- malformed input;
- empty input;
- missing data;
- permission failure;
- resource exhaustion;
- boundary values;
- ownership/lifetime;
- deterministic ordering;
- security boundary behavior.

Use the narrowest relevant test first, then the applicable release gates.

## Git discipline

Do not rewrite history or force-push unless explicitly required and understood.

Before modifying a branch, establish its current SHA. After modification, report branch, previous SHA, new SHA, files changed, tests run, and remaining evidence gaps.

Do not create duplicate implementations on multiple branches.

When local and remote branch heads differ, stop and reconcile provenance before using either head as release evidence.

## Canonical repository documents

Read these before substantial work:

- AGENTS.md
- SKILLS.md
- docs/ENGINEERING_OPERATING_MODEL.md
- docs/ENGINEERING_STATUS.md
- docs/ENGINEERING_PRINCIPLES.md
- docs/ENGINEERING_WORKFLOW.md
- docs/BRANCH_PROVENANCE.md
- docs/ENGINEERING_GATES.md
- docs/ARCHITECTURE.md
- docs/API_CONTRACT.md
- docs/SECURITY.md
- docs/ADR/README.md

For a security change, also read the relevant ADRs and tests.

## Rule for future agents

If a future agent cannot determine what exists, why it exists, which branch introduced it, what evidence supports it, what remains blocked, and what must not be repeated, the repository documentation is incomplete.


## Release evidence and anti-repetition protocol

Release work is governed by evidence provenance, not by memory or by the existence of a green local command.

Before running a potentially expensive or security-sensitive verification:

1. establish the exact current branch and SHA;
2. locate the latest prior evidence SHA for the same property;
3. compare production-relevant files between that evidence SHA and current HEAD;
4. rerun only when the changed surface, environment, toolchain, or evidence requirement makes the prior result non-applicable;
5. record the decision in `docs/RELEASE_EVIDENCE.md`.

A verification result must record, as applicable:

- branch and exact source SHA;
- parent/ancestor relationship when evidence is inherited;
- production-relevant file delta since the evidence baseline;
- host/container/OS/kernel/toolchain;
- compiler and build flags;
- exact command or CI-equivalent command;
- test/build/install/ABI/security result;
- skipped/unavailable checks and why;
- evidence class;
- security limitations and attacker/trust-boundary assumptions;
- performance workload/statistics when performance is involved;
- business/operational/cost consequences when material;
- the next gate and explicit owner/action.

Do not rerun a gate merely to create a newer timestamp when the source and required environment are unchanged. Do not reuse evidence silently when production code, ABI, compiler/toolchain, configuration, security boundary, or runtime assumptions changed.

For multi-branch work, branch ancestry is part of the implementation contract. The canonical release/development line is current `main` unless the repository explicitly changes it. Historical merged branches remain provenance references only. If another branch contains a requested feature, consume or reconcile that existing work rather than implementing a second copy.

The authoritative developer runtime for OS/platform evidence is:

```
Windows 11 host
  -> Docker Desktop
  -> Ubuntu 24.04
  -> pkgintel verification environment
```

The assistant must not claim to have executed commands inside that environment. When real kernel/VFS/namespace/capability evidence is required, provide exact commands; the developer's returned output is the evidence.

### Role-specific review depth

- **Staff/Principal Software Engineering:** architecture, ownership/lifetime, API/ABI, complexity, portability, reliability, release engineering, maintainability, and reversibility.
- **Principal Security Engineering:** attacker capability, trust boundaries, filesystem/VFS behavior, privilege/capability assumptions, fail-closed semantics, resource exhaustion, memory/integer safety, evidence strength, and security-claim truthfulness.
- **CA/Finance:** material CAPEX/OPEX, unit economics, operational cost, vendor dependency, maintenance burden, risk-adjusted cost, ROI, cash-flow and opportunity cost.
- **MBA/Management/Product:** priority, scope, dependencies, delivery risk, operational ownership, support burden, stakeholder value, roadmap sequencing, and reversible decision-making.

Apply only the dimensions material to the decision. The point is complete engineering judgment, not ceremony.
