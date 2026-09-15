# IR Type System v2 — Implementation Design

## References

- [ir-type-system-v2-design.md](ir-type-system-v2-design.md) — conceptual type
  hierarchy, type algebra, design goals, interning algorithm, migration phases.
- [ir-type-system.md](ir-type-system.md) — current 2-byte bitmask type system.
- [flow-type-system.md](flow-type-system.md) — Flow type system that feeds
  into IR types during lowering.

This document covers the concrete C++ implementation: data structures, class
layouts, APIs, integration points, and invariants.

---

## 1. Type Representation

### Current: 2-byte bitmask

```cpp
class Type {
  uint16_t bitmask_;  // 13 primitive kinds as bit flags
};
static_assert(sizeof(Type) == 2);
```

### New: 4-byte opaque index

```cpp
class Type {
  uint32_t id_;  // Index into TypeContext type table
};
static_assert(sizeof(Type) == 4);
```

`Type` is a pure opaque handle. It carries no type information itself — all
type data lives in the `TypeContext` type table, indexed by `id_`. Two
types are equal iff their `id_` values are equal.

The size increase from 2 to 4 bytes fits within the existing `Value` layout
constraint (`sizeof(valueType) <= 4`, enforced at `IR.h:737`).

### Well-Known Type IDs

The `TypeContext` constructor pre-allocates entries for all primitive types
and commonly used unions. These have fixed `id_` values:

```cpp
// Pre-allocated type IDs (assigned by TypeContext constructor).
// These are constants, valid for any TypeContext instance.
static constexpr uint32_t kNoTypeId        = 0;
static constexpr uint32_t kEmptyId         = 1;
static constexpr uint32_t kUninitId        = 2;
static constexpr uint32_t kUndefinedId     = 3;
static constexpr uint32_t kNullId          = 4;
static constexpr uint32_t kBooleanId       = 5;
static constexpr uint32_t kStringId        = 6;
static constexpr uint32_t kNumberId        = 7;
static constexpr uint32_t kBigIntId        = 8;
static constexpr uint32_t kSymbolId        = 9;
static constexpr uint32_t kEnvironmentId   = 10;
static constexpr uint32_t kPrivateNameId   = 11;
static constexpr uint32_t kFunctionCodeId  = 12;
static constexpr uint32_t kObjectId        = 13;
static constexpr uint32_t kBits32Id        = 14;
static constexpr uint32_t kAnyTypeId       = 15;  // Union of all JS-observable types
static constexpr uint32_t kNumericId           = 16;  // Number | BigInt
static constexpr uint32_t kAnyEmptyUninitId    = 17;  // any | Empty | Uninit
// ... other common unions as needed ...
static constexpr uint32_t kFirstDynamicId      = 32;  // IDs below this are reserved
// The well-known ID assignment is frozen: IDs are never removed or reordered.
// New well-known IDs are only appended before kFirstDynamicId.
```

The `Type::createX()` static methods return these well-known IDs:

```cpp
static constexpr Type createNoType()      { return Type(kNoTypeId); }
static constexpr Type createNumber()      { return Type(kNumberId); }
static constexpr Type createString()      { return Type(kStringId); }
static constexpr Type createObject()      { return Type(kObjectId); }
static constexpr Type createAnyType()     { return Type(kAnyTypeId); }
static constexpr Type createBits32()      { return Type(kBits32Id); }
// ... etc.
```

These remain `static constexpr` — no context needed to create primitive types.

### Encoding Examples

| Type | id_ | Table entry |
|------|-----|-------------|
| `NoType` | `0` | `{kind: NoType}` |
| `Number` | `7` | `{kind: Number}` |
| `Object` (unrefined) | `13` | `{kind: Object}` |
| `Number \| String` | `N` | `{kind: Union, arms: [Number, String]}` |
| `ClassA` | `N` | `{kind: ClassInstance, classId: A}` |
| `Array(Number)` | `N` | `{kind: Array, elemType: Number}` |
| `Number \| ClassA` | `N` | `{kind: Union, arms: [Number, ClassA]}` |
| `ClassA \| ClassB` | `N` | `{kind: Union, arms: [ClassA, ClassB]}` |
| `AnyType` | `15` | `{kind: Union, arms: [all JS-observable types]}` |
| `bits32` | `14` | `{kind: Bits32}` |

**Note on `bits32`**: `bits32` is not yet used in the current codebase. It is
reserved for future use (physical 32-bit integer, signless). Its well-known
ID and `TypeKind` enum value are defined from the start so they are stable.

---

## 2. Type Table Entries

Each entry in the type table describes a type. All entries are immutable once
created.

The `TypeKind` enum is defined in full from the start, but refined kinds
(`ClassInstance`, `Array`, `Tuple`, `Function`, `ExactObject`, `Int32`,
`Uint32`) are dead code until their respective migration phase activates
them. In Phase 1, only the leaf kinds and `Union` are populated.

### TypeKind

```cpp
enum class TypeKind : uint8_t {
  // --- Leaf kinds (no sub-type data) ---
  NoType,         // Empty set, bottom
  Undefined,
  Null,
  Boolean,
  Number,         // Unrefined fp64
  BigInt,
  String,
  Symbol,
  Empty,          // TDZ
  Uninit,         // Declared, not initialized
  Environment,
  PrivateName,
  FunctionCode,
  Object,         // Unrefined JS object
  Bits32,         // Physical 32-bit integer, signless

  // --- Number subtypes (still fp64, value constrained) ---
  Int32,          // Integer in [-2^31, 2^31 - 1]
  Uint32,         // Integer in [0, 2^32 - 1]

  // --- Refined object types ---
  ClassInstance,  // Nominal class instance
  Array,          // Array with known element type
  Tuple,          // Fixed-length positional types
  Function,       // Callable with known signature
  ExactObject,    // Known exact field set

  // --- Composite ---
  Union,          // Union of two or more types
};
```

### TypeEntry

```cpp
struct TypeEntry {
  TypeKind kind;

  union {
    // ClassInstance
    struct {
      uint32_t superClassTypeId; // type table ID of parent ClassInstance,
                                 // or kNoTypeId if no parent
      Identifier className;     // for runtime type descriptors / errors
      uint32_t fieldOffset;     // into fieldArrays_
      uint16_t fieldCount;
    } classInstance;

    // Array
    struct {
      Type elemType;
    } array;

    // Tuple
    struct {
      uint32_t elemOffset;  // into typeArrays_
      uint16_t elemCount;
    } tuple;

    // Function
    struct {
      Type returnType;
      Type thisType;        // NoType if no `this` parameter
      Type restType;        // NoType if no rest parameter
      uint32_t paramOffset; // into paramArrays_
      uint16_t paramCount;
      bool isAsync;
      bool isGenerator;
    } function;

    // ExactObject
    struct {
      uint32_t fieldOffset; // into fieldArrays_
      uint16_t fieldCount;
    } exactObject;

    // Union
    struct {
      uint32_t armOffset;   // into typeArrays_
      uint16_t armCount;    // >= 2
    } union_;
  };
};
```

### Side Arrays

Variable-length data is stored in side arrays within `TypeContext`,
referenced by (offset, count) pairs in the `TypeEntry`:

| Side array | Element type | Used by |
|-----------|-------------|---------|
| `typeArrays_` | `Type` | Tuple elements, Union arms |
| `paramArrays_` | `FunctionParam` | Function parameters |
| `fieldArrays_` | `ExactObjectField` | ExactObject fields |

```cpp
struct FunctionParam {
  Type type;
  bool isOptional;
};

struct ExactObjectField {
  Identifier name;
  Type type;
};
```

**ExactObject field ordering**: field order is preserved and significant.
Two ExactObjects with the same fields in different orders are different types.
The intern key hashes the fields in order, so `{x: number, y: string}` and
`{y: string, x: number}` produce different type table entries.

**C++17 note**: the `TypeEntry` pseudo-code above uses C++20 designated
initializers (`{.array = {elemType}}`). The actual C++17 implementation will
use explicit construction or a tagged-union helper.

---

## 3. TypeContext

### Responsibilities

- Owns the type table (all `TypeEntry` values).
- Provides all type operations (union, intersect, subtract, isSubsetOf).
- Stores the class inheritance hierarchy.
- Interns types via hash-consing.

### Ownership

One `TypeContext` per `Module`:

```cpp
class Module : public Value {
  // ...
  TypeContext typeContext_;
  // ...
public:
  TypeContext &getTypeContext() { return typeContext_; }
};
```

**Header dependency**: `Type` is defined in `IR.h`. `TypeContext` uses
`Type` and is defined in `TypeContext.h`. `Module` (in `IR.h`) embeds
`TypeContext` by value. The include order is:
1. `IR.h` forward-declares `class TypeContext;`
2. `IR.h` defines `Type` (before `Module`).
3. `IR.h` defines `Module` with `TypeContext typeContext_;` — this
   requires the full definition, so `IR.h` includes `TypeContext.h`
   above the `Module` class definition (after `Type` is defined).
4. `TypeContext.h` includes only the `Type` definition (not `Module`).

### Storage Layout

```cpp
class TypeContext {
 public:
  // --- Phase 1: thread-local "current" context ---
  static TypeContext &current();

  // --- Type operations ---
  Type unionTy(Type A, Type B);
  Type intersectTy(Type A, Type B);
  Type subtractTy(Type A, Type B);
  bool isSubsetOf(Type A, Type B);

  // --- Type queries ---
  TypeKind getKind(Type t) const;
  bool canBeNumber(Type t) const;
  bool canBeString(Type t) const;
  bool canBeObject(Type t) const;
  bool canBeNull(Type t) const;
  bool canBeUndefined(Type t) const;
  bool isPrimitive(Type t) const;
  bool isNoType(Type t) const;
  // ... etc.

  // --- Refined type construction ---
  Type createClassInstance(
      Type superClassType, Identifier className,
      llvh::ArrayRef<ExactObjectField> fields);
  Type createArray(Type elemType);
  Type createTuple(llvh::ArrayRef<Type> elemTypes);
  Type createFunction(
      Type returnType,
      Type thisType,
      Type restType,
      llvh::ArrayRef<FunctionParam> params,
      bool isAsync,
      bool isGenerator);
  Type createExactObject(llvh::ArrayRef<ExactObjectField> fields);

  // --- Refined type accessors ---
  Type getClassSuperType(Type t) const;
  Identifier getClassName(Type t) const;
  Type getArrayElementType(Type t) const;
  const FunctionParam *getFunctionParams(Type t, uint16_t &count) const;
  // ... etc.

  // --- Class hierarchy ---
  bool isSubclass(Type a, Type b) const;

 private:
  static thread_local TypeContext *current_;

  // Type table. Index 0 = NoType. Pre-allocated entries for primitives.
  std::vector<TypeEntry> entries_;

  // Side arrays for variable-length data.
  std::vector<Type> typeArrays_;
  std::vector<FunctionParam> paramArrays_;
  std::vector<ExactObjectField> fieldArrays_;

  // Interning table: structural hash → type ID.
  llvh::DenseMap<TypeEntryKey, uint32_t> internTable_;
};
```

### Construction

The `TypeContext` constructor pre-allocates all well-known types:

```cpp
TypeContext::TypeContext() {
  // Index 0: NoType
  entries_.push_back({TypeKind::NoType, {}});
  // Index 1: Empty
  entries_.push_back({TypeKind::Empty, {}});
  // Index 2: Uninit
  entries_.push_back({TypeKind::Uninit, {}});
  // ... all primitives and well-known unions ...
  // Index kAnyTypeId: Union of all JS-observable types
  // Index kNumericId: Union(Number, BigInt)
  // ...
  assert(entries_.size() == kFirstDynamicId);
}
```

### Interning

Every type construction method interns the result. Since sub-types are already
interned (they are `Type` values = indices), the intern key is a shallow hash
of the `TypeEntry` contents:

```
createArray(elemType):
  key = {kind: Array, elemType.id_}
  if internTable_.count(key):
    return Type(internTable_[key])
  id = entries_.size()
  entries_.push_back({TypeKind::Array, {.array = {elemType}}})
  internTable_[key] = id
  return Type(id)
```

No deep recursive hashing is needed because sub-types are indices.

---

## 4. Type Operations

All type operations are methods on `TypeContext`. They look up the
`TypeEntry` for each operand and dispatch based on `TypeKind`.

### unionTy(A, B)

```cpp
Type TypeContext::unionTy(Type A, Type B) {
  if (A == B) return A;
  if (isNoType(A)) return B;
  if (isNoType(B)) return A;
  if (isSubsetOf(A, B)) return B;
  if (isSubsetOf(B, A)) return A;
  return createUnionImpl(A, B);
}
```

`createUnionImpl` constructs a canonical `Union` entry:
1. Flatten: if A or B is already a Union, extract its arms.
2. Collect all arms into a single list.
3. Deduplicate: remove identical arms (same `id_`).
4. Subsume: if arm X is a subtype of arm Y, drop X.
5. Sort arms by `id_` for canonical order.
6. If one arm remains, return it directly (no Union entry).
7. Intern the Union entry.

Special cases for object refinements:
- `union(ClassA, ClassB)` where A extends B → return B.
- `union(refined, Object)` → return Object (unrefined subsumes all).
- `union(Array(S), Array(T))` where S ≠ T → Union (arrays are invariant).

### intersectTy(A, B)

```cpp
Type TypeContext::intersectTy(Type A, Type B) {
  if (A == B) return A;
  if (isSubsetOf(A, B)) return A;
  if (isSubsetOf(B, A)) return B;
  return intersectImpl(A, B);
}
```

`intersectImpl` rules:
- Two disjoint leaf types → NoType.
- `intersect(Object, ClassA)` → ClassA.
- `intersect(ClassA, ClassB)` unrelated → NoType.
- Distribute over unions on either side:
  `intersect(Union(A₁,...,Aₙ), B)` = `union(intersect(A₁,B), ..., intersect(Aₙ,B))`.
  `intersect(A, Union(B₁,...,Bₙ))` = `union(intersect(A,B₁), ..., intersect(A,Bₙ))`.

Note: cases like `intersect(Number, Int32)` and `intersect(ClassA, ClassB)`
where one is a subtype of the other are handled by the `isSubsetOf` checks
in `intersectTy` before `intersectImpl` is reached.

### subtractTy(A, B)

```cpp
Type TypeContext::subtractTy(Type A, Type B) {
  if (isSubsetOf(A, B)) return createNoType();
  if (areDisjoint(A, B)) return A;
  return subtractImpl(A, B);
}
```

`subtractImpl` rules:
- Distribute over unions in A:
  `subtract(Union(A₁,...,Aₙ), B)` = `union(subtract(A₁,B), ..., subtract(Aₙ,B))`.
- `subtract(Number, Int32)` → Number (conservative).
- `subtract(Object, ClassA)` → Object (conservative).

### isSubsetOf(A, B)

```cpp
bool TypeContext::isSubsetOf(Type A, Type B) {
  if (A == B) return true;
  if (isNoType(A)) return true;

  TypeKind akind = getKind(A);
  TypeKind bkind = getKind(B);

  // Union on left: every arm must be a subset.
  if (akind == TypeKind::Union) {
    for (Type arm : getUnionArms(A))
      if (!isSubsetOf(arm, B))
        return false;
    return true;
  }

  // Union on right: A must be a subset of some arm.
  if (bkind == TypeKind::Union) {
    for (Type arm : getUnionArms(B))
      if (isSubsetOf(A, arm))
        return true;
    return false;
  }

  // Both are non-union.
  return isBaseSubsetOf(A, akind, B, bkind);
}
```

`isBaseSubsetOf` handles non-union cases:
- `Int32 <: Number`, `Uint32 <: Number`.
- `ClassA <: Object` (any refined object is a subset of unrefined Object).
- `ClassA <: ClassB` iff A is a subclass of B (walked via
  `superClassTypeId` in the type table entries).
- All other cross-kind pairs → false.

### areDisjoint(A, B)

Direct kind-based check without materializing an intersection:

```cpp
bool TypeContext::areDisjoint(Type A, Type B) {
  if (A == B) return isNoType(A);

  TypeKind ak = getKind(A);
  TypeKind bk = getKind(B);

  // Distribute over unions.
  if (ak == TypeKind::Union) {
    for (Type arm : getUnionArms(A))
      if (!areDisjoint(arm, B))
        return false;
    return true;
  }
  if (bk == TypeKind::Union) {
    for (Type arm : getUnionArms(B))
      if (!areDisjoint(A, arm))
        return false;
    return true;
  }

  // Two non-union types: disjoint unless one is a subtype of the other
  // or they share a kind with overlap (e.g., ClassA/ClassB with inheritance).
  return !isBaseSubsetOf(A, ak, B, bk) && !isBaseSubsetOf(B, bk, A, ak)
      && !haveKindOverlap(ak, bk);
}
```

This avoids interning throwaway intersection results.

---

## 5. API Surface

### Type Class (Thin Handle)

`Type` is a thin 4-byte handle. It provides only operations that don't
require the type table: identity comparison, hashing, and well-known
constructors.

```cpp
class Type {
  uint32_t id_;

  constexpr explicit Type(uint32_t id) : id_(id) {}

 public:
  // --- Identity ---
  constexpr bool operator==(Type RHS) const { return id_ == RHS.id_; }
  constexpr bool operator!=(Type RHS) const { return id_ != RHS.id_; }
  llvh::hash_code hash() const { return llvh::hash_value(id_); }

  // --- Well-known constructors (constexpr, no context needed) ---
  static constexpr Type createNoType()       { return Type(kNoTypeId); }
  static constexpr Type createUndefined()    { return Type(kUndefinedId); }
  static constexpr Type createNull()         { return Type(kNullId); }
  static constexpr Type createBoolean()      { return Type(kBooleanId); }
  static constexpr Type createString()       { return Type(kStringId); }
  static constexpr Type createNumber()       { return Type(kNumberId); }
  static constexpr Type createBigInt()       { return Type(kBigIntId); }
  static constexpr Type createSymbol()       { return Type(kSymbolId); }
  static constexpr Type createObject()       { return Type(kObjectId); }
  static constexpr Type createEmpty()        { return Type(kEmptyId); }
  static constexpr Type createUninit()       { return Type(kUninitId); }
  static constexpr Type createEnvironment()  { return Type(kEnvironmentId); }
  static constexpr Type createPrivateName()  { return Type(kPrivateNameId); }
  static constexpr Type createFunctionCode() { return Type(kFunctionCodeId); }
  static constexpr Type createBits32()       { return Type(kBits32Id); }
  static constexpr Type createAnyType()      { return Type(kAnyTypeId); }
  static constexpr Type createNumeric()      { return Type(kNumericId); }

  // Aliases (backwards compatibility, see ir-type-system.md).
  static constexpr Type createInt32()   { return createNumber(); }
  static constexpr Type createUint32()  { return createNumber(); }

  // Pre-allocated well-known union (backwards compatibility).
  static constexpr Type createAnyEmptyUninit() {
    return Type(kAnyEmptyUninitId);
  }

  // --- Context-dependent operations (Phase 1: use thread-local) ---
  // These delegate to TypeContext::current().

  static Type unionTy(Type A, Type B);
  static Type intersectTy(Type A, Type B);
  static Type subtractTy(Type A, Type B);
  bool isSubsetOf(Type t) const;
  bool canBeType(Type t) const;

  // --- Context-dependent queries (Phase 1: use thread-local) ---
  bool isNoType() const;
  bool isAnyType() const;
  bool canBeNumber() const;
  bool canBeString() const;
  bool canBeObject() const;
  bool canBeNull() const;
  bool canBeUndefined() const;
  bool canBeEmpty() const;
  bool canBeUninit() const;
  bool canBeBigInt() const;
  bool canBeBoolean() const;
  bool canBeSymbol() const;
  bool isPrimitive() const;
  bool canBePrimitive() const;
  bool canBeAny() const;
  bool isNonPtr() const;
  bool isNumberType() const;
  bool isStringType() const;
  bool isObjectType() const;
  bool isAnyEmptyUninitType() const;

  // Reimplemented on top of the new representation (delegate to
  // TypeContext::current()):
  //
  // countTypes(): returns 0 for NoType, arm count for Union, 1 otherwise.
  // getFirstTypeKind(): returns the TypeKind from the table entry
  //   (or first arm's kind for Union).
  // Type::iterator: iterates over union arms (or yields the single kind
  //   for non-union types). Delegates to TypeContext::getUnionArms()
  //   for unions.

  void print(llvh::raw_ostream &OS) const;
  void Profile(llvh::FoldingSetNodeID &ID) const { ID.AddInteger(id_); }

  friend class TypeContext;
};
```

### Phase 1: Delegation to Thread-Local

All context-dependent methods delegate to the thread-local context:

```cpp
// Static operations:
Type Type::unionTy(Type A, Type B) {
  return TypeContext::current().unionTy(A, B);
}

// Instance queries:
bool Type::canBeNumber() const {
  return TypeContext::current().canBeNumber(*this);
}

bool Type::isNoType() const {
  return id_ == kNoTypeId;
}
```

Note: some queries like `isNoType()` and `isAnyType()` can remain constexpr
by comparing against well-known IDs. Others like `canBeNumber()` must consult
the context because a Union containing Number should also return true.

### canBeX() Implementation in TypeContext

```cpp
bool TypeContext::canBeNumber(Type t) const {
  // Fast path: well-known IDs.
  if (t.id_ == kAnyTypeId || t.id_ == kNumberId || t.id_ == kNumericId)
    return true;

  TypeKind kind = getKind(t);
  if (kind == TypeKind::Number || kind == TypeKind::Int32 ||
      kind == TypeKind::Uint32)
    return true;
  if (kind == TypeKind::Union) {
    for (Type arm : getUnionArms(t))
      if (canBeNumber(arm))
        return true;
  }
  return false;
}
```

The well-known ID checks avoid table lookups and union arm iteration for
the most common types (`AnyType`, `Number`, `Numeric`). Each `canBeX()`
method has its own set of well-known fast paths. This prevents the
performance regression that would otherwise occur from `AnyType` being
represented as a multi-arm union.

Union arm recursion terminates because unions are always flattened (arms
are never themselves unions).

---

## 6. Class Hierarchy

There is no separate class registry. Class identity is the type table
`id_`, and the inheritance chain is stored directly in the `TypeEntry`
payload (`classInstance.superClassTypeId`).

### Registration

```cpp
Type TypeContext::createClassInstance(
    Type superClassType,
    Identifier className,
    llvh::ArrayRef<ExactObjectField> fields) {
  // ClassInstance entries are NOT interned — each class declaration
  // produces a unique entry, even if structurally identical to another.
  uint32_t id = entries_.size();
  TypeEntry entry{};
  entry.kind = TypeKind::ClassInstance;
  entry.classInstance.superClassTypeId = superClassType.id_;
  entry.classInstance.className = className;
  entry.classInstance.fieldOffset = fieldArrays_.size();
  entry.classInstance.fieldCount = fields.size();
  fieldArrays_.insert(fieldArrays_.end(), fields.begin(), fields.end());
  entries_.push_back(entry);
  return Type(id);
}
```

### Subtype Check

```cpp
bool TypeContext::isSubclass(Type a, Type b) const {
  assert(getKind(a) == TypeKind::ClassInstance);
  assert(getKind(b) == TypeKind::ClassInstance);
  uint32_t cur = a.id_;
  while (cur != kNoTypeId) {
    if (cur == b.id_)
      return true;
    cur = entries_[cur].classInstance.superClassTypeId;
  }
  return false;
}
```

O(depth), typically small. Can be optimized later with DFS numbering if
needed.

---

## 7. Interning

### Within TypeContext (Acyclic Path)

For non-looping types, all type construction methods intern their results.
Since sub-types are `Type` values (= integer indices) and are already
interned before their parent, the intern key is a shallow hash of the
entry's contents — no recursive hashing needed.

For looping (recursive) types, a separate batch allocation protocol is
used — see "Batch Allocation for Looping Types" below.

```cpp
struct TypeEntryKey {
  TypeKind kind;
  // Kind-specific content, using Type id_ values for sub-types.
  // Structurally hashed and compared.
  unsigned hash() const;
  bool operator==(const TypeEntryKey &other) const;
};
```

Example: `Array{elemType: Number}` hashes as `hash(Array, kNumberId)`.

The `internTable_` maps `TypeEntryKey → uint32_t` (type table index).
Duplicate entries are never created.

### From Flow Types

When lowering Flow types to IR types, the three-table interning algorithm
from [ir-type-system-v2-design.md §Interning Algorithm](ir-type-system-v2-design.md#interning-algorithm-flow-types-to-ir-types)
is used:

1. **Identity table**: `flow::Type *` → `ir::Type` (pointer-keyed fast path).
2. **Non-looping dedup table**: `const TypeInfo *` → `ir::Type`, keyed by
   Flow's `TypeInfo::hash()` (intentionally shallow — kind + arity, not
   deep structural) and `TypeInfo::equals()` (deep structural for
   non-looping types). The shallow hash means more collisions, but
   `equals()` resolves them correctly.
3. **Looping dedup table**: `const TypeInfo *` → `ir::Type`, keyed by
   shallow hash + co-inductive equals.

**Important**: the identity table is keyed by `flow::Type *` (the wrapper),
not `TypeInfo *`. The `isLooping` flag that controls routing between tables
2 and 3 lives on the `Type` wrapper (`FlowContext.h:175`), not on `TypeInfo`.
Two different `Type *` wrappers can point to the same `TypeInfo *` with
different `isLooping` flags. The dedup tables (2 and 3) are keyed by
`TypeInfo *` using structural equality, which produces correct results
regardless of `isLooping`.

This interning algorithm *replaces* the current `flowTypeToIRType` static
method (`ESTreeIRGen-typed-class.cpp:410–454`), which does not handle
cycles.

See the design document for the full algorithm and rationale.

### Batch Allocation for Looping Types

The acyclic interning path (§7 "Within TypeContext") assumes sub-types
are already interned before their parent. For looping (recursive) types,
this assumption breaks: the sub-types include the type itself.

Consider `type Tree = null | {left: Tree, right: Tree}`. The cycle is:

```
Union(Null, ExactObject) → ExactObject → field "left" → Union → ...
```

The Union, the ExactObject, and the field reference to Union all need IDs,
but they refer to each other.

#### Construction Protocol

Looping types are constructed in three phases within `TypeContext`:

**Phase A — Reserve IDs.** Walk the cycle cluster (all reachable looping
types not yet in the identity table). For each node, allocate an `entries_`
slot and assign an ID, but leave the entry contents uninitialized
(placeholder):

```cpp
// Reserve an ID without filling in the entry.
uint32_t TypeContext::reserveEntry() {
  uint32_t id = entries_.size();
  entries_.push_back(TypeEntry{});  // placeholder, kind = unset
  return id;
}
```

Record the mapping from each `flow::Type *` to its reserved ID in the
identity table immediately. This ensures that when we encounter the same
type again during the fill phase (the cycle), we find the reserved ID
instead of recursing infinitely.

**Phase B — Fill entries.** Walk the cluster again. For each node, construct
the `TypeEntry` using the reserved IDs for cross-references:

```cpp
// For the ExactObject node in the Tree example:
TypeEntry &entry = entries_[reservedExactObjectId];
entry.kind = TypeKind::ExactObject;
// The field "left" references the Union, whose ID was reserved in Phase A.
entry.exactObject.fieldOffset = fieldArrays_.size();
entry.exactObject.fieldCount = 2;
fieldArrays_.push_back({leftName, Type(reservedUnionId)});
fieldArrays_.push_back({rightName, Type(reservedUnionId)});
```

The key insight: sub-type references use the **reserved** IDs, which are
final — they won't change. The entries are incomplete only in that their
contents haven't been written yet, but their IDs are stable.

Union canonicalization is partially performed during this phase: arms are
deduplicated and sorted by `id_` (all arms have assigned IDs from Phase A).
Subsumption is **skipped** because it would need to inspect entry contents
that may still be placeholders. This is safe because Flow has already
canonicalized its unions — see §8 for details.

**Phase C — Intern.** After all entries are filled, insert each into the
`internTable_`. The key is computed from the now-complete entry contents.
If a structurally identical entry already exists (detected via the looping
dedup table using co-inductive equality in the Flow lowering algorithm),
the reserved entry is wasted — it remains in `entries_` but is never
referenced. This is a minor space cost.

#### Invariants During Construction

- `entries_` may contain **placeholder entries** (kind = unset) between
  Phase A and Phase B. These must never be accessed by type operations.
  Since the looping cluster is being constructed by the lowering pass (not
  by an optimization pass), no concurrent type operations reference these
  IDs until construction completes.

- The `internTable_` does not contain entries for placeholder types. It is
  populated only in Phase C after entries are finalized. This means
  `createArray()`, `createExactObject()`, etc. (the normal acyclic intern
  path) will not find or collide with partially-constructed entries.

- The identity table (`flow::Type *` → `ir::Type`) IS populated during
  Phase A. This is what breaks the cycle: when the fill phase encounters a
  `flow::Type *` that is part of the cluster, it finds the reserved ID in
  the identity table and uses it directly.

#### Example: Interning `Tree = null | {left: Tree, right: Tree}`

```
Phase A (reserve):
  Union  → reserve ID #40, identity[flowUnion]  = Type(40)
  ExactObj → reserve ID #41, identity[flowExact] = Type(41)

Phase B (fill):
  Intern non-looping sub-types:
    Null → already interned (well-known kNullId)
    "left" field type → lookup flowUnion in identity table → Type(40)
    "right" field type → lookup flowUnion in identity table → Type(40)

  Fill entries_[41] (ExactObject):
    kind = ExactObject
    fields = [{left, Type(40)}, {right, Type(40)}]

  Fill entries_[40] (Union):
    arms = [Type(kNullId), Type(41)]
    sort by id_ → [Type(kNullId=4), Type(41)]
    kind = Union

Phase C (intern):
  Insert entries_[40] and entries_[41] into internTable_.
```

---

## 8. Union Canonicalization

Union arms must be in canonical order for interning to produce unique entries.

### Rules

1. **Flatten**: nested unions are expanded into a flat arm list.
2. **Deduplicate**: arms with the same `id_` are removed.
3. **Subsume** (acyclic path only): if arm A is a subtype of arm B, drop A.
   In particular, if unrefined `Object` is an arm, all object refinements
   are dropped. Subsumption requires inspecting the full type entries of
   each arm, so it is only performed when all arms are fully finalized.
   During looping-type batch construction (Phase B in §7), subsumption is
   **skipped** — arms are deduplicated and sorted, but not
   subsumption-reduced. This is safe because looping types come from Flow,
   which has already canonicalized its unions.
4. **Sort**: arms are sorted by `id_`. This is a deterministic total order
   (arbitrary but consistent within a single `TypeContext`).
5. **Collapse**: if one arm remains, return it directly (no Union entry).

### Sorting and Looping Types

Sorting by `id_` sidesteps the looping-type sorting problem: looping types
cannot be sorted by structural comparison (co-inductive comparison doesn't
give a total order), but they CAN be sorted by their interned index. The
interning algorithm assigns IDs before union canonicalization, so all arms
have stable IDs.

---

## 9. Thread-Local Context (Phase 1)

### Motivation

Phase 1 must be API-compatible: existing call sites like
`Type::unionTy(a, b)` and `t.canBeNumber()` must work without passing a
context parameter. The thread-local provides the context implicitly.

### Implementation

```cpp
class TypeContext {
  static thread_local TypeContext *current_;

 public:
  static TypeContext &current() {
    assert(current_ && "No TypeContext set on this thread");
    return *current_;
  }
};

/// RAII guard that sets the thread-local context for the duration of a scope.
class TypeContextRAII {
  TypeContext *prev_;

 public:
  explicit TypeContextRAII(TypeContext &ctx)
      : prev_(TypeContext::current_) {
    TypeContext::current_ = &ctx;
  }
  ~TypeContextRAII() {
    TypeContext::current_ = prev_;
  }
};
```

### Context Lifetime

The TLS context must be set whenever any `Type` operation is used — not just
in optimization passes. Instruction constructors call `unionTy` (e.g.,
`ToPropertyKeyInst` at `Instrs.h:225`), IR construction helpers call it
(e.g., `functionNewTargetType` at `IR.cpp:177`), and Flow type lowering
calls it (e.g., `flowTypeToIRType` at `ESTreeIRGen-typed-class.cpp:432`).

The guard must therefore be installed at Module creation time, so that
all subsequent operations on the Module (IR construction, passes, codegen)
are covered:

```cpp
// At Module creation:
TypeContextRAII guard(module->getTypeContext());
// All Type operations for the lifetime of the Module are now covered.
```

Unit tests that use `Type` algebra directly (e.g.,
`unittests/IR/BuilderTest.cpp:209`) must also install the guard.

**TLS overhead**: on some platforms (notably older Android), `thread_local`
access involves a function call through `__tls_get_addr`. Since every
`canBeX()` and type operation goes through `TypeContext::current()`, code
in tight loops should cache the context reference locally:

```cpp
TypeContext &ctx = TypeContext::current();
for (auto &inst : block) {
  // Use ctx directly, not TypeContext::current() each time.
  if (ctx.canBeNumber(inst.getType())) { ... }
}
```

### Phase 2: Explicit Parameter

All call sites are mechanically transformed to accept an explicit
`TypeContext &`:

```cpp
// Phase 1 (thread-local):
Type result = Type::unionTy(a, b);

// Phase 2 (explicit):
Type result = ctx.unionTy(a, b);
```

Phase 2 is incremental: during the transition, both the explicit-parameter
API and the thread-local API coexist. Call sites are converted one at a
time. The thread-local is removed only at the END of Phase 2, after all
call sites have been converted.

---

## 10. Integration Points

### Value::getType() / setType()

No API changes. `Value` stores a `Type` member (now 4 bytes instead of 2).
The existing size assertion `sizeof(valueType) <= 4` already allows this.

### Inherent Types (Instrs.h)

Instructions with inherent types set them in constructors. Current pattern:

```cpp
// ToPropertyKeyInst constructor:
setType(Type::unionTy(Type::createString(), Type::createSymbol()));
```

This continues to work. `unionTy` now delegates to the thread-local context,
which looks up the pre-allocated entries for `String` and `Symbol` and creates
(or finds) the `Union(String, Symbol)` entry.

**Constraint**: instruction constructors that call `unionTy` or similar must
execute within an `TypeContextRAII` scope. This is ensured by installing
the guard at Module creation time (see §9), which covers all subsequent
instruction creation.

### TypeInference Pass

The TypeInference pass (~76 calls to `unionTy`, ~6 to `subtractTy`, ~30 to
`isSubsetOf`) works unchanged via the thread-local context. The
`TypeContextRAII` guard is installed at the top of `runOnModule`.

### LiteralIRType

`Module` owns a `ValueOFS<LiteralIRType>` folding set that deduplicates
`Type` values used as literal operands. The `Profile` method now uses the
single `id_` field:

```cpp
void Profile(llvh::FoldingSetNodeID &ID) const {
  ID.AddInteger(id_);
}
```

### FlowChecker Lowering

The lowering from Flow types to IR types occurs in the Flow checker / IR
generator:

1. The `Module`'s `TypeContext` is created.
2. The interning algorithm (§7) processes all reachable Flow types.
3. The resulting IR `Type` values are attached to instructions and values.

### Flow Type Lowering Notes

- **ClassConstructorType**: a class constructor is a callable with a known
  signature. Lowered to `Function(constructorSignature)`. Until now lowered
  to unrefined `Object` due to limitations of the old type system.
- **NativeFunction**: native functions are called via specialized
  instructions, not through JS closures. They don't need an IR type
  representation. Currently hardcoded to `Type::createNumber()`
  (`ESTreeIRGen-typed-class.cpp:437`).
- **CPtr**: Flow's internal C pointer type for native interop. Currently
  lowered to `Number`. May warrant its own internal type (like `bits32`)
  in a future phase.
- **UntypedFunction**: functions with no type information. Lowered to
  unrefined `Object` in the IR.

---

## 11. Invariants and Assertions

### Type Invariants

1. **Uniqueness**: two types with the same `id_` are identical. Two types with
   different `id_` values are different. Guaranteed by hash-consing.

2. **Union canonicality**: Union arms are sorted by `id_`, deduplicated,
   subsumption-reduced, and contain at least 2 arms.

3. **No nested unions**: Union arms are never themselves unions (flattened).

4. **Well-known IDs**: IDs below `kFirstDynamicId` are reserved and have
   fixed meanings across all `TypeContext` instances.

5. **Context affinity**: a `Type` value is only meaningful within the
   `TypeContext` that created it. Mixing types from different contexts is
   undefined behavior. Since `Type` is a bare `uint32_t` with no room for a
   context pointer, this invariant cannot be enforced inside `Type` itself.
   It is enforced indirectly: the type table bounds-check
   (`id < entries_.size()`) will catch most violations, and in practice each
   compilation unit uses a single Module/context.

### Debug Assertions

```cpp
// In type table access:
assert(id < entries_.size() && "invalid type ID");

// In union construction:
assert(armCount >= 2 && "union must have at least 2 arms");

// In class subtype check:
assert(getKind(a) == TypeKind::ClassInstance
    && "not a class type");
```

---

## 12. Performance Considerations

### Cost Model

With the pure-index approach, every `canBeNumber()`, `isSubsetOf()`, etc.
requires a table lookup instead of a bitmask test. The table is a
`std::vector` — lookups are a pointer dereference + offset, typically in L1
cache for types used frequently.

For union queries like `canBeNumber()` on `Union(Number, String)`, the
implementation must check each arm. Unions are typically small (2–4 arms), so
this is a short linear scan.

### Future Optimization: Bitmask Cache

If profiling reveals that table lookups are a bottleneck, a bitmask cache can
be added without changing the type algebra:

- Each `TypeEntry` stores a precomputed `uint16_t kindMask` — the set of
  primitive kinds reachable from this type.
- `canBeNumber(t)` becomes `entries_[t.id_].kindMask & NumberBit`.
- This is an implementation optimization layered on top of the clean design.
  It does not affect the public API or the type algebra.

This optimization is explicitly deferred. The clean design comes first;
performance tuning comes after profiling on real workloads.

---

## 13. Summary of Changes by Phase

### Phase 1: Type representation and context (API-compatible)

**Files modified:**
- `include/hermes/IR/IR.h`: `Type` class rewritten (pure index).
- `include/hermes/IR/IR.h`: `Module` class (add `TypeContext` member).
- New: `include/hermes/IR/TypeContext.h`.
- New: `lib/IR/TypeContext.cpp`.
- `Module` constructor: install `TypeContextRAII` guard.
- Unit tests using `Type` algebra: install `TypeContextRAII` guard.

**Semantic changes:** None. All types are well-known primitives. The context
produces identical results to the old bitmask operations. The change is
that operations now go through the type table instead of being direct
bitmask arithmetic, but the results are identical.

### Phase 2: Explicit context parameter (mechanical)

**Files modified:** Every file that calls `unionTy`, `intersectTy`,
`subtractTy`, `isSubsetOf`, `canBeX()`, etc. (~128+ call sites across
~23 files).

**Semantic changes:** None. Thread-local removed, explicit parameter added.

### Phases 3–8: Incremental type enrichment

Each phase adds new `TypeKind` values, extends the type operations, and
adds new query/construction methods to `TypeContext`. Existing methods
continue to work.
