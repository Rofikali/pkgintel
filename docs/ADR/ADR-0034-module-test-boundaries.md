# ADR-0034 — Module, Test, and Boundary Architecture

- Status: Accepted
- Date: 2026-10-06

## Decision

pkgintel uses explicit architectural modules. Each significant subsystem has a coherent responsibility, implementation boundary, ownership/lifetime/error invariants, and tests appropriate to that boundary.

The public ABI remains under include/pkgintel/. Private implementation state lives under src/internal/ and is never part of the public ABI.

Initial modules: core, target, snapshot, package, artifact, diagnostic, support, and backends/dpkg.

Future resource governance and bounded metadata parsing will be independent modules rather than backend-local policy.

## Dependency rules

- Public API is the only interface exposed to external consumers.
- Core orchestrates; it does not own backend-specific parsing.
- Backends may consume target/domain/resource services but must not depend on CLI code.
- Public headers must not include private implementation headers.
- CMake targets follow meaningful dependency boundaries; one library per .c file is not required.

## Test architecture

Tests mirror architectural boundaries: unit tests verify module-local contracts; integration tests verify public API and cross-module behavior; security tests verify security properties; regression tests preserve fixed behavior.

A green integration test does not prove a module contract is correct.

## Migration rule

Structural migration must preserve observable behavior. CRLF semantics, aggregate resource governance, new limits, and other semantic changes are separate gates.

Every structural checkpoint must pass Debug and sanitizer builds before semantic work continues.
