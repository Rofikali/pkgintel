# ADR-0011: Public C ABI Symbol Visibility

- Status: Accepted for v0.x; final policy required before v1.0
- Date: 2026-10-05

## Context

`pkgintel` exposes a C ABI intended to support the native CLI and a future Rust FFI layer. Public headers must describe the supported contract, while implementation symbols must remain private. Compiler visibility and linker exports are therefore part of the ABI, not merely a build detail.

## Decision

During the 0.x architecture stabilization period, the library may use default visibility to keep the reviewed public API linkable while the symbol set is being audited.

Before v1.0, the project will adopt an explicit linker export/version policy. Only reviewed ABI symbols will be exported. Internal functions and data symbols must not become part of the supported ABI accidentally.

The ABI review must cover:

- exported function names;
- symbol visibility;
- calling conventions and C linkage;
- ownership/lifetime semantics;
- fixed-width integer usage;
- structure layout exposed by public headers;
- SONAME policy;
- API/ABI compatibility rules.

## Security consequences

An explicit export set reduces accidental ABI exposure and limits the surface that future consumers, including Rust FFI, can depend upon. It also makes ABI review and compatibility testing auditable.

## Rejected alternatives

### Export everything permanently

Rejected because internal implementation details would become de-facto ABI.

### Hide everything immediately without completing the export audit

Rejected for v0.x because it can make valid public declarations fail to link and can hide incomplete ABI review behind build configuration.
