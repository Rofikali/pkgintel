# ADR-0012: CI Is a Release-Engineering Gate

- Status: Accepted
- Date: 2026-10-05

## Context

`pkgintel` is low-level C software operating on untrusted package metadata and filesystem/ELF data. A developer-machine build is insufficient evidence of correctness. Compiler, linker, sanitizer, and test behavior must be reproducible in controlled Linux environments.

## Decision

GitHub Actions on Ubuntu 24.04 is a mandatory verification gate for repository changes affecting the engine.

The minimum matrix is:

- GCC Debug
- GCC Release
- Clang Debug
- Clang Release
- Sanitized build with ASan/UBSan

A feature is not considered complete until the relevant CI jobs pass. Failures are investigated before additional architectural layers are added.

CI results are evidence, not proof of absence of defects. Security-sensitive parsers additionally require adversarial tests and fuzzing.

## Consequences

Positive:

- compiler differences are exposed early;
- linker/ABI problems are detected before release;
- sanitizer regressions become visible;
- the repository has reproducible evidence for engineering decisions.

Negative:

- changes take longer to declare complete;
- CI configuration itself becomes maintained engineering infrastructure.

## Rule

Do not stack new subsystem work on top of a red foundational gate unless the change is specifically a repair of that gate.
