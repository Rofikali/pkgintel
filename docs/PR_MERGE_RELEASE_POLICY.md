# PR, Review, Merge, and Release Policy

## Purpose

This document defines the pull-request and merge discipline for pkgintel.

pkgintel is a security-sensitive systems project. A GitHub pull request is a review and integration mechanism; it is not, by itself, proof that a release candidate is safe or complete.

The release decision is bound to an exact source SHA and its applicable evidence set.

## 1. Branch responsibility

The current three-branch topology is:

```
main
  |
  v
codex/p0-foundation
  |
  v
codex/p0-module-architecture
```

### main

Historical/base integration line.

Do not use it as the implementation starting point for current P0 work when the required work already exists on an ancestor/release branch.

### codex/p0-foundation

Foundation line.

It contains earlier foundational work and is an ancestor of the current P0 integration line. Do not recreate its functionality on the cumulative branch.

### codex/p0-module-architecture

Current cumulative P0 release-candidate line.

It contains the foundation plus module architecture, public API/ABI work, DPKG semantics, resource governance, filesystem security, fuzzing, performance evidence, release documentation, and later verification work.

Current P0 release work continues here unless an explicit repository decision changes the release line.

## 2. PR identity

A release-candidate PR is identified by:

```
repository
base branch
head branch
exact head SHA
```

For PR #1:

- repository: `Rofikali/pkgintel`
- base: `main`
- head: `codex/p0-module-architecture`
- merge decision: blocked until all applicable mandatory release gates are PASS.

A PR must never be described merely as "passed" without stating which source SHA the evidence applies to.

## 3. PR is not release approval

These are separate states:

```
implementation complete
        !=
tests green
        !=
review complete
        !=
release gates complete
        !=
merge approved
```

A green CI result proves only the properties exercised by that CI job.

A required privileged Linux/VFS property cannot be promoted to PASS from an unprivileged skipped test.

## 4. Review layers

### Staff/Principal Software Engineering review

Review:

- architecture and module boundaries;
- ownership and lifetime;
- API/ABI compatibility;
- failure semantics;
- complexity and resource bounds;
- portability;
- test completeness;
- maintainability;
- operational readiness;
- reversibility;
- provenance and evidence quality.

### Principal Security Engineering review

Review:

- attacker-controlled input;
- trust boundaries;
- filesystem/VFS semantics;
- path traversal;
- symlinks and magic links;
- mount boundaries;
- capability and namespace assumptions;
- fail-closed behavior;
- resource exhaustion;
- memory/integer safety;
- information disclosure;
- whether the runtime actually exercised the claimed primitive.

### CA/Finance review

Apply when the change has material economic consequences:

- engineering cost;
- infrastructure/tooling cost;
- operational cost;
- maintenance burden;
- vendor dependency;
- risk-adjusted cost;
- opportunity cost;
- ROI/cash-flow impact.

Do not invent financial precision when reliable repository/business data is unavailable.

### MBA/Management/Product review

Apply when the decision affects scope, sequencing, delivery, or operations:

- customer/business value;
- priority;
- dependency ordering;
- delivery risk;
- support burden;
- operational ownership;
- roadmap impact;
- technical-debt implications;
- reversibility.

These dimensions are applied where material; they are not ceremony for trivial changes.

## 5. Evidence hierarchy

Evidence must be at least as strong as the claim:

1. Source verified
2. Unit/integration verified
3. Sanitizer/fuzz verified
4. Real runtime verified
5. Benchmark verified
6. Production-like verified

Never promote:

```
SKIP -> PASS
simulation -> real-runtime evidence
old SHA -> current SHA
CI coverage -> untested platform primitive
```

Evidence from an ancestor SHA is inherited only after a production-relevant applicability comparison.

## 6. Merge gate

The default release decision is:

```
MERGE =
  Correctness
  AND Security
  AND Resource governance
  AND API/ABI contract
  AND Required compiler/configuration evidence
  AND Required runtime evidence
  AND Performance evidence where applicable
  AND Documentation
  AND Known-limitations record
  AND Current-SHA provenance
  AND Final review
```

One mandatory failed or unavailable gate blocks merge.

A strong result in another category does not compensate for a mandatory security or release-gate failure.

## 7. Exact-SHA approval rule

Review and release evidence is bound to the exact reviewed source SHA.

If a PR moves from:

```
SHA-A -> SHA-B
```

after the relevant review/evidence, the final decision must determine whether the existing evidence remains applicable.

Requalification is required when any evidence applicability condition changes, including:

- production-relevant source;
- public API/ABI;
- tests affecting the property;
- compiler/toolchain;
- build flags/configuration;
- target OS/kernel/platform;
- security boundary or runtime capability;
- benchmark workload/method;
- verification command;
- release policy;
- prior evidence validity/completeness.

Documentation-only changes normally do not invalidate implementation evidence, but the evidence ledger must record the provenance and applicability decision.

## 8. Required pre-merge checklist

Before merging a release candidate, verify:

- [ ] exact branch and head SHA established;
- [ ] base branch established;
- [ ] branch ancestry reconciled;
- [ ] production-relevant diff reviewed;
- [ ] implementation complete;
- [ ] deterministic correctness evidence complete;
- [ ] sanitizer/fuzz evidence complete where applicable;
- [ ] genuine security-runtime evidence complete where applicable;
- [ ] required compiler/configuration matrix complete;
- [ ] public API/ABI review complete;
- [ ] installation/consumer compatibility evidence complete;
- [ ] performance evidence complete where applicable;
- [ ] documentation and ADRs synchronized;
- [ ] known limitations recorded;
- [ ] no duplicate implementation exists on another branch;
- [ ] all evidence points to the current or explicitly inherited SHA;
- [ ] final Staff/Principal review complete;
- [ ] final Principal Security review complete for security-sensitive changes;
- [ ] CA/Finance and MBA/Management/Product review completed where materially applicable;
- [ ] merge decision recorded.

## 9. Merge execution discipline

Do not merge merely because the PR is old, CI is green, or the implementation "looks finished."

Before merge:

1. refresh the remote PR/head SHA;
2. confirm the reviewed SHA is still the PR head;
3. compare the final production-relevant diff;
4. confirm all mandatory gates;
5. confirm no unresolved security finding remains;
6. confirm known limitations are documented;
7. record the final decision.

Do not force-push or rewrite history unless explicitly required and understood.

## 10. Post-merge verification

Merge is not the end of release engineering.

After merging, verify:

- `main` contains the intended commit/merge result;
- required CI/release workflows run against the merged state;
- no merge conflict or accidental file loss occurred;
- release documentation points to the merged SHA;
- security-sensitive evidence remains traceable;
- the release status is updated;
- rollback/revert remains understood.

If the merge changes production-relevant behavior beyond the reviewed SHA, treat that as a new evidence checkpoint rather than silently inheriting the previous decision.

## 11. No duplicate work across branches

Before implementing or reviewing a requested item:

1. identify all three branches;
2. compare ancestry;
3. search history for the behavior/fix;
4. inspect the current implementation;
5. inspect tests;
6. inspect ADRs;
7. inspect evidence;
8. classify the gap as implementation, contract, test, security, evidence, documentation, performance, operational, or product debt;
9. choose the smallest change.

If the work already exists on an ancestor or cumulative branch:

```
do not implement again
```

If implementation exists but evidence is missing:

```
verify it
```

If evidence exists but documentation is stale:

```
reconcile documentation
```

If implementation is incorrect:

```
fix the existing implementation
```

Do not create parallel mechanisms to avoid inspecting existing work.

## 12. Release evidence record

Every significant final decision should preserve:

```
PR:
Base:
Head branch:
Reviewed SHA:
Evidence SHA(s):
Production-relevant delta:
Implementation result:
Test result:
Security result:
ABI/API result:
Performance result:
Runtime environment:
Known limitations:
Business/operational impact:
Final decision:
Reviewer/review status:
Next action:
```

The repository's canonical evidence ledger is `docs/RELEASE_EVIDENCE.md`.

The branch/work provenance record is `docs/BRANCH_PROVENANCE.md`.

The engineering operating contract is `AGENTS.md` and the related engineering documents.

## 13. Current PR #1 policy

PR #1 remains a release-candidate integration PR.

It must remain unmerged until:

- the required compiler/configuration matrix is closed;
- final API/ABI review is complete;
- applicable final release/security verification is complete;
- known limitations are recorded;
- final P0 sign-off is explicit.

Creating or updating the PR does not itself close any technical gate.

## 14. Engineering principle

The governing principle is:

> Merge the exact source that was reviewed and whose mandatory evidence is understood.

Not:

> Merge whatever happens to be green today.

The goal is not maximum ceremony. The goal is traceable, reproducible, economically justified, security-honest integration with no duplicated engineering work.
