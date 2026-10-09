# P1 Capability Decision

## Status

**Decision:** SELECTED — Versioned machine-readable JSON export of the existing pkgintel observation model.

**Decision scope:** P1 product-selection and requirements gate. The selection is now followed by the reviewed HLD/LLD and frozen schema; implementation is tracked in PR #7 and remains subject to applicable evidence gates.

**Decision basis:** current `main` after PR #6 merge, commit `9bbbdbb237a1f7601686e818e1df2cbb416f905a`.

## 1. Decision summary

P1 will validate whether pkgintel's existing structured inspection result can become a useful machine-readable integration boundary without materially expanding the discovery/security surface.

The selected capability is:

> **A versioned JSON projection of a completed pkgintel scan, initially exposed through the CLI as deterministic stdout output.**

The first implementation should **not** introduce:

- network transport;
- database persistence;
- cloud upload;
- a JSON-over-HTTP service;
- a new public C serializer API;
- a new package manager;
- ELF parsing;
- broad filesystem crawling.

The purpose is to turn the existing package/installation/artifact/diagnostic model into a stable integration artifact while keeping the security boundary narrow.

## 2. Why this capability

The product charter defines pkgintel as an evidence-producing inspection engine rather than a presentation-only package parser. That product promise is difficult to validate if the structured result can only be consumed through internal C objects or human-oriented CLI text.

JSON export is therefore selected as an **integration-enabling capability**, not as the product's differentiation by itself.

It validates several important assumptions at once:

1. the domain model is sufficiently coherent for external consumption;
2. package, installation, artifact, and diagnostic records have stable semantics;
3. observation quality can be represented without overclaiming;
4. partial, incomplete, inaccessible, and resource-limited scans can be represented explicitly;
5. downstream tools can consume pkgintel without linking directly to the C ABI;
6. schema versioning can evolve independently from the native ABI;
7. the existing security/resource model remains intact when results leave the native process.

This is a better P1 learning loop than immediately adding a high-risk parser or widening Linux coverage.

## 3. Candidate comparison

Scoring is 1–5. Higher is better.

- **Product value:** direct usefulness to a real consumer.
- **Security posture:** lower new attack surface / easier to verify.
- **Architecture fit:** alignment with the existing P0 boundaries.
- **Implementation leverage:** value relative to engineering effort.
- **Integration leverage:** ability to enable downstream consumers.
- **Reversibility:** ability to change or remove the capability without destabilizing the core.

| Candidate | Product value | Security posture | Architecture fit | Implementation leverage | Integration leverage | Reversibility | Decision |
|---|---:|---:|---:|---:|---:|---:|---|
| Versioned JSON export | 4 | 4 | 5 | 4 | 5 | 5 | **SELECT** |
| Additional package adapters | 4 | 3 | 4 | 3 | 3 | 4 | Defer |
| ELF inspection | 5 | 2 | 4 | 2 | 3 | 3 | Defer |
| APT metadata/cache | 3 | 3 | 4 | 3 | 2 | 4 | Defer |
| Rust FFI | 3 | 4 | 3 | 2 | 4 | 3 | Defer |
| Broader filesystem correlation | 4 | 2 | 4 | 2 | 3 | 2 | Defer |

The scores are engineering decision estimates, not measured commercial facts. Customer discovery may later overturn the ranking.

## 4. Principal Software Engineering assessment

### Architecture

JSON serialization should sit downstream of the immutable snapshot/domain model:

```
target
  ↓
scan
  ↓
immutable snapshot
  ↓
domain records
  ↓
JSON projection
  ↓
stdout / external consumer
```

The serializer must not become a second source of truth.

The C domain model remains authoritative. JSON is a representation of that model.

### ABI strategy

P1 should avoid adding a public serializer ABI unless requirements prove it necessary.

The initial boundary should be:

- existing C scan API;
- existing immutable snapshot;
- CLI serialization layer;
- versioned JSON schema.

This deliberately separates:

- **native ABI compatibility**, and
- **machine-readable data-schema compatibility**.

That separation reduces the risk of prematurely freezing a serializer API into the 0.x C ABI.

### Determinism

For equivalent snapshots, JSON output should be deterministic with respect to:

- field names;
- record ordering;
- enum representation;
- diagnostic ordering;
- numeric representation;
- schema version.

Determinism is important for testing, diffing, caching, reproducibility, and downstream automation.

If an ordering is not semantically defined by the domain model, the serializer must define one rather than depending on hash/table iteration order.

## 5. Principal Security Engineering assessment

JSON export is lower risk than adding an attacker-facing parser, but it is not security-free.

### Required security properties

1. **No execution**
   - Serialization must never execute discovered paths, binaries, interpreters, or package-manager commands.

2. **No target mutation**
   - Serialization must operate on the immutable result and must not modify the inspected target.

3. **No path re-resolution**
   - Serialization must not reopen or re-resolve target paths merely to produce output.

4. **Lossless hostile-byte handling**
   - Linux paths are arbitrary bytes.
   - The JSON representation must define how non-UTF-8 bytes are represented without data loss or invalid JSON.

5. **Bounded output work**
   - Output size and allocation behavior must be reasoned about.
   - The serializer must not introduce unchecked multiplication, addition, recursion, or allocation based on target-controlled values.

6. **Explicit partial-state semantics**
   - A resource-limited or incomplete scan must not serialize as if it were a clean complete inventory.

7. **Diagnostic safety**
   - Human-readable diagnostic text must not become a machine-readable security assertion.
   - Stable diagnostic codes and structured fields remain authoritative.

8. **No information invention**
   - JSON must expose only facts present in the snapshot.
   - It must not infer capabilities, vulnerabilities, execution state, or security posture that the scanner did not establish.

### Security review requirement

The serializer needs its own hostile-input tests, especially for:

- arbitrary Linux byte paths;
- quotes, backslashes, control characters, and delimiters;
- very long strings;
- large artifact/diagnostic counts;
- empty strings and empty arrays;
- resource-limit snapshots;
- inaccessible/missing artifacts;
- unknown installation states;
- malformed-but-representable diagnostic data.

A successful JSON parse is not sufficient evidence of semantic correctness.

## 6. CA / Finance assessment

The selected capability has a favorable initial cost profile because it reuses the existing scan and snapshot pipeline.

Expected incremental costs:

- serializer implementation and tests;
- schema documentation;
- compatibility tests;
- additional fuzz/property-style testing where useful;
- future schema maintenance.

Avoided costs:

- no database;
- no network service;
- no cloud storage;
- no new daemon;
- no recurring infrastructure dependency;
- no new package-manager backend;
- no new privileged runtime requirement.

The financial hypothesis being tested is:

> Can a reusable, local, machine-readable inspection result create integration value without adding meaningful operating cost?

No revenue or customer willingness-to-pay claim is made yet.

## 7. MBA / Product assessment

### User/problem

Potential consumers need structured inspection data they can:

- pipe into another program;
- archive;
- diff;
- test;
- ingest into their own tooling;
- transform without scraping human CLI text.

### Why now

The P0 native foundation and domain separation now make this a relatively small vertical step. Deferring machine-readable output while adding more discovery sources would increase the amount of internal functionality that lacks an external integration boundary.

### What success looks like

A downstream consumer should be able to run a scan and consume the result without:

- parsing human text;
- linking to private implementation structs;
- depending on internal C headers;
- executing package-manager commands itself.

### What would cause us to stop

Stop or redesign if:

- the domain model cannot represent observations without ambiguity;
- deterministic serialization exposes unstable semantics;
- output requires unsafe target re-resolution;
- resource amplification is materially larger than expected;
- consumers need a different contract than JSON;
- the schema becomes a dumping ground for internal implementation details.

## 8. P1 functional requirements

### R1 — Schema identity

Every document must identify:

- schema name;
- schema version;
- producer/product identity;
- product version where available.

Schema versioning must be independent from the native library ABI version.

### R2 — Scan outcome

The document must represent:

- successful completion;
- partial/incomplete observation;
- resource-limit termination;
- relevant diagnostics.

A process exit status alone is not sufficient to describe observation quality.

### R3 — Domain records

The projection must preserve the distinction between:

- package identity;
- installation state;
- artifact observation;
- diagnostic/evidence information.

No record type may be silently collapsed into another.

### R4 — Evidence provenance

Where the snapshot contains source/provenance information, JSON must preserve it.

The output must distinguish observed facts from inferred/correlated relationships where the model provides that distinction.

### R5 — Linux paths

The schema must define a lossless representation for arbitrary Linux path bytes.

A consumer must be able to distinguish:

- valid UTF-8;
- non-UTF-8 bytes;
- empty values;
- missing/unavailable values.

The serializer must not silently replace invalid bytes with a lossy Unicode replacement character.

### R6 — Deterministic output

Equivalent snapshots must serialize deterministically.

The ordering contract must be documented and tested.

### R7 — Stable machine-readable enums

Machine consumers must receive stable enum identifiers or numeric codes whose compatibility rules are documented.

Human labels must not be the only representation of semantic state.

### R8 — CLI boundary

Initial CLI behavior should support JSON output to stdout.

Human-oriented progress/status text must not corrupt stdout JSON. Diagnostic/progress presentation belongs on stderr when the CLI contract requires both.

### R9 — No network

The initial JSON export performs no network activity.

### R10 — No persistence requirement

The serializer does not require a database, cache, queue, or background service.

## 9. Non-functional requirements

### Performance

Serialization must be measured separately from scanning.

At minimum record:

- snapshot size/count;
- output bytes;
- serialization wall time;
- peak memory or allocation behavior where practical.

Do not claim that JSON serialization performance represents scan performance.

### Resource bounds

The design must state:

- maximum relevant nesting depth;
- string length handling;
- aggregate output-size behavior;
- allocation strategy;
- integer overflow protections.

If a configured output limit is introduced, its unit, scope, exhaustion rule, and partial-output semantics must be explicit.

### Compatibility

The schema must document:

- additive changes;
- incompatible changes;
- field removal policy;
- enum evolution;
- unknown-field handling;
- unknown-enum handling;
- schema-version negotiation expectations.

## 10. Proposed initial JSON shape

This is a requirements sketch, not a frozen schema.

```json
{
  "schema": {
    "name": "pkgintel.scan",
    "version": 1
  },
  "producer": {
    "name": "pkgintel",
    "version": "..."
  },
  "scan": {
    "status": "complete",
    "diagnostics": []
  },
  "packages": [],
  "installations": [],
  "artifacts": []
}
```

The final schema must be derived from the actual domain model, not from this example.

Do not expose private implementation fields merely because they are available internally.

## 11. Acceptance criteria

P1 JSON export is not complete until all applicable criteria pass:

1. A known fixture produces valid JSON.
2. The JSON parses with an independent standards-compliant parser.
3. Equivalent snapshots produce byte-for-byte deterministic output.
4. Package, installation, artifact, and diagnostic semantics remain distinct.
5. Partial/resource-limited scans remain distinguishable from complete scans.
6. Arbitrary Linux path bytes have a documented lossless representation.
7. Hostile strings cannot break JSON syntax.
8. Large/count-boundary fixtures do not overflow or crash.
9. Serializer tests pass under GCC and Clang.
10. ASan/UBSan coverage passes for serializer paths.
11. Fuzzing is added or explicitly justified for the serializer.
12. No discovered program is executed.
13. No target content is modified.
14. No target path is re-resolved solely for serialization.
15. CLI stdout contains only the declared JSON contract in JSON mode.
16. Documentation defines schema versioning and compatibility rules.
17. API/ABI review confirms no accidental public ABI expansion.
18. Performance measurements separate scan cost from serialization cost.

## 12. Required design review before implementation

Before coding:

### HLD review

Define:

- serializer ownership;
- snapshot traversal boundary;
- CLI integration;
- stdout/stderr behavior;
- schema ownership;
- failure boundary.

### LLD review

Define:

- escaping algorithm;
- byte-to-JSON representation;
- deterministic ordering;
- integer formatting;
- allocation strategy;
- error propagation;
- partial-output policy;
- cleanup/lifetime behavior.

### Security review

Threat-model:

- hostile package metadata;
- hostile filenames;
- malformed diagnostics;
- enormous result sets;
- serialization amplification;
- output truncation;
- terminal/control-character injection;
- accidental target re-access.

### API/ABI review

Confirm that JSON schema evolution does not silently become C ABI evolution.

### Evidence plan

Identify exact tests and runtime conditions before implementation.

## 13. Deferred capabilities

The following remain candidates for later P1/P2 work:

- ELF inspection;
- broader filesystem correlation;
- APT metadata/cache;
- additional package-manager adapters;
- Rust FFI.

The selection of JSON export does not imply these are permanently rejected.

## 14. Decision outcome

**SELECTED:** versioned JSON export, CLI-first and stdout-first.

**NOT SELECTED:** ELF, APT, broad filesystem crawling/correlation, Rust FFI, and additional package adapters as the immediate P1 implementation.

**Reason:** JSON export is the smallest reversible capability that can validate an important product assumption — whether pkgintel's evidence-aware domain model is useful to external consumers — while adding comparatively little new privileged or attacker-facing surface.

The next engineering artifact is **HLD/LLD review of this selected capability**.

No implementation should begin until that review confirms the domain, security, resource, API/ABI, and acceptance contracts above.


## 13. Implementation handoff

The selected capability has moved from decision/design into implementation on PR #7.

Authoritative implementation contract:

- `docs/P1_JSON_HLD_LLD.md`
- `docs/P1_JSON_SCHEMA.md`

The implementation remains intentionally CLI-private and does not add a public C JSON ABI.

Implementation acceptance is not implied by the product decision. PR #7 must still establish build, sanitizer, independent-parser, fuzz, ABI, performance, and exact-SHA review evidence before merge.


## Implementation outcome — 2026-10-09

**Status: IMPLEMENTED AND MERGED.** The selected P1 capability shipped through [PR #7](https://github.com/Rofikali/pkgintel/pull/7).

- Reviewed implementation head: `a75214124d0d1027c1eb1e9eedd2a358832a84c1`.
- Merge commit: `8b4380d4f353017854a2104ac5a39a8029675c66`.
- Exact-head verification: [GitHub Actions run 37882487979](https://github.com/Rofikali/pkgintel/actions/runs/37882487979).
- Frozen v1 schema: `docs/P1_JSON_SCHEMA.md`.
- HLD/LLD: `docs/P1_JSON_HLD_LLD.md`.
- Post-merge engineering handoff and reproduction details: `docs/P1_JSON_EXPORT_HANDOFF.md`.

The dedicated strict Linux filesystem security runtime passed the genuine mount-boundary test. The separate generic unprivileged sanitizer job skipped that mount test; the skip is not a security pass. See the handoff document for exact evidence classification, fuzz resource results, benchmark caveats, and future-agent instructions.
