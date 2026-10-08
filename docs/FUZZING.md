# Fuzzing Contract

## Purpose

pkgintel uses fuzzing to exercise bounded parser and filesystem-correlation behavior against attacker-controlled bytes. The fuzz targets are evidence tools, not replacements for deterministic unit tests or genuine Linux security-runtime tests.

## Current targets

### dpkg status

`tests/fuzz/fuzz_dpkg_status.c` drives arbitrary bytes into the Dpkg status-record input through the public `pkg_scan()` contract.

The harness appends a known-good package record so that malformed input and a valid terminal record are both exercised.

### dpkg package file list

`tests/fuzz/fuzz_dpkg_filelist.c` drives arbitrary bytes into a package `.list` file through the public `pkg_scan()` contract.

The harness uses a fixed valid package metadata record and varies the package-file evidence.

## Safety properties

Each target:

- uses a deterministic temporary target root;
- exercises production code through the public scan API;
- keeps the fuzz input bounded at 128 KiB per iteration;
- enables package-file correlation;
- applies package/package-file resource ceilings;
- destroys returned snapshots;
- treats unexpected internal errors as harness failures.

The production sources and harness translation units are compiled with Clang SanitizerCoverage via `-fsanitize=fuzzer-no-link` plus AddressSanitizer and UndefinedBehaviorSanitizer. The final fuzz executable link adds `-fsanitize=fuzzer,address,undefined`, which supplies the libFuzzer driver. This compile/link split is intentional: coverage instrumentation belongs in the code under test, while the fuzzer `main()` is added only at the final link.

## Build

Fuzzing is opt-in and does not alter the normal developer build:

```bash
cmake -S . -B build-fuzz -G Ninja \
  -DCMAKE_C_COMPILER=clang \
  -DPKGINTEL_BUILD_TESTS=ON \
  -DPKGINTEL_BUILD_FUZZERS=ON
cmake --build build-fuzz
```

## Run

Use the committed seed corpus first:

```bash
./build-fuzz/pkgintel_fuzz_dpkg_status \
  tests/fuzz/corpus/dpkg_status
```

```bash
./build-fuzz/pkgintel_fuzz_dpkg_filelist \
  tests/fuzz/corpus/dpkg_filelist
```

For a bounded verification run, add an explicit time limit, for example:

```bash
./build-fuzz/pkgintel_fuzz_dpkg_status \
  -max_total_time=60 \
  tests/fuzz/corpus/dpkg_status
```

Repeat for `pkgintel_fuzz_dpkg_filelist`.

## Evidence rule

A fuzz target being buildable is not a fuzzing PASS.

A fuzz gate requires:

1. both targets build successfully with Clang;
2. both targets execute successfully under ASan/UBSan + libFuzzer;
3. no sanitizer finding is reported;
4. the fuzzer reports real coverage/features (for example `cov:` / `ft:`) rather than the `no interesting inputs` instrumentation warning;
5. a bounded run demonstrates corpus growth or an explicit coverage analysis explains why the existing corpus is already minimal;
6. the exact command, environment, duration, and result are recorded;
7. the seed corpus remains reproducible.

A future CI/release job must run the targets with an explicit bounded time budget. Do not claim fuzzing coverage merely because a target exists or because ASan/UBSan are clean. Sanitizer safety and coverage-guided effectiveness are separate release properties.

## Design decision

The initial fuzz boundary is deliberately the Dpkg parser/correlation path. Filesystem mount semantics remain covered by the genuine privileged security test because simulating those kernel primitives inside a fuzz harness would weaken the evidence model.

New fuzz targets should be added only when they cover a distinct attack surface or invariant that cannot be adequately exercised by the existing targets and deterministic tests.
