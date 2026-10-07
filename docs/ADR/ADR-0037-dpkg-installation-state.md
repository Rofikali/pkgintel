# ADR-0037 — Dpkg Installation-State Classification

- Status: Accepted
- Date: 2026-10-07

## Context

The initial dpkg adapter only emitted records whose complete `Status:` field was exactly `install ok installed`. That created two correctness problems:

1. valid installed packages with a different desired action, such as `hold ok installed`, could be excluded;
2. transitional, removed, or broken packages disappeared from the result model even though their state is meaningful evidence.

The dpkg Status field is a three-part value: desired action, error flag, and actual package state. The desired action is not equivalent to the current installation state.

## Decision

v0.1 persists an explicit `pkg_installation_state` on each package record.

The mapping is:

- actual state `installed` with no `reinstreq` flag → `PKG_INSTALLATION_INSTALLED`;
- actual state `not-installed` or `config-files` → `PKG_INSTALLATION_REMOVED`;
- actual states `half-installed`, `unpacked`, `half-configured`, `triggers-awaited`, or `triggers-pending` → `PKG_INSTALLATION_PARTIAL`;
- `reinstreq` error flag → `PKG_INSTALLATION_PARTIAL`;
- malformed, unknown, or unsupported state token → `PKG_INSTALLATION_UNKNOWN`, with a diagnostic.

The desired-action token (`install`, `hold`, `deinstall`, `purge`, etc.) does not determine the installation state.

Filesystem consistency remains a separate dimension. `pkg_package_get_consistency()` describes observed package-file evidence and does not infer installation completeness from the dpkg state alone.

## Consequences

### Positive

- package results no longer silently omit meaningful dpkg states;
- hold/install/remove intent cannot be confused with actual state;
- broken/reinstallation-required packages are not falsely reported as healthy;
- the public model can distinguish package lifecycle state from filesystem evidence.

### Negative

- the scanner may now correlate file lists for packages that are not fully installed; missing file lists become explicit diagnostics rather than invisible omission;
- the public enum remains intentionally coarser than the complete dpkg state vocabulary;
- future dpkg states require an explicit semantic decision rather than accidental fallback behavior.

## Rejected alternatives

### Keep only installed packages

Rejected because it hides partial and removed state and makes the public `PARTIAL`/`REMOVED` enum values effectively dead API.

### Use the first Status token as installation state

Rejected because desired action is intent, not actual package state. A held installed package is still installed.

### Treat every unknown state as installed

Rejected because that is fail-open behavior and can create false security conclusions.

## Test gate

The unit suite must cover at minimum:

- `hold ok installed` → INSTALLED;
- `install ok unpacked` → PARTIAL;
- `deinstall ok config-files` → REMOVED;
- `install reinstreq installed` → PARTIAL;
- deterministic package ordering after state classification;
- unknown/malformed state → UNKNOWN plus diagnostic.

Any expansion of the public state enum requires synchronized source, tests, ABI review, security documentation, architecture documentation, and release notes.
