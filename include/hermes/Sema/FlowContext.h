/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SEMA_FLOWCONTEXT_H
#define HERMES_SEMA_FLOWCONTEXT_H

#include "hermes/AST/NativeContext.h"
#include "hermes/Sema/SemContext.h"

#include "llvh/ADT/DenseSet.h"
#include "llvh/ADT/MapVector.h"
#include "llvh/ADT/SetVector.h"

namespace hermes {

namespace ESTree {
class MethodDefinitionNode;
}

namespace flow {

#define _HERMES_SEMA_FLOW_SINGLETONS \
  _HERMES_SEMA_FLOW_DEFKIND(Void)    \
  _HERMES_SEMA_FLOW_DEFKIND(Null)    \
  _HERMES_SEMA_FLOW_DEFKIND(Boolean) \
  _HERMES_SEMA_FLOW_DEFKIND(String)  \
  _HERMES_SEMA_FLOW_DEFKIND(CPtr)    \
  _HERMES_SEMA_FLOW_DEFKIND(Number)  \
  _HERMES_SEMA_FLOW_DEFKIND(BigInt)  \
  _HERMES_SEMA_FLOW_DEFKIND(Any)     \
  _HERMES_SEMA_FLOW_DEFKIND(Mixed)

#define _HERMES_SEMA_FLOW_COMPLEX_TYPES      \
  _HERMES_SEMA_FLOW_DEFKIND(Union)           \
  _HERMES_SEMA_FLOW_DEFKIND(Array)           \
  _HERMES_SEMA_FLOW_DEFKIND(Tuple)           \
  _HERMES_SEMA_FLOW_DEFKIND(UntypedFunction) \
  _HERMES_SEMA_FLOW_DEFKIND(TypedFunction)   \
  _HERMES_SEMA_FLOW_DEFKIND(NativeFunction)  \
  _HERMES_SEMA_FLOW_DEFKIND(Class)           \
  _HERMES_SEMA_FLOW_DEFKIND(ClassConstructor)

enum class TypeKind : uint8_t {
#define _HERMES_SEMA_FLOW_DEFKIND(name) name,
  _HERMES_SEMA_FLOW_SINGLETONS _HERMES_SEMA_FLOW_COMPLEX_TYPES
#undef _HERMES_SEMA_FLOW_DEFKIND

      _FirstPrimary = Void,
  _LastPrimary = BigInt,
  _FirstSingleton = Void,
  _LastSingleton = Mixed,
  _FirstId = Class,
  _LastId = ClassConstructor,
  _FirstFunction = UntypedFunction,
  _LastFunction = NativeFunction,
};

/// The backing storage for Types.
/// Indicates the TypeKind and can be compared for equality, etc.
class TypeInfo {
  TypeKind const kind_;
  /// Set to true once the type has been calculated.
  mutable bool hashed_ = false;
  /// Valid only if \c hashed_ is set to true, this is the calculated hash value
  /// of this type.
  mutable unsigned hashValue_;

 public:
  explicit TypeInfo(TypeKind kind) : kind_(kind) {}

  static bool classof(const TypeInfo *) {
    return true;
  }

  bool isSingleton() const {
    return kind_ <= TypeKind::_LastSingleton;
  }

  TypeKind getKind() const {
    return kind_;
  }

  /// Return the TypeKind as a string.
  llvh::StringRef getKindName() const;

  /// Comparison stores visited set for a pair of (this, other).
  /// If the comparison diverges or terminates prior to seeing a visited pair,
  /// it operates as if there was no visited set in the usual fashion.
  /// However, if we see a visited pair, that must mean we haven't diverged
  /// prior to it, and we can know that the Types must be equal.
  /// The kinds must have been checked somewhere else, or are being checked up
  /// the callstack.
  /// If we see the same pair again, we can't perform any new work because we
  /// have no new information at that point.
  class CompareState {
   public:
    /// Pairs of (this, other) that we have visited.
    llvh::SetVector<std::pair<const TypeInfo *, const TypeInfo *>> visited{};

    /// Cache for memoizing computation that could otherwise be slow.
    /// Pairs of (this, other) and the result of the comparison/equality.
    /// Note that for equals(), the value is a bool stored as int, so it can be
    /// cast directly to bool for returning.
    /// Never gets elements popped during comparison, unlike the visited set.
    llvh::SmallDenseMap<std::pair<const TypeInfo *, const TypeInfo *>, int>
        cache{};
  };

  /// Compare this type and other type lexicographically and return -1, 0, 1
  /// correspondingly.
  /// The less than and greater than comparisons are, in some sense, arbitrary,
  /// they only need to be consistent, so we can sort types, but don't have an
  /// inherent "meaning".
  /// Equality however is well-defined: it compares structural types "deeply"
  /// and nominal types (subclasses of \c TypeWithId) "shallowly".
  ///
  /// The types must be known to be non-looping; looping unions
  /// are not comparable because we have no good way to order cycles.
  /// This restriction isn't enforced - the caller must know the types are
  /// non-looping and breaking this precondition will result in unpredictable
  /// behavior.
  int compare(const TypeInfo *other) const;

  /// \param state the state for tracking cycles through the comparison.
  int compare(const TypeInfo *other, CompareState &state) const;

  /// Check whether types are equal.
  /// May be used on looping types (with known looping union arms).
  bool equals(const TypeInfo *other) const;

  /// \param state the state for tracking cycles through the comparison.
  ///   The state is restored to its pre-call state when the function returns.
  bool equals(const TypeInfo *other, CompareState &state) const;

  /// Calculate a shallow hash of this type for hash tables, etc.
  /// Types that compare equal using \c compare() must have the same hash.
  /// However, the hash of each type is cached after it is calculated the first
  /// time, so the amortized cost should be O(1).
  /// TODO: The cache mechanism is kept around for faster lookups and to reduce
  /// calls to llvh::hash_value, along with the ability to make hashing slightly
  /// more expensive and deeper even if we aren't able to hash the entire
  /// structure always.
  unsigned hash() const;
};

/// Used when TypeInfo is the key in, e.g. std::map.
struct TypeInfoLessThan {
  bool operator()(const TypeInfo *lhs, const TypeInfo *rhs) const {
    return lhs->compare(rhs) < 0;
  }
};

/// An instance of a creation of a Type.
/// Associates the backing storage TypeInfo with the AST node that created it.
class Type {
 public:
  /// Backing storage indicating the kind of the type.
  /// nullptr before it's initialized.
  TypeInfo *info = nullptr;

  /// If non-null, the AST node used for the source location for this type.
  /// If null, this type was created for some reason that was unassociated with
  /// JS source.
  ESTree::Node *node = nullptr;

  explicit Type(TypeInfo *info = nullptr, ESTree::Node *node = nullptr)
      : info(info), node(node) {}
};

class SingletonType : public TypeInfo {
 public:
  explicit SingletonType(TypeKind kind) : TypeInfo(kind) {
    assert(classof(this) && "Invalid SingletonType kind");
  }
  static bool classof(const TypeInfo *t) {
    return t->getKind() >= TypeKind::_FirstSingleton &&
        t->getKind() <= TypeKind::_LastSingleton;
  }

  /// Compare two instances of the same TypeKind.
  int _compareImpl(const SingletonType *other, CompareState &state) const {
    assert(
        this->getKind() == other->getKind() &&
        "only the same TypeKind can be compared");
    return 0;
  }
  /// Compare two instances of the same TypeKind.
  bool _equalsImpl(const SingletonType *other, CompareState &state) const {
    return _compareImpl(other, state) == 0;
  }
  /// Calculate the type-specific hash.
  unsigned _hashImpl() const {
    return (unsigned)getKind();
  }
};

class PrimaryType : public SingletonType {
 public:
  explicit PrimaryType(TypeKind kind) : SingletonType(kind) {
    assert(classof(this) && "Invalid PrimaryType kind");
  }

  static bool classof(const TypeInfo *t) {
    return t->getKind() >= TypeKind::_FirstPrimary &&
        t->getKind() <= TypeKind::_LastPrimary;
  }
};

template <TypeKind KIND, class BASE>
class SingleType : public BASE {
 public:
  explicit SingleType() : BASE(KIND) {}
  static bool classof(const TypeInfo *t) {
    return t->getKind() == KIND;
  }
};

using VoidType = SingleType<TypeKind::Void, PrimaryType>;
using NullType = SingleType<TypeKind::Null, PrimaryType>;
using BooleanType = SingleType<TypeKind::Boolean, PrimaryType>;
using StringType = SingleType<TypeKind::String, PrimaryType>;
using CPtrType = SingleType<TypeKind::CPtr, PrimaryType>;
using NumberType = SingleType<TypeKind::Number, PrimaryType>;
using BigIntType = SingleType<TypeKind::BigInt, PrimaryType>;

using AnyType = SingleType<TypeKind::Any, SingletonType>;
using MixedType = SingleType<TypeKind::Mixed, SingletonType>;

/// Unions are handled specially because their existence and layout is dependent
/// on their arms.
///
/// Some of the union arms may be "looping":
/// they may point to types which eventually point back to themselves,
/// meaning there's no end point to the recursive comparison.
/// Comparable union arms are non-looping.
///
/// For unions which may be looping (e.g. via type aliases but not via
/// nominal types), they are first populated with unordered arms.
/// The arms are sorted and uniqued during type alias resolution,
/// with the looping types split from canonicalized types (sorted, uniqued).
/// Looping types can be uniqued slowly using \c TypeInfo::equals.
/// Once canonicalization happens, they are registered as canonicalized in
/// UnionType.
///
/// Unions can also be created with known non-looping/looping types.
///
/// In either case, UnionType will not be used when it is known to only have one
/// type following canonicalization.
/// It will be replaced with the single type.
class UnionType : public SingleType<TypeKind::Union, TypeInfo> {
  /// When numNonLoopingTypes_ is non-negative:
  ///   The first numNonLoopingTypes_ are canonical (sorted, uniqued).
  ///   The other types are recursive.
  /// When numNonLoopingTypes_ is -1:
  ///   No types have been checked for canonicalization.
  llvh::SmallVector<Type *, 4> types_{};

  /// Number of types at the start of types_ that are known to be non-recursive,
  /// sorted, and uniqued.
  /// -1 prior to any processing (if the types have no known information yet).
  int32_t numNonLoopingTypes_ = -1;

  bool hasAny_ = false;
  bool hasMixed_ = false;

 public:
  /// Create a UnionType with an unknown list of types to be canonicalized
  /// later.
  explicit UnionType(llvh::SmallVector<Type *, 4> &&types);
  /// Create a UnionType already knowing the canonical and recursive types.
  explicit UnionType(
      llvh::SmallVector<Type *, 4> &&nonLoopingTypes,
      llvh::SmallVector<Type *, 4> &&loopingTypes) {
    setCanonicalTypes(std::move(nonLoopingTypes), std::move(loopingTypes));
  }

  /// Set the type arms for the union after the canonical representation is
  /// known.
  void setCanonicalTypes(
      llvh::SmallVector<Type *, 4> &&nonLoopingTypes,
      llvh::SmallVector<Type *, 4> &&loopingTypes);

  /// \return the number of known non-looping types if that has been determined,
  /// -1 if it hasn't.
  /// The number will have been calculated during FlowChecker,
  /// so callers after the FlowChecker has run generally don't have to worry
  /// about it being -1.
  int32_t getNumNonLoopingTypes() const {
    return numNonLoopingTypes_;
  }

  /// \return the members of the union.
  llvh::ArrayRef<Type *> getTypes() const {
    return types_;
  }
  /// \return the non-looping members of the union.
  llvh::ArrayRef<Type *> getNonLoopingTypes() const {
    assert(numNonLoopingTypes_ >= 0 && "types haven't been canonicalized yet");
    return getTypes().take_front(numNonLoopingTypes_);
  }
  /// \return the looping members of the union.
  llvh::ArrayRef<Type *> getLoopingTypes() const {
    assert(numNonLoopingTypes_ >= 0 && "types haven't been canonicalized yet");
    return getTypes().drop_front(numNonLoopingTypes_);
  }

  bool hasAny() const {
    assert(numNonLoopingTypes_ >= 0 && "types haven't been canonicalized yet");
    return hasAny_;
  }
  bool hasMixed() const {
    assert(numNonLoopingTypes_ >= 0 && "types haven't been canonicalized yet");
    return hasMixed_;
  }
  /// \return true if the union contains a "void" arm.
  bool hasVoid() const {
    assert(numNonLoopingTypes_ >= 0 && "types haven't been canonicalized yet");
    return llvh::isa<VoidType>(types_[0]->info);
  }
  /// \return true if the union contains a "null" arm.
  bool hasNull() const {
    assert(numNonLoopingTypes_ >= 0 && "types haven't been canonicalized yet");
    static_assert(
        (int)TypeKind::Null == (int)TypeKind::Void + 1,
        "null and void aren't adjacent");
    // Null must be either in the first or second slot.
    if (llvh::isa<NullType>(types_[0]->info))
      return true;
    return types_.size() > 1 && llvh::isa<NullType>(types_[1]->info);
  }

  /// Compare two instances of the same TypeKind.
  int _compareImpl(const UnionType *other, CompareState &state) const;
  /// Compare two instances of the same TypeKind.
  bool _equalsImpl(const UnionType *other, CompareState &state) const;
  /// Calculate the type-specific hash.
  unsigned _hashImpl() const;

  /// Collapse nested unions, remove duplicates,
  /// and deterministically sort the types.
  /// Categorize the resulting types into non-looping and looping.
  /// NOTE: This function isn't to be used on type aliases, which can have
  /// union-only cycles, multiple layers of nested unions, etc. Those are to be
  /// resolved in the FlowChecker's DeclareScopeTypes call.
  /// This function only works when every element of \p types has itself already
  /// been correctly resolved, with nested unions collapsed and no duplicates.
  static void canonicalizeTypes(
      llvh::ArrayRef<Type *> types,
      llvh::SmallVectorImpl<Type *> &nonLoopingTypes,
      llvh::SmallVectorImpl<Type *> &loopingTypes);

  /// Unique types and sort them according to \c TypeInfo::compare.
  /// \param types the types that are known to be non-recursive.
  static void sortAndUniqueNonLoopingTypes(
      llvh::SmallVectorImpl<Type *> &types);

  /// Remove all the duplicates in the list of looping \p types.
  /// Runs in quadratic time on the length of the types because they can't be
  /// sorted.
  static void uniqueLoopingTypesSlow(llvh::SmallVectorImpl<Type *> &types);

 private:
  // Iterate types_ and set the hasAny_ and hasMixed_ flags.
  void setHasAnyMixed();
};

class ArrayType : public SingleType<TypeKind::Array, TypeInfo> {
  Type *element_ = nullptr;

 public:
  /// Initialize a new instance.
  explicit ArrayType(Type *element) : element_(element) {}

  Type *getElement() const {
    assert(isInitialized());
    return element_;
  }

  /// Compare two instances of the same TypeKind.
  int _compareImpl(const ArrayType *other, CompareState &state) const;
  /// Compare two instances of the same TypeKind.
  bool _equalsImpl(const ArrayType *other, CompareState &state) const;
  /// Calculate the type-specific hash.
  unsigned _hashImpl() const;

 private:
  bool isInitialized() const {
    return element_ != nullptr;
  }
};

class TupleType : public TypeInfo {
  llvh::SmallVector<Type *, 4> types_{};

 public:
  /// Initialize a new instance.
  explicit TupleType(llvh::ArrayRef<Type *> types) : TypeInfo(TypeKind::Tuple) {
    types_.append(types.begin(), types.end());
  }

  llvh::ArrayRef<Type *> getTypes() const {
    return types_;
  }

  /// Compare two instances of the same TypeKind.
  int _compareImpl(const TupleType *other, CompareState &state) const;
  /// Compare two instances of the same TypeKind.
  bool _equalsImpl(const TupleType *other, CompareState &state) const;
  /// Calculate the type-specific hash.
  unsigned _hashImpl() const;

  static bool classof(const TypeInfo *t) {
    return t->getKind() == TypeKind::Tuple;
  }
};

class BaseFunctionType : public TypeInfo {
 private:
  bool isAsync_ = false;
  bool isGenerator_ = false;

 public:
  /// Initialize a new typed function instance.
  explicit BaseFunctionType(TypeKind kind, bool isAsync, bool isGenerator)
      : TypeInfo(kind), isAsync_(isAsync), isGenerator_(isGenerator) {}

  bool isAsync() const {
    return isAsync_;
  }
  bool isGenerator() const {
    return isGenerator_;
  }

  static bool classof(const TypeInfo *t) {
    return t->getKind() >= TypeKind::_FirstFunction &&
        t->getKind() <= TypeKind::_LastFunction;
  }
};

class TypedFunctionType : public BaseFunctionType {
 public:
  using Param = std::pair<Identifier, Type *>;

 private:
  /// Result type.
  Type *return_ = nullptr;
  /// Optional "this" parameter type.
  Type *thisParam_ = nullptr;
  /// Parameter types.
  llvh::SmallVector<Param, 2> params_{};

 public:
  /// Initialize a new typed function instance.
  explicit TypedFunctionType(
      Type *returnType,
      Type *thisParam,
      llvh::ArrayRef<Param> params,
      bool isAsync,
      bool isGenerator)
      : BaseFunctionType(TypeKind::TypedFunction, isAsync, isGenerator),
        return_(returnType),
        thisParam_(thisParam) {
    params_.append(params.begin(), params.end());
  }

  Type *getReturnType() const {
    return return_;
  }
  Type *getThisParam() const {
    return thisParam_;
  }
  const llvh::ArrayRef<Param> getParams() const {
    return params_;
  }

  /// Compare two instances of the same TypeKind.
  int _compareImpl(const TypedFunctionType *other, CompareState &state) const;
  /// Compare two instances of the same TypeKind.
  bool _equalsImpl(const TypedFunctionType *other, CompareState &state) const;
  /// Calculate the type-specific hash.
  unsigned _hashImpl() const;

  static bool classof(const TypeInfo *t) {
    return t->getKind() == TypeKind::TypedFunction;
  }
};

class NativeFunctionType : public BaseFunctionType {
 public:
  using Param = TypedFunctionType::Param;

 private:
  /// Result type.
  Type *return_ = nullptr;
  /// Parameter types.
  llvh::SmallVector<Param, 2> params_{};
  /// The signature of the native function.
  NativeSignature *signature_;

 public:
  /// Initialize a new native function instance.
  explicit NativeFunctionType(
      Type *returnType,
      llvh::ArrayRef<TypedFunctionType::Param> params,
      NativeSignature *signature)
      : BaseFunctionType(TypeKind::NativeFunction, false, false),
        return_(returnType),
        signature_(signature) {
    assert(signature && "signature must be non-null");
    params_.append(params.begin(), params.end());
  }

  Type *getReturnType() const {
    return return_;
  }
  const llvh::ArrayRef<Param> getParams() const {
    return params_;
  }
  NativeSignature *getSignature() const {
    return signature_;
  }

  /// Compare two instances of the same TypeKind.
  int _compareImpl(const NativeFunctionType *other, CompareState &state) const;
  /// Compare two instances of the same TypeKind.
  bool _equalsImpl(const NativeFunctionType *other, CompareState &state) const;
  /// Calculate the type-specific hash.
  unsigned _hashImpl() const;

  static bool classof(const TypeInfo *t) {
    return t->getKind() == TypeKind::NativeFunction;
  }
};

class UntypedFunctionType : public BaseFunctionType {
 public:
  explicit UntypedFunctionType(bool isAsync, bool isGenerator)
      : BaseFunctionType(TypeKind::UntypedFunction, isAsync, isGenerator) {}

  /// Compare two instances of the same TypeKind.
  int _compareImpl(const UntypedFunctionType *other, CompareState &state) const;
  /// Compare two instances of the same TypeKind.
  bool _equalsImpl(const UntypedFunctionType *other, CompareState &state) const;
  /// Calculate the type-specific hash.
  unsigned _hashImpl() const;

  static bool classof(const TypeInfo *t) {
    return t->getKind() == TypeKind::UntypedFunction;
  }
};

/// A complex type annotated with a unique id (for that specific kind) to allow
/// predictable sorting without examining recursive attributes.
class TypeWithId : public TypeInfo {
  /// A unique global id used for predictable sorting.
  size_t const id_;
  /// Has this type been initialized.
  bool initialized_ = false;

 public:
  TypeWithId(TypeKind kind, const size_t id) : TypeInfo(kind), id_(id) {}

  static bool classof(const TypeInfo *t) {
    return t->getKind() >= TypeKind::_FirstId &&
        t->getKind() <= TypeKind::_LastId;
  }

  /// \return the per-type unique id.
  size_t getId() const {
    return id_;
  }

  /// \return false if this type has only been forward decclared but not
  ///     initialized yet.
  bool isInitialized() const {
    return initialized_;
  }

  /// Compare two instances of the same TypeKind.
  int _compareImpl(const TypeWithId *other, CompareState &state) const {
    return id_ < other->id_ ? -1 : id_ == other->id_ ? 0 : 1;
  }
  /// Compare two instances of the same TypeKind.
  int _equalsImpl(const TypeWithId *other, CompareState &state) const {
    return _compareImpl(other, state) == 0;
  }
  /// Calculate the type-specific hash.
  unsigned _hashImpl() const;

 protected:
  /// Mark the type as initialized.
  void markAsInitialized() {
    initialized_ = true;
  }
};

class ClassType : public TypeWithId {
 public:
  /// Represents either a class field or a method.
  struct Field {
    const Identifier name;
    Type *const type;
    /// The slot for PrLoad and PrStore, used during IRGen.
    /// This ideally should be computed during conversion to IR Type,
    /// but we don't have that yet.
    const size_t layoutSlotIR;
    /// If the field is a method, AST for the method.
    ESTree::MethodDefinitionNode *const method;

    /// Whether this field is a method that has been overridden by a subclass.
    /// This is the only field that can be modified after initialization, as
    /// subclasses are processed.
    bool overridden{false};

    Field(
        Identifier name,
        Type *type,
        size_t layoutSlotIR,
        ESTree::MethodDefinitionNode *method = nullptr)
        : name(name), type(type), layoutSlotIR(layoutSlotIR), method(method) {}

    bool isMethod() const {
      return method != nullptr;
    }
  };

  /// Lookup entry in the table, used to quickly find fields.
  struct FieldLookupEntry {
    /// The class on which this field is defined.
    /// Fields may occur anywhere in the inheritance chain.
    ClassType *classType;

    /// Slot index in the fields_ list in classType.
    size_t fieldsIndex;

    explicit FieldLookupEntry() = default;
    explicit FieldLookupEntry(ClassType *classType, size_t fieldsIndex)
        : classType(classType), fieldsIndex(fieldsIndex) {}

    /// Helper functions to retrieve the field given the lookup entry.
    const Field *getField() const {
      return &classType->fields_[fieldsIndex];
    }
    Field *getFieldMut() const {
      return &classType->fields_[fieldsIndex];
    }
  };

 private:
  /// Class name.
  Identifier const className_;
  /// The non-inherited fields of the class, which are pointed to from
  /// the fieldNameMap_ for lookup from either this class or subclasses.
  llvh::SmallVector<Field, 4> fields_{};
  /// The constructor function (FunctionType).
  Type *constructorType_ = nullptr;
  /// The .prototype property ([[HomeObject]] in the spec) contains methods.
  /// This class encodes those methods.
  /// ClassTypes which represent home objects have null homeObjectType_.
  Type *homeObjectType_ = nullptr;
  /// Map from field name to field lookup entry.
  /// Contains all fields in this class and all superClasses.
  /// This allows us to quickly check how many fields to allocate for the class,
  /// as well as quick lookup to see if a field exists.
  /// This also means we can query which class the field exists on easily.
  /// Use a MapVector to make sure it's deterministic to iterate.
  llvh::SmallMapVector<Identifier, FieldLookupEntry, 4> fieldNameMap_{};

  /// Super class, nullptr if this class doesn't extend anything.
  Type *superClass_ = nullptr;

 public:
  explicit ClassType(size_t id, Identifier className);
  explicit ClassType(
      size_t id,
      Identifier className,
      llvh::ArrayRef<Field> fields,
      Type *constructorType,
      Type *homeObjectType,
      Type *superClass);

  /// Initialize an empty (freshly created) instance. Note that fields are
  /// immutable after this.
  void init(
      llvh::ArrayRef<Field> fields,
      Type *constructorType,
      Type *homeObjectType,
      Type *superClass);

  static bool classof(const TypeInfo *t) {
    return t->getKind() == TypeKind::Class;
  }

  Identifier getClassName() const {
    return className_;
  }
  llvh::StringRef getClassNameOrDefault() const {
    return className_.isValid() ? className_.str()
                                : llvh::StringLiteral("<anonymous>");
  }
  const llvh::ArrayRef<Field> getFields() const {
    assert(isInitialized());
    return fields_;
  }
  Type *getConstructorType() const {
    assert(isInitialized());
    return constructorType_;
  }
  TypedFunctionType *getConstructorTypeInfo() const {
    assert(isInitialized());
    return llvh::cast_or_null<TypedFunctionType>(constructorType_->info);
  }
  Type *getHomeObjectType() const {
    assert(isInitialized());
    return homeObjectType_;
  }
  ClassType *getHomeObjectTypeInfo() const {
    assert(isInitialized());
    return llvh::cast_or_null<ClassType>(
        homeObjectType_ ? homeObjectType_->info : nullptr);
  }
  const llvh::SmallMapVector<Identifier, FieldLookupEntry, 4> &getFieldNameMap()
      const {
    assert(isInitialized());
    return fieldNameMap_;
  }
  /// Return the lookup entry of a field, None if it doesn't exist.
  hermes::OptValue<FieldLookupEntry> findField(Identifier id) const;

  Type *getSuperClass() const {
    assert(isInitialized());
    return superClass_;
  }
  ClassType *getSuperClassInfo() const {
    assert(isInitialized());
    return llvh::cast_or_null<ClassType>(
        superClass_ ? superClass_->info : nullptr);
  }
};

/// The type of the constructor of the class. This is what a class expression
/// returns. Similarly, this is the type of the class variable in class
/// statements.
class ClassConstructorType : public TypeWithId {
  /// The class this constructor is for.
  Type *const classType_;

 public:
  explicit ClassConstructorType(size_t id, Type *classType)
      : TypeWithId(TypeKind::ClassConstructor, id), classType_(classType) {
    markAsInitialized();
  }

  static bool classof(const TypeInfo *t) {
    return t->getKind() == TypeKind::ClassConstructor;
  }

  Type *getClassType() const {
    return classType_;
  }
  ClassType *getClassTypeInfo() const {
    return llvh::cast<ClassType>(classType_->info);
  }
};

class FlowContext {
  friend class FlowTypesDumper;

 public:
  /// A dummy type to prevent accidentally invoking mutating accessors.
  struct ForUpdate {};

  FlowContext(const FlowContext &) = delete;
  void operator=(const FlowContext &) = delete;

  FlowContext();
  ~FlowContext();

  const auto &declTypeMap() const {
    return declTypes_;
  }
  const auto &nodeTypeMap() const {
    return nodeTypes_;
  }
  auto &declTypeMap(ForUpdate) {
    return declTypes_;
  }
  auto &nodeTypeMap(ForUpdate) {
    return nodeTypes_;
  }

  /// Return the type associated with the specified decl, or nullptr.
  Type *findDeclType(const sema::Decl *decl) const;

  /// Return the type, which must exist, associated with the specified
  /// declaration.
  Type *getDeclType(const sema::Decl *decl) const;

  /// Associate a type with an expression node. Do nothing if the specified
  /// type is nullptr.
  void setNodeType(const ESTree::Node *node, Type *type);

  /// Return the type associated with the specified AST node, or nullptr.
  Type *findNodeType(const ESTree::Node *node) const;

  /// Return the optional type associated with a node or "any" if no type.
  Type *getNodeTypeOrAny(const ESTree::Node *node) const;

  // Singleton accessors
#define _HERMES_SEMA_FLOW_DEFKIND(name) \
  TypeInfo *get##name##Info() const {   \
    return &name##Info_;                \
  }                                     \
  Type *get##name() const {             \
    return &name##Instance_;            \
  }
  _HERMES_SEMA_FLOW_SINGLETONS
#undef _HERMES_SEMA_FLOW_DEFKIND

  /// Get a singleton type by index.
  Type *getSingletonType(TypeKind kind) const;

  /// Options on which arms to exclude from a created union.
  struct UnionExcludes {
    /// Whether to remove 'void' from the result.
    bool excludeVoid = false;
    /// Whether to remove 'null' from the result.
    bool excludeNull = false;

    UnionExcludes() {}
  };

  /// Allocate a union from the given \p types.
  /// The types may contain duplicates or other unions.
  UnionType *createNonCanonicalizedUnion(llvh::SmallVector<Type *, 4> &&types) {
    return &allocUnion_.emplace_back(std::move(types));
  }
  /// Canonicalize the given \p types, and create a union from them if more than
  /// one type remains. Otherwise, just return the single type.
  /// \param excludes a set of TypeKinds to erase from the final type.
  /// \pre the unified list of types must include at least one element not in \p
  ///   excludes.
  TypeInfo *maybeCreateUnion(
      llvh::ArrayRef<Type *> types,
      UnionExcludes excludes = {});
  /// Create an initialized "maybe" type (void | null | type).
  UnionType *createPopulatedNullable(Type *type);

  ArrayType *createArray(Type *element) {
    assert(element);
    return &allocArray_.emplace_back(element);
  }
  TupleType *createTuple(llvh::ArrayRef<Type *> types) {
    return &allocTuple_.emplace_back(types);
  }
  TypedFunctionType *createFunction(
      Type *returnType,
      Type *thisParam,
      llvh::ArrayRef<TypedFunctionType::Param> params,
      bool isAsync,
      bool isGenerator) {
    return &allocTypedFunction_.emplace_back(
        returnType, thisParam, params, isAsync, isGenerator);
  }
  NativeFunctionType *createNativeFunction(
      Type *returnType,
      llvh::ArrayRef<TypedFunctionType::Param> params,
      NativeSignature *signature) {
    return &allocNativeFunction_.emplace_back(returnType, params, signature);
  }
  UntypedFunctionType *createUntypedFunction(bool isAsync, bool isGenerator) {
    return &allocUntypedFunction_.emplace_back(isAsync, isGenerator);
  }
  ClassType *createClass(Identifier name) {
    return &allocClass_.emplace_back(allocClass_.size(), name);
  }
  ClassConstructorType *createClassConstructor(Type *classType) {
    return &allocClassConstructor_.emplace_back(
        allocClassConstructor_.size(), classType);
  }
  Type *createType(ESTree::Node *node = nullptr) {
    return &allocTypes_.emplace_back(nullptr, node);
  }
  Type *createType(TypeInfo *type, ESTree::Node *node = nullptr) {
    return &allocTypes_.emplace_back(type, node);
  }

 private:
  /// Allocate a union from the given \p types. The types may not contain
  /// duplicates or other unions.
  UnionType *createCanonicalizedUnion(
      llvh::SmallVector<Type *, 4> &&nonLoopingTypes,
      llvh::SmallVector<Type *, 4> &&loopingTypes) {
    return &allocUnion_.emplace_back(
        std::move(nonLoopingTypes), std::move(loopingTypes));
  }

  /// Types associated with declarations.
  llvh::DenseMap<const sema::Decl *, Type *> declTypes_{};

  /// Types associated with expression nodes.
  llvh::DenseMap<const ESTree::Node *, Type *> nodeTypes_{};

  std::deque<Type> allocTypes_{};

  // Declare allocators for complex types.
#define _HERMES_SEMA_FLOW_DEFKIND(name) std::deque<name##Type> alloc##name##_{};
  _HERMES_SEMA_FLOW_COMPLEX_TYPES
#undef _HERMES_SEMA_FLOW_DEFKIND

  // Define the singletons
#define _HERMES_SEMA_FLOW_DEFKIND(name) \
  mutable name##Type name##Info_{};     \
  mutable Type name##Instance_{&name##Info_};
  _HERMES_SEMA_FLOW_SINGLETONS
#undef _HERMES_SEMA_FLOW_DEFKIND
};

} // namespace flow
} // namespace hermes

#endif // HERMES_SEMA_FLOWCONTEXT_H
