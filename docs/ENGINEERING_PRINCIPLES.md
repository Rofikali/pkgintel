# pkgintel Engineering Principles

## Purpose

This document defines how pkgintel makes engineering decisions. It is not a checklist of fashionable principles. A principle is useful only when it improves correctness, security, operability, performance, maintainability, or changeability under the project's real constraints.

The standard is **evidence-backed engineering**:

```
problem
  -> requirements and constraints
  -> candidate designs
  -> architecture / interface decision
  -> algorithm and mathematical model
  -> implementation
  -> verification
  -> measurement
  -> documented decision
```

Passing tests is evidence, not proof of production readiness. A design is accepted only when the applicable engineering gates are satisfied.

## 1. Business/product truth before technical truth

Engineering decisions start with the product reality they serve. Before selecting an architecture, algorithm, pattern, optimization, or dependency, establish:

- the customer/user/operator problem;
- the measurable product outcome;
- acceptance criteria and explicit non-goals;
- economics and operating constraints;
- security, legal, and compliance constraints where applicable;
- the cost of failure and the cost of the proposed solution.

A technically elegant solution that solves the wrong product problem is still an engineering failure.

## 2. Domain truth before implementation

Model the real domain before choosing data structures or modules. Separate entities, state, observed evidence, derived conclusions, ownership, and lifecycle. Do not encode a domain distinction merely because it is convenient for the current implementation.

## 3. HLD before LLD; LLD before code

High-level boundaries and responsibilities come before low-level interfaces. Low-level contracts come before implementation. This ordering makes security boundaries, ownership, resource limits, and change impact reviewable.

## 4. Mathematical/resource invariants before optimization

State bounds, arithmetic constraints, complexity, capacity, resource budgets, and failure accounting before selecting algorithms or optimizations. A faster implementation that violates a resource invariant is not an optimization.

## 5. Security/evidence strength must match the claim

Threat modeling and evidence classification precede implementation of security-sensitive behavior. Source inspection, deterministic tests, sanitizer evidence, and real kernel/runtime evidence prove different things. Never promote one evidence class into another.

## 6. Separation of concerns

Keep different reasons to change separated.

Current examples:

- CLI presentation and exit-code policy belong to `src/cli/`.
- target confinement and descriptor-relative filesystem access belong to the target layer.
- package-manager interpretation belongs to a backend such as `src/backends/dpkg/`.
- normalized package/artifact/diagnostic state belongs to the domain model.
- public ABI declarations belong under `include/pkgintel/`.
- shared implementation contracts belong under `src/internal/`.

Separation is valuable when it prevents unrelated changes from crossing a boundary. Extra modules without a meaningful responsibility are not an architectural improvement.

## 7. High cohesion

A module should contain behavior that changes for closely related reasons.

Prefer:

```
target -> target policy and filesystem observation
dpkg   -> dpkg metadata interpretation
snapshot -> result ownership and mutation
artifact -> artifact semantics
diagnostic -> diagnostic semantics
```

Avoid modules that become generic dumping grounds.

## 8. Loose coupling

Depend on the smallest stable contract required by a consumer.

The dependency direction should make it possible to change a backend or implementation without changing unrelated domain code. Public consumers must not depend on `src/` or private struct layout.

Coupling is not automatically bad. A direct dependency is acceptable when the relationship is stable, local, and cheaper than an abstraction. We optimize for **appropriate coupling**, not zero coupling.

## 9. DRY: don't duplicate knowledge

DRY means avoiding multiple independently maintained representations of the same knowledge. It does not mean mechanically deduplicating every similar line of code.

Good DRY:

- one authoritative resource-budget definition;
- one stable diagnostic code for one semantic condition;
- one shared bounded-record reader for dpkg metadata where the security invariant is identical.

Bad DRY:

- forcing unrelated operations through an abstraction only because their code looks similar;
- creating a universal helper before a second real use case exists.

## 10. SOLID, translated to C

SOLID is used as a reasoning tool, not as an object-oriented requirement.

- **Single Responsibility:** each module has a focused reason to change.
- **Open/Closed:** stable interfaces can permit new backend implementations without rewriting the orchestration layer, when the abstraction is justified.
- **Liskov Substitution:** implementations of the same private/public contract must preserve its documented semantics, including errors, ownership, and resource behavior.
- **Interface Segregation:** interfaces expose only the operations their consumers need.
- **Dependency Inversion:** higher-level policy depends on explicit contracts rather than scattering platform details throughout the system.

C function pointers, opaque handles, private headers, and small interfaces are valid mechanisms for these principles. None is mandatory merely to satisfy SOLID.

## 11. Reuse after understanding

Reusable code should capture a stable invariant or repeated operation.

Before extracting an abstraction ask:

1. Is the behavior genuinely the same?
2. Is the contract stable?
3. Does reuse reduce duplicated knowledge or duplicated risk?
4. Does the abstraction make ownership/error behavior clearer?
5. Does it reduce or increase coupling?

Premature generalization is treated as a cost.

## 12. Design patterns are solutions, not goals

A named pattern is never a reason by itself to introduce code.

The decision process is:

```
problem -> constraints -> alternatives -> trade-offs -> chosen design
```

Only then may we name the resulting pattern if that vocabulary helps communication.

For example, future package backends may use a Strategy-like function interface if multiple backend implementations genuinely need the same lifecycle contract. A registry or plugin system requires a separate security and lifecycle justification.

## 13. Composition over accidental inheritance

C has no class inheritance model, and pkgintel should not simulate one unnecessarily. Prefer composition, explicit ownership, small interfaces, and opaque handles where they produce a clearer contract.

## 14. Encapsulation and information hiding

Public headers expose supported concepts, not implementation layout.

An opaque handle protects:

- ABI compatibility;
- internal data-structure freedom;
- ownership enforcement;
- security invariants;
- future implementation changes.

Private mutation functions must remain outside the public ABI unless a deliberate API decision says otherwise.

## 15. Principle of least knowledge

A component should know only what it needs to perform its responsibility.

For example, a parser should not need to know CLI formatting. A CLI should not manipulate snapshot internals. A target implementation should not decide package semantics.

This reduces accidental dependencies and makes security review tractable.

## 16. Fail closed

When a requested security or resource control is unsupported, reject it rather than silently pretending it was applied.

When an input violates a security boundary, reject it and preserve structured evidence.

When a resource limit is exceeded, return the documented resource-limit result and preserve the already committed partial snapshot where the contract allows it.

## 17. Explicit ownership and lifetime

Every heap object must have a clear owner and destruction path.

For v0.1 snapshots:

- records and owned strings belong to the snapshot;
- public accessors borrow data rather than transferring ownership;
- failed construction must not expose partially initialized objects;
- destruction must release all committed and reserved storage.

Ownership is part of the API/ABI contract.

## 18. Transactional mutation

A multi-step mutation follows:

```
validate -> reserve -> construct -> commit
```

not:

```
partially mutate -> discover failure -> attempt repair
```

A failure before commit must leave the previously committed state valid.

This principle is especially important for resource exhaustion and allocation failure.

## 19. Evidence before optimization

Do not introduce arenas, slabs, SIMD, assembly, hash tables, caching, concurrency, or other complexity because they sound faster.

Measure the representative workload first.

The optimization decision must include:

- end-to-end impact;
- memory impact;
- security impact;
- portability;
- test burden;
- maintenance cost.

See the allocation and assembly ADRs for current decisions.

## 20. Determinism

Where the public contract requires reproducible results, implementation choices must not leak nondeterministic ordering or behavior.

A faster data structure that makes public results unstable is not automatically an improvement.

## 21. Reversibility matters

Prefer small, isolated changes while a design is still experimental. Expensive-to-reverse decisions affecting ABI, security boundaries, persistence, concurrency, or domain semantics should be recorded as ADRs before deep implementation.

## 22. Principal-engineer decision record

For every significant architectural change, record:

- problem;
- context and constraints;
- alternatives considered;
- selected design;
- rejected alternatives and why;
- algorithm/data structure;
- mathematical bounds and complexity;
- ownership/lifetime;
- security consequences;
- testing strategy;
- performance evidence;
- operational consequences;
- compatibility/ABI impact;
- follow-up or re-evaluation trigger.

This is the expected engineering standard for pkgintel.
