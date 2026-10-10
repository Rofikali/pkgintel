# Branch Provenance and Anti-Duplication Record

> **Authoritative branch snapshot — 2026-10-10 UTC**
>
> This section is the current branch/PR inventory at the exact observed GitHub state below. Older checkpoints retained later in this file are historical records, not current branch-status claims. Re-query GitHub before each implementation or release decision.

## Canonical integration point

- Repository: `Rofikali/pkgintel`
- Default branch: `main`
- Observed `main` SHA: `044bf56bdeca14e9b23c5b6cf94cd301a5d27b5f`
- Latest commit: `fix: reject ambiguous JSON in reference consumer`
- PR #12: merged at this SHA.
- Current product stage: P0 foundation and P1 JSON export are merged; P2 reference-consumer validation exists and now rejects duplicate JSON keys and non-standard JSON constants.
- New work must start from the latest fetched `main`, not a historical feature branch.

## Branch inventory

GitHub reported **14 remote branches** in this snapshot. There are more than the three historical P0 branches. Branch names are retained refs, not evidence that work is active or missing from `main`.

| Remote branch | Observed head | PR / state | Classification and action |
|---|---|---|---|
| `main` | `044bf56bdeca14e9b23c5b6cf94cd301a5d27b5f` | Canonical | Only integration base for new work |
| `codex/p0-foundation` | `9a3ddd9f3dc0faa12eeec3bda30800df045f2518` | Historical; incorporated into cumulative P0 | Do not replay |
| `codex/p0-module-architecture` | `1a99cbe2e8bfeea427775a38877f70c34766c234` | PR #1 merged | Historical cumulative P0 implementation; do not replay |
| `codex/p0-post-merge-reconciliation` | `ee98282111136c2b37686bfbac756a36cf3b76c5` | PR #2 merged | Historical reconciliation; do not use as base |
| `codex/p1-capability-decision` | `a75214124d0d1027c1eb1e9eedd2a358832a84c1` | PR #7 merged | P1 JSON-export implementation history; inspect current main before any change |
| `codex/p1-product-charter` | `998f39a1b5d405a5d278a1bf70c27596c5539dcf` | PR #6 merged | Product charter is already on main |
| `codex/p2-reference-consumer` | `7ca3584e53b67d048e34fa967cb998323a1aa233` | PR #10 merged | Original reference-consumer experiment is already on main; strict parsing was added by PR #12 |
| `docs/finalize-current-main-pointer` | `b63fc970f14cbe4bb719ce3c31f916b64721e587` | PR #5 merged | Historical provenance docs |
| `docs/finalize-p0-provenance` | `ae5bf0e45ed9c7869504f4c51c9b3ea521594594` | PR #4 merged | Historical provenance docs |
| `docs/p1-json-export-handoff` | `268480032ca7c9ffd9f7832ecdba120e77da482c` | PR #8 merged | Handoff docs are on main; use current version |
| `docs/p2-consumer-validation-decision` | `289e7e6feaaf37acf8a5e7fd0f26b147395479e0` | PR #9 merged | P2 decision docs are on main; use current version |
| `docs/reconcile-agent-branch-state` | `9b738699427d5fa9857e5ca3698d9569a5b85f6f` | PR #3 merged | Historical agent/provenance docs |
| `docs/reconcile-current-branch-provenance` | `b01fe04d2689c1dec0602dc8bc44e7c7d4a4ec4f` | PR #11 open, draft | Stale snapshot based on old main SHA; superseded by this inventory and should not be merged unchanged |
| `fix/p2-strict-json-input` | `8ed81e2e3ec4e4253366278f1373a8007ecab49a` | PR #12 merged | Strict JSON parsing is on main; branch is historical |

## Pull-request ledger

| PR | Topic | State | Merge commit / head |
|---|---|---|---|
| #1 | P0 module architecture and security release candidate | Merged | `ec44a111cf215337d16e3c8800574248e109a3f1` |
| #2 | P0 post-merge reconciliation | Merged | `a9da03acda5bc60329f309df0c484c1859fc4a0b` |
| #3 | Agent branch-state reconciliation | Merged | `8f425bfd7e40fc047c80fa91f8a5cc54208ec313` |
| #4 | P0 provenance | Merged | `5fa5e9e89e93ea8320a5d92740906b4b23f5464b` |
| #5 | Current-main pointer | Merged | `6f49bb01c0264cce2a1fd2e4861ad668b60d7f15` |
| #6 | Product charter | Merged | `9bbbdbb237a1f7601686e818e1df2cbb416f905a` |
| #7 | P1 JSON-export capability | Merged | `8b4380d4f353017854a2104ac5a39a8029675c66` |
| #8 | P1 JSON-export handoff | Merged | `d1ea8908576e90a048943f2710acfe72789754ff` |
| #9 | P2 consumer-validation decision | Merged | `9a8391689d723a7721746bb972c953493ff45d45` |
| #10 | Internal downstream reference consumer | Merged | `95bbfb5e4088dd600a62babc790a77fad9cf5bd0` |
| #11 | Branch provenance / anti-duplication | Open draft; stale base snapshot | Head `b01fe04d2689c1dec0602dc8bc44e7c7d4a4ec4f` |
| #12 | Strict JSON parsing in reference consumer | Merged | `044bf56bdeca14e9b23c5b6cf94cd301a5d27b5f` |

PR status is authoritative for whether a change was merged. A retained branch can still point to the original pre-squash commit after its PR was merged.

## How to interpret branch comparisons safely

This repository uses squash merges. Therefore, commit-graph ahead/behind counts alone can mislead: the original feature branch commits may not be ancestors of the squash-merge commit even though their content was integrated. Before replaying anything:

1. Check the PR state and merge commit.
2. Compare the branch's actual file tree with current `main`.
3. Inspect the current implementation, tests, documentation, and exact verification SHA.
4. Cherry-pick/reimplement only a demonstrably missing change, never just because Git reports commits on a retained branch.

The three P0 branches are historical, not three competing active development lines. The other P1/P2/documentation branches listed above are also historical except the stale draft PR #11, which must be reconciled or closed. No historical branch should be deleted until branch provenance is confirmed and deletion is deliberately chosen; this task does not delete branches.

## Anti-duplication gate before each change

Before editing source or tests, establish:

- current remote `main` SHA and working branch;
- all relevant open/merged PRs and branch heads;
- the introducing commit and current code path;
- existing tests and ADRs;
- what exact property remains unimplemented or unverified;
- whether the gap is code, contract, test, documentation, platform evidence, release operations, or product validation;
- the precise evidence SHA and whether it remains applicable.

If behavior exists but evidence is missing, add evidence rather than a second implementation. If code is wrong, fix the existing path. If no concrete user/business requirement justifies a new production capability, do not invent one merely to create activity.

## Verification and platform evidence

The user's authoritative runtime is Windows 11 host → Docker Desktop → Ubuntu 24.04. Provide exact commands for the user to run when a claim depends on that runtime. Never claim those commands ran here.

For security evidence, record source SHA, container/image, mounts, privilege/capabilities, kernel, build directory, command, result, and limitations. `SKIP` is not `PASS`. Reuse earlier evidence only after checking production-relevant source/configuration deltas.

## Staff/principal, security, and business decision discipline

- **Staff/principal engineering:** architecture, ownership/lifetime, API/ABI, portability, resource bounds, maintainability, release sequencing.
- **Principal security engineering:** attacker-controlled input, trust boundaries, fail-closed behavior, memory/integer safety, exhaustion behavior, runtime assumptions, strength of evidence.
- **CA/finance:** incremental cost, operational burden, maintenance and opportunity cost when economically material; do not invent ROI without data.
- **MBA/management/product:** user value, dependency ordering, delivery risk, support ownership, scope, and reversibility when choosing priorities.

Apply each lens where it changes the decision; avoid ceremonial documentation that does not improve correctness or business outcomes.

## Historical checkpoints

The remainder of this file preserves prior branch/release notes for provenance. Any branch heads, statuses, current-main pointers, or "current" labels in that historical material refer to their original checkpoints and are not authoritative over the snapshot above.


## Repository

Rofikali/pkgintel

## Purpose

This document is the canonical branch/work provenance record for pkgintel.

Branch history is engineering context. Before changing code, an engineer or coding agent must establish what branch contains the work, which commits introduced it, what has already been verified, and what remains.

The purpose is to prevent:

- duplicate implementation;
- repeated security fixes;
- resurrecting rejected designs;
- conflicting implementations;
- invalid evidence reuse;
- accidental work from an obsolete checkout;
- unnecessary review and merge churn.

## Current branch topology

Post-merge integration is now complete. The development branches were cumulative, while `main` is the canonical post-merge line.

```text
main @ 8f425bfd...
  ^
  | PR #1 merge
  |
codex/p0-module-architecture @ 1a99cbe2...
  ^
  |
codex/p0-foundation @ 9a3ddd9f...
```

Verified: `codex/p0-foundation` is an ancestor of `codex/p0-module-architecture`; PR #1 intentionally integrated the cumulative module-architecture branch directly into `main`, so foundation did not require a separate merge event.


The repository historically used three relevant P0 branches; after merge, only `main` is canonical:

```
main
  |
  | +50 commits
  v
codex/p0-foundation
  |
  | +283 commits
  v
codex/p0-module-architecture
```

GitHub comparison currently establishes:

- `main` -> `codex/p0-foundation`: 50 commits ahead;
- `codex/p0-foundation` -> `codex/p0-module-architecture`: 265 commits ahead;
- `main` -> `codex/p0-module-architecture`: 315 commits ahead.

### Exact known base/current references

Post-merge integration references (final reconciled state):

```text
main
  8f425bfd7e40fc047c80fa91f8a5cc54208ec313

reviewed P0 head
  1a99cbe2e8bfeea427775a38877f70c34766c234

codex/p0-foundation
  9a3ddd9f3dc0faa12eeec3bda30800df045f2518
```


```
main
  003ac31c9f6b12e71a13d8958002a5263717ffe8

codex/p0-foundation
  9a3ddd9f3dc0faa12eeec3bda30800df045f2518

codex/p0-module-architecture
  c5225b8d817d8a8c7050d3f7645ed70e038b1680
```

The historical P0 release-candidate pull request was:

- PR #1;
- head: `codex/p0-module-architecture`;
- base: `main`;
- merge status: **merged**;
- merge commit: `ec44a111cf215337d16e3c8800574248e109a3f1`;
- merge time: `2026-10-08T15:00:15Z`;
- post-merge CI at the merge SHA: **PASS**.

## Branch responsibilities

### main

Canonical post-merge integration and development line.

`main` at `8f425bfd7e40fc047c80fa91f8a5cc54208ec313` is now the source of truth. New implementation work should branch from current `main` unless an explicit workflow decision says otherwise.

### codex/p0-foundation

Foundation line.

This branch contains the earlier foundational work and is an ancestor of the current module-architecture line.

Do not recreate foundation functionality on `codex/p0-module-architecture`.

### codex/p0-module-architecture

Historical cumulative P0 integration branch.

This branch contains the foundation work plus the later module architecture, API/ABI, DPKG parsing/correlation, target containment, resource governance, security verification, test architecture, fuzzing, performance evidence, and associated documentation. It was integrated into `main` through PR #1 and is no longer the active P0 development line.

## Post-merge branch lifecycle

`codex/p0-module-architecture` and `codex/p0-foundation` are retained temporarily for historical provenance. Branch names are references; Git history is the permanent record. Once provenance/release documentation is reconciled, these merged/historical branches may be deleted without rewriting the commits now reachable from `main`.

## Current local/remote provenance issue (historical checkpoint)

An earlier checkpoint observed a local/remote mismatch. That mismatch was subsequently reconciled by fast-forwarding the Codespace checkout to the current remote release-candidate head.

Current observed remote release-candidate state:

```
branch:
codex/p0-module-architecture

remote HEAD:
c5225b8d817d8a8c7050d3f7645ed70e038b1680
```

The latest ten commits after production source checkpoint `946a7fc91c689dfbe408f254b81f20145ce9fa24` are documentation/provenance updates only. The synchronized developer checkout used for the earlier sanitizer evidence was `40b1d37d3eba5b3ba1d5207b495dbf00f3efb867`; the current remote head is later but has no production-relevant source delta.

The repository remote and the synchronized verification checkout are at the current release checkpoint. The latest commits are documentation/evidence updates only; no production-relevant source, public API/ABI, tests, or build configuration changed.

### Evidence rule

Every verification result must identify the exact source SHA.

Never write:

```
"pkgintel passed"
```

when the evidence was actually produced by a different commit.

Instead write:

```
Configuration:
Compiler:
Build type:
Source SHA:
Environment:
Command:
Result:
Evidence class:
Remaining limitations:
```

## Previously established work on the current release line

The current module-architecture line already contains substantial work. Existing repository documentation and CI identify, among other things:

- public API/module boundaries;
- DPKG parsing and package/file correlation contracts;
- bounded metadata materialization and resource limits;
- target-root containment;
- hostile-filesystem security verification;
- deterministic regression tests;
- ASan/UBSan verification;
- Clang/libFuzzer safety and coverage/effectiveness work;
- repeated performance and allocation measurements;
- evidence-gated optimization policy;
- explicit rejection/deferment of speculative assembly/SIMD and custom allocation strategies.

Future work must inspect these existing implementations, tests, ADRs, and evidence before adding another mechanism.

## P9.2 performance provenance

The current release-candidate PR records repeated performance evidence at:

```
5c36a768a58b5b85fbb8f41acbe6ebcffddc54b1
```

That evidence must not be silently transferred to an unrelated SHA.

If P9.3 or later changes code after that measurement, performance claims must be re-qualified as applicable.

## Anti-duplication protocol

Before implementing a requested fix:

1. Establish the current branch.
2. Establish the current local SHA.
3. Establish the current GitHub branch SHA when remote state matters.
4. Compare the branch with its ancestor.
5. Search commit history for the behavior/fix.
6. Inspect the current implementation.
7. Inspect relevant tests.
8. Inspect relevant ADRs and engineering-status records.
9. Identify whether the requested behavior is:
   - already implemented;
   - implemented but insufficiently tested;
   - implemented but insufficiently documented;
   - partially implemented;
   - incorrect;
   - absent.
10. Only then choose code changes.

If the feature already exists, do not implement it again.

If evidence is missing, add evidence rather than duplicate implementation.

If the implementation is incorrect, fix the existing implementation rather than creating a parallel path.

## Change provenance record

For each significant change, preserve this chain:

```
Finding
  ->
branch
  ->
previous SHA
  ->
introducing commit(s)
  ->
current behavior
  ->
violated invariant
  ->
required correction
  ->
new commit
  ->
verification
  ->
remaining evidence gap
```

For security findings, additionally record:

```
attacker capability
trust boundary
security property
evidence class
runtime assumptions
failure mode
```

For performance findings, additionally record:

```
workload
environment
baseline SHA
measurement method
statistical summary
bottleneck evidence
proposed change
post-change measurement
```

For CA/MBA/management decisions, additionally record where material:

```
business objective
cost
risk
delivery impact
operational ownership
opportunity cost
reversibility
decision
```

## Local environment provenance

The authoritative developer verification workflow is:

```
Windows 11 host
  ->
Docker Desktop
  ->
Ubuntu 24.04
  ->
pkgintel repository/build/test environment
```

When the claim depends on real Linux kernel/VFS/namespace/capability behavior, the assistant should provide exact commands for the developer to run in this environment.

The assistant must not invent or imply that a command was executed in the real developer environment.

The developer's returned output becomes the runtime evidence record.

## Release-gate provenance

A gate is not complete merely because related work exists.

Track separately:

- implementation;
- deterministic test;
- sanitizer/fuzz evidence;
- real Linux security evidence;
- ABI/install evidence;
- performance evidence;
- compiler/configuration evidence;
- operational/release evidence.

A skipped or unavailable test remains a gap.

A test on an earlier SHA remains evidence for that earlier SHA.

A green CI job does not replace a required real-runtime verification when the property depends on the kernel or privileged environment.

## Rule for future agents

A future engineer/agent must be able to answer all of these before changing code:

1. Which branch am I on?
2. What is its exact SHA?
3. What are its ancestors?
4. Which branch already contains the requested work?
5. Which commit introduced the relevant behavior?
6. What invariant is being protected?
7. What tests already exist?
8. What ADR or decision governs the behavior?
9. What evidence already exists?
10. At which SHA was that evidence produced?
11. What evidence is still missing?
12. What is the smallest correct next change?

If these questions cannot be answered from the repository and current runtime evidence, stop and establish provenance before implementation.

## Final rule

```
Know what exists.
Know where it exists.
Know why it exists.
Know which commit introduced it.
Know which SHA the evidence belongs to.
Do not duplicate work.
Do not reuse stale evidence silently.
Do not confuse implementation with verification.
Do not confuse a skipped security test with PASS.
```


## Final post-merge reconciliation

Current canonical main SHA at this documentation checkpoint: `5fa5e9e89e93ea8320a5d92740906b4b23f5464b`.

PR #2 and PR #3 completed documentation reconciliation after PR #1. PR #3 updated the agent contract to make current `main` the canonical development line. The historical P0 branches may be removed as branch references when repository maintenance permits; no implementation work depends on keeping them alive.
