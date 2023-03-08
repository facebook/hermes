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

  flow::ClassType *classType = consType->getClassType();

  auto *classBody = ESTree::cast<ESTree::ClassBodyNode>(node->_body);

  Value *superClass = nullptr;
  if (node->_superClass) {
    superClass = genExpression(node->_superClass);
  }

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

    // Check that 'super()' call is the first statement in SH for derived
    // classes.
    // TODO: This is intentionally overly restrictive. In the future, we can
    // check that super() call happens prior to any function calls or 'this'
    // accesses.
    if (node->_superClass) {
      // Attempt to extract the super() call from the first statement of the
      // block.
      ESTree::CallExpressionNode *superCall =
          llvh::dyn_cast_or_null<ESTree::CallExpressionNode>(
              llvh::dyn_cast<ESTree::ExpressionStatementNode>(
                  &llvh::cast<ESTree::BlockStatementNode>(
                       llvh::cast<ESTree::FunctionExpressionNode>(
                           consMethod->_value)
                           ->_body)
                       ->_body.front())
                  ->_expression);
      if (!superCall || !llvh::isa<ESTree::SuperNode>(superCall->_callee)) {
        Mod->getContext().getSourceErrorManager().error(
            node->getSourceRange(),
            "first statement in derived class constructor must be super();");
      }
    }

    consFunction = genFunctionExpression(
        llvh::cast<ESTree::FunctionExpressionNode>(consMethod->_value),
        consName,
        node->_superClass);
  } else {
    // Create an empty constructor.
    if (superClass) {
      // TODO: An implicit constructor has to pass all arguments along to the
      // parent class.
      Mod->getContext().getSourceErrorManager().error(
          node->getStartLoc(),
          "inherited classes implicit constructor unsupported, "
          "add an explicit constructor");
    }
    Function *func;
    {
      IRBuilder::SaveRestore saveState{Builder};
      func = Builder.createFunction(
          consName,
          Function::DefinitionKind::ES5Function,
          true,
          CustomDirectives{
              .sourceVisibility = SourceVisibility::Default,
              .alwaysInline = false});
      Builder.setInsertionBlock(Builder.createBasicBlock(func));
      Builder.createReturnInst(Builder.getLiteralUndefined());
    }
    consFunction = Builder.createCreateFunctionInst(func);
  }
  emitStore(Builder, consFunction, getDeclData(decl), true);

  // Create and populate the "prototype" property (vtable).
  // Must be done even if there are no methods to enable 'instanceof'.
  auto *homeObject = emitClassAllocation(
      classType->getHomeObjectType(),
      superClass ? Builder.createLoadPropertyInst(
                       superClass, kw_.identPrototype->str())
                 : nullptr);

  // The 'prototype' property is initially set as non-configurable,
  // and we're overwriting it with our own.
  // So we can't use StoreOwnProperty here because that attempts to define a
  // configurable property.
  // TODO: Do this properly by using a new instruction for class creation.
  Builder.createStorePropertyStrictInst(
      homeObject,
      consFunction,
      Builder.getLiteralString(kw_.identPrototype->str()));
}

Value *ESTreeIRGen::emitClassAllocation(
    flow::ClassType *classType,
    Value *parent) {
  // TODO: should create a sealed object, etc.
  AllocObjectLiteralInst::ObjectPropertyMap propMap{};
  propMap.resize(classType->getFieldNameMap().size());

  // Generate code for each field, place it in the propMap.
  for (const auto it : classType->getFieldNameMap()) {
    flow::ClassType::FieldLookupEntry entry = it.second;
    const flow::ClassType::Field &field = *entry.getField();
    assert(
        propMap[field.layoutSlotIR].first == nullptr &&
        "every entry must be filled exactly once");

    if (field.isMethod()) {
      // Create the code for the method.
      if (entry.classType == classType) {
        Value *function = genFunctionExpression(
            llvh::cast<ESTree::FunctionExpressionNode>(field.method->_value),
            field.name);
        propMap[field.layoutSlotIR] = {
            Builder.getLiteralString(field.name), function};
      } else {
        assert(parent && "inherited field without parent ClassType");
        // Method is inherited. Read it from the parent.
        propMap[field.layoutSlotIR] = {
            Builder.getLiteralString(field.name),
            Builder.createPrLoadInst(
                parent,
                field.layoutSlotIR,
                Builder.getLiteralString(field.name),
                flowTypeToIRType(field.type))};
        continue;
      }
    } else {
      propMap[field.layoutSlotIR] = {
          Builder.getLiteralString(field.name),
          getDefaultInitValue(field.type)};
    }
  }

  // TODO: Have a specific instruction for allocating an object from a class
  // that sets the parent, uses the prop map, etc.
  Value *result;
  if (propMap.empty()) {
    result = Builder.createAllocObjectInst(0, parent);
  } else {
    result = Builder.createAllocObjectLiteralInst(propMap);
    if (parent) {
      // TODO: Ensure that parent is typed correctly as 'object'.
      Builder.createStoreParentInst(parent, result);
    }
  }
  return result;
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
