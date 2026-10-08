# pkgintel Agent Engineering Contract

## Purpose

This is the repository-level operating contract for humans, coding agents, reviewers, and future maintainers.

pkgintel is a security-sensitive systems project. Work must meet the applicable Staff/Principal Software Engineer, Principal Security Engineer, CA/Finance, and MBA/Management standards described below.

The objective is not maximum code or maximum abstraction. The objective is a system whose behavior, security claims, costs, interfaces, and verification evidence are explainable and reproducible.

## Current working branch

~~~
main
  |
  v
codex/p0-foundation
  |
  v
codex/p0-module-architecture   <-- current P0 integration line
~~~

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

1. Requirements and business/operational constraints.
2. HLD boundary and responsibility.
3. LLD contracts and invariants.
4. Mathematical/resource model.
5. Threat model and security boundary.
6. Algorithm/data-structure choice.
7. Design-pattern vocabulary, only if useful.
8. Implementation.
9. Tests and evidence.
10. Performance measurement.
11. Documentation/ADR/API updates.
12. Release and operational decision.

Do not start with a pattern, data structure, optimization, abstraction, or implementation technique before establishing the problem and invariant.

### Canonical skill references

- `SKILLS.md` — capability contract and review questions.
- `docs/ENGINEERING_PRINCIPLES.md` — engineering principles and SOLID/design reasoning.
- `docs/HLD.md` — high-level architecture.
- `docs/LLD.md` — low-level contracts and invariants.
- `docs/MATHEMATICS.md` — quantitative and arithmetic reasoning.
- `docs/ALGORITHMS.md` — algorithm/data-structure decisions.
- `docs/PERFORMANCE.md` — measurement and performance policy.
- `docs/SECURITY.md` — security model and evidence requirements.
- `docs/API_CONTRACT.md` — public API/ABI contract.
- `docs/ADR/README.md` — architectural decision history.

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

## Canonical repository documents

Read these before substantial work:

- AGENTS.md
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
