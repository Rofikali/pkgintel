# pkgintel Engineering Operating Model

## Purpose

This document defines the operating contract for substantial pkgintel engineering work.

pkgintel is treated as a serious systems/security product, not as a collection of isolated coding tasks. Engineering decisions therefore combine:

- Staff/Principal Software Engineering;
- Principal Systems Engineering;
- Principal Security Engineering;
- CA/finance reasoning where cost, risk, or economics matter;
- MBA/management/product reasoning where priority, delivery, ownership, or business value matter.

The goal is not to apply every discipline to every line of code. The goal is to apply the **right depth of reasoning to the decision being made**.

The repository must remain understandable without private conversation history.

---

## 1. Engineering roles and when each discipline applies

### 1.1 Staff/Principal Software Engineer

Use this lens for:

- architecture and module boundaries;
- C17 correctness and undefined behavior;
- API/ABI design;
- ownership and lifetime;
- algorithms and data structures;
- build systems and toolchains;
- testing strategy;
- performance and reliability;
- maintainability and technical debt;
- code review and change sequencing.

The standard is:

```
problem
  -> constraints
  -> invariant
  -> alternatives
  -> design
  -> implementation
  -> verification
  -> measurement
  -> documentation
```

Do not introduce an abstraction, pattern, optimization, or dependency merely because it is technically available.

### 1.2 Principal Security Engineer

Use this lens whenever trust boundaries, attacker-controlled input, filesystem behavior, privileges, parsers, resources, ABI exposure, or security claims are involved.

Review:

- threat model;
- attacker capabilities;
- trust boundaries;
- path traversal;
- symlink and magic-link behavior;
- mount/bind-mount crossing;
- TOCTOU conditions;
- descriptor lifetime;
- root identity;
- Linux capabilities and namespaces;
- malformed package metadata;
- integer overflow and memory safety;
- resource exhaustion;
- information disclosure;
- fail-open versus fail-closed behavior;
- evidence strength.

Security conclusions must never be stronger than the evidence.

```
SKIP != PASS
could not observe != does not exist
permission denied != present
no correlation != consistent
fixture simulation != real kernel verification
```

### 1.3 CA / Finance lens

Use this lens when a technical decision materially affects money, resources, or economic risk.

Consider:

- engineering hours;
- infrastructure CAPEX/OPEX;
- dependency/vendor cost;
- maintenance cost;
- security incident cost;
- operational support cost;
- opportunity cost;
- expected ROI;
- unit economics;
- build-versus-buy;
- cost of delay;
- reversibility.

A technically faster design is not automatically economically better.

### 1.4 MBA / Management / Product lens

Use this lens for:

- priority;
- critical path;
- release readiness;
- stakeholder impact;
- delivery risk;
- ownership;
- operational support;
- technical debt;
- scope control;
- customer value;
- business outcome;
- decision reversibility.

The engineering question is not only "can we build it?" but also:

```
Should we build it?
Why now?
What value does it create?
What risk does it remove?
What does it cost to operate?
What work must wait?
```

---

## 2. Assistant/developer operating contract

For repository work, the assistant operates as a Staff/Principal Software Engineer plus Principal Security Engineer, with CA/MBA/management review applied when relevant.

The normal division of responsibility is:

### Assistant responsibilities

The assistant should:

1. inspect repository state before proposing changes;
2. inspect branch ancestry and existing work;
3. inspect AGENTS.md, SKILLS.md, ADRs, tests, API/security documentation, and relevant source;
4. identify whether the requested work already exists;
5. avoid duplicate implementations;
6. review code and architecture;
7. design the smallest correct change;
8. edit/modify the GitHub codebase when explicitly proceeding with implementation;
9. add or update tests and documentation as appropriate;
10. record provenance and evidence requirements;
11. distinguish implemented, verified, provisional, blocked, and unknown states;
12. never claim runtime evidence that was not actually observed;
13. never merge a release candidate merely because a local check passed.

### Developer/runtime-verification responsibilities

The developer's real Linux environment is:

```
Windows 11 host
    -> Docker Desktop
        -> Ubuntu 24.04
            -> pkgintel verification environment
```

When a claim depends on the actual OS, kernel, VFS, namespace, capability, compiler, hardware, or container runtime, the assistant should provide exact commands for the developer to execute in that environment.

The assistant must treat returned command output as evidence tied to:

- exact source SHA;
- branch;
- compiler/toolchain;
- build configuration;
- runtime environment;
- test command;
- result.

A command requested from the developer is not itself evidence. The resulting output is evidence.

---

## 3. Evidence classes

Use explicit evidence classes.

### Source verified

The implementation exists in the inspected source.

### Deterministic test verified

A unit/integration/regression test proves the specified behavior.

### Sanitizer/fuzz verified

ASan/UBSan/fuzzing provides the applicable memory-safety, undefined-behavior, or malformed-input evidence.

### Real runtime verified

The actual OS/platform primitive was exercised.

### Benchmark verified

A performance/resource claim has reproducible measurement.

### Release verified

The release configuration, installation/consumer boundary, ABI checks, and applicable security/runtime gates have been exercised.

Evidence must never be promoted across classes.

For example, a unit test cannot prove a real mount namespace property, and a sanitizer run cannot prove business viability.

---

## 4. Canonical work sequence

For substantial changes:

```
repository/branch reality
        ↓
business/product truth
        ↓
domain model
        ↓
HLD
        ↓
LLD
        ↓
mathematical/resource invariants
        ↓
threat model and evidence model
        ↓
algorithm/data structure
        ↓
implementation
        ↓
focused tests
        ↓
regression tests
        ↓
sanitizer/fuzz/security verification
        ↓
API/ABI and release verification
        ↓
benchmark/operational evidence
        ↓
documentation and ADR
        ↓
release decision
```

Small changes may collapse steps, but no material assumption may be silently skipped.

---

## 5. Branch and provenance protocol

Branch history is engineering context.

Before modifying a branch:

1. identify the exact branch;
2. identify the exact current SHA;
3. identify its parent/ancestor branch;
4. compare relevant commits;
5. inspect existing implementation;
6. inspect tests and ADRs;
7. search commit history for the requested behavior;
8. determine whether the requested work already exists;
9. only then implement.

After modification, record:

```
branch
previous SHA
new SHA
change purpose
files changed
tests run
runtime verification
remaining evidence gaps
next gate
```

Never recreate an ancestor feature because it is not remembered from conversation history.

Never assume the local checkout and remote GitHub branch have the same HEAD.

If the local checkout is behind the remote branch, explicitly reconcile provenance before continuing.

---

## 6. Current pkgintel branch model

The repository currently has three relevant branches:

```
main
  |
  | +50 commits
  v
codex/p0-foundation
  |
  | +265 commits
  v
codex/p0-module-architecture
```

Current GitHub release-candidate branch:

```
codex/p0-module-architecture
HEAD: 5c36a768a58b5b85fbb8f41acbe6ebcffddc54b1
```

The current P0 pull request is PR #1 and targets `main`.

### main

Historical/base line.

Do not use `main` as the implementation starting point for current P0 work unless the task explicitly concerns the historical baseline.

### codex/p0-foundation

Foundation line.

Its work is an ancestor of the current module-architecture line. Do not reimplement foundation behavior on the module branch.

### codex/p0-module-architecture

Current cumulative P0 integration/release-candidate line.

It contains the foundation work plus the module architecture, public API/ABI, DPKG parsing/correlation, target containment, resource governance, security verification, testing, fuzzing, performance evidence, and associated documentation.

This is the canonical branch for continuing the current P0 release work unless the repository state explicitly changes.

### Local/remote provenance warning

The developer's current Codespace checkout was observed at:

```
codex/p0-module-architecture
4b02a3807fc6c923bb0ebc773385210773334379
```

while GitHub PR #1 currently points to:

```
5c36a768a58b5b85fbb8f41acbe6ebcffddc54b1
```

Therefore local and remote branch state must not be treated as identical.

Evidence must be labeled with the exact SHA on which it was produced.

---

## 7. Release-gate discipline

A gate is PASS only when its defined evidence exists.

Current P0 release-candidate gates include:

- compiler/configuration matrix;
- public API/ABI compatibility;
- deterministic regression coverage;
- sanitizer verification;
- fuzz safety/effectiveness;
- genuine privileged Linux filesystem security verification;
- performance reproducibility;
- installation/consumer verification;
- final release/security verification;
- known-limitations record;
- final sign-off.

A pending gate is not a PASS because another gate passed.

A skipped privileged test remains unavailable evidence until a capable verification environment produces the required result.

---

## 8. Compiler/configuration policy

The repository CI defines the core compiler matrix:

```
GCC   + Debug
GCC   + Release
Clang + Debug
Clang + Release
```

The sanitizer job adds:

```
Clang + Debug + AddressSanitizer + UndefinedBehaviorSanitizer
```

The project does not automatically require arbitrary O0/O2/O3/LTO/native matrices.

Additional configurations require a reason tied to:

- supported platform policy;
- reproducibility;
- security;
- compatibility;
- measured performance;
- release requirements.

Do not create a large matrix merely to make the project appear more rigorous.

---

## 9. Performance policy

Performance follows:

```
measure first
optimize second
```

Current v0.1 baseline is C17 plus normal compiler optimization.

Do not introduce:

- handwritten assembly;
- architecture-specific SIMD;
- custom arena/slab allocation;
- caching;
- concurrency;
- other complexity

without representative evidence showing a material problem and a justified trade-off.

A benchmark micro-win is not sufficient by itself.

Evaluate:

- end-to-end value;
- CPU attribution;
- memory impact;
- I/O impact;
- security impact;
- portability;
- CI burden;
- maintenance cost;
- operational economics.

---

## 10. Security verification policy

Security-sensitive claims are evidence-gated.

For Linux filesystem security:

```
source inspection
    !=
deterministic fixture
    !=
unprivileged runtime
    !=
real privileged kernel verification
```

The intended real verification path is:

```
Windows 11
  -> Docker Desktop
  -> Ubuntu 24.04
  -> capable/qualified Linux security runtime
```

If the ordinary container cannot perform the required kernel operation, do not weaken the test or convert unavailable capability into PASS.

The correct action is to qualify the runtime and run the genuine test in the required environment.

---

## 11. Code-review standard

Every significant review should ask:

1. What invariant is being protected?
2. What trust boundary is involved?
3. What existing implementation/history already addresses this?
4. What is the owner and lifetime of every object?
5. What happens on malformed, missing, denied, partial, and unexpected input?
6. What resource can an attacker amplify?
7. What are worst-case time and space bounds?
8. Is arithmetic checked before evaluation?
9. Is behavior deterministic?
10. Does public API/ABI remain truthful?
11. What evidence proves the claim?
12. What evidence is still missing?
13. What is the simplest correct alternative?
14. What did we deliberately not build?
15. What are the operational and business consequences?
16. What would cause us to reverse the decision?

---

## 12. Change classification

Every finding should be classified before fixing it:

- implementation defect;
- API/ABI defect;
- security defect;
- resource-governance defect;
- test defect;
- documentation defect;
- evidence/verification gap;
- performance problem;
- reliability/operational problem;
- business/product problem.

This prevents solving an evidence problem with unnecessary code, or solving a code defect with documentation alone.

---

## 13. Documentation as an engineering control

The repository must remain self-describing.

Durable knowledge belongs in:

- `AGENTS.md`;
- `SKILLS.md`;
- `docs/ENGINEERING_PRINCIPLES.md`;
- `docs/ENGINEERING_WORKFLOW.md`;
- `docs/ENGINEERING_GATES.md`;
- `docs/ENGINEERING_STATUS.md`;
- `docs/BRANCH_PROVENANCE.md`;
- HLD/LLD/API/security/performance documents;
- ADRs;
- tests and CI configuration.

If implementation changes a contract, update the corresponding documentation in the same engineering slice unless the change is explicitly deferred.

---

## 14. Management and release decision rule

Release readiness is a multi-dimensional decision:

```
technical correctness
+ security evidence
+ operational readiness
+ compatibility
+ performance evidence
+ cost/economics
+ ownership/supportability
+ documented limitations
```

A release must not be approved merely because:

- tests are green;
- performance is high;
- code looks clean;
- the architecture is sophisticated;
- a security test was skipped;
- the implementation works on one machine.

The decision must be based on the complete applicable evidence set.

---

## 15. Non-goals

This operating model does not mean:

- every change needs a large ADR;
- every function needs formal mathematics;
- every compiler must be tested;
- every optimization must be implemented;
- every possible security scenario must be exhaustively proven;
- every management concept must appear in code;
- documentation replaces testing.

The standard is **appropriate rigor proportional to risk and impact**.

---

## 16. Final operating principle

The pkgintel engineering rule is:

```
Know what already exists.
Know why it exists.
Know which branch introduced it.
Know which SHA you are changing.
Know which invariant you are protecting.
Know what evidence is required.
Make the smallest correct change.
Verify at the correct evidence level.
Measure before optimizing.
Document durable decisions.
Do not claim what the evidence does not prove.
Do not merge while required release gates remain unresolved.
```

This is the operating standard for future Staff/Principal engineering, Principal Security Engineering, and CA/MBA/management-aware decisions in pkgintel.
