# Flow Type System (FlowContext)

## Overview

The Flow type system in Hermes is a full semantic type system used during static
analysis and type checking. Unlike the IR bitmask type system (13 bits, 2 bytes),
it supports nominal classes, generics, function signatures, tuples, exact objects,
union types with recursive structures, and subtyping with variance rules.

**Key files:**
- `include/hermes/Sema/FlowContext.h` — all type class definitions
- `lib/Sema/FlowContext.cpp` — comparison, hashing, canonicalization
- `lib/Sema/FlowChecker.cpp` — type checking, `canAFlowIntoB()`, annotation parsing
- `lib/Sema/FlowChecker-scopetypes.cpp` — scope resolution, alias resolution, looping type detection

## Memory Representation

Types use a three-layer architecture:

1. **`Type`** — a lightweight wrapper holding a `TypeInfo *info` pointer, an
   `ESTree::Node *node` (source location), and an `isLooping` flag for recursive
   structures.

2. **`TypeInfo`** — abstract base class for type backing storage. Carries:
   - `TypeKind kind_` — discriminator enum
   - Lazy-cached hash (`hashed_`, `hashValue_`)
   - Virtual `_compareImpl()`, `_equalsImpl()`, `_hashImpl()`

3. **Concrete type classes** — `UnionType`, `ClassType`, `TypedFunctionType`, etc.

All types are allocated in `std::deque` containers within `FlowContext`, providing
stable pointers without invalidation. Singleton types (Void, Null, Number, etc.)
exist as single instances; complex types are interned via hash-consing where
appropriate.

## Complete Type Kinds

### Singleton Types (immutable, one instance each)

| Type | Meaning |
|------|---------|
| `Void` | JS `undefined` / absence of value |
| `Null` | JS `null` |
| `Boolean` | `true` or `false` |
| `String` | JS string |
| `Number` | IEEE 754 double |
| `BigInt` | Arbitrary-precision integer |
| `CPtr` | Internal C pointer (native interop) |
| `Any` | Supertype of everything; outputs require checked casts |
| `Mixed` | Bidirectional any (flows both in and out freely) |
| `Generic` | Placeholder for unspecialized generic class/alias (must not escape checker) |
| `InferencePlaceholder` | Used during generic type argument inference |

### Complex (Structural) Types

| Type | Description |
|------|-------------|
| `Union` | `A \| B \| C` — canonical sorted union with looping-arm support |
| `Array` | `Array<T>` — single element type, invariant |
| `Tuple` | `[A, B, C]` — fixed-length, ordered, typed |
| `ExactObject` | `{x: A, y: B}` — named fields, exact (no extra properties) |
| `TypedFunction` | `(p1: A, p2: B) => R` — full signature with this-param, optional params |
| `UntypedFunction` | Function with no type information (only async/generator flags) |
| `NativeFunction` | C++ native function with a `NativeSignature` |
| `Class` | Nominal class with fields, methods, constructor, superclass chain |
| `ClassConstructor` | Type of a class variable/declaration (distinct from instance type) |

## Singleton Types

Created once in `FlowContext` and accessed via getters (`getVoid()`, `getNull()`,
etc.). Comparison is pointer identity (O(1)). Hash is the `TypeKind` ordinal.

### `Any` vs `Mixed`

- **`Any`**: accepts all inputs. When a value typed `any` flows *out* to a
  concrete type, a runtime checked cast is required (`needCheckedCast = true`).
- **`Mixed`**: flows both directions without checked casts. Used where the type
  system cannot determine the type but no runtime check is desired.

## Union Types

Represents `A | B | C`. Encoded as a vector of arms with a dual-mode
canonicalization strategy.

### Canonicalization

1. Nested unions are flattened (one level).
2. Arms are partitioned into **non-looping** (sorted, uniqued via structural
   comparison) and **looping** (recursive arms, deduplicated via O(n²) matching).
3. `numNonLoopingTypes_` tracks the boundary.
4. Single-arm unions collapse: `maybeCreateUnion()` returns the arm directly.

### Optional / Nullable

No separate Optional type. These are represented as unions:
- `type?` → `Void | Null | type`
- `?type` (nullable) → `Void | Null | type`

`createPopulatedNullable(type)` creates the canonical union.

### Looping (Recursive) Types

Structural recursive types like `type Tree = null | {val: number, children: Tree[]}`
cause cycles. The `FindLoopingTypes` pass marks all arms on a cycle with
`isLooping = true`. Looping arms use slow O(n²) matching during comparison since
they cannot be deterministically sorted. Nominal types (classes) stop recursion
traversal — they are compared by identity, not structure.

## Array Types

`Array<T>` with a single element type. **Invariant**: `Array<A>` does NOT flow to
`Array<B>` even if `A <: B`, because arrays are mutable at runtime.

## Tuple Types

`[A, B, C]` — fixed length, positional types. **Invariant**: element count and
types must match exactly.

## Exact Object Types

`{x: A, y: B}` — named, typed fields with exact semantics (no extra properties
allowed). Field order matters for comparison. Each field has:
- `Identifier name`
- `Type *type`

A `fieldNameMap_` provides O(1) lookup by name.

Note: Inexact objects (`{x: number, ...}`) are not yet implemented.

## Function Types

### Base: `BaseFunctionType`
Carries `isAsync_` and `isGenerator_` flags. Async/generator status must match
exactly for subtyping.

### `TypedFunctionType`
Full signature: return type, optional this-param, parameter list where each
parameter has a name, type, and optional flag.

### `UntypedFunctionType`
No signature — only async/generator flags. Flows to other untyped functions
regardless of actual parameters.

### `NativeFunctionType`
Points to a `NativeSignature` from NativeContext. Flows only if signatures are
identical (pointer equality).

### Function Subtyping

Functions use standard variance rules:
- **Contravariant in parameters**: callee's parameter types must accept the
  caller's argument types.
- **Covariant in return type**: callee's return type must be accepted where the
  caller's return type is expected.
- Async/generator status must match exactly.
- Function kind must match (Typed ≠ Untyped ≠ Native).

## Class Types (Nominal)

Classes are **nominal** — subtyping is based on the inheritance chain, not
structure. Each class has a unique ID (`TypeWithId` base class).

### Fields

```cpp
struct Field {
    Identifier name;
    Type *type;
    size_t layoutSlotIR;   // maps to PrLoad/PrStore IR slots
    bool isPrivate;
    ESTree::MethodDefinitionNode *method;  // nullptr for data fields
    bool overridden;       // can be modified by subclasses
};
```

- `fields_` — direct (non-inherited) fields only.
- `fieldNameMap_` — all accessible fields including inherited (name → index).
- `privateFieldNameMap_` — private fields (own class only).

### Other Properties

- `constructorType_` — function type of the constructor.
- `homeObjectType_` — the class representing `[[HomeObject]]` prototype.
- `superClass_` — parent class (nullable).

### Class Subtyping

`ClassA` flows to `ClassB` iff `ClassB` is in `ClassA`'s superclass chain.
No structural matching — two classes with identical fields but different
declarations are incompatible.

### `ClassConstructorType`

The type of a class declaration/variable (e.g., `const C = class { ... }`).
Wraps a `ClassType`. Distinct from the instance type.

## Generics

### Representation

```cpp
template <class Spec>
class GenericInfo {
    ESTree::Node *originalNode;
    DenseMap<ArrayRef<TypeInfo*>, Spec*> specializations;  // memoized
};
```

### Specialization

- Type arguments are matched to type parameters.
- A specialization cache maps type argument tuples to instantiated types/AST
  clones, preventing duplicate work.
- `validateAndBindTypeParameters()` populates a binding table mapping parameter
  names to concrete types.

### `InferencePlaceholder`

Used during generic type argument inference. When a generic function is called
without explicit type arguments, placeholders are created and filled in by
`matchConstraintToType()` based on argument types.

## Subtyping (`canAFlowIntoB`)

The core function returns `CanFlowResult { bool canFlow, bool needCheckedCast }`.

### Rules (in priority order)

1. **Identity**: `a == b` → flows.
2. **Top types**: `_ → any` flows; `_ → mixed` flows; `any → _` flows but
   requires checked cast.
3. **Unions**:
   - `(A | B) → C` flows iff `A → C` AND `B → C`.
   - `A → (B | C)` flows iff `A → B` OR `A → C`.
4. **Structural types** (all invariant):
   - Arrays: element types must be equal.
   - Tuples: length must match, element-wise equal.
   - Objects: field names, order, and types must match exactly.
5. **Nominal types**: class chain lookup.
6. **Functions**: contravariant params, covariant return, matching kind and
   async/generator flags.

### Checked Casts

When `any` flows into a concrete type, `needCheckedCast = true` signals that
IRGen must insert a runtime type check instruction (`CheckedTypeCastInst`).

## Type Comparison, Equality, and Hashing

### Comparison (`TypeInfo::compare`)

Lexicographic structural ordering. Tracks visited pairs in `CompareState` to
detect cycles (returns 0 for revisited pairs). Results are cached.

### Equality (`TypeInfo::equals`)

Structural equality with cycle tolerance. Special handling for incomplete unions
(created during alias resolution before canonicalization).

### Hashing (`TypeInfo::hash`)

Intentionally shallow — unions hash on `(kind, armCount)`, arrays ignore element
type, tuples/objects hash on size only. This avoids O(n) cost in hash tables while
maintaining the invariant that `equals()` implies equal hashes.

## Type Resolution Pipeline

Three stages during scope processing:

1. **Forward declarations** — create `ClassType` instances with IDs but no fields.
   Record type alias names without resolving their RHS.

2. **Alias resolution** — resolve type alias RHS annotations. Detect and reject
   union-only cycles. Populate forward-declared aliases.

3. **Complete declarations** — parse class bodies (fields, methods, constructor,
   superclass). Initialize `ClassType` fields. Canonicalize unions.

This staging allows mutual references between classes and type aliases.

## Type Narrowing

### `getNonOptionalSingleType(type)`

Extracts the single non-void, non-null arm from a union. Returns nullptr if
multiple non-optional arms exist. Used in control flow after null checks.

### `tryNarrowType(exprType, targetType)`

Attempts to narrow a union to a target type. Returns the narrowed type and whether
a checked cast is needed. Used after type guards and conditional branches.

## Relationship to the IR Type System

The Flow type system and IR type system are separate:

- **Flow types** operate during semantic analysis (`FlowChecker`). They are rich,
  structural/nominal, and support generics.
- **IR types** operate during optimization and codegen. They are a 16-bit bitmask
  of 13 primitive kinds.

Connection points:
- `ClassType::Field::layoutSlotIR` maps Flow fields to IR `PrLoad`/`PrStore` slot
  indices.
- IRGen converts Flow types to IR types and inserts `CheckedTypeCastInst` where
  `needCheckedCast` was flagged.
- Flow's `Number`, `String`, `Boolean`, etc. map directly to IR type bits.
- Flow's `Class` maps to IR's `Object` bit.
- Flow's `Union` maps to the IR union of its arms' IR types.

## Key Design Properties

- **Nominal classes, structural everything else**: classes use identity; arrays,
  tuples, objects, functions use structure.
- **Invariant containers**: arrays, tuples, and objects are invariant because JS
  values are mutable.
- **Cycle-tolerant**: comparison, equality, and hashing all handle recursive types
  via visited-pair tracking.
- **Lazy hashing**: avoids expensive recursive hashing until a type enters a hash
  table.
- **Stable pointers**: `std::deque` allocation ensures type pointers never
  invalidate.
- **Forward declaration support**: types can be created with an ID before their
  fields are known, enabling mutual references.
