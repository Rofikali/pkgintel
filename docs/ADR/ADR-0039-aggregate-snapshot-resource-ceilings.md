# ADR-0039: Aggregate Snapshot Resource Ceilings

- Status: Accepted
- Date: 2026-10-07

## Context

The v0.1 scan API already limits package count and package-file records per package. Those controls are not sufficient as an aggregate memory/work bound: the theoretical product of the two values is far larger than a safe snapshot size, and diagnostics can also grow with malformed or missing package metadata.

Resource accounting must therefore exist at the scan-result boundary rather than only inside a backend parser.

## Decision

Every scan-created snapshot has two hard implementation ceilings:

- 1,000,000 artifact records
- 100,000 diagnostic records

The ceilings are internal v0.1 safety controls, not public tuning knobs. They are enforced by the snapshot mutation functions so every current producer shares the same aggregate boundary.

When an aggregate ceiling is reached:

1. the current valid snapshot contents remain intact;
2. the scan returns PKG_ERR_RESOURCE_LIMIT;
3. no additional artifact/diagnostic record is committed;
4. allocation failure remains distinguishable from a deliberate resource limit.

The existing public max_packages and max_package_files controls remain supported as caller-requested limits. The aggregate ceilings are independent hard safety boundaries.

## Rationale

This prevents a caller or hostile target from multiplying individually valid package/file limits into an unbounded snapshot.

The snapshot is the correct accounting boundary because it owns package-correlated artifact records, diagnostics, and their aggregate lifetime.

## Consequences

A scan may return a partial snapshot with PKG_ERR_RESOURCE_LIMIT. Consumers must not interpret that result as a complete scan.

The ceilings are intentionally conservative safety limits for v0.1. Future evidence shows they can be revised only through an explicit resource-budget review covering memory, CPU, compatibility, and security impact.

## Not solved by this ADR

These controls do not yet bound:

- aggregate filesystem traversal;
- total bytes read;
- recursion depth;
- individual file sizes;
- wall-clock duration;
- descriptor count;
- future ELF/APT/cache parser budgets.

Those remain separate gates and must not be advertised as implemented.

## Test requirement

Resource-limit tests must distinguish caller-requested package/file limits, parser record-size limits, aggregate snapshot ceilings, and allocation failure.

A resource-limit result must preserve the already committed snapshot state.