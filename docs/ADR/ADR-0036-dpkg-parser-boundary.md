# ADR-0036 — Dpkg Record Boundary and EOF Semantics

- Status: Accepted
- Date: 2026-10-07

## Context

The dpkg status database and package file lists are untrusted input. A parser that grows a line buffer without a hard bound can be turned into a memory-exhaustion path.

The v0.1 implementation uses one bounded record reader for both dpkg status records and package-file records. The boundary therefore needs to be an explicit engineering contract rather than an implementation detail.

## Decision

v0.1 defines a hard maximum of **65,536 bytes of record content before the LF delimiter**.

- 65,535 bytes: accepted.
- 65,536 bytes: accepted.
- 65,537 bytes or more before LF: rejected as `PKG_ERR_RESOURCE_LIMIT`.
- EOF after non-empty record content is a valid final record even without LF.
- An empty EOF is not a record.
- CRLF is accepted. The CR is part of the bounded input content and is removed during normalization.
- The bound applies before semantic field interpretation, so malformed records are bounded just like valid records.

A resource-limit result may contain the package records already committed before the limit was encountered, according to the scan result contract. The current parser does not allocate a buffer proportional to an attacker-controlled record beyond this fixed bound.

## Rationale

The exact boundary is tested at N-1/N/N+1 rather than only with a large oversized fixture. This prevents accidental off-by-one changes from silently weakening or tightening the resource contract.

EOF-terminated records are accepted because line-oriented metadata may legally end without a final line-feed; rejecting such input would turn a representation detail into a false parse failure.

## Security consequences

The parser has a deterministic upper bound on per-record materialization. This reduces memory-exhaustion risk from hostile package metadata while preserving a useful diagnostic distinction between malformed data and resource exhaustion.

The boundary does not replace aggregate scan budgets. Total bytes, allocations, descriptors, and wall-clock work remain separate resource-accounting concerns.

## Test requirement

The v0.1 unit suite must retain explicit tests for:

1. 65,535-byte record;
2. 65,536-byte record;
3. 65,537-byte record;
4. non-LF-terminated final record.

Any future change to the boundary requires updates to this ADR, security documentation, tests, and release notes/compatibility review together.
