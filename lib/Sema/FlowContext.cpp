/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Sema/FlowContext.h"

#include "llvh/ADT/Hashing.h"

namespace hermes {
namespace flow {

namespace {
/// Compare two sequences of types lexicographically.
template <typename It, typename Comp>
int lexicographicalComparison(
    It first1,
    It last1,
    It first2,
    It last2,
    Comp compare) {
  for (; first1 != last1; ++first1, ++first2) {
    // If sequence 2 is a prefix of sequence 1, sequence 1 is greater.
    if (first2 == last2)
      return 1;
    if (auto tmpRes = compare(*first1, *first2))
      return tmpRes;
  }

  return first2 == last2 ? 0 : -1;
}

template <typename It, typename F>
llvh::hash_code hashTypes(It first, It last, F hash) {
  llvh::hash_code hashCode = 0;
  for (; first != last; ++first) {
    hashCode = llvh::hash_combine(hashCode, hash(*first));
  }
  return hashCode;
}

/// A helper for comparing bools, returning -1, 0, +1.
inline int cmpHelper(bool a, bool b) {
  return (int)a - (int)b;
}

/// A helper for comparing nullable Type pointers, returning -1, 0, +1.
int cmpHelper(Type *a, Type *b) {
  return a ? b ? a->compare(b) : 1 : b ? -1 : 0;
}

} // anonymous namespace

llvh::StringRef Type::getKindName() const {
  switch (kind_) {
    case TypeKind::Void:
      return "void";
    case TypeKind::Null:
      return "null";
    case TypeKind::Boolean:
      return "boolean";
    case TypeKind::String:
      return "string";
    case TypeKind::Number:
      return "number";
    case TypeKind::BigInt:
      return "bigint";
    case TypeKind::Any:
      return "any";
    case TypeKind::Mixed:
      return "mixed";
    case TypeKind::Union:
      return "union";
    case TypeKind::Function:
      return "function";
    case TypeKind::Class:
      return "class";
    case TypeKind::ClassConstructor:
      return "class constructor";
    case TypeKind::Array:
      return "array";
  }
  llvm_unreachable("invalid TypeKind");
}

int Type::compare(const Type *other) const {
  if (other == this)
    return 0;
  if (kind_ < other->kind_)
    return -1;
  else if (kind_ > other->kind_)
    return 1;

  switch (kind_) {
#define _HERMES_SEMA_FLOW_DEFKIND(name)                         \
  case TypeKind::name:                                          \
    return static_cast<const name##Type *>(this)->_compareImpl( \
        static_cast<const name##Type *>(other));
    _HERMES_SEMA_FLOW_SINGLETONS _HERMES_SEMA_FLOW_COMPLEX_TYPES
#undef _HERMES_SEMA_FLOW_DEFKIND
  }
  llvm_unreachable("invalid TypeKind");
}

unsigned Type::hash() const {
  if (LLVM_LIKELY(hashed_))
    return hashValue_;

  unsigned hv;
  switch (kind_) {
#define _HERMES_SEMA_FLOW_DEFKIND(name)                      \
  case TypeKind::name:                                       \
    hv = static_cast<const name##Type *>(this)->_hashImpl(); \
    break;

    _HERMES_SEMA_FLOW_SINGLETONS _HERMES_SEMA_FLOW_COMPLEX_TYPES

#undef _HERMES_SEMA_FLOW_DEFKIND
        ;
    default:
      llvm_unreachable("invalid TypeKind");
  }
  hashed_ = true;
  return hashValue_ = hv;
}

void UnionType::init(llvh::ArrayRef<Type *> types) {
  assert(types_.empty() && "Union already initialized");

  types_.reserve(types.size());

  auto addType = [this](Type *t) {
    assert(
        !llvh::isa<UnionType>(t) && "nested union should have been flattened");
    types_.push_back(t);
    if (t->getKind() == TypeKind::Any)
      hasAny_ = true;
    else if (t->getKind() == TypeKind::Mixed)
      hasMixed_ = true;
  };

  // Copy the union types to the types_, but flatten nested unions. Note that
  // there can't be more than one level.
  for (Type *elemType : types) {
    if (auto *unionType = llvh::dyn_cast<UnionType>(elemType)) {
      for (Type *nestedElem : unionType->types_) {
        addType(nestedElem);
      }
    } else {
      addType(elemType);
    }
  }

  // Sort for predictable order.
  std::sort(types_.begin(), types_.end(), [](Type *a, Type *b) {
    return a->compare(b) < 0;
  });

  // Remove identical union arms.
  types_.erase(
      std::unique(
          types_.begin(),
          types_.end(),
          [](Type *a, Type *b) { return a->equals(b); }),
      types_.end());
}

int UnionType::_compareImpl(const UnionType *other) const {
  return lexicographicalComparison(
      types_.begin(),
      types_.end(),
      other->types_.begin(),
      other->types_.end(),
      [](Type *a, Type *b) { return a->compare(b); });
}

unsigned UnionType::_hashImpl() const {
  return (unsigned)llvh::hash_combine(
      (unsigned)TypeKind::Union,
      hashTypes(
          types_.begin(), types_.end(), [](Type *t) { return t->hash(); }));
}

int ArrayType::_compareImpl(const ArrayType *other) const {
  return element_->compare(other->element_);
}

unsigned ArrayType::_hashImpl() const {
  return (unsigned)llvh::hash_combine(
      (unsigned)TypeKind::Array, element_->hash());
}

void FunctionType::init(
    Type *returnType,
    Type *thisParam,
    llvh::ArrayRef<Param> params,
    bool isAsync,
    bool isGenerator) {
  assert(!isInitialized() && "FunctionType already initialized");
  return_ = returnType;
  thisParam_ = thisParam;
  params_.append(params.begin(), params.end());
  isAsync_ = isAsync;
  isGenerator_ = isGenerator;
  markAsInitialized();
}

/// Compare two instances of the same TypeKind.
int FunctionType::_compareImpl(const FunctionType *other) const {
  if (auto tmp = cmpHelper(isAsync_, other->isAsync_))
    return tmp;
  if (auto tmp = cmpHelper(isGenerator_, other->isGenerator_))
    return tmp;
  if (auto tmp = cmpHelper(thisParam_, other->thisParam_))
    return tmp;
  if (auto tmp = lexicographicalComparison(
          params_.begin(),
          params_.end(),
          other->params_.begin(),
          other->params_.end(),
          [](const Param &pa, const Param &pb) {
            return pa.second->compare(pb.second);
          })) {
    return tmp;
  }
  return return_->compare(other->return_);
}

/// Calculate the type-specific hash.
unsigned FunctionType::_hashImpl() const {
  return (unsigned)llvh::hash_combine(
      (unsigned)TypeKind::Function,
      isAsync_,
      isGenerator_,
      thisParam_ ? thisParam_->hash() : 0,
      hashTypes(
          params_.begin(),
          params_.end(),
          [](const Param &p) { return p.second->hash(); }),
      return_->hash());
}

unsigned TypeWithId::_hashImpl() const {
  return (unsigned)llvh::hash_combine((unsigned)getKind(), getId());
}

void ClassType::init(
    llvh::ArrayRef<Field> fields,
    FunctionType *constructorType,
    ClassType *homeObjectType,
    ClassType *superClass) {
  fields_.reserve(fields.size());
  fields_.append(fields.begin(), fields.end());

  constructorType_ = constructorType;
  homeObjectType_ = homeObjectType;
  superClass_ = superClass;

  fieldNameMap_.reserve(fields.size());
  size_t index = 0;
  for (const auto &f : this->fields_) {
    assert(
        fieldNameMap_.count(f.name) == 0 && "Duplicate field name in a class");

    fieldNameMap_[f.name] = FieldLookupEntry{this, index++};
  }

  markAsInitialized();
}

hermes::OptValue<ClassType::FieldLookupEntry> ClassType::findField(
    Identifier id) const {
  auto it = fieldNameMap_.find(id);
  if (it == fieldNameMap_.end())
    return llvh::None;
  return it->second;
}

FlowContext::FlowContext() = default;
FlowContext::~FlowContext() = default;

Type *FlowContext::findDeclType(const sema::Decl *decl) const {
  assert(decl && "nullptr sema::Decl");
  auto it = declTypes_.find(decl);
  return it != declTypes_.end() ? it->second : nullptr;
}

Type *FlowContext::getDeclType(const sema::Decl *decl) const {
  Type *res = findDeclType(decl);
  assert(res && "unresolved declaration");
  return res;
}

void FlowContext::setNodeType(const ESTree::Node *node, Type *type) {
  if (type)
    nodeTypes_[node] = type;
}

Type *FlowContext::findNodeType(const ESTree::Node *node) const {
  auto it = nodeTypes_.find(node);
  return it != nodeTypes_.end() ? it->second : nullptr;
}

Type *FlowContext::getNodeTypeOrAny(const ESTree::Node *node) const {
  auto it = nodeTypes_.find(node);
  return it != nodeTypes_.end() ? it->second : getAny();
}

Type *FlowContext::getSingletonType(TypeKind kind) const {
  switch (kind) {
#define _HERMES_SEMA_FLOW_DEFKIND(name) \
  case TypeKind::name:                  \
    return &name##Instance_;
    _HERMES_SEMA_FLOW_SINGLETONS
#undef _HERMES_SEMA_FLOW_DEFKIND
    default:
      llvm_unreachable("invalid singleton TypeKind");
      return nullptr;
  }
}

UnionType *FlowContext::createPopulatedUnion(llvh::ArrayRef<Type *> types) {
  UnionType *res = createUnion();
  res->init(types);
  return res;
}

UnionType *FlowContext::createPopulatedNullable(Type *type) {
  return createPopulatedUnion({getVoid(), getNull(), type});
}

} // namespace flow
} // namespace hermes
