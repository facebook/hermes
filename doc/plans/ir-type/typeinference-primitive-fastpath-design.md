# Primitive-Bitmask Fast Path for TypeContext — Design

> **Status: shipped.** The fast path uses a single **DenseMap** mask→Type cache.
> The prototype also evaluated lazy-flat and eager-flat array caches behind a
> build-time `HERMES_PRIMCACHE` selector; the live working set turned out to be
> tiny (~40–60 distinct primitive masks per compile), so the DenseMap won on
> cache locality and the flat variants and selector were removed. See the
> companion results note. Sections below that describe the three-variant cache
> abstraction are retained for historical rationale.

## Context

The IR Type v2 redesign replaced the old 2-byte inline bitmask `Type` with a
4-byte handle (`id_`) into a per-`Module` `TypeContext`. Type operations became
out-of-line calls; union/intersect canonicalization goes through a
`DenseMap<UnionInternKey, …>` whose key is a `SmallVector<uint32_t,8>`.

Measured cost (direct in-source `[TI-TIMER]`, Release, 5-run median; OLD =
pre-Phase-1 bitmask `e91cc037f`, NEW = `static_h` HEAD):

| Bundle | OLD | NEW | regression |
|---|---|---|---|
| Fb4aBundle.js 2019 (40MB) | 4.49s | 5.32s | +18.6% |
| map/bundle-new.js 2023 (78MB) | 1.79s | 2.02s | +12.7% |
| fb4a marketplace 2026 (52MB) | 6.36s | 7.24s | +13.9% |

Profiling (`sample`) attributes the dominant added cost to `unionTy` →
`createUnionImpl` (the `DenseMap`/`SmallVector` interning) and, secondarily,
`isSubsetOf`/`intersectTy`. The hottest per-instruction operations
(`operator==` for fixpoint convergence, `getType`/`setType`) are inline `uint32`
ops and were never the problem.

This design re-embeds the old bitmask as a **fast path** for primitive-only
types so that the common case of `union`/`intersect`/`subset`/`canBe*` becomes
inline bit/table-lookup work with no `DenseMap` access — while keeping the
`Type` handle and the `id_`-equality invariant untouched.

## Goal & Scope

- **Bar (i): measurement-grade prototype** — semantically exact for everything
  TypeInference exercises (so convergence/iteration is identical and timings are
  trustworthy), but we do not port non-hot consumers or polish diagnostics.
- Must support **integer types first-class** (Int32/Uint32/UInt31), because
  integer-producing inference and instructions are planned next. Integers are
  handled in the fast path, not punted to the slow path.
- Out of scope: refined object types (ClassInstance/Array/…) stay on the slow
  path; no changes to `getKind`/`getUnionArms`/`print`/iterators.

## Maskable Set (12-bit mask)

The mask covers exactly the kinds that flow through inference's algebra:

- **9 disjoint bits**: Undefined, Null, Boolean, BigInt, String, Symbol,
  Object, Empty, Uninit.
- **3-bit number-family code** (see lattice below).

Empty/Uninit are included because TDZ inference manipulates them (Phis union
them; `ThrowIf`/`UnionNarrowTrusted` subtract `Empty`).

**Non-maskable** (→ existing slow path; correct but ~never exercised because
these are always-known, never inferred, and never merged with JS values):
Environment, FunctionCode, PrivateName, Bits32, and all refined object kinds.

Packed cache index = `(disjointBits << 3) | numberCode` → up to 12 bits
(`2^12 = 4096`; valid number codes 0..5, so ~3072 reachable combinations).

## Number-Family Lattice

Six elements, all subsets of `Number` (fp64):

```
            Number                       code 5
              |
        Int32 ∪ Uint32                   code 4   (join of Int32, Uint32)
           /     \
        Int32    Uint32                  code 2 / 3
           \     /
          UInt31                         code 1   (Int32 ∩ Uint32, the meet)
              |
              ∅                          code 0
```

Three constexpr 6×6 tables: `join` (union), `meet` (intersect), `sub`
(subtract → tightest representable superset, e.g. `Number − Int32 = Number`).
`subset(a,b)` derivable as `meet(a,b)==a` or a 4th boolean table. Tables only
ever yield the six valid codes ⇒ canonical-by-construction (no post-hoc
canonicalization pass).

## Type → Mask

Add `uint16_t primMask` to `TypeEntry`, sentinel `kNotMaskable = 0xFFFF`. Set at
every entry-creation site:
- leaf maskable kind → its bit (number leaves → the matching number code);
- `NoType` → 0;
- union → if all arms maskable, disjoint = OR of arm disjoint-bits and number =
  `join` of arm number-codes; else `kNotMaskable`;
- refined/internal creators → `kNotMaskable`.

Free to read: rides the `entries_[id]` cache line already loaded for `.kind`.

## Operations

Each hot op gains a top-of-function fast path; if either operand is
`kNotMaskable`, fall through to the existing implementation unchanged.

- **Queries** (`isSubsetOf`, `areDisjoint`, `canBe*`, `isPrimitive`,
  `canBePrimitive`, `isNonPtr`, `isKnownPrimitiveType`): disjoint-bit tests plus
  a number-code table lookup. No cache access, no materialization.
  `isSubsetOf = (da&db)==da && numberSubset(na,nb)`; `canBeNumber = nc != 0`;
  etc.
- **Constructors** (`unionTy`/`intersectTy`/`subtractTy`): compute the result
  mask (disjoint via `OR`/`AND`/`&~`, number via `join`/`meet`/`sub`), then
  `lookupPrimMask(result)`.
- `lookupPrimMask(m)`: consult the cache; on miss, **materialize** — decode the
  mask to leaf ids (number code → `Number` / `Int32` / `Uint32` / `UInt31` /
  the `{Int32,Uint32}` arm pair) and intern via the **existing**
  `createUnionFromLeafArms` — then record in the cache, and set the resulting
  entry's `primMask`.

## Cache Abstraction (prototype)

> Historical: this three-variant comparison was prototype-only. The shipped code
> uses the **DenseMap** variant exclusively; the seam, the `HERMES_PRIMCACHE`
> selector, and the flat/eager variants were removed (the live working set is
> ~40–60 masks, so DenseMap won on locality). See the results note.

The mask→Type cache sat behind a small seam (`PrimMaskCache`) so we could build
three binaries and run the identical harness on each. Selected at build time
(macro or template tag — separate binaries, so no runtime cost):

1. **DenseMap** — `DenseMap<uint32_t, Type>`, lazy. Sparse; single-`uint32` key
   (far cheaper than today's `SmallVector` key).
2. **Lazy flat** — `std::vector<Type>` of size `2^12`, init `kInvalidId`;
   lookup is an index, miss → materialize+store.
3. **Eager flat** — same array, fully populated at `TypeContext` construction
   (materialize all reachable masks up front); lookup is a pure index load,
   never a miss. Construction cost is one-time per Module; report it separately
   from `[TI-TIMER]` (which times only `runOnModule`).

All three funnel materialization through the same interning routine, so they are
behaviorally identical and differ only in lookup cost / construction cost.

## Canonical-Form Invariant & Safety

The `primMask` field and the cache are **pure accelerators in front of
`internTable_`, which remains the single source of canonical truth.** A type is
maskable-or-not deterministically by its kinds; both the fast and slow paths
intern through `internTable_`, so every type has exactly one canonical `id_`
regardless of path. `operator==` (and the fixpoint convergence test) is
therefore unaffected.

## Correctness Gate

The fast path is a pure optimization. Validation:
1. Emitted `.hbc` must be **byte-identical** to baseline-NEW for all three
   bundles (`cmp` the outputs).
2. `[TI-COUNTERS]` amplification must be **unchanged** vs baseline-NEW (proves
   per-op cost changed, not convergence behavior).
3. Existing `TypeContextTest` unit tests pass.

## Measurement Plan (prototype)

> Historical: how the three variants were compared. Final shipped numbers are in
> the results note (taken with `-ftime-report`).

`cp` the current NEW `hermesc` aside (preserve baseline binary). Build the three
cache variants. For each variant, run the `[TI-TIMER]` harness (5 runs, median)
on the three bundles; compare to baseline-NEW (5.32 / 2.02 / 7.24 s) and the OLD
floor (4.49 / 1.79 / 6.36 s). Also record eager-flat construction overhead.
Report a table: {OLD, NEW-baseline, NEW+DenseMap, NEW+lazy-flat,
NEW+eager-flat} × {three bundles}.

## Files (as shipped)

- `include/hermes/IR/TypeContext.h`: `TypeEntry::primMask` + the `kNotMaskable`
  sentinel; mask bit / number-code constants; the three number-family
  `inline constexpr` tables (join/meet/subtract); `leafKindToMask`;
  `kNumPrimMasks`; the `llvh::DenseMap<uint32_t, Type> primCache_` member; the
  `lookupPrimMask`/`materializePrimMask` declarations.
- `lib/IR/TypeContext.cpp`: the both-operands-maskable fast paths in the hot ops
  (union/intersect/subtract, isSubsetOf/areDisjoint, the canBe* queries, the
  isPrimitive/canBePrimitive/isNonPtr/isKnownPrimitiveType predicates);
  `lookupPrimMask` (DenseMap find/insert) + `materializePrimMask`; `primMask`
  population in `TypeEntry::createLeaf` and `TypeContext::addUnionEntry`.
- `unittests/IR/TypeContextTest.cpp`: primMask-value, materialization
  round-trip, and per-op fast/slow equivalence tests.

There is no `PrimMaskCache` seam, `HERMES_PRIMCACHE` selector, or eager
population path in the shipped code — those were prototype-only; see the
"Cache Abstraction (prototype)" section below for the historical rationale.

The prototype also used a throwaway measurement scaffold (`[TI-TIMER]` /
`[TI-COUNTERS]` in `lib/Optimizer/Scalar/TypeInference.cpp`); it was reverted
and is not part of the shipped change. Final numbers were taken with the
built-in `-ftime-report` instead (see the results note).

## Deferred (post-prototype, if the win justifies productionization)

- Port non-hot consumers / printing if we later embed the mask in the handle.
- Number-family extensions beyond the 6-element lattice (if future refinements
  add elements, the tables grow accordingly).
- The worklist / no-clear-rerun restructuring of TypeInference itself (the
  larger, orthogonal lever — separate effort).
