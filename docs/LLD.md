# Low-Level Design (LLD)

## Purpose

The LLD turns an HLD decision into implementable contracts: data structures, call flow, ownership, failure semantics, algorithms, and invariants.

## 1. LLD contract template

For a non-trivial component document:

1. Inputs and preconditions
2. Outputs and postconditions
3. Ownership and lifetime
4. State transitions
5. Error taxonomy
6. Resource accounting
7. Algorithm/data structure
8. Integer/size arithmetic
9. Security invariants
10. Complexity
11. Test strategy
12. Benchmark strategy

## 2. Example: snapshot append

Conceptual flow:

```
append record
    |
    +-- validate arguments
    |
    +-- check aggregate record ceiling
    |
    +-- validate size arithmetic
    |
    +-- reserve/grow storage
    |
    +-- construct owned fields
    |
    +-- commit record
    |
    +-- update accounting
```

The committed count must not increase until construction succeeds.

For an array of N elements of size S:

```
N <= SIZE_MAX / S
```

must hold before calculating `N * S`.

For a byte budget:

```
used + required <= limit
```

must be evaluated without allowing `used + required` itself to overflow.

## 3. Example: geometric dynamic array

The current v0.1 record arrays use geometric capacity growth.

If:

```
C_0 = 8
C_{k+1} = 2 C_k
```

then a sequence of appends has amortized O(1) append cost, while an individual resize is O(n). Storage is O(n).

The implementation must cap growth at the semantic maximum and check multiplication before `realloc`.

The algorithm is selected because the current workload requires append-heavy construction and no evidence currently justifies a more complex allocator.

## 4. Failure semantics

A component must distinguish:

- invalid input;
- unsupported operation;
- permission/security rejection;
- I/O failure;
- malformed/corrupt evidence;
- resource exhaustion;
- internal allocation failure.

Where the public contract permits partial results, a failed later operation must not destroy earlier committed observations.

## 5. Ownership example

```
snapshot
  +-- package records
  |     +-- owned name
  |     +-- owned version
  |     +-- owned architecture
  |
  +-- artifact records
  |     +-- owned path
  |
  +-- diagnostic records
        +-- owned code/message
```

Public accessors borrow these values. The snapshot remains the owner.

## 6. LLD review rule

If an implementation cannot state its invariant, ownership, failure behavior, resource unit, and complexity, the design is not finished enough to implement safely.

LLD is therefore a reasoning artifact, not merely a document written after coding.
