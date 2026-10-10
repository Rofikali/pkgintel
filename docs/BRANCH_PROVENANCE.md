# Branch Provenance and Anti-Duplication Record

> **Current-state addendum — 2026-10-10 UTC**
>
> This section supersedes older branch-head snapshots and historical statements below where they disagree. Branch names and heads are point-in-time observations; re-check Git refs before each new implementation or release decision.

## Current canonical source of truth

**Use `main` for new work.** The current GitHub `main` head at this snapshot is:

- Branch: `main`
- SHA: `95bbfb5e4088dd600a62babc790a77fad9cf5bd0`
- Latest commit: `test: validate P1 JSON through reference consumer`
- Product/release stage: P0 foundation merged; P1 versioned CLI JSON export merged; P2 downstream/reference-consumer validation is the current engineering gate.

The current source of truth is the latest `main` commit, not a retained feature branch or an older local checkout. Before implementation, fetch and inspect the latest `origin/main`, then branch from it. Do not merge or fast-forward a dirty worktree until local changes and untracked evidence have been inventoried and preserved.

## P0 branch reconciliation

| Branch | Observed remote head | Current role |
|---|---|---|
| `main` | `95bbfb5e4088dd600a62babc790a77fad9cf5bd0` | Canonical integration and new-work base |
| `codex/p0-foundation` | `9a3ddd9f3dc0faa12eeec3bda30800df045f2518` | Historical foundation branch; its work is incorporated through the cumulative P0 branch |
| `codex/p0-module-architecture` | `1a99cbe2e8bfeea427775a38877f70c34766c234` | Historical cumulative P0 branch; merged to `main` through PR #1 |
| `codex/p0-post-merge-reconciliation` | `ee98282111136c2b37686bfbac756a36cf3b76c5` | Historical P0 documentation-reconciliation branch; not the current integration line |

The observed P0 history establishes that `codex/p0-foundation` is an ancestor of `codex/p0-module-architecture`. PR #1 merged the cumulative module-architecture line into `main` at merge commit `ec44a111cf215337d16e3c8800574248e109a3f1`. Do not replay the foundation or module-architecture implementation on top of current `main`.

At this snapshot, GitHub comparison reports that `main` is 18 commits ahead of `codex/p0-post-merge-reconciliation`. That branch is retained historical context, not a suitable base for new work. Its five P0 post-merge documentation commits are already superseded by later mainline product, P1, and P2 work.

## Other observed work branches

The following remote refs were also present during this audit:

| Branch | Observed head | Interpretation |
|---|---|---|
| `codex/p1-capability-decision` | `a75214124d0d1027c1eb1e9eedd2a358832a84c1` | Historical P1 capability/JSON-export implementation branch; JSON export is merged to `main` |
| `codex/p1-product-charter` | `998f39a1b5d405a5d278a1bf70c27596c5539dcf` | Historical product-charter branch; charter is present on `main` |
| `codex/p2-reference-consumer` | `7ca3584e53b67d048e34fa967cb998323a1aa233` | Historical P2 consumer-validation branch; inspect current `main` before continuing |
| `docs/finalize-current-main-pointer` | `b63fc970f14cbe4bb719ce3c31f916b64721e587` | Historical documentation branch |
| `docs/finalize-p0-provenance` | `ae5bf0e45ed9c7869504f4c51c9b3ea521594594` | Historical documentation branch |
| `docs/p1-json-export-handoff` | `268480032ca7c9ffd9f7832ecdba120e77da482c` | Historical documentation handoff branch |
| `docs/p2-consumer-validation-decision` | `289e7e6feaaf37acf8a5e7fd0f26b147395479e0` | Historical P2 decision branch |
| `docs/reconcile-agent-branch-state` | `9b738699427d5fa9857e5ca3698d9569a5b85f6f` | Historical agent/provenance reconciliation branch |

These rows are a snapshot, not a permanent assertion that each branch is open, merged, or safe to delete. Confirm pull-request state and ancestry before deleting any ref. The commits remain the provenance record.

## Current anti-duplication decision

Before implementing a requested capability:

1. Check the actual local branch, `HEAD`, worktree, untracked files, remotes, and fetched remote heads.
2. Treat `main` as canonical only after fetching it and confirming the current remote SHA.
3. Search `docs/ENGINEERING_STATUS.md`, `docs/RELEASE_EVIDENCE.md`, `docs/BRANCH_PROVENANCE.md`, the relevant ADRs, tests, and the P1/P2 handoff documents.
4. Compare the proposed change with the existing implementation and its verification SHA.
5. Classify the gap as implementation, contract, test, documentation, evidence, operational, or product/business work.
6. Reuse valid evidence only after proving that the production-relevant source, configuration, toolchain, and runtime assumptions have not changed.
7. Create one focused branch from the latest `main`; do not implement the same feature independently on historical branches.

The local developer checkout observed during this audit was on `codex/p0-module-architecture` at `40b1d37d3eba5b3ba1d5207b495dbf00f3efb867`, with a modified `instructions.md` and numerous untracked build/install directories and fuzz-corpus files. This state is not equivalent to the current remote `main`. Preserve it until the developer has reviewed the diff and classified the untracked artifacts. Do not run cleanup or destructive checkout/reset commands as part of branch reconciliation.

## Verification provenance boundary

Security-test results from the dedicated `pkgintel-security-verify` container apply to the exact source/build state actually tested. The normal development container is named `pkgintel-dev`; `pkgintel` is the Docker Compose project/service naming context, not the actual development container name. Record container name, image, mounts, privilege, OS/kernel, source SHA, build directory, and exact command with each runtime result.

This audit observed five passing configurations for the genuine mount-boundary/procfs magic-link test on the local verification checkout. Those results are evidence for that test surface and tested source state; they are not proof that all pkgintel security properties or the current `main` tree have been reverified.



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
