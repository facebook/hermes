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

#define _HERMES_SEMA_FLOW_COMPLEX_TYPES       \
  _HERMES_SEMA_FLOW_DEFKIND(Union)            \
  _HERMES_SEMA_FLOW_DEFKIND(Function)         \
  _HERMES_SEMA_FLOW_DEFKIND(Class)            \
  _HERMES_SEMA_FLOW_DEFKIND(ClassConstructor) \
  _HERMES_SEMA_FLOW_DEFKIND(Array)

enum class TypeKind : uint8_t {
#define _HERMES_SEMA_FLOW_DEFKIND(name) name,
  _HERMES_SEMA_FLOW_SINGLETONS _HERMES_SEMA_FLOW_COMPLEX_TYPES
#undef _HERMES_SEMA_FLOW_DEFKIND

      _FirstPrimary = Void,
  _LastPrimary = BigInt,
  _FirstSingleton = Void,
  _LastSingleton = Mixed,
  _FirstId = Function,
  _LastId = Array,
};

class Type {
  TypeKind const kind_;

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

  llvh::StringRef getKindName() const;
};

class PrimaryType : public Type {
 public:
  explicit PrimaryType(TypeKind kind) : Type(kind) {
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

using AnyType = SingleType<TypeKind::Any, Type>;
using MixedType = SingleType<TypeKind::Mixed, Type>;

class UnionType : public SingleType<TypeKind::Union, Type> {
  llvh::SmallVector<Type *, 4> types_{};
  bool hasAny_ = false;
  bool hasMixed_ = false;

 public:
  /// Initialize an empty (freshly created) instance.
  void init(llvh::ArrayRef<Type *> types);

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
  /// \return true if the union contains exactly the specified type.
  bool hasType(Type *t) const;

  /// \return true if the two unions are structurally equal.
  bool isUnionEqual(const UnionType *o) const;

  /// Hash the contents of the union. This is O(size-of-union).
  size_t calcUnionHash() const;
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

  /// Calculate a hash of the type kind combined with the id.
  size_t calcTypeWithIdHash() const;

 protected:
  /// Mark the type as initialized.
  void markAsInitialized() {
    initialized_ = true;
  }
};

class FunctionType : public TypeWithId {
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

 public:
  explicit FunctionType(size_t id) : TypeWithId(TypeKind::Function, id) {}

  /// Initialize a new instance.
  void init(
      Type *returnType,
      Type *thisParam,
      llvh::ArrayRef<Param> params,
      bool isAsync,
      bool isGenerator);

  static bool classof(const Type *t) {
    return t->getKind() == TypeKind::Function;
  }

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
};

class ClassType : public TypeWithId {
 public:
  struct Field {
    Identifier name;
    Type *type;
    Field(Identifier name, Type *type) : name(name), type(type) {}
  };

 private:
  /// Class name.
  Identifier const className_;
  /// Fields.
  llvh::SmallVector<Field, 4> fields_{};
  /// The constructor function.
  FunctionType *constructorType_ = nullptr;
  /// Map from identifier to field index.
  llvh::SmallDenseMap<Identifier, size_t> fieldNameMap_{};

 public:
  explicit ClassType(size_t id, Identifier className)
      : TypeWithId(TypeKind::Class, id), className_(className) {}

  /// Initialize an empty (freshly created) instance. Note that fields are
  /// immutable after this.
  void init(llvh::ArrayRef<Field> fields, FunctionType *constructorType);

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
  const llvh::SmallDenseMap<Identifier, size_t> &getFieldNameMap() const {
    assert(isInitialized());
    return fieldNameMap_;
  }
  /// Return the index of a field which must exist.
  size_t getFieldIndex(Identifier id) const;

  /// Return a pointer to an existing field or nullptr.
  const Field *findField(Identifier id) const;
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

class ArrayType : public TypeWithId {
  Type *element_ = nullptr;

 public:
  explicit ArrayType(size_t id) : TypeWithId(TypeKind::Array, id) {}

  /// Initialize a new instance.
  void init(Type *element) {
    assert(!isInitialized() && "ArrayType already initilized");
    element_ = element;
    markAsInitialized();
  }

  static bool classof(const Type *t) {
    return t->getKind() == TypeKind::Array;
  }

  Type *getElement() const {
    assert(isInitialized());
    return element_;
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

  /// Create an initialized union.
  UnionType *createPopulatedUnion(llvh::ArrayRef<Type *> types);
  /// Create an initialized "maybe" type (void | null | type).
  UnionType *createPopulatedNullable(Type *type);

  UnionType *createUnion() {
    return &allocUnion_.emplace_back();
  }
  FunctionType *createFunction() {
    return &allocFunction_.emplace_back(allocFunction_.size());
  }
  ClassType *createClass(Identifier name) {
    return &allocClass_.emplace_back(allocClass_.size(), name);
  }
  ClassConstructorType *createClassConstructor(ClassType *classType) {
    return &allocClassConstructor_.emplace_back(
        allocClassConstructor_.size(), classType);
  }
  ArrayType *createArray() {
    return &allocArray_.emplace_back(allocArray_.size());
  }

 private:
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
