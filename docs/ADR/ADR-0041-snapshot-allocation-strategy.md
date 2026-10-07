# ADR-0041: Snapshot Allocation Strategy

- **Status:** Accepted
- **Date:** 2026-10-07
- **Decision:** Retain explicit `malloc`/`realloc` ownership in v0.1; do not introduce an arena/slab allocator without workload evidence.
- **Scope:** Snapshot packages, artifacts, diagnostics, and owned strings.

## Context

The snapshot contains variable-length package strings, artifact paths, diagnostic strings, and growable record arrays. Dynamic allocation therefore exists at several ownership boundaries.

Dynamic allocation creates real failure and security cases: integer overflow before allocation, allocation exhaustion, ownership mistakes, partial construction, use-after-free, and resource-exhaustion denial of service. Those risks must be controlled explicitly.

Replacing dynamic allocation with SIMD would not address these risks. SIMD is a data-parallel computation technique, while allocation is a storage/ownership concern.

An arena or slab allocator could reduce allocation count and simplify destruction, but it would introduce new invariants: arena capacity arithmetic, alignment, lifetime coupling, fragmentation policy, failure semantics, and potentially larger peak reservations. There is currently no representative profiling or allocation telemetry proving that this complexity is justified.

## Decision

For v0.1:

1. Keep standard `malloc`, `calloc`, `realloc`, and `free`.
2. Validate all attacker-influenced size additions and multiplications before allocation.
3. Check every allocation result.
4. Preserve transactional mutation: failed allocation must not increase the committed record count or expose partially initialized records.
5. Keep aggregate record ceilings independent from caller-requested limits.
6. Treat allocation failure as an internal failure and resource exhaustion as a distinct controlled result where a configured/hard resource ceiling is reached.
7. Keep private mutation functions outside the public ABI.
8. Test the public scan contract rather than making external tests depend on private allocation internals.
9. For deterministic allocator-failure testing, compile a test-only static copy of the production sources with allocation symbols redirected to a test allocator. Do not rely on executable-level linker `--wrap` to intercept allocations made by a separately linked shared library.

## Allocation-failure test architecture

The allocator-failure test intentionally uses a test-only static library built from the same production source list. Its `malloc`, `calloc`, and `realloc` calls are compile-time redirected to test allocator functions defined only by the test executable. This makes failure injection reach actual library-owned allocation sites while leaving the shipped shared-library ABI and production build unchanged.

The earlier executable-level linker-`--wrap` approach was rejected because the unit test links against the shared library; wrapping symbols in the executable does not provide a reliable guarantee that allocations originating inside that already-linked shared object will pass through the test wrappers.

The gate therefore requires both:
- deterministic failure at each observed library allocation call; and
- a successful baseline scan proving the same test path exercises the intended allocation sites.

## Why not an arena yet?

An arena is attractive when many short-lived objects share one lifetime and allocation overhead is a measured bottleneck. That is plausible for pkgintel, but not yet demonstrated.

Before adopting one, measure representative scans for:

- allocation count by object class;
- bytes allocated by object class;
- peak live snapshot bytes;
- realloc growth/copy cost;
- allocation failure behavior under constrained memory;
- scan CPU time attributable to allocation;
- peak RSS and fragmentation.

If allocation overhead is immaterial relative to filesystem and parser work, an arena would add complexity without meaningful product value.

## Security requirements for any future allocator

A future arena/slab implementation must preserve:

- checked size arithmetic;
- bounded total capacity;
- deterministic failure at capacity;
- alignment correctness;
- object lifetime/ownership invariants;
- no access after arena destruction;
- transactional snapshot mutation;
- sanitizer/fuzz coverage;
- equivalent public API and error semantics.

A custom allocator is therefore a security-sensitive architectural change, not a cosmetic performance optimization.

## SIMD and assembly distinction

SIMD/assembly is governed separately by ADR-0040. It can optimize CPU-bound data processing after profiling identifies a material hotspot. It cannot replace memory allocation and must not be introduced as a response to allocator-safety concerns.

## Re-evaluation gate

Revisit this ADR only when representative profiling or production-like memory measurements demonstrate that allocation overhead, fragmentation, or peak snapshot memory is a material bottleneck or security/resource concern. Any replacement must include benchmark evidence, failure-injection tests, sanitizer coverage, and an explicit ownership model.
