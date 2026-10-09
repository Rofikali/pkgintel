# P1 JSON Export — Engineering Handoff

## Status and provenance

**Status: MERGED.** P1 JSON export is part of `main` through PR #7.

- PR: https://github.com/Rofikali/pkgintel/pull/7
- Reviewed PR head: `a75214124d0d1027c1eb1e9eedd2a358832a84c1`
- Merge commit: `8b4380d4f353017854a2104ac5a39a8029675c66`
- Successful CI run for the reviewed head: https://github.com/Rofikali/pkgintel/actions/runs/37882487979
- Verification environment for CI: GitHub-hosted Linux runners; the dedicated security-runtime job exercises genuine Linux filesystem primitives.
- This handoff is a post-merge documentation update. Do not confuse its documentation commit with the implementation SHA used by the CI evidence above.

The P1 implementation is merged because the exact PR head completed its required CI gates and the final source review found no blocker in the reviewed serialization, CLI, schema, fuzz, or build boundaries.

## What shipped

The CLI supports versioned JSON export through:

```sh
pkgintel scan --json
```

Implementation and contract locations:

- `src/json/json.c` — streaming JSON writer.
- `src/internal/json.h` — private serializer interface.
- `src/cli/main.c` — CLI integration.
- `tests/unit/test_json.c` and `tests/unit/test_main.c` — serializer unit coverage.
- `tests/json/validate_cli_json.py` — independent JSON parser validation.
- `tests/fuzz/fuzz_dpkg_status.c` and `tests/fuzz/fuzz_dpkg_filelist.c` — parser fuzzing that invokes JSON serialization on usable snapshots.
- `tests/benchmark/benchmark_scan.c` — separate scan/serialization measurements.
- `docs/P1_JSON_SCHEMA.md` — frozen v1 contract.
- `docs/P1_JSON_HLD_LLD.md` — design and implementation detail.
- `docs/P1_CAPABILITY_DECISION.md` — why JSON was selected.

## Architecture and invariants

```text
target -> scan -> immutable snapshot -> public read-only accessors
       -> CLI-private JSON projection -> stdout
```

- The snapshot/domain model remains authoritative; JSON is only a projection.
- The serializer borrows the snapshot and does not own or mutate it.
- The serializer does not scan, reopen/re-resolve target paths, follow target links, invoke package managers, execute discovered values, or modify the inspected target.
- JSON serialization is CLI-private and is not a new public C serializer ABI; the shared-library ABI remains separate.
- Schema version and native API/SONAME version evolve independently.
- Scan outcome is explicit. A resource-limited scan must not be presented as a complete observation.
- Byte-oriented fields use standard padded RFC 4648 Base64 so arbitrary Linux bytes are represented losslessly.
- For an identical immutable snapshot and outcome, serialization promises deterministic bytes.
- The writer streams output rather than building a full intermediate JSON DOM.
- A write/flush/encoding/internal failure returns non-success. Partial stdout may exist; a non-zero exit means consumers must not treat it as a complete JSON document.

## Exact verification evidence

Successful workflow: [run 37882487979](https://github.com/Rofikali/pkgintel/actions/runs/37882487979), for reviewed PR head `a75214124d0d1027c1eb1e9eedd2a358832a84c1`.

- GCC Debug and Release: build, CTest, exported ABI audit, private-symbol audit, installed-consumer verification — PASS.
- Clang Debug and Release: same checks — PASS.
- Clang ASan/UBSan build, CTest, sanitized install — PASS.
- Independent CLI JSON parser test — PASS in the compiler/configuration matrix.
- Both JSON-aware libFuzzer targets — PASS.
- Dedicated strict Linux `security-runtime` job — PASS; genuine filesystem mount-boundary/procfs security test ran, not skipped.
- ABI/private-symbol and installed-consumer checks — PASS in the matrix.
- Benchmark ran in GCC Release. Its omission in other matrix jobs is by design, not a test failure.

### Security qualification — preserve this wording

There are two distinct test environments and results:

1. **Dedicated strict Linux filesystem security runtime: PASS.** It ran the genuine mount-boundary test under the configured strict security mode. This is the evidence for the real Linux filesystem primitive.
2. **Generic sanitized CTest job: the mount test was SKIPPED.** That job runs unprivileged and cannot guarantee the mount capability. The job's remaining sanitizer/tests/install steps passed. The skip is not a security pass and must never be reported as one.

Keep the ordinary developer environment least-privileged. Do not make its security test pass by weakening assertions, fabricating mounts, or converting skip status into success. If a future runtime cannot provide the required primitive, record SKIP in permissive mode or fail in strict mode.

### Fuzz resource budget

The CI fuzz environment explicitly sets:

```sh
ASAN_OPTIONS=quarantine_size_mb=64:thread_local_quarantine_size_kb=256:detect_leaks=1:abort_on_error=1
```

Observed results on the reviewed head:

- Dpkg status parser + JSON serialization: 35,832 executions; peak observed RSS about 143 MiB.
- Dpkg file-list parser + JSON serialization: 28,112 executions; peak observed RSS about 146 MiB.

An earlier run using default ASan quarantine showed about 420 MiB RSS. That is historical context; the bounded-quarantine results above are the current reviewed evidence. Do not generalize these short fuzz runs into proof of absence of defects or a universal memory ceiling.

### Initial benchmark baseline

GitHub-hosted Ubuntu runner; measurements are observations, not proof of improvement against a previous version:

| Workload | Scan wall time | Serialization time | JSON output | Approx. serialization throughput |
|---|---:|---:|---:|---:|
| 100 packages × 100 files, 3 iterations; 30,000 artifacts observed | 0.04907 s | 0.02078 s | 9,557,661 bytes | 460 MB/s |
| 1,000 packages × 20 files, 1 iteration; 20,000 artifacts observed | 0.04039 s | 0.01483 s | 6,656,187 bytes | 449 MB/s |

These are limited single-run CI measurements affected by runner noise and cache state. Repeat representative workloads before making comparative performance claims.

## Reproduce locally

Use the repository's documented Ubuntu 24.04-in-Docker workflow and inspect the current checkout first. Do not assume the checkout is at the merge SHA.

```sh
git status --short --branch
git fetch origin
git log -1 --oneline --decorate
```

Configure/build/test the CLI and unit tests:

```sh
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DPKGINTEL_BUILD_TESTS=ON \
  -DPKGINTEL_BUILD_CLI=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Run the independent JSON parser test through CTest as registered by the project. For an installed CLI, use a known test target/fixture and validate stdout as JSON; do not use progress or diagnostic text as JSON input. Inspect `--help` and the current CLI contract before relying on arguments beyond `scan --json`.

Sanitizer build (adjust compiler to match the current CI workflow):

```sh
cmake -S . -B build-sanitized -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DPKGINTEL_BUILD_TESTS=ON \
  -DPKGINTEL_BUILD_CLI=ON \
  -DCMAKE_C_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined' \
  -DCMAKE_SHARED_LINKER_FLAGS='-fsanitize=address,undefined'
cmake --build build-sanitized
ASAN_OPTIONS=detect_leaks=1:abort_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
ctest --test-dir build-sanitized --output-on-failure
```

For genuine privileged filesystem verification, use the dedicated security service and exact commands in `docs/ENGINEERING_STATUS.md` / `docs/SECURITY.md`. A normal unprivileged CTest run is not a substitute for the strict runtime gate.

For authoritative current commands, use `.github/workflows/ci.yml` rather than copying old commands from this handoff blindly.

## Scope boundaries — not shipped by P1

Do not assume any of the following exist because JSON export shipped:

- HTTP/network API or daemon;
- database, cloud persistence, or background queue;
- ELF analysis;
- APT cache/metadata analysis;
- broad filesystem crawling;
- Rust FFI;
- public C JSON serializer ABI;
- vulnerability detection, SBOM completeness, or commercial security claims.

The JSON document describes only what the existing scan/snapshot actually observes. It must not imply a complete inventory of the operating system or infer vulnerabilities/capabilities that pkgintel has not established.

## Handoff checklist for every agent/engineer

Before the next change:

1. Read `AGENTS.md`, `docs/AGENT_ONBOARDING.md`, `docs/ENGINEERING_STATUS.md`, `docs/ENGINEERING_WORKFLOW.md`, `docs/BRANCH_PROVENANCE.md`, and `docs/PRINCIPAL_ENGINEERING_REVIEW_PROTOCOL.md`.
2. Inspect current `main` SHA, working tree, branch ancestry, open PRs, relevant commits, tests, and ADRs. Never start from memory alone.
3. Read the JSON schema and HLD/LLD before modifying output fields, enums, byte encoding, ordering, or error semantics.
4. Treat schema v1 as a compatibility contract. A breaking change requires an explicit schema-version decision and migration story.
5. Preserve the public/private boundary: do not add serializer symbols to the shared library by convenience.
6. Add adversarial tests when changing serialization, parser input, output failure handling, or resource accounting. A successful parse alone is not semantic correctness.
7. Keep scan timing and serialization timing separate; benchmark representative cases and disclose limitations.
8. Run applicable GCC/Clang Debug/Release, sanitizers, fuzz, ABI/private-symbol, installed-consumer, and strict security-runtime gates for relevant changes.
9. Tie every PASS to the exact SHA, workflow run, environment, and tested property. Distinguish source, unit, sanitizer/fuzz, real-runtime, benchmark, and production-like evidence.
10. Never report SKIP as PASS. The dedicated strict filesystem runtime is the security evidence; the unprivileged sanitized job is not.
11. Update the schema/HLD/LLD, engineering status, README, and ADRs when contracts or decisions change.
12. Do not merge merely because a job is green; review exact diff, provenance, and all required evidence.

## Recommended next step

P1 JSON export is delivered. Do not immediately add another capability by momentum. First review product/customer value and the existing capability backlog; identify the smallest next experiment that creates measurable integration value without widening the security surface unnecessarily. Record the choice in an ADR/capability decision before implementation.

## Engineering principles

> Observe, do not modify. Observe, do not execute. Confine, do not escape. Bound, do not exhaust. Validate, do not trust. Explain, do not overclaim.
