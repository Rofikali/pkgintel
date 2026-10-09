# pkgintel Product Charter

## Status

This document is the product and business north star for the post-P0 engineering phase. It complements `docs/HLD.md`, `docs/LLD.md`, `docs/SECURITY.md`, `docs/API_CONTRACT.md`, and `docs/ENGINEERING_OPERATING_MODEL.md`.

P0 established the native engineering foundation. This charter establishes what that foundation is intended to become and how future scope is selected.

## 1. Product definition

**pkgintel is a security-conscious Linux software-environment intelligence engine.**

It discovers package and filesystem facts, normalizes them into a structured domain model, correlates package metadata with filesystem artifacts, and produces explainable observations without modifying or executing the target system.

The core is intended to be reusable as a native library. A CLI is one consumer; future security, infrastructure, fleet, audit, forensic, or higher-level applications may consume the same inspection engine.

The central product question is:

> What software environment exists on this Linux target, what evidence supports that observation, how do package and filesystem observations relate, and where is the evidence incomplete or inconsistent?

pkgintel is therefore an **evidence-producing inspection engine**, not merely a package database parser.

## 2. Problem

Linux software state is represented by multiple sources that do not necessarily agree:

- package-manager metadata;
- installed/partial/removed state;
- package-owned file lists;
- actual filesystem observations;
- future binary/ELF metadata;
- future package/cache metadata;
- future system capability information.

A consumer reading only one source can answer only part of the question. pkgintel provides a controlled boundary between those raw observations and applications that need a defensible software-environment view.

The engine must preserve the distinction between what metadata claims, what the filesystem observation found, what could not be observed, what was rejected as invalid, what was inferred/correlated, and what remains unknown.

## 3. Product promise

For supported targets, pkgintel aims to provide:

1. **Read-only inspection** — scanning does not modify target content.
2. **No execution of discovered software** — discovered programs are data, not commands.
3. **Target confinement** — rootfs inspection cannot silently escape its target boundary.
4. **Bounded work** — attacker-controlled input cannot create uncontrolled work within implemented limits.
5. **Structured observations** — results are packages, installations, artifacts, and diagnostics rather than presentation-only text.
6. **Evidence provenance** — consumers can distinguish source types and confidence.
7. **Reusable native interface** — the inspection engine is separated from CLI presentation.
8. **Fail-closed security behavior** — unsupported or unverifiable security-sensitive operations are not silently treated as successful.

These are product-level promises only where the corresponding implementation and evidence gates support them.

## 4. What pkgintel is not

pkgintel is not currently:

- a vulnerability scanner;
- an SBOM generator;
- a package installer/updater;
- a configuration-management system;
- an antivirus engine;
- a full filesystem crawler;
- a general-purpose Linux telemetry agent;
- a cloud database of host inventories;
- an execution/sandbox engine;
- a package-manager replacement.

Those may become integration opportunities or future products, but they are not implicit v0.1 scope.

## 5. Target users and consumers

Potential consumers are product hypotheses, not committed markets:

- **Security software:** reliable package/artifact facts as input to security analysis.
- **Infrastructure/fleet software:** deterministic software-environment observations.
- **Audit/compliance tooling:** structured, reproducible evidence instead of shell-output scraping.
- **Incident-response/forensic tooling:** controlled observation with explicit distinction between observed, missing, inaccessible, and inconsistent evidence.
- **Native applications:** a reusable library instead of package-manager-specific parsing.

The first commercial/customer segment is not selected by this charter. Market validation must precede a commercial commitment.

## 6. Domain model

The core model is:

```
Package
   |
   | state on a target
   v
Installation
   |
   | correlates with observations
   v
Artifact
   |
   +----> Diagnostic / Evidence
```

These concepts remain separate.

- **Package** — package identity and metadata.
- **Installation** — package state within a target.
- **Artifact** — observed filesystem object or package-owned-file relationship.
- **Diagnostic** — structured information about invalid, unavailable, partial, inconsistent, or otherwise relevant evidence.

A package identity is not an installation, and an installation is not proof that every expected artifact exists.

## 7. Current product boundary: v0.1

The current v0.1 vertical slice is:

- Debian-compatible Linux;
- dpkg package database enumeration;
- explicit installed/partial/removed state;
- selected package-owned-file correlation;
- controlled filesystem observation;
- package/artifact/diagnostic result model;
- read-only target confinement;
- resource-bounded package metadata parsing;
- opaque C API;
- installable CMake package;
- basic CLI with versioned JSON export via `pkgintel scan --json`.

Explicitly outside implemented v0.1:

- ELF inspection;
- capability inference;
- APT metadata/cache analysis;
- full filesystem crawling;
- vulnerability/SBOM features;
- commercial/cloud inventory features;
- Rust FFI.

A future feature does not enter the public ABI merely because an internal prototype exists.

## 8. Long-term capability map

The intended long-term direction is a layered intelligence engine:

```
                    pkgintel
                       |
          Linux software-environment
                intelligence
                       |
        +--------------+--------------+
        |              |              |
     Discovery     Correlation     Analysis
        |              |              |
      dpkg           package <->      ELF
      APT            artifact        capabilities
      other          consistency     metadata
      sources        provenance      diagnostics
```

Potential future capabilities include additional package-manager adapters, ELF/binary inspection, broader filesystem inventory, package/cache metadata, capability/security metadata, versioned machine-readable export, Rust FFI, and higher-level integrations.

These are candidates. Versioned CLI JSON export is now implemented as the P1 integration boundary; see `docs/P1_JSON_EXPORT_HANDOFF.md` for exact evidence. Remaining candidates must earn entry through product value, domain-model review, security review, resource accounting, API/ABI impact analysis, tests, and operational evidence. The next gate is validating a real downstream consumer before selecting another production capability.

## 9. Product differentiation

The intended differentiation is not simply "written in C" or "faster package parsing."

It is the combination of:

- evidence-aware observations;
- security-first target inspection;
- reusable native core;
- deterministic contracts;
- low operational footprint.

Performance is important, but performance is a supporting property rather than the sole product identity.

## 10. Security product principles

Security is part of the product:

```
observe, do not modify
observe, do not execute
confine, do not escape
bound, do not exhaust
validate, do not trust
explain, do not overclaim
```

Security claims must be tied to evidence classes. A unit test does not prove a Linux kernel mount boundary; sanitizer evidence does not prove business viability; a benchmark does not prove security; a simulated filesystem fixture does not replace real VFS verification.

The product must never convert unavailable evidence into a positive security claim.

## 11. CA / Finance decision framework

Material technical decisions must consider:

- engineering effort;
- infrastructure cost;
- dependency/vendor cost;
- security-review cost;
- maintenance and support burden;
- incident exposure;
- opportunity cost;
- expected customer value;
- reversibility;
- cost of delay.

The preferred architecture is not the one with the most technology. It is the one that creates sufficient product value with acceptable technical, security, operational, and financial risk.

Before introducing a recurring infrastructure dependency, the project should be able to explain its unit cost and why it is necessary.

## 12. MBA / Management / Product decision framework

For every material P1+ capability, answer:

1. Who needs it and what problem is solved?
2. What value becomes possible or materially better?
3. Why now?
4. What technical/security/business risk is introduced?
5. What engineering and operating cost is introduced?
6. What dependencies must exist first?
7. Is the decision reversible?
8. What observable evidence means success?
9. What valuable work must wait?
10. What evidence would cause us to stop or redesign it?

A feature is not automatically valuable because it is technically interesting.

## 13. Success measures

P1 success must be measured at three levels.

### Product correctness

- supported target facts are represented accurately;
- package/artifact relationships are explainable;
- incomplete or inaccessible observations remain distinguishable;
- unsupported capabilities fail explicitly.

### Engineering quality

- no material security invariant is weakened;
- resource behavior is bounded and tested;
- API/ABI changes are deliberate;
- deterministic behavior is defined where required;
- relevant compiler, sanitizer, fuzz, runtime, and release gates pass.

### Product/economic value

Eventually measure:

- time saved versus ad-hoc package-manager/shell inspection;
- useful downstream integrations;
- inspection throughput/resource cost;
- deployment/operational cost;
- customer willingness to adopt/pay;
- support burden;
- value of additional intelligence versus implementation cost.

Exact commercial KPIs remain unselected until customer discovery provides evidence.

## 14. P1 entry criteria

P1 implementation must not begin by selecting a random feature.

The selected capability must have:

1. a defined user/problem statement;
2. explicit in-scope/out-of-scope behavior;
3. domain-model impact identified;
4. HLD boundary identified;
5. LLD/security implications identified;
6. resource dimensions identified;
7. API/ABI impact identified;
8. deterministic/error semantics identified;
9. acceptance tests identified;
10. release/evidence gates identified;
11. engineering and operational cost considered;
12. an ADR when the decision is materially difficult to reverse.

## 15. P1 decision candidates

These are candidates, not an implementation order:

| Candidate | Potential value | Main risk/complexity |
|---|---|---|
| ELF inspection | richer binary/software intelligence | parser/security/resource surface |
| Broader filesystem correlation | stronger consistency analysis | traversal, scale, VFS complexity |
| APT metadata/cache | richer Debian coverage | large metadata/security surface |
| Versioned JSON export | easier integrations | output compatibility contract |
| Rust FFI | safer higher-level integration | ABI/lifetime/build boundary |
| Additional package adapters | broader Linux coverage | domain normalization/platform variance |

Selection must be evidence-driven.

## 16. Product architecture principle

Grow vertically before growing indiscriminately:

```
validated product problem
        ↓
smallest valuable capability
        ↓
domain model
        ↓
security boundary
        ↓
resource model
        ↓
API boundary
        ↓
implementation
        ↓
evidence
        ↓
release
```

Avoid feature -> framework -> dependencies -> large API -> unclear product.

## 17. Decision rule

> **Build the smallest implementation that validates the largest product and architectural assumption.**

Then measure it. Do not optimize or generalize before evidence shows that the capability is valuable and the chosen boundary is correct.

## 18. P0 -> P1 transition

P0 is complete. It established the native architecture, public C boundary, domain separation, dpkg vertical slice, target security boundary, resource governance, testing/fuzzing foundations, real privileged filesystem security verification, compiler/configuration evidence, performance evidence, and release/provenance discipline.

P1 therefore starts from a working foundation.

The next decision is not "how do we code more?"

It is:

> **Which additional software-environment intelligence capability creates the highest validated value while preserving the security and architectural properties established in P0?**

That is the P1 product-selection gate.

## 19. Governance

This charter is the product-level source of truth.

When documents disagree:

- product intent comes from this charter;
- technical structure comes from HLD/LLD/architecture;
- security guarantees come from the security model and verified evidence;
- public compatibility comes from the API/ABI contract;
- release readiness comes from engineering gates and release evidence.

A product statement must never override a technical or security constraint without an explicit decision record.

A technically possible feature must not become roadmap scope without product and economic justification.

## 20. Current decision

**P0:** closed.

**P1:** product-definition gate established.

**Immediate next gate:** perform a structured P1 capability decision across the candidate areas using customer value, security surface, architecture impact, resource cost, implementation effort, and reversibility.

**No P1 implementation feature is selected by this document alone.**

The next artifact should be a P1 requirements/decision record for the selected capability, followed by HLD/LLD review before implementation.
