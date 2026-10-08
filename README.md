# pkgintel

`pkgintel` is a C17/Linux software-environment intelligence engine.

The project discovers, normalizes, correlates, and explains package metadata and filesystem observations without modifying the target.

## v0.1 implementation scope

The current v0.1 vertical slice is intentionally narrower than the long-term product vision:

- Linux / Debian-compatible targets
- dpkg package database enumeration with explicit installed/partial/removed state
- selected package-owned-file correlation
- controlled filesystem observation
- package/artifact/diagnostic result model
- read-only target confinement
- resource-bounded package metadata parsing
- opaque C API with an explicit, tested exported-symbol allowlist
- basic CLI and installable CMake package

The following are **planned, not implemented v0.1 functionality**:

- ELF inspection
- capability inference
- APT metadata/cache analysis
- JSON serialization
- full filesystem crawling
- vulnerability/SBOM/commercial features
- Rust FFI

The public cache/capability headers and snapshot accessors are deliberately not part of the current v0.1 API. They will be introduced only when their data model, ownership, resource accounting, security behavior, tests, and ABI surface are ready for review.

## Engineering skills

The repository engineering capability contract is in [SKILLS.md](SKILLS.md), covering systems/C, HLD/LLD, SOLID, design-pattern reasoning, mathematics, security, ABI/FFI, testing, performance, CA/Finance, and MBA/Management/Product decision-making.

## Engineering onboarding

New agents and engineers must start with:

- `AGENTS.md`
- `docs/AGENT_ONBOARDING.md`
- `docs/ENGINEERING_STATUS.md`
- `docs/ENGINEERING_WORKFLOW.md`
- `docs/BRANCH_PROVENANCE.md`

These documents define branch provenance, anti-duplication rules, evidence standards, security verification, and the Staff/Principal Engineer + CA/MBA/Management review discipline.

## Engineering rule

> Build the smallest implementation that validates the largest architectural assumptions.

Performance follows the same rule: **measure first, optimize second**. C17 and compiler optimization are the v0.1 baseline. Hand-written assembly is deliberately excluded unless a later evidence gate proves a material end-to-end benefit that justifies its security, portability, CI, and maintenance cost. See `docs/ADR/ADR-0040-assembly-policy.md`.

See `docs/ARCHITECTURE.md`, `docs/API.md`, and `docs/SECURITY.md` for the engineering contract.

## Engineering design discipline

pkgintel is developed using an evidence-backed Principal/Staff Engineer discipline. Architecture is documented at both high and low levels, and algorithms/data structures are reviewed with mathematical bounds, complexity, security implications, tests, and benchmarks. Design principles such as separation of concerns, cohesion/coupling, DRY, SOLID, reuse, encapsulation, and design patterns are applied as decision tools rather than as rules to satisfy for their own sake.

- `docs/ENGINEERING_PRINCIPLES.md` — engineering principles and decision method
- `docs/HLD.md` — high-level architecture and boundaries
- `docs/LLD.md` — low-level contracts, ownership, invariants, and failure semantics
- `docs/ALGORITHMS.md` — algorithms/data structures and selection policy
- `docs/MATHEMATICS.md` — complexity, overflow, and resource-bound reasoning
- `docs/ARCHITECTURE.md` — current architecture contract
- `docs/ENGINEERING_GATES.md` — verification and release gates
- `docs/ENGINEERING_STATUS.md` — current implementation, evidence, branch lineage, and P0 handoff status
