/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_IR_IR_H
#define HERMES_IR_IR_H

#include "hermes/ADT/OwningFoldingSet.h"
#include "hermes/ADT/WordBitSet.h"
#include "hermes/AST/Context.h"
#include "hermes/AST/NativeContext.h"
#include "hermes/Support/Conversions.h"
#include "hermes/Support/ScopeChain.h"

#ifndef HERMESVM_LEAN
#include "hermes/AST/ESTree.h"
#endif

#include "llvh/ADT/Hashing.h"
#include "llvh/ADT/SmallPtrSet.h"
#include "llvh/ADT/SmallVector.h"
#include "llvh/ADT/StringRef.h"
#include "llvh/ADT/Twine.h"
#include "llvh/ADT/ilist_node.h"
#include "llvh/ADT/iterator_range.h"
#include "llvh/IR/SymbolTableListTraits.h"
#include "llvh/Support/Casting.h"
#include "llvh/Support/MathExtras.h"
#include "llvh/Support/raw_ostream.h"

#include <deque>
#include <unordered_map>
#include <vector>

namespace hermes {

class Module;
class VariableScope;
class Function;
class BasicBlock;
class JSDynamicParam;
class Instruction;
class Context;
class TerminatorInst;

/// Representation of a type in the IR. This roughly corresponds for JavaScript
/// types, but represents lower level concepts like "empty" type for TDZ and
/// integers.
class Type {
 public:
  // Encodes the JavaScript type hierarchy.
  enum TypeKind {
    /// An TDZ variable before its declaration.
    Empty,
    /// A typed variable after its declaration, but before initialization.
    /// At runtime this maps to undefined.
    Uninit,
    Undefined,
    Null,
    Boolean,
    String,
    Number,
    BigInt,
    Environment,
    /// Function code (IR Function value), not a closure.
    FunctionCode,
    Object,

    LAST_TYPE
  };

 private:
  static_assert(LAST_TYPE <= 16, "Type tag must fit in 16 bits");

  /// Return the string representation of the type at index \p idx.
  llvh::StringRef getKindStr(TypeKind idx) const {
    // The strings below match the values in TypeKind.
    static const char *const names[] = {
        "empty",
        "uninit",
        "undefined",
        "null",
        "boolean",
        "string",
        "number",
        "bigint",
        "environment",
        "functionCode",
        "object"};
    return names[idx];
  }

#define BIT_TO_VAL(XX) (1 << TypeKind::XX)
#define IS_VAL(XX) (bitmask_ == (1 << TypeKind::XX))

#define NUM_BIT_TO_VAL(XX) (1 << NumTypeKind::XX)
#define NUM_IS_VAL(XX) (numBitmask_ == (1 << NumTypeKind::XX))

  // All possible types including "empty" and "uninit", but not including
  // special internal types that are never mixed with other types.
  static constexpr uint16_t TYPE_ANY_EMPTY_UNINIT_MASK =
      ((1u << TypeKind::LAST_TYPE) - 1) & ~BIT_TO_VAL(Environment) &
      ~BIT_TO_VAL(FunctionCode);
  // All of the above types except "empty" and "uninit".
  static constexpr uint16_t TYPE_ANY_MASK =
      TYPE_ANY_EMPTY_UNINIT_MASK & ~BIT_TO_VAL(Empty) & ~BIT_TO_VAL(Uninit);

  static constexpr uint16_t PRIMITIVE_BITS = BIT_TO_VAL(Number) |
      BIT_TO_VAL(String) | BIT_TO_VAL(BigInt) | BIT_TO_VAL(Null) |
      BIT_TO_VAL(Undefined) | BIT_TO_VAL(Boolean);

  static constexpr uint16_t NONPTR_BITS = BIT_TO_VAL(Number) |
      BIT_TO_VAL(Boolean) | BIT_TO_VAL(Null) | BIT_TO_VAL(Undefined);

  /// Each bit represent the possibility of the type being the type that's
  /// represented in the enum entry.
  uint16_t bitmask_{0};

  /// The constructor is only accessible by static builder methods.
  constexpr explicit Type(uint16_t mask) : bitmask_(mask) {}

 public:
  static constexpr Type unionTy(Type A, Type B) {
    return Type(A.bitmask_ | B.bitmask_);
  }

  static constexpr Type intersectTy(Type A, Type B) {
    // This is sound but not complete, but this is only used for disjointness
    // check.
    return Type(A.bitmask_ & B.bitmask_);
  }

  static constexpr Type subtractTy(Type A, Type B) {
    return Type(A.bitmask_ & ~B.bitmask_);
  }

  static constexpr Type createNoType() {
    return Type(0);
  }
  static constexpr Type createAnyEmptyUninit() {
    return Type(TYPE_ANY_EMPTY_UNINIT_MASK);
  }
  static constexpr Type createAnyType() {
    return Type(TYPE_ANY_MASK);
  }
  /// Create an uninitialized TDZ type.
  static constexpr Type createEmpty() {
    return Type(BIT_TO_VAL(Empty));
  }
  static constexpr Type createUninit() {
    return Type(BIT_TO_VAL(Uninit));
  }
  static constexpr Type createUndefined() {
    return Type(BIT_TO_VAL(Undefined));
  }
  static constexpr Type createNull() {
    return Type(BIT_TO_VAL(Null));
  }
  static constexpr Type createBoolean() {
    return Type(BIT_TO_VAL(Boolean));
  }
  static constexpr Type createString() {
    return Type(BIT_TO_VAL(String));
  }
  static constexpr Type createObject() {
    return Type(BIT_TO_VAL(Object));
  }
  static constexpr Type createNumber() {
    return Type(BIT_TO_VAL(Number));
  }
  /// This is just an alias of createNumber(). We used to track whether a
  /// number was known to be an integer, but we don't anymore. Still, we don't
  /// want to lose the callsite information, so we keep this alias.
  static constexpr Type createInt32() {
    return createNumber();
  }
  /// This is just an alias of createNumber(). We used to track whether a
  /// number was known to be an integer, but we don't anymore. Still, we don't
  /// want to lose the callsite information, so we keep this alias.
  static constexpr Type createUint32() {
    return createNumber();
  }
  static constexpr Type createBigInt() {
    return Type(BIT_TO_VAL(BigInt));
  }
  static constexpr Type createNumeric() {
    return unionTy(createNumber(), createBigInt());
  }
  static constexpr Type createEnvironment() {
    return Type(BIT_TO_VAL(Environment));
  }
  static constexpr Type createFunctionCode() {
    return Type(BIT_TO_VAL(FunctionCode));
  }

  constexpr bool isNoType() const {
    return bitmask_ == 0;
  }

  constexpr bool isAnyEmptyUninitType() const {
    return bitmask_ == TYPE_ANY_EMPTY_UNINIT_MASK;
  }
  constexpr bool isAnyType() const {
    return bitmask_ == TYPE_ANY_MASK;
  }

  constexpr bool isEmptyType() const {
    return IS_VAL(Empty);
  }
  constexpr bool isUninitType() const {
    return IS_VAL(Uninit);
  }
  constexpr bool isUndefinedType() const {
    return IS_VAL(Undefined);
  }
  constexpr bool isNullType() const {
    return IS_VAL(Null);
  }
  constexpr bool isBooleanType() const {
    return IS_VAL(Boolean);
  }
  constexpr bool isStringType() const {
    return IS_VAL(String);
  }
  constexpr bool isObjectType() const {
    return IS_VAL(Object);
  }
  constexpr bool isNumberType() const {
    return IS_VAL(Number);
  }
  constexpr bool isBigIntType() const {
    return IS_VAL(BigInt);
  }
  constexpr bool isEnvironmentType() const {
    return IS_VAL(Environment);
  }
  constexpr bool isFunctionCodeType() const {
    return IS_VAL(FunctionCode);
  }

  /// \return the TypeKind of the first set bit. This is intended to be used
  /// when there is single type set. If there are no types, it returns
  /// LAST_TYPE.
  TypeKind getFirstTypeKind() const {
    auto res = LLVM_LIKELY(bitmask_)
        ? (TypeKind)llvh::countTrailingZeros(bitmask_, llvh::ZB_Undefined)
        : TypeKind::LAST_TYPE;
    assert(res <= LAST_TYPE && "Invalid bitmask");
    return res;
  }

  /// \return how many valid types are represented by this (union) type.
  unsigned countTypes() const {
    return llvh::countPopulation(bitmask_);
  }

  /// \return true if the type is one of the known javascript primitive types:
  /// Number, BigInt, Null, Boolean, String, Undefined.
  constexpr bool isKnownPrimitiveType() const {
    return isPrimitive() && 1 == countTypes();
  }

  constexpr bool isPrimitive() const {
    // Check if any bit except the primitive bits is on.
    return bitmask_ && !(bitmask_ & ~PRIMITIVE_BITS);
  }

  /// \return true if the type is not referenced by a pointer in javascript.
  constexpr bool isNonPtr() const {
    // One or more of NONPTR_BITS must be set, and no other bit must be set.
    return bitmask_ && !(bitmask_ & ~NONPTR_BITS);
  }
#undef BIT_TO_VAL
#undef IS_VAL
#undef NUM_BIT_TO_VAL
#undef NUM_IS_VAL

  /// \returns true if this type is a subset of \p t.
  constexpr bool isSubsetOf(Type t) const {
    return !(bitmask_ & ~t.bitmask_);
  }

  /// \returns true if the type \p t can be any of the types that this type
  /// represents. For example, if this type is "string|number" and \p t is
  /// a string the result is true because this type can represent strings.
  constexpr bool canBeType(Type t) const {
    return t.isSubsetOf(*this);
  }

  /// \returns true if this type can represent a string value.
  constexpr bool canBeString() const {
    return canBeType(Type::createString());
  }

  /// \returns true if this type can represent a bigint value.
  constexpr bool canBeBigInt() const {
    return canBeType(Type::createBigInt());
  }

  /// \returns true if this type can represent a number value.
  constexpr bool canBeNumber() const {
    return canBeType(Type::createNumber());
  }

  /// \returns true if this type can represent an object.
  constexpr bool canBeObject() const {
    return canBeType(Type::createObject());
  }

  /// \returns true if this type can represent a boolean value.
  constexpr bool canBeBoolean() const {
    return canBeType(Type::createBoolean());
  }

  /// \returns true if this type can represent an "empty" value.
  constexpr bool canBeEmpty() const {
    return canBeType(Type::createEmpty());
  }

  /// \returns true if this type can represent an "uninit" value.
  constexpr bool canBeUninit() const {
    return canBeType(Type::createUninit());
  }

  /// \returns true if this type can represent an undefined value.
  constexpr bool canBeUndefined() const {
    return canBeType(Type::createUndefined());
  }

  /// \returns true if this type can represent a null value.
  constexpr bool canBeNull() const {
    return canBeType(Type::createNull());
  }

  /// \returns true if this type can represent an "any" type value.
  constexpr bool canBeAny() const {
    return canBeType(Type::createAnyType());
  }

  /// Return true if this type is a proper subset of \p t. A "proper subset"
  /// means that it is a subset bit is not equal.
  constexpr bool isProperSubsetOf(Type t) const {
    return bitmask_ != t.bitmask_ && !(bitmask_ & ~t.bitmask_);
  }

  void print(llvh::raw_ostream &OS) const;

  /// The hash of a Type is the hash of its opaque value.
  llvh::hash_code hash() const {
    return llvh::hash_value(bitmask_);
  }

  constexpr bool operator==(Type RHS) const {
    return bitmask_ == RHS.bitmask_;
  }
  constexpr bool operator!=(Type RHS) const {
    return !(*this == RHS);
  }

  class iterator;

  /// Return an iterator over the types in this Type.
  iterator begin() const;
  /// Return an "end" iterator over the types in this Type.
  iterator end() const;

  /// Allow Type to be used as a llvh::FoldingSet.
  void Profile(llvh::FoldingSetNodeID &ID) const {
    ID.AddInteger(bitmask_);
  }
};

static_assert(sizeof(Type) == 2, "Type must not be too big");

/// An iterator over the types in a Type.
class Type::iterator {
  friend class Type;
  Type type_;
  unsigned index_;

  iterator(Type type, unsigned index) : type_(type), index_(index) {
    skip();
  }

  /// Skip to the first set bit in the bitmask.
  void skip() {
    while (index_ < sizeof(type_.bitmask_) * CHAR_BIT &&
           !(type_.bitmask_ & (1 << index_))) {
      ++index_;
    }
  }

 public:
  bool operator==(const iterator &RHS) const {
    return type_ == RHS.type_ && index_ == RHS.index_;
  }
  bool operator!=(const iterator &RHS) const {
    return !(*this == RHS);
  }

  iterator &operator++() {
    assert(index_ < sizeof(type_.bitmask_) * CHAR_BIT && "Out of bounds");
    ++index_;
    skip();
    return *this;
  }
  iterator operator++(int) {
    auto copy = *this;
    ++*this;
    return copy;
  }

  Type operator*() const {
    assert(index_ < sizeof(type_.bitmask_) * CHAR_BIT && "Out of bounds");
    return Type(1 << index_);
  }
};

inline Type::iterator Type::begin() const {
  return iterator(*this, 0);
}
inline Type::iterator Type::end() const {
  return iterator(*this, sizeof(bitmask_) * CHAR_BIT);
}
} // namespace hermes

namespace llvh {
template <>
struct FoldingSetTrait<hermes::Type> {
  static inline void Profile(hermes::Type t, FoldingSetNodeID &ID) {
    t.Profile(ID);
  }
};
} // namespace llvh

namespace hermes {

/// Describes the potential side effects of an instruction. The side effects are
/// described by a series of bits, each of which specifies a particular way in
/// which this instruction may observe/modify the state of the world, or
/// otherwise restrict its movement/deletion.
class SideEffect {
 public:
  SideEffect() {
    memset(this, 0, sizeof(SideEffect));
  }

  /// Define each of the fields in a SideEffect.
  /// \c ReadStack and \c WriteStack imply that an instruction may read/write to
  /// a stack location. Such an instruction must take an \c AllocStackInst
  /// operand which it can read/write to.
  /// \c ReadFrame and \c WriteFrame imply that an instruction may read/write
  /// variables in the frame.
  /// \c ReadHeap and \c WriteHeap imply that an instruction may read/write
  /// objects and other values on the heap.
  /// \c Throw implies that an instruction may throw an exception.
  /// \c ExecuteJS implies that an instruction may execute arbitrary user
  /// provided JS. An instruction with \c ExecuteJS set must also have bits set
  /// for throwing and accessing the frame and heap.
  /// \c FirstInBlock implies that an instruction must be at the start of a
  /// basic block. An instruction with \c FirstInBlock set must precede all
  /// instructions that do not have it set.
  /// \c Idempotent implies that repeated execution of an instruction will not
  /// affect the state of the world or the result of the instruction. For
  /// example, two identical instructions that have \p Idempotent set may be
  /// merged if they are only separated by instructions that have no side
  /// effects.

#define SIDE_EFFECT_FIELDS \
  FIELD(ReadStack)         \
  FIELD(WriteStack)        \
  FIELD(ReadFrame)         \
  FIELD(WriteFrame)        \
  FIELD(ReadHeap)          \
  FIELD(WriteHeap)         \
  FIELD(Throw)             \
  FIELD(ExecuteJS)         \
  FIELD(FirstInBlock)      \
  FIELD(Idempotent)

#define FIELD(field)        \
  bool get##field() const { \
    return f##field##_;     \
  }
  SIDE_EFFECT_FIELDS
#undef FIELD

#define FIELD(field)         \
  SideEffect &set##field() { \
    f##field##_ = true;      \
    return *this;            \
  }
  SIDE_EFFECT_FIELDS
#undef FIELD

  /// Create side effects for an instruction that may execute user JS. Executing
  /// JS implies several other side-effects, so this helps populate them.
  static SideEffect createExecute() {
    return SideEffect{}
        .setReadFrame()
        .setReadHeap()
        .setWriteFrame()
        .setWriteHeap()
        .setExecuteJS()
        .setThrow();
  }

  /// Create side effects for an instruction that may have arbitrary side
  /// effects, and should not be deleted or reordered. For now this is just an
  /// alias for createExecute.
  static SideEffect createUnknown() {
    return createExecute();
  }

  /// Determine whether the given instruction modifies the state of the world in
  /// any an observable way.
  bool hasSideEffect() const {
    // Note that we do not check ExecuteJS here because any observable
    // side-effect of a call requires one of these other bits to be set.
    return getWriteStack() || getWriteHeap() || getWriteFrame() || getThrow();
  }

  /// Determine if the instruction is pure. That is, whether separate
  /// invocations of the instruction with the same operands will always yield
  /// the same result, and will not modify the state of the world in any
  /// observable way. Note that this excludes instructions that allocate (e.g.
  /// AllocObject, CreateFunction) since they produce a different reference each
  /// time.
  bool isPure() const {
    return !hasSideEffect() && !getReadStack() && !getReadHeap() &&
        !getReadFrame() && getIdempotent();
  }

  /// Helper functions to expose the SideEffectKind interface.
  /// TODO: Delete these once all instructions are migrated off SideEffectKind.
  bool mayExecute() const {
    return getThrow() || getExecuteJS();
  }
  bool mayWriteOrWorse() const {
    return getWriteStack() || getWriteFrame() || getWriteHeap() || mayExecute();
  }
  bool mayReadOrWorse() const {
    return getReadStack() || getReadFrame() || getReadHeap() ||
        mayWriteOrWorse();
  }

  /// Check if these side effects are well formed. For now, this just checks
  /// that we correctly set/unset bits when an instruction is marked as being
  /// able to execute.
  bool isWellFormed() const {
    if (getExecuteJS()) {
      // If an instruction may execute, it cannot be idempotent, and could throw
      // or access the frame/heap.
      if (getIdempotent() || !getReadFrame() || !getReadHeap() ||
          !getWriteFrame() || !getWriteHeap() || !getThrow())
        return false;
    }
    return true;
  }

 private:
#define FIELD(field) bool f##field##_ : 1;
  SIDE_EFFECT_FIELDS
#undef FIELD
#undef SIDE_EFFECT_FIELDS
};

enum class ValueKind : uint8_t {
  First_ValueKind,
#define DEF_VALUE(CLASS, PARENT) CLASS##Kind,
#define BEGIN_VALUE(CLASS, PARENT) First_##CLASS##Kind,
#define DEF_TAG(NAME, PARENT) NAME##Kind,
#define END_VALUE(CLASS) Last_##CLASS##Kind,
#define MARK_VALUE(CLASS) CLASS##Kind,
#define MARK_FIRST(CLASS, PARENT) First_##CLASS##Kind,
#define MARK_LAST(CLASS) Last_##CLASS##Kind,
#include "hermes/IR/ValueKinds.def"
  Last_ValueKind,
};

static inline bool kindInRange(ValueKind kind, ValueKind from, ValueKind to) {
  return kind > from && kind < to;
}

/// Return true if the specified kind falls in the range of the specified class.
#define HERMES_IR_KIND_IN_CLASS(kind, CLASS) \
  kindInRange(                               \
      kind, ValueKind::First_##CLASS##Kind, ValueKind::Last_##CLASS##Kind)

/// Return the numeric offset of the specified kind from the first kind in the
/// class.
#define HERMES_IR_KIND_TO_OFFSET(CLASS, kind) \
  ((int)(kind) - (int)ValueKind::First_##CLASS##Kind - 1)

/// Convert from an offset to a ValueKind inside a class.
#define HERMES_IR_OFFSET_TO_KIND(CLASS, offset) \
  ((ValueKind)((int)ValueKind::First_##CLASS##Kind + (offset) + 1))

/// Return number of values in an IR class.
#define HERMES_IR_CLASS_LENGTH(CLASS) \
  ((int)ValueKind::Last_##CLASS##Kind - (int)ValueKind::First_##CLASS##Kind - 1)

/// A linked list of function scopes provided as context during IRGen.
/// This how e.g. the debugger can provide information that an identifier 'foo'
/// should be captured from a function two levels down the lexical stack.
class SerializedScope {
 public:
  /// Parent scope, if any.
  std::shared_ptr<const SerializedScope> parentScope;
  /// Original name of the function, if any.
  Identifier originalName;
  /// The generated name of the variable holding the function in the parent's
  /// frame, which is what we need to look up to reference ourselves. It is only
  /// set if there is an alias binding from \c originalName (which must be
  /// valid) and said variable, which must have a different name (since it is
  /// generated). Function::lazyClosureAlias_.
  Identifier closureAlias;
  /// List of variable names in the frame.
  llvh::SmallVector<Identifier, 16> variables;
};

#ifndef HERMESVM_LEAN
/// The source of a lazy AST node.
struct LazySource {
  /// The type of node (such as a FunctionDeclaration or FunctionExpression).
  ESTree::NodeKind nodeKind{ESTree::NodeKind::Empty};
  /// The source buffer id in which this function can be find.
  uint32_t bufferId{0};
  /// The range of the function within the buffer (the whole function node, not
  /// just the lazily parsed body).
  SMRange functionRange;
  /// The Yield param to restore when eagerly parsing.
  bool paramYield{false};
  /// The Await param to restore when eagerly parsing.
  bool paramAwait{false};
};
#endif

/// A set of attributes to be associated with Values.
union Attributes {
  struct {
#define ATTRIBUTE(_valueKind, name, _string) uint16_t name : 1;
#include "hermes/IR/Attributes.def"
  };

  uint16_t flags_;

  /// No attributes on construction.
  explicit Attributes() : flags_(0) {
    // Make sure the bits can fit into flags_.
    static_assert(sizeof(Attributes) == sizeof(flags_), "too many attributes");
  }

  /// \return true if there are no attributes on the function.
  bool isEmpty() const {
    return flags_ == 0;
  }

  /// Clear all attributes on the function.
  void clear() {
    flags_ = 0;
  }

  /// \return a string describing the attributes if there are any.
  /// If there are no attributes, returns "".
  /// If there are attributes returns, e.g. "[allCallsitesKnownInStrictMode]".
  std::string getDescriptionStr() const;
};

class Value {
 public:
  using UseListTy = llvh::SmallVector<Instruction *, 2>;
  using Use = std::pair<Value *, unsigned>;

 private:
  // We declare operator delete as a private member below. Classes declared
  // as friend below invokes constructor of Value subtypes directly.
  // C++ requires that if an exception is raised during construction of
  // a new object, the object is deleted.
  // As a result, delete must be accessible to callers of constructors.
  // On some (but not all) compilers, the -fno-exceptions compiler flag
  // removes this requirement.
  friend class Module;
  friend class IRBuilder;

  ValueKind Kind;

  /// Bitset of the attributes associated with this value.
  /// If this exceeds 2 bytes, the actual storage should be moved to Module,
  /// and this field replaced with a bit which indicates whether the storage
  /// exists.
  /// ValueKind is 1 byte and Type is 4 bytes, so we can store one uint16_t here
  /// without increasing the size of Value.
  Attributes attributes_;

  /// The JavaScript type of the value.
  Type valueType = Type::createAnyType();

  /// A list of users of this instruction.
  UseListTy Users;

  // Instances of Value are not supposed to be deleted directly because we
  // want to avoid defining a virtual destructor. The private operator
  // delete can only be invoked by the \c destroy() method.

  // Use sized deallocation to speed things up, if it is available
#ifdef __cpp_sized_deallocation
  void operator delete(void *p, size_t s) {
    ::operator delete(p, s);
  }
#else
  void operator delete(void *p) {
    ::operator delete(p);
  }
#endif

 protected:
  explicit Value(ValueKind k) {
    static_assert(sizeof(Kind) == 1, "ValueKind too big");
    static_assert(sizeof(attributes_) == 2, "attributes_ increases Value size");
    static_assert(
        sizeof(valueType) <= 4,
        "Type aligning to 4 bytes allows attributes_ to not increase Value size");
    Kind = k;
  }

 public:
  Value(const Value &) = delete;
  void operator=(const Value &) = delete;

  /// Run a Value's destructor and deallocate its memory.
  static void destroy(Value *V);

  /// \return the users of the value.
  const UseListTy &getUsers() const;

  /// \returns the number of users the value has.
  unsigned getNumUsers() const;

  /// \returns true if the value has some users.
  bool hasUsers() const;

  /// \returns true if the value has only one user.
  bool hasOneUser() const;

  /// Remove the first occurrence of \p Inst from the Users list.
  void removeUse(Use U);

  /// Add the instruction \p Inst to the Users list.
  /// \returns the use handle.
  Value::Use addUser(Instruction *Inst);

  /// Replaces all uses of the current value with \p Other.
  void replaceAllUsesWith(Value *Other);

  /// \returns the kind of the value.
  ValueKind getKind() const {
    return Kind;
  }

  /// \returns the string representation of the Value kind.
  llvh::StringRef getKindStr() const;

  /// Sets a new type \p type to the value.
  ///
  /// Types for Instructions should only be set in the constructors
  /// or during the TypeInference pass.
  /// The only instructions which set the type in the constructor are
  /// those with inherent types (see \c Instruction::getInherentType),
  /// and those which are simply copying the types from a single operand
  /// (e.g. MovInst, HBCLoadConstInst).
  /// All other Instruction types will be set during inference in TypeInference.
  void setType(Type type) {
#ifndef NDEBUG
    if (llvh::isa<Function>(this)) {
      assert(
          type.isFunctionCodeType() &&
          "Functions cannot have non-functionCode types");
    }
#endif
    valueType = type;
  }

  /// \returns the JavaScript type of the value.
  Type getType() const {
    return valueType;
  }

  /// Takes a Module parameter to allow easily moving the attributes_ storage
  /// into Module if necessary in the future.
  /// Will create default Attributes if they don't exist already.
  /// \return the Attributes of this value.
  Attributes &getAttributesRef(const Module *) {
    return attributes_;
  }
  /// Takes a Module parameter to allow easily moving the attributes_ storage
  /// into Module if necessary in the future.
  /// \return the Attributes by value, because nonexistent attributes might not
  ///   be stored anywhere if/when we use a side table.
  Attributes getAttributes(const Module *) const {
    return attributes_;
  }

  static bool classof(const Value *) {
    return true;
  }

  using iterator = UseListTy::iterator;
  using const_iterator = UseListTy::const_iterator;

  inline const_iterator users_begin() const {
    return Users.begin();
  }
  inline const_iterator users_end() const {
    return Users.end();
  }
  inline iterator users_begin() {
    return Users.begin();
  }
  inline iterator users_end() {
    return Users.end();
  }
};

/// Utility class to create elements that are subclasses of Value in the
/// folding set. This is necessary because calling new on a type requires
/// access to its deleter when exceptions are enabled, but the deleter of
/// Value is private.
template <typename T>
struct ValueCreator {
  template <typename... Args>
  static T *create(Args &&...args) {
    return new T(std::forward<Args>(args)...);
  }
};

/// Deleter for Values.
struct ValueDeleter {
  void operator()(Value *V) {
    Value::destroy(V);
  }
};

/// This represents a function parameter.
class Parameter : public Value {
  friend class Function;
  Parameter(const Parameter &) = delete;
  void operator=(const Parameter &) = delete;

  /// The function that contains this paramter.
  Function *parent_;

  /// The formal name of the parameter
  Identifier name_;

  explicit Parameter(Function *parent, Identifier name)
      : Value(ValueKind::ParameterKind), parent_(parent), name_(name) {
    assert(parent_ && "Invalid parent");
  }

 public:
  Function *getParent() const {
    return parent_;
  }
  /// \brief Return a constant reference to the value's name.
  Identifier getName() const {
    return name_;
  }
  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::ParameterKind;
  }
};

/// This represents a JS function parameter, all of which are optional.
class JSDynamicParam : public Value {
  friend class Function;
  friend class IRBuilder;
  JSDynamicParam(const JSDynamicParam &) = delete;
  void operator=(const JSDynamicParam &) = delete;

  /// The function that contains this paramter.
  Function *parent_;

  /// The formal name of the parameter
  Identifier name_;

  explicit JSDynamicParam(Function *parent, Identifier name)
      : Value(ValueKind::JSDynamicParamKind), parent_(parent), name_(name) {
    assert(parent_ && "Invalid parent");
  }

 public:
  Context &getContext() const;

  Function *getParent() const {
    return parent_;
  }

  /// \brief Return a constant reference to the value's name.
  Identifier getName() const {
    return name_;
  }

  /// Return the index of this parameter in the function's parameter list.
  /// "this" parameter is excluded from the list.
  uint32_t getIndexInParamList() const;

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::JSDynamicParamKind;
  }
};

/// This represent an empty or missing value.
class EmptySentinel : public Value {
  EmptySentinel(const EmptySentinel &) = delete;
  void operator=(const EmptySentinel &) = delete;

 public:
  explicit EmptySentinel() : Value(ValueKind::EmptySentinelKind) {}

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::EmptySentinelKind;
  }
};

/// This represents a label, which is a thin wrapper around an identifier.
class Label : public Value {
  Label(const Label &) = delete;
  void operator=(const Label &) = delete;

  // Label is "special" it is never created separately - it is only embedded
  // in existing instructions. To prevent accidents we list them here as
  // friends.

  void *operator new(size_t) {
    llvm_unreachable("Labels cannot be allocated separately");
  }

  /// The formal name of the parameter
  Identifier text;

 public:
  explicit Label(Identifier txt) : Value(ValueKind::LabelKind), text(txt) {}

  Identifier get() const {
    return text;
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::LabelKind;
  }
};

class Literal : public Value {
  Literal(const Literal &) = delete;
  void operator=(const Literal &) = delete;

 public:
  explicit Literal(ValueKind k) : Value(k) {}

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), Literal);
  }
};

class LiteralEmpty : public Literal {
  LiteralEmpty(const LiteralEmpty &) = delete;
  void operator=(const LiteralEmpty &) = delete;

 public:
  explicit LiteralEmpty() : Literal(ValueKind::LiteralEmptyKind) {
    setType(Type::createEmpty());
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::LiteralEmptyKind;
  }
};

class LiteralUninit : public Literal {
  LiteralUninit(const LiteralUninit &) = delete;
  void operator=(const LiteralUninit &) = delete;

 public:
  explicit LiteralUninit() : Literal(ValueKind::LiteralUninitKind) {
    setType(Type::createUninit());
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::LiteralUninitKind;
  }
};

class LiteralNull : public Literal {
  LiteralNull(const LiteralNull &) = delete;
  void operator=(const LiteralNull &) = delete;

 public:
  explicit LiteralNull() : Literal(ValueKind::LiteralNullKind) {
    setType(Type::createNull());
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::LiteralNullKind;
  }
};

class LiteralUndefined : public Literal {
  LiteralUndefined(const LiteralUndefined &) = delete;
  void operator=(const LiteralUndefined &) = delete;

 public:
  explicit LiteralUndefined() : Literal(ValueKind::LiteralUndefinedKind) {
    setType(Type::createUndefined());
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::LiteralUndefinedKind;
  }
};

class LiteralBigInt : public Literal, public llvh::FoldingSetNode {
  LiteralBigInt(const LiteralBigInt &) = delete;
  LiteralBigInt &operator=(const LiteralBigInt &) = delete;

  // value holds the BigInt literal string as parsed by the front-end.
  UniqueString *value;

 public:
  explicit LiteralBigInt(UniqueString *v)
      : Literal(ValueKind::LiteralBigIntKind), value(v) {
    setType(Type::createBigInt());
  }

  UniqueString *getValue() const {
    return value;
  }

  static void Profile(llvh::FoldingSetNodeID &ID, UniqueString *value) {
    ID.AddPointer(value);
  }

  void Profile(llvh::FoldingSetNodeID &ID) const {
    LiteralBigInt::Profile(ID, value);
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::LiteralBigIntKind;
  }
};

class LiteralNumber : public Literal, public llvh::FoldingSetNode {
  LiteralNumber(const LiteralNumber &) = delete;
  void operator=(const LiteralNumber &) = delete;
  double value;

 public:
  double getValue() const {
    return value;
  }

  /// Check whether the number can be represented in integer
  /// of type T without losing any precision or information.
  /// If not representible, \returns llvh::None.
  template <typename T>
  llvh::Optional<T> isIntTypeRepresentible() const {
    // Check the value is within the bounds of T.
    // Converting double to int when it's out of bound is undefined behavior.
    // Although it is harmless in this scenario, we should still avoid it.
    if (value > std::numeric_limits<T>::max() ||
        value < std::numeric_limits<T>::min()) {
      return llvh::None;
    }
    if (std::isnan(value)) {
      return llvh::None;
    }
    // Check the value is integer and is not -0.
    T valAsInt = static_cast<T>(value);
    if (valAsInt == value && (valAsInt || !std::signbit(value))) {
      return valAsInt;
    }
    return llvh::None;
  }

  /// Check whether the number is positive zero.
  bool isPositiveZero() const {
    return value == 0.0 && !std::signbit(value);
  }

  /// Check whether the number is negative zero.
  bool isNegativeZero() const {
    return value == 0.0 && std::signbit(value);
  }

  /// Check whether the number can be represented in unsigned 8-bit integer
  /// without losing any precision or information.
  bool isUInt8Representible() const {
    return isIntTypeRepresentible<uint8_t>().hasValue();
  }

  /// Check whether the number can be represented in 32-bit integer
  /// without losing any precision or information.
  bool isInt32Representible() const {
    return isIntTypeRepresentible<int32_t>().hasValue();
  }

  /// Check whether the number can be represented in unsigned 32-bit integer
  /// without losing any precision or information.
  bool isUInt32Representible() const {
    return isIntTypeRepresentible<uint32_t>().hasValue();
  }

  /// Convert the number to uint8_t without losing precision. If not doable,
  /// assertion will fail.
  uint32_t asUInt8() const {
    auto tmp = isIntTypeRepresentible<uint8_t>();
    assert(tmp && "Cannot convert to uint8_t");
    return tmp.getValue();
  }

  /// Convert the number to int32_t without losing precision. If not doable,
  /// assertion will fail.
  int32_t asInt32() const {
    auto tmp = isIntTypeRepresentible<int32_t>();
    assert(tmp && "Cannot convert to int32_t");
    return tmp.getValue();
  }

  /// Convert the number to uint32_t without losing precision. If not doable,
  /// assertion will fail.
  uint32_t asUInt32() const {
    auto tmp = isIntTypeRepresentible<uint32_t>();
    assert(tmp && "Cannot convert to uint32_t");
    return tmp.getValue();
  }

  /// Convert the number to an 32-bit integer according to the rules specified
  /// in ES5.1 section 9.5. This is notably different from what a C/C++ cast
  /// to integer would return for numbers larger than 2*32.
  int32_t truncateToInt32() const {
    return hermes::truncateToInt32(value);
  }

  uint32_t truncateToUInt32() const {
    return hermes::truncateToUInt32(value);
  }

  /// Attempt to convert to an array index.
  OptValue<uint32_t> convertToArrayIndex() const {
    return doubleToArrayIndex(value);
  }

  explicit LiteralNumber(double val)
      : Literal(ValueKind::LiteralNumberKind), value(val) {
    if (isInt32Representible()) {
      setType(Type::createInt32());
    } else {
      setType(Type::createNumber());
    }
  }

  static void Profile(llvh::FoldingSetNodeID &ID, double value) {
    ID.AddInteger(llvh::DoubleToBits(value));
  }

  void Profile(llvh::FoldingSetNodeID &ID) const {
    LiteralNumber::Profile(ID, value);
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::LiteralNumberKind;
  }
};

class LiteralString : public Literal, public llvh::FoldingSetNode {
  LiteralString(const LiteralString &) = delete;
  void operator=(const LiteralString &) = delete;
  Identifier value;

 public:
  Identifier getValue() const {
    return value;
  }

  explicit LiteralString(Identifier val)
      : Literal(ValueKind::LiteralStringKind), value(val) {
    setType(Type::createString());
  }

  static void Profile(llvh::FoldingSetNodeID &ID, Identifier value) {
    ID.AddPointer(value.getUnderlyingPointer());
  }

  void Profile(llvh::FoldingSetNodeID &ID) const {
    LiteralString::Profile(ID, value);
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::LiteralStringKind;
  }
};

class LiteralBool : public Literal {
  LiteralBool(const LiteralBool &) = delete;
  void operator=(const LiteralBool &) = delete;
  bool value;

 public:
  bool getValue() const {
    return value;
  }

  explicit LiteralBool(bool val)
      : Literal(ValueKind::LiteralBoolKind), value(val) {
    setType(Type::createBoolean());
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::LiteralBoolKind;
  }
};

/// This represents the JavaScript global object. Only one instance of it
/// can exist in a module.
class GlobalObject : public Literal {
  GlobalObject(const GlobalObject &) = delete;
  void operator=(const GlobalObject &) = delete;

 public:
  explicit GlobalObject() : Literal(ValueKind::GlobalObjectKind) {
    setType(Type::createObject());
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::GlobalObjectKind;
  }
};

template <class T, ValueKind K, Type typer() = Type::createAnyType>
class LiteralWrapper : public Literal, public llvh::FoldingSetNode {
 protected:
  T data_;

 public:
  explicit LiteralWrapper(T data) : Literal(K), data_(data) {
    setType(typer());
  }

  T getData() const {
    return data_;
  }

  static bool classof(const Value *V) {
    return V->getKind() == K;
  }

  static void Profile(llvh::FoldingSetNodeID &ID, T data) {
    if constexpr (std::is_integral_v<T>)
      ID.AddInteger(data);
    else if constexpr (std::is_enum_v<T>)
      ID.AddInteger(static_cast<std::underlying_type_t<T>>(data));
    else if constexpr (std::is_pointer_v<T>)
      ID.AddPointer(data);
    else
      ID.Add(data);
  }

  void Profile(llvh::FoldingSetNodeID &ID) const {
    Profile(ID, data_);
  }
};

using LiteralIRType =
    LiteralWrapper<Type, ValueKind::LiteralIRTypeKind, Type::createNull>;
using LiteralNativeSignature =
    LiteralWrapper<NativeSignature *, ValueKind::LiteralNativeSignatureKind>;
using LiteralNativeExtern = LiteralWrapper<
    NativeExtern *,
    ValueKind::LiteralNativeExternKind,
    Type::createNumber>;

/// This represents a JavaScript variable, that's allocated in the function.
class Variable : public Value {
 public:
 private:
  Variable(const Variable &) = delete;
  void operator=(const Variable &) = delete;

  /// The textual representation of the variable in the JavaScript program.
  Identifier text;

  /// The scope that owns the variable.
  VariableScope *parent;

  /// If true, this variable obeys the TDZ rules.
  bool obeysTDZ_ = false;

  /// If true, this variable is const.
  bool isConst_ = false;

 public:
  explicit Variable(VariableScope *scope, Identifier txt);

  ~Variable();

  Identifier getName() const {
    return text;
  }
  VariableScope *getParent() const {
    return parent;
  }

  bool getObeysTDZ() const {
    return obeysTDZ_;
  }
  void setObeysTDZ(bool value) {
    obeysTDZ_ = value;
  }

  bool getIsConst() const {
    return isConst_;
  }
  void setIsConst(bool value) {
    isConst_ = value;
  }

  /// Return the index of this variable in the function's variable list.
  int getIndexInVariableList() const;

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::VariableKind;
  }
};

/// A property of the global object.
class GlobalObjectProperty : public Value, public llvh::FoldingSetNode {
  /// The variable name.
  LiteralString *name_;

  /// Was it explicitly declared (which means that it is non-configurable).
  bool declared_ = false;

 public:
  GlobalObjectProperty(LiteralString *name)
      : Value(ValueKind::GlobalObjectPropertyKind), name_(name) {}

  static bool classof(const Value *v) {
    return v->getKind() == ValueKind::GlobalObjectPropertyKind;
  }

  LiteralString *getName() const {
    return name_;
  }

  bool isDeclared() const {
    return declared_;
  }

  void orDeclared(bool declared) {
    declared_ |= declared;
  }

  static void Profile(llvh::FoldingSetNodeID &ID, LiteralString *name) {
    ID.AddPointer(name);
  }
  void Profile(llvh::FoldingSetNodeID &ID) const {
    Profile(ID, name_);
  }
};

/// This is the base class for all instructions in the high-level IR.
class Instruction
    : public llvh::ilist_node_with_parent<Instruction, BasicBlock>,
      public Value {
  friend class Value;
  Instruction(const Instruction &) = delete;
  void operator=(const Instruction &) = delete;

  /// The basic block containing this instruction.
  BasicBlock *Parent;
  /// Saves the instruction operands.
  llvh::SmallVector<Value::Use, 2> Operands;

  SMLoc location_{};
  /// The statement of which this Instruction is a part.
  /// If 0, then there is no corresponding source statement.
  uint32_t statementIndex_{0};

 protected:
  explicit Instruction(ValueKind kind) : Value(kind), Parent(nullptr) {}

  /// A constructor used for cloning instructions. It copies the \c kind and all
  /// other non-operand data from the original instruction. The operands are
  /// taken from \p operands (because the original operands are likely not
  /// useful).
  /// It deliberately doesn't take a reference as a first parameter since we
  /// don't want it to resemble a copy constructor (to avoid confusion).
  explicit Instruction(
      const Instruction *src,
      llvh::ArrayRef<Value *> operands);

  /// Add an operand to the operand list.
  void pushOperand(Value *Val);

  /// Implementation of \c getInherentType, default implementation returns
  /// None.
  ///
  /// Subclasses should override this and call it in the constructor if the
  /// instructions themselves have an inherent type.
  ///
  /// \return a Type if the instruction has an inherent type, None otherwise.
  static llvh::Optional<Type> getInherentTypeImpl() {
    return llvh::None;
  }

 public:
  void setOperand(Value *Val, unsigned Index);
  Value *getOperand(unsigned Index) const;
  unsigned getNumOperands() const;
  void removeOperand(unsigned index);

  /// Returns a vector of flags indicating which operands the instruction writes
  /// to.
  WordBitSet<> getChangedOperands();

  /// An "inherent type" is a type that the instruction will _always_ have,
  /// regardless of the operands that it is provided with.
  /// The inherent type MUST be the tightest type bound that can be placed
  /// on the result of the instruction - it must not be possible to narrow
  /// the inherent type through any kind of type inference.
  ///
  /// If an Instruction has an inherent type, the constructor should set the
  /// type of the Instruction to the inherent type by calling
  /// \c getInherentTypeImpl.
  ///
  /// \return a Type if the instruction has an inherent type, None otherwise.
  llvh::Optional<Type> getInherentType();

  /// \return whether this instruction has an output value.
  bool hasOutput() const;

  /// \return whether this instruction is a typed instruction and should not,
  /// e.g. have its type inferred by TypeInference.
  bool isTyped() const;

  /// Returns true if any of the operands can have an "empty" or "uninit" type.
  bool acceptsEmptyType() const;

  /// Returns true if any of the operands can have an "empty" or "uninit" type.
  /// The default implementation returns false. This method has to be overridden
  /// by a few instructions that can handle an empty type (ThrowIf, Mov, etc).
  bool acceptsEmptyTypeImpl() const {
    return false;
  }

  void setLocation(SMLoc loc) {
    location_ = loc;
  }
  SMLoc getLocation() const {
    return location_;
  }
  bool hasLocation() const {
    return location_.isValid();
  }

  /// Update the statement that this Instruction belongs to.
  /// Set to 0 if the Instruction isn't part of a source statement.
  void setStatementIndex(uint32_t statementIndex) {
    statementIndex_ = statementIndex;
  }

  uint32_t getStatementIndex() const {
    return statementIndex_;
  }

  /// A debug utility that dumps the textual representation of the IR to the
  /// given ostream, defaults to stdout.
  void dump(llvh::raw_ostream &os = llvh::outs()) const;

  /// Replace the first operand from \p From to \p To. The value \p From must
  /// be an operand of the instruction. The method only replaces the first
  /// occurrence of \p From.
  void replaceFirstOperandWith(Value *OldValue, Value *NewValue);

  /// Move or insert an instruction at a given position. We deliberately do not
  /// have iterator versions of these methods because these functions rely on
  /// the position being a valid instruction (which is not the case with end
  /// iterators, for instance).
  void insertBefore(Instruction *InsertPos);
  void insertAfter(Instruction *InsertPos);
  void moveBefore(Instruction *Later);

  void removeFromParent();
  void eraseFromParent();

  /// Return the name of the instruction.
  llvh::StringRef getName();

  /// \returns the side effect of the instruction.
  SideEffect getSideEffect() const;

  Context &getContext() const;
  BasicBlock *getParent() const {
    return Parent;
  }
  void setParent(BasicBlock *parent) {
    Parent = parent;
  }

  /// \return the Function to which this Instruction belongs.
  inline Function *getFunction() const;

  /// \return the Module to which this Instruction belongs.
  inline Module *getModule() const;

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), Instruction);
  }

  /// Return the hash code the this instruction.
  llvh::hash_code getHashCode() const;

  /// Return true if \p RHS is equal to this instruction.
  bool isIdenticalTo(const Instruction *RHS) const;
};

} // namespace hermes

//===----------------------------------------------------------------------===//
// ilist_traits for Instruction
//===----------------------------------------------------------------------===//

namespace llvh {

template <>
struct ilist_alloc_traits<::hermes::Instruction> {
  static void deleteNode(::hermes::Instruction *V) {
    ::hermes::Value::destroy(V);
  }
};

} // namespace llvh

namespace hermes {

class BasicBlock : public llvh::ilist_node_with_parent<BasicBlock, Function>,
                   public Value {
  BasicBlock(const BasicBlock &) = delete;
  void operator=(const BasicBlock &) = delete;

 public:
  using InstListType = llvh::iplist<Instruction>;

 private:
  InstListType InstList{};
  Function *Parent;

 public:
  explicit BasicBlock(Function *parent);

  /// A debug utility that dumps the textual representation of the IR to \p os,
  /// defaults to stdout.
  void dump(llvh::raw_ostream &os = llvh::outs()) const;

  /// Used by LLVM's graph trait.
  void printAsOperand(llvh::raw_ostream &OS, bool) const;

  /// \brief Returns the terminator instruction if the block is well formed or
  /// null if the block is not well formed.
  TerminatorInst *getTerminator();
  const TerminatorInst *getTerminator() const;

  InstListType &getInstList() {
    return InstList;
  }

  void push_back(Instruction *I);
  void removeFromParent();
  void eraseFromParent();
  void remove(Instruction *I);
  void erase(Instruction *I);

  Context &getContext() const;
  Function *getParent() const {
    return Parent;
  }
  void setParent(Function *parent) {
    Parent = parent;
  }

  /// Needed by llvh::ilist_node_with_parent. Returns the offset of the
  /// aproperiate list based on the type of the argument.
  static InstListType BasicBlock::*getSublistAccess(Instruction *) {
    return &BasicBlock::InstList;
  }

  using iterator = InstListType::iterator;
  using const_iterator = InstListType::const_iterator;
  using reverse_iterator = InstListType::reverse_iterator;
  using const_reverse_iterator = InstListType::const_reverse_iterator;

  using range = llvh::iterator_range<iterator>;
  using const_range = llvh::iterator_range<const_iterator>;
  using reverse_range = llvh::iterator_range<reverse_iterator>;
  using const_reverse_range = llvh::iterator_range<const_reverse_iterator>;

  inline iterator begin() {
    return InstList.begin();
  }
  inline iterator end() {
    return InstList.end();
  }
  inline reverse_iterator rbegin() {
    return InstList.rbegin();
  }
  inline reverse_iterator rend() {
    return InstList.rend();
  }
  inline const_iterator begin() const {
    return InstList.begin();
  }
  inline const_iterator end() const {
    return InstList.end();
  }
  inline const_reverse_iterator rbegin() const {
    return InstList.rbegin();
  }
  inline const_reverse_iterator rend() const {
    return InstList.rend();
  }
  inline size_t size() const {
    return InstList.size();
  }
  inline bool empty() const {
    return InstList.empty();
  }
  inline Instruction &front() {
    return InstList.front();
  }
  inline const Instruction &front() const {
    return InstList.front();
  }
  inline Instruction &back() {
    return InstList.back();
  }
  inline const Instruction &back() const {
    return InstList.back();
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::BasicBlockKind;
  }
};

} // namespace hermes

//===----------------------------------------------------------------------===//
// ilist_traits for BasicBlock
//===----------------------------------------------------------------------===//

namespace llvh {

template <>
struct ilist_alloc_traits<::hermes::BasicBlock> {
  static void deleteNode(::hermes::BasicBlock *V) {
    ::hermes::Value::destroy(V);
  }
};

} // namespace llvh

namespace hermes {

/// VariableScope is a lexical scope. This inherits from ilist_node twice, once
/// for its owning list in the module, and again for the child list that each
/// scope is part of. The ilist_tags used are just templated on type of the
/// owner of the specific list, but they could be any type in principle.
class VariableScope
    : public Value,
      public llvh::ilist_node<VariableScope, llvh::ilist_tag<Module>>,
      public llvh::ilist_node<VariableScope, llvh::ilist_tag<VariableScope>> {
  using VariableListType = llvh::SmallVector<Variable *, 8>;

  /// The variables associated with this scope.
  VariableListType variables_;

  /// The VariableScope representing the parent of this scope, or null if no
  /// such scope exists.
  VariableScope *parentScope_;

  /// The list of children of this VariableScope. This is necessary so we can
  /// update their parents if this scope is eliminated.
  llvh::simple_ilist<VariableScope, llvh::ilist_tag<VariableScope>> children_;

 public:
  VariableScope(VariableScope *parentScope);

  /// \returns a list of variables.
  VariableListType &getVariables() {
    return variables_;
  }
  /// \returns a list of variables.
  llvh::ArrayRef<Variable *> getVariables() const {
    return variables_;
  }

  /// Add a variable \p V to the variable list.
  void addVariable(Variable *V) {
    variables_.push_back(V);
  }

  /// Get the parent scope, which may be null if there isn't one.
  VariableScope *getParentScope() const {
    return parentScope_;
  }

  /// Remove this scope from the scope chain, by moving all of its children to
  /// instead be children of its parent.
  void removeFromScopeChain();

  ~VariableScope() {
    // Free all variables.
    for (auto *v : variables_) {
      Value::destroy(v);
    }
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::VariableScopeKind;
  }
};

class Function : public llvh::ilist_node_with_parent<Function, Module>,
                 public Value {
  Function(const Function &) = delete;
  void operator=(const Function &) = delete;

 public:
  using BasicBlockListType = llvh::iplist<BasicBlock>;

  enum class DefinitionKind {
    ES5Function,
    ES6Constructor,
    ES6Arrow,
    ES6Method,
    // This corresponds to the synthetic function we create in IRGen, which is
    // the inner generator function passed as the argument to CreateGenerator.
    GeneratorInner,
  };

  /// Enum describing restrictions on how this function may be invoked.
  enum class ProhibitInvoke {
    ProhibitNone,
    ProhibitConstruct,
    ProhibitCall,
  };

  /// \return true when every callsite of this Function is absolutely known
  /// and analyzable, even in the presence of Error structured stack trace.
  bool allCallsitesKnown() const {
    return getAttributes(parent_)._allCallsitesKnownInStrictMode && strictMode_;
  }

  /// \return true when every callsite of this Function is absolutely known
  /// and analyzable except for perhaps the usage of Error structured stack
  /// trace in loose mode.
  bool allCallsitesKnownExceptErrorStructuredStackTrace() const {
    return getAttributes(parent_)._allCallsitesKnownInStrictMode;
  }

  /// \return a ProhibitInvoke value representing restrictions on how this
  /// function may be invoked.
  ProhibitInvoke getProhibitInvoke() const;

 private:
  /// The Module owning this function.
  Module *parent_;

  /// The basic blocks in this function.
  BasicBlockListType BasicBlockList{};
  /// JS parameters, which are all technically optional. "this" is at index 0.
  llvh::SmallVector<JSDynamicParam *, 4> jsDynamicParams_{};
  /// Flag indicating whether the JS "this" dynamic param has been added to
  /// params.
  bool jsThisAdded_ = false;
  /// Parameter used as an operand in GetNewTarget to easily find all users.
  JSDynamicParam newTargetParam_;
  /// Parameter used as an operand in GetParentScope to easily find all users.
  JSDynamicParam parentScopeParam_;
  /// The user-specified original name of the function,
  /// or if not specified (e.g. anonymous), the inferred name.
  /// If there was no inference, an empty string.
  Identifier originalOrInferredName_;
  /// What kind of function is this - es5, constructor, etc.
  DefinitionKind const definitionKind_;
  /// Whether the function is in strict mode.
  const bool strictMode_{};
  /// The source location of the function.
  SMRange SourceRange{};
  /// Information on custom directives found in this function.
  CustomDirectives customDirectives_{};

  Type returnType_ = Type::createAnyType();

  /// A name derived from \c originalOrInferredName_, but unique in the Module.
  /// Used only for printing and diagnostic.
  Identifier internalName_;

  /// The number of expected arguments, derived from the formal parameters given
  /// in the function signature.
  /// Only parameters up to the first parameter with an initializer are counted.
  /// See ES14.1.7 for details.
  uint32_t expectedParamCountIncludingThis_{0};

  /// The current statement count in this function.
  /// If statementCount_ > 0, it represents the current statement being
  /// generated.
  /// If statementCount == 0, then we haven't started generating statements.
  /// If statementCount_ is None we've completed generating statements
  /// and have cleared it in preparation for lowering steps.
  OptValue<uint32_t> statementCount_{0};

 protected:
  explicit Function(
      ValueKind kind,
      Module *parent,
      Identifier originalName,
      DefinitionKind definitionKind,
      bool strictMode,
      CustomDirectives customDirectives,
      SMRange sourceRange,
      Function *insertBefore = nullptr);

 public:
  ~Function();

  Module *getParent() const {
    return parent_;
  }

  /// \returns whether this is the top level function (i.e. global scope).
  bool isGlobalScope() const;

  /// \return whether this is a anonymous function.
  bool isAnonymous() const {
    return originalOrInferredName_.str().empty();
  }

  /// \param isDescriptive whether in a more descriptive manner, e.g.
  /// "arrow function" instead of "arrow".
  /// \return the string representation of enum \c DefinitionKind.
  std::string getDefinitionKindStr(bool isDescriptive) const;

  /// \return the string representation in a more human readable manner that
  /// should be used in error messages. Currently it only prefixes a "anonymous"
  /// when it's needed, e.g. "anonymous function" instead of "function".
  std::string getDescriptiveDefinitionKindStr() const;

  /// \return the source code representation string of the function if its
  /// source visibility is non-default, or llvh::None if it's the default.
  /// Specifically, it returns the real source code for function declared with
  /// 'show source' and an empty string for function declared with 'hide source'
  /// or 'sensitive'. See comments in the implementation for details.
  llvh::Optional<llvh::StringRef> getSourceRepresentationStr() const;

  /// \returns the internal name of the function.
  const Identifier getInternalName() const {
    return internalName_;
  }

  /// \returns the string representation of internal name.
  llvh::StringRef getInternalNameStr() const {
    return internalName_.str();
  }

  /// \returns the original function name specified by the user,
  /// or if not specified, the inferred name.
  const Identifier getOriginalOrInferredName() const {
    return originalOrInferredName_;
  }

  /// \return the Context of the parent module.
  Context &getContext() const;

  void addBlock(BasicBlock *BB);

  /// Set \p newEntry as the entry block to this function.
  /// \p newEntry must already exist in this function's BasicBlockList.
  void moveBlockToEntry(BasicBlock *newEntry);

  /// Add a new JS parameter.
  void addJSDynamicParam(JSDynamicParam *param);
  /// Add the "this" JS parameter.
  void addJSThisParam(JSDynamicParam *param);
  /// \return true if JS "this" parameter was added.
  bool jsThisAdded() const {
    return jsThisAdded_;
  }

  Type getReturnType() const {
    return returnType_;
  }
  void setReturnType(Type returnType) {
    returnType_ = returnType;
  }

  /// \return the new.target parameter.
  JSDynamicParam *getNewTargetParam() {
    return &newTargetParam_;
  }
  /// \return the new.target parameter.
  const JSDynamicParam *getNewTargetParam() const {
    return &newTargetParam_;
  }

  /// \return the parent scope parameter.
  JSDynamicParam *getParentScopeParam() {
    return &parentScopeParam_;
  }

  const BasicBlockListType &getBasicBlockList() const {
    return BasicBlockList;
  }
  BasicBlockListType &getBasicBlockList() {
    return BasicBlockList;
  }

  /// Erase all the basic blocks and instructions in this function.
  /// Then remove the function from the module, remove all references.
  /// However this does not deallocate (destroy) the memory of this function.
  void eraseFromParentNoDestroy();

  /// A debug utility that dumps the textual representation of the IR to \p os,
  /// defaults to stdout.
  void dump(llvh::raw_ostream &os = llvh::outs()) const;

  /// Return the kind of function: constructor, arrow, etc.
  DefinitionKind getDefinitionKind() const {
    return definitionKind_;
  }

  /// Return whether the function is in strict mode.
  bool isStrictMode() const {
    return strictMode_;
  }

  /// Return the source range covered by the function.
  SMRange getSourceRange() const {
    return SourceRange;
  }

  /// Return the source visibility of the function.
  SourceVisibility getSourceVisibility() const {
    return customDirectives_.sourceVisibility;
  }

  /// \return whether the function should always be attempted to be inlined,
  /// bypassing the heuristics that normally determine that.
  bool getAlwaysInline() const {
    return customDirectives_.alwaysInline;
  }

  OptValue<uint32_t> getStatementCount() const {
    return statementCount_;
  }

  void setStatementCount(uint32_t count) {
    statementCount_ = count;
  }

  /// Increments the statement count of the function.
  /// Requires that the statement count has never been cleared.
  void incrementStatementCount() {
    assert(
        statementCount_.hasValue() &&
        "cannot incrementStatementCount after clearing it");
    statementCount_ = *statementCount_ + 1;
  }

  void clearStatementCount() {
    statementCount_ = llvh::None;
  }

  auto &getJSDynamicParams() {
    return jsDynamicParams_;
  }

  const auto &getJSDynamicParams() const {
    return jsDynamicParams_;
  }
  /// \return a JS parameter by index. Index 0 is "this", 1 the first declared
  /// parameter, etc.
  JSDynamicParam *getJSDynamicParam(uint32_t index) const {
    return jsDynamicParams_[index];
  }

  void setExpectedParamCountIncludingThis(uint32_t count) {
    expectedParamCountIncludingThis_ = count;
  }

  /// \return the number of params this function takes, including 'this'. Should
  /// always return a number greater than 0.
  uint32_t getExpectedParamCountIncludingThis() const {
    assert(
        expectedParamCountIncludingThis_ > 0 &&
        "there should always be at least the 'this' parameter");
    return expectedParamCountIncludingThis_;
  }

  /// \return true if the function should be compiled lazily.
  bool isLazy() const {
    return false;
  }

  using iterator = BasicBlockListType::iterator;
  using const_iterator = BasicBlockListType::const_iterator;
  using reverse_iterator = BasicBlockListType::reverse_iterator;
  using const_reverse_iterator = BasicBlockListType::const_reverse_iterator;

  inline iterator begin() {
    return BasicBlockList.begin();
  }
  inline iterator end() {
    return BasicBlockList.end();
  }
  inline reverse_iterator rbegin() {
    return BasicBlockList.rbegin();
  }
  inline reverse_iterator rend() {
    return BasicBlockList.rend();
  }
  inline const_iterator begin() const {
    return BasicBlockList.begin();
  }
  inline const_iterator end() const {
    return BasicBlockList.end();
  }
  inline const_reverse_iterator rbegin() const {
    return BasicBlockList.rbegin();
  }
  inline const_reverse_iterator rend() const {
    return BasicBlockList.rend();
  }
  inline size_t size() const {
    return BasicBlockList.size();
  }
  inline bool empty() const {
    return BasicBlockList.empty();
  }
  inline BasicBlock &front() {
    return BasicBlockList.front();
  }
  inline BasicBlock &back() {
    return BasicBlockList.back();
  }

  void viewGraph();

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), Function);
  }
};

class NormalFunction final : public Function {
 public:
  /// \param parent Module this function will belong to.
  /// \param originalName User-specified function name, or an empty string.
  /// \param strictMode Whether this function uses strict mode.
  /// \param isGlobal Whether this is the global (top-level) function.
  /// \param sourceRange Range of source code this function comes from.
  /// \param insertBefore Another function in \p parent where this function
  ///   should be inserted before. If null, appends to the end of the module.
  explicit NormalFunction(
      Module *parent,
      Identifier originalName,
      DefinitionKind definitionKind,
      bool strictMode,
      CustomDirectives customDirectives,
      SMRange sourceRange,
      Function *insertBefore = nullptr)
      : Function(
            ValueKind::NormalFunctionKind,
            parent,
            originalName,
            definitionKind,
            strictMode,
            customDirectives,
            sourceRange,
            insertBefore) {}

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::NormalFunctionKind;
  }
};

/// The "outer" generator function, used with CreateFunctionInst and invoked as
/// a normal function.
class GeneratorFunction final : public Function {
 public:
  explicit GeneratorFunction(
      Module *parent,
      Identifier originalName,
      DefinitionKind definitionKind,
      bool strictMode,
      CustomDirectives customDirectives,
      SMRange sourceRange,
      Function *insertBefore)
      : Function(
            ValueKind::GeneratorFunctionKind,
            parent,
            originalName,
            definitionKind,
            strictMode,
            customDirectives,
            sourceRange,
            insertBefore) {}

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::GeneratorFunctionKind;
  }
};

class AsyncFunction final : public Function {
 public:
  explicit AsyncFunction(
      Module *parent,
      Identifier originalName,
      DefinitionKind definitionKind,
      bool strictMode,
      CustomDirectives customDirectives,
      SMRange sourceRange,
      Function *insertBefore)
      : Function(
            ValueKind::AsyncFunctionKind,
            parent,
            originalName,
            definitionKind,
            strictMode,
            customDirectives,
            sourceRange,
            insertBefore) {}

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::AsyncFunctionKind;
  }
};

} // namespace hermes

//===----------------------------------------------------------------------===//
// ilist_traits for Function
//===----------------------------------------------------------------------===//

namespace llvh {

template <>
struct ilist_alloc_traits<::hermes::Function> {
  static void deleteNode(::hermes::Function *V) {
    ::hermes::Value::destroy(V);
  }
};

template <>
struct ilist_alloc_traits<::hermes::VariableScope> {
  static void deleteNode(::hermes::VariableScope *V) {
    ::hermes::Value::destroy(V);
  }
};

} // namespace llvh

namespace hermes {

class Module : public Value {
  Module(const Module &) = delete;
  void operator=(const Module &) = delete;

 public:
  using FunctionListType = llvh::iplist<Function>;
  using VariableScopeListType =
      llvh::iplist<VariableScope, llvh::ilist_tag<Module>>;

  using RawStringList = std::vector<LiteralString *>;

  struct CJSModule {
    /// ID of the module, allocated using a counter.
    uint32_t id;
    /// Filename for the module.
    Identifier filename;
    /// Pointer to the wrapper function for the module.
    Function *function;
  };

 private:
  std::shared_ptr<Context> Ctx;
  /// Optionally specify the top level function, if it isn't the first one.
  Function *topLevelFunction_{};

  FunctionListType FunctionList{};

  /// List of all the VariableScopes owned by this module.
  VariableScopeListType variableScopes_{};

  GlobalObject globalObject_{};
  LiteralEmpty literalEmpty{};
  LiteralUninit literalUninit_{};
  LiteralUndefined literalUndefined{};
  LiteralNull literalNull{};
  LiteralBool literalFalse{false};
  LiteralBool literalTrue{true};
  EmptySentinel emptySentinel_{};

  // Uniqued values.

  template <typename T>
  using ValueOFS = OwningFoldingSet<T, ValueCreator<T>, ValueDeleter>;

  ValueOFS<LiteralNumber> literalNumbers_{};
  ValueOFS<LiteralBigInt> literalBigInts_{};
  ValueOFS<LiteralString> literalStrings_{};
  ValueOFS<GlobalObjectProperty> globalProperties_{};
  ValueOFS<LiteralIRType> literalIRTypes_{};
  ValueOFS<LiteralNativeSignature> nativeSignatures_{};
  ValueOFS<LiteralNativeExtern> nativeExterns_{};

  /// Map from an identifier to a number indicating how many times it has been
  /// used. This allows to construct unique internal names derived from regular
  /// identifiers.
  llvh::DenseMap<Identifier, unsigned> internalNamesMap_;

  /// Storage for the CJS modules.
  /// Use a deque to allow for non-copying push_back, to avoid invalidating the
  /// pointers in cjsModuleFunctionMap_ below.
  std::deque<CJSModule> cjsModules_{};

  using CJSModuleIterator = std::deque<CJSModule>::iterator;

  /// Map from functions to members of the CJS module storage.
  llvh::DenseMap<Function *, CJSModule *> cjsModuleFunctionMap_{};
  /// Map from file names to CJS module IDs.
  llvh::DenseMap<Identifier, uint32_t> cjsModuleFilenameMap_{};
  /// Map from segment IDs to CJS module wrapper functions.
  /// The vectors in the map have no duplicates; this is enforced using
  /// cjsModuleFunctionMap_.
  llvh::DenseMap<uint32_t, std::vector<Function *>> cjsModuleSegmentMap_{};

  /// true if all CJS modules have been resolved.
  bool cjsModulesResolved_{false};

  using CJSModuleUseGraph =
      std::unordered_map<Function *, llvh::SmallPtrSet<Function *, 2>>;

  /// Graph which maps from a function F to a set of functions which it uses.
  /// Used to determine what functions must be generated for a given segment.
  /// Lazily generated the first time bytecode is generated for a split bundle,
  /// and used when generating bytecode for a given segment.
  /// If empty, this has not been generated yet.
  CJSModuleUseGraph cjsModuleUseGraph_{};

  struct HashRawStrings {
    std::size_t operator()(const RawStringList &rawStrings) const {
      return llvh::hash_combine_range(rawStrings.begin(), rawStrings.end());
    }
  };

  /// A map from a list of raw strings from a template literal to its unique id.
  std::unordered_map<RawStringList, uint32_t, HashRawStrings>
      templateObjectIDMap_;

  /// Set to true when lowerIR has been called on this Module.
  bool isLowered_{false};

  /// Set to true when generator functions have been lowered.
  bool areGeneratorsLowered_{false};

 public:
  explicit Module(std::shared_ptr<Context> ctx)
      : Value(ValueKind::ModuleKind), Ctx(std::move(ctx)) {}

  ~Module();

  Context &getContext() const {
    return *Ctx;
  }

  std::shared_ptr<Context> shareContext() const {
    return Ctx;
  }

  /// Derive a unique internal name from a specified identifier.
  Identifier deriveUniqueInternalName(Identifier originalName);

  using iterator = FunctionListType::iterator;
  using const_iterator = FunctionListType::const_iterator;
  using reverse_iterator = FunctionListType::reverse_iterator;
  using const_reverse_iterator = FunctionListType::const_reverse_iterator;

  /// Add a new function to the module.
  void push_back(Function *F);
  /// Insert a new function at a specific position in the module.
  void insert(iterator position, Function *F);

  const FunctionListType &getFunctionList() const {
    return FunctionList;
  }
  FunctionListType &getFunctionList() {
    return FunctionList;
  }

  /// Set the top-level function of this module - the function that will be
  /// executed when the module is executed.
  void setTopLevelFunction(Function *topLevelFunction) {
    assert(
        topLevelFunction->getParent() == this &&
        "topLevelFunction from a different module");
    assert(!topLevelFunction_ && "Top level function is already set.");
    topLevelFunction_ = topLevelFunction;
  }

  /// Return the top-level function.
  Function *getTopLevelFunction() {
    assert(topLevelFunction_ && "top-level function hasn't been created yet");
    // If the top level function hasn't been overridden, return the first
    // function.
    return topLevelFunction_;
  }

  /// Get the list of variable scopes owned by this module.
  VariableScopeListType &getVariableScopes() {
    return variableScopes_;
  }
  const VariableScopeListType &getVariableScopes() const {
    return variableScopes_;
  }

  /// Create the specified global property if it doesn't exist. If it does
  /// exist, its declared property is updated by a logical OR.
  GlobalObjectProperty *addGlobalProperty(Identifier name, bool declared);

  /// Create a new literal number of value \p value.
  LiteralNumber *getLiteralNumber(double value);

  /// Create a new literal BigInt of value \p value.
  LiteralBigInt *getLiteralBigInt(UniqueString *value);

  /// Create a new literal string of value \p value.
  LiteralString *getLiteralString(Identifier value);

  /// Create a new literal bool of value \p value.
  LiteralBool *getLiteralBool(bool value);

  /// Create a new literal representing an IR type.
  LiteralIRType *getLiteralIRType(Type value);

  /// Create a new LiteralNativeSignature.
  LiteralNativeSignature *getLiteralNativeSignature(NativeSignature *data);

  /// Create a new LiteralNativeExtern.
  LiteralNativeExtern *getLiteralNativeExtern(NativeExtern *data);

  /// Create a new literal 'empty'.
  LiteralEmpty *getLiteralEmpty() {
    return &literalEmpty;
  }

  /// Return the Uninit literal singleton.
  LiteralUninit *getLiteralUninit() {
    return &literalUninit_;
  }

  /// Create a new literal 'undefined'.
  LiteralUndefined *getLiteralUndefined() {
    return &literalUndefined;
  }

  /// Create a new literal null.
  LiteralNull *getLiteralNull() {
    return &literalNull;
  }

  /// Return the GlobalObject value.
  GlobalObject *getGlobalObject() {
    return &globalObject_;
  }

  /// Return the shared instance of EmptySentinel.
  EmptySentinel *getEmptySentinel() {
    return &emptySentinel_;
  }

  /// Add a new CJS module entry, given the function representing the module.
  void addCJSModule(
      uint32_t segmentID,
      uint32_t id,
      Identifier name,
      Function *function) {
    cjsModules_.emplace_back(CJSModule{id, name, function});
    CJSModule &module = cjsModules_.back();
    {
      auto result = cjsModuleFilenameMap_.try_emplace(name, id);
      (void)result;
      assert(
          (result.second || result.first->second == id) &&
          "Module must have the same ID for the same name");
    }
    {
      auto result = cjsModuleFunctionMap_.try_emplace(function, &module);
      (void)result;
      assert(
          result.second && "Should only insert each CJS wrapper function once");
    }
    cjsModuleSegmentMap_[segmentID].push_back(function);
  }

  /// \return the CommonJS module given the wrapping function if it is a module,
  /// else nullptr.
  const CJSModule *findCJSModule(Function *function) const {
    auto it = cjsModuleFunctionMap_.find(function);
    return it == cjsModuleFunctionMap_.end() ? nullptr : it->second;
  }

  /// \return the CommonJS module ID given the filename if it is a module,
  /// else nullptr.
  llvh::Optional<uint32_t> findCJSModuleID(Identifier filename) const {
    auto it = cjsModuleFilenameMap_.find(filename);
    if (it != cjsModuleFilenameMap_.end()) {
      return it->second;
    }
    return llvh::None;
  }

  /// \return the registered CommonJS modules.
  llvh::iterator_range<CJSModuleIterator> getCJSModules() {
    return {cjsModules_.begin(), cjsModules_.end()};
  }

  /// \return true if all CJS modules in this Module have been resolved.
  bool getCJSModulesResolved() const {
    return cjsModulesResolved_;
  }

  /// Set to true if all CJS modules in this Module have been resolved.
  void setCJSModulesResolved(bool cjsModulesResolved) {
    cjsModulesResolved_ = cjsModulesResolved;
  }

  /// \return the set of functions which are used by the modules in the segment
  /// specified by \p segment. Order is unspecified, so the return value
  /// should not be used for iteration, only for checking membership.
  llvh::DenseSet<Function *> getFunctionsInSegment(uint32_t segment);

  /// \param rawStrings a list of raw strings from a template literal
  /// \return (rawStrings, id) where rawStrings is the stored string list,
  ///   and the id is the newly allocated template object ID.
  std::pair<llvh::ArrayRef<LiteralString *>, uint32_t> emplaceTemplateObject(
      RawStringList &&rawStrings);

  bool isLowered() const {
    return isLowered_;
  }

  void setLowered(bool isLowered) {
    isLowered_ = isLowered;
  }

  bool areGeneratorsLowered() const {
    return areGeneratorsLowered_;
  }

  void setGeneratorsLowered(bool lowered) {
    areGeneratorsLowered_ = lowered;
  }

  inline iterator begin() {
    return FunctionList.begin();
  }
  inline iterator end() {
    return FunctionList.end();
  }
  inline reverse_iterator rbegin() {
    return FunctionList.rbegin();
  }
  inline reverse_iterator rend() {
    return FunctionList.rend();
  }
  inline const_iterator begin() const {
    return FunctionList.begin();
  }
  inline const_iterator end() const {
    return FunctionList.end();
  }
  inline const_reverse_iterator rbegin() const {
    return FunctionList.rbegin();
  }
  inline const_reverse_iterator rend() const {
    return FunctionList.rend();
  }
  inline size_t size() const {
    return FunctionList.size();
  }
  inline bool empty() const {
    return FunctionList.empty();
  }
  inline Function &front() {
    return FunctionList.front();
  }
  inline Function &back() {
    return FunctionList.back();
  }

  void viewGraph();
  void dump(llvh::raw_ostream &os = llvh::outs()) const;

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::ModuleKind;
  }

 private:
  /// Calculate the CJS module function graph, if it hasn't been calculated yet.
  /// Caches the result in cjsModuleUseGraph_.
  void populateCJSModuleUseGraph();
};

/// The hash of a Type is the hash of its opaque value.
static inline llvh::hash_code hash_value(Type V) {
  return V.hash();
}

inline Function *Instruction::getFunction() const {
  return Parent->getParent();
}
inline Module *Instruction::getModule() const {
  return getFunction()->getParent();
}

} // end namespace hermes

namespace llvh {

raw_ostream &operator<<(raw_ostream &OS, const hermes::Type &T);

} // namespace llvh

#endif
