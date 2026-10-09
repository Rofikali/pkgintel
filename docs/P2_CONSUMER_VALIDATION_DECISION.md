# P2 Decision — Validate a Downstream Consumer Before Expanding Scope

## Status

**Decision: SELECT the next engineering gate; defer selection of the next production capability.**

P1 versioned CLI JSON export is implemented and merged. The repository has no open GitHub issues or pull requests that establish the next customer problem. The product charter also says the initial commercial/customer segment is a hypothesis and market validation must precede a commercial commitment.

Therefore, do not select ELF inspection, another package backend, APT cache analysis, broader filesystem crawling, or Rust FFI simply because they are technically interesting. The next step is to establish whether the shipped JSON boundary solves a real consumer integration problem and what the consumer actually needs.

This is a product-validation decision, not a claim that a customer has been interviewed or that the product has validated market demand.

## Problem and hypothesis

**Hypothesis:** a downstream tool can consume pkgintel's versioned scan output to make a useful decision without scraping human-oriented CLI text or linking against private C implementation details.

The first consumer may be an existing user-owned tool or a small reference consumer. A synthetic fixture alone can test contract mechanics, but it is not evidence of customer demand.

## Proposed validation experiment

1. Identify one concrete consumer and the job it needs to perform.
2. Ask what records and distinctions it needs: package identity/state, artifact observations, diagnostics, completeness/resource-limit status, and schema-version handling.
3. Integrate against the current public JSON contract without reading private headers or parsing human CLI output.
4. Record contract friction, missing semantics, operational constraints, and whether the integration changes the consumer's workflow.
5. Separate defects in the existing contract from genuinely new product requirements.
6. Only then compare production capability candidates against the observed need.

## Acceptance criteria

The validation gate is complete only when the evidence records:

- a named consumer/use case (which may be an internal reference consumer if no external user is available, explicitly labelled as such);
- the decision the consumer makes from pkgintel output;
- a reproducible end-to-end integration using the shipped CLI/schema;
- correct handling of schema version and incomplete/resource-limited observations;
- no dependence on private C structs, human-text scraping, or ungrounded security conclusions;
- concrete integration gaps and a decision to fix, defer, or reject each one;
- the next capability ranked against this evidence, engineering cost, security surface, reversibility, and maintenance burden.

A passing JSON syntax test alone does not satisfy this gate. An internal reference consumer proves technical usability, not market demand.

## Candidate production capabilities — deferred pending evidence

| Candidate | Why it may matter | Principal risk / cost | Current decision |
|---|---|---|---|
| Additional package-manager backend | Broader distro coverage | New parser/backend semantics, fixtures, and security surface | Defer until target users require it |
| ELF inspection | Adds binary evidence beyond package metadata | Complex hostile-input parser and substantial security/resource work | Defer until a use case justifies it |
| APT metadata/cache analysis | Additional package provenance and available-version context | More filesystem formats and ambiguous/stale state | Defer until consumer need is demonstrated |
| Rust FFI | Makes the native core easier to consume from Rust services | Ownership/error/lifetime contract and ABI integration cost | Defer until a Rust consumer exists |
| Broader filesystem inventory | More artifact coverage | Larger I/O, resource, privacy, and containment surface | Defer; not implied by current correlation |

These are qualitative hypotheses, not measured market scores. Re-rank them when actual consumer evidence exists.

## Non-goals

- Do not add an HTTP server, database, cloud persistence, or background queue as part of this validation.
- Do not change the public C ABI merely to make a demo easier.
- Do not expand target scanning or add new privileged behavior without a separate threat/resource review.
- Do not describe a reference integration as customer validation.
- Do not claim revenue, product-market fit, or willingness to pay from successful tests.
- Do not add abstractions or design patterns without a concrete consumer requirement.

## Go/no-go rule

- **Go:** a concrete consumer job is established, the current contract can support it or the smallest missing contract is well defined, and the integration has measurable utility.
- **Fix contract first:** the existing output is ambiguous, unstable, or fails to preserve an evidence distinction required by the consumer.
- **No-go / gather more evidence:** no concrete consumer problem is available, or the proposed expansion has no demonstrated value over the current output.

## Engineering handoff

Before implementation, follow `docs/PRINCIPAL_ENGINEERING_REVIEW_PROTOCOL.md`, inspect the exact current `main` SHA and branch ancestry, and write the requirements, non-goals, HLD/LLD, resource model, security invariants, and acceptance tests appropriate to the selected change. Preserve the existing evidence hierarchy: source review, tests, sanitizer/fuzz, real runtime, benchmarks, and production-like validation are different claims.

The P1 implementation evidence remains tied to the reviewed implementation SHA and CI run documented in `docs/P1_JSON_EXPORT_HANDOFF.md`; this decision document does not create new implementation evidence.
