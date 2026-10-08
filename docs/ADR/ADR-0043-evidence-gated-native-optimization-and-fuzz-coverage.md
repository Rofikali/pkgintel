# ADR-0043: Evidence-gated native optimization and fuzz coverage instrumentation

## Status

Accepted for the current v0.1 release-candidate path.

## Context

pkgintel is a system-facing C library whose current hot paths are primarily:

- bounded Dpkg metadata parsing;
- package/file correlation;
- pathname validation and filesystem metadata observation;
- descriptor-relative Linux filesystem operations;
- bounded allocation and snapshot construction.

The project must not introduce assembly, handwritten SIMD, compiler-specific vector intrinsics, custom allocators, or concurrency merely because those techniques are available.

The current fuzz release gate also exposed an evidence problem: the initial libFuzzer executable linked the fuzzer driver, but the production fuzz core and harness were not explicitly compiled with `-fsanitize=fuzzer-no-link`. The resulting run was sanitizer-clean but reported no interesting inputs and retained a one-entry corpus. That is insufficient evidence of coverage-guided effectiveness.

## Decision

### 1. Fuzz instrumentation

Compile every translation unit participating in the fuzz target with:

- `-fsanitize=fuzzer-no-link`;
- AddressSanitizer;
- UndefinedBehaviorSanitizer.

Link only the final fuzz executable with:

- `-fsanitize=fuzzer,address,undefined`.

This preserves coverage instrumentation in the code under test while adding libFuzzer's driver only once at the final executable link.

The release fuzz gate therefore distinguishes:

1. buildability;
2. sanitizer safety;
3. actual coverage-guided effectiveness.

A clean sanitizer run without real coverage/features is not a complete fuzz PASS.

### 2. Assembly/SIMD is not a current implementation requirement

No handwritten assembly, architecture-specific SIMD, or compiler intrinsic optimization is introduced for the current pkgintel v0.1 path.

The reason is workload evidence, not stylistic preference. The dominant operations currently involve parsing, string handling, filesystem syscalls, VFS metadata, allocation, and bounded result construction. There is no current benchmark/profiler evidence showing a stable CPU-bound arithmetic kernel whose optimization would materially improve end-to-end scan latency.

### 3. Optimization decision rule

Native optimization may be introduced only after:

```
representative workload
    -> reproducible benchmark
    -> profiler evidence
    -> identified hot path
    -> correctness/invariant review
    -> candidate optimization
    -> before/after benchmark
    -> sanitizer/regression verification
    -> portability/security/maintenance review
```

An optimization must improve the measured end-to-end workload, not merely a microbenchmark, unless the microbenchmark itself is an explicit product requirement.

For SIMD/assembly specifically, the candidate must also document:

- target CPU feature requirements and fallback path;
- dispatch mechanism;
- alignment and aliasing assumptions;
- integer overflow and bounds behavior;
- sanitizer/test coverage for scalar and optimized paths;
- performance gain on representative hardware;
- maintenance cost and portability impact.

### 4. Preferred optimization order

Unless profiling proves otherwise, evaluate in this order:

1. algorithm/data-structure improvements;
2. I/O and syscall behavior;
3. allocation and copying reduction;
4. compiler optimization and build configuration;
5. cache/layout improvements;
6. architecture-portable vectorization/intrinsics;
7. handwritten assembly only when the preceding evidence demonstrates a remaining material gain.

The project must not optimize a non-hot path.

## Alternatives considered

### Handwritten assembly now

Rejected. There is no evidence of a stable CPU hotspot requiring it, while it increases portability, review, testing, and maintenance cost.

### SIMD intrinsics now

Rejected for the same reason. Intrinsics are appropriate when profiling identifies a vectorizable kernel and scalar fallback semantics can remain explicit.

### Add generic optimization abstractions now

Rejected under YAGNI/KISS. An abstraction without a measured second use case increases coupling and review surface.

### Keep current fuzz link-only instrumentation

Rejected. LLVM documents that libFuzzer relies on SanitizerCoverage instrumentation and supports `-fsanitize=fuzzer-no-link` when instrumentation is required without adding the fuzzer driver's `main()`. citeturn0search0

## Consequences

Positive:

- fuzz evidence becomes technically meaningful rather than merely sanitizer-clean;
- performance work remains evidence-driven;
- security-sensitive code avoids unnecessary architecture-specific complexity;
- scalar correctness remains the reference behavior;
- future SIMD/assembly work has explicit acceptance criteria.

Negative:

- maximum theoretical CPU throughput may be lower than a specialized implementation;
- profiling and benchmark work must precede optimization;
- a future optimized kernel may require multiple code paths and additional CI coverage.

## Re-evaluation trigger

Revisit this ADR when a reproducible representative benchmark and profiler show a CPU-bound function consuming a material share of scan time and a candidate native optimization produces a measured end-to-end benefit large enough to justify its complexity.

Until that evidence exists, assembly/SIMD is explicitly deferred rather than considered missing functionality.
