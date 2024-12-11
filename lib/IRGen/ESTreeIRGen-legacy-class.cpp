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

  ESTree::MethodDefinitionNode *consMethodNode = nullptr;
  for (auto &classElement : classBody->_body) {
    if (auto *method =
            llvh::dyn_cast<ESTree::MethodDefinitionNode>(&classElement);
        method && method->_kind == kw_.identConstructor) {
      consMethodNode = method;
    }
  }
  if (!consMethodNode) {
    assert(false && "Missing constructor");
    Mod->getContext().getSourceErrorManager().error(
        classNode->getSourceRange(), "Error: missing constructor");
  }

  // Prepare arguments for and emit CreateClassInst.
  NormalFunction *consFunction = genBasicFunction(
      className,
      llvh::cast<ESTree::FunctionExpressionNode>(consMethodNode->_value),
      curScope->getVariableScope(),
      superClassNode,
      Function::DefinitionKind::ES6Constructor);
  AllocStackInst *clsPrototypeOutput = Builder.createAllocStackInst(
      genAnonymousLabelName("clsPrototype"), Type::createObject());
  CreateClassInst *createClass = Builder.createCreateClassInst(
      curScope, consFunction, superCls, clsPrototypeOutput);
  auto *clsPrototype = Builder.createLoadStackInst(clsPrototypeOutput);
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
  Value *res = emitCall(
      call, callee, Builder.getEmptySentinel(), false, thisParam, newTarget);
  auto *checkedThis = curFunction()->capturedState.thisVal;
  auto *checkedThisScope =
      emitResolveScopeInstIfNeeded(checkedThis->getParent());
  Builder.createThrowIfThisInitializedInst(
      Builder.createLoadFrameInst(checkedThisScope, checkedThis));
  // Correctly pick between the provided `this` and the return value of the
  // super call.
  auto *initializedThisVal = Builder.createGetConstructedObjectInst(
      thisParam, llvh::cast<CallInst>(res));
  Builder.createStoreFrameInst(
      checkedThisScope, initializedThisVal, checkedThis);
  return initializedThisVal;
}

} // namespace irgen
} // namespace hermes
