# P1 JSON Export — HLD / LLD Review

## Status

**Design status:** IMPLEMENTATION BASELINE FROZEN; implementation/evidence review is now in progress.

**Scope:** versioned machine-readable JSON projection of the existing immutable scan result, initially CLI-first and stdout-first.

This document records the concrete data structures, algorithms, mathematical bounds, ownership, security boundaries, concurrency assumptions, API/ABI decisions, and verification plan that must be satisfied before implementation is accepted.

## 1. Current repository facts

The current result is a materialized domain snapshot:

    pkg_snapshot
      ├── packages[]
      ├── artifacts[]
      ├── diagnostics[]
      ├── target_root
      └── resource accounting

Packages refer to contiguous artifact ranges through artifact_start, artifact_count, and owner_snapshot.

Artifacts contain path + path_size, kind, state, logical_size, allocated_size, and allocated_size_valid.

Diagnostics contain code, message, status, severity, and evidence_source.

The public API exposes indexed read-only access to these collections. The serializer can therefore be a downstream consumer rather than a new domain owner.

## 2. Architectural decision

The architecture is:

    CLI
     │
     │ scan
     ▼
    pkg_scan(...)
     │
     ▼
    pkg_snapshot
     │
     │ read-only traversal
     ▼
    JSON projection
     │
     ├── stdout: JSON document
     └── stderr: operational/progress errors

The serializer does not scan, resolve target paths, reopen files, invoke package managers, modify the target, own the snapshot, introduce a cache/database/network, or become a public C ABI in P1.

Responsibility:

| Component | Owns |
|---|---|
| scan/core | observation and scan semantics |
| snapshot | result ownership/lifetime |
| public snapshot/package/artifact/diagnostic API | read-only access contract |
| JSON serializer | representation only |
| CLI | mode selection, stdout/stderr, exit policy |

## 3. Domain model remains authoritative

JSON is a projection:

    domain model → projection → JSON

It must not become a second domain model.

The serializer must not invent package state, filesystem state, vulnerability information, capabilities, security conclusions, or provenance.

## 4. Important source-review finding: scan status

The current pkg_snapshot does not store a scan status field.

The scan operation returns pkg_status, and the current CLI already distinguishes PKG_ERR_RESOURCE_LIMIT.

Therefore JSON scan status must initially be derived from the scan operation status supplied by the caller, not invented as a snapshot property.

This preserves:

    operation outcome
          ≠
    snapshot observations

A future richer scan-outcome model may be justified, but that is a separate domain/API decision.

## 5. DSA decision

The existing snapshot uses contiguous dynamic arrays for packages, artifacts, and diagnostics with geometric growth.

For serialization, do not build a second collection or intermediate JSON DOM.

Use a streaming writer/state machine over the immutable snapshot.

Conceptually:

    snapshot → writer → JSON tokens → stdout

This keeps additional serializer memory O(1) apart from a bounded output buffer.

A serializer-side hash map is rejected because the snapshot already contains the package/artifact relationship through contiguous artifact ranges. A second index would add memory, hashing, collision behavior, and another representation of domain knowledge.

## 6. Ordering and deterministic output

The current arrays have insertion/scan order, but that order must not automatically be treated as a public semantic contract.

Before implementation, inspect the scan path and establish whether package, artifact, and diagnostic order is deterministic across equivalent inputs.

Decision rule:

1. preserve existing deterministic order if proven;
2. otherwise define a canonical order;
3. only then introduce sorting.

If sorting is required:

    T_sort(N) = O(N log N)

and its temporary memory and benchmark cost must be documented.

Do not sort every collection merely because deterministic output sounds desirable.

## 7. JSON projection algorithm

Primary flow:

    validate serializer inputs
        ↓
    write schema metadata
        ↓
    write producer metadata
        ↓
    write scan outcome
        ↓
    write packages sequentially
        ↓
    write artifacts sequentially
        ↓
    write diagnostics
        ↓
    close JSON document
        ↓
    flush/check output

For each value:

    read accessor
        ↓
    validate representation
        ↓
    escape/encode
        ↓
    emit

No target access occurs.

Without sorting:

    T(P,A,D,S) = O(P + A + D + S)

where P = packages, A = artifacts, D = diagnostics, and S = total bytes processed/emitted.

Additional serializer memory is O(1) plus bounded writer buffering.

## 8. JSON escaping

The serializer must correctly escape quotation marks, reverse solidus, and control characters.

It must never copy raw arbitrary bytes directly between JSON quotes.

For a representable string S:

    parse(serialize(S)) = S

for the declared string domain.

## 9. Linux path representation

Artifact paths are byte-oriented:

    pkg_path {
        const unsigned char *data;
        size_t size;
    }

Linux path bytes are not required to be UTF-8.

Therefore a plain JSON string cannot be the authoritative lossless representation.

Initial schema direction:

    "path": {
        "encoding": "base64",
        "data": "..."
    }

Exact field names are frozen during schema review.

Base64 is selected because every byte has a deterministic JSON-safe representation without locale dependence or Unicode replacement loss.

For N path bytes, base64 output is bounded by:

    4 * ceil(N / 3)

plus JSON structural overhead.

All arithmetic must be overflow-safe.

A future human-friendly UTF-8 display field may be considered, but it must never replace the authoritative byte representation.

## 10. Mathematical and resource model

Serializer output is not equivalent to existing snapshot string accounting.

    OutputBytes != SnapshotStringBytes

and output may be larger because of JSON syntax, escaping, and base64 expansion.

Before multiplication:

    count <= SIZE_MAX / element_size

Before addition:

    required <= SIZE_MAX - current

Base64 expansion must likewise be calculated without overflow.

P1 should initially avoid a configurable serializer output limit unless product requirements demand one.

If a limit is later added, define its unit, scope, accounting point, exhaustion status, and partial-output policy.

For machine-readable stdout, the preferred failure policy is not to claim success after emitting an incomplete JSON document.

## 11. Memory / ownership / lifetime

The serializer borrows the snapshot.

    snapshot owns records
    serializer borrows records
    caller retains snapshot ownership

The serializer must not free snapshot fields, mutate records, retain pointers after return, or transfer ownership.

Serialization does not mutate the snapshot, so a serialization failure leaves the snapshot valid.

This is preferable to a mutable intermediate representation requiring rollback.

## 12. Concurrency

P1 introduces no serializer threads.

The model is:

- one serializer invocation;
- one immutable snapshot;
- no shared mutable serializer state;
- no locks.

Independent snapshots may be serialized concurrently by independent callers if the public lifetime contract permits it.

A future parallel serializer would require measured workload justification, deterministic ordering, bounded worker count, output coordination, memory-amplification analysis, and shutdown semantics.

## 13. SOLID and design principles

Single Responsibility:
- serializer converts an existing result into JSON;
- it does not discover packages or enforce target confinement.

Open/Closed:
- no plugin framework is justified for P1.

Liskov:
- any future output sink abstraction must preserve ordering, write-error, short-write, and flush semantics.

Interface Segregation:
- prefer a small internal writer contract over a generic I/O framework.

Dependency Inversion:
- serializer depends on snapshot access, not filesystem/package-manager internals.

DRY:
- reuse existing accessors and domain semantics;
- do not duplicate installation-state or artifact-state mappings in JSON code.

## 14. Design patterns

A small writer abstraction may be justified if it cleanly isolates output failure handling from JSON token generation.

Do not introduce:

- factory hierarchies;
- strategy registries;
- plugin systems;
- dependency-injection containers;
- JSON object-model frameworks;
- visitor frameworks.

There is currently no evidence that these solve a real P1 problem.

## 15. API / ABI decision

P1 remains CLI-first and does not add a public C serializer API.

A public JSON function would immediately create long-lived questions about returned-buffer ownership, allocation ABI, lifetime, output limits, and error behavior.

The serializer should prefer public read-only accessors. If required information is unavailable, that is an API/domain design finding to review before adding an accessor.

Do not cast opaque public handles back into private structures merely to avoid an API discussion.

## 16. Scan status and diagnostics

The scan operation outcome and diagnostics are different dimensions:

    scan status = operation outcome
    diagnostics = evidence/details

JSON must preserve both.

For example:

    scan.status = resource_limit

must not become complete merely because a snapshot exists.

A diagnostic must not automatically become a global scan failure unless the documented scan semantics say so.

## 17. Evidence and consistency

Package consistency is already computed by the domain layer from correlation state and artifact observations.

The serializer must expose the public semantic result rather than reimplementing consistency.

Do not create:

    C consistency logic
          +
    JSON consistency logic

That would allow semantic drift.

If consumers later need correlation state itself, that is a domain/schema requirement to review separately.

## 18. Security threat model

Potentially hostile projected values include package names, versions, architectures, artifact paths, diagnostic codes/messages, and values derived from target metadata.

The serializer must:

1. never execute values;
2. never interpret values as shell syntax;
3. never resolve paths;
4. never reopen target files;
5. never follow links;
6. never mutate the target;
7. escape JSON syntax correctly;
8. bound arithmetic;
9. preserve arbitrary path bytes;
10. fail safely on output errors.

Control characters must be escaped rather than emitted raw, including values that could otherwise act as terminal control sequences.

## 19. Failure semantics

Possible failures include invalid arguments, invalid snapshot state, allocation failure if buffering is used, output I/O failure, encoding failure, and arithmetic/resource failure.

Preferred contract:

    success → complete valid JSON document
    failure → non-success status and no claim of complete JSON

The CLI must preserve an appropriate non-zero exit status.

A partial JSON stream must never be mistaken for a successful machine-readable result.

## 20. Testing model

Unit tests should cover:

- empty snapshot;
- one package;
- zero artifacts;
- many artifacts;
- diagnostics;
- all enum values;
- zero and large numeric values;
- quotes/backslashes/control characters;
- arbitrary path bytes;
- empty arrays;
- resource-limit scan status.

Properties:

    parse(serialize(snapshot)) == expected semantic model

and:

    serialize(snapshot) == serialize(snapshot)

for deterministic output.

Use an independent JSON parser.

Fuzzing should establish:

- no crash;
- no sanitizer finding;
- output is valid complete JSON or a controlled failure;
- no target access.

## 21. ABI/API verification

Because P1 should not add a public serializer ABI:

- existing ABI symbol list must remain unchanged;
- installed consumer tests remain unchanged;
- no public header is required solely for JSON.

Any accidental new exported serializer symbol is a gate failure.

## 22. Performance model

Measure separately:

    scan_time
    serialization_time
    output_bytes
    end_to_end_time

At minimum:

    output_throughput = output_bytes / serialization_time

and:

    serialization_overhead = serialization_time / scan_time

Do not publish JSON serialization performance as scanner performance.

Expected scaling without sorting:

    O(P + A + D + S)

If sorting is later introduced:

    O(P log P + A log A + D log D + S)

where appropriate.

## 23. CLI contract

The current command is:

    pkgintel scan

P1 direction:

    pkgintel scan --json

Exact flag naming is subject to CLI review, but JSON mode must guarantee:

- JSON document only on stdout;
- operational/progress text on stderr;
- non-zero exit on failed serialization;
- resource-limited scans remain distinguishable;
- no human-readable banners on stdout.

## 24. Schema compatibility

Schema version 1 must define:

- additive-field rules;
- breaking-change rules;
- field removal policy;
- enum evolution;
- unknown-field handling;
- unknown-enum handling;
- schema-version semantics.

Schema version is independent from library SONAME, native ABI, and CLI version.

## 25. Alternatives

### JSON DOM
Rejected for P1 because it adds O(output-size) intermediate memory and another ownership model.

### Public C JSON API
Rejected for P1 because it freezes allocation/ownership/error semantics before consumer demand is known.

### Serialize directly from target
Rejected because it re-enters security boundaries and can diverge from the immutable result.

### Sort everything
Rejected by default until source/runtime evidence proves existing ordering is insufficient.

### Hash/index everything
Rejected because current domain relationships already provide direct traversal.

### Serializer threads
Rejected because no measured workload justifies the complexity or memory amplification.

## 26. Implementation gate

Before implementation is accepted:

- [ ] final JSON schema fields approved;
- [ ] canonical ordering decision made from source/runtime evidence;
- [ ] path byte encoding frozen;
- [ ] scan-status mapping frozen;
- [ ] diagnostic representation frozen;
- [ ] output failure semantics frozen;
- [ ] ownership/lifetime contract frozen;
- [ ] no public ABI expansion confirmed;
- [ ] output arithmetic bounds reviewed;
- [ ] test matrix accepted;
- [ ] fuzz strategy accepted;
- [ ] benchmark methodology accepted.

## 27. Principal Engineering decision

The smallest design that validates the product hypothesis is:

    immutable snapshot
          ↓
    bounded read-only traversal
          ↓
    streaming JSON writer
          ↓
    stdout

It deliberately does not add a second domain model, JSON DOM, public serializer ABI, concurrency, registry, plugin system, database, or networking.

This is the intended Staff/Principal outcome:

> Use DSA, algorithms, mathematics, SOLID, design patterns, security, concurrency, and performance analysis to justify what we need — and use the same disciplines to justify what we deliberately do not need.

## 28. Final design status

HLD: accepted as the direction.

LLD: sufficiently specified for implementation planning, subject to closing the implementation-gate checklist.

DSA: contiguous snapshot traversal; no serializer-side index/DOM.

Algorithm: linear projection with bounded streaming output; sorting only if source ordering cannot be proven deterministic.

Mathematics: explicit overflow-safe arithmetic and base64/output expansion model.

Memory: snapshot-owned, serializer-borrowed; no intermediate JSON DOM.

Concurrency: none introduced.

Patterns: minimal writer abstraction only if it clarifies sink/error handling; framework patterns rejected.

Security: serializer never re-enters target observation and treats projected values as untrusted data.

API/ABI: no new public serializer ABI in P1.

Evidence: implementation-specific runtime, performance, and security evidence remains pending and must not be claimed until produced.

Next gate: close the implementation-gate checklist, then implement and run the applicable verification matrix.


## 26. Implementation closure and frozen v1 schema

Source review closed the remaining design questions:

- **Exact schema:** frozen in `docs/P1_JSON_SCHEMA.md`.
- **Package ordering:** source-level proof exists in `src/backends/dpkg/dpkg.c`; packages are sorted by name, architecture, then version before correlation. The serializer preserves this order and does not sort again.
- **Artifact ordering:** each package's artifacts are the contiguous snapshot range created by correlation, preserving package-file-list order.
- **Diagnostic ordering:** diagnostics preserve snapshot emission order.
- **Scan status:** `PKG_OK -> complete`; `PKG_ERR_RESOURCE_LIMIT -> resource_limit`. Other scan failures do not reach JSON mode.
- **Byte encoding:** all byte-oriented strings are encoded as standard padded RFC 4648 Base64 objects. This applies to package metadata, paths, and diagnostics, avoiding invalid UTF-8 output and lossy replacement.
- **Output failure:** write/flush failures return non-success. The CLI exits non-zero; a partial stream is never represented as a successful result.
- **Writer placement:** serializer implementation is private under `src/json/json.c`; its private header is under `src/internal/json.h`.
- **ABI:** no public serializer header or exported serializer symbol is introduced.
- **Memory:** no output-sized intermediate DOM or allocation is required; the serializer streams directly to `FILE *`.
- **Relationship model:** artifacts remain nested under their owning package in JSON, while the C snapshot remains the authoritative flat artifact collection.

The implementation is intentionally projection-only: it consumes public read-only accessors and never re-enters target observation.

The schema and implementation must still pass the applicable build, sanitizer, parser, fuzz, CLI, ABI, and performance evidence gates before PR #7 can be accepted.
