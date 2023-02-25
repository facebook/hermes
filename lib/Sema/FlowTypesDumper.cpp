/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FlowTypesDumper.h"

namespace hermes {
namespace flow {

size_t FlowTypesDumper::getNumber(const Type *type) {
  if (type->isSingleton())
    return 0;

  auto [it, inserted] = typeNumber_.try_emplace(type, types_.size());
  if (inserted)
    types_.push_back(type);
  return it->second + 1;
}

void FlowTypesDumper::printTypeRef(llvh::raw_ostream &os, const Type *type) {
  os << type->getKindName();
  if (!type->isSingleton())
    os << " %t." << getNumber(type);
}

void FlowTypesDumper::printTypeDescription(
    llvh::raw_ostream &os,
    const Type *type) {
  os << type->getKindName();
  if (type->isSingleton()) {
    os << '\n';
    return;
  }

  os << ' ';

  switch (type->getKind()) {
    case TypeKind::Void:
    case TypeKind::Null:
    case TypeKind::Boolean:
    case TypeKind::String:
    case TypeKind::Number:
    case TypeKind::BigInt:
    case TypeKind::Any:
    case TypeKind::Mixed:
      llvm_unreachable("singletons already handled");
      break;

    case TypeKind::Union: {
      bool first = true;
      for (Type *t : llvh::cast<UnionType>(type)->getTypes()) {
        if (!first)
          os << " | ";
        first = false;
        printTypeRef(os, t);
      }
    } break;

    case TypeKind::Function: {
      auto *funcType = llvh::cast<FunctionType>(type);
      if (funcType->isAsync())
        os << "async ";
      if (funcType->isGenerator())
        os << "generator ";
      os << '(';

      bool first = true;
      if (Type *thisType = funcType->getThisParam()) {
        os << "this: ";
        printTypeRef(os, thisType);
        first = false;
      }
      for (const FunctionType::Param &param : funcType->getParams()) {
        if (!first)
          os << ", ";
        first = false;
        if (param.first.isValid())
          os << param.first;
        os << ": ";
        printTypeRef(os, param.second);
      }
      os << "): ";
      printTypeRef(os, funcType->getReturnType());
    } break;

    case TypeKind::Class: {
      auto *classType = llvh::cast<ClassType>(type);
      os << classType->getClassName() << " {\n";
      if (auto *constrType = classType->getConstructorType()) {
        os << "  %constructor: ";
        printTypeRef(os, constrType);
        os << '\n';
      }
      for (const ClassType::Field &field : classType->getFields()) {
        os << "  " << field.name << ": ";
        printTypeRef(os, field.type);
        os << '\n';
      }
      os << '}';
    } break;

    case TypeKind::ClassConstructor:
      printTypeRef(os, llvh::cast<ClassConstructorType>(type)->getClassType());
      break;

    case TypeKind::Array:
      printTypeRef(os, llvh::cast<ArrayType>(type)->getElement());
      break;
  }

  os << '\n';
}

void FlowTypesDumper::printAllNumberedTypes(llvh::raw_ostream &os) {
  for (size_t i = 0; i != types_.size(); ++i) {
    os << "%t." << (i + 1) << " = ";
    printTypeDescription(os, types_[i]);
  }
}

void FlowTypesDumper::printAllTypes(
    llvh::raw_ostream &os,
    const FlowContext &flowTypes) {
  // The last type number printed.
  size_t lastNumber = 0;
  auto printAll = [&os, this, &lastNumber](const auto &all) {
    for (const auto &t : all) {
      // Don't print duplicate types.
      size_t number = getNumber(&t);
      if (number <= lastNumber)
        continue;
      lastNumber = number;

      printTypeRef(os, &t);
      os << " = ";
      printTypeDescription(os, &t);
    }
  };

#define _HERMES_SEMA_FLOW_DEFKIND(name) printAll(flowTypes.alloc##name##_);
  _HERMES_SEMA_FLOW_COMPLEX_TYPES
#undef _HERMES_SEMA_FLOW_DEFKIND
}

} // namespace flow
} // namespace hermes
