# Branch Provenance and Anti-Duplication Record

## Repository

Rofikali/pkgintel

## Current P0 engineering line

~~~
main
  |
  | 50 commits
  v
codex/p0-foundation
  |
  | 193 commits
  v
codex/p0-module-architecture
~~~

The current working line is codex/p0-module-architecture.

The cumulative difference from main is 243 commits at the P0 checkpoint.

## Meaning of the branches

### main

Historical/base line.

Do not use it as the starting point for current P0 work unless the task explicitly concerns the historical baseline.

### codex/p0-foundation

Foundation work.

Its changes are ancestors of the current module-architecture line.

### codex/p0-module-architecture

Current P0 integration line.

It contains the foundation work plus the module architecture, security, API, resource-governance, testing, and documentation work built on top of it.

## Anti-duplication rule

Before implementing a proposed fix:

1. compare the current branch with its parent/ancestor;
2. search commit history for the relevant behavior;
3. inspect the current implementation;
4. inspect its tests and ADRs;
5. only then decide whether new code is necessary.

Never reimplement an ancestor feature because it is not remembered from conversation history.

## Change provenance

For each future significant fix record:

~~~
Finding
  -> current branch
  -> current SHA
  -> introducing commit(s)
  -> current behavior
  -> required correction
  -> new commit
  -> verification
~~~

## Current branch checkpoint

At the documentation checkpoint used to create this record:

~~~
codex/p0-module-architecture
HEAD: 87e632a0570afdd7c0b03d648c0a37216a5d22a5
~~~

The checkpoint commit is:

~~~
docs: link current engineering status
~~~

Future agents must refresh the branch head before modifying files because this SHA can move.

## Why this matters

Branch history is part of engineering context.

Without provenance, teams and agents can:

- duplicate fixes;
- resurrect rejected designs;
- create conflicting implementations;
- misread inherited behavior as new work;
- waste review cycles;
- lose the reason behind security controls.

The repository should therefore be self-describing enough that a new engineer can reconstruct the engineering sequence without private chat history.
