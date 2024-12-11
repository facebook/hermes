/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ESTreeIRGen.h"
#include "hermes/AST/ESTree.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Support/StringTable.h"
#include "llvh/ADT/ScopeExit.h"

namespace hermes {
namespace irgen {

Value *ESTreeIRGen::genLegacyClassExpression(
    ESTree::ClassExpressionNode *node,
    Identifier nameHint) {
  // This is the possibly empty scope containing the class expression name.
  emitScopeDeclarations(node->getScope());
  return genLegacyClassLike(node, nameHint);
}

void ESTreeIRGen::genLegacyClassDeclaration(
    ESTree::ClassDeclarationNode *node) {
  emitScopeDeclarations(node->getScope());
  auto *CCI = genLegacyClassLike(node, Identifier{});
  // Legacy class declarations have two variables that are created. One for
  // after the class body, and one that is only visible inside the class body.
  // Here, we should initialize the outer variable, which can be found by
  // looking at the declaration decl.
  sema::Decl *decl =
      semCtx_.getDeclarationDecl(cast<ESTree::IdentifierNode>(node->_id));
  assert(decl && "ClassDeclarationNode missing a DeclarationDecl.");
  emitStore(CCI, getDeclData(decl), true);
}

CreateClassInst *ESTreeIRGen::genLegacyClassLike(
    ESTree::ClassLikeNode *classNode,
    Identifier nameHint) {
  ESTree::IdentifierNode *id = getClassID(classNode);
  Identifier className = id ? Identifier::getFromPointer(id->_name)
      : nameHint.isValid()  ? nameHint
                            : Builder.createIdentifier("");
  auto *superClassNode = getSuperClass(classNode);
  auto *classBody = getClassBody(classNode);
  Value *superCls = superClassNode ? genExpression(superClassNode)
                                   : Builder.getEmptySentinel();

  auto *curScope = curFunction()->curScope;
  auto *curVarScope = curScope->getVariableScope();
  // Holds the .prototype of the class.
  Variable *clsPrototypeVar = Builder.createVariable(
      curScope->getVariableScope(),
      Builder.createIdentifier("?" + className.str() + ".prototype"),
      Type::createObject(),
      true);
  // Holds the class
  Variable *classVar = Builder.createVariable(
      curVarScope,
      Builder.createIdentifier("?" + className.str()),
      Type::createObject(),
      true);

  std::shared_ptr<LegacyClassContext> savedClsCtx =
      curFunction()->legacyClassContext;
  // Push a new class context.
  curFunction()->legacyClassContext =
      std::make_shared<LegacyClassContext>(classVar);
  // Pop it when we are done generating IR for this class node.
  auto popCtx = llvh::make_scope_exit(
      [this, savedClsCtx] { curFunction()->legacyClassContext = savedClsCtx; });

  // Function code for the constructor.
  NormalFunction *consCode;
  if (ESTree::getDecoration<ESTree::ClassLikeDecoration>(classNode)
          ->implicitCtorFunctionInfo) {
    consCode =
        genLegacyImplicitConstructor(classNode, className, superClassNode);
  } else {
    ESTree::MethodDefinitionNode *consMethodNode = nullptr;
    for (auto &classElement : classBody->_body) {
      if (auto *method =
              llvh::dyn_cast<ESTree::MethodDefinitionNode>(&classElement);
          method && method->_kind == kw_.identConstructor) {
        consMethodNode = method;
        break;
      }
    }
    assert(consMethodNode && "no explicit or implicit constructor found");
    consCode = genBasicFunction(
        className,
        llvh::cast<ESTree::FunctionExpressionNode>(consMethodNode->_value),
        curScope->getVariableScope(),
        superClassNode,
        Function::DefinitionKind::ES6Constructor,
        clsPrototypeVar);
  }
  AllocStackInst *clsPrototypeOutput = Builder.createAllocStackInst(
      genAnonymousLabelName("clsPrototype"), Type::createObject());
  CreateClassInst *createClass = Builder.createCreateClassInst(
      curScope, consCode, superCls, clsPrototypeOutput);
  auto *clsPrototype = Builder.createLoadStackInst(clsPrototypeOutput);

  /// Add a method to a given object \p O. In practice, O should either be the
  /// class itself or the class prototype.
  auto addMethod =
      [this](Value *O, llvh::StringRef kind, Value *key, Value *closure) {
        if (kind == "get") {
          Builder.createStoreGetterSetterInst(
              closure,
              Builder.getLiteralUndefined(),
              O,
              key,
              IRBuilder::PropEnumerable::No);
        } else if (kind == "set") {
          Builder.createStoreGetterSetterInst(
              Builder.getLiteralUndefined(),
              closure,
              O,
              key,
              IRBuilder::PropEnumerable::No);
        } else {
          assert(kind == "method" && "unhandled method definition");
          Builder.createStoreOwnPropertyInst(
              closure, O, key, IRBuilder::PropEnumerable::No);
        }
      };
  // Space used to convert property keys to strings.
  llvh::SmallVector<char, 32> buffer;
  for (auto &classElement : classBody->_body) {
    if (auto *method =
            llvh::dyn_cast<ESTree::MethodDefinitionNode>(&classElement)) {
      if (method->_kind == kw_.identConstructor) {
        continue;
      }
      Value *key;
      Identifier nameHint{};
      if (method->_computed) {
        key = Builder.createToPropertyKeyInst(genExpression(method->_key));
      } else {
        nameHint = Mod->getContext().getIdentifier(
            propertyKeyAsString(buffer, method->_key));
        key = Builder.getLiteralString(nameHint);
      }
      auto isStatic = method->_static;
      auto *homeObject = isStatic ? classVar : clsPrototypeVar;
      auto *funcValue = genFunctionExpression(
          llvh::cast<ESTree::FunctionExpressionNode>(method->_value),
          nameHint,
          superClassNode,
          Function::DefinitionKind::ES6Method,
          homeObject);
      if (isStatic) {
        addMethod(createClass, method->_kind->str(), key, funcValue);
      } else {
        addMethod(clsPrototype, method->_kind->str(), key, funcValue);
      }
    }
  }

  // Make the class name binding available after we've generated all computed
  // keys.
  if (id)
    emitStore(createClass, resolveIdentifier(id), true);

  // Initialize the class context variables.
  Builder.createStoreFrameInst(curScope, createClass, classVar);
  Builder.createStoreFrameInst(curScope, clsPrototype, clsPrototypeVar);

  return createClass;
}

Value *ESTreeIRGen::genLegacyDerivedThis() {
  assert(
      curFunction()->hasLegacyClassContext() &&
      "no legacy class context found.");
  assert(
      (semCtx_.nearestNonArrow(curFunction()->getSemInfo())->constructorKind ==
       sema::FunctionInfo::ConstructorKind::Derived) &&
      curFunction()->capturedState.thisVal &&
      "captured state not properly set.");
  auto *checkedThis = curFunction()->capturedState.thisVal;
  auto *baseConstructorScope =
      emitResolveScopeInstIfNeeded(checkedThis->getParent());
  return Builder.createThrowIfInst(
      Builder.createLoadFrameInst(baseConstructorScope, checkedThis),
      Type::createEmpty());
}

Value *ESTreeIRGen::genLegacyDirectSuper(ESTree::CallExpressionNode *call) {
  assert(
      curFunction()->hasLegacyClassContext() &&
      "no legacy class context found.");
  auto &LC = curFunction()->legacyClassContext;
  assert(LC->constructor && "class context not properly set.");
  assert(
      (semCtx_.nearestNonArrow(curFunction()->getSemInfo())->constructorKind ==
       sema::FunctionInfo::ConstructorKind::Derived) &&
      curFunction()->capturedState.thisVal &&
      "captured state not properly set.");
  assert(
      llvh::isa<ESTree::SuperNode>(getCallee(call)) &&
      "super is not being invoked");
  auto *newTarget = genNewTarget();
  auto *constructorScope =
      emitResolveScopeInstIfNeeded(LC->constructor->getParent());
  // We want to invoke the parent of the class we are generating.
  auto *callee = Builder.createLoadParentNoTrapsInst(
      Builder.createLoadFrameInst(constructorScope, LC->constructor));
  // Derived classes don't take in a `this`, so construct it here.
  auto *thisParam = Builder.createCreateThisInst(callee, newTarget);
  auto *superRes = emitCall(
      call, callee, Builder.getEmptySentinel(), false, thisParam, newTarget);
  auto *checkedThis = curFunction()->capturedState.thisVal;
  auto *checkedThisScope =
      emitResolveScopeInstIfNeeded(checkedThis->getParent());
  Builder.createThrowIfThisInitializedInst(
      Builder.createLoadFrameInst(checkedThisScope, checkedThis));
  Value *initializedThisVal = nullptr;
  if (auto *CI = llvh::dyn_cast<CallInst>(superRes)) {
    // Correctly pick between the provided `this` and the return value of the
    // super call.
    initializedThisVal = Builder.createGetConstructedObjectInst(thisParam, CI);
  } else {
    // If it's not a simple call instruction, then the logic of
    // GetConstructedObject is already being replicated in the way the call was
    // emitted.
    initializedThisVal = superRes;
  }
  // A construct call always returns object.
  initializedThisVal->setType(Type::createObject());
  Builder.createStoreFrameInst(
      checkedThisScope, initializedThisVal, checkedThis);
  return initializedThisVal;
}

Value *ESTreeIRGen::genLegacyDerivedConstructorRet(
    ESTree::ReturnStatementNode *node,
    Value *returnValue) {
  // Easy optimization- if we are returning `this`, then we already do the
  // required checks when generating the IR for the `this` expression.
  if (node && node->_argument &&
      llvh::isa<ESTree::ThisExpressionNode>(node->_argument)) {
    return returnValue;
  }
  assert(
      (curFunction()->getSemInfo()->constructorKind ==
       sema::FunctionInfo::ConstructorKind::Derived) &&
      curFunction()->capturedState.thisVal &&
      "captured state not properly set.");
  auto *checkedThis = curFunction()->capturedState.thisVal;
  auto *baseConstructorScope =
      emitResolveScopeInstIfNeeded(checkedThis->getParent());

  // Easy optimization- we will often be returning a literal undefined because
  // of implicit returns. Returning undefined is functionally equivalent to
  // returning `this`.
  if (llvh::isa<LiteralUndefined>(returnValue)) {
    return Builder.createThrowIfInst(
        Builder.createLoadFrameInst(baseConstructorScope, checkedThis),
        Type::createEmpty());
  }

  // Every return statement in a derived class constructor must be either an
  // object or undefined. If it's undefined, then convert that to the checked
  // this. Conceptually then, we generate IR like the following:
  //
  //   if (returnValue is object || returnValue is function) {
  //     return returnValue;
  //   } else if (returnValue === undefined) {
  //     return checkedThis();
  //   } else {
  //     throw new TypeError();
  //   }

  auto *curBB = Builder.getInsertionBlock();
  auto *returnBB = Builder.createBasicBlock(curFunction()->function);
  auto *checkIfUndefBB = Builder.createBasicBlock(curFunction()->function);
  auto *throwInvalidRetBB = Builder.createBasicBlock(curFunction()->function);
  auto *loadCheckedThisBB = Builder.createBasicBlock(curFunction()->function);
  auto *canUseRetVal = Builder.createTypeOfIsInst(
      returnValue,
      Builder.getLiteralTypeOfIsTypes(
          TypeOfIsTypes{}.withFunction(true).withObject(true)));

  // 10.a. If result.[[Value]] is an Object, return result.[[Value]].
  Builder.createCondBranchInst(canUseRetVal, returnBB, checkIfUndefBB);

  // 10.c. If result.[[Value]] is not undefined, throw a TypeError exception.
  Builder.setInsertionBlock(checkIfUndefBB);
  Builder.createCondBranchInst(
      Builder.createBinaryOperatorInst(
          returnValue,
          Builder.getLiteralUndefined(),
          ValueKind::BinaryStrictlyEqualInstKind),
      loadCheckedThisBB,
      throwInvalidRetBB);

  // 12. Let thisBinding be ? constructorEnv.GetThisBinding().
  // 13. Assert: thisBinding is an Object.
  // 14. Return thisBinding.
  Builder.setInsertionBlock(loadCheckedThisBB);
  auto *checkedThisVal = Builder.createThrowIfInst(
      Builder.createLoadFrameInst(baseConstructorScope, checkedThis),
      Type::createEmpty());
  Builder.createBranchInst(returnBB);

  Builder.setInsertionBlock(throwInvalidRetBB);
  Builder.createThrowTypeErrorInst(Builder.getLiteralString(
      "Derived constructors may only return an object or undefined."));

  Builder.setInsertionBlock(returnBB);
  return Builder.createPhiInst(
      {returnValue, checkedThisVal}, {curBB, loadCheckedThisBB});
}

NormalFunction *ESTreeIRGen::genLegacyImplicitConstructor(
    ESTree::ClassLikeNode *classNode,
    const Identifier &className,
    ESTree::Node *superClassNode) {
  const auto &LC = curFunction()->legacyClassContext;
  assert(LC->constructor && "class context not properly set");

  // Retrieve the FunctionInfo for the implicit constructor, which must exist.
  sema::FunctionInfo *funcInfo =
      ESTree::getDecoration<ESTree::ClassLikeDecoration>(classNode)
          ->implicitCtorFunctionInfo;
  assert(
      funcInfo &&
      "Semantic resolver failed to decorate class with implicit ctor");

  auto *consFunc = Builder.createFunction(
      className,
      Function::DefinitionKind::ES6Constructor,
      /* strictMode */ true,
      funcInfo->customDirectives);

  auto compileFunc = [this,
                      consFunc,
                      funcInfo,
                      LC,
                      superClassNode,
                      isDerived = funcInfo->constructorKind ==
                          sema::FunctionInfo::ConstructorKind::Derived,
                      parentScope =
                          curFunction()->curScope->getVariableScope()] {
    FunctionContext newFunctionContext{this, consFunc, funcInfo};
    newFunctionContext.superClassNode_ = superClassNode;
    newFunctionContext.legacyClassContext = LC;

    auto *entryBB = Builder.createBasicBlock(consFunc);
    emitFunctionPrologue(
        nullptr,
        entryBB,
        InitES5CaptureState::No,
        DoEmitDeclarations::No,
        parentScope);
    // Default subclass constructors must call super.
    if (isDerived) {
      // All derived constructors must have a captured state `this`.
      curFunction()->capturedState.thisVal = Builder.createVariable(
          curFunction()->curScope->getVariableScope(),
          Builder.createIdentifier("?CHECKED_this"),
          Type::createObject(),
          true);
      auto *baseConstructorScope =
          emitResolveScopeInstIfNeeded(LC->constructor->getParent());
      auto *callee = Builder.createLoadParentNoTrapsInst(
          Builder.createLoadFrameInst(baseConstructorScope, LC->constructor));
      auto newTarget = Builder.createGetNewTargetInst(
          curFunction()->function->getNewTargetParam());
      auto *createdThis = Builder.createCreateThisInst(callee, newTarget);
      // Emit IR equivalent to super(...arguments)
      auto *initializedThisVal = genBuiltinCall(
          BuiltinMethod::HermesBuiltin_applyArguments,
          {callee, createdThis, newTarget});
      // Construct call always returns object.
      initializedThisVal->setType(Type::createObject());
      Builder.createStoreFrameInst(
          curFunction()->curScope,
          initializedThisVal,
          newFunctionContext.capturedState.thisVal);
      // Returning undefined in a derived constructor is coerced into returning
      // the `thisVal` we just set.
      emitFunctionEpilogue(Builder.getLiteralUndefined());
      consFunc->setReturnType(Type::createObject());
    } else {
      emitFunctionEpilogue(Builder.getLiteralUndefined());
      consFunc->setReturnType(Type::createUndefined());
    }
  };
  enqueueCompilation(
      classNode, ExtraKey::ImplicitClassConstructor, consFunc, compileFunc);
  return consFunc;
}

} // namespace irgen
} // namespace hermes
