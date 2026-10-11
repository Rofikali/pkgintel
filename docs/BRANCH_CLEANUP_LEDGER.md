# Branch Cleanup Ledger and Provenance Register

**Snapshot:** 2026-10-11 UTC  
**Repository:** `Rofikali/pkgintel`  
**Canonical branch:** `main`  
**Canonical SHA:** `7c324f1fdc8a26ad4b8eab3d9d4d211f1311d0eb`  
**Observed remote refs:** 22 total — `main` plus 21 non-main refs.

This is a point-in-time inventory. Before any deletion, re-fetch live branch heads and pull-request state. A branch head changing after this snapshot invalidates the corresponding deletion decision until reviewed again.

## 1. Decision rules

- The only canonical development/integration branch is `main`.
- New work starts from the freshly fetched current `main`.
- A merged PR does not by itself prove that every later commit on its branch was integrated; compare exact head and PR diff.
- Squash merges usually mean feature-branch commits are not ancestors of main. Ahead/behind counts alone are not a deletion oracle.
- A closed PR can be merged or unmerged. Inspect `merged_at`, PR diff, and the current tree.
- Deleting a branch ref does not delete commits reachable from main, but may make unique, unmerged commits harder to discover. Record unique work before deleting.
- No branch is deleted by this document; deletion requires a live recheck and a branch-deletion-capable GitHub action.

## 2. Canonical mainline milestones

| PR | Work | Result |
|---|---|---|
| #1 | P0 cumulative module architecture | Merged as `ec44a111cf215337d16e3c8800574248e109a3f1` |
| #2–#5 | P0 post-merge and provenance docs | Merged documentation reconciliation |
| #6 | Product charter | Merged |
| #7 | P1 capability decision and versioned CLI JSON export | Merged |
| #8 | P1 JSON export handoff | Merged |
| #9 | P2 downstream-consumer validation decision | Merged |
| #10 | Internal reference-consumer validation | Merged |
| #11 | Stale provenance draft | Closed without merge; explicitly superseded by #13 |
| #12 | Strict JSON input in reference consumer | Merged |
| #13 | Branch inventory reconciliation | Merged |
| #14 | JSON decoder-failure normalization | Merged |
| #15 | Reject embedded NUL in dpkg records | Merged; review-governance gap recorded below |
| #16–#17 | Branch provenance refreshes | Merged |
| #18 | EOF-terminated NUL record regression tests | Merged |
| #19 | Provenance refresh after #18 | Merged |
| #20 | Validate dpkg status desired-action/error-flag tokens | Merged |

PR links:
- [#1](https://github.com/Rofikali/pkgintel/pull/1), [#2](https://github.com/Rofikali/pkgintel/pull/2), [#3](https://github.com/Rofikali/pkgintel/pull/3), [#4](https://github.com/Rofikali/pkgintel/pull/4), [#5](https://github.com/Rofikali/pkgintel/pull/5)
- [#6](https://github.com/Rofikali/pkgintel/pull/6), [#7](https://github.com/Rofikali/pkgintel/pull/7), [#8](https://github.com/Rofikali/pkgintel/pull/8), [#9](https://github.com/Rofikali/pkgintel/pull/9), [#10](https://github.com/Rofikali/pkgintel/pull/10)
- [#11](https://github.com/Rofikali/pkgintel/pull/11), [#12](https://github.com/Rofikali/pkgintel/pull/12), [#13](https://github.com/Rofikali/pkgintel/pull/13), [#14](https://github.com/Rofikali/pkgintel/pull/14), [#15](https://github.com/Rofikali/pkgintel/pull/15)
- [#16](https://github.com/Rofikali/pkgintel/pull/16), [#17](https://github.com/Rofikali/pkgintel/pull/17), [#18](https://github.com/Rofikali/pkgintel/pull/18), [#19](https://github.com/Rofikali/pkgintel/pull/19), [#20](https://github.com/Rofikali/pkgintel/pull/20)

## 3. Full branch inventory and disposition

All SHAs below are the exact remote branch heads observed in the 2026-10-11 snapshot. “Delete candidate” means only after live recheck and documentation merge; it does not mean already deleted.

| Branch | Observed head SHA | Associated work / PR | Current disposition |
|---|---|---|---|
| `main` | `7c324f1fdc8a26ad4b8eab3d9d4d211f1311d0eb` | Canonical integration line; PRs #1–#20 as above | **KEEP** |
| `codex/p0-foundation` | `9a3ddd9f3dc0faa12eeec3bda30800df045f2518` | P0 foundation ancestor; incorporated through cumulative `codex/p0-module-architecture` / PR #1 | Delete candidate after ancestry/content check; do not replay |
| `codex/p0-module-architecture` | `1a99cbe2e8bfeea427775a38877f70c34766c234` | Cumulative P0 implementation; PR #1 merged | Delete candidate after live PR/head check |
| `codex/p0-post-merge-reconciliation` | `ee98282111136c2b37686bfbac756a36cf3b76c5` | P0 reconciliation; PR #2 merged | Delete candidate after live PR/head check |
| `codex/p1-capability-decision` | `a75214124d0d1027c1eb1e9eedd2a358832a84c1` | P1 decision + JSON export; PR #7 merged | Delete candidate after confirming current JSON implementation matches merged PR |
| `codex/p1-product-charter` | `998f39a1b5d405a5d278a1bf70c27596c5539dcf` | Product charter; PR #6 merged | Delete candidate after live PR/head check |
| `codex/p2-reference-consumer` | `7ca3584e53b67d048e34fa967cb998323a1aa233` | P2 reference consumer; PR #10 merged | Delete candidate after current test/docs content check |
| `docs/branch-inventory-current` | `01498adfca3ec736862f99e5fa915c5da02d3f1e` | Branch inventory; PR #13 merged | Delete candidate; current ledger supersedes snapshot |
| `docs/finalize-current-main-pointer` | `b63fc970f14cbe4bb719ce3c31f916b64721e587` | Main pointer/provenance; PR #5 merged | Delete candidate after live PR/head check |
| `docs/finalize-p0-provenance` | `ae5bf0e45ed9c7869504f4c51c9b3ea521594594` | P0 provenance/release state; PR #4 merged | Delete candidate after live PR/head check |
| `docs/p1-json-export-handoff` | `268480032ca7c9ffd9f7832ecdba120e77da482c` | JSON handoff; PR #8 merged | Delete candidate after current handoff doc check |
| `docs/p2-consumer-validation-decision` | `289e7e6feaaf37acf8a5e7fd0f26b147395479e0` | P2 consumer decision; PR #9 merged | Delete candidate after current decision doc check |
| `docs/reconcile-agent-branch-state` | `9b738699427d5fa9857e5ca3698d9569a5b85f6f` | Agent state after P0; PR #3 merged | Delete candidate after live PR/head check |
| `docs/reconcile-branch-inventory-after-pr14` | `716b437a06b568e51e047fab676486bae8efe256` | Inventory/provenance after #14; PR #16 merged | Delete candidate; current ledger supersedes snapshot |
| `docs/reconcile-current-branch-provenance` | `b01fe04d2689c1dec0602dc8bc44e7c7d4a4ec4f` | PR #11 closed **without merge**; its stale main SHA/branch snapshot was superseded by merged PR #13 | Delete candidate only after confirming no unique still-needed facts; original PR diff/comments preserve the historical context |
| `docs/reconcile-provenance-after-pr18` | `b07c2e46bda2630a1778321114cff72d3a7edb2a` | Provenance refresh; PR #19 merged | Delete candidate after confirming this ledger supersedes its point-in-time state |
| `docs/refresh-provenance-post-pr16` | `02052a0d56b44caa7b7a6a940268984905ff3e2b` | Provenance refresh; PR #17 merged | Delete candidate; current ledger supersedes snapshot |
| `fix/dpkg-status-token-validation` | `07e2b74f059e5c3a5032c9989259a926f34d58d9` | dpkg desired-action/error-flag validation; PR #20 merged as `7c324f1fdc8a26ad4b8eab3d9d4d211f1311d0eb` | Delete candidate after exact-head/PR recheck; behavior is on main |
| `fix/p2-json-parser-failure-normalization` | `0d97cc1e772bb141c8fce68bccc7ddfebdf42b45` | Reference-consumer decoder failure normalization; PR #14 merged | Delete candidate after current test/source check |
| `fix/p2-strict-json-input` | `8ed81e2e3ec4e4253366278f1373a8007ecab49a` | Strict JSON input; PR #12 merged | Delete candidate after current test/source check |
| `security/reject-nul-in-dpkg-records` | `4c0a7538189d6dc4d98a9651e0c10227943c58f4` | dpkg embedded-NUL rejection; PR #15 merged as `01852345ba39ea867a2073c5c218373df7a96c0c` | Delete candidate after live PR/head check and security-evidence preservation |
| `test/dpkg-nul-eof-records` | `4238cb9f8fb4dc7947b9711952866feb7a02bf8b` | EOF-terminated NUL regressions; PR #18 merged | Delete candidate after confirming regression tests are on main |

## 4. Important squash-merge caveat

GitHub comparisons for several historical feature branches show them as “diverged” or “ahead” even though their associated PRs are merged. This is expected when PR changes were squash-merged: the feature commits are not necessarily ancestors of the merge commit. The comparisons can show changed files from the common ancestor that are already present on main.

Therefore, use this evidence order:
1. exact current remote branch head;
2. PR state and `merged_at`, not just `closed`;
3. exact PR head SHA and PR diff;
4. merged SHA and post-merge source/test content;
5. current branch comparison for unique work after the PR head;
6. open PR/review/evidence references;
7. deletion eligibility.

A branch that moved after the PR head may contain post-PR commits not integrated by that PR. Re-audit those commits before deletion.

## 5. Current open-PR check

At this inventory snapshot, the all-state PR listing contained PRs #1–#20 and each was closed; PR #11 is explicitly unmerged and superseded by #13. Recheck the live open-PR list immediately before branch deletion, because the state may have changed since this snapshot.

## 6. Governance gap: written review policy vs actual controls

The live branch API reported `protected: false` for all 22 observed refs, including `main`. The repository's written policy calls for exact-SHA review and CI gates, but the branch protection/ruleset state did not demonstrate that those gates were enforced by GitHub settings.

PR #15 merged without independent approval visible in the recorded review list. This is recorded as a governance gap, not as evidence that the parser fix is wrong.

Before changing controls, inspect current branch protection and rulesets. For a solo maintainer, choose an enforceable configuration that requires PR workflow and mandatory CI without locking the owner out; if independent review cannot be required by the available account/team structure, document that limitation explicitly. Never describe a written policy as an enforced technical control.

## 7. Verification references

- Current main post-merge CI for `7c324f1fdc8a26ad4b8eab3d9d4d211f1311d0eb`: https://github.com/Rofikali/pkgintel/actions/runs/38153120422 — 7/7 passed.
- PR #20 exact-head PR CI: https://github.com/Rofikali/pkgintel/actions/runs/38153013822 — 7/7 passed.
- PR #20 exact-head push CI: https://github.com/Rofikali/pkgintel/actions/runs/38153010070 — 7/7 passed.
- PR #19 exact-head PR CI: https://github.com/Rofikali/pkgintel/actions/runs/38152202087 — 7/7 passed.
- PR #18: https://github.com/Rofikali/pkgintel/pull/18.
- PR #15: https://github.com/Rofikali/pkgintel/pull/15.

Some Release benchmark steps are conditional/skipped in the CI matrix. Do not call skipped benchmark steps performance evidence. The genuine Linux security job and unprivileged sanitizer jobs have different evidence scopes.

## 8. Deletion execution checklist

For each non-main branch, immediately before deleting:
- [ ] branch still exists and current head equals the audited SHA, or any movement has been re-audited;
- [ ] associated PR state and `merged_at` checked;
- [ ] no open PR uses the branch;
- [ ] all unique source/test/docs work is on main or explicitly archived in the PR/handover ledger;
- [ ] branch commits after the reviewed PR head are checked;
- [ ] required CI/evidence links and merge SHA are recorded;
- [ ] no pending review or release evidence depends on the branch;
- [ ] deletion is executed by a GitHub operation that explicitly supports deleting refs;
- [ ] branch absence is verified afterward.

Do not delete `main`. Do not use force-push/reset as a substitute for deleting a feature-branch ref. Do not claim a deletion succeeded without checking the live branch list afterward.
