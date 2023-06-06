/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SEMA_FLOWCONTEXT_H
#define HERMES_SEMA_FLOWCONTEXT_H

#include "hermes/Sema/SemContext.h"

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
  _HERMES_SEMA_FLOW_DEFKIND(Number)  \
  _HERMES_SEMA_FLOW_DEFKIND(BigInt)  \
  _HERMES_SEMA_FLOW_DEFKIND(Any)     \
  _HERMES_SEMA_FLOW_DEFKIND(Mixed)

#define _HERMES_SEMA_FLOW_COMPLEX_TYPES \
  _HERMES_SEMA_FLOW_DEFKIND(Union)      \
  _HERMES_SEMA_FLOW_DEFKIND(Array)      \
  _HERMES_SEMA_FLOW_DEFKIND(Function)   \
  _HERMES_SEMA_FLOW_DEFKIND(Class)      \
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
};

class Type {
  TypeKind const kind_;
  /// Set to true once the type has been calculated.
  mutable bool hashed_ = false;
  /// Valid only if \c hashed_ is set to true, this is the calculated hash value
  /// of this type.
  mutable unsigned hashValue_;

 public:
  explicit Type(TypeKind kind) : kind_(kind) {}

  static bool classof(const Type *) {
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

  /// Compare this type and other type lexicographically and return -1, 0, 1
  /// correspondingly.
  /// The less than and greater than comparisons are, in some sense, arbitrary,
  /// they only need to be consistent, so we can sort types, but don't have an
  /// inherent "meaning".
  /// Equality however is well-defined: it compares structural types "deeply"
  /// and nominal types (subclasses of \c TypeWithId) "shallowly".
  int compare(const Type *other) const;

  /// Wrapper around \c compare() == 0.
  bool equals(const Type *other) const {
    return compare(other) == 0;
  }

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

class SingletonType : public Type {
 public:
  explicit SingletonType(TypeKind kind) : Type(kind) {
    assert(classof(this) && "Invalid SingletonType kind");
  }
  static bool classof(const Type *t) {
    return t->getKind() >= TypeKind::_FirstSingleton &&
        t->getKind() <= TypeKind::_LastSingleton;
  }

  /// Compare two instances of the same TypeKind.
  int _compareImpl(const SingletonType *other) const {
    assert(
        this->getKind() == other->getKind() &&
        "only the same TypeKind can be compared");
    return 0;
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

  static bool classof(const Type *t) {
    return t->getKind() >= TypeKind::_FirstPrimary &&
        t->getKind() <= TypeKind::_LastPrimary;
  }
};

template <TypeKind KIND, class BASE>
class SingleType : public BASE {
 public:
  explicit SingleType() : BASE(KIND) {}
  static bool classof(const Type *t) {
    return t->getKind() == KIND;
  }
};

using VoidType = SingleType<TypeKind::Void, PrimaryType>;
using NullType = SingleType<TypeKind::Null, PrimaryType>;
using BooleanType = SingleType<TypeKind::Boolean, PrimaryType>;
using StringType = SingleType<TypeKind::String, PrimaryType>;
using NumberType = SingleType<TypeKind::Number, PrimaryType>;
using BigIntType = SingleType<TypeKind::BigInt, PrimaryType>;

using AnyType = SingleType<TypeKind::Any, SingletonType>;
using MixedType = SingleType<TypeKind::Mixed, SingletonType>;

class UnionType : public SingleType<TypeKind::Union, Type> {
  llvh::SmallVector<Type *, 4> types_{};
  bool hasAny_ = false;
  bool hasMixed_ = false;

 public:
  explicit UnionType(llvh::SmallVector<Type *, 4> &&types);

  /// Return the members of the union.
  llvh::ArrayRef<Type *> getTypes() const {
    return types_;
  }

  bool hasAny() const {
    return hasAny_;
  }
  bool hasMixed() const {
    return hasMixed_;
  }
  /// \return true if the union contains a "void" arm.
  bool hasVoid() const {
    return llvh::isa<VoidType>(types_[0]);
  }

  /// Compare two instances of the same TypeKind.
  int _compareImpl(const UnionType *other) const;
  /// Calculate the type-specific hash.
  unsigned _hashImpl() const;

  /// Canonicalize the given \p types, by collapsing nested unions, removing
  /// duplicates, and deterministically sorting types.
  static llvh::SmallVector<Type *, 4> canonicalizeTypes(
      llvh::ArrayRef<Type *> types);
};

class ArrayType : public SingleType<TypeKind::Array, Type> {
  Type *element_ = nullptr;

 public:
  /// Initialize a new instance.
  void init(Type *element) {
    assert(!isInitialized() && "ArrayType already initilized");
    element_ = element;
  }

  Type *getElement() const {
    assert(isInitialized());
    return element_;
  }

  /// Compare two instances of the same TypeKind.
  int _compareImpl(const ArrayType *other) const;
  /// Calculate the type-specific hash.
  unsigned _hashImpl() const;

 private:
  bool isInitialized() const {
    return element_ != nullptr;
  }
};

class FunctionType : public SingleType<TypeKind::Function, Type> {
 public:
  using Param = std::pair<Identifier, Type *>;

 private:
  /// Result type.
  Type *return_ = nullptr;
  /// Optional "this" parameter type.
  Type *thisParam_ = nullptr;
  /// Parameter types.
  llvh::SmallVector<Param, 2> params_{};
  bool isAsync_ = false;
  bool isGenerator_ = false;
  /// Type has been initialized flag.
  bool initialized_ = false;

 public:
  /// Initialize a new instance.
  void init(
      Type *returnType,
      Type *thisParam,
      llvh::ArrayRef<Param> params,
      bool isAsync,
      bool isGenerator);

  Type *getReturnType() const {
    assert(isInitialized());
    return return_;
  }
  Type *getThisParam() const {
    assert(isInitialized());
    return thisParam_;
  }
  const llvh::ArrayRef<Param> getParams() const {
    assert(isInitialized());
    return params_;
  }
  bool isAsync() const {
    return isAsync_;
  }
  bool isGenerator() const {
    return isGenerator_;
  }

  /// Compare two instances of the same TypeKind.
  int _compareImpl(const FunctionType *other) const;
  /// Calculate the type-specific hash.
  unsigned _hashImpl() const;

 private:
  bool isInitialized() const {
    return initialized_;
  }
  /// Mark the type as initialized.
  void markAsInitialized() {
    initialized_ = true;
  }
};

/// A complex type annotated with a unique id (for that specific kind) to allow
/// predictable sorting without examining recursive attributes.
class TypeWithId : public Type {
  /// A unique global id used for predictable sorting.
  size_t const id_;
  /// Has this type been initialized.
  bool initialized_ = false;

 public:
  TypeWithId(TypeKind kind, const size_t id) : Type(kind), id_(id) {}

  static bool classof(const Type *t) {
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
  int _compareImpl(const TypeWithId *other) const {
    return id_ < other->id_ ? -1 : id_ == other->id_ ? 0 : 1;
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
    Identifier name;
    Type *type;
    /// The slot for PrLoad and PrStore, used during IRGen.
    /// This ideally should be computed during conversion to IR Type,
    /// but we don't have that yet.
    size_t layoutSlotIR;
    /// If the field is a method, AST for the method.
    ESTree::MethodDefinitionNode *method;
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

    /// Helper function to retrieve the field given the lookup entry.
    const Field *getField() const {
      return &classType->fields_[fieldsIndex];
    }
  };

 private:
  /// Class name.
  Identifier const className_;
  /// The non-inherited fields of the class, which are pointed to from
  /// the fieldNameMap_ for lookup from either this class or subclasses.
  llvh::SmallVector<Field, 4> fields_{};
  /// The constructor function.
  FunctionType *constructorType_ = nullptr;
  /// The .prototype property ([[HomeObject]] in the spec) contains methods.
  /// This class encodes those methods.
  /// ClassTypes which represent home objects have null homeObjectType_.
  ClassType *homeObjectType_ = nullptr;
  /// Map from field name to field lookup entry.
  /// Contains all fields in this class and all superClasses.
  /// This allows us to quickly check how many fields to allocate for the class,
  /// as well as quick lookup to see if a field exists.
  /// This also means we can query which class the field exists on easily.
  llvh::SmallDenseMap<Identifier, FieldLookupEntry> fieldNameMap_{};

  /// Super class, nullptr if this class doesn't extend anything.
  ClassType *superClass_ = nullptr;

 public:
  explicit ClassType(size_t id, Identifier className)
      : TypeWithId(TypeKind::Class, id), className_(className) {}

  /// Initialize an empty (freshly created) instance. Note that fields are
  /// immutable after this.
  void init(
      llvh::ArrayRef<Field> fields,
      FunctionType *constructorType,
      ClassType *homeObjectType,
      ClassType *superClass);

  static bool classof(const Type *t) {
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
  FunctionType *getConstructorType() const {
    assert(isInitialized());
    return constructorType_;
  }
  ClassType *getHomeObjectType() const {
    assert(isInitialized());
    return homeObjectType_;
  }
  const llvh::SmallDenseMap<Identifier, FieldLookupEntry> &getFieldNameMap()
      const {
    assert(isInitialized());
    return fieldNameMap_;
  }
  /// Return the lookup entry of a field, None if it doesn't exist.
  hermes::OptValue<FieldLookupEntry> findField(Identifier id) const;

  ClassType *getSuperClass() const {
    assert(isInitialized());
    return superClass_;
  }
};

/// The type of the constructor of the class. This is what a class expression
/// returns. Similarly, this is the type of the class variable in class
/// statements.
class ClassConstructorType : public TypeWithId {
  /// The class this constructor is for.
  ClassType *const classType_;

 public:
  explicit ClassConstructorType(size_t id, ClassType *classType)
      : TypeWithId(TypeKind::ClassConstructor, id), classType_(classType) {
    markAsInitialized();
  }

  static bool classof(const Type *t) {
    return t->getKind() == TypeKind::ClassConstructor;
  }

  ClassType *getClassType() const {
    return classType_;
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
  name##Type *get##name() const {       \
    return &name##Instance_;            \
  }
  _HERMES_SEMA_FLOW_SINGLETONS
#undef _HERMES_SEMA_FLOW_DEFKIND

  /// Get a singleton type by index.
  Type *getSingletonType(TypeKind kind) const;

  /// Canonicalize the given \p types, and create a union from them if more than
  /// one type remains. Otherwise, just return the single type.
  Type *maybeCreateUnion(llvh::ArrayRef<Type *> types);
  /// Create an initialized "maybe" type (void | null | type).
  UnionType *createPopulatedNullable(Type *type);

  ArrayType *createArray() {
    return &allocArray_.emplace_back();
  }
  FunctionType *createFunction() {
    return &allocFunction_.emplace_back();
  }
  ClassType *createClass(Identifier name) {
    return &allocClass_.emplace_back(allocClass_.size(), name);
  }
  ClassConstructorType *createClassConstructor(ClassType *classType) {
    return &allocClassConstructor_.emplace_back(
        allocClassConstructor_.size(), classType);
  }

 private:
  /// Allocate a union from the given \p types. The types may not contain
  /// duplicates or other unions.
  UnionType *createUnion(llvh::SmallVector<Type *, 4> &&types) {
    return &allocUnion_.emplace_back(std::move(types));
  }

  /// Types associated with declarations.
  llvh::DenseMap<const sema::Decl *, Type *> declTypes_{};

  /// Types associated with expression nodes.
  llvh::DenseMap<const ESTree::Node *, Type *> nodeTypes_{};

  // Declare allocators for complex types.
#define _HERMES_SEMA_FLOW_DEFKIND(name) std::deque<name##Type> alloc##name##_{};
  _HERMES_SEMA_FLOW_COMPLEX_TYPES
#undef _HERMES_SEMA_FLOW_DEFKIND

  // Define the singletons
#define _HERMES_SEMA_FLOW_DEFKIND(name) mutable name##Type name##Instance_{};
  _HERMES_SEMA_FLOW_SINGLETONS
#undef _HERMES_SEMA_FLOW_DEFKIND
};

} // namespace flow
} // namespace hermes

#endif // HERMES_SEMA_FLOWCONTEXT_H
