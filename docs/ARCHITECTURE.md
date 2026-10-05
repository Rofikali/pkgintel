# pkgintel Architecture

## 1. Architectural corrections to v0.1 proposal

The original proposal is strong, but several concepts need tightening before implementation:

1. **C17 is the implementation contract.** Do not introduce C++ or C++-style ownership abstractions. The repository and build must enforce C17.
2. **Package, installation, and artifact remain distinct.** A package is metadata; an installation is package state within a target; an artifact is an observed filesystem object.
3. **The target is the security boundary.** Every filesystem operation must resolve relative to the target. No subsystem may silently escape the target root.
4. **dpkg and APT are adapters.** Generic domain structures must not contain dpkg/apt-specific types.
5. **Discovery is evidence-producing, not truth-producing.** Results preserve provenance and confidence; permission failures and unsupported metadata are observations, not silent omissions.
6. **The first vertical slice must be narrow.** Start with context → target → dpkg package enumeration → selected package-file correlation → diagnostics → CLI/JSON. ELF, capabilities, and APT follow only after that boundary is proven.
7. **Do not promise full filesystem correctness in v0.1.** Package-owned-file verification is bounded and policy-driven; a full `/` crawl is not the default.
8. **Do not expose `size` ambiguously.** Use named semantics such as `logical_size`, `allocated_size`, and `package_installed_size`.
9. **JSON is an output contract, not the domain model.** Version the JSON schema independently from the C ABI.
10. **Stable ABI comes after API review and ABI testing.** v0.1 can expose an explicitly versioned C API, but the project must not claim ABI stability until ABI checks and compatibility policy exist.

## 2. Layers

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

## 3. Target model

`pkg_target` represents a root filesystem and its access policy. Local scanning is a target whose root is `/`; a rootfs target is an explicitly supplied directory. All path handling must use target-relative operations where practical.

The target owns no package truth. Backends discover facts about it.

## 4. Scan phases

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

## 5. Concurrency

v0.1 does not expose a parallel scanning API. Independent contexts may be used concurrently. Internal parallelism is allowed only after correctness and resource-limit accounting are established.

## 6. Repository layout

```text
include/pkgintel/     public C headers
src/                  implementation
src/core/             context, target, status, result
src/backends/dpkg/    dpkg adapter
src/fs/               filesystem observation
src/elf/              ELF parser
src/capability/       capability inference
src/apt/              APT adapter
src/output/            human/JSON serializers
src/cli/              CLI only
tests/unit/            deterministic unit tests
tests/integration/     Ubuntu/Debian fixtures
tests/security/        hostile-input fixtures
tests/fuzz/             fuzz harnesses
benchmarks/            performance measurements
docs/                  contracts and design decisions
```

## 7. First vertical slice

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
