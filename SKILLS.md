# pkgintel Engineering Skills Contract

## Purpose

This document defines the engineering capabilities expected of a Staff/Principal Engineer working on pkgintel. It is a capability map, not a checklist to claim without evidence.

The standard is:

understand -> model -> design -> implement -> verify -> measure -> document -> operate -> review

A future agent must use the skill areas relevant to the change. A skill is considered demonstrated only when source evidence, tests, measurements, or documented decisions support the claim.

## 1. Systems and C Engineering

Required capabilities:

- C17 language semantics, translation units, linkage, storage duration, object lifetime, qualifiers, undefined behavior, implementation-defined behavior, and portability.
- Pointers, ownership, aliasing, alignment, integer conversions, overflow, underflow, signed/unsigned boundaries, and checked arithmetic.
- File descriptors, openat, openat2, fstat, O_PATH, O_NOFOLLOW, O_CLOEXEC, errno semantics, and Linux filesystem behavior.
- Memory allocation with malloc/calloc/realloc, failure atomicity, capacity growth, fragmentation, and destruction paths.
- ABI design: symbol visibility, opaque handles, struct-size versioning, enum stability, calling conventions, ownership, and compatibility.
- CMake/build systems, compiler warnings, sanitizers, fuzzing, static analysis, and reproducible builds.
- Debugging with source-level evidence rather than assumptions.

## 2. Architecture: HLD and LLD

Every non-trivial feature must be understandable at two levels.

### HLD skill

Be able to explain:

- system boundaries;
- modules and responsibilities;
- dependency direction;
- trust boundaries;
- data flow;
- resource boundaries;
- public versus private interfaces;
- extension points;
- operational deployment boundaries.

Canonical source: docs/HLD.md.

### LLD skill

Be able to specify:

- inputs/preconditions;
- outputs/postconditions;
- ownership/lifetime;
- state transitions;
- error taxonomy;
- resource accounting;
- data structures;
- arithmetic invariants;
- security invariants;
- complexity;
- tests and benchmarks.

Canonical source: docs/LLD.md.

A design is incomplete when the HLD is correct but the LLD invariants are unspecified, or when the LLD is locally correct but violates the HLD boundary.

## 3. Software Design Principles

Use principles to improve the system, not to satisfy terminology.

### SOLID translated to C

- Single Responsibility: focused reasons to change.
- Open/Closed: stable contracts can admit new implementations where justified.
- Liskov Substitution: implementations preserve the full documented contract, including errors and ownership.
- Interface Segregation: consumers receive only the interface they need.
- Dependency Inversion: policy depends on explicit stable contracts rather than scattered platform details.

Mechanisms include opaque handles, function-pointer interfaces, private headers, composition, and small contracts.

### Core engineering principles

- Separation of concerns.
- High cohesion.
- Appropriate coupling rather than zero coupling.
- DRY as elimination of duplicated knowledge, not mechanical deduplication.
- Encapsulation and information hiding.
- Least knowledge.
- Fail closed.
- Explicit ownership and lifetime.
- Transactional mutation: validate -> reserve -> construct -> commit.
- Determinism.
- Reversibility.
- Evidence before optimization.

Canonical source: docs/ENGINEERING_PRINCIPLES.md.

## 4. Design Patterns and Architectural Patterns

Patterns are vocabulary for a proven constraint, never the starting point.

Expected skill:

1. identify the problem;
2. state constraints and invariants;
3. compare alternatives;
4. choose the simplest design that satisfies them;
5. name a pattern only if it improves communication.

Patterns potentially relevant to pkgintel include:

- Strategy-like backend interfaces for genuinely interchangeable package-manager backends;
- Adapter boundaries between backend evidence and the normalized domain model;
- Factory/constructor functions for controlled opaque-object creation;
- Registry patterns only when plugin discovery is actually required;
- Facade-like public APIs over internal modules;
- Builder-style staged construction only when it improves validation/commit semantics;
- Iterator/traversal patterns for future filesystem discovery;
- RAII-like cleanup discipline implemented explicitly in C, including single-owner cleanup paths.

Do not introduce inheritance simulations, dependency-injection frameworks, plugin registries, or generic abstractions merely because they have familiar names.

## 5. Algorithms and Data Structures

Be able to reason about:

- time complexity;
- space complexity;
- worst-case versus expected behavior;
- deterministic ordering;
- bounded linear scans;
- sorting and binary search;
- hash tables and adversarial inputs;
- graph traversal;
- explicit stacks versus recursion;
- capacity growth;
- streaming versus materialization.

Current v0.1 preference is conservative bounded structures. Replace linear behavior only when representative measurements justify the additional memory, complexity, or security surface.

Canonical source: docs/ALGORITHMS.md.

## 6. Mathematics and Quantitative Reasoning

Mathematics is an engineering tool for proving bounds and preventing invalid assumptions.

Required areas:

### Arithmetic safety

For multiplication:

N <= SIZE_MAX / S

must hold before calculating N * S.

For addition:

used <= limit - required

is safer than evaluating used + required before checking overflow.

### Capacity growth

Reason about geometric growth, maximum representable capacity, allocation size, and number of reallocations.

### Resource budgets

For every budget define:

- unit;
- scope;
- initial value;
- consumption event;
- exhaustion condition;
- partial-result semantics;
- error/status result.

### Complexity

State expected and worst-case time/space where material.

### Probability/statistics

Use statistics when benchmarking or estimating operational behavior. Never present a sample measurement as a deterministic guarantee.

### Measurement discipline

Distinguish:

- measured fact;
- calculated bound;
- assumption;
- estimate;
- hypothesis.

Canonical source: docs/MATHEMATICS.md.

## 7. Security Engineering

Principal-security review must cover:

- trust boundaries;
- attacker-controlled bytes;
- parser limits;
- path traversal;
- symlink races;
- magic links;
- mount and bind-mount crossing;
- root identity;
- privilege/capability assumptions;
- TOCTOU behavior;
- special files;
- resource exhaustion;
- information disclosure;
- integer/memory safety;
- fail-open versus fail-closed behavior;
- evidence strength.

Security claims must match evidence. In particular:

- inability to observe is not proof of absence;
- permission denial is not proof of presence;
- no correlation is not proof of consistency;
- a simulated mount is not proof of a real mount boundary;
- a skipped privileged test is not a pass.

Canonical source: docs/SECURITY.md and relevant ADRs.

## 8. API, ABI, and FFI Engineering

Required capabilities:

- stable public headers;
- opaque object ownership;
- borrowed versus owned data;
- lifetime and invalidation rules;
- status/error taxonomy;
- enum compatibility;
- struct-size/version negotiation;
- symbol visibility;
- ABI tests;
- C-to-Rust FFI safety boundaries;
- backwards/forwards compatibility decisions.

Public ABI changes require deliberate review and documentation before implementation becomes difficult to reverse.

## 9. Testing and Verification

Think in evidence layers:

1. source inspection;
2. unit/integration tests;
3. sanitizer tests;
4. fuzz tests;
5. real Linux primitive tests;
6. benchmark evidence;
7. production-like/release verification.

Required test thinking:

- happy path;
- malformed input;
- empty input;
- missing input;
- permission failure;
- unexpected I/O;
- resource exhaustion;
- arithmetic boundaries;
- ownership/lifetime;
- deterministic ordering;
- security boundary;
- regression behavior.

SKIP_UNAVAILABLE must remain distinct from PASS.

## 10. Performance and Reliability

Performance engineering requires:

- representative workloads;
- end-to-end measurement;
- CPU/memory/I/O attribution;
- allocation analysis;
- latency and throughput where relevant;
- regression thresholds;
- reproducible benchmark environment.

Reliability engineering requires:

- failure containment;
- partial-result semantics;
- deterministic behavior;
- cleanup correctness;
- bounded resources;
- observability;
- operational recovery.

Canonical source: docs/PERFORMANCE.md and docs/ENGINEERING_GATES.md.

## 11. Linux and Platform Engineering

Know the real platform contract behind the abstraction:

- Linux VFS and path resolution;
- file descriptors and descriptor-relative operations;
- mounts/namespaces/capabilities;
- /proc and magic links;
- permissions and errno;
- Debian/dpkg filesystem conventions;
- kernel-version requirements;
- portability boundaries.

If a property depends on the kernel, the test must exercise the kernel primitive when practical.

## 12. Package and Evidence Domain Modeling

Keep these concepts separate:

Package identity -> installation state -> filesystem artifact evidence -> diagnostic evidence

Do not infer filesystem consistency solely from dpkg installation state.

Evidence must retain enough provenance to explain:

- what was requested;
- what was observed;
- what could not be observed;
- why an observation is classified as missing, broken, denied, or unverifiable.

## 13. CA / Finance Engineering

For material architecture or product decisions, evaluate:

- CAPEX;
- OPEX;
- engineering hours;
- infrastructure cost;
- maintenance cost;
- dependency/vendor cost;
- failure/security cost;
- opportunity cost;
- expected ROI;
- cash-flow implications;
- unit economics;
- build-versus-buy.

Do not optimize a technical metric while ignoring total cost of ownership.

## 14. MBA / Management / Product Engineering

A Principal Engineer also manages technical scope.

Evaluate:

- business objective;
- customer value;
- priority;
- dependencies;
- critical path;
- delivery risk;
- technical debt;
- operational ownership;
- support burden;
- stakeholder communication;
- reversibility;
- release readiness.

Use ADRs for decisions whose future reversal would be expensive.

## 15. Documentation and Knowledge Transfer

A production-grade system should be self-describing.

Maintain alignment among:

- AGENTS.md;
- SKILLS.md;
- HLD;
- LLD;
- architecture;
- API contract;
- security model;
- mathematics;
- algorithms;
- performance;
- ADRs;
- engineering status;
- tests;
- release gates;
- branch provenance.

When implementation changes a contract, update the relevant documentation in the same engineering change unless the change is explicitly documented as deferred.

## 16. Principal Engineer Review Questions

Before declaring work complete:

1. What invariant are we protecting?
2. What is the trust boundary?
3. What evidence proves the behavior?
4. What happens on malformed, missing, denied, partial, and unexpected input?
5. What is the owner and lifetime of every object?
6. What resource can an attacker or user amplify?
7. What are the worst-case time and space bounds?
8. Does the public API/ABI remain truthful?
9. Is behavior deterministic?
10. What is the simplest alternative?
11. What did we deliberately not build?
12. How will we test it on real Ubuntu 24.04 when kernel behavior matters?
13. What does it cost to build, run, maintain, and support?
14. What would make us reverse this decision?

## 17. Skill Evidence Rule

Do not write "production-grade" merely because code compiles.

A capability becomes an engineering claim only when backed by appropriate evidence:

- design claim -> HLD/LLD/ADR;
- correctness claim -> tests;
- memory/UB claim -> sanitizer/fuzz evidence;
- Linux security claim -> real runtime evidence;
- performance claim -> benchmark evidence;
- ABI claim -> ABI/install consumer evidence;
- business claim -> explicit assumptions and calculations;
- operational claim -> production-like verification.

The goal is not to master every topic simultaneously. The goal is to apply the right depth to the problem at hand.
