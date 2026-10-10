#!/usr/bin/env python3
"""Contract-focused unit tests for the P2 reference consumer."""
import importlib.util
import pathlib
import sys
import unittest
from unittest import mock

HERE = pathlib.Path(__file__).resolve().parent
SPEC = importlib.util.spec_from_file_location(
    "pkgintel_reference_consumer", HERE / "reference_consumer.py"
)
CONSUMER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(CONSUMER)


def byte_value(text):
    import base64
    return {"encoding": "base64", "data": base64.b64encode(text).decode("ascii")}


def document(status="complete"):
    return {
        "schema": {"name": "pkgintel.scan", "version": 1},
        "producer": {"name": "pkgintel", "version": "0.1.0"},
        "scan": {"status": status},
        "packages": [{
            "name": byte_value(b"demo"),
            "version": byte_value(b"1.0"),
            "architecture": byte_value(b"amd64"),
            "installation_state": "installed",
            "consistency": "unknown",
            "installed_size_bytes": 12,
            "file_count": 1,
            "artifacts": [{
                "path": byte_value(b"/usr/bin/demo"),
                "kind": "regular",
                "state": "present",
                "logical_size_bytes": 12,
                "logical_size_available": True,
                "allocated_size_bytes": 4096,
                "allocated_size_available": True,
            }],
        }],
        "diagnostics": [],
    }


class ReferenceConsumerTests(unittest.TestCase):
    def test_accepts_complete_document_and_summarizes_without_package_data(self):
        result = CONSUMER.evaluate_document(document(), 0)
        self.assertEqual(result, {
            "decision": "accept_complete",
            "scan_status": "complete",
            "packages": 1,
            "artifacts": 1,
            "diagnostics": 0,
        })

    def test_resource_limit_is_explicitly_rejected_as_incomplete(self):
        result = CONSUMER.evaluate_document(document("resource_limit"), 3)
        self.assertEqual(result["decision"], "reject_incomplete")
        self.assertEqual(result["scan_status"], "resource_limit")

    def test_rejects_duplicate_json_object_keys(self):
        with self.assertRaisesRegex(CONSUMER.ConsumerError, "duplicate JSON object key"):
            CONSUMER.parse_json_document(
                '{"schema":{"version":1,"version":2}}'
            )

    def test_rejects_non_standard_json_constants(self):
        for constant in ("NaN", "Infinity", "-Infinity"):
            with self.subTest(constant=constant):
                with self.assertRaisesRegex(
                    CONSUMER.ConsumerError, "non-standard JSON constant"
                ):
                    CONSUMER.parse_json_document('{"value":' + constant + '}')

    @unittest.skipUnless(
        hasattr(sys, "set_int_max_str_digits"),
        "Python runtime has no configurable integer-string conversion limit",
    )
    def test_normalizes_integer_conversion_limit_errors(self):
        previous_limit = sys.get_int_max_str_digits()
        try:
            sys.set_int_max_str_digits(640)
            text = '{"value":' + ("9" * 700) + '}'
            with self.assertRaisesRegex(
                CONSUMER.ConsumerError, "invalid JSON document"
            ):
                CONSUMER.parse_json_document(text)
        finally:
            sys.set_int_max_str_digits(previous_limit)

    def test_normalizes_decoder_recursion_errors(self):
        with mock.patch.object(
            CONSUMER.json, "loads", side_effect=RecursionError("nesting too deep")
        ):
            with self.assertRaisesRegex(
                CONSUMER.ConsumerError, "invalid JSON document"
            ):
                CONSUMER.parse_json_document("{}")

    def test_rejects_unknown_schema_version(self):
        data = document()
        data["schema"]["version"] = 2
        with self.assertRaisesRegex(CONSUMER.ConsumerError, "unsupported version"):
            CONSUMER.evaluate_document(data, 0)

    def test_rejects_exit_status_conflicting_with_document(self):
        with self.assertRaisesRegex(CONSUMER.ConsumerError, "conflicts with scan status"):
            CONSUMER.evaluate_document(document(), 3)

    def test_rejects_invalid_base64(self):
        data = document()
        data["packages"][0]["name"]["data"] = "%%%="
        with self.assertRaisesRegex(CONSUMER.ConsumerError, "invalid base64"):
            CONSUMER.evaluate_document(data, 0)

    def test_rejects_mismatched_file_count(self):
        data = document()
        data["packages"][0]["file_count"] = 0
        with self.assertRaisesRegex(CONSUMER.ConsumerError, "file_count"):
            CONSUMER.evaluate_document(data, 0)

    def test_rejects_unknown_artifact_state(self):
        data = document()
        data["packages"][0]["artifacts"][0]["state"] = "secure"
        with self.assertRaisesRegex(CONSUMER.ConsumerError, "unknown or invalid enum"):
            CONSUMER.evaluate_document(data, 0)

    def test_rejects_boolean_as_unsigned_integer(self):
        data = document()
        data["packages"][0]["installed_size_bytes"] = True
        with self.assertRaisesRegex(CONSUMER.ConsumerError, "unsigned integer"):
            CONSUMER.evaluate_document(data, 0)


if __name__ == "__main__":
    unittest.main()
