# High-Level Design (HLD)

## Purpose

The HLD describes **what major parts exist, why they exist, and how they depend on one another**. It deliberately avoids implementation-level details such as individual allocation calls.

## 1. v0.1 system boundary

```
                    pkgintel public C API
                             |
                    +--------+--------+
                    |                 |
                 Context            Scan
                                      |
                              Core orchestration
                                      |
             +------------------------+----------------+
             |                        |                |
           Target                  Backend          Snapshot
             |                        |                |
       confinement                 dpkg             packages
       filesystem                 future rpm        artifacts
       observation                future apk        diagnostics
```

The CLI is a consumer of the public API, not a privileged path into implementation internals.

## 2. Module responsibilities

| Module | Primary responsibility | Must not own |
|---|---|---|
| Context | explicit scan configuration/runtime state | CLI presentation |
| Target | controlled access to a target root | package semantics |
| Core scan | orchestration and policy | backend-specific parsing |
| Backend | interpret package-manager evidence | CLI formatting |
| Snapshot | aggregate result ownership/lifecycle | target security policy |
| Package | normalized package semantics | backend file-format parsing |
| Artifact | filesystem observation model | package-manager state |
| Diagnostic | structured evidence about failures/observations | presentation formatting |
| CLI | arguments, presentation, exit policy | core discovery logic |

## 3. Dependency direction

The intended direction is:

```
CLI
 |
 v
Public API / scan orchestration
 |
 +----> target abstraction
 |
 +----> backend abstraction ----> dpkg implementation
 |
 +----> normalized domain model
```

Implementation modules may depend downward on explicit private contracts. The public API must never require consumers to include private headers.

## 4. Security boundary

The target root is a security boundary.

The target layer owns the mechanisms that enforce:

- root descriptor ownership;
- no-follow root opening;
- target-relative resolution;
- `RESOLVE_IN_ROOT`;
- `RESOLVE_NO_MAGICLINKS`;
- `RESOLVE_NO_XDEV`.

Higher layers request observations through this boundary rather than reconstructing path-security rules independently.

## 5. Resource boundary

Resource governance is layered.

```
scan policy
    |
backend-specific limits
    |
aggregate snapshot ceilings
    |
allocation and arithmetic safety
```

A semantic budget and an allocation safety check solve different problems and must both remain present.

Current aggregate snapshot ceilings include artifact count, diagnostic count, and owned string storage. Unsupported future budgets must not be represented as enforced controls.

## 6. Data flow

```
target
  -> validate
  -> discover backend
  -> read bounded metadata
  -> normalize package
  -> correlate selected files
  -> create artifact observations
  -> create diagnostics for unavailable/invalid evidence
  -> finalize snapshot
  -> expose immutable public result
```

The flow preserves evidence and distinguishes operation status from individual observations.

## 7. HLD rules for future expansion

New subsystems such as ELF, APT, filesystem traversal, JSON output, or Rust FFI must first answer:

1. What domain responsibility is being introduced?
2. What existing boundary should it use?
3. What new trust boundary exists?
4. What resource dimensions does it consume?
5. What public API, if any, is required?
6. What ordering/determinism contract is required?
7. What tests prove the boundary?
8. What ADR is needed before the decision becomes difficult to reverse?

No feature enters the public ABI solely because an internal prototype exists.
