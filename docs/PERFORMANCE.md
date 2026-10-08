# Performance Baseline

Performance optimization is an evidence gate, not a source-code preference.

## Purpose

This benchmark establishes a reproducible v0.1 baseline for the implemented dpkg scan path. It intentionally uses a synthetic Debian-like target with controlled package and file-list cardinality so changes can be compared against the same workload.

Build the benchmark with the normal test configuration:

```sh
cmake -S . -B build -G Ninja -DPKGINTEL_BUILD_TESTS=ON
cmake --build build --target pkgintel_benchmark
```

Run the default workload:

```sh
./build/pkgintel_benchmark
```

Arguments are:

```text
pkgintel_benchmark <packages> <files_per_package> <iterations>
```

For example:

```sh
./build/pkgintel_benchmark 100 100 20
./build/pkgintel_benchmark 1000 100 10
```

## Recorded metrics

The benchmark reports:

- workload cardinality;
- wall-clock time;
- user CPU time;
- system CPU time;
- peak resident set size reported by `getrusage()`;
- observed package/artifact counts;
- artifacts per second.

Linux `perf stat` can provide additional CPU-level evidence such as task-clock, cycles, instructions, branches, cache behavior, and page faults. The Linux kernel documentation describes `perf stat` as a tool for collecting hardware/software performance events.

Do not compare raw numbers from unrelated machines as if they were equivalent. CPU frequency/boost behavior can make results vary; reproducible benchmark environments are therefore required for meaningful comparisons.

## Current v0.1 evidence checkpoint

A Release benchmark was executed on the Docker/Ubuntu development environment and repeated three times per workload. The reproducibility metadata is:

- source SHA: `4b02a3807fc6c923bb0ebc773385210773334379` (local checkout used for the measurement);
- GCC: 13.3.0;
- Clang: 18.1.3;
- CPU: 12th Gen Intel(R) Core(TM) i5-1235U;
- logical CPUs: 12;
- physical cores: 6;
- sockets: 1;
- kernel: Linux 5.15.167.4-microsoft-standard-WSL2;
- OS: Ubuntu 24.04.5 LTS;
- build type: Release;
- workload: synthetic Debian-like target with controlled package/file-list cardinality.

The local checkout SHA is an ancestor of the current P0 branch head; the benchmark source contract was unchanged by the later documentation-only branch changes. The measurements therefore remain valid as evidence for the implemented benchmark path, while the exact final release baseline should be re-run at the final release head.

### End-to-end scan benchmark

| Workload | Iterations | Runs | Mean artifacts/sec | Min | Max | Sample CV |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 100 packages × 100 files | 20 | 3 | 792,452 | 684,997 | 860,689 | 11.88% |
| 1,000 packages × 100 files | 10 | 3 | 871,385 | 795,557 | 924,986 | 7.75% |
| 5,000 packages × 20 files | 5 | 3 | 710,472 | 682,927 | 725,758 | 3.36% |

The repeated runs show no monotonic degradation and preserve the same broad throughput range as the initial measurement. The smaller 100 × 100 workload has the largest run-to-run variation, while the larger workloads are more stable. This is sufficient reproducibility evidence for the current performance checkpoint, but it is not a machine-independent regression threshold.

The runs also continue to show substantially more system CPU time than user CPU time. That is consistent with the benchmark exercising filesystem/syscall-heavy synthetic correlation work, but it is a hypothesis about where time is spent rather than profiler proof. No allocator, SIMD, or assembly change is justified from this observation alone.

### Allocation evidence

The allocation benchmark was repeated three times for the 1,000 × 100 workload:

| Workload | Observed artifacts | Mean artifacts/sec | Peak live allocator bytes | Artifact malloc calls | Total malloc calls |
| --- | ---: | ---: | ---: | ---: | ---: |
| 1,000 × 100 | 500,000 | 829,423 | 10,466,120 | 500,000 | 535,005 |

All three allocation runs reported identical allocation counts and live-byte totals, with final live bytes of zero. Throughput ranged from 808,654 to 847,314 artifacts/sec (sample CV 2.35%).

The allocation evidence continues to show one artifact allocation per observed artifact in this synthetic workload, with reallocation concentrated in growable storage. These measurements do **not** justify an arena/slab allocator.

### Engineering decision

**Decision: performance reproducibility checkpoint PASS; retain the current allocation strategy and defer assembly/SIMD and allocator redesign.**

The repeated benchmark requirement and required environment metadata have now been captured. The evidence is strong enough to establish a reproducible v0.1 performance checkpoint, but it does not establish a universal performance SLA or regression threshold.

Native optimization remains evidence-gated:

1. representative workload;
2. reproducible benchmark;
3. profiler evidence;
4. identified hot path;
5. correctness/invariant review;
6. candidate optimization;
7. before/after end-to-end benchmark;
8. sanitizer/regression verification;
9. portability, security, and maintenance review.

The final release process must re-run the applicable benchmark at the final release head if any performance-sensitive source changes occur after this checkpoint.


## Allocation instrumentation

A separate benchmark target provides allocation evidence without changing the
production shared library:

```sh
cmake --build build --target pkgintel_benchmark_alloc
./build/pkgintel_benchmark_alloc 100 100 5
```

The instrumented target compiles the production sources into a private static
copy and redirects their `malloc`, `calloc`, `realloc`, and `free` calls to a
benchmark-only probe. It reports:

- allocation call counts;
- requested bytes by allocation family;
- realloc requested bytes;
- realloc growth/shrink bytes;
- peak live allocator bytes;
- final live allocator bytes.

The live-byte measurement uses glibc `malloc_usable_size()` for diagnostic
accounting. This is benchmark evidence only; it is not part of the product
ABI and must not be used to infer portable allocator semantics.

Run the allocation benchmark over the same workload shapes used by the normal
baseline:

```sh
./build/pkgintel_benchmark_alloc 1000 100 5
./build/pkgintel_benchmark_alloc 100 1000 5
./build/pkgintel_benchmark_alloc 5000 20 5
```

The allocation probe now also classifies allocation callsites by production module:
context, scan, snapshot, package, artifact, diagnostic, target, dpkg, and support.
This is deliberately a callsite-level attribution, not an ownership graph: an
allocation or free is attributed to the source module that issued that call.
Use this evidence to identify which module's allocation activity changes with
workload shape before considering finer-grained object-class instrumentation.

The allocation probe deliberately does not classify every allocation by source
object yet. That is a later profiling refinement if aggregate evidence shows
allocation behavior is material. Do not use this instrumentation alone to
justify an arena/slab allocator.

## Allocation decision gate

This benchmark does **not** claim that an arena/slab allocator is justified.

Before changing the allocation strategy, collect:

1. allocation count by object class;
2. allocated bytes by object class;
3. peak live snapshot bytes;
4. realloc growth/copy cost;
5. allocation-related CPU time;
6. peak RSS;
7. end-to-end scan time.

The current accepted decision remains ADR-0041: standard allocation is retained until representative evidence shows allocation overhead, fragmentation, or memory footprint is materially affecting the product.

## Benchmark hygiene

Record at minimum:

- commit SHA;
- compiler and version;
- build type;
- CPU model;
- core count;
- kernel version;
- workload dimensions;
- iteration count;
- benchmark output.

Do not treat a single run as a regression threshold. Use repeated runs and compare distributions or stable summaries.

The benchmark is diagnostic evidence. It is not a correctness test and is not registered as a CTest test.
