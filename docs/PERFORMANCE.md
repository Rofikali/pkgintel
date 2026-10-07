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
