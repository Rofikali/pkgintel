# ADR-0035 — Defer Cache and Capability APIs from v0.1

- Status: Accepted
- Date: 2026-10-07

## Context

The initial architecture included cache and capability concepts in the public header set and snapshot API. During the v0.1 ABI audit, these accessors were found to be exported even though their implementation and semantic contracts were not part of the proven first vertical slice.

Publishing an unfinished type or accessor creates a compatibility commitment before the project has established:

- domain semantics;
- ownership and lifetime rules;
- evidence/provenance semantics;
- resource accounting;
- hostile-input behavior;
- integration tests;
- security tests;
- ABI expectations.

An ABI audit must therefore treat accidental exports as a defect, not as a reason to expand the allowlist.

## Decision

Cache and capability headers and snapshot accessors are **deferred from the v0.1 public API**.

The v0.1 public ABI contains only the reviewed symbols in `tests/abi/public_symbols.txt`.

The following concepts remain architectural roadmap items:

- cache analysis;
- capability inference;
- related cache/capability result models;
- corresponding public accessors.

They may return in a later feature slice only after a dedicated design and security review.

## Required re-entry gate

Before either feature enters the public ABI, it must pass:

1. **Semantic gate** — define exactly what the result means and what evidence supports it.
2. **Ownership gate** — define allocation, lifetime, borrowing, and destruction semantics.
3. **Resource gate** — define per-operation and aggregate limits and failure behavior.
4. **Security gate** — define hostile-input, confinement, parser, and privilege properties.
5. **Test gate** — add unit, integration, regression, and security coverage.
6. **ABI gate** — review declarations and update the explicit exported-symbol allowlist.
7. **Documentation gate** — update architecture, API, security, and release-scope documentation together.

## Consequence

The public v0.1 API is smaller, but it is more honest and easier to stabilize.

This intentionally favors a narrow verified contract over speculative API surface. Internal implementation work may proceed behind private boundaries without forcing external consumers to depend on incomplete designs.

## Rejected alternative

**Add the unfinished symbols to the ABI allowlist.**

Rejected because it converts an implementation accident into a supported API commitment and hides the missing semantic/security work behind a green ABI test.
