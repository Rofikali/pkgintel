# pkgintel Architecture

## 1. Architectural corrections to v0.1 proposal

The original proposal is strong, but several concepts need tightening before implementation:

1. **C17 is the implementation contract.** Do not introduce C++ or C++-style ownership abstractions. The repository and build must enforce C17.
2. **Package, installation, and artifact remain distinct.** A package is metadata; an installation is package state within a target; an artifact is an observed filesystem object.
3. **The target is the security boundary.** Every filesystem operation must resolve relative to the target. No subsystem may silently escape the target root.
4. **dpkg and APT are adapters.** Generic domain structures must not contain dpkg/apt-specific types.
5. **Discovery is evidence-producing, not truth-producing.** Results preserve provenance and confidence; permission failures and unsupported metadata are observations, not silent omissions.
6. **The first vertical slice must be narrow.** The current slice is context → target → dpkg package enumeration → explicit installation-state classification → selected package-file correlation → diagnostics → basic CLI. ELF, capabilities, APT, JSON, and other expansion work follow only after the existing boundary is proven.
7. **Do not promise full filesystem correctness in v0.1.** Package-owned-file verification is bounded and policy-driven; a full / crawl is not the default.
8. **Do not expose size ambiguously.** Use named semantics such as `logical_size`, `allocated_size`, and `package_installed_size`.
9. **JSON is an output contract, not the domain model.** When implemented, version the JSON schema independently from the C ABI.
10. **Stable ABI comes after API review and ABI testing.** v0.1 can expose an explicitly versioned C API, but the project must not claim ABI stability until ABI checks and compatibility policy exist.
11. **Unimplemented controls must not be advertised as enforced controls.** Reserved resource-limit fields are rejected when non-zero until the corresponding accounting exists.
12. **Unimplemented scan features must fail closed.** ELF, cache, and capability feature flags are reserved API surface in v0.1 and are rejected with `PKG_ERR_UNSUPPORTED`; they must never be accepted and silently ignored.
13. **Optimization must be evidence-driven.** v0.1 does not use hand-written assembly. Assembly is permitted only after a measured end-to-end hotspot, compiler-output review, security review, and architecture-specific CI/fallback plan justify its maintenance cost. See `docs/ADR/ADR-0040-assembly-policy.md`.

## 2. Source, header, and interface boundaries

The repository uses a deliberate three-level contract model:

| Location | Meaning | Consumer | Compatibility expectation |
|---|---|---|---|
| `include/pkgintel/` | **Supported external contract** | Library users, CLI, future Rust FFI, external applications | Public API/ABI review applies |
| `src/` | **Implementation** | pkgintel implementation only | May change as internals evolve |
| `src/internal/` | **Implementation-only shared contract** | Multiple internal modules that need a shared private contract | Not public API; may change without external compatibility guarantees |

### 2.1 `include/` = supported external contract

Anything under `include/pkgintel/` is potentially consumed by code outside the repository. Public headers therefore contain only intentionally supported API concepts: opaque public objects, enums/status values, options, accessors, lifecycle functions, and other reviewed ABI surface.

A public header must not include a private implementation header or expose private struct layout merely for implementation convenience.

The fact that a header is public does not automatically mean ABI stability has been promised. ABI stability is a separate release gate.

### 2.2 `src/` = implementation

Source files under `src/` implement the library and CLI. Implementation details, algorithms, private data structures, platform-specific code, and internal orchestration belong here.

Internal source files may include public headers and private internal headers as required by their module contract. External consumers must never be required to include a file from `src/`.

### 2.3 `src/internal/` = implementation-only shared contract

`src/internal/` contains private headers that multiple implementation modules need to share. These headers are source-level contracts between internal modules, not public API.

Current private contracts are intentionally split by responsibility:

- `pkg_model.h` — internal object/record layout shared by implementation modules.
- `pkg_support.h` — low-level shared support declarations.
- `pkg_target.h` — target-internal filesystem operations.
- `pkg_snapshot.h` — snapshot mutation operations.
- `pkg_backend.h` — backend entry points.

There is deliberately **no compatibility umbrella** that re-exports all private contracts. New implementation code must include only the private contracts it actually needs.

This directory is not a dumping ground for every declaration. When a private dependency is only between one or two tightly related modules, prefer a module-local private header. Split shared contracts when the dependency boundary is architecturally meaningful.

## 3. Layers

```
CLI / presentation
        |
Application orchestration
        |
Generic domain + scan model
        |
Evidence / diagnostics
        |
Backend interfaces
   |        |        |
 dpkg   filesystem   future adapters
```

The core library owns domain behavior. The CLI owns argument parsing, presentation, exit-code policy, and serialization selection. Serialization beyond the current basic CLI is a later contract.

## 4. Target model

`pkg_target` represents a root filesystem and its access policy. Local scanning is a target whose root is /; a rootfs target is an explicitly supplied directory. All path handling must use target-relative operations where practical. On Linux, the target root is opened as a real directory with `O_NOFOLLOW`, and target-relative `openat2()` resolution uses `RESOLVE_IN_ROOT | RESOLVE_NO_MAGICLINKS | RESOLVE_NO_XDEV` so lexical traversal, magic links, and mount/bind-mount crossings cannot silently change the target boundary.

The target owns no package truth. Backends discover facts about it.

## 5. Scan phases

```
validate options
  -> inspect target/platform
  -> detect package backend
  -> enumerate package metadata
  -> correlate selected package files
  -> collect evidence/diagnostics
  -> finalize immutable result
  -> serialize through the selected presentation layer
```

Each phase must have explicit resource accounting and failure semantics.

The dpkg parser has a fixed 65,536-byte record-content ceiling. The boundary is part of the parser contract: records up to and including 65,536 bytes before LF are accepted, a longer record returns a resource-limit result, and an EOF-terminated final record is valid. This prevents parser buffering from becoming an unbounded input-growth path.

## 6. Concurrency

v0.1 does not expose a parallel scanning API. Independent contexts may be used concurrently. Internal parallelism is allowed only after correctness and resource-limit accounting are established.

## 7. Repository layout

```
include/pkgintel/     public C headers
src/                  implementation
src/internal/         shared private implementation contracts
src/core/             core orchestration
src/target/           target abstraction
src/snapshot/         snapshot/result ownership
src/package/          package domain behavior
src/artifact/         artifact domain behavior
src/diagnostic/       diagnostic domain behavior
src/support/          shared low-level implementation helpers
src/backends/dpkg/    dpkg adapter
src/fs/               filesystem observation (planned)
src/elf/              ELF parser (planned)
src/capability/       capability inference (planned)
src/apt/              APT adapter (planned)
src/output/           human/JSON serializers (planned)
src/cli/              CLI only
tests/unit/            deterministic unit tests
tests/integration/     Ubuntu/Debian fixtures
tests/security/        hostile-input fixtures
tests/fuzz/             fuzz harnesses
benchmarks/            performance measurements
docs/                  contracts and design decisions
```

## 8. First vertical slice

The current first vertical slice is:

```
pkg_context
pkg_target
pkg_status
pkg_diagnostic
pkg_snapshot / pkg_scan_result
DPKG backend
selected package-file correlation
basic CLI
unit tests
installable CMake package
ABI export allowlist
```

ELF, capabilities, APT/cache analysis, JSON serialization, Rust FFI, vulnerability data, SBOM generation, and commercial functionality remain later slices.

## 9. Public API deferral rule

Future functionality must not enter the public ABI merely because an internal stub or prototype exists.

In particular, cache and capability result types/accessors were removed from the v0.1 public boundary because their semantics and implementation contracts are not yet complete. They will return through a separate design gate covering:

1. domain semantics;
2. ownership and lifetime;
3. provenance/evidence model;
4. resource accounting;
5. hostile-input behavior;
6. public API review;
7. ABI allowlist update;
8. integration and security tests;
9. documentation.

This prevents an unfinished design from becoming a compatibility commitment.


## 10. Dpkg installation-state semantics

The dpkg `Status:` field is a three-part contract: desired action, error flag, and actual package state. The engine does not confuse desired action with current installation state.

The v0.1 mapping is:

| dpkg actual state / condition | pkgintel state |
|---|---|
| `installed` with no `reinstreq` error flag | `PKG_INSTALLATION_INSTALLED` |
| `not-installed` or `config-files` | `PKG_INSTALLATION_REMOVED` |
| `half-installed`, `unpacked`, `half-configured`, `triggers-awaited`, `triggers-pending` | `PKG_INSTALLATION_PARTIAL` |
| `reinstreq` error flag | `PKG_INSTALLATION_PARTIAL` |
| malformed, unknown, or unsupported state token | `PKG_INSTALLATION_UNKNOWN` |

The first desired-action token (`install`, `hold`, `deinstall`, `purge`, or other valid dpkg values) does not determine `pkg_installation_state`. For example, `hold ok installed` remains `PKG_INSTALLATION_INSTALLED`.

Installation state and filesystem consistency are separate dimensions. A partial package is not automatically declared inconsistent merely because dpkg reports a transitional state; consistency is based on the package-file evidence that was actually observed. An unknown state produces a diagnostic rather than being silently treated as installed.

This mapping is deliberately smaller than the complete dpkg state machine. Any future public state expansion requires an API/ABI review and corresponding tests and documentation.

## 11. Performance implementation policy

The v0.1 performance policy is deliberately conservative. C17 plus normal compiler optimization is the baseline. Hand-written assembly is not justified by the current workload evidence and is therefore excluded from the production implementation.

The project will optimize in this order: algorithm/data model → I/O/syscall behavior → allocation/ownership → compiler optimization/LTO → profiled portable C → compiler intrinsics → hand-written assembly only after an explicit evidence gate.

Assembly, if ever introduced, must remain an internal implementation detail with a portable fallback where required. A microbenchmark win is insufficient; the change must demonstrate a meaningful end-to-end benefit and pass architecture-specific CI, sanitizer/security review, and maintenance-cost review. The complete decision framework is recorded in ADR-0040.
