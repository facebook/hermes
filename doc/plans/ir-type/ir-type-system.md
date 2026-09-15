# Hermes IR Type System

## Overview

The IR type system represents the set of possible runtime types a value can have at
a given program point. It uses a **bitmask-based union type** encoding: each bit in
a 16-bit integer represents one primitive type, and a `Type` value is the set of
types that a value *might* be. This makes union, intersection, and subset operations
trivial bitwise operations. The entire `Type` fits in 2 bytes.

**Key files:**
- `include/hermes/IR/IR.h:58-414` — `Type` class and `TypeKind` enum
- `lib/Optimizer/Scalar/TypeInference.cpp` — type inference pass
- `lib/IR/IR.cpp:1040-1080` — type printing

## Primitive Types (TypeKind)

Each type occupies one bit in the bitmask (`include/hermes/IR/IR.h:61-86`):

| Bit | TypeKind      | Description |
|-----|---------------|-------------|
| 0   | Empty         | TDZ — variable before its declaration |
| 1   | Uninit        | After declaration, before initialization (maps to `undefined` at runtime) |
| 2   | Undefined     | The JS `undefined` value |
| 3   | Null          | The JS `null` value |
| 4   | Boolean       | `true` or `false` |
| 5   | String        | JS string |
| 6   | Number        | IEEE 754 double |
| 7   | BigInt        | Arbitrary-precision integer |
| 8   | Symbol        | ES2015 symbol |
| 9   | Environment   | Lexical scope (internal; not mixed with JS types) |
| 10  | PrivateName   | Private class field name (internal) |
| 11  | FunctionCode  | IR function reference, not a JS closure |
| 12  | Object        | JS object (including arrays, functions, regexps, etc.) |

13 types total. Types 0-8 and 12 can appear in user-facing values; 9-11 are
internal to the compiler.

## Composite / Pre-computed Masks

Defined as constants in `IR.h:122-134`:

| Name | Bits | Meaning |
|------|------|---------|
| `TYPE_ANY_MASK` | all except Empty, Uninit | Standard "any" — all observable JS types |
| `TYPE_ANY_EMPTY_UNINIT_MASK` | all except Environment, PrivateName, FunctionCode | "any" including TDZ states |
| `PRIMITIVE_BITS` | Number \| String \| BigInt \| Null \| Undefined \| Boolean \| Symbol | JS primitives |
| `NONPTR_BITS` | Number \| Boolean \| Null \| Undefined | Non-pointer values (useful for GC) |

## Type Operations

All operations are `constexpr` and work on bitmasks:

```cpp
// Union: value could be either type
static Type unionTy(Type A, Type B);       // A.bitmask_ | B.bitmask_

// Intersection: only types in both
static Type intersectTy(Type A, Type B);   // A.bitmask_ & B.bitmask_

// Subtraction: remove types in B from A
static Type subtractTy(Type A, Type B);    // A.bitmask_ & ~B.bitmask_
```

### Querying

- `isSubsetOf(Type t)` — all bits of `this` are in `t`
- `canBeType(Type t)` — non-empty intersection
- `isProperSubsetOf(Type t)` — strict subset
- `isPrimitive()` — only bits in `PRIMITIVE_BITS`
- `isNonPtr()` — only bits in `NONPTR_BITS`
- `isKnownPrimitiveType()` — exactly one primitive bit set

### Special Values

- `createNoType()` — empty bitmask (0). Represents "impossible" / unreachable.
- `createAnyType()` — `TYPE_ANY_MASK`. Default for unknown values.

## How Types Attach to the IR

Every `Value` (base class for instructions, literals, parameters, etc.) carries a
`Type valueType` member (`IR.h:712`), defaulting to `createAnyType()`.

### Literals Get Exact Types

| Literal class | Type |
|---------------|------|
| `LiteralEmpty` | Empty |
| `LiteralUninit` | Uninit |
| `LiteralUndefined` | Undefined |
| `LiteralNull` | Null |
| `LiteralNumber` | Number (or Int32 if the value fits) |
| `LiteralString` | String |
| `LiteralBigInt` | BigInt |
| `LiteralBool` | Boolean |
| `GlobalObject` | Object |

### Instructions Declare Inherent Types

Instructions can declare `static Optional<Type> getInherentTypeImpl()` to declare
a type that is always correct regardless of operands:

- Arithmetic/conversion: `AsNumberInst` → Number, `AsInt32Inst` → Int32
- Allocation: `AllocArrayInst`, `AllocObjectLiteralInst`, `CreateFunctionInst` → Object
- Scope: `CreateScopeInst`, `ResolveScopeInst`, `GetParentScopeInst` → Environment
- Type queries: `TypeOfInst` → String
- Coercion: `AddEmptyStringInst` → String, `ToPropertyKeyInst` → String | Symbol

Instructions without an inherent type (e.g. `LoadPropertyInst`, `CallInst`,
`CatchInst`) default to `any` and rely on the inference pass.

## Type Inference Pass

**File:** `lib/Optimizer/Scalar/TypeInference.cpp`

A whole-program, fixed-point, forward type inference that narrows value types.

### Algorithm

1. **Partition** functions and variables into groups (union-find) based on mutual
   use. Functions that call each other or share variables are in the same partition.

2. **Clear** all instruction types to their inherent type (or NoType). Save
   pre-pass types as upper bounds.

3. **Fixed-point iteration** over each partition:
   - Infer parameter types from call sites (union of all actual arguments).
   - Infer instruction types from operand types (see rules below).
   - Infer return types from `ReturnInst` values.
   - Infer variable/stack types from stored values.
   - Repeat until no type changes.

4. **Clamp**: intersect inferred types with pre-pass types to prevent widening
   beyond what was previously known.

### Inference Rules for Binary Operations

| Operation | Result Type |
|-----------|-------------|
| Comparisons (`==`, `<`, `instanceof`, etc.) | Boolean |
| `+` with a String operand | String |
| `+` of two Numbers | Number |
| `+` of two BigInts | BigInt |
| `+` of two non-string primitives | Number \| BigInt |
| `+` general case | String \| Number \| BigInt |
| `-`, `*`, `/`, `**` | Number \| BigInt |
| `<<`, `>>` | Number \| BigInt |
| `>>>` | Uint32 (subtype of Number) |
| `&`, `\|`, `^` | Int32 \| BigInt |
| `%` | Int32 \| BigInt |

### Inference Rules for Unary Operations

| Operation | Result Type |
|-----------|-------------|
| `typeof` | String |
| `void` | Undefined |
| `!` | Boolean |
| `-`, `++`, `--` on Number | Number |
| `-`, `++`, `--` on BigInt | BigInt |
| `-`, `++`, `--` general | Number \| BigInt |
| `~` on Number | Int32 |
| `~` on BigInt | BigInt |
| `~` general | Int32 \| BigInt |

### PHI Nodes

A PHI's type is the **union** of all its incoming values' types. The algorithm
recursively traverses nested PHIs to collect all non-PHI inputs.

### Memory Locations (Variables, Stack Slots)

The inferred type of a memory location is the **union** of all values stored to it.

### Parameters

If all call sites of a function are known, a parameter's type is the union of all
corresponding arguments across call sites (defaulting to Undefined for missing
arguments). If any call site is unknown, the parameter gets `any`.

### Return Types

The return type of a function is the union of all `ReturnInst` operand types.
Generator functions always return `any` because `.return()` can inject any value.

## Type Narrowing Instructions

Several instructions refine (narrow) types:

### ThrowIfInst
Removes `Empty` and `Uninit` from the type — used for TDZ checks. After this
instruction, the value is known not to be in TDZ.

### CheckedTypeCastInst
Intersects the input type with a specified target type. If the intersection is
empty, the code is unreachable.

### UnionNarrowTrustedInst
Intersects a saved result type with the operand type. Used after conditional
branches to narrow union types (e.g., after a `typeof` check).

## Type Printing

Types are printed as pipe-separated names: `number|string|undefined`. Special
cases:
- Empty bitmask → `notype`
- All types except Empty/Uninit → `any`
- All types including Empty/Uninit → `any|empty|uninit`

## Design Properties

- **Monotonic narrowing**: the inference pass can only narrow types, never widen
  them beyond the pre-pass upper bound.
- **Conservative**: if in doubt, the type is `any`. Soundness is prioritized
  over precision.
- **No nominal object types**: Object is a single type — there is no
  `Array`, `RegExp`, `Function` distinction in the IR type system.
- **No function signature types**: `FunctionCode` is a single type with no
  parameter/return type information embedded.
- **Cheap**: 2-byte representation, all operations are bitwise, no allocations.
