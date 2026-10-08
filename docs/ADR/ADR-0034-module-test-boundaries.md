# ADR-0034 — Module, Test, and Boundary Architecture

- Status: Accepted
- Date: 2026-10-06

## Decision

pkgintel uses explicit architectural modules. Each significant subsystem has a coherent responsibility, implementation boundary, ownership/lifetime/error invariants, and tests appropriate to that boundary.

The repository uses three intentional source/interface zones:

- `include/pkgintel/` = **supported external contract**.
- `src/` = **implementation**.
- `src/internal/` = **implementation-only shared contract** between internal modules.

The public ABI remains under `include/pkgintel/`. Private implementation state and shared internal contracts live under `src/internal/` and are never part of the public ABI.

Initial modules: core, target, snapshot, package, artifact, diagnostic, support, and backends/dpkg.

Future resource governance and bounded metadata parsing will be independent modules rather than backend-local policy.

## Interface and dependency rules

- Public API is the only interface exposed to external consumers.
- Public headers live under `include/pkgintel/`.
- Public headers must not include private implementation headers.
- `src/` contains implementation and is never an external include dependency.
- `src/internal/` contains only private contracts required across internal module boundaries; it is not a general dumping ground for declarations.
- A tightly scoped private dependency should prefer a module-local private header when that convention is introduced explicitly.
- CMake install/export rules must expose only reviewed public headers.
- Core orchestrates; it does not own backend-specific parsing.
- Backends may consume target/domain/resource services but must not depend on CLI code.
- CMake targets follow meaningful dependency boundaries; one library per .c file is not required.

The directory rules are architectural rules, not cosmetic conventions. They communicate dependency direction and reduce accidental coupling to private implementation details.

## Test architecture

Tests mirror architectural boundaries: unit tests verify module-local contracts; integration tests verify public API and cross-module behavior; security tests verify security properties; regression tests preserve fixed behavior.

A green integration test does not prove a module contract is correct.

## Migration rule

Structural migration must preserve observable behavior. CRLF semantics, aggregate resource governance, new limits, and other semantic changes are separate gates.

Every structural checkpoint must pass Debug and sanitizer builds before semantic work continues.
