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
    if (a->getKind() < b->getKind())
      return true;
    if (a->getKind() > b->getKind())
      return false;
    // Both have the same kind. If they have no ids, they must be equal.
    if (llvh::isa<TypeWithId>(a)) {
      assert(
          a == b &&
          "Can't have two different instances of the same kind without id");
      return false;
    }

    // Two types of the same kind, with ids. Just compare the ids.
    return llvh::cast<TypeWithId>(a)->getId() <
        llvh::cast<TypeWithId>(b)->getId();
  });

  // Remove identical union arms.
  types_.erase(std::unique(types_.begin(), types_.end()), types_.end());
}

bool UnionType::hasType(Type *t) const {
  for (auto *elemType : types_) {
    if (t->getKind() > elemType->getKind())
      break;
    if (t == elemType)
      return true;
  }
  return false;
}

bool UnionType::isUnionEqual(const UnionType *o) const {
  return o == this ||
      std::equal(types_.begin(), types_.end(), o->types_.begin());
}

size_t UnionType::calcUnionHash() const {
  llvh::hash_code hashCode((size_t)TypeKind::Union);
  for (Type *t : types_) {
    assert(!llvh::isa<UnionType>(t) && "nested union types are invalid");
    if (auto *tid = llvh::dyn_cast<TypeWithId>(t))
      hashCode = llvh::hash_combine(hashCode, tid->getId());
    else
      hashCode = llvh::hash_combine(hashCode, (TypeKind)t->getKind());
  }

  return hashCode;
}

size_t TypeWithId::calcTypeWithIdHash() const {
  return llvh::hash_combine((unsigned)getKind(), getId());
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

void ClassType::init(
    llvh::ArrayRef<Field> fields,
    FunctionType *constructorType) {
  fields_.reserve(fields.size());
  fields_.append(fields.begin(), fields.end());
  constructorType_ = constructorType;

  fieldNameMap_.reserve(fields.size());
  size_t index = 0;
  for (const auto &f : this->fields_) {
    assert(
        fieldNameMap_.count(f.name) == 0 && "Duplicate field name in a class");

    fieldNameMap_[f.name] = index++;
  }
  markAsInitialized();
}

size_t ClassType::getFieldIndex(Identifier id) const {
  auto it = fieldNameMap_.find(id);
  assert(it != fieldNameMap_.end() && "field must exist");
  return it->second;
}

const ClassType::Field *ClassType::findField(Identifier id) const {
  auto it = fieldNameMap_.find(id);
  return it != fieldNameMap_.end() ? &fields_[it->second] : nullptr;
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
