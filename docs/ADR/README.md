# Architecture Decision Records

These ADRs record architectural decisions that are expensive to reverse and important for security, compatibility, and long-term maintenance.

## ADR-0001 — C17 as the implementation language

**Status:** Accepted

pkgintel uses ISO C17 as its core implementation language. C provides direct control over memory, file descriptors, Linux syscalls, ABI boundaries, and low-level ELF/filesystem inspection. Consequence: memory safety, ownership, integer overflow, parser robustness, and concurrency are explicit engineering concerns.

## ADR-0002 — Ubuntu/Debian as the first platform family

**Status:** Accepted

The first supported platform family is Linux systems using Debian/Ubuntu packaging conventions. Portability is designed into interfaces, but non-Debian package backends are deferred.

## ADR-0003 — dpkg as the first package backend

**Status:** Accepted

dpkg metadata is the first package-manager backend. The implementation reads authoritative local metadata directly rather than depending on package-manager command execution. Backend-specific code is isolated for future RPM/DNF/Pacman/APK support.

## ADR-0004 — Opaque public C objects

**Status:** Accepted

Public objects such as context, target, snapshot, package, artifact, and diagnostic are opaque. This prevents ABI consumers from depending on struct layout and allows internal evolution.

## ADR-0005 — No database in v0.1

**Status:** Accepted

pkgintel does not require PostgreSQL, SQLite, Redis, or another persistent database for v0.1. A scan is a point-in-time inventory and the snapshot is the primary result.

## ADR-0006 — Read-only discovery by default

**Status:** Accepted

pkgintel observes systems but does not install, remove, upgrade, modify packages, execute discovered programs, or mutate the target filesystem.

## ADR-0007 — Target abstraction

**Status:** Accepted

Inspection occurs through a target abstraction. Initial targets are local and arbitrary root filesystems. A target path is not itself a security boundary; the implementation relies on an owned root descriptor and constrained resolution.

## ADR-0008 — CLI as a library consumer

**Status:** Accepted

The CLI consumes the public library API. Core discovery logic must not become trapped inside command-line code.

## ADR-0009 — Rust through the C ABI

**Status:** Accepted

Future Rust integration consumes the public C ABI rather than internal structs. ABI ownership, lifetime, error semantics, visibility, and FFI tests are release-gated.

## ADR-0010 — Evidence-based discovery

**Status:** Accepted

Observed evidence is distinct from inferred conclusions. dpkg, filesystem, ELF, APT, and heuristic observations must remain explainable and attributable.

## ADR-0011 — Package, installation, and artifact are separate domains

**Status:** Accepted

Package identity, local installation state, and filesystem artifacts are separate concepts. The long-term model is Package -> Installation -> Artifact observations.

## ADR-0012 — Immutable scan snapshots

**Status:** Accepted

A completed scan produces an immutable snapshot. Records are owned by the snapshot and exposed through borrowed accessors. Concurrent read-only access is intended to be safe.

## ADR-0013 — Filesystem paths are byte sequences

**Status:** Accepted

Filesystem paths use explicit-length byte sequences and are not assumed to be UTF-8. Human-readable rendering is a presentation concern.

## ADR-0014 — Descriptor-relative filesystem security

**Status:** Accepted

Target filesystem access uses a root descriptor and constrained relative resolution. Path strings are never treated as the security boundary.

## ADR-0015 — Do not follow symlinks during artifact identity observation

**Status:** Accepted

Artifact classification observes symlinks as symlinks by default. Explicit resolution, if ever needed, is a separate controlled operation.

## ADR-0016 — Fail closed on target escape

**Status:** Accepted

Attempts to escape a target root, traverse prohibited magic links, or violate resolution policy are rejected and become structured diagnostics.

## ADR-0017 — Bounded resource consumption

**Status:** Accepted

Scanning is bounded by resource policies because metadata, file lists, directories, malformed files, sparse files, and ELF objects can cause memory, CPU, descriptor, or I/O exhaustion.

## ADR-0018 — No automatic execution of discovered software

**Status:** Accepted

pkgintel does not execute discovered compilers, binaries, package scripts, shell commands, or helper programs merely to identify capabilities.

## ADR-0019 — Versioned public API separate from product version

**Status:** Accepted

Public API version is tracked separately from product release version. ABI stability is not claimed until documented compatibility gates pass.

## ADR-0020 — JSON/output schema separate from C object model

**Status:** Accepted

Serialized output is versioned independently from the C object representation. CLI serialization maps snapshots to an explicit output schema.

## ADR-0021 — Diagnostics use stable machine-readable codes

**Status:** Accepted

Diagnostics have stable codes separate from human-readable messages. Automation must never parse English text.

## ADR-0022 — Partial results are first-class

**Status:** Accepted

A scan may return a useful snapshot even when some observations fail. Operation status, snapshot completeness/health, and individual diagnostics are separate concepts.

## ADR-0023 — Package backends are pluggable

**Status:** Accepted

Package-manager discovery is behind backend-specific interfaces so future ecosystems can provide evidence through the same normalized model.

## ADR-0024 — Library core is independent of CLI presentation

**Status:** Accepted

Formatting, colors, progress display, and CLI exit policy belong to the CLI layer. The core returns structured data and diagnostics.

## ADR-0025 — Security boundaries are release gates

**Status:** Accepted

Path traversal, symlink behavior, special files, permissions, malformed metadata, resource exhaustion, sanitizers, fuzzing, and regression tests are release gates.

## ADR-0026 — No shell-out implementation for core discovery

**Status:** Accepted

Core discovery does not depend on dpkg-query, dpkg, apt, find, file, ldd, or shell pipelines. Direct files and syscalls reduce attack surface and environment dependence.

## ADR-0027 — Deterministic and reproducible scan semantics

**Status:** Accepted

For the same target state and configuration, normalized results should have deterministic ordering and semantics. Enumeration or hash-map nondeterminism must not leak into public output.

## ADR-0028 — Error taxonomy is part of the API contract

**Status:** Accepted

Errors distinguish invalid arguments, permission, not-found, I/O, unsupported, corruption, cancellation, limits, and internal failures. Numeric statuses are not casually renumbered.

## ADR-0029 — No hidden global mutable state

**Status:** Accepted

The library has no hidden global mutable scanner state. State belongs to explicit context, target, scan, and snapshot objects.

## ADR-0030 — Architecture decisions are reviewed before irreversible implementation

**Status:** Accepted

Decisions affecting public API, security boundaries, persistence, backend abstractions, concurrency, or ABI are recorded before becoming deeply embedded.

## ADR-0031 — Resource budget units and scope

**Status:** Accepted

Resource limits are security controls, so each limit has an explicit unit and scope.
`max_packages` is a per-scan limit on installed package records. `max_package_files`
is a per-package limit on non-empty package-file records. Every consumed non-empty
record consumes one unit regardless of whether the path is valid, missing, or
otherwise unverifiable; malformed records may therefore produce an
`UNKNOWN`/`UNVERIFIABLE` artifact observation. Empty records consume no budget.

A configured maximum is a maximum number of records the scanner may consume, not a
requirement to report `PKG_ERR_RESOURCE_LIMIT` merely because the count equals the
maximum. If the input ends after exactly N non-empty records, the scan may complete
normally. `PKG_ERR_RESOURCE_LIMIT` is returned when the scanner encounters a further
non-empty record that it would have to consume. The partial snapshot accumulated
before that refused record is preserved.

These per-package limits are not aggregate scan-wide DoS protection. Future global
byte, time, descriptor, artifact, and other budgets must be defined independently.

# Open architectural decisions before v0.1 freeze

These are deliberately open decisions, not accidental implementation choices:

1. Snapshot completeness/health enum and semantics.
2. Exact package/installation/artifact ownership graph.
3. Unknown-vs-zero measurement representation for sizes.
4. Artifact allocated-size portability semantics.
5. Diagnostic code registry and namespace.
6. Cache vs cache-entry domain split.
7. ELF parsing trust boundary and maximum parse bytes.
8. Concurrency guarantees for context and target objects.
9. Cancellation mechanism and semantics.
10. ABI symbol export/versioning policy.
11. Public API compatibility policy through 0.x and 1.0.
12. Stable ordering rules for packages/artifacts/diagnostics.
13. Maximum path/string lengths and byte limits.
14. openat2 minimum-kernel and fallback policy.
15. Mount/bind-mount boundary semantics.
16. Special-file observation policy.
17. Package identity normalization rules.
18. Multi-architecture package semantics.
19. APT metadata authority and trust model.
20. JSON schema compatibility policy.