# Implementation Memory

Quick reference for the IR type system v2. For full design see
[ir-type-v2-implementation.md](ir-type-v2-implementation.md).

## Build
- ASan build: `cmake -B cmake-build-asan -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DHERMES_ENABLE_ADDRESS_SANITIZER=ON -DCMAKE_CXX_FLAGS="-O1" -DCMAKE_C_FLAGS="-O1"`
- TypeContext.cpp is part of `hermesFrontend` target in `lib/CMakeLists.txt`.
- Unit test target: `HermesIRTests` (in `unittests/IR/CMakeLists.txt`).

## Well-Known IDs
Leaf types and well-known unions (NullOrUndef, AnyType, Numeric, AnyEmptyUninit) are pre-allocated with fixed IDs. Remaining IDs up to `kFirstDynamicId` are reserved padding.

## Design Decisions Beyond the Design Docs
- Added `TypeKind::UInt31` to close the lattice: `Int32 ∩ Uint32 = UInt31` (integers in [0, 2^31-1]). The design doc left this as an open question. UInt31 <: Int32, UInt31 <: Uint32, UInt31 <: Number.
- Added `kNullOrUndefId` as a well-known union (needed for `InstSimplify.cpp` migration).

## Type Class
- `Type` is 4 bytes (`uint32_t id_`). `TypeContext` is a `friend`.
- `constexpr` methods (identity checks, well-known constructors) stay inline in `IR.h`.
- All non-trivial methods (`canBeX`, `unionTy`, `print`, `iterator`, etc.) defined in `TypeContext.cpp` to avoid include-order coupling.
- `getUnionArms` is out-of-line because `ArrayRef<Type>` pointer arithmetic needs complete `Type`.
- `typeArrays_` is `vector<Type>`. Union arm arrays and tuple elements stored there.

## Queries & Operations
- Each `canBeX` has well-known ID fast paths before falling back to `containsMatchingKind`, a private template helper taking a predicate. `allMatchKind` is the universal-quantifier counterpart.
- File-scoped kind helpers (`isNumberKind`, `isObjectKind`, `isPrimitiveKind`, `isNonPtrKind`) encode subtype relationships: Int32/Uint32/UInt31 are number subtypes; ClassInstance/Array/Tuple/Function/ExactObject are object refinements.
- Union intern table: `DenseMap<UnionInternKey, uint32_t, UnionInternKeyInfo>`. Key is sorted arm IDs in a SmallVector. Pre-populated for well-known unions.
- `createUnionImpl`: flatten → sort → dedup → subsume → intern.
- `isSubsetOf`/`areDisjoint` are `const`; `unionTy`/`intersectTy`/`subtractTy` are non-const (may create entries).
- `intersectTy` leaf-leaf: returns UInt31 for overlapping number-family (Int32 ∩ Uint32), NoType otherwise.
- `subtractTy` returns conservative approximation when exact result is not representable.

## Thread-Local Context & Module
- `TypeContext::current_` is `static thread_local` (defined in `TypeContext.cpp`).
- `TypeContextRAII` saves/restores the pointer (friend of `TypeContext`).
- `Module` owns `TypeContext typeContext_` (default-constructed). Access via `getTypeContext()`.
- `TypeContext.h` is included in `IR.h` (no dependency on `Type` definition).
- RAII guards installed at all production compilation entry points (`BCProviderFromSrc.cpp`, `CompilerDriver.cpp`, `shermes.cpp`, plus lazy/eval paths in `HBC.cpp`).

## Gotchas
- **Iterator invalidation**: `intersectTy`/`subtractTy` must copy union arms to a `SmallVector` before calling `unionTy`, because `unionTy` may reallocate `typeArrays_`.
- **RAII guards everywhere**: ALL test files creating Module need guards — not just those using `Type::` directly. Instruction constructors call `unionTy`.
- **Header layering**: see "Type Class" above — non-trivial methods must be out-of-line.

## Formatting
- `format` uses "any" shorthand when the type is a superset of AnyType. Extra arms (empty, uninit) appended with `|`.
- `kindName()` is file-scoped in `TypeContext.cpp`, covers all `TypeKind` values.
