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
  if (decl->generic) {
    // Skip generics that aren't specialized.
    return;
  }

  flow::Type *declType = flowContext_.findDeclType(decl);
  flow::ClassConstructorType *consType =
      llvh::dyn_cast_or_null<flow::ClassConstructorType>(
          declType ? declType->info : nullptr);

  // If the class is not annotated with a type, it is legacy, and we don't
  // support that yet.
  if (!consType) {
    Mod->getContext().getSourceErrorManager().error(
        node->getStartLoc(), "Legacy JS classes not supported (yet)");
    return;
  }

  flow::ClassType *classType = consType->getClassTypeInfo();

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
      ESTree::NodeList &blockStmtBody =
          llvh::cast<ESTree::BlockStatementNode>(
              llvh::cast<ESTree::FunctionExpressionNode>(consMethod->_value)
                  ->_body)
              ->_body;
      ESTree::Node *firstStatement = nullptr;
      for (ESTree::Node &it : blockStmtBody) {
        firstStatement = &it;
        auto *exprSt = llvh::dyn_cast<ESTree::ExpressionStatementNode>(&it);
        // Skip directives.
        if (!exprSt || !exprSt->_directive) {
          break;
        }
      }
      ESTree::ExpressionStatementNode *exprStatement =
          llvh::dyn_cast_or_null<ESTree::ExpressionStatementNode>(
              firstStatement);
      ESTree::CallExpressionNode *superCall = exprStatement
          ? llvh::dyn_cast<ESTree::CallExpressionNode>(
                exprStatement->_expression)
          : nullptr;
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

    // Use the compiledEntities_ cache even though we're not enqueuing a
    // function compilation (because the function is trivial).
    // This way we avoid making multiple implicit constructors for the same
    // classType, allowing us to populate the target operand of CallInsts.
    if (Value *found =
            findCompiledEntity(node, ExtraKey::ImplicitClassConstructor)) {
      func = llvh::cast<Function>(found);
    } else {
      IRBuilder::SaveRestore saveState{Builder};
      func = Builder.createFunction(
          consName,
          Function::DefinitionKind::ES5Function,
          true,
          CustomDirectives{
              .sourceVisibility = SourceVisibility::Default,
              .alwaysInline = true});
      func->addJSThisParam();
      func->setExpectedParamCountIncludingThis(1);
      Builder.setInsertionBlock(Builder.createBasicBlock(func));
      Builder.createReturnInst(Builder.getLiteralUndefined());
      CompiledMapKey key(node, (unsigned)ExtraKey::ImplicitClassConstructor);
      compiledEntities_[key] = func;
    }

    consFunction =
        Builder.createCreateFunctionInst(curFunction()->functionScope, func);
  }
  emitStore(consFunction, getDeclData(decl), true);

  // Create and populate the "prototype" property (vtable).
  // Must be done even if there are no methods to enable 'instanceof'.
  Value *vtable = nullptr;
  if (superClass) {
    auto it = classConstructors_.find(classType->getSuperClassInfo());
    assert(it != classConstructors_.end() && "missing super class constructor");
    auto *RSI = emitResolveScopeInstIfNeeded(
        it->second.homeObjectVar->getParent(), curFunction()->functionScope);
    vtable = Builder.createLoadFrameInst(RSI, it->second.homeObjectVar);
    // TODO: This will be known to be the actual type when we properly use an
    // instruction for class creation, but for now we need an object here
    // because we want to use PrLoad on it.
    vtable->setType(Type::createObject());
  }
  auto *homeObject =
      emitClassAllocation(classType->getHomeObjectTypeInfo(), vtable);

  // Store the home object in a variable so that we can reference it later,
  // e.g. when we emit method calls.
  Variable *homeObjectVar = Builder.createVariable(
      curFunction()->function->getFunctionScope(),
      Builder.createIdentifier(
          llvh::Twine("?") + classType->getClassName().str() + ".prototype"),
      flowTypeToIRType(classType->getHomeObjectType()));
  Builder.createStoreFrameInst(
      curFunction()->functionScope, homeObject, homeObjectVar);

  // Check to make sure this is a valid class definition,
  // because there may have been errors.
  if (auto *createCallable =
          llvh::dyn_cast<BaseCreateCallableInst>(consFunction)) {
    auto [it, inserted] = classConstructors_.try_emplace(
        classType, createCallable->getFunctionCode(), homeObjectVar);
    (void)it;
    assert(
        it->second.constructorFunc == createCallable->getFunctionCode() &&
        "redefinition of constructor function");
  }

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
        if (auto *CFI = llvh::dyn_cast<CreateFunctionInst>(function)) {
          // If this field represents a final method, record the IR function so
          // we can use it to populate the target of calls.
          if (!field.overridden) {
            auto [_, success] =
                finalMethods_.try_emplace(&field, CFI->getFunctionCode());
            (void)success;
            assert(success && "Method already emitted");
          }
        }
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
  switch (type->info->getKind()) {
    case flow::TypeKind::Void:
      return Builder.getLiteralUndefined();
    case flow::TypeKind::Null:
      return Builder.getLiteralNull();
    case flow::TypeKind::Boolean:
      return Builder.getLiteralBool(false);
    case flow::TypeKind::String:
      return Builder.getLiteralString("");
    case flow::TypeKind::CPtr:
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
          llvh::cast<flow::UnionType>(type->info)->getTypes()[0]);
    case flow::TypeKind::TypedFunction:
    case flow::TypeKind::NativeFunction:
    case flow::TypeKind::UntypedFunction:
    case flow::TypeKind::Class:
    case flow::TypeKind::ClassConstructor:
    case flow::TypeKind::Array:
    case flow::TypeKind::Tuple:
      return Builder.getLiteralPositiveZero();
    case flow::TypeKind::Generic:
      hermes_fatal("invalid typekind");
  }
}

Type ESTreeIRGen::flowTypeToIRType(flow::Type *flowType) {
  switch (flowType->info->getKind()) {
    case flow::TypeKind::Void:
      return Type::createUndefined();
    case flow::TypeKind::Null:
      return Type::createNull();
    case flow::TypeKind::Boolean:
      return Type::createBoolean();
    case flow::TypeKind::String:
      return Type::createString();
    case flow::TypeKind::CPtr:
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
           llvh::cast<flow::UnionType>(flowType->info)->getTypes()) {
        res = Type::unionTy(res, flowTypeToIRType(elemType));
      }
      return res;
    }
    case flow::TypeKind::NativeFunction:
      return Type::createNumber();
    case flow::TypeKind::TypedFunction:
    case flow::TypeKind::UntypedFunction:
      return Type::createObject();
    case flow::TypeKind::Class:
      return Type::createObject();
    case flow::TypeKind::ClassConstructor:
      return Type::createObject();
    case flow::TypeKind::Array:
    case flow::TypeKind::Tuple:
      return Type::createObject();
    case flow::TypeKind::Generic:
      hermes_fatal("invalid typekind");
  }
}

} // namespace irgen
} // namespace hermes
