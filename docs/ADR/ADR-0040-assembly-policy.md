# ADR-0040: Evidence-Based Assembly Policy

- Status: Accepted
- Date: 2026-10-07

## Context

pkgintel is being engineered as a small, security-sensitive C17 systems library. The project must choose implementation technology based on measurable requirements rather than prestige, perceived low-levelness, or benchmark folklore.

Assembly can provide value when a measured hot path benefits from an instruction-set-specific implementation that the compiler cannot produce adequately. It also introduces substantial costs: portability constraints, ABI/PCS risk, compiler and linker complexity, sanitizer/debugging friction, maintenance burden, and additional review surface for security-sensitive code.

The current v0.1 workload is dominated by package metadata parsing, bounded text processing, filesystem observation, path-resolution policy, snapshot construction, diagnostics, and allocation. No demonstrated hot loop currently requires hand-written assembly.

## Decision

**Do not introduce hand-written assembly into the v0.1 production library.**

C17 remains the implementation contract. The compiler's optimized output is the default implementation strategy.

Assembly may enter a later performance-critical path only through an explicit evidence gate:

1. A representative benchmark identifies a stable CPU hotspot.
2. Profiling shows the hotspot materially affects end-to-end scan latency or throughput.
3. A portable C implementation exists and is covered by correctness, sanitizer, and security tests.
4. Compiler optimization output has been inspected; the expected instruction sequence cannot reasonably be obtained from portable C/compiler intrinsics, or the assembly provides a material measurable advantage.
5. The proposed assembly is isolated behind a narrow internal interface and does not become public ABI.
6. CI covers every supported architecture/toolchain combination for which the assembly is enabled, with a tested portable fallback where required.
7. Benchmark evidence demonstrates a meaningful end-to-end improvement, not merely a microbenchmark win.
8. Code review covers calling convention, register preservation, stack alignment, clobbers, speculation-sensitive behavior, fault behavior, and platform-specific assumptions.
9. Security review confirms that the assembly does not weaken bounds checks, constant-time requirements where applicable, control-flow integrity assumptions, or sanitizer coverage.
10. Documentation records the measured reason for the assembly and the fallback policy.

## Current evidence

The v0.1 architecture does not currently have a demonstrated computational hotspot where hand-written assembly is justified. The principal costs are expected to come from filesystem I/O, metadata parsing, system calls, path resolution, allocation, and result construction. Optimizing a small CPU instruction sequence before measuring end-to-end behavior would therefore be premature optimization.

## Why not assembly now?

### Engineering

The project is still stabilizing its domain semantics, security boundaries, ABI, and resource accounting. Assembly would increase implementation complexity while those contracts are still moving.

### Security

Hand-written assembly creates another language-level boundary where memory safety assumptions, register state, stack discipline, and control-flow behavior must be audited. That cost is justified only by evidence of material benefit.

### Operations and management

Each architecture-specific implementation increases CI combinations, release risk, maintenance cost, onboarding cost, and incident-debugging complexity. A speedup that does not improve end-to-end service economics is not automatically valuable.

### CA/MBA perspective

The relevant decision metric is not “assembly is faster.” It is:

`incremental engineering cost + security/release risk` versus `measured end-to-end performance value + resulting capacity/cost benefit`.

A microbenchmark improvement with no meaningful effect on scan throughput, latency, infrastructure cost, or user-facing capacity does not justify the additional maintenance liability.

## What evidence would change the decision?

Assembly becomes a candidate when measurements show something like:

- a CPU kernel consumes a significant fraction of total scan CPU time;
- the kernel is called often enough for optimization to affect end-to-end latency;
- compiler-generated code leaves a material performance gap;
- the improvement survives realistic fixtures and not only synthetic data;
- the gain is large enough to offset additional CI/security/maintenance cost.

The exact threshold is not fixed in advance. It must be evaluated against real profiling data and product requirements.

## Preferred optimization order

1. Correct algorithm and data model.
2. Correct I/O and syscall behavior.
3. Allocation and ownership efficiency.
4. Compiler optimization and LTO where appropriate.
5. Portable C optimization based on profiling.
6. Compiler intrinsics where they provide a clear, maintainable benefit.
7. Architecture-specific assembly only when the evidence gate passes.

## ABI policy

Assembly is an implementation detail. It must never be added to the public ABI merely because an optimized implementation exists.

## Current decision

For v0.1: **NO HAND-WRITTEN ASSEMBLY.**

This is not an anti-assembly rule. It is a measurement rule: if evidence later demonstrates that assembly materially improves the real product, the project should use it deliberately and document why.