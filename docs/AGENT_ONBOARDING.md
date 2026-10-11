# Agent and Engineer Onboarding

## 1. What pkgintel is

pkgintel is a security-sensitive Linux package/system inspection library and tooling project.

The current implementation is a C17 core targeting Debian/Ubuntu packaging conventions, with dpkg as the first package backend. The design uses opaque public C objects, target-root filesystem confinement, bounded metadata parsing, deterministic snapshots, structured diagnostics, and explicit resource governance.

The repository is intentionally being developed as a serious systems project rather than as a demo.

## 2. First five minutes

A new engineer or agent should read, in this order:

1. AGENTS.md
2. docs/ENGINEERING_STATUS.md
3. docs/ENGINEERING_PRINCIPLES.md
4. docs/ENGINEERING_GATES.md
5. docs/ARCHITECTURE.md
6. docs/API_CONTRACT.md
7. docs/SECURITY.md
8. docs/ADR/README.md
9. docs/AGENT_HANDOFF_A_TO_Z.md
10. docs/BRANCH_CLEANUP_LEDGER.md
11. docs/BRANCH_PROVENANCE.md
12. docs/RELEASE_EVIDENCE.md

Then fetch current GitHub refs, inspect the exact current `main` SHA, check open PRs, and inspect the actual local worktree/untracked files before editing. GitHub state does not establish local worktree cleanliness.

## 3. Branch model and cleanup state

The three P0 branches were a historical progression, not three independent implementations:

```
codex/p0-foundation
       |
       v
codex/p0-module-architecture -- PR #1 --> main
```

That historical explanation remains useful for P0 provenance, but it is not a current inventory. The 2026-10-11 GitHub snapshot found 22 remote refs (main + 21 non-main). The exact heads, PR dispositions, unique-work cautions, and deletion checklist are in `docs/BRANCH_CLEANUP_LEDGER.md`.

Current policy:
- `main` is the only canonical integration line.
- P0/P1/P2 and dpkg security work listed in the ledger is already integrated by its recorded PRs; do not replay it.
- Squash-merged branches may appear ahead/diverged in GitHub comparisons because their feature commits are not ancestors of the squash merge. Compare PR diffs and current source content, not ahead/behind alone.
- PR #11 was closed without merge because its stale provenance snapshot was superseded by PR #13; it was not an implementation PR.
- Do not delete a branch until its current head, PR state, any post-PR commits, unique files, and evidence dependencies have been reconciled.
- Branch cleanup is not complete merely because the ledger exists. The intended final state is main only, after each non-main ref has been safely deleted and the absence verified.

## 4. Current architectural direction

Important module boundaries are:

~~~
public API
    |
    v
core scan orchestration
    |
    +--> target       security boundary / filesystem observation
    +--> backend      package-manager interpretation
    +--> snapshot     result ownership / lifetime
    +--> package      package semantics
    +--> artifact     filesystem evidence
    +--> diagnostic   structured explanation
    +--> support      shared narrow utilities
~~~

Public headers live under include/pkgintel/. Private implementation contracts live under src/internal/.

## 5. Security model

The target root is a security boundary.

For rootfs targets, the implementation is designed around an owned root descriptor and Linux openat2() resolution with:

- RESOLVE_IN_ROOT;
- RESOLVE_NO_MAGICLINKS;
- RESOLVE_NO_XDEV.

Root opening uses directory/no-follow/close-on-exec constraints.

Artifact identity observation uses O_PATH | O_NOFOLLOW; explicit symlink following, when required to determine whether a link resolves within the permitted target, is a separate controlled operation.

The project distinguishes ordinary symlinks from procfs-style magic links and simulated directories from real mount boundaries.

## 6. Evidence model

pkgintel distinguishes:

~~~
package identity
    !=
dpkg installation state
    !=
filesystem artifact evidence
    !=
derived consistency conclusion
~~~

This is a core design rule.

For example:

- a package marked PARTIAL by dpkg is not automatically filesystem-inconsistent;
- a missing artifact is stronger evidence than an inability to observe an artifact;
- permission denial does not prove presence or absence;
- a broken symlink is not a present regular file;
- an uncorrelated package cannot truthfully be reported as consistently verified.

## 7. Resource model

Resource controls are security controls.

Current v0.1 controls include bounded dpkg records, package limits, per-package package-file limits, aggregate artifact/diagnostic ceilings, and owned-string accounting.

Do not advertise a resource limit as enforced until the enforcement point and tests exist.

A configured maximum is normally a maximum allowed count, not an error merely because the count exactly equals the maximum. Exhaustion occurs when another unit would need to be consumed.

## 8. How to review a change

Use this sequence:

~~~
history
  -> requirement
  -> invariant
  -> threat model
  -> existing contract
  -> implementation
  -> tests
  -> documentation
  -> verification
  -> release impact
~~~

Ask:

1. Is this already implemented?
2. Is the observed behavior actually wrong?
3. Which invariant is violated?
4. Is the issue code, API, test, documentation, or evidence?
5. Can the result be derived from existing authoritative state instead of adding duplicate state?
6. What is the smallest correct patch?
7. What existing behavior might break?
8. What proves the fix?
9. What evidence remains unavailable?

## 9. Real OS verification

Some claims require privileged Linux behavior.

For these, identify the exact command/test to run inside Ubuntu 24.04 Docker Desktop rather than inventing a result.

Typical examples include real bind mounts, real procfs mounts, procfs magic-link behavior, mount namespace behavior, capability-dependent filesystem tests, and kernel-version-specific syscalls.

Expected result states are:

~~~
PASS
SKIP_UNAVAILABLE
FAIL
~~~

Only PASS satisfies the corresponding security release gate.

## 10. Business and management review

Technical work is also evaluated for business value.

When material, document engineering cost, operational cost, infrastructure cost, maintenance cost, failure/recovery cost, security risk, opportunity cost, customer/product value, and reversibility.

A more complex architecture is not automatically more professional.

The preferred decision is the smallest design that satisfies correctness, security, operational, and product requirements.

## 11. What a good handoff looks like

A completed handoff should let another engineer answer:

- What changed?
- Why?
- On which branch?
- From which commit?
- Which files changed?
- Which invariants changed?
- Which tests prove it?
- Which security claims are verified?
- Which claims still require privileged OS evidence?
- What remains?
- What should the next engineer do?
- What must the next engineer not redo?

If those answers require private conversation history, the repository documentation is insufficient.
