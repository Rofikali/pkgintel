# Principal/Staff Engineer Workflow

## Purpose

This is the operational workflow for future humans and coding agents working on pkgintel.

It converts the engineering principles into a repeatable process.

## Phase 0 — Establish reality

Before touching code:

~~~
repository
  -> branch
  -> commit
  -> ancestry
  -> working tree
  -> existing implementation
  -> tests
  -> ADRs
~~~

Never begin from memory or from an old conversation summary when current repository evidence is available.

## Phase 1 — Establish business/product truth

Before technical design, state what success means outside the code:

- who the user/customer/operator is;
- what problem the product must solve;
- measurable acceptance criteria and non-goals;
- cost, deployment, support, legal/compliance, and operational constraints;
- failure impact and acceptable risk;
- whether the change creates, protects, or merely maintains product value.

Do not optimize an implementation before establishing the product outcome it is meant to serve.

## Phase 2 — Build the domain model

Define the real domain entities, states, evidence, relationships, ownership, and invariants. Distinguish observed facts from derived conclusions. If the domain model is wrong, a correct implementation will still produce the wrong product.

## Phase 3 — HLD / LLD

Establish the high-level boundary and then the low-level contracts before implementation. HLD answers *who owns what and how components interact*; LLD answers *what each interface guarantees, including lifetime, errors, resources, and state transitions*.

## Phase 4 — Mathematical/resource invariants

State quantitative limits and conservation rules before coding: integer bounds, maximum record sizes, aggregate budgets, complexity, capacity, timeouts, and failure accounting.

## Phase 5 — Security/evidence model

Define the threat model, trust assumptions, attacker capabilities, security boundary, evidence strength, and fail-closed behavior. A conclusion must never be stronger than the observation that supports it.

## Phase 6 — Classify the work

Every finding must be classified as one or more of:

- implementation defect;
- API/ABI contract defect;
- security defect;
- resource-governance defect;
- test defect;
- documentation defect;
- evidence/verification gap;
- performance problem;
- operational problem;
- product/business problem.

This prevents fixing documentation when code is wrong, or changing code when only evidence is missing.

## Phase 7 — State the invariant

Write the invariant in plain language and, where useful, mathematically.

Examples:

~~~
Only ENOENT establishes definitive absence.

No filesystem correlation evidence means consistency is UNKNOWN,
not CONSISTENT.

A target-relative path must not cross the target root or a prohibited
filesystem boundary.

A resource limit must be enforced before the resource is consumed.
~~~

If the invariant cannot be stated clearly, the implementation should not be changed yet.

## Phase 8 — Threat and failure analysis

For security-sensitive code consider malicious metadata, malformed bytes, long records, integer overflow, allocation failure, permission changes, symlink races, mount changes, magic links, special files, stale descriptors, unexpected errno values, kernel/platform differences, denial of service, and information leakage.

For management/business decisions additionally consider cost, delivery risk, operational ownership, customer impact, opportunity cost, and reversibility.

## Phase 9 — Inspect before designing

Search source, public headers, internal headers, tests, ADRs, API/security documentation, and recent branch history.

A new state field or helper must not be introduced if an existing authoritative representation can express the same invariant safely.

Avoid duplicated sources of truth.

## Phase 10 — Design the smallest correct change

Preferred order:

1. clarify contract if ambiguous;
2. fix implementation;
3. add regression tests;
4. update documentation;
5. add an ADR if the decision is expensive to reverse.

Do not mix unrelated refactors into a security fix.

## Phase 11 — Implement and test

First run the smallest relevant test.

Then:

~~~
unit
  -> integration
  -> sanitizer
  -> fuzz
  -> security
  -> benchmark
  -> release matrix
~~~

Do not claim a later gate passed because an earlier gate passed.

## Phase 12 — ADR/API documentation

Record durable decisions and public-contract changes before the final runtime claim. Documentation must state the exact semantics, evidence level, known limitations, and verification procedure.

## Phase 13 — Real OS verification

If a property depends on Linux kernel/VFS/namespace/capability behavior, identify the exact environment requirement.

Use the developer's Ubuntu 24.04 environment inside Docker Desktop when appropriate.

Record:

- host assumption;
- container image;
- kernel version;
- user ID;
- capabilities;
- namespace state;
- command;
- expected result;
- actual result.

A privileged test that cannot run is SKIP_UNAVAILABLE, not PASS.

## Phase 14 — Review the diff as a Principal Engineer

After implementation inspect changed behavior, unchanged invariants, ownership, error paths, resource accounting, security boundary, ABI impact, test coverage, documentation consistency, complexity, performance, and operational consequences.

Ask:

> Did the patch solve the actual problem, or merely make the test green?

## Phase 15 — Business and management checkpoint

For changes with material architecture or operational cost, record:

~~~
benefit
vs
engineering cost
vs
runtime cost
vs
security risk
vs
maintenance cost
vs
reversibility
~~~

Reject complexity whose value cannot be demonstrated.

## Phase 16 — Handoff

Every significant change should finish with:

- branch;
- old SHA;
- new SHA;
- summary;
- files;
- tests;
- evidence level;
- remaining blockers;
- next action;
- explicit non-goals.

## Release decision

A release decision is a conjunction, not an average:

~~~
Release =
  Correctness
  AND Security
  AND Resource governance
  AND API/ABI contract
  AND Operational readiness
  AND Required verification evidence
~~~

A strong score in five areas does not compensate for a failed mandatory security gate.
