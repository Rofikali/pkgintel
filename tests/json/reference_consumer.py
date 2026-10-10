#!/usr/bin/env python3
"""Internal P2 reference consumer for the public pkgintel CLI JSON contract.

This is an integration probe, not a production security scanner or a claim of
market validation. It deliberately uses only the CLI and Python's standard
library; it does not link to pkgintel or import private implementation code.
"""
import base64
import json
import subprocess
import sys


class ConsumerError(ValueError):
    """The producer output cannot safely support the consumer's decision."""


def _require(condition, message):
    if not condition:
        raise ConsumerError(message)


def _reject_duplicate_keys(pairs):
    """Reject duplicate object keys instead of silently keeping the last key."""
    result = {}
    for key, value in pairs:
        if key in result:
            raise ConsumerError(f"duplicate JSON object key: {key}")
        result[key] = value
    return result


def _reject_non_json_constant(value):
    """Reject Python json's non-standard NaN and Infinity extensions."""
    raise ConsumerError(f"non-standard JSON constant: {value}")


def parse_json_document(text):
    """Parse strict JSON, rejecting duplicate keys and non-standard constants."""
    try:
        return json.loads(
            text,
            object_pairs_hook=_reject_duplicate_keys,
            parse_constant=_reject_non_json_constant,
        )
    except ConsumerError:
        # Duplicate-key and non-standard-constant failures are intentional,
        # specific validation errors; preserve their messages.
        raise
    except (ValueError, RecursionError) as exc:
        # The stdlib decoder can raise ValueError for integer-conversion limits
        # and RecursionError for deeply nested input, not only JSONDecodeError.
        raise ConsumerError(f"invalid JSON document: {exc}") from exc


def _is_uint(value):
    return isinstance(value, int) and not isinstance(value, bool) and value >= 0


def _decode_byte_value(value, where):
    _require(isinstance(value, dict), f"{where}: expected byte-value object")
    _require(value.get("encoding") == "base64", f"{where}: unsupported encoding")
    encoded = value.get("data")
    _require(isinstance(encoded, str), f"{where}: base64 data must be a string")
    try:
        decoded = base64.b64decode(encoded, validate=True)
    except (ValueError, base64.binascii.Error) as exc:
        raise ConsumerError(f"{where}: invalid base64 data") from exc
    _require(base64.b64encode(decoded).decode("ascii") == encoded,
             f"{where}: non-canonical base64 data")


def _require_enum(value, allowed, where):
    _require(isinstance(value, str) and value in allowed,
             f"{where}: unknown or invalid enum value")


def evaluate_document(document, producer_exit_code):
    """Validate schema v1 and return a conservative processing decision."""
    _require(isinstance(document, dict), "root: expected JSON object")

    schema = document.get("schema")
    _require(isinstance(schema, dict), "schema: expected object")
    _require(schema.get("name") == "pkgintel.scan", "schema: unexpected name")
    version = schema.get("version")
    _require(type(version) is int and version == 1,
             f"schema: unsupported version {version!r}")

    producer = document.get("producer")
    _require(isinstance(producer, dict), "producer: expected object")
    _require(producer.get("name") == "pkgintel", "producer: unexpected name")
    _require(isinstance(producer.get("version"), str) and producer["version"],
             "producer: missing version")

    scan = document.get("scan")
    _require(isinstance(scan, dict), "scan: expected object")
    status = scan.get("status")
    _require(status in ("complete", "resource_limit"), "scan: unknown status")
    expected_exit = 0 if status == "complete" else 3
    _require(producer_exit_code == expected_exit,
             f"producer exit {producer_exit_code} conflicts with scan status {status}")

    packages = document.get("packages")
    diagnostics = document.get("diagnostics")
    _require(isinstance(packages, list), "packages: expected array")
    _require(isinstance(diagnostics, list), "diagnostics: expected array")

    installation_states = {"unknown", "installed", "partial", "removed"}
    consistency_states = {
        "unknown", "consistent", "inconsistent", "missing_artifact",
        "broken_link", "permission_denied", "unexpected_artifact", "unverifiable",
    }
    artifact_kinds = {"unknown", "regular", "directory", "symlink", "other"}
    artifact_states = {
        "unknown", "present", "missing", "broken_link", "permission_denied",
        "unverifiable",
    }
    diagnostic_statuses = {
        "ok", "out_of_memory", "invalid_argument", "internal_error", "io_error",
        "permission", "not_found", "unsupported", "resource_limit", "corrupt_data",
        "cancelled",
    }
    severities = {"info", "notice", "warning", "error", "fatal"}
    evidence_sources = {"unknown", "dpkg", "apt", "filesystem", "elf", "path", "heuristic"}

    artifact_total = 0
    for package_index, package in enumerate(packages):
        where = f"packages[{package_index}]"
        _require(isinstance(package, dict), f"{where}: expected object")
        for field in ("name", "version", "architecture"):
            _decode_byte_value(package.get(field), f"{where}.{field}")
        _require_enum(package.get("installation_state"), installation_states,
                      f"{where}.installation_state")
        _require_enum(package.get("consistency"), consistency_states,
                      f"{where}.consistency")
        _require(_is_uint(package.get("installed_size_bytes")),
                 f"{where}.installed_size_bytes: expected unsigned integer")
        _require(_is_uint(package.get("file_count")),
                 f"{where}.file_count: expected unsigned integer")
        artifacts = package.get("artifacts")
        _require(isinstance(artifacts, list), f"{where}.artifacts: expected array")
        _require(package["file_count"] == len(artifacts),
                 f"{where}: file_count does not match artifacts length")
        artifact_total += len(artifacts)

        for artifact_index, artifact in enumerate(artifacts):
            artifact_where = f"{where}.artifacts[{artifact_index}]"
            _require(isinstance(artifact, dict), f"{artifact_where}: expected object")
            _decode_byte_value(artifact.get("path"), f"{artifact_where}.path")
            _require_enum(artifact.get("kind"), artifact_kinds, f"{artifact_where}.kind")
            _require_enum(artifact.get("state"), artifact_states, f"{artifact_where}.state")
            for field in ("logical_size_bytes", "allocated_size_bytes"):
                _require(_is_uint(artifact.get(field)),
                         f"{artifact_where}.{field}: expected unsigned integer")
            for field in ("logical_size_available", "allocated_size_available"):
                _require(type(artifact.get(field)) is bool,
                         f"{artifact_where}.{field}: expected boolean")

    for diagnostic_index, diagnostic in enumerate(diagnostics):
        where = f"diagnostics[{diagnostic_index}]"
        _require(isinstance(diagnostic, dict), f"{where}: expected object")
        _decode_byte_value(diagnostic.get("code"), f"{where}.code")
        _decode_byte_value(diagnostic.get("message"), f"{where}.message")
        _require_enum(diagnostic.get("status"), diagnostic_statuses, f"{where}.status")
        _require_enum(diagnostic.get("severity"), severities, f"{where}.severity")
        _require_enum(diagnostic.get("evidence_source"), evidence_sources,
                      f"{where}.evidence_source")

    return {
        "decision": "accept_complete" if status == "complete" else "reject_incomplete",
        "scan_status": status,
        "packages": len(packages),
        "artifacts": artifact_total,
        "diagnostics": len(diagnostics),
    }


def main(argv):
    if len(argv) != 2:
        print(f"usage: {argv[0]} /path/to/pkgintel", file=sys.stderr)
        return 2

    try:
        process = subprocess.run(
            [argv[1], "scan", "--json"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
            timeout=60,
        )
        if process.stderr:
            sys.stderr.buffer.write(process.stderr)
            sys.stderr.buffer.flush()
        try:
            text = process.stdout.decode("utf-8", errors="strict")
        except UnicodeDecodeError as exc:
            raise ConsumerError(
                f"producer stdout is not one complete UTF-8 JSON document: {exc}"
            ) from exc
        document = parse_json_document(text)

        decision = evaluate_document(document, process.returncode)
        print(json.dumps(decision, sort_keys=True, separators=(",", ":")))
        return 0 if decision["decision"] == "accept_complete" else 3
    except (OSError, subprocess.SubprocessError, ConsumerError) as exc:
        print(f"reference-consumer: reject: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
