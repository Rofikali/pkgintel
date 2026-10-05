# pkgintel

`pkgintel` is a C17/Linux software-environment intelligence engine.

The project discovers, normalizes, correlates, and explains package metadata, filesystem artifacts, ELF metadata, caches, and software capabilities without modifying the target.

## v0.1 focus

- Linux / Debian-compatible targets
- dpkg package database
- package-owned file correlation
- controlled filesystem observation
- basic ELF inspection
- development-toolchain capability detection
- APT cache metadata
- human-readable and versioned JSON output
- read-only, resource-bounded scanning
- opaque C API designed for a future stable Rust FFI boundary

## Engineering rule

> Build the smallest implementation that validates the largest architectural assumptions.

See `docs/ARCHITECTURE.md`, `docs/API.md`, `docs/SECURITY.md`, and `docs/BUILDING.md` for the engineering contract.
