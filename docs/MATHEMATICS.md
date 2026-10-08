# Mathematical Engineering Model

## Purpose

Mathematics is used in pkgintel to prove bounds and reason about resource, memory, and performance behavior. It is not a separate academic exercise.

## 1. Integer-safe allocation

For N objects each requiring S bytes:

```
N * S <= SIZE_MAX
```

must be established before multiplication.

Equivalent implementation guard:

```
N <= SIZE_MAX / S
```

when S is non-zero.

For addition:

```
A + B <= SIZE_MAX
```

must be established before calculating the sum.

## 2. Budget accounting

For a resource with current usage U, requested amount R, and maximum M:

```
U + R <= M
```

must be checked without overflowing U + R.

A safe form is:

```
R <= M - U
```

after establishing U <= M.

This matters for the aggregate snapshot string budget.

## 3. Geometric growth

With initial capacity C and growth factor g > 1:

```
C_k = C g^k
```

For g = 2, total copied elements over a long append sequence is bounded by a geometric series and therefore O(n).

The individual resize remains O(n), but the amortized append cost is O(1).

## 4. Record-size bounds

If a parser accepts at most B bytes per record, one record cannot require more than O(B) parser-buffer storage.

For the v0.1 dpkg backend:

```
B = 65,536 bytes
```

before the record terminator rules described by the parser contract.

This is separate from record-count limits.

## 5. Aggregate memory model

A simplified snapshot memory model is:

```
M_snapshot =
    M_package_arrays
  + M_artifact_arrays
  + M_diagnostic_arrays
  + M_owned_strings
  + M_target_root
  + M_parser_state
  + M_other_runtime_state
```

The current 64 MiB string ceiling constrains only the explicitly accounted package/artifact/diagnostic string storage. It does not claim to bound total process RSS.

That distinction is intentional and must remain visible in documentation.

## 6. Complexity notation

Use:

- O(1) for constant work;
- O(log n) for logarithmic search;
- O(n) for linear work;
- O(n log n) for comparison sorting;
- O(V+E) for graph traversal.

Always state what n, V, E, or B represent.

Worst-case and expected/amortized complexity must not be conflated.

## 7. Mathematical review rule

For every security-sensitive size or count:

1. identify the unit;
2. identify the maximum;
3. identify the arithmetic;
4. prove the arithmetic cannot overflow;
5. test the boundary;
6. document what the bound does **not** cover.

This is the minimum mathematical discipline for resource-safe C.
