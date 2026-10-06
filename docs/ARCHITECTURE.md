# pkgintel Architecture

## 1. Architectural corrections to v0.1 proposal

The original proposal is strong, but several concepts need tightening before implementation:

1. **C17 is the implementation contract.** Do not introduce C++ or C++-style ownership abstractions. The repository and build must enforce C17.
2. **Package, installation, and artifact remain distinct.** A package is metadata; an installation is package state within a target; an artifact is an observed filesystem object.
3. **The target is the security boundary.** Every filesystem operation must resolve relative to the target. No subsystem may silently escape the target root.
4. **dpkg and APT are adapters.** Generic domain structures must not contain dpkg/apt-specific types.
5. **Discovery is evidence-producing, not truth-producing.** Results preserve provenance and confidence; permission failures and unsupported metadata are observations, not silent omissions.
6. **The first vertical slice must be narrow.** Start with context → target → dpkg package enumeration → selected package-file correlation → diagnostics → CLI/JSON. ELF, capabilities, and APT follow only after that boundary is proven.
7. **Do not promise full filesystem correctness in v0.1.** Package-owned-file verification is bounded and policy-driven; a full / crawl is not the default.
8. **Do not expose size ambiguously.** Use named semantics such as `logical_size`, `allocated_size`, and `package_installed_size`.
9. **JSON is an output contract, not the domain model.** Version the JSON schema independently from the C ABI.
10. **Stable ABI comes after API review and ABI testing.** v0.1 can expose an explicitly versioned C API, but the project must not claim ABI stability until ABI checks and compatibility policy exist.

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

For example, `src/internal/pkg_internal.h` currently contains private record structures and internal functions needed across core, target, snapshot, package, diagnostic, and backend implementation.

Private headers are intentionally not installed or advertised as part of the public SDK.

This directory is not a dumping ground for every declaration. When a private dependency is only between one or two tightly related modules, prefer a module-local private header. Split the broad internal contract only when the dependency boundary becomes architecturally meaningful.

### 2.4 Directory rules are architectural rules

The repository should enforce these rules in CI:

- Public headers live under `include/pkgintel/`.
- Shared private headers live under `src/internal/`.
- No other `.h` files are placed directly under `src/` or arbitrary source directories unless a future ADR explicitly defines a module-local private-header convention.
- External-facing documentation refers to `include/pkgintel/` as the supported interface, not to internal headers.
- CMake installation/export rules must install only the reviewed public headers.

The purpose is not to enforce a folder style for its own sake. The directory structure communicates dependency direction and prevents accidental coupling to private implementation details.

## 3. Layers

```text
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
 dpkg     APT   filesystem/ELF
```

The core library owns domain behavior. The CLI owns argument parsing, presentation, exit-code policy, and serialization selection.

## 4. Target model

`pkg_target` represents a root filesystem and its access policy. Local scanning is a target whose root is /; a rootfs target is an explicitly supplied directory. All path handling must use target-relative operations where practical.

The target owns no package truth. Backends discover facts about it.

## 5. Scan phases

```text
validate options
  -> inspect target/platform
  -> detect package backend
  -> enumerate package metadata
  -> correlate selected package files
  -> collect evidence/diagnostics
  -> optional artifact/ELF/capability phases
  -> finalize immutable result
  -> serialize
```

Each phase must have explicit resource accounting and failure semantics.

## 6. Concurrency

v0.1 does not expose a parallel scanning API. Independent contexts may be used concurrently. Internal parallelism is allowed only after correctness and resource-limit accounting are established.

## 7. Repository layout

```text
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

The first implementation should be:

```text
pkg_context
pkg_target
pkg_status
pkg_diagnostic
pkg_scan_result
DPKG backend
selected package-file correlation
basic CLI
JSON v0.1 schema
unit + integration tests
```

Do not add Rust, SQLite, remote scanning, vulnerability data, SBOM generation, or commercial functionality to this slice.
