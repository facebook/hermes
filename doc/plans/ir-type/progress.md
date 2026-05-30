# Implementation Progress

Tracks progress of `doc/plans/ir-type/phase1-steps.md` for
`doc/plans/ir-type/ir-type-system-v2-design.md` and
`doc/plans/ir-type/ir-type-v2-implementation.md`.

The file has two sections: "Status" and "Context Notes".

**Status Section**:

Each row contains the step label from the detailed, plan, a brief description, list of
dependency labels, Status (initially empty), optional brief note (initially empty).

The status of a row is one of:
- "" (empty) initially, before work has started
- "wip" as soon as work on that raw has started.
- "done" when work has completed successfully. Rarely "Brief Note" may contain very brief
explanation. More details in "Context Notes".
- "blocked" when work cannot proceed for some reason. "Brief Note" must contain a brief
explanation. More details in "Context Notes".

Example on start:

| Step 11 | Port util binding | 5 |  |  |

**Context Notes**:

After completing work on a step, either successfully or by blocking, a section for that
step is added. It needs to have the format from the following example (empty bullets can
be omitted):

```
### Step 11: Port util binding
- **Files**: created `foo.c`, modified `bar.c`.
- **Decisions**:
-- Decision 1 concise explanation
-- Decision 2 concise explanation
- **What was done**: ...
- **Issues**: ...
- **Notes for next step**: ...
```

## Status

| Step | Description | Depends On | Status | Brief Note (optional) |
|------|-------------|------------|--------|-----------------------|
| P1-S1 | TypeContext skeleton with well-known types | — | done | |
| P1-S2 | Type queries on TypeContext | P1-S1 | done | |
| P1-S3 | Type operations with interning | P1-S2 | done | |
| P1-S4 | Utility methods | P1-S3 | done | |
| P1-S5 | Thread-local context and RAII guard | P1-S1 | done | |
| P1-S6 | Wire TypeContext into Module | P1-S1 | done | |
| P1-S7 | Install RAII guards at compilation entry points | P1-S5, P1-S6 | done | |
| P1-S7.5 | Add RAII guards to unit tests | P1-S7 | done | |
| P1-S8 | Rewrite Type class | P1-S4, P1-S7.5 | done | |
| P2-S1 | Foundation: accessors and conventions | P1-S8 | done | |
| P2-S2 | Migrate IRVerifier to explicit context | P2-S1 |  | |
| P2-S3 | Migrate Instrs.{h,cpp} and IR.cpp | P2-S1 |  | |
| P2-S4 | Migrate IRGen (ESTreeIRGen-*.cpp) | P2-S1 |  | |
| P2-S5 | Migrate InstSimplify pass | P2-S1 |  | |
| P2-S6 | Migrate TypeInference pass (heaviest) | P2-S1 |  | |
| P2-S7 | Migrate remaining optimizer + BCGen | P2-S1 |  | |
| P2-S8 | Migrate test fixtures | P2-S1 |  | |
| P2-S9 | Remove TLS infrastructure | P2-S2..P2-S8 |  | |

## Context Notes

### P2-S1: Foundation — accessors and conventions
- **Files**: modified `include/hermes/IR/IRBuilder.h`, `lib/IRGen/ESTreeIRGen.h`, `include/hermes/IR/TypeContext.h`, `lib/IR/TypeContext.cpp`, `unittests/IR/TypeContextTest.cpp`.
- **What was done**: Added `IRBuilder::getTypeContext()` and `ESTreeIRGen::getTypeContext()` accessors. Renamed `TypeContext::format` → `TypeContext::print` (matches `Type::print`). Added missing operations on TypeContext to reach API parity with TLS-using `Type` methods: `canBeAny`, `canBeType`, `isProperSubsetOf`, `isKnownPrimitiveType`, plus the `arms()` range adapter (returns `ArmRange` over a Type — yields the type itself for non-unions, each arm for unions, nothing for NoType). 4 new unit tests (42 total `TypeContextTest` cases). Phase 2 row added to status table for tracking.
- **Decisions**:
  - `ArmRange` and its iterator store the raw `uint32_t typeId_` instead of `Type` because `Type` is forward-declared in `TypeContext.h` (definition is in `IR.h`, which includes `TypeContext.h`). Methods that touch `Type` (`arms()`, `iterator::operator*`, `ArmRange::end`) are out-of-line in `TypeContext.cpp` where `Type` is complete. Same pattern as the existing `getUnionArms`.
  - The new helpers (`canBeAny`, `canBeType`, etc.) are also out-of-line for the same incomplete-type reason. Once Phase 2 is complete and Type loses its TLS-using methods, the layering can be revisited but it's not necessary.
  - `print` chosen over `format` for naming consistency with `Type::print` and `llvh::raw_ostream` conventions; `format` would have implied "build a string".
  - No callers migrated yet — the TLS-using methods on `Type` remain intact so the build stays green throughout P2-S2…P2-S8.

### P1-S2: Type queries on TypeContext
- **Files**: modified `include/hermes/IR/TypeContext.h`, `lib/IR/TypeContext.cpp`, `unittests/IR/TypeContextTest.cpp`.
- **What was done**: Added `canBeNumber`, `canBeString`, `canBeObject`, `canBeNull`, `canBeUndefined`, `canBeEmpty`, `canBeUninit`, `canBeBigInt`, `canBeBoolean`, `canBeSymbol`, `isNoType`, `isPrimitive`, `canBePrimitive`, `isNonPtr`. Private helpers `containsMatchingKind` and `allMatchKind` take predicate functions for leaf/union dispatch. File-scoped kind helpers (`isNumberKind`, `isObjectKind`, `isPrimitiveKind`, `isNonPtrKind`) encode subtype relationships. 7 new unit tests (13 total).
- **Decisions**:
  - Well-known ID fast paths in each `canBeX` method, per the plan spec. Avoids table lookups for common types.
  - Kind helpers are subtype-aware: `isNumberKind` includes Int32/Uint32, `isObjectKind` includes ClassInstance/Array/Tuple/Function/ExactObject, `isPrimitiveKind` and `isNonPtrKind` include number subtypes. Refined kinds are dead code in Phase 1 but the predicates are correct when they activate.
  - `isPrimitive`/`isNonPtr` return false for NoType, matching the old bitmask semantics (`bitmask_ && !(bitmask_ & ~BITS)`).

### P1-S3: Type operations with interning
- **Files**: modified `include/hermes/IR/TypeContext.h`, `lib/IR/TypeContext.cpp`, `unittests/IR/TypeContextTest.cpp`.
- **What was done**: Added `isSubsetOf`, `areDisjoint`, `unionTy`, `intersectTy`, `subtractTy` public methods. Added `UnionInternKey`/`UnionInternKeyInfo` structs for DenseMap-based union interning. Private `createUnionImpl` handles full canonicalization (flatten, sort, dedup, subsume, intern). Static helpers `isLeafSubtype` and `areLeafKindsDisjoint` handle leaf-level type relationships including future-proof Number/Object family rules. Intern table pre-populated with well-known unions in constructor. 10 new unit tests (23 total TypeContext tests).
- **Decisions**:
  - Used `DenseMap<UnionInternKey, uint32_t, UnionInternKeyInfo>` with SmallVector<uint32_t, 8> key and custom DenseMapInfo (sentinels use size-1 vectors with UINT32_MAX/UINT32_MAX-1, real keys always size >= 2).
  - `isLeafSubtype` handles Int32/Uint32 <: Number and object refinements <: Object even though unused in Phase 1 — keeps algorithms correct when refined types arrive.
  - `areLeafKindsDisjoint` uses the subtype check plus Number-family overlap rule; all other distinct kinds are disjoint.
  - `subtractTy` returns conservative approximation (returns `a` unchanged) when a is a leaf that's not subset of b and not disjoint from b.
  - `intersectTy` distributes over unions recursively; base case for two non-subset number-family leafs returns `kUInt31Id` (the only such overlap); other non-subset leafs return NoType.
  - Added `TypeKind::UInt31` (Int32 ∩ Uint32, integers in [0, 2^31-1]) to close the lattice gap. UInt31 <: Int32, UInt31 <: Uint32, UInt31 <: Number. Pre-allocated as well-known IDs: kInt32Id=15, kUint32Id=16, kUInt31Id=17 (shifted union IDs up by 3). 5 new unit tests (28 total).

### P1-S4: Utility methods
- **Files**: modified `include/hermes/IR/TypeContext.h`, `lib/IR/TypeContext.cpp`, `unittests/IR/TypeContextTest.cpp`.
- **What was done**: Added `countKinds`, `getFirstKind`, `format` methods. Added `kindName` file-scoped helper for TypeKind→string mapping. Forward-declared `llvh::raw_ostream` in header. 5 new unit tests (28 total).
- **Decisions**:
  - `format` uses "any" shorthand when the union is a superset of AnyType (matching old `Type::print()`). Extra arms beyond AnyType (e.g. empty, uninit) are appended with `|`.
  - `kindName` covers all TypeKind values including deferred refined types and Bits32, for completeness.
  - `getFirstKind` returns `TypeKind::NoType` for NoType (matching old behavior where `getFirstTypeKind` returns `LAST_TYPE` for empty bitmask — callers check `isNoType()` first).

### P1-S5: Thread-local context and RAII guard
- **Files**: modified `include/hermes/IR/TypeContext.h`, `lib/IR/TypeContext.cpp`, `unittests/IR/TypeContextTest.cpp`.
- **What was done**: Added `static thread_local TypeContext *current_` and `static TypeContext &current()` to `TypeContext`. Added `TypeContextRAII` class with save/restore semantics. 2 new unit tests (35 total).
- **Decisions**:
  - `TypeContextRAII` is a friend of `TypeContext` to access `current_` directly, keeping the static member private.
  - `current()` returns a reference (not pointer) and asserts non-null — callers never need to check.

### P1-S1: TypeContext skeleton with well-known types
- **Files**: created `include/hermes/IR/TypeContext.h`, `lib/IR/TypeContext.cpp`, `unittests/IR/TypeContextTest.cpp`; modified `lib/CMakeLists.txt`, `unittests/IR/CMakeLists.txt`.
- **Decisions**:
  - TypeEntry uses raw `uint32_t` for all type references (not `Type`) since `Type` is still a bitmask. Changed to `Type` in P1-S8.
  - Added `kNullOrUndefId` (id=18) as a well-known union, needed for InstSimplify.cpp migration in P1-S8.
  - Reserved IDs 19-31 padded with NoType placeholders.
  - Union arm arrays stored in sorted order by ID for AnyType and AnyEmptyUninit.
- **What was done**: Created TypeContext with TypeKind enum (all kinds including deferred refined types), TypeEntry tagged-union struct, well-known ID constants, constructor that pre-allocates 15 leaf types and 4 well-known unions, getKind() and getUnionArms() accessors. Added to hermesFrontend build target. 6 unit tests.

### P1-S6: Wire TypeContext into Module
- **Files**: modified `include/hermes/IR/IR.h`.
- **What was done**: Added `#include "hermes/IR/TypeContext.h"` to IR.h. Added `TypeContext typeContext_` private member and `getTypeContext()` public accessor to `Module`. The context is default-constructed when `Module` is constructed. No semantic change — nothing uses `typeContext_` yet.
- **Decisions**:
  - Included `TypeContext.h` at the top with other hermes includes (alphabetically after `hermes/FrontEndDefs/Typeof.h`) rather than mid-file, since TypeContext.h has no dependency on `Type` or other IR.h definitions.

### P1-S7: Install RAII guards at compilation entry points
- **Files**: modified `lib/BCGen/HBC/BCProviderFromSrc.cpp`, `lib/CompilerDriver/CompilerDriver.cpp`, `tools/shermes/shermes.cpp`.
- **What was done**: Added `TypeContextRAII typeContextGuard(M->getTypeContext())` (or `M.getTypeContext()` for stack-allocated Module) immediately after Module creation in all three production compilation entry points. No new includes needed — all files already include `IR.h` which includes `TypeContext.h`.
- **Decisions**:
  - Test files that create `Module` (BasicBlockTest, LoopAnalysisTest, BCGen/TestHelpers, BCGen/HBC, VMRuntime/TestHelpers1, API/SegmentTestCompile) are deferred to P1-S7.5.
  - No additional production `Module` creation sites found beyond the three in the plan.

### P1-S7.5: Add RAII guards to unit tests
- **Files**: modified `unittests/IR/BuilderTest.cpp`, `unittests/IR/VariableTest.cpp`, `unittests/IR/IRUtilsTest.cpp`, `unittests/IR/IRVerifierTest.cpp`.
- **What was done**: Added `TypeContextRAII typeContextGuard(M.getTypeContext())` after Module creation in every test that uses `Type::` operations. For `BuilderTest::Types` (which uses `Type::unionTy` without a Module), added a standalone `TypeContext` + RAII guard.
- **Decisions**:
  - Only added guards to tests that directly use `Type::` operations (grep-verified). Test files that create Module but don't use `Type::` (BasicBlockTest, LoopAnalysisTest, etc.) were not modified — they don't need guards since P1-S8's static constructors (`createAnyType()`, etc.) remain constexpr and don't require the thread-local context. **(P1-S8 retroactively required guards in ALL Module-creating tests.)**
  - No new includes needed — all four files already include `IR.h` which includes `TypeContext.h`.

### P1-S8: Rewrite Type class
- **Files**: modified `include/hermes/IR/IR.h`, `include/hermes/IR/TypeContext.h`, `lib/IR/TypeContext.cpp`, `lib/IR/IR.cpp`, `lib/BCGen/SH/SH.cpp`, `lib/Optimizer/Scalar/InstSimplify.cpp`, `lib/BCGen/HBC/HBC.cpp`, and many test files (BasicBlockTest, LoopAnalysisTest, IRHashTest, BuilderTest, IRVerifierTest, IRUtilsTest, BCGen/TestHelpers, BCGen/HBC, VMRuntime/TestHelpers1, API/SegmentTestCompile, Optimizer/InstructionEscapeAnalysisTest, Optimizer/PassManagerTest).
- **What was done**: Replaced `Type`'s `uint16_t bitmask_` with `uint32_t id_` (index into TypeContext). Removed old nested `enum TypeKind` and bitmask infrastructure. All type operations (`unionTy`, `intersectTy`, `subtractTy`, predicates, `print`, `iterator`) delegate to `TypeContext::current()`. Well-known constructors and identity checks remain `constexpr`. Changed `TypeContext::typeArrays_` from `vector<uint32_t>` to `vector<Type>`. Moved `getUnionArms` out-of-line to resolve incomplete-type issue. Fixed SH.cpp to use `TypeKind::Empty` etc. instead of `Type::Empty`. Replaced `constexpr Type kNullOrUndef` in InstSimplify.cpp with `Type::createNullOrUndef()`. Added RAII guards at lazy compilation (`compileLazyFunctionWorker`) and eval compilation (`compileEvalWorker`) entry points in HBC.cpp. Added RAII guards to all remaining Module-creating test files.
- **Decisions**:
  - `Type` methods that need the context (queries, operations, `print`, `iterator`) are defined out-of-line in `TypeContext.cpp` to avoid include-order coupling. Only `constexpr` methods (identity checks, constructors, `operator==`) remain inline in `IR.h`.
  - `TypeContext::getUnionArms` moved out-of-line because `ArrayRef<Type>` pointer arithmetic requires `Type` to be complete, but `Type` is defined after `TypeContext.h` is included.
  - Fixed latent iterator-invalidation bug in `intersectTy`/`subtractTy`: arms are now copied to a `SmallVector` before calling `unionTy` which may reallocate `typeArrays_`.
  - Added `createNullOrUndef()` static constructor to Type (returns well-known `kNullOrUndefId`).
  - `sizeof(Type)` changed from 2 to 4 bytes. Updated `static_assert`.
  - `Type::iterator` reimplemented: for non-union types yields the type itself; for unions iterates arms via `getUnionArms`.
- **Issues**: P1-S7/P1-S7.5 missed two production compilation paths (`compileLazyFunctionWorker`, `compileEvalWorker` in HBC.cpp) and many test files that create Module but don't directly use `Type::`. These all needed RAII guards once Type methods became non-trivial.

