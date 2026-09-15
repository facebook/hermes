# Phase 1 Implementation Steps

Phase 1 replaces the 2-byte bitmask `Type` with a 4-byte opaque index into
`TypeContext`, maintaining API compatibility for the vast majority of
call sites. A small number of call sites require targeted changes — see
"Known Call-Site Migrations" below.

References:
- [ir-type-v2-implementation.md](ir-type-v2-implementation.md) — full design
- [ir-type-system.md](ir-type-system.md) — current bitmask system

## Performance Note

Phase 1 replaces inline constexpr bitwise ops (single OR/AND/NOT) with
thread-local lookup → table lookup → (for unions) arm iteration. This is
inherently slower. For Phase 1, only well-known types exist, so the
well-known ID fast paths in `canBeX()` (`if (id == kAnyTypeId) return
true`) keep the common case to a few integer comparisons — slower than a
bitmask AND, but not drastically so.

If profiling reveals a bottleneck, a bitmask cache can be layered on
without changing the type algebra: each `TypeEntry` stores a precomputed
`uint16_t kindMask`, and `canBeNumber(t)` becomes
`entries_[t].kindMask & NumberBit`. See implementation design §12. This
optimization is explicitly deferred until profiling on real workloads.

## Dependency Graph

```
P1-S1 (skeleton)
 ├──→ P1-S2 (queries) ──→ P1-S3 (operations) ──→ P1-S4 (utilities)
 ├──→ P1-S5 (thread-local)
 └──→ P1-S6 (wire into Module)
                    P1-S5 + P1-S6 ──→ P1-S7 (RAII at entry points)
                           P1-S7 ──→ P1-S7.5 (RAII in tests)
                    P1-S4 + P1-S7.5 ──→ P1-S8 (rewrite Type)
```

Commits are linear; the order below respects all dependencies.

---

## Known Call-Site Migrations

These call sites cannot be preserved as-is and require targeted changes
in P1-S8:

1. **`constexpr Type` via `unionTy`**: `lib/Optimizer/Scalar/InstSimplify.cpp:41`
   defines `constexpr Type kNullOrUndef = Type::unionTy(...)`. After the
   rewrite, `unionTy` delegates to the thread-local context and cannot be
   `constexpr`. Fix: add `kNullOrUndefId` to the well-known
   pre-allocated types in P1-S1 and replace the `constexpr` expression
   with `Type::createNullOrUndef()`.

2. **`switch` on `Type::` enum values**: `lib/BCGen/SH/SH.cpp:2279–2307`
   switches on `type.getFirstTypeKind()` using `case Type::String`,
   `case Type::LAST_TYPE`, etc. These reference the old nested `TypeKind`
   enum. Fix: change to `case TypeKind::String`, etc. Remove the
   `LAST_TYPE` case (add a `default:` or exhaust all cases).

3. **`Type::iterator` yields `Type`, not `TypeKind`**:
   `lib/BCGen/SH/SH.cpp:2332` does `for (Type t : checkTypes)`. The
   current iterator dereferences to a single-kind `Type` object (e.g.,
   `Type(1 << index_)`). The new iterator must preserve this contract:
   dereference yields a `Type` (with a single-kind well-known ID), not a
   `TypeKind`.

4. **`Type::LAST_TYPE` sentinel**: Used in `SH.cpp:2305`, `IR.h`, and
   `IR.cpp:1054`. Removed in the new enum. Each usage needs a case-by-case
   fix (typically replace with `default:` or remove).

---

## P1-S1: TypeContext skeleton with well-known types

**Short description:** Create `TypeContext` with `TypeKind` enum,
`TypeEntry` struct, well-known ID constants, and constructor that
pre-allocates all well-known type entries (primitives and common unions).

**Dependencies:** None.

**Details:**

New files:
- `include/hermes/IR/TypeContext.h`
- `lib/IR/TypeContext.cpp`

Add `IR/TypeContext.cpp` to the `hermesFrontend` source list in
`lib/CMakeLists.txt`.

Define:
- `enum class TypeKind : uint8_t` — all kinds from the design (leaf kinds,
  refined kinds, Union). Refined kinds (ClassInstance, Array, Tuple, Function,
  ExactObject, Int32, Uint32) are defined but unused in Phase 1.
- `struct TypeEntry` — tagged union with kind-specific payloads.
- `struct FunctionParam`, `struct ExactObjectField` — side array element
  types (defined but unused in Phase 1).
- Well-known ID constants (`kNoTypeId` through `kFirstDynamicId`).
- `class TypeContext`:
  - `std::vector<TypeEntry> entries_` — type table.
  - `std::vector<uint32_t> typeArrays_` — stores union arm IDs (using raw
    `uint32_t` since `Type` is still a bitmask at this point; changed to
    `Type` in P1-S8).
  - Constructor pre-allocates all well-known entries: leaf primitives
    (NoType through Bits32) and well-known unions (AnyType, Numeric,
    AnyEmptyUninit, NullOrUndef). Asserts
    `entries_.size() == kFirstDynamicId` at end.
  - `TypeKind getKind(uint32_t id) const` — returns the kind of a type
    entry.
  - `llvh::ArrayRef<uint32_t> getUnionArms(uint32_t id) const` — returns
    the arm IDs of a union entry.

All public methods take `uint32_t` IDs (not `Type`). Changed to `Type` in
P1-S8 when the `Type` class is rewritten.

**Completion criteria:**
- Builds successfully.
- New unit test file `unittests/IR/TypeContextTest.cpp` (added to
  `unittests/IR/CMakeLists.txt`):
  - Constructs an `TypeContext`.
  - Verifies `getKind(kNumberId) == TypeKind::Number` (and other
    primitives).
  - Verifies `getKind(kAnyTypeId) == TypeKind::Union`.
  - Verifies `getUnionArms(kAnyTypeId)` contains the expected primitive
    IDs.
  - Verifies `getUnionArms(kNumericId)` = {kNumberId, kBigIntId}.

---

## P1-S2: Type queries on TypeContext

**Short description:** Add `canBeX()`, `isPrimitive()`, `isNonPtr()`, and
related predicate methods to `TypeContext`.

**Dependencies:** P1-S1.

**Details:**

Add methods to `TypeContext` (all take `uint32_t id`):
- `canBeNumber`, `canBeString`, `canBeObject`, `canBeNull`,
  `canBeUndefined`, `canBeEmpty`, `canBeUninit`, `canBeBigInt`,
  `canBeBoolean`, `canBeSymbol`.
- `isNoType`, `isPrimitive`, `canBePrimitive`, `isNonPtr`.

Each method implements well-known ID fast paths (e.g., `canBeNumber` returns
true immediately for `kAnyTypeId`, `kNumberId`, `kNumericId`) before falling
back to kind-based checks. For unions, iterate arms and check recursively
(arms are never unions, so recursion depth is 1).

**Completion criteria:**
- Builds successfully.
- Unit tests in `TypeContextTest.cpp`:
  - `canBeNumber(kNumberId)` is true.
  - `canBeNumber(kStringId)` is false.
  - `canBeNumber(kAnyTypeId)` is true (AnyType is a union containing
    Number).
  - `canBeNumber(kNumericId)` is true.
  - `isPrimitive(kNumberId)` is true.
  - `isPrimitive(kObjectId)` is false (Object is NOT in `PRIMITIVE_BITS`;
    primitives are: Number, String, BigInt, Null, Undefined, Boolean,
    Symbol).
  - `isNonPtr(kNumberId)` is true.
  - `isNonPtr(kStringId)` is false.
  - Queries on well-known union types return correct results.

---

## P1-S3: Type operations with interning

**Short description:** Add `unionTy`, `intersectTy`, `subtractTy`,
`isSubsetOf`, and `areDisjoint` to `TypeContext`. Add the intern table
for deduplicating dynamically created union types.

**Dependencies:** P1-S2.

**Details:**

Add to `TypeContext`:
- `llvh::DenseMap<TypeEntryKey, uint32_t> internTable_` — maps structural
  keys to type IDs. Pre-populated for well-known types in the constructor.
- `struct TypeEntryKey` — hashable/comparable key derived from a
  `TypeEntry`'s contents. For unions, the key includes the sorted arm IDs.
- `bool isSubsetOf(uint32_t a, uint32_t b)` — full implementation per
  design: union-on-left (all arms subset), union-on-right (some arm is
  superset), base cases.
- `bool areDisjoint(uint32_t a, uint32_t b)` — kind-based disjointness
  without materializing intersections.
- `uint32_t unionTy(uint32_t a, uint32_t b)` — identity shortcuts, subset
  shortcuts, then `createUnionImpl` with full canonicalization (flatten,
  dedup, subsume, sort by ID, collapse single-arm, intern).
- `uint32_t intersectTy(uint32_t a, uint32_t b)` — subset shortcuts,
  distribute over unions.
- `uint32_t subtractTy(uint32_t a, uint32_t b)` — subset → NoType,
  disjoint → A, distribute over unions in A.

In Phase 1, the only dynamically created types are unions of well-known
primitives. Refined-type cases (ClassInstance subtyping, Array invariance,
etc.) are dead code paths that return conservative results or assert.

**Completion criteria:**
- Builds successfully.
- Unit tests in `TypeContextTest.cpp`:
  - `unionTy(kNumberId, kStringId)` returns a union; querying its arms
    gives {kNumberId, kStringId} (sorted).
  - `unionTy(kNumberId, kNumberId)` returns `kNumberId` (identity).
  - `unionTy(kNoTypeId, kStringId)` returns `kStringId`.
  - `unionTy(kNumberId, kAnyTypeId)` returns `kAnyTypeId` (subset
    subsumed).
  - Interning: two calls to `unionTy(kNumberId, kStringId)` return the
    same ID.
  - `intersectTy(kNumberId, kStringId)` returns `kNoTypeId` (disjoint).
  - `intersectTy(kNumberId, kAnyTypeId)` returns `kNumberId`.
  - `subtractTy(kAnyTypeId, kNumberId)` returns a union without Number.
  - `isSubsetOf(kNumberId, kAnyTypeId)` is true.
  - `isSubsetOf(kAnyTypeId, kNumberId)` is false.
  - `isSubsetOf(kNoTypeId, kNumberId)` is true.
  - `areDisjoint(kNumberId, kStringId)` is true.
  - `areDisjoint(kNumberId, kNumericId)` is false.

---

## P1-S4: Utility methods

**Short description:** Add `countKinds`, type formatting, and union arm
iteration helpers to `TypeContext`.

**Dependencies:** P1-S3.

**Details:**

Add to `TypeContext`:
- `unsigned countKinds(uint32_t id)` — returns 0 for NoType, arm count
  for Union, 1 otherwise. Replacement for `Type::countTypes()`.
- `TypeKind getFirstKind(uint32_t id)` — returns the TypeKind for
  non-union types; for unions, returns the kind of the first arm.
  Replacement for `Type::getFirstTypeKind()`.
- `void format(llvh::raw_ostream &OS, uint32_t id) const` — prints the
  human-readable type name. Leaf kinds print their name (`number`,
  `string`, etc.). Unions print pipe-separated arms (`number|string`).
  Special cases: NoType → `notype`, AnyType → `any`.
  Replacement for `Type::print()`.

**Completion criteria:**
- Builds successfully.
- Unit tests:
  - `countKinds(kNoTypeId)` returns 0.
  - `countKinds(kNumberId)` returns 1.
  - `countKinds(kAnyTypeId)` returns 8 (all JS-observable types).
  - `countKinds(unionTy(kNumberId, kStringId))` returns 2.
  - `getFirstKind(kNumberId)` returns `TypeKind::Number`.
  - `format` output matches current `Type::print()` output for all
    well-known types and simple unions.

---

## P1-S5: Thread-local context and RAII guard

**Short description:** Add `TypeContext::current()` thread-local accessor
and `TypeContextRAII` RAII guard.

**Dependencies:** P1-S1.

**Details:**

Add to `TypeContext`:
- `static thread_local TypeContext *current_` — private.
- `static TypeContext &current()` — asserts `current_ != nullptr`,
  returns `*current_`.

New class `TypeContextRAII`:
- Constructor takes `TypeContext &`, saves `current_`, sets new current.
- Destructor restores previous `current_`.
- Non-copyable, non-movable.

**Completion criteria:**
- Builds successfully.
- Unit tests:
  - Install guard with context A: `TypeContext::current()` returns A.
  - Nested guards: inner guard returns inner context, after destruction
    outer context is restored.

---

## P1-S6: Wire TypeContext into Module

**Short description:** Add `TypeContext` member to `Module` and the
necessary header includes.

**Dependencies:** P1-S1.

**Details:**

In `include/hermes/IR/IR.h`:
- Forward-declare `class TypeContext;` near the top (before `Type`).
- Include `hermes/IR/TypeContext.h` after the `Type` class definition
  and before the `Module` class definition.
- Add `TypeContext typeContext_;` private member to `Module`.
- Add `TypeContext &getTypeContext() { return typeContext_; }` public
  accessor.

The `TypeContext` constructor runs automatically when `Module` is
constructed. No semantic change — nothing uses `typeContext_` yet.

**Completion criteria:**
- Builds successfully (full build including all tests, no test changes
  needed).

---

## P1-S7: Install RAII guards at compilation entry points

**Short description:** Install `TypeContextRAII` at all known `Module`
creation sites so the thread-local context is set during compilation.

**Dependencies:** P1-S5, P1-S6.

**Details:**

Install `TypeContextRAII` immediately after `Module` creation in every
site that creates a `Module` and performs type operations on it:
1. `lib/BCGen/HBC/BCProviderFromSrc.cpp` — in `BCProviderFromSrc::create`,
   after the `std::make_shared<Module>(context)` call (~line 233).
2. `lib/CompilerDriver/CompilerDriver.cpp` — after the
   `std::make_shared<Module>(context)` call (~line 1983).
3. `tools/shermes/shermes.cpp` — after the stack-allocated
   `Module M(context)` (~line 911).

Any additional `Module` creation sites discovered during implementation
(e.g., in tests or tools) must also install the guard. A grep for
`Module(` and `make_shared<Module>` across the tree should catch them all.

The guard must be in a scope that encompasses all subsequent type operations
on the Module (IR construction, optimization passes, codegen). In all
cases, the existing scope after Module creation covers this.

No semantic change — `TypeContext::current()` is now set during
compilation, but nothing calls it yet.

**Completion criteria:**
- Builds successfully (full build including all tests, no test changes
  needed).

---

## P1-S7.5: Add RAII guards to unit tests (preparation)

**Short description:** Add `TypeContext` and `TypeContextRAII` setup
to all test fixtures that use `Type` operations, in preparation for the
Type rewrite. While `Type` is still bitmask-based, the guards are no-ops.

**Dependencies:** P1-S7.

**Details:**

Grep for `Type::` across all test files to find every test that uses
`Type` operations. For each, add an `TypeContext` instance and
`TypeContextRAII` guard to the test fixture (constructor or `SetUp()`).

Known test files:
- `unittests/IR/BuilderTest.cpp` — uses `Type::unionTy()`, `canBeX()`,
  `isAnyEmptyUninitType()`.
- `unittests/IR/VariableTest.cpp` — uses `Type::createAnyType()`.
- `unittests/IR/IRUtilsTest.cpp` — uses `Type::createAnyType()`.
- `unittests/IR/IRVerifierTest.cpp` — uses `Type::createNumber()`,
  `Type::createString()`.

Additional test files may be discovered by the grep.

No semantic change — `Type` still uses the bitmask; the guards prepare
for P1-S8 so that the Type rewrite doesn't need to touch test files.

**Completion criteria:**
- All tests pass unchanged (guards are no-ops).

---

## P1-S8: Rewrite Type class

**Short description:** Replace `Type`'s `uint16_t bitmask_` with
`uint32_t id_`, delegate all methods to `TypeContext::current()`, and
replace the `TypeKind` enum.

**Dependencies:** P1-S4, P1-S7.5 (all prior steps).

**Details:**

This is the largest step. It is atomic because changing the representation
breaks all method implementations simultaneously.

**In `include/hermes/IR/IR.h`:**

Remove:
- Old `enum TypeKind { Empty, ..., LAST_TYPE }` (lines 61-86).
- Old `Type` class body (lines 58-414).
- Old `Type::iterator` class (lines 419-460).

Replace with new `Type` class:
- `uint32_t id_` private member.
- `constexpr explicit Type(uint32_t id)` private constructor.
- Static well-known constructors (`createNoType()`, `createNumber()`, etc.)
  return well-known IDs. These remain `static constexpr`.
- `createInt32()` and `createUint32()` are aliases for `createNumber()`.
- `createNullOrUndef()` returns `kNullOrUndefId` (for InstSimplify.cpp
  migration).
- `createNumeric()` returns `kNumericId`. All well-known union
  constructors return pre-allocated IDs directly — they must NOT call
  `unionTy()`, which would require the thread-local context.
- `operator==`, `operator!=` compare `id_` values.
- `hash()` returns `llvh::hash_value(id_)`.
- `Profile()` uses `ID.AddInteger(id_)`.
- All type operations (`unionTy`, `intersectTy`, `subtractTy`) delegate
  to `TypeContext::current()`.
- All type queries (`canBeNumber()`, `isPrimitive()`, etc.) delegate to
  `TypeContext::current()`.
- `isNoType()` and `isAnyType()` can compare directly against well-known
  IDs without context lookup.
- `countTypes()` delegates to `TypeContext::current().countKinds()`.
- `getFirstTypeKind()` delegates to
  `TypeContext::current().getFirstKind()`.
- `print()` delegates to `TypeContext::current().format()`.
- `Type::iterator` reimplemented: dereference yields a single-kind `Type`
  (not `TypeKind`) to preserve the existing API contract. For non-union
  types, yields the type itself as the single element. For unions,
  iterates arms via `TypeContext::current().getUnionArms()`, wrapping
  each arm ID as a `Type`.

**In `TypeContext.h` / `TypeContext.cpp`:**

Change all public method signatures from `uint32_t` to `Type`. Internally
extract `id_` via friend access. This is mechanical.

Change `typeArrays_` from `std::vector<uint32_t>` to `std::vector<Type>`.

**Header layering:** `Type` methods that call `TypeContext::current()`
are defined out-of-line in `IR.cpp` (not inline in `IR.h`). This avoids
include-order coupling between `IR.h` and `TypeContext.h`. Only methods
that don't need the context (e.g., `isNoType()`, `operator==`, static
well-known constructors) remain inline/constexpr in `IR.h`.

**In `lib/IR/IR.cpp`:**

Remove old `Type::print()` implementation (~lines 1040-1080). Replace with
delegation to `TypeContext::current().format()`.

**Old `TypeKind` callers:**

The old `TypeKind` enum (plain `enum` in `hermes` namespace) is replaced by
the new `enum class TypeKind` in `TypeContext.h`. Callers that reference
`TypeKind::Number` etc. continue to work (same qualified names). Callers
that relied on implicit integer conversion (e.g., `1 << TypeKind::Number`
for bitmask construction) no longer exist — the bitmask is gone.

The `LAST_TYPE` sentinel is removed. Callers using it need case-by-case
fixes.

**Note:** Unit test RAII guards are already in place from P1-S7.5.
P1-S8 should not need to modify test files (other than updating
`getUnionArms()` return type from `ArrayRef<uint32_t>` to
`ArrayRef<Type>` in `TypeContextTest.cpp`).

**Note:** `IR.h:707` has a pre-edited comment saying "Type is 4 bytes"
while `sizeof(Type)` is currently 2. This becomes correct after P1-S8;
update the `static_assert(sizeof(Type) == 2)` at `IR.h:416` to
`static_assert(sizeof(Type) == 4)`.

**API parity checklist — complete list of public `Type` methods that
must be preserved with identical semantics:**

Static constructors (all remain `static constexpr`):
- `createNoType()`, `createAnyType()`, `createAnyEmptyUninit()`.
- `createEmpty()`, `createUninit()`, `createUndefined()`, `createNull()`,
  `createBoolean()`, `createString()`, `createNumber()`, `createBigInt()`,
  `createSymbol()`, `createObject()`.
- `createInt32()`, `createUint32()` (aliases for `createNumber()`).
- `createNumeric()`, `createEnvironment()`, `createPrivateName()`,
  `createFunctionCode()`.
- `createNullOrUndef()` (new, for InstSimplify.cpp migration).

Static operations (delegate to `TypeContext::current()`):
- `unionTy(Type, Type)`, `intersectTy(Type, Type)`,
  `subtractTy(Type, Type)`.

Instance predicates (delegate to `TypeContext::current()`):
- `isSubsetOf(Type)`, `canBeType(Type)`, `isProperSubsetOf(Type)`.
- `canBeString()`, `canBeBigInt()`, `canBeSymbol()`, `canBeNumber()`,
  `canBeObject()`, `canBeBoolean()`, `canBeEmpty()`, `canBeUninit()`,
  `canBeUndefined()`, `canBeNull()`, `canBeAny()`.
- `isPrimitive()`, `canBePrimitive()`, `isNonPtr()`,
  `isKnownPrimitiveType()`.

Instance type checks (compare against well-known IDs, no context needed):
- `isNoType()`, `isAnyType()`, `isAnyEmptyUninitType()`.
- `isEmptyType()`, `isUninitType()`, `isUndefinedType()`, `isNullType()`,
  `isBooleanType()`, `isStringType()`, `isObjectType()`, `isNumberType()`,
  `isBigIntType()`, `isSymbolType()`, `isEnvironmentType()`,
  `isPrivateNameType()`, `isFunctionCodeType()`.

Utilities (delegate to `TypeContext::current()`):
- `countTypes()`, `getFirstTypeKind()`.
- `print(raw_ostream &)`.
- `Type::iterator` (dereferences to `Type`).

Identity / hashing (no context needed):
- `operator==`, `operator!=`, `hash()`, `Profile()`.

**Completion criteria:**
- Full build succeeds.
- `check-hermes` passes — all existing tests produce identical results.
- Call-site changes limited to: `Type`, `TypeContext`, `IR.cpp`, unit
  test fixtures, and the known migrations listed in "Known Call-Site
  Migrations" above (InstSimplify.cpp, SH.cpp, and any other
  `LAST_TYPE` / nested-enum users).
