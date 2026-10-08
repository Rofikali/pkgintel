# pkgintel Working Instructions

This file is intentionally short. The repository-level engineering contract is in `AGENTS.md`.

## Mandatory onboarding

Before substantial work, read:

1. `AGENTS.md`
2. `docs/AGENT_ONBOARDING.md`
3. `docs/ENGINEERING_STATUS.md`
4. `docs/ENGINEERING_WORKFLOW.md`
5. `docs/BRANCH_PROVENANCE.md`
6. `docs/ENGINEERING_PRINCIPLES.md`
7. `docs/ENGINEERING_GATES.md`
8. `docs/ARCHITECTURE.md`
9. `docs/API_CONTRACT.md`
10. `docs/SECURITY.md`
11. relevant ADRs and tests

## Local Docker workflow

The normal developer workflow is:

~~~
docker compose build
docker compose up -d
docker compose exec pkgintel bash
~~~

The developer's intended real OS verification environment is Ubuntu 24.04 running inside Docker Desktop on a Windows 11 host.

When a test requires capabilities that the current container does not have, do not weaken the test. Report the exact commands and environment requirements needed for the Ubuntu 24.04 verification run.

## Engineering standard

All guidance, code changes, reviews, and documentation should be performed at the applicable:

- Staff/Principal Engineer level;
- Principal Security Engineer level;
- CA/Finance level for material cost/business decisions;
- MBA/Management/Product level for scope, delivery, operational, and product decisions.

The repository must remain self-describing. Do not rely on private conversation history to explain branch provenance, architectural intent, security claims, or verification status.

## Git and branch discipline

The current P0 line is:

~~~
main
  -> codex/p0-foundation
  -> codex/p0-module-architecture
~~~

Do not repeat ancestor work. Refresh the current branch SHA before modifying code and report the resulting commit and verification evidence after changes.
