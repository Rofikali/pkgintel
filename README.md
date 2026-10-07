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

## Engineering rule

> Build the smallest implementation that validates the largest architectural assumptions.

See `docs/ARCHITECTURE.md`, `docs/API.md`, and `docs/SECURITY.md` for the engineering contract.
