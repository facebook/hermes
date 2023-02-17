/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ESTreeIRGen.h"

namespace hermes {
namespace irgen {

void ESTreeIRGen::genClassDeclaration(ESTree::ClassDeclarationNode *node) {
  auto *id = llvh::cast<ESTree::IdentifierNode>(node->_id);
  sema::Decl *decl = getIDDecl(id);
  flow::ClassConstructorType *consType =
      llvh::dyn_cast_or_null<flow::ClassConstructorType>(
          flowContext_.findDeclType(decl));

  // If the class is not annotated with a type, it is legacy, and we don't
  // support that yet.
  if (!consType) {
    Mod->getContext().getSourceErrorManager().error(
        node->getStartLoc(), "Legacy JS classes not supported (yet)");
    return;
  }

  assert(
      !node->_superClass &&
      "super class should have been rejected by the type checker");

  flow::ClassType *classType = consType->getClassType();

  auto *classBody = ESTree::cast<ESTree::ClassBodyNode>(node->_body);

  // Emit the explicit constructor, if present.
  Value *consFunction;
  Identifier consName = classType->getClassName().isValid()
      ? classType->getClassName()
      : Identifier();
  if (classType->getConstructorType()) {
    // Find the constructor method.
    auto it = std::find_if(
        classBody->_body.begin(),
        classBody->_body.end(),
        [this](const ESTree::Node &n) {
          if (auto *method = llvh::dyn_cast<ESTree::MethodDefinitionNode>(&n))
            if (method->_kind == kw_.identConstructor)
              return true;
          return false;
        });
    assert(it != classBody->_body.end() && "constructor must exist");

    auto *consMethod = llvh::cast<ESTree::MethodDefinitionNode>(&*it);

    consFunction = genFunctionExpression(
        llvh::cast<ESTree::FunctionExpressionNode>(consMethod->_value),
        consName);

  } else {
    // Create an empty constructor.
    Function *func;
    {
      IRBuilder::SaveRestore saveState{Builder};
      func = Builder.createFunction(
          consName, Function::DefinitionKind::ES5Function, true);
      Builder.setInsertionBlock(Builder.createBasicBlock(func));
      Builder.createReturnInst(Builder.getLiteralUndefined());
    }
    consFunction = Builder.createCreateFunctionInst(func);
  }
  emitStore(Builder, consFunction, getDeclData(decl), true);
}

Value *ESTreeIRGen::emitClassAllocation(flow::ClassType *classType) {
  // TODO: should create a sealed object, etc.
  AllocObjectLiteralInst::ObjectPropertyMap propMap{};

  for (const flow::ClassType::Field &field : classType->getFields()) {
    propMap.emplace_back(
        Builder.getLiteralString(field.name), getDefaultInitValue(field.type));
  }

  return propMap.empty()
      ? (Value *)Builder.createAllocObjectInst(0)
      : (Value *)Builder.createAllocObjectLiteralInst(propMap);
}

Value *ESTreeIRGen::getDefaultInitValue(flow::Type *type) {
  switch (type->getKind()) {
    case flow::TypeKind::Void:
      return Builder.getLiteralUndefined();
    case flow::TypeKind::Null:
      return Builder.getLiteralNull();
    case flow::TypeKind::Boolean:
      return Builder.getLiteralBool(false);
    case flow::TypeKind::String:
      return Builder.getLiteralString("");
    case flow::TypeKind::Number:
      return Builder.getLiteralPositiveZero();
    case flow::TypeKind::BigInt:
      return Builder.getLiteralBigInt(
          Mod->getContext().getIdentifier("0").getUnderlyingPointer());
    case flow::TypeKind::Any:
    case flow::TypeKind::Mixed:
      return Builder.getLiteralUndefined();
    case flow::TypeKind::Union:
      return getDefaultInitValue(
          llvh::cast<flow::UnionType>(type)->getTypes()[0]);
    case flow::TypeKind::Function:
    case flow::TypeKind::Class:
    case flow::TypeKind::ClassConstructor:
    case flow::TypeKind::Array:
      return Builder.getLiteralPositiveZero();
  }
}

Type ESTreeIRGen::flowTypeToIRType(flow::Type *flowType) {
  switch (flowType->getKind()) {
    case flow::TypeKind::Void:
      return Type::createUndefined();
    case flow::TypeKind::Null:
      return Type::createNull();
    case flow::TypeKind::Boolean:
      return Type::createBoolean();
    case flow::TypeKind::String:
      return Type::createString();
    case flow::TypeKind::Number:
      return Type::createNumber();
    case flow::TypeKind::BigInt:
      return Type::createBigInt();
    case flow::TypeKind::Any:
    case flow::TypeKind::Mixed:
      return Type::createAnyType();
    case flow::TypeKind::Union: {
      Type res = Type::createNoType();
      for (flow::Type *elemType :
           llvh::cast<flow::UnionType>(flowType)->getTypes()) {
        res = Type::unionTy(res, flowTypeToIRType(elemType));
      }
      return res;
    }
    case flow::TypeKind::Function:
      return Type::createClosure();
    case flow::TypeKind::Class:
      return Type::createObject();
    case flow::TypeKind::ClassConstructor:
      return Type::createClosure();
    case flow::TypeKind::Array:
      return Type::createObject();
  }
}

} // namespace irgen
} // namespace hermes
