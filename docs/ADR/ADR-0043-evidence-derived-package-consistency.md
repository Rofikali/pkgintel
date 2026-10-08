# ADR-0043 — Evidence-derived package consistency

**Status:** Accepted

## Context

Package installation state from dpkg and filesystem artifact evidence are separate domains. A package can be reported as installed while its filesystem evidence is missing, broken, denied, or otherwise unverifiable. Conversely, a scanner that did not request or complete file correlation has no evidence from which to claim filesystem consistency.

The previous implementation inferred consistency from only missing-file and invalid-path counters. That allowed packages containing BROKEN_LINK or PERMISSION_DENIED artifacts to be reported as CONSISTENT, and allowed a package with no requested correlation to be indistinguishable from a successfully correlated package with no bad evidence.

## Decision

Each package has an internal correlation state:

- NOT_REQUESTED — file correlation was not requested.
- COMPLETE — the package file list was successfully consumed to completion and every consumed non-empty record has an artifact observation.
- INCOMPLETE — correlation was requested but the package file list could not be fully established, including missing metadata, invalid package identity, read failure, or resource exhaustion.

The public consistency result is derived from complete artifact evidence only.

Rules:

1. NOT_REQUESTED or INCOMPLETE -> PKG_CONSISTENCY_UNKNOWN.
2. COMPLETE with all artifacts PRESENT -> PKG_CONSISTENCY_CONSISTENT.
3. COMPLETE with only MISSING failures -> PKG_CONSISTENCY_MISSING_ARTIFACT.
4. COMPLETE with only BROKEN_LINK failures -> PKG_CONSISTENCY_BROKEN_LINK.
5. COMPLETE with only PERMISSION_DENIED failures -> PKG_CONSISTENCY_PERMISSION_DENIED.
6. COMPLETE with only UNVERIFIABLE failures -> PKG_CONSISTENCY_UNVERIFIABLE.
7. COMPLETE with multiple distinct failure classes -> PKG_CONSISTENCY_INCONSISTENT.
8. PKG_CONSISTENCY_UNEXPECTED_ARTIFACT remains reserved until filesystem enumeration can establish that evidence.

ENOENT is the only observation error that establishes absence. Other errno values indicate that the requested observation could not be established and must not increment missing-file accounting.

A successfully processed empty package file list is COMPLETE and therefore may produce CONSISTENT: zero expected records is different from correlation not being requested or not completing.

## Consequences

- Consistency cannot be falsely promoted from absence of evidence.
- Artifact records remain the authoritative evidence for package consistency.
- Existing package counters remain useful reporting fields but are not the sole semantic source for consistency.
- Mixed failure classes intentionally collapse to INCONSISTENT at the package summary level while retaining individual artifact evidence.
- Unexpected filesystem artifacts are not claimed by v0.1 because current correlation is expected-file driven.
- Future backends must preserve the distinction between requested, complete, and incomplete evidence.

## Verification requirements

Tests must cover:

- no correlation -> UNKNOWN;
- complete all-present correlation -> CONSISTENT;
- missing -> MISSING_ARTIFACT;
- broken link -> BROKEN_LINK;
- permission denial where the runtime permits it -> PERMISSION_DENIED;
- unverifiable evidence -> UNVERIFIABLE;
- mixed failure classes -> INCONSISTENT;
- resource-truncated correlation -> UNKNOWN for the incomplete package;
- unexpected errno is not counted as missing.
