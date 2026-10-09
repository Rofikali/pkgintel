# P2 Reference Consumer — Technical Usability Experiment

## Status and scope

This is an **internal reference consumer**, not a customer integration and not market validation. It exists to test whether a separate program can consume the shipped `pkgintel scan --json` interface without parsing human-oriented CLI output, importing private C headers, or linking to the native library.

It is intentionally implemented with Python's standard library. It is not part of the production C library, installed package, or public C ABI.

## Consumer job

Decide whether one CLI scan result is eligible for downstream processing as a **complete observation**.

- `accept_complete`: the producer exited 0, the document says `complete`, and the required v1 fields and value domains validate.
- `reject_incomplete`: the producer exited 3, the document says `resource_limit`, and the document is structurally valid. The result is summarized for diagnostics but must not be ingested as a complete observation.
- `reject` / exit 2: the producer cannot be executed, stdout is not one complete UTF-8 JSON document, the exit status conflicts with the document, or the schema/contract is unsupported or invalid.

This decision means only that the observation is complete according to the current scan operation status. It does **not** mean all operating-system files were inventoried, package contents are safe, a machine is vulnerability-free, or the scan has market-proven utility.

## HLD / trust boundary

```text
reference consumer
  -> executes [pkgintel, scan, --json] without a shell
  -> captures stdout, stderr, and exit status separately
  -> parses stdout as UTF-8 JSON
  -> validates schema identity/version, fields, enums, Base64, counts
  -> checks exit status against scan.status
  -> emits a small decision summary (no package names or paths)
```

- stdout is untrusted input, even when produced by a local executable.
- stderr is forwarded separately and never parsed as data.
- The command is an argument vector; shell interpolation is not used.
- Unknown schema versions and unknown enum values fail closed.
- Additive unknown object fields are ignored to preserve the documented additive compatibility rule.
- Base64 values are decoded only to validate their canonical encoding; the consumer does not display or execute package/path values.
- The script uses no third-party packages, network, persistent storage, or elevated privileges.
- Output counters describe records in the document, not total host inventory.

## Resource model and limitations

The consumer buffers stdout because the current CLI emits one JSON document and this reference experiment needs to validate it before making a decision. Its memory use is therefore O(document size), unlike the streaming C serializer. It is suitable for a bounded test/experiment, not yet a production large-inventory consumer. The scan's resource-limit state is preserved and rejected as complete input.

No additional maximum input size is imposed here because the current public CLI contract does not yet define a JSON output-size ceiling. If this consumer is proposed for production, establish a byte limit before reading/buffering output and test its exhaustion semantics.

## Verification

The unit test exercises:
- complete accepted result;
- resource-limited result rejected as incomplete;
- unknown schema version;
- producer exit status/document mismatch;
- invalid Base64;
- package file-count mismatch;
- unknown artifact state;
- booleans rejected where unsigned integers are required.

The end-to-end CTest invokes the actual built `pkgintel` executable. A successful complete scan should result in exit 0 and `accept_complete`. A valid resource-limited scan remains exit 3 and `reject_incomplete`; that is correct consumer behavior, not a failed complete-scan test. Other producer failures or invalid output are rejected.

## Reproduction in Ubuntu 24.04 / Docker

```sh
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DPKGINTEL_BUILD_TESTS=ON \
  -DPKGINTEL_BUILD_CLI=ON
cmake --build build
ctest --test-dir build -R 'pkgintel[.]json[.]reference_consumer' --output-on-failure
```

The end-to-end test scans the actual target visible to the current runtime. It must not be described as a test of every target root, every package-manager backend, or a real customer workflow.

## Initial gap assessment

This experiment is intended to discover integration friction, not silently define a new production contract. Any mismatch should be classified as:
1. defect in the existing v1 implementation/contract;
2. validation gap in this reference consumer;
3. genuinely new downstream requirement.

Do not expand the schema or production API until a concrete consumer job requires it. The experiment is technically useful if it proves that schema/version/status distinctions can be consumed safely and reproducibly. It is not evidence of willingness to pay or customer demand.
