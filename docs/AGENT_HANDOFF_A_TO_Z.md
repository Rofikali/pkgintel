# pkgintel — A-to-Z Agent and Engineer Handover

**Document purpose:** enable a new human engineer or coding agent to continue pkgintel safely without guessing, repeating completed work, overstating evidence, or losing architectural and security context.

**Snapshot date:** 2026-10-11 UTC  
**Canonical branch:** `main`  
**Canonical source SHA at this snapshot:** `7c324f1fdc8a26ad4b8eab3d9d4d211f1311d0eb`  
**Repository:** https://github.com/Rofikali/pkgintel  
**Status:** P0 foundation, P1 JSON export, P2 reference-consumer validation, dpkg NUL rejection, EOF regression coverage, and dpkg status-token validation are integrated. This is not a production-release certification.

This is a source-controlled handover, not a substitute for rechecking live refs, current CI, or local worktrees.

## 1. Start here: first 15 minutes

Read in this order:

1. `AGENTS.md` — binding agent/engineering contract.
2. `SKILLS.md` — engineering capability and review questions.
3. `docs/ENGINEERING_STATUS.md` — current milestone, completed work, next gate.
4. `docs/BRANCH_CLEANUP_LEDGER.md` — every remote branch observed in the 2026-10-11 snapshot and disposition.
5. `docs/BRANCH_PROVENANCE.md` — historical provenance and squash-merge caveats.
6. `docs/ENGINEERING_WORKFLOW.md` and `docs/PR_MERGE_RELEASE_POLICY.md` — work, review, merge, and release workflow.
7. `docs/ARCHITECTURE.md`, `docs/HLD.md`, `docs/LLD.md` — system boundaries.
8. `docs/API_CONTRACT.md`, `docs/API.md` — public API/ABI and semantic contract.
9. `docs/SECURITY.md`, `docs/SECURITY_VERIFICATION_RUNTIME.md` — threat model and real-runtime requirements.
10. `docs/ENGINEERING_GATES.md`, `docs/RELEASE_EVIDENCE.md`, `docs/FUZZING.md`, `docs/PERFORMANCE.md` — required evidence and limitations.
11. `docs/ADR/README.md` and relevant ADRs — decisions and why they exist.
12. `docs/P1_JSON_EXPORT_HANDOFF.md`, `docs/P2_CONSUMER_VALIDATION_DECISION.md`, `docs/P2_REFERENCE_CONSUMER.md`, and `docs/DPKG_NUL_RECORD_SECURITY_REVIEW.md` — capability/security handoffs.

Before changing files, fetch current `main`, record its SHA, inspect recent commits, check open PRs, inspect the worktree and untracked files in the actual development checkout, and compare the requested behavior against code, tests, ADRs, and release evidence. Never infer the state of a developer's local worktree from GitHub alone.

## 2. Product and domain purpose

pkgintel is a security-sensitive Linux package/system inspection library and CLI. It reads package-manager metadata and correlates package file lists with filesystem observations. Debian/dpkg is the first implemented package backend.

The project should produce evidence and diagnostics, not pretend that missing observations are facts. The central domain separation is:

```
package identity
    != dpkg desired action
    != dpkg actual installation state
    != filesystem artifact observation
    != package consistency conclusion
```

Current product scope is deliberately narrow. Do not add APT metadata parsing, ELF analysis, caching, capability analysis, broad recursive inventory, or other features just because their APIs or future plans are mentioned. Check the current status and product decision documents for the active gate.

## 3. Current integration facts

At the snapshot SHA above:

- P0 module architecture was integrated by PR #1.
- Product charter was integrated by PR #6.
- Versioned CLI JSON export was integrated by PR #7.
- P1 handoff/provenance documentation was integrated by PR #8.
- P2 reference-consumer validation was integrated by PR #10.
- Strict JSON consumer input handling was integrated by PR #12.
- Branch inventory reconciliation was integrated by PR #13.
- JSON decoder-failure normalization was integrated by PR #14.
- Embedded-NUL dpkg record rejection was integrated by PR #15.
- Branch provenance refreshes were integrated by PRs #16, #17, and #19.
- EOF-terminated NUL record regression tests were integrated by PR #18.
- dpkg desired-action/error-flag vocabulary validation was integrated by PR #20.

Current exact merged source SHA is `7c324f1fdc8a26ad4b8eab3d9d4d211f1311d0eb`. Its post-merge GitHub Actions run 38153120422 completed successfully (7/7 jobs): https://github.com/Rofikali/pkgintel/actions/runs/38153120422. PR #20 exact-head push run 38153010070 and PR run 38153013822 also passed 7/7. Some benchmark steps are conditionally skipped in some matrix configurations; a skip is not a benchmark result. These CI results prove only the behavior exercised by those jobs.

Security evidence for PR #15 included a reported dedicated security-container run on the exact tested source state: GCC 13.3 Debug build, CTest 5/5 with zero skipped under `PKGINTEL_REQUIRE_PRIVILEGED_SECURITY_TESTS=ON`, plus sanitizer/regression evidence documented in the security review. Confirm source/evidence SHA applicability before using it as evidence for a later source change.

## 4. Code map

- `include/pkgintel/`: public headers; changes can affect API/ABI.
- `src/core/`: scan orchestration and context behavior.
- `src/target/`: target-root ownership and filesystem security boundary.
- `src/backends/dpkg/`: dpkg metadata/file-list parser and correlation logic.
- `src/snapshot/`: result/snapshot ownership, mutation, and resource ceilings.
- `src/package/`: package semantics/accessors.
- `src/artifact/`: artifact evidence.
- `src/diagnostic/`: structured diagnostics.
- `src/json/` and `src/internal/json.h`: JSON serialization.
- `src/cli/`: CLI behavior and output contract.
- `src/support/`: shared status/string support.
- `src/internal/`: private implementation contracts; not public ABI.
- `tests/unit/`: deterministic unit/regression tests.
- `tests/security/test_target_mounts.c`: genuine Linux mount-boundary and procfs magic-link checks.
- `tests/fuzz/`: dpkg status/file-list fuzz targets and corpora.
- `tests/json/`: independent JSON/reference-consumer checks.
- `tests/install/`: installed consumer/API surface checks.
- `tests/benchmark/`: benchmark and allocation instrumentation.
- `tests/abi/public_symbols.txt`: public-symbol expectation.
- `.github/workflows/ci.yml`: CI build/test/security gates.
- `docs/ADR/`: architecture decisions; read before changing an established invariant.

## 5. Architecture and contracts

High-level flow:

```
public C API / CLI
       |
       v
scan orchestration
       +--> target (root descriptor and containment)
       +--> dpkg backend (untrusted metadata parsing)
       +--> snapshot (owned observations and limits)
       +--> package semantics
       +--> artifact evidence
       +--> diagnostics
       +--> JSON serializer / CLI presentation
```

Public objects are opaque. Public headers under `include/pkgintel/` define caller-facing behavior. Private headers under `src/internal/` are not public API. Read the API contract before altering ownership, error codes, struct-size initialization, thread-safety, or serialization.

The API contract explicitly states that v0.1 is **not yet declared ABI-stable**. Do not promise ABI stability merely because the CI has symbol/installed-consumer checks. An ABI release milestone still requires the policy decisions documented in `docs/API.md`.

## 6. Security model and non-negotiable invariants

Treat dpkg metadata, file names, filesystem metadata, and binary input as hostile.

1. **Read-only:** do not mutate target content.
2. **No execution:** discovered programs are data, not commands; no shell or implicit execution in normal discovery.
3. **Target containment:** rootfs operations must remain confined to the selected root.
4. **Least privilege:** do not auto-escalate.
5. **Fail closed:** malformed/unknown security-sensitive evidence must not be promoted to verified facts.
6. **Bounded parsing:** enforce record and count limits before allocation/indexing.
7. **Checked arithmetic:** validate additions, multiplications, lengths, and counts.
8. **Race resistance:** use descriptor-relative operations and post-open verification where object identity matters.
9. **Byte-safe paths:** Linux paths are arbitrary bytes, not guaranteed UTF-8.
10. **Evidence provenance:** distinguish source metadata, filesystem observation, derived conclusion, and unknown/unavailable evidence.

Target-root design uses an owned root directory descriptor and Linux resolution constraints including `RESOLVE_IN_ROOT`, `RESOLVE_NO_MAGICLINKS`, and `RESOLVE_NO_XDEV` where implemented. Review ADR-0038 and the implementation before making claims about exact behavior.

Do not conflate:
- could not observe != does not exist;
- permission denied != present;
- broken link != present regular file;
- no correlation != consistent;
- partial dpkg state != filesystem inconsistency;
- SKIP != PASS;
- passing CI != proof of every possible attack.

### dpkg parser-specific guarantees

- A hard maximum of 65,536 bytes of record content before LF; exact boundary behavior is documented in `docs/SECURITY.md` and ADR-0036.
- EOF without final LF is accepted as a complete record, except malformed NUL-bearing record content remains rejected.
- Embedded NUL is not treated as a safe C-string terminator. NUL-bearing status metadata fails with `PKG_ERR_PARSE`; NUL-bearing file-list records are rejected from correlation, mark correlation incomplete, and emit the stable malformed-file-list diagnostic.
- Status tokens are validated against dpkg's recognized desired-action vocabulary (`unknown`, `install`, `hold`, `deinstall`, `purge`) and error flags (`ok`, `reinstreq`). Actual state and error flag determine classification; desired action does not.
- Unknown/malformed status combinations must not be silently promoted to INSTALLED.
- Review ADR-0036 and ADR-0037, `docs/DPKG_NUL_RECORD_SECURITY_REVIEW.md`, and the corresponding unit/fuzz tests before touching parser behavior.

## 7. Resource and feature limitations

Implemented v0.1 controls include:
- bounded dpkg status/file-list records;
- per-scan package count;
- per-package non-empty file-list record count;
- aggregate snapshot artifact ceiling of 1,000,000;
- aggregate diagnostic ceiling of 100,000;
- transactional snapshot mutation.

The public option fields for aggregate filesystem file count, directory count, recursion depth, bytes read, individual file size, ELF size, wall-clock duration, cache entries, and general descriptor/allocation budgets are reserved/not enforced in this slice. Unsupported non-zero options must return `PKG_ERR_UNSUPPORTED`; do not make them appear enforced by accepting and ignoring them.

APT/ELF parsers and several feature flags are future scope, not shipped behavior. Read `docs/API.md`, `docs/SECURITY.md`, and current ADRs for the authoritative details.

## 8. JSON and downstream consumer

The CLI has versioned JSON export. The reference consumer is a test/validation consumer, not a production customer or proof of ecosystem adoption. Strict JSON parsing rejects ambiguous input according to the contract; decoder failures are normalized and covered by regression tests. Inspect:
- `docs/P1_JSON_SCHEMA.md`
- `docs/P1_JSON_HLD_LLD.md`
- `docs/P1_JSON_EXPORT_HANDOFF.md`
- `docs/P2_CONSUMER_VALIDATION_DECISION.md`
- `docs/P2_REFERENCE_CONSUMER.md`
- `tests/json/`

Do not duplicate the reference consumer or redo strict-input fixes unless current source/tests show a concrete uncovered defect.

## 9. Build, tests, and CI

The GitHub CI matrix includes GCC/Clang Debug and Release, sanitizer tests, fuzz-oriented validation, JSON/reference consumer checks, ABI/export checks, installed-consumer checks, and a real Linux filesystem security job. Read `.github/workflows/ci.yml` for the exact current job names and conditions.

Typical repository-local verification commands (confirm the current CMake options/targets before execution):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Do not claim these commands were run unless there is a recorded result for the exact source SHA and environment. For security, compiler-matrix, install, sanitizer, fuzz, and benchmark work, use the CI-equivalent command documented in workflow/docs rather than substituting a narrower smoke test.

## 10. Real OS / platform verification

The developer's intended topology is:

```
Windows 11 host
  -> Docker Desktop
  -> Ubuntu 24.04
  -> pkgintel developer container
```

Use a separate dedicated security-verification container for mount/VFS/namespace tests. The normal development container should remain least-privileged. UID 0 inside a restricted container is not proof that mount operations are available. If a genuine mount test receives EPERM, inspect capabilities, seccomp, user namespace, and mount namespace; do not weaken the test or turn it into a pass.

When kernel/VFS/namespace/capability evidence is required, provide exact commands to the developer and wait for their returned output. Do not claim that commands were executed in the developer's containers by the assistant. Record image, container name, kernel, UID, capabilities, namespace state, source SHA, build flags, command, output, and test result.

## 11. Staff/Principal review protocol

For a non-trivial change:
1. state the problem and invariant;
2. inspect existing code, history, tests, ADRs, evidence, and branches;
3. classify the gap: implementation, contract, tests, security, resource, API/ABI, evidence, docs, performance, operations, or product;
4. define acceptance criteria and threat assumptions;
5. consider alternatives and choose the smallest correct change;
6. check arithmetic, complexity, ownership/lifetime, state transitions, and rollback/reversibility;
7. implement regression tests;
8. update API/security/ADR/status/evidence docs;
9. run the appropriate CI/runtime gates;
10. review exact final diff and exact head SHA;
11. merge only after mandatory gates pass;
12. verify post-merge CI and reconcile documentation.

Apply CA/Finance analysis when there is material cost or business consequence: CAPEX/OPEX, maintenance, operational burden, risk-adjusted cost, ROI, opportunity cost. Apply MBA/management/product analysis to scope, sequencing, dependencies, delivery risk, support burden, operational ownership, and customer value. Do not invent financial precision or product demand.

## 12. Pull request and branch policy

- `main` is the canonical integration line. New work branches from the freshly fetched current `main`.
- Never implement the same feature independently on several branches.
- A branch name is not proof of unique work; a merge commit or PR being closed is not sufficient alone to establish that a feature is integrated.
- Squash merges do not preserve feature-branch commits as ancestors. Compare PR diffs and final source content, not only ahead/behind commit counts.
- For every change, record branch, prior SHA, new SHA, files, tests, exact CI run, review state, known gaps, and merge SHA.
- After merge, verify the exact `main` commit and post-merge CI.
- Branch cleanup must follow `docs/BRANCH_CLEANUP_LEDGER.md`; no non-main branch is to be deleted before its unique work and PR disposition are documented.
- The GitHub branch-list API reported `protected: false` for the 22 baseline refs. The connected integration could not read the branch-protection endpoint (403), so the full branch-protection/ruleset configuration is **not verified** by this audit. Confirm it with owner-level settings access before claiming any rule is or is not enforced. See `docs/BRANCH_CLEANUP_LEDGER.md`.

## 13. Current next work — investigate, do not assume

The most recent source review raised a **candidate lifecycle question** in `src/target/target.c`: whether a relative root path is stored at target creation and opened only later, potentially allowing the process working directory to change between creation and scan. This is not yet a confirmed defect or demonstrated escape.

A new engineer should:
1. inspect the current implementation and all callers;
2. read the public target API contract and ADR-0038;
3. determine whether relative roots are allowed and what path identity is promised;
4. create a focused regression test that changes working directory between target creation and scan if the current API makes that scenario relevant;
5. determine the correct failure/identity semantics before implementation;
6. evaluate descriptor ownership and TOCTOU implications;
7. prove the behavior with deterministic tests, and use real Linux verification if the claim depends on kernel/VFS semantics.

Do not make a code change merely because the question is listed here. Confirm the defect first. Also inspect current open issues/PRs and the branch-cleanup status before selecting the next task.

## 14. Governance and merge controls

PR #15 (dpkg embedded-NUL security fix) merged on 2026-10-11 without independent approval visible in the recorded review list, despite the written policy describing review as a gate. Repository branch protection was reported as not enforcing that gate. This is a governance gap, not proof that the code fix is incorrect.

Before changing settings, verify current owner permissions and supported ruleset controls. A solo-maintainer setup should at minimum require PR workflow and passing required CI where GitHub supports it, avoid locking the owner out, and explicitly document any impossible independent-review requirement. Never report a policy as enforced unless live settings confirm it.

## 15. Evidence and business truth

Use these evidence labels precisely:
- Source verified
- Unit/integration verified
- Sanitizer/fuzz verified
- Real runtime verified
- Benchmark verified
- Production-like verified

Record exact SHA, environment, command, outcome, skipped tests, assumptions, and remaining limitations. Benchmark results require a reproducible workload and measurement methodology. Do not describe CI as performance proof if benchmark steps were skipped or no comparable baseline exists.

From a CA/Finance and management perspective, prioritize correctness/security and verified customer value over feature count. Avoid premature APT/ELF/cache expansion until the product gate and acceptance criteria justify the maintenance cost.

## 16. Branch cleanup acceptance

The 2026-10-11 baseline inventory found 22 refs including `main`, before this handover branch was created. Its 21 non-main refs and exact observed heads/PR dispositions are listed in `docs/BRANCH_CLEANUP_LEDGER.md`. The temporary `docs/complete-agent-handover-branch-cleanup` branch exists only to deliver PR #21; after merge, it is also a deletion candidate once the merged docs and post-merge CI are verified. The intended end state is one canonical remote branch, `main`, after every other ref has been individually reconciled and safely deleted. This document alone does not claim that deletion has occurred.

Do not delete:
- `main`;
- any branch whose current head differs from the audited SHA until re-audited;
- any branch with an open PR;
- any branch containing unmerged production code or unique docs/tests not incorporated into main;
- any branch needed to preserve a pending review/evidence workflow.

A remote branch deletion removes a reference, not the reachable commits in main. Nonetheless, deleting a branch before recording its purpose, unique commits, and PR state can make future investigation harder. The branch ledger is the prerequisite.

## 17. Definition of done for a new agent

A handover is successful only when the next agent can identify:
- what the product does and explicitly does not do;
- current source SHA and release/milestone state;
- code and public/private boundaries;
- threat model and implemented versus reserved controls;
- current tests, runtime evidence, and gaps;
- important ADRs and why decisions exist;
- all historical branches and their work;
- what must not be repeated;
- the next unconfirmed question to investigate;
- how to make, verify, review, merge, and roll back a change;
- which actions require real Ubuntu 24.04 developer/security-container evidence;
- who owns the next action and what evidence will close it.

If any of those cannot be established from current source and documented evidence, investigate before implementing.


## State refresh — 2026-10-12 UTC (supersedes older snapshot fields)

- Current canonical branch: `main` at `e7366500ba0de548bcc7e490124196b510d38746` (PR #23, handoff/cleanup ledger refresh).
- PR #22 merged target-root identity pinning at `714a0b42612b95622dcdb6e5c1983ece27c2b06d`. Target creation now opens and retains the root directory FD; relative roots are resolved at creation, symlink roots fail at creation, and the public contract/ADR/tests were updated together.
- PR #22 exact-head CI passed 7/7. PR #23 exact-head CI passed 7/7. Post-merge CI for current `main` passed 7/7 at [run 38206440486](https://github.com/Rofikali/pkgintel/actions/runs/38206440486).
- There are zero open PRs. A live inventory immediately after PR #23 reported 25 branch refs (including `main`), of which 24 were non-main. A temporary handoff refresh branch was subsequently created; re-fetch live refs before any cleanup decision.
- Branch deletion remains unexecuted: the available GitHub connector has no delete-branch/ref operation. Do not use ref movement as a deletion substitute. Keep `main`; verify exact live heads and PR disposition before deleting via an authorized GitHub operation.
- Next engineering gate remains evidence-led downstream-consumer validation as specified in `docs/P2_CONSUMER_VALIDATION_DECISION.md`. No new production capability is justified by customer evidence in the repository snapshot. Do not invent a feature or claim customer validation occurred.
- The GitHub Actions checks are repository-hosted evidence. Do not represent them as a local run inside Windows 11 → Docker Desktop → Ubuntu 24.04. Privileged OS/VFS/mount claims still require the documented real-runtime evidence.
