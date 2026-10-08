# Algorithms and Data Structures

## Purpose

Algorithms and data structures are a first-class engineering discipline in pkgintel. We choose them from workload, security, memory, determinism, and maintenance constraints—not from interview-fashion.

## Decision method

```
problem
 -> constraints
 -> candidate algorithms/data structures
 -> mathematical model
 -> complexity
 -> security implications
 -> implementation
 -> tests/properties/fuzzing
 -> benchmark
 -> decision
```

## Current algorithms

### 1. Geometric dynamic arrays

Snapshot package, artifact, and diagnostic collections grow geometrically.

- append: amortized O(1);
- resize: O(n) for copied elements;
- storage: O(n);
- growth must stop at the semantic maximum;
- size multiplication must be checked before allocation.

The design keeps append logic simple and predictable while preserving bounded resource behavior.

### 2. Bounded sequential metadata parsing

The dpkg backend reads metadata records sequentially with a fixed maximum record size.

This is effectively O(B) work for a record of length B and O(1) parser-buffer space with respect to record size, because the parser does not allocate a record-sized buffer.

The bound is a security invariant: input length cannot force unbounded parser materialization.

### 3. Linear package-file correlation

The current v0.1 slice performs bounded, package-owned-file correlation without introducing a general-purpose index structure.

This is intentionally conservative. Before replacing a linear operation with sorting, binary search, or hashing, measure representative package/file counts and lookup frequency.

## Candidate future structures

### Sorting

Sorting can provide deterministic ordering and enable binary search.

Typical comparison:

- sort: O(n log n);
- binary lookup after sorting: O(log n);
- memory: implementation-dependent;
- one-time sort cost may be worthwhile if many lookups follow.

The comparator must define a deterministic, transitive ordering.

### Hash table

Expected lookup can be O(1), but the trade-offs include:

- additional memory;
- hashing CPU cost;
- resizing;
- collision behavior;
- deterministic iteration concerns;
- adversarial-key considerations.

A hash table is not automatically better than linear search for small or bounded collections.

### Filesystem traversal

Future full-tree traversal must explicitly choose DFS or BFS and define:

- explicit stack vs recursion;
- symlink semantics;
- cycle detection;
- mount boundary policy;
- maximum depth;
- file/directory budgets;
- descriptor lifetime;
- error/partial-result behavior.

For a graph with V vertices and E edges, traversal is generally O(V+E). An explicit DFS stack has memory proportional to traversal depth, while BFS may require a frontier proportional to breadth.

The current v0.1 scanner deliberately does not claim full filesystem traversal.

## Algorithm selection rule

An asymptotically superior algorithm can still be the wrong production choice when:

- the input is small;
- the operation is rare;
- constants dominate;
- memory overhead matters;
- determinism becomes harder;
- security analysis becomes substantially more complex.

For pkgintel, **measured end-to-end behavior beats theoretical elegance**.
