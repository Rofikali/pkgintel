# Engineering Gates

This document is the release discipline for pkgintel. A feature is not complete because it compiles or works on one developer machine.

## Gate 0 — Design

Before implementation:

- domain responsibility is identified;
- public/private boundary is explicit;
- threat model is reviewed;
- ownership/lifetime is defined;
- failure modes are defined;
- resource limits are identified;
- target-boundary implications are reviewed.

## Gate 1 — Build

Required configurations:

- GCC + Debug;
- GCC + Release;
- Clang + Debug;
- Clang + Release;
- Clang + ASan/UBSan.

No new warning should be accepted casually.

## Gate 2 — Correctness

Every feature gets deterministic unit tests and, where applicable, real Ubuntu/Debian integration fixtures.

Tests must cover:

- happy path;
- missing data;
- malformed data;
- permission failure;
- resource limit;
- empty input;
- boundary values;
- ownership/lifetime behavior.

## Gate 3 — Security

For system-facing code:

- no uncontrolled process execution;
- no target-root escape;
- no unbounded parser allocation;
- integer overflow checked;
- symlink policy tested;
- special-file policy tested;
- hostile input fixture added;
- fuzz target considered or added.

## Gate 4 — Performance

Measure before optimizing. Record:

- wall time;
- CPU time;
- peak RSS;
- files examined;
- bytes read;
- package count;
- diagnostics count.

Performance claims require a reproducible fixture and environment.

## Gate 5 — API/ABI

Public C API changes require:

- ownership review;
- error semantics review;
- thread-safety review;
- symbol visibility review;
- compatibility impact review.

v0.1 is not ABI-stable. ABI stability is a release milestone, not an assumption.

## Gate 6 — Documentation

Behavior changes require documentation updates. At minimum, update the relevant architecture, API, security, and CLI contracts.

## Gate 7 — Release

A release candidate requires all applicable gates to pass and a written record of known limitations. Unknown behavior is not silently classified as success.
