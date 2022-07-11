/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_IR_IR_H
#define HERMES_IR_IR_H

#include "hermes/ADT/WordBitSet.h"
#include "hermes/AST/Context.h"
#include "hermes/Support/Conversions.h"
#include "hermes/Support/ScopeChain.h"

#ifndef HERMESVM_LEAN
#include "hermes/AST/ESTree.h"
#endif

#include "llvh/ADT/FoldingSet.h"
#include "llvh/ADT/Hashing.h"
#include "llvh/ADT/SmallPtrSet.h"
#include "llvh/ADT/SmallVector.h"
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
class Parameter;
class Instruction;
class Context;
class TerminatorInst;

/// This is an instance of a JavaScript type.
class Type {
  // Encodes the JavaScript type hierarchy.
  enum TypeKind {
    /// An uninitialized TDZ.
    Empty,
    Undefined,
    Null,
    Boolean,
    String,
    Number,
    BigInt,
    Object,
    Closure, // Subtype of Object.
    RegExp, // Subtype of Object.

    LAST_TYPE
  };

  enum NumTypeKind {
    Double,
    Int32,
    Uint32,

    LAST_NUM_TYPE
  };

  /// Return the string representation of the type at index \p idx.
  StringRef getKindStr(TypeKind idx) const {
    // The strings below match the values in TypeKind.
    static const char *const names[] = {
        "empty",
        "undefined",
        "null",
        "boolean",
        "string",
        "number",
        "bigint",
        "object",
        "closure",
        "regexp"};
    return names[idx];
  }

#define BIT_TO_VAL(XX) (1 << TypeKind::XX)
#define IS_VAL(XX) (bitmask_ == (1 << TypeKind::XX))

#define NUM_BIT_TO_VAL(XX) (1 << NumTypeKind::XX)
#define NUM_IS_VAL(XX) (numBitmask_ == (1 << NumTypeKind::XX))

  // The 'Any' type means all possible types.
  static constexpr unsigned TYPE_ANY_MASK = (1u << TypeKind::LAST_TYPE) - 1;

  static constexpr unsigned PRIMITIVE_BITS = BIT_TO_VAL(Number) |
      BIT_TO_VAL(String) | BIT_TO_VAL(BigInt) | BIT_TO_VAL(Null) |
      BIT_TO_VAL(Undefined) | BIT_TO_VAL(Boolean);

  static constexpr unsigned OBJECT_BITS =
      BIT_TO_VAL(Object) | BIT_TO_VAL(Closure) | BIT_TO_VAL(RegExp);

  static constexpr unsigned NONPTR_BITS = BIT_TO_VAL(Number) |
      BIT_TO_VAL(Boolean) | BIT_TO_VAL(Null) | BIT_TO_VAL(Undefined);

  static constexpr unsigned ANY_NUM_BITS =
      NUM_BIT_TO_VAL(Double) | NUM_BIT_TO_VAL(Int32) | NUM_BIT_TO_VAL(Uint32);

  static constexpr unsigned INTEGER_BITS =
      NUM_BIT_TO_VAL(Int32) | NUM_BIT_TO_VAL(Uint32);

  /// Each bit represent the possibility of the type being the type that's
  /// represented in the enum entry.
  unsigned bitmask_{TYPE_ANY_MASK};
  /// Each bit represent the possibility of the type being the subtype of number
  /// that's represented in the number type enum entry. If the number bit is not
  /// set, this bitmask is meaningless.
  unsigned numBitmask_{ANY_NUM_BITS};

  /// The constructor is only accessible by static builder methods.
  constexpr explicit Type(unsigned mask, unsigned numMask = ANY_NUM_BITS)
      : bitmask_(mask), numBitmask_(numMask) {}

 public:
  constexpr Type() = default;

  static constexpr Type unionTy(Type A, Type B) {
    return Type(A.bitmask_ | B.bitmask_, A.numBitmask_ | B.numBitmask_);
  }

  static constexpr Type intersectTy(Type A, Type B) {
    // This is sound but not complete, but this is only used for disjointness
    // check.
    return Type(A.bitmask_ & B.bitmask_);
  }

  static constexpr Type subtractTy(Type A, Type B) {
    return Type(A.bitmask_ & ~B.bitmask_, A.numBitmask_ & ~B.numBitmask_);
  }

  constexpr bool isNoType() const {
    return bitmask_ == 0;
  }

  static constexpr Type createNoType() {
    return Type(0);
  }
  static constexpr Type createAnyType() {
    return Type(TYPE_ANY_MASK);
  }
  /// Create an uninitialized TDZ type.
  static constexpr Type createEmpty() {
    return Type(BIT_TO_VAL(Empty));
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
  static constexpr Type createBigInt() {
    return Type(BIT_TO_VAL(BigInt));
  }
  static constexpr Type createNumeric() {
    return unionTy(createNumber(), createBigInt());
  }
  static constexpr Type createClosure() {
    return Type(BIT_TO_VAL(Closure));
  }
  static constexpr Type createRegExp() {
    return Type(BIT_TO_VAL(RegExp));
  }
  static constexpr Type createInt32() {
    return Type(BIT_TO_VAL(Number), NUM_BIT_TO_VAL(Int32));
  }
  static constexpr Type createUint32() {
    return Type(BIT_TO_VAL(Number), NUM_BIT_TO_VAL(Uint32));
  }

  constexpr bool isAnyType() const {
    return bitmask_ == TYPE_ANY_MASK;
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

  bool isObjectType() const {
    // One or more of OBJECT_BITS must be set, and no other bit must be set.
    return bitmask_ && !(bitmask_ & ~OBJECT_BITS);
  }

  constexpr bool isNumberType() const {
    return IS_VAL(Number);
  }
  constexpr bool isBigIntType() const {
    return IS_VAL(BigInt);
  }
  constexpr bool isClosureType() const {
    return IS_VAL(Closure);
  }
  constexpr bool isRegExpType() const {
    return IS_VAL(RegExp);
  }
  constexpr bool isInt32Type() const {
    return IS_VAL(Number) && NUM_IS_VAL(Int32);
  }
  constexpr bool isUint32Type() const {
    return IS_VAL(Number) && NUM_IS_VAL(Uint32);
  }
  constexpr bool isIntegerType() const {
    return IS_VAL(Number) && (numBitmask_ && !(numBitmask_ & ~INTEGER_BITS));
  }

  /// \return true if the type is one of the known javascript primitive types:
  /// Number, BigInt, Null, Boolean, String, Undefined.
  constexpr bool isKnownPrimitiveType() const {
    return isPrimitive() && 1 == llvh::countPopulation(bitmask_);
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

  /// \returns true if this type can represent a subtype of object.
  constexpr bool canBeObjectSubtype() const {
    return bitmask_ & OBJECT_BITS;
  }

  /// \returns true if this type can represent a boolean value.
  constexpr bool canBeBoolean() const {
    return canBeType(Type::createBoolean());
  }

  /// \returns true if this type can represent an "empty" value.
  constexpr bool canBeEmpty() const {
    return canBeType(Type::createEmpty());
  }

  /// \returns true if this type can represent an undefined value.
  constexpr bool canBeUndefined() const {
    return canBeType(Type::createUndefined());
  }

  /// \returns true if this type can represent a null value.
  constexpr bool canBeNull() const {
    return canBeType(Type::createNull());
  }

  /// \returns true if this type can represent a closure value.
  constexpr bool canBeClosure() const {
    return canBeType(Type::createClosure());
  }

  /// \returns true if this type can represent a regex value.
  constexpr bool canBeRegex() const {
    return canBeType(Type::createRegExp());
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
};

/// This lattice describes the kind of side effect that instructions have.
/// The side effects are organized in a hierarchy, and higher levels are a
/// superset of lower levels. The exact semantics of the side effects levels are
/// documented in the IR document.
enum class SideEffectKind {
  /// Does not read, write to memory.
  None,
  /// Instruction may read memory.
  MayRead,
  /// Instruction may read or write memory.
  MayWrite,
  /// The side effects of the instruction are unknown and we can't make any
  /// assumptions.
  Unknown,
};

enum class ValueKind : uint8_t {
#define INCLUDE_ALL_INSTRS
#define DEF_VALUE(CLASS, PARENT) CLASS##Kind,
#define MARK_VALUE(CLASS) CLASS##Kind,
#define MARK_FIRST(CLASS) First_##CLASS##Kind,
#define MARK_LAST(CLASS) Last_##CLASS##Kind,
#include "hermes/IR/ValueKinds.def"
#undef INCLUDE_ALL_INSTRS
};

/// \returns true if \p kind is equal to \p base or a subkind of it.
static inline bool kindIsA(ValueKind kind, ValueKind base) {
  switch (base) {
    default:
      return kind == base;
#define INCLUDE_ALL_INSTRS
#define MARK_FIRST(CLASS)                            \
  case ValueKind::CLASS##Kind:                       \
    return kind >= ValueKind::First_##CLASS##Kind && \
        kind <= ValueKind::Last_##CLASS##Kind;
#include "hermes/IR/ValueKinds.def"
#undef INCLUDE_ALL_INSTRS
  }
}

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

  /// The JavaScript type of the value.
  Type valueType;

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

  /// Removes all uses of self
  void removeAllUses();

  /// \returns true if the value \p other is a user of this value.
  bool hasUser(Value *other);

  /// \returns the kind of the value.
  ValueKind getKind() const {
    return Kind;
  }

  /// \returns the string representation of the Value kind.
  StringRef getKindStr() const;

  /// Sets a new type \p type to the value.
  void setType(Type type) {
    valueType = type;
  }

  /// \returns the JavaScript type of the value.
  Type getType() const {
    return valueType;
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

/// This represents a function parameter.
class Parameter : public Value {
  Parameter(const Parameter &) = delete;
  void operator=(const Parameter &) = delete;

  /// The function that contains this paramter.
  Function *Parent;

  /// The formal name of the parameter
  Identifier Name;

 public:
  explicit Parameter(Function *parent, Identifier name);

  void removeFromParent();

  Context &getContext() const;
  Function *getParent() const {
    return Parent;
  }
  void setParent(Function *parent) {
    Parent = parent;
  }

  /// \brief Return a constant reference to the value's name.
  Identifier getName() const;

  /// \returns true if this parameter is a 'this' parameter.
  bool isThisParameter() const;

  /// Return the index of this parameter in the function's parameter list.
  /// "this" parameter is excluded from the list.
  int getIndexInParamList() const;

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::ParameterKind;
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
    return kindIsA(V->getKind(), ValueKind::LiteralKind);
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

/// This represents a JavaScript variable, that's allocated in the function.
class Variable : public Value {
 public:
  enum class DeclKind {
    Var,
    Let,
    Const,
  };

  /// Return true if this DeclKind needs to track TDZ.
  static bool declKindNeedsTDZ(DeclKind dk) {
    return dk != DeclKind::Var;
  }

 private:
  Variable(const Variable &) = delete;
  void operator=(const Variable &) = delete;

  /// Declaration kind: var/let/const.
  DeclKind declKind;

  /// The textual representation of the variable in the JavaScript program.
  Identifier text;

  /// The scope that owns the variable.
  VariableScope *parent;

  /// If true, this variable obeys the TDZ rules.
  bool obeysTDZ_ = false;

 protected:
  explicit Variable(
      ValueKind k,
      VariableScope *scope,
      DeclKind declKind,
      Identifier txt);

 public:
  explicit Variable(VariableScope *scope, DeclKind declKind, Identifier txt)
      : Variable(ValueKind::VariableKind, scope, declKind, txt){};

  ~Variable();

  DeclKind getDeclKind() const {
    return declKind;
  }

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

  /// Return the index of this variable in the function's variable list.
  int getIndexInVariableList() const;

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::VariableKind;
  }
};

/// A property of the global object.
class GlobalObjectProperty : public Value {
  /// The module that owns it.
  Module *parent_;

  /// The variable name.
  LiteralString *name_;

  /// Was it explicitly declared (which means that it is non-configurable).
  bool declared_;

 public:
  GlobalObjectProperty(Module *parent, LiteralString *name, bool declared)
      : Value(ValueKind::GlobalObjectPropertyKind),
        parent_(parent),
        name_(name),
        declared_(declared) {}

  static bool classof(const Value *v) {
    return v->getKind() == ValueKind::GlobalObjectPropertyKind;
  }

  Module *getParent() const {
    return parent_;
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

  /// \returns the side effect of the derived instruction.
  SideEffectKind getDerivedSideEffect();

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

 public:
  void setOperand(Value *Val, unsigned Index);
  Value *getOperand(unsigned Index) const;
  unsigned getNumOperands() const;
  void removeOperand(unsigned index);

  /// Returns a vector of flags indicating which operands the instruction writes
  /// to.
  WordBitSet<> getChangedOperands();

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
  void dump(llvh::raw_ostream &os = llvh::outs());

  /// Replace the first operand from \p From to \p To. The value \p From must
  /// be an operand of the instruction. The method only replaces the first
  /// occurrence of \p From.
  void replaceFirstOperandWith(Value *OldValue, Value *NewValue);

  /// Erase all operands whose value is \p Value, and unregister the user.
  void eraseOperand(Value *Value);

  void insertBefore(Instruction *InsertPos);
  void insertAfter(Instruction *InsertPos);
  void moveBefore(Instruction *Later);
  void removeFromParent();
  void eraseFromParent();

  /// Return the name of the instruction.
  StringRef getName();

  /// \returns true if the instruction has some side effect.
  bool hasSideEffect() {
    return getDerivedSideEffect() != SideEffectKind::None;
  }

  /// \returns true if the instruction may read memory.
  bool mayReadMemory() {
    return getDerivedSideEffect() >= SideEffectKind::MayRead;
  }
  /// \returns true if the instruction may write to memory.
  bool mayWriteMemory() {
    return getDerivedSideEffect() >= SideEffectKind::MayWrite;
  }
  /// \returns true if the instruction may execute code by means of throwing
  /// an exception or by executing code.
  bool mayExecute() {
    return getDerivedSideEffect() > SideEffectKind::MayWrite;
  }

  Context &getContext() const;
  BasicBlock *getParent() const {
    return Parent;
  }
  void setParent(BasicBlock *parent) {
    Parent = parent;
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::InstructionKind);
  }

  /// An opaque class representing the variety of an instruction.
  ///
  /// Variety is more specific than ValueKind: it encapsulates all properties of
  /// an instruction except its operands. For example, the "+" and "-" binary
  /// operator instructions have the same getKind() but different getVariety().
  class Variety {
    friend class Instruction;
    std::pair<unsigned, unsigned> kinds_;
    explicit Variety(std::pair<unsigned, unsigned> kinds) : kinds_(kinds) {}

   public:
    bool operator==(const Variety &other) const {
      return kinds_ == other.kinds_;
    }
    bool operator!=(const Variety &other) const {
      return kinds_ != other.kinds_;
    }
    friend llvh::hash_code hash_value(Variety variety) {
      return llvh::hash_value(variety.kinds_);
    }
  };

  /// Return the variety of this instruction.
  Variety getVariety() const;

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

  /// A debug utility that dumps the textual representation of the IR to stdout.
  void dump();

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

/// VariableScope is a lexical scope.
class VariableScope : public Value {
  using Value::Value;
  using VariableListType = llvh::SmallVector<Variable *, 8>;

  friend class Function;

  /// The function where the scope is declared.
  Function *function_;

  /// The variables associated with this scope.
  VariableListType variables_;

 protected:
  /// VariableScope is abstract and should not be constructed directly. Use a
  /// subclass such as Function.
  VariableScope(ValueKind kind, Function *function)
      : Value(kind), function_(function) {}

  VariableScope(Function *function)
      : VariableScope(ValueKind::VariableScopeKind, function) {}

 public:
  /// \return the function where the scope is declared.
  Function *getFunction() const {
    return function_;
  }

  /// Return true if this is the global function scope.
  bool isGlobalScope() const;

  /// \returns a list of variables.
  VariableListType &getVariables() {
    return variables_;
  }

  /// Add a variable \p V to the variable list.
  void addVariable(Variable *V) {
    variables_.push_back(V);
  }

  ~VariableScope() {
    // Free all variables.
    for (auto *v : variables_) {
      Value::destroy(v);
    }
  }

  static bool classof(const Value *V) {
    switch (V->getKind()) {
      case ValueKind::VariableScopeKind:
      case ValueKind::ExternalScopeKind:
        return true;
      default:
        return false;
    }
  }
};

/// An ExternalScope is a container for variables injected from the environment,
/// i.e. upvars from an enclosing scope. These variables are stored at a given
/// depth in the scope chain.
class ExternalScope : public VariableScope {
  /// The scope depth represented by this external scope
  const int32_t depth_ = 0;

 public:
  ExternalScope(Function *function, int32_t depth);

  /// \return the scope depth
  int32_t getDepth() const {
    return depth_;
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::ExternalScopeKind;
  }
};

class Function : public llvh::ilist_node_with_parent<Function, Module>,
                 public Value {
  Function(const Function &) = delete;
  void operator=(const Function &) = delete;

 public:
  using BasicBlockListType = llvh::iplist<BasicBlock>;
  using ParameterListType = llvh::SmallVector<Parameter *, 8>;

  enum class DefinitionKind {
    ES5Function,
    ES6Constructor,
    ES6Arrow,
    ES6Method,
  };

 private:
  /// The Module owning this function.
  Module *parent_;

  /// Indicates whether this is the global scope.
  bool isGlobal_;

  /// List of external scopes owned by this function. Deleted upon destruction.
  llvh::SmallVector<VariableScope *, 4> externalScopes_;

  /// The function scope - it is always the first scope in the scope list.
  VariableScope functionScope_;

  /// The basic blocks in this function.
  BasicBlockListType BasicBlockList{};
  /// The function parameters.
  ParameterListType Parameters;
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
  /// The source visibility of the function.
  SourceVisibility sourceVisibility_;

  /// A name derived from \c originalOrInferredName_, but unique in the Module.
  /// Used only for printing and diagnostic.
  Identifier internalName_;

  /// The "this" parameter.
  Parameter *thisParameter{};

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

#ifndef HERMESVM_LEAN
  /// The source of a function, containing function type, range and buffer ID.
  LazySource lazySource_;

  /// The SerializedScope of the lazyCompilationAst.
  std::shared_ptr<SerializedScope> lazyScope_{};

  /// The parent's generated closure variable for this function. It is non-null
  /// only if there is an alias binding from \c originalOrInferredName_ (which
  /// must be valid) to said variable. Used only by lazy compilation.
  ///
  /// For a named function expresion:
  ///     myExpression(function bar() { somecode; })
  ///
  /// We generate the code that's really more similar to:
  ///     function anon_0_closure() {
  ///         var bar=anon_0_closure;   // Nametable alias, not a real frameload
  ///         somecode;
  ///     }
  ///     myExpression(anon_0_closure);
  ///
  /// When we generate the `function bar() {...}`, this field will be set to
  /// the `anon_0_closure` variable to capture this relationship.
  Variable *lazyClosureAlias_{};
#endif

 protected:
  explicit Function(
      ValueKind kind,
      Module *parent,
      Identifier originalName,
      DefinitionKind definitionKind,
      bool strictMode,
      SourceVisibility sourceVisibility,
      bool isGlobal,
      SMRange sourceRange,
      Function *insertBefore = nullptr);

 public:
  /// \param parent Module this function will belong to.
  /// \param originalName User-specified function name, or an empty string.
  /// \param strictMode Whether this function uses strict mode.
  /// \param isGlobal Whether this is the global (top-level) function.
  /// \param sourceRange Range of source code this function comes from.
  /// \param insertBefore Another function in \p parent where this function
  ///   should be inserted before. If null, appends to the end of the module.
  explicit Function(
      Module *parent,
      Identifier originalName,
      DefinitionKind definitionKind,
      bool strictMode,
      SourceVisibility sourceVisibility,
      bool isGlobal,
      SMRange sourceRange,
      Function *insertBefore = nullptr)
      : Function(
            ValueKind::FunctionKind,
            parent,
            originalName,
            definitionKind,
            strictMode,
            sourceVisibility,
            isGlobal,
            sourceRange,
            insertBefore) {}

  ~Function();

  Module *getParent() const {
    return parent_;
  }

  /// \returns whether this is the top level function (i.e. global scope).
  bool isGlobalScope() const {
    return isGlobal_;
  }

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
  StringRef getInternalNameStr() const {
    return internalName_.str();
  }

  /// \returns the original function name specified by the user,
  /// or if not specified, the inferred name.
  const Identifier getOriginalOrInferredName() const {
    return originalOrInferredName_;
  }

  /// \return the Context of the parent module.
  Context &getContext() const;

  /// Add a new scope to the function.
  /// This is delete'd in our destructor.
  void addExternalScope(ExternalScope *scope) {
    externalScopes_.push_back(scope);
  }

  VariableScope *getFunctionScope() {
    return &functionScope_;
  }

  void addBlock(BasicBlock *BB);
  void addParameter(Parameter *A);

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

  /// A debug utility that dumps the textual representation of the IR to stdout.
  void dump();

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
    return sourceVisibility_;
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

  ParameterListType &getParameters() {
    return Parameters;
  }

  void setExpectedParamCountIncludingThis(uint32_t count) {
    expectedParamCountIncludingThis_ = count;
  }

  uint32_t getExpectedParamCountIncludingThis() const {
    return expectedParamCountIncludingThis_;
  }

  void setThisParameter(Parameter *thisParam) {
    assert(!thisParameter && "This parameter can only be created once");
    thisParameter = thisParam;
  }
  Parameter *getThisParameter() const {
    return thisParameter;
  }

#ifndef HERMESVM_LEAN
  LazySource &getLazySource() {
    return lazySource_;
  }

  void setLazyScope(std::shared_ptr<SerializedScope> vars) {
    lazyScope_ = std::move(vars);
  }
  std::shared_ptr<SerializedScope> getLazyScope() const {
    return lazyScope_;
  }

  void setLazyClosureAlias(Variable *var) {
    lazyClosureAlias_ = var;
  }
  Variable *getLazyClosureAlias() const {
    return lazyClosureAlias_;
  }
#endif

  /// \return true if the function should be compiled lazily.
  bool isLazy() const {
#ifdef HERMESVM_LEAN
    return false;
#else
    return lazySource_.nodeKind != ESTree::NodeKind::Empty;
#endif
  }

  using iterator = BasicBlockListType::iterator;
  using const_iterator = BasicBlockListType::const_iterator;
  using reverse_iterator = BasicBlockListType::reverse_iterator;
  using const_reverse_iterator = BasicBlockListType::const_reverse_iterator;

  using arg_iterator = ParameterListType::iterator;
  using const_arg_iterator = ParameterListType::const_iterator;

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
  inline arg_iterator arg_begin() {
    return Parameters.begin();
  }
  inline arg_iterator arg_end() {
    return Parameters.end();
  }
  inline const_arg_iterator arg_begin() const {
    return Parameters.begin();
  }
  inline const_arg_iterator arg_end() const {
    return Parameters.end();
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
    return kindIsA(V->getKind(), ValueKind::FunctionKind);
  }
};

class GeneratorFunction final : public Function {
 public:
  explicit GeneratorFunction(
      Module *parent,
      Identifier originalName,
      DefinitionKind definitionKind,
      bool strictMode,
      SourceVisibility sourceVisibility,
      bool isGlobal,
      SMRange sourceRange,
      Function *insertBefore)
      : Function(
            ValueKind::GeneratorFunctionKind,
            parent,
            originalName,
            definitionKind,
            strictMode,
            sourceVisibility,
            isGlobal,
            sourceRange,
            insertBefore) {}

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::GeneratorFunctionKind);
  }
};

class GeneratorInnerFunction final : public Function {
 public:
  explicit GeneratorInnerFunction(
      Module *parent,
      Identifier originalName,
      DefinitionKind definitionKind,
      bool strictMode,
      bool isGlobal,
      SMRange sourceRange,
      Function *insertBefore)
      : Function(
            ValueKind::GeneratorInnerFunctionKind,
            parent,
            originalName,
            definitionKind,
            strictMode,
            // TODO(T84292546): change to 'Sensitive' once the outer gen fn name
            //  is used in the err stack trace instead of the inner gen fn name.
            SourceVisibility::HideSource,
            isGlobal,
            sourceRange,
            insertBefore) {
    setType(Type::createAnyType());
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::GeneratorInnerFunctionKind);
  }
};

class AsyncFunction final : public Function {
 public:
  explicit AsyncFunction(
      Module *parent,
      Identifier originalName,
      DefinitionKind definitionKind,
      bool strictMode,
      SourceVisibility sourceVisibility,
      bool isGlobal,
      SMRange sourceRange,
      Function *insertBefore)
      : Function(
            ValueKind::AsyncFunctionKind,
            parent,
            originalName,
            definitionKind,
            strictMode,
            sourceVisibility,
            isGlobal,
            sourceRange,
            insertBefore) {}

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::AsyncFunctionKind);
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

} // namespace llvh

namespace hermes {

class Module : public Value {
  Module(const Module &) = delete;
  void operator=(const Module &) = delete;

 public:
  using FunctionListType = llvh::iplist<Function>;

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
  using GlobalObjectPropertyList = std::vector<GlobalObjectProperty *>;

  std::shared_ptr<Context> Ctx;
  /// Optionally specify the top level function, if it isn't the first one.
  Function *topLevelFunction_{};

  FunctionListType FunctionList{};

  /// A list of all global properties, in the order of declaration.
  GlobalObjectPropertyList globalPropertyList_{};
  /// Mapping global property names to instances in the list.
  llvh::DenseMap<Identifier, GlobalObjectProperty *> globalPropertyMap_{};

  GlobalObject globalObject_{};
  LiteralEmpty literalEmpty{};
  LiteralUndefined literalUndefined{};
  LiteralNull literalNull{};
  LiteralBool literalFalse{false};
  LiteralBool literalTrue{true};
  EmptySentinel emptySentinel_{};

  using LiteralNumberFoldingSet = llvh::FoldingSet<LiteralNumber>;
  using LiteralBigIntFoldingSet = llvh::FoldingSet<LiteralBigInt>;
  using LiteralStringFoldingSet = llvh::FoldingSet<LiteralString>;

  LiteralNumberFoldingSet literalNumbers{};
  LiteralBigIntFoldingSet literalBigInts{};
  LiteralStringFoldingSet literalStrings{};

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
    topLevelFunction_ = topLevelFunction;
  }

  /// Return the top-level function.
  Function *getTopLevelFunction() {
    assert(
        !FunctionList.empty() && "top-level function hasn't been created yet");
    // If the top level function hasn't been overridden, return the first
    // function.
    return !topLevelFunction_ ? &*FunctionList.begin() : topLevelFunction_;
  }

  using GlobalObjectPropertyIterator = GlobalObjectPropertyList::const_iterator;

  llvh::iterator_range<GlobalObjectPropertyIterator> getGlobalProperties() {
    return {globalPropertyList_.begin(), globalPropertyList_.end()};
  }

  /// Find the specified global property and return a pointer to it or nullptr.
  GlobalObjectProperty *findGlobalProperty(Identifier name);

  /// Create the specified global property if it doesn't exist. If it does
  /// exist, its declared property is updated by a logical OR.
  GlobalObjectProperty *addGlobalProperty(Identifier name, bool declared);

  /// Remove the specified property from the list and free it.
  void eraseGlobalProperty(GlobalObjectProperty *prop);

  /// Create a new literal number of value \p value.
  LiteralNumber *getLiteralNumber(double value);

  /// Create a new literal BigInt of value \p value.
  LiteralBigInt *getLiteralBigInt(UniqueString *value);

  /// Create a new literal string of value \p value.
  LiteralString *getLiteralString(Identifier value);

  /// Create a new literal bool of value \p value.
  LiteralBool *getLiteralBool(bool value);

  /// Create a new literal 'empty'.
  LiteralEmpty *getLiteralEmpty() {
    return &literalEmpty;
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

  /// Given a list of raw strings from a template literal, get its unique id.
  uint32_t getTemplateObjectID(RawStringList &&rawStrings);

  bool isLowered() const {
    return isLowered_;
  }

  void setLowered(bool isLowered) {
    isLowered_ = isLowered;
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
  void dump();

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

} // end namespace hermes

namespace llvh {

raw_ostream &operator<<(raw_ostream &OS, const hermes::Type &T);

} // namespace llvh

#endif
