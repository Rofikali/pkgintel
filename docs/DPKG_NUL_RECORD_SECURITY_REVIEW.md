# Dpkg Embedded-NUL Security Review

**Review date:** 2026-10-10  
**Repository:** `Rofikali/pkgintel`  
**Code commit reviewed and tested:** `4ca351e33c72d56c0a7bdda39880b6d737ec9a05`  
**Branch:** `security/reject-nul-in-dpkg-records`

## Decision

**Review outcome: no confirmed defect found in the reviewed change; verification evidence supports proceeding to the repository's normal merge/review process.**

This is a scoped review of embedded-NUL handling in dpkg status and package file-list records. It is not a general security certification or a claim that all pkgintel attack surfaces are exhaustively tested.

## Security behavior reviewed

- The bounded record reader detects embedded NUL bytes instead of allowing C-string interpretation to silently discard the suffix.
- A NUL-bearing dpkg status record fails parsing with `PKG_ERR_PARSE`.
- A NUL-bearing package file-list record is rejected for correlation; the package is marked `PKG_CORRELATION_INCOMPLETE` and a `PKG_DPKG_FILELIST_MALFORMED` diagnostic is emitted.
- The regression checks assert that a hostile file-list record does not produce correlated package files/artifacts and that a NUL-bearing status record does not create a truncated package identity.
- The dispatcher explicitly handles the file-list malformed-record return code (`rc == 4)); this path does not fall through to successful correlation.

## Verification evidence

The commands below were run against the code at the reviewed commit, before the documentation-only commit that adds this report and clarifies the security contract.

| Environment | Verification | Result |
|---|---|---|
| Development container, `/tmp/pkgintel-nul-review` | Focused `pkgintel.unit.core` CTest | PASS |
| Development container | ASAN/UBSAN build | PASS |
| Development container | Full ASAN/UBSAN CTest suite | 4 tests passed; `pkgintel.security.mounts` skipped because this environment could not provide the required mount capability |
| Security container, `/tmp/pkgintel-nul-security-review` | Debug build | PASS (55/55 build steps) |
| Security container | Full CTest suite, including `pkgintel.security.mounts` | PASS (5/5; zero skipped) |
| Security container | `git diff --check origin/main...HEAD` | PASS |

The security-container mount test executed and passed. The development-container sanitizer run did **not** execute that mount test, so it must not be described as sanitizer coverage of the mount test.

## Reproduction

Run these commands from the **development container** for the sanitizer evidence:

```sh
cd /tmp/pkgintel-nul-review
cmake --build build-review-sanitized
ASAN_OPTIONS=detect_leaks=1:abort_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
ctest --test-dir build-review-sanitized --output-on-failure
```

Run the full suite from the **security container** for real mount-boundary and procfs magic-link verification:

```sh
cd /tmp/pkgintel-nul-security-review
ctest --test-dir build-security-review --output-on-failure
```

## Scope and remaining limits

- The passing tests demonstrate the exercised regression and runtime paths; they do not prove the absence of all parser, filesystem, or concurrency defects.
- The genuine mount test validates the current test cases in the qualified security-container environment, not every possible kernel/filesystem configuration.
- Normal repository review, branch protection, CI, and maintainer approval remain required before merge.
- Build directories and `tests/json/__pycache__/` reported by `git status` are generated, untracked artifacts; they are not tracked source changes. Keep generated artifacts out of the patch.

## Evidence provenance

The tested source commit was `4ca351e33c72d56c0a7bdda39880b6d737ec9a05`. The security-contract update and this review report are documentation-only follow-up commits on the same feature branch; they do not change the tested C implementation.
