# Principal / Staff Engineering Review Protocol

## Purpose

This document is the reusable review protocol for substantial pkgintel engineering work.

It answers a recurring question:

> What should a Staff/Principal Engineer systematically discuss, verify, document, and challenge every time a material capability or architectural change is proposed?

The answer is not "apply every engineering buzzword every time."

The Principal standard is:

> apply the right engineering disciplines at the right depth, make the reasoning explicit, preserve the evidence, and avoid unnecessary complexity.

Architecture decision records preserve why a choice was made, what alternatives were rejected, and what would cause reconsideration. This protocol therefore treats decision reasoning as a durable engineering artifact.

## 1. Mandatory mental model

For every material change, reason through:

```
Repository reality
    ↓
Business / product problem
    ↓
Domain model
    ↓
Requirements + non-goals
    ↓
HLD
    ↓
LLD
    ↓
Invariants
    ↓
Mathematics + resource model
    ↓
Algorithms + data structures
    ↓
Design principles
    ↓
Design patterns, only if justified
    ↓
Memory / ownership / lifetime
    ↓
Concurrency / synchronization
    ↓
Security / threat model
    ↓
API / ABI / FFI boundary
    ↓
Implementation
    ↓
Tests / sanitizers / fuzzing
    ↓
Real runtime verification
    ↓
Benchmark / capacity evidence
    ↓
Documentation / ADR
    ↓
Release / operational / business decision
```

Not every change needs equal depth at every stage. A one-line typo fix does not need a mathematical proof. A new parser, public API, security boundary, concurrency mechanism, persistent format, or major performance optimization does.

## 2. Establish reality before reasoning

Never begin from memory.

Before changing code:

### Repository

- What repository and exact branch are we on?
- What is the exact HEAD SHA?
- Is local state different from remote state?
- Is the requested capability already implemented?
- Is there an existing branch, PR, commit, ADR, test, or partial implementation?
- Is the requested change duplicating earlier work?

### Provenance

Record:

```
base branch
base SHA
working branch
working SHA
ancestor relationship
relevant existing commits
prior evidence SHA
```

### Evidence reuse

Before rerunning an expensive gate:

1. find the latest evidence for the same property;
2. compare production-relevant changes since that evidence;
3. compare environment/toolchain/configuration;
4. decide whether the old evidence remains applicable;
5. rerun only when required.

Never create a newer timestamp merely to make old evidence look newer.

## 3. Business / Product / Management lens

Before asking "how do we build it?", ask:

### Problem

- Who has the problem?
- What exactly is painful or valuable?
- What happens if we do nothing?
- Is this a customer problem, operator problem, security problem, engineering problem, or all four?

### Outcome

Define measurable success.

Examples:

- lower scan time;
- more accurate evidence;
- machine-readable integration;
- lower operational cost;
- reduced attack surface;
- improved compatibility;
- reduced support burden.

### Scope

Explicitly state:

- in scope;
- out of scope;
- current phase;
- future phase.

### Economics

When material:

- engineering effort;
- infrastructure cost;
- dependency cost;
- maintenance cost;
- support cost;
- security/failure cost;
- opportunity cost;
- cost of delay;
- expected business value;
- build vs buy.

### Reversibility

Classify the decision:

- easily reversible;
- moderately reversible;
- expensive to reverse;
- effectively irreversible.

The less reversible the decision, the more deliberate the review should be.

## 4. Domain modeling

Before selecting a data structure, model what the system actually means.

Ask:

- What are the entities?
- What are their states?
- Which facts are observed?
- Which facts are derived?
- Which conclusions are heuristic?
- What is evidence?
- What is provenance?
- What is ownership?
- What is lifecycle?
- What is authoritative?
- What is merely a projection?

For pkgintel, preserve the conceptual separation:

```
Package
   ↓
Installation state
   ↓
Filesystem artifact evidence
   ↓
Diagnostic / explanation
```

Do not collapse distinct domain concepts merely because one C struct is convenient for the current implementation.

## 5. Requirements and invariants

For every non-trivial feature, write:

### Functional requirements

What must happen?

### Non-functional requirements

What quality must it have?

Examples:

- deterministic;
- bounded;
- secure;
- portable;
- observable;
- performant;
- maintainable.

### Non-goals

What must explicitly not be built?

### Invariants

What must always remain true?

Examples:

```
target-relative operations cannot escape target
unsupported options are rejected
snapshot remains internally consistent
public ownership rules remain valid
JSON output never becomes a second source of truth
resource accounting cannot overflow
```

If we cannot state the invariant, we are not ready to implement.

## 6. HLD review

At HLD level ask:

- What are the system boundaries?
- Which module owns each responsibility?
- What is the dependency direction?
- Where is the trust boundary?
- Where is the resource boundary?
- Which interface is public?
- Which interface is private?
- What data flows between modules?
- Which component is authoritative?
- What can fail independently?
- What can be changed later without breaking consumers?

For pkgintel:

```
CLI / presentation
        ↓
application orchestration
        ↓
domain + scan model
        ↓
evidence / diagnostics
        ↓
backend interfaces
   ↓       ↓
 dpkg   filesystem
```

A new feature should fit this architecture or explicitly justify changing it.

## 7. LLD review

For each important function/module specify:

- inputs;
- preconditions;
- outputs;
- postconditions;
- ownership;
- lifetime;
- invalidation;
- errors;
- partial-result behavior;
- resource accounting;
- state transitions;
- security assumptions;
- complexity;
- tests.

The LLD must be concrete enough that another engineer can implement it without inventing missing contracts.

## 8. DSA: Data Structures

Data structures are not interview exercises. They are resource, security, determinism, and maintainability decisions.

For every important structure ask:

### What data structure is required?

Examples:

- contiguous vector;
- linked list;
- hash table;
- ordered tree;
- heap;
- queue;
- stack;
- graph;
- arena;
- streaming buffer.

### Why this one?

Evaluate:

- lookup cost;
- insertion cost;
- deletion cost;
- iteration cost;
- ordering;
- memory overhead;
- cache locality;
- allocation behavior;
- adversarial behavior;
- deterministic output;
- implementation complexity.

### Questions that must be answered

- Is ordering required?
- Is stable ordering required?
- Can input order be trusted?
- Does deterministic output require sorting?
- Can duplicates occur?
- How are duplicates represented?
- What is the maximum element count?
- What is the maximum capacity?
- What happens when allocation fails?
- Can the structure be streamed instead of materialized?

Do not choose a hash table merely because average O(1) sounds better. If deterministic ordered output is required and N is modest, a vector plus sort may be simpler and safer.

## 9. Algorithms

For each significant algorithm document:

```
input
→ transformation
→ output
```

Then determine:

- best case;
- expected case;
- worst case;
- time complexity;
- space complexity;
- I/O complexity;
- allocation behavior;
- adversarial behavior;
- determinism;
- failure behavior.

Typical examples:

### Linear scan

[
T(N)=O(N)
]

Useful when N is bounded and the simpler design wins.

### Sorting

[
T(N)=O(Nlog N)
]

Potentially justified when deterministic canonical ordering is required.

### Hash lookup

Expected:

[
O(1)
]

But worst-case behavior, memory overhead, hash flooding, and deterministic behavior must be considered.

### Streaming

If output can be emitted without retaining all records:

[
Space = O(1) 	ext{ or } O(B)
]

instead of:

[
Space = O(N)
]

where B is a bounded buffer size.

The choice must follow the product contract and evidence, not a desire for theoretical sophistication.

## 10. Mathematics and quantitative reasoning

Mathematics is used to prevent invalid assumptions.

### Arithmetic safety

Before multiplication:

[
N leq rac{SIZE_MAX}{S}
]

Before addition:

[
used leq limit-required
]

Do not evaluate the potentially overflowing expression first.

### Capacity

For a dynamic array:

- initial capacity;
- growth factor;
- maximum representable capacity;
- allocation byte count;
- number of reallocations;
- failure behavior.

### Resource budget

Every resource limit should define:

```
unit
scope
initial budget
consumption event
exhaustion condition
result/status
partial-result semantics
```

### Complexity

Separate:

- calculated bound;
- measured result;
- assumption;
- estimate;
- hypothesis.

Never call an estimate a guarantee.

## 11. Design principles / SOLID

Use principles as decision tools.

### Single Responsibility

Ask:

> What is this module's reason to change?

If a serializer changes when filesystem confinement changes, boundaries are probably wrong.

### Open / Closed

Ask whether new implementations can be added behind an existing contract where that is genuinely useful.

Do not build plugin frameworks before there are real extension requirements.

### Liskov Substitution

An implementation must preserve the entire contract:

- return values;
- errors;
- ownership;
- lifetime;
- resource semantics;
- security guarantees.

### Interface Segregation

Consumers should depend only on what they need.

Do not expose a huge internal interface when a small contract is sufficient.

### Dependency Inversion

High-level policy should not become coupled to platform details unnecessarily.

In C this can be achieved with:

- opaque handles;
- function tables;
- small interfaces;
- private headers;
- composition.

## 12. Design patterns

The Principal rule is:

> Do not start with a pattern. Start with a constraint.

Use a pattern only when it makes the design clearer or more robust.

Potential pkgintel examples:

### Adapter

Useful when translating dpkg-specific representation into the generic domain model.

### Strategy-like interface

Useful if multiple package-manager implementations genuinely share a stable contract.

### Factory / constructor

Useful for controlled opaque-object creation and invariant establishment.

### Facade

The public API can act as a controlled facade over internal modules.

### Builder-style staged construction

Useful when construction naturally follows:

```
validate
→ reserve
→ construct
→ commit
```

### Iterator / traversal abstraction

Potentially useful for future filesystem traversal.

### Registry

Only justified if runtime backend/plugin discovery is actually required.

Do not introduce:

- fake inheritance;
- generic dependency-injection systems;
- plugin registries;
- abstract factories;
- framework-like layers

without a concrete problem they solve.

A rejected pattern is itself a valid engineering decision when the rejection prevents unnecessary complexity.

## 13. Memory / ownership / lifetime

For C, this section is mandatory for material changes.

For every pointer or object answer:

- Who allocates it?
- Who owns it?
- Who may borrow it?
- How long is it valid?
- When is it freed?
- Can it be invalidated?
- What happens if construction fails halfway?
- What happens during teardown?
- Can two objects alias it?
- Is mutation allowed?
- Is thread-safe read-only access possible?

Prefer explicit ownership.

Use failure-atomic construction:

```
validate
→ reserve
→ construct
→ commit
```

Avoid:

```
partially mutate
→ discover failure
→ repair everything
```

Every failure path must have a cleanup story.

## 14. Concurrency

Do not add concurrency because "production systems should be concurrent."

Ask:

- Is concurrency actually required?
- What workload justifies it?
- Is the domain immutable?
- Can independent contexts run concurrently?
- Which objects are mutable?
- What is shared?
- Who owns synchronization?
- What is the contention point?
- What is the shutdown protocol?
- What happens to queued work?
- Are there cancellation semantics?
- Can concurrency amplify resource exhaustion?
- Does concurrency preserve deterministic output?

If immutable snapshots can be safely read by multiple consumers, adding locks may only increase complexity.

If concurrency is introduced, document:

```
thread ownership
shared state
synchronization
memory ordering
lifecycle
shutdown
cancellation
backpressure
resource limits
```

## 15. Security / Principal Security review

Every system-facing feature asks:

### Attacker

- What can the attacker control?
- Bytes?
- Paths?
- Files?
- Package metadata?
- CLI arguments?
- Environment?
- Resource consumption?

### Trust boundaries

Identify:

```
untrusted input
→ validation
→ parser
→ normalized domain
→ output
```

### Threats

Consider, as applicable:

- path traversal;
- symlink substitution;
- magic links;
- mount/bind-mount crossing;
- TOCTOU;
- special files;
- malformed metadata;
- integer overflow;
- memory corruption;
- allocation exhaustion;
- CPU exhaustion;
- descriptor exhaustion;
- information disclosure;
- privilege escalation;
- process execution.

### Security truthfulness

Always preserve:

```
SKIP != PASS
could not observe != does not exist
permission denied != present
no correlation != consistency
fixture simulation != real kernel evidence
```

The strength of the security claim cannot exceed the strength of the evidence.

## 16. Resource governance

Treat resource limits as part of correctness and security.

For each attacker-amplifiable resource:

- package count;
- artifact count;
- diagnostic count;
- record size;
- path length;
- bytes read;
- files examined;
- directory depth;
- allocations;
- descriptors;
- CPU time;
- wall-clock time;
- output size.

State:

```
limit
accounting point
overflow handling
exhaustion result
cleanup behavior
partial-result policy
```

An unbounded parser is a security issue even if it is functionally correct.

## 17. API / ABI / FFI

Before changing public interfaces ask:

- Is this really public?
- Can it remain internal?
- What is the ownership contract?
- What is the lifetime?
- What are error semantics?
- Are enums stable?
- Is struct layout exposed?
- Is struct_size needed?
- What happens to old consumers?
- What happens to future consumers?
- What is the symbol visibility?
- Does SONAME policy change?
- Does Rust FFI eventually depend on this?

Do not expose speculative functionality merely because it may be useful later.

Public ABI is a long-term compatibility commitment.

## 18. Serialization / external formats

For any JSON, binary, text, or protocol output:

- Is the format a projection or the source of truth?
- Is schema version independent from native ABI?
- Are enums stable?
- Is output deterministic?
- How are arbitrary Linux bytes represented?
- What is the maximum output size?
- Can output be streamed?
- What happens on partial serialization?
- Can consumers distinguish complete vs partial scans?
- Are diagnostics machine-readable?
- Does serialization re-touch the target?
- Does serialization execute anything?
- Does serialization introduce new trust boundaries?

Never silently convert arbitrary bytes into lossy Unicode.

## 19. Testing strategy

Every material feature should map requirements to evidence.

Minimum applicable categories:

- happy path;
- empty input;
- malformed input;
- missing input;
- permission denial;
- resource exhaustion;
- boundary arithmetic;
- ownership/lifetime;
- deterministic output;
- regression behavior;
- hostile security input.

Then choose evidence layers:

```
source inspection
→ unit/integration
→ sanitizer
→ fuzz
→ real Linux runtime
→ benchmark
→ release/consumer verification
```

A test proves only what it actually exercises.

## 20. Fuzzing

Fuzz whenever the code parses or transforms attacker-controlled bytes and the cost is justified.

For a parser/fuzzer document:

- input domain;
- invariants;
- maximum input size;
- sanitizer configuration;
- timeout/resource limits;
- crash handling;
- corpus strategy;
- coverage expectations;
- known parser boundaries.

Fuzzing is evidence for robustness against malformed input; it is not proof of absence of all vulnerabilities.

## 21. Performance

Performance work follows:

```
measure
→ attribute
→ model
→ change
→ benchmark
→ compare
→ decide
```

Measure where applicable:

- wall time;
- CPU time;
- allocations;
- peak RSS;
- bytes read;
- syscalls;
- package count;
- artifact count;
- output size.

For every optimization ask:

- What bottleneck was measured?
- How large is the gain?
- Is it end-to-end?
- What memory cost was introduced?
- What security surface changed?
- What portability changed?
- What maintenance burden changed?
- Is the gain worth the complexity?

Do not optimize a benchmark number that does not matter to the product.

## 22. Reliability / failure behavior

For every failure ask:

- Does the process continue?
- Does the operation become partial?
- Is partial state observable?
- Can the result be safely consumed?
- Are resources released?
- Is retry safe?
- Is the failure deterministic?
- Is the error actionable?
- Is data lost?
- Is the security boundary preserved?

Important state transitions should be explicit.

## 23. Observability

When operationally material:

- What should be logged?
- What should never be logged?
- Which metrics prove health?
- Which metrics reveal resource pressure?
- Which events need tracing?
- What identifiers are safe?
- Can logs leak target paths or sensitive package information?
- Are counters bounded?
- Does observability itself create meaningful overhead?

Do not log attacker-controlled paths as if they were trusted identifiers.

## 24. Portability and platform contract

Ask:

- Which Linux versions are supported?
- Which kernel primitive is required?
- Is behavior compiler-specific?
- Is behavior architecture-specific?
- Is an extension being used?
- Is there a fallback?
- What happens when the primitive is unavailable?

If a security claim depends on Linux kernel/VFS behavior, source-level tests alone are insufficient.

## 25. Documentation / ADR

A material decision should leave durable reasoning.

An ADR should normally capture:

```
Context
Decision
Alternatives
Trade-offs
Consequences
Verification
Revisit conditions
```

Record the why, not merely the what. Keep one architectural decision per record and preserve superseded decisions rather than silently rewriting history.

Not every code change needs an ADR.

Use one when the decision:

- changes architecture;
- changes public contracts;
- introduces material security risk;
- introduces significant technical debt;
- is expensive to reverse;
- is contested;
- selects a technology/dependency;
- materially affects operations or economics.

## 26. CA / Finance review

When material, explicitly discuss:

```
CAPEX
OPEX
engineering effort
maintenance cost
support cost
security/failure cost
vendor/dependency cost
opportunity cost
ROI
cost of delay
total cost of ownership
```

A faster parser that requires a complex dependency may reduce CPU cost but increase supply-chain risk, patching burden, build complexity, platform incompatibility, and long-term maintenance. The correct decision is the best risk-adjusted total outcome.

## 27. MBA / Management / Product review

For material roadmap decisions:

- Why this feature now?
- What does it unblock?
- What does it delay?
- Who owns it?
- Who operates it?
- Who supports it?
- What is the critical path?
- What dependency could block delivery?
- What is the minimum useful scope?
- What can be deferred safely?
- What is the customer/business outcome?
- What would make us stop?

Principal engineering includes saying no to technically interesting work when it does not serve the current product boundary.

## 28. Explicit alternatives

Never document only the chosen design.

At minimum consider, where meaningful:

```
A: simplest implementation
B: alternative data structure/algorithm
C: alternative architecture
D: defer / do nothing
```

For each alternative record:

- correctness;
- security;
- complexity;
- performance;
- memory;
- compatibility;
- cost;
- reversibility.

The winning design should be explainable without saying:

> "This is how we usually do it."

## 29. What should be deliberately rejected

Principal engineering is partly the discipline of refusing unnecessary complexity.

Ask explicitly:

- Do we need a new abstraction?
- Do we need a design pattern?
- Do we need concurrency?
- Do we need caching?
- Do we need a database?
- Do we need a dependency?
- Do we need a new public API?
- Do we need a new service?
- Do we need a new thread?
- Do we need an optimization?
- Do we need a framework?

If not, document the rejection briefly.

Negative decisions are valuable because they prevent future engineers from reopening the same question without new evidence.

## 30. Completion checklist

### Reality

- [ ] exact branch known
- [ ] exact SHA known
- [ ] existing implementation/history inspected
- [ ] prior evidence checked

### Product

- [ ] problem defined
- [ ] measurable outcome defined
- [ ] non-goals defined
- [ ] cost/priority considered where material

### Architecture

- [ ] HLD reviewed
- [ ] LLD reviewed
- [ ] domain model correct
- [ ] boundaries explicit

### DSA / algorithms

- [ ] data structures justified
- [ ] algorithm justified
- [ ] deterministic ordering considered
- [ ] worst-case complexity considered
- [ ] adversarial behavior considered

### Mathematics

- [ ] integer overflow checked
- [ ] capacity bounds defined
- [ ] resource limits defined
- [ ] measured facts separated from assumptions

### Design principles

- [ ] cohesion/coupling reviewed
- [ ] SOLID applied where useful
- [ ] DRY applied to knowledge, not syntax
- [ ] unnecessary abstraction rejected

### Patterns

- [ ] pattern need identified before pattern selection
- [ ] alternatives considered
- [ ] unnecessary patterns rejected

### Memory / concurrency

- [ ] ownership documented
- [ ] lifetime documented
- [ ] failure cleanup verified
- [ ] concurrency justified or explicitly rejected
- [ ] shutdown/cancellation reviewed if concurrent

### Security

- [ ] attacker model reviewed
- [ ] trust boundaries reviewed
- [ ] parser/path/resource threats reviewed
- [ ] fail-closed behavior reviewed
- [ ] evidence strength matches claim

### API / ABI

- [ ] public/private boundary reviewed
- [ ] ownership/error semantics reviewed
- [ ] compatibility impact reviewed
- [ ] ABI evidence updated if applicable

### Verification

- [ ] focused tests
- [ ] regression tests
- [ ] sanitizer/fuzz where applicable
- [ ] real Linux runtime where applicable
- [ ] benchmark where applicable

### Documentation

- [ ] HLD/LLD updated
- [ ] API/security docs updated
- [ ] ADR added if material
- [ ] status/provenance updated
- [ ] known limitations recorded

### Decision

- [ ] implementation result known
- [ ] evidence result known
- [ ] remaining gaps explicit
- [ ] next gate explicit
- [ ] release decision not stronger than evidence

## 31. Short version: questions to repeat every time

For day-to-day work, the full protocol can be compressed to these questions:

1. What problem are we solving?
2. What already exists?
3. What invariant must remain true?
4. What is the simplest correct design?
5. What domain model are we protecting?
6. Which data structures and algorithms are justified?
7. What are the mathematical and resource bounds?
8. Who owns every object and how long does it live?
9. Do we actually need concurrency?
10. Which design principles apply?
11. Do we actually need a design pattern?
12. What can an attacker control?
13. What is the trust boundary?
14. What can fail, and what happens then?
15. Does the public API/ABI remain truthful?
16. What evidence proves the claim?
17. What evidence is still missing?
18. What did we deliberately not build?
19. What does this cost to build, run, maintain, and support?
20. What would make us reverse the decision?

If these questions are answered honestly, the work is being approached as engineering rather than merely coding.

## 32. Relationship to existing pkgintel documents

This protocol does not replace existing contracts.

Use:

- `docs/ENGINEERING_OPERATING_MODEL.md` for the overall operating model;
- `docs/ENGINEERING_PRINCIPLES.md` for principles;
- `docs/ALGORITHMS.md` for algorithms/data structures;
- `docs/MATHEMATICS.md` for quantitative invariants;
- `docs/ENGINEERING_GATES.md` for release gates;
- `docs/ARCHITECTURE.md` for architecture;
- `docs/HLD.md` / `docs/LLD.md` for design;
- `docs/API_CONTRACT.md` for public API;
- `docs/SECURITY.md` for security;
- ADRs for individual significant decisions.

This document is the repeatable review protocol connecting those disciplines.

## 33. Final rule

The Staff/Principal standard is not:

> "Know every technology."

It is:

> **Know what matters, know why it matters, choose the smallest design that satisfies the real constraints, understand the trade-offs, verify the claims at the correct evidence level, and leave enough durable reasoning that the next engineer does not have to rediscover it.**

That is the standard this repository should apply to future work.
