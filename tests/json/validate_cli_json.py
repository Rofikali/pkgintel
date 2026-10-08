#!/usr/bin/env python3
import json
import subprocess
import sys

proc = subprocess.run([sys.argv[1], "scan", "--json"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False)
if proc.returncode not in (0, 3):
    sys.stderr.write(proc.stderr.decode("utf-8", errors="replace"))
    raise SystemExit(f"unexpected CLI exit status: {proc.returncode}")

if proc.stderr:
    # Operational diagnostics are allowed on stderr; JSON must remain stdout-only.
    pass

try:
    document = json.loads(proc.stdout.decode("utf-8"))
except Exception as exc:
    raise SystemExit(f"CLI did not emit valid UTF-8 JSON: {exc}")

assert document["schema"]["name"] == "pkgintel.scan"
assert document["schema"]["version"] == 1
assert document["producer"]["name"] == "pkgintel"
assert document["scan"]["status"] in ("complete", "resource_limit")
assert isinstance(document["packages"], list)
assert isinstance(document["diagnostics"], list)
for package in document["packages"]:
    assert package["name"]["encoding"] == "base64"
    assert package["version"]["encoding"] == "base64"
    assert package["architecture"]["encoding"] == "base64"
    assert isinstance(package["artifacts"], list)
print("independent JSON parse: PASS")