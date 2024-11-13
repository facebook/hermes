/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ESTreeIRGen.h"

#include "hermes/AST/RecursiveVisitor.h"

namespace hermes {
namespace irgen {

void ESTreeIRGen::genBody(ESTree::NodeList &Body) {
  LLVM_DEBUG(llvh::dbgs() << "Compiling body.\n");

  // Generate code for the declarations statements.
  for (auto &Node : Body) {
    LLVM_DEBUG(
        llvh::dbgs() << "IRGen node of type " << Node.getNodeName() << ".\n");
    genStatement(&Node);
  }
}

void ESTreeIRGen::genStatement(ESTree::Node *stmt) {
  LLVM_DEBUG(
      llvh::dbgs() << "IRGen statement of type " << stmt->getNodeName()
                   << "\n");
  IRBuilder::ScopedLocationChange slc(Builder, stmt->getDebugLoc());

  Builder.getFunction()->incrementStatementCount();

  if (/* auto *FD = */ llvh::dyn_cast<ESTree::FunctionDeclarationNode>(stmt)) {
    // It has already been hoisted. Do nothing.  But, keep this to
    // match the AST structure, and we may want to do something in the
    // future.
    return;
  }

  if (/* auto *IS = */ llvh::dyn_cast<ESTree::ImportDeclarationNode>(stmt)) {
    // Import has already been hoisted. Do nothing. But, keep this to
    // match the AST structure, and we may want to do something in the
    // future.
    return;
  }

  if (auto *IF = llvh::dyn_cast<ESTree::IfStatementNode>(stmt)) {
    return genIfStatement(IF);
  }

  if (auto *FIS = llvh::dyn_cast<ESTree::ForInStatementNode>(stmt)) {
    return genForInStatement(FIS);
  }

  if (auto *FOS = llvh::dyn_cast<ESTree::ForOfStatementNode>(stmt)) {
    return genForOfStatement(FOS);
  }

  if (auto *Ret = llvh::dyn_cast<ESTree::ReturnStatementNode>(stmt)) {
    return genReturnStatement(Ret);
  }

  if (auto *exprStmt = llvh::dyn_cast<ESTree::ExpressionStatementNode>(stmt)) {
    return genExpressionWrapper(exprStmt->_expression);
  }

  if (auto *SW = llvh::dyn_cast<ESTree::SwitchStatementNode>(stmt)) {
    return genSwitchStatement(SW);
  }

  if (auto *VDN = llvh::dyn_cast<ESTree::VariableDeclarationNode>(stmt)) {
    return genVariableDeclaration(VDN);
  }

  // IRGen the content of the block.
  if (auto *BS = llvh::dyn_cast<ESTree::BlockStatementNode>(stmt)) {
    emitScopeDeclarations(BS->getScope());
    for (auto &Node : BS->_body) {
      genStatement(&Node);
    }

    return;
  }

  if (auto *Label = llvh::dyn_cast<ESTree::LabeledStatementNode>(stmt)) {
    // Create a new basic block which is the continuation of the current block
    // and the jump target of the label.
    BasicBlock *next = Builder.createBasicBlock(curFunction()->function);

    // Set the jump point for the label to the new block.
    curFunction()->initLabel(Label, next, nullptr);

    // Now, generate the IR for the statement that the label is annotating.
    genStatement(Label->_body);

    // End the current basic block with a jump to the new basic block.
    Builder.createBranchInst(next);
    Builder.setInsertionBlock(next);

    return;
  }

  // Handle the call expression that could appear in the context of statement
  // expr without the ExpressionStatementNode wrapper.
  if (auto *call = llvh::dyn_cast<ESTree::CallExpressionNode>(stmt)) {
    return genExpressionWrapper(call);
  }

  if (auto *W = llvh::dyn_cast<ESTree::WhileStatementNode>(stmt)) {
    LLVM_DEBUG(llvh::dbgs() << "IRGen 'while' statement\n");
    genWhileLoop(W);
    return;
  }

  if (auto *D = llvh::dyn_cast<ESTree::DoWhileStatementNode>(stmt)) {
    LLVM_DEBUG(llvh::dbgs() << "IRGen 'do..while' statement\n");
    genDoWhileLoop(D);
    return;
  }

  if (auto *F = llvh::dyn_cast<ESTree::ForStatementNode>(stmt)) {
    LLVM_DEBUG(llvh::dbgs() << "IRGen 'for' statement\n");
    genForLoop(F);
    return;
  }

  if (auto *breakStmt = llvh::dyn_cast<ESTree::BreakStatementNode>(stmt)) {
    LLVM_DEBUG(llvh::dbgs() << "IRGen 'break' statement\n");

    auto &label = curFunction()->label(breakStmt);
    assert(label.breakTarget && "breakTarget not set");

    genFinallyBeforeControlChange(
        curFunction()->surroundingTry,
        label.surroundingTry,
        ControlFlowChange::Break);
    Builder.createBranchInst(label.breakTarget);

    // Continue code generation for stuff that comes after the break statement
    // in a new dead block.
    auto newBlock = Builder.createBasicBlock(curFunction()->function);
    Builder.setInsertionBlock(newBlock);
    return;
  }

  if (auto *continueStmt =
          llvh::dyn_cast<ESTree::ContinueStatementNode>(stmt)) {
    LLVM_DEBUG(llvh::dbgs() << "IRGen 'continue' statement\n");

    auto &label = curFunction()->label(continueStmt);
    assert(label.continueTarget && "continueTarget not set");

    genFinallyBeforeControlChange(
        curFunction()->surroundingTry,
        label.surroundingTry,
        ControlFlowChange::Continue,
        label.continueTarget);
    Builder.createBranchInst(label.continueTarget);

    // Continue code generation for stuff that comes after the break statement
    // in a new dead block.
    auto newBlock = Builder.createBasicBlock(curFunction()->function);
    Builder.setInsertionBlock(newBlock);
    return;
  }

  if (auto *T = llvh::dyn_cast<ESTree::TryStatementNode>(stmt)) {
    genTryStatement(T);
    return;
  }

  if (auto *T = llvh::dyn_cast<ESTree::ThrowStatementNode>(stmt)) {
    LLVM_DEBUG(llvh::dbgs() << "IRGen 'throw' statement\n");
    Value *rightHandVal = genExpression(T->_argument);
    Builder.createThrowInst(rightHandVal);

    // Throw interferes with control flow, hence we need a new block.
    auto newBlock =
        Builder.createBasicBlock(Builder.getInsertionBlock()->getParent());
    Builder.setInsertionBlock(newBlock);
    return;
  }

  // Handle empty statements.
  if (llvh::isa<ESTree::EmptyStatementNode>(stmt)) {
    return;
  }

  // Handle debugger statements.
  if (llvh::isa<ESTree::DebuggerStatementNode>(stmt)) {
    Builder.createDebuggerInst();
    return;
  }

  if (auto *importDecl = llvh::dyn_cast<ESTree::ImportDeclarationNode>(stmt)) {
    return genImportDeclaration(importDecl);
  }

  if (auto *exportDecl =
          llvh::dyn_cast<ESTree::ExportNamedDeclarationNode>(stmt)) {
    return genExportNamedDeclaration(exportDecl);
  }

  if (auto *exportDecl =
          llvh::dyn_cast<ESTree::ExportDefaultDeclarationNode>(stmt)) {
    return genExportDefaultDeclaration(exportDecl);
  }

  if (auto *exportDecl =
          llvh::dyn_cast<ESTree::ExportAllDeclarationNode>(stmt)) {
    return genExportAllDeclaration(exportDecl);
  }

  if (llvh::isa<ESTree::TypeAliasNode>(stmt))
    return;

  if (auto *classDecl = llvh::dyn_cast<ESTree::ClassDeclarationNode>(stmt)) {
    return genClassDeclaration(classDecl);
  }

  Builder.getModule()->getContext().getSourceErrorManager().error(
      stmt->getSourceRange(), Twine("invalid statement encountered."));
}

void ESTreeIRGen::genExpressionWrapper(ESTree::Node *expr) {
  Value *val = genExpression(expr);
  if (curFunction()->globalReturnRegister) {
    Builder.createStoreStackInst(val, curFunction()->globalReturnRegister);
  }
}

void ESTreeIRGen::genVariableDeclaration(
    ESTree::VariableDeclarationNode *declaration) {
  for (auto &decl : declaration->_declarations)
    genVariableDeclarator(
        declaration->_kind, cast<ESTree::VariableDeclaratorNode>(&decl));
}

void ESTreeIRGen::genVariableDeclarator(
    ESTree::NodeLabel kind,
    ESTree::VariableDeclaratorNode *declarator) {
  IRBuilder::ScopedLocationChange slc(Builder, declarator->getDebugLoc());
  Builder.getFunction()->incrementStatementCount();

  auto lref = createLRef(declarator->_id, true);
  if (declarator->_init) {
    Identifier nameHint{};
    if (llvh::isa<ESTree::IdentifierNode>(declarator->_id))
      nameHint = getNameFieldFromID(declarator->_id);
    lref.emitStore(genExpression(declarator->_init, nameHint));
  } else if (kind == kw_.identLet) {
    // "let" declarations without an initializer must be set to undefined.
    lref.emitStore(Builder.getLiteralUndefined());
  }
}

void ESTreeIRGen::genIfStatement(ESTree::IfStatementNode *IfStmt) {
  LLVM_DEBUG(llvh::dbgs() << "IRGen IF-stmt.\n");

  auto Parent = Builder.getInsertionBlock()->getParent();
  auto ThenBlock = Builder.createBasicBlock(Parent);
  auto ElseBlock = Builder.createBasicBlock(Parent);
  auto ContinueBlock = Builder.createBasicBlock(Parent);

  genExpressionBranch(IfStmt->_test, ThenBlock, ElseBlock, nullptr);

  // IRGen the Then:
  Builder.setInsertionBlock(ThenBlock);
  genStatement(IfStmt->_consequent);
  Builder.createBranchInst(ContinueBlock);

  // IRGen the Else, if it exists:
  Builder.setInsertionBlock(ElseBlock);
  if (IfStmt->_alternate) {
    genStatement(IfStmt->_alternate);
  }

  Builder.createBranchInst(ContinueBlock);
  Builder.setInsertionBlock(ContinueBlock);
}

void ESTreeIRGen::genWhileLoop(ESTree::WhileStatementNode *loop) {
  // Create the basic blocks that make the while structure.
  Function *function = Builder.getInsertionBlock()->getParent();
  BasicBlock *bodyBlock = Builder.createBasicBlock(function);
  BasicBlock *exitBlock = Builder.createBasicBlock(function);
  BasicBlock *postTestBlock = Builder.createBasicBlock(function);

  // Initialize the goto labels.
  curFunction()->initLabel(loop, exitBlock, postTestBlock);

  // Branch out of the loop if the condition is false.
  genExpressionBranch(loop->_test, bodyBlock, exitBlock, nullptr);
  // Generate the body.
  Builder.setInsertionBlock(bodyBlock);

  // Save the outer scope in case we create an inner scope.
  auto *outerScope = curFunction()->curScope;

  // If the body captures, create an inner scope for it. Note that unlike for
  // loops, we don't care whether the test expression captures, since the loop
  // does not create variables visible in the test expression.
  if (Mod->getContext().getEnableES6BlockScoping() &&
      !treeDoesNotCapture(loop->_body)) {
    auto *bodyVarScope = curFunction()->getOrCreateInnerVariableScope(loop);
    auto *bodyScope = Builder.createCreateScopeInst(bodyVarScope, outerScope);
    curFunction()->curScope = bodyScope;
  }

  genStatement(loop->_body);

  // Restore the outer scope.
  curFunction()->curScope = outerScope;

  // After executing the content of the body, jump to the post test block.
  Builder.createBranchInst(postTestBlock);
  Builder.setInsertionBlock(postTestBlock);

  // Branch out of the loop if the condition is false.
  {
    FunctionContext::AllowRecompileRAII allowRecompile(*curFunction());
    genExpressionBranch(loop->_test, bodyBlock, exitBlock, nullptr);
  }
  // Following statements are inserted to the exit block.
  Builder.setInsertionBlock(exitBlock);
}

void ESTreeIRGen::genDoWhileLoop(ESTree::DoWhileStatementNode *loop) {
  // Create the basic blocks that make the do-while structure.
  Function *function = Builder.getInsertionBlock()->getParent();
  BasicBlock *bodyBlock = Builder.createBasicBlock(function);
  BasicBlock *exitBlock = Builder.createBasicBlock(function);
  BasicBlock *postTestBlock = Builder.createBasicBlock(function);

  // Initialize the goto labels.
  curFunction()->initLabel(loop, exitBlock, postTestBlock);

  // Jump to the body.
  Builder.createBranchInst(bodyBlock);
  // Generate the body.
  Builder.setInsertionBlock(bodyBlock);
  genStatement(loop->_body);
  // After executing the content of the body, jump to the post test block.
  Builder.createBranchInst(postTestBlock);
  Builder.setInsertionBlock(postTestBlock);
  // Branch out of the loop if the condition is false.
  genExpressionBranch(loop->_test, bodyBlock, exitBlock, nullptr);
  // Following statements are inserted to the exit block.
  Builder.setInsertionBlock(exitBlock);
}

bool ESTreeIRGen::treeDoesNotCapture(ESTree::Node *tree) {
  // Stops the recursion when a function node is visited and sets a flag.
  class MayBeCaptures : public ESTree::RecursionDepthTracker<MayBeCaptures> {
   public:
    bool mayBeCaptures = false;

    void visit(ESTree::FunctionDeclarationNode *node) {
      mayBeCaptures = true;
    }
    void visit(ESTree::FunctionExpressionNode *node) {
      mayBeCaptures = true;
    }
    void visit(ESTree::ArrowFunctionExpressionNode *node) {
      mayBeCaptures = true;
    }
    void visit(ESTree::ClassExpressionNode *node) {
      mayBeCaptures = true;
    }
    void visit(ESTree::ClassDeclarationNode *node) {
      mayBeCaptures = true;
    }
    void visit(ESTree::Node *n) {
      // Stop the recursion early if we have our answer.
      if (mayBeCaptures)
        return;
      ESTree::visitESTreeChildren(*this, n);
    }

    void recursionDepthExceeded(ESTree::Node *) {
      // In case of too deep recursion, assume the expression captures.
      mayBeCaptures = true;
    }
  };

  // Nonexistent trees don't capture.
  if (!tree)
    return true;

  MayBeCaptures mbc;
  ESTree::visitESTreeNodeNoReplace(mbc, tree);
  return !mbc.mayBeCaptures;
}

void ESTreeIRGen::genForLoop(ESTree::ForStatementNode *loop) {
  // IS this a "scoped" for loop?
  // TODO: check if any of the declared variables are captured.
  if (loop->_init) {
    if (auto *VD =
            llvh::dyn_cast<ESTree::VariableDeclarationNode>(loop->_init)) {
      if (VD->_kind != kw_.identVar) {
        bool testExprCaptures = !treeDoesNotCapture(loop->_test);
        bool updateExprCaptures = !treeDoesNotCapture(loop->_update);

        // Invoke the scoped loop generation only if something is potentially
        // captured in the loop.
        if (testExprCaptures || updateExprCaptures ||
            !treeDoesNotCapture(loop->_body)) {
          genScopedForLoop(loop, testExprCaptures, updateExprCaptures);
          return;
        }
      }
    }
  }

  // Create the basic blocks that make the while structure.
  Function *function = Builder.getInsertionBlock()->getParent();
  BasicBlock *bodyBlock = Builder.createBasicBlock(function);
  BasicBlock *exitBlock = Builder.createBasicBlock(function);
  BasicBlock *postTestBlock = Builder.createBasicBlock(function);
  BasicBlock *updateBlock = Builder.createBasicBlock(function);

  // Initialize the goto labels.
  curFunction()->initLabel(loop, exitBlock, updateBlock);

  // Declarations created by the init statement.
  emitScopeDeclarations(loop->getScope());

  // Generate IR for the loop initialization.
  // The init field can be a variable declaration or any expression.
  // https://github.com/estree/estree/blob/master/spec.md#forstatement
  if (loop->_init) {
    if (llvh::isa<ESTree::VariableDeclarationNode>(loop->_init)) {
      genStatement(loop->_init);
    } else {
      genExpression(loop->_init);
    }
  }

  // Branch out of the loop if the condition is false.
  if (loop->_test)
    genExpressionBranch(loop->_test, bodyBlock, exitBlock, nullptr);
  else
    Builder.createBranchInst(bodyBlock);

  // Generate the update sequence of 'for' loops.
  Builder.setInsertionBlock(updateBlock);
  if (loop->_update)
    genExpression(loop->_update);

  // After executing the content of the body, jump to the post test block.
  Builder.createBranchInst(postTestBlock);
  Builder.setInsertionBlock(postTestBlock);

  // Branch out of the loop if the condition is false.
  if (loop->_test) {
    FunctionContext::AllowRecompileRAII allowRecompile(*curFunction());
    genExpressionBranch(loop->_test, bodyBlock, exitBlock, nullptr);
  } else {
    Builder.createBranchInst(bodyBlock);
  }

  // Generate the body.
  // Do this after the init/test/update expressions to make sure they have a
  // different statement count.
  // TODO: Determine if they should each also incrementStatementCount.
  Builder.setInsertionBlock(bodyBlock);
  genStatement(loop->_body);
  Builder.createBranchInst(updateBlock);

  // Following statements are inserted to the exit block.
  Builder.setInsertionBlock(exitBlock);
}

void ESTreeIRGen::genScopedForLoop(
    ESTree::ForStatementNode *loop,
    bool testExprCaptures,
    bool updateExprCaptures) {
  // A scoped for-loop has "interesting" semantics. The declared variables live
  // in a new scope for every iteration (so they can be captured). Additionally:
  // - the test condition executes in the current iteration's scope.
  // - the update condition executes in the next iteration's scope.
  //
  // The "natural" way to implement this would be to unroll the loop, so we
  // can emit the first iteration with its own scope and then emit the update
  // expression in the beginning of the second iteration. But that would explode
  // the code size. So we use a flag.
  //
  //      let/const oldVars = loop->init()
  //      var first = true;
  // loopBlock:
  //      Scope s = createScope
  //        declare newVars in s
  //        newVars = oldVars
  //        if !first
  //            loop->update(newVars)
  //        if !loop->test(newVars)
  //            goto exitBlock
  //        loop->body(newVars)
  //  updateBlock:
  //        oldVars(ignoring constness) = newVars
  //        first = false
  //      end s
  //      goto loopBlock

  // Clearly, this structure is awful for optimization, so we try to improve it.
  //
  // OPTIMIZATION1:
  // If the update expression doesn't capture anything, it can be emitted in the
  // current iteration scope instead of the next one.
  //      let/const oldVars = loop->init()
  // loopBlock:
  //      Scope s = createScope
  //        declare newVars in s
  //        newVars = oldVars
  //        if !loop->test(newVars)
  //            goto exitBlock
  //        loop->body(newVars)
  //  updateBlock:
  //        oldVars(ignoring constness) = newVars
  //      end s
  //      loop->update(oldVars)
  //      goto loopBlock
  //
  // OPTIMIZATION2:
  // If both the test and update expression doesn't capture anything:
  //      let/const oldVars = loop->init()
  //      if !loop->test(oldVars)
  //        goto exitBlock
  // loopBlock:
  //      Scope s = createScope
  //        declare newVars in s
  //        newVars = oldVars
  //        loop->body(newVars)
  //  updateBlock:
  //        oldVars(ignoring constness) = newVars
  //      end s
  //      loop->update(oldVars)
  //      if loop->test(oldVars)
  //          goto loopBlock
  //      goto loopBlock
  // exitBlock:

  bool testOrUpdateCapture = testExprCaptures | updateExprCaptures;

  // Create the basic blocks that make the while structure.
  Function *function = Builder.getInsertionBlock()->getParent();
  BasicBlock *dontExitBlock = nullptr;
  if (!testOrUpdateCapture && loop->_test) {
    dontExitBlock = Builder.createBasicBlock(function);
  }
  BasicBlock *loopBlock = Builder.createBasicBlock(function);
  BasicBlock *firstIterBlock = nullptr;
  BasicBlock *notFirstIterBlock = nullptr;
  if (updateExprCaptures) {
    firstIterBlock = Builder.createBasicBlock(function);
    notFirstIterBlock = Builder.createBasicBlock(function);
  }
  BasicBlock *bodyBlock = Builder.createBasicBlock(function);
  BasicBlock *updateBlock = Builder.createBasicBlock(function);
  BasicBlock *exitBlock = Builder.createBasicBlock(function);
  // A mapping between an "old" variable and its "new" version in the loop body
  // scope.
  struct VarMapping {
    sema::Decl *decl;
    Variable *oldVar;
    Variable *newVar;
    VarMapping(sema::Decl *decl, Variable *oldVar, Variable *newVar)
        : decl(decl), oldVar(oldVar), newVar(newVar) {}
  };
  llvh::SmallVector<VarMapping, 2> vars{};

  // Initialize the goto labels.
  curFunction()->initLabel(loop, exitBlock, updateBlock);

  // Declarations created by the init statement.
  emitScopeDeclarations(loop->getScope());

  // Generate IR for the loop initialization.
  assert(
      loop->_init && llvh::isa<ESTree::VariableDeclarationNode>(loop->_init) &&
      "A scoped for-loop must have an init declaration");
  genStatement(loop->_init);

  if (!testOrUpdateCapture && loop->_test) {
    // Branch out of the loop if the condition is false.
    genExpressionBranch(loop->_test, dontExitBlock, exitBlock, nullptr);
    Builder.setInsertionBlock(dontExitBlock);
  }

  Builder.setLocation(loop->_init->getDebugLoc());
  // Create the "first" flag and set it to true.
  AllocStackInst *firstStack = nullptr;
  if (updateExprCaptures) {
    firstStack = Builder.createAllocStackInst(
        genAnonymousLabelName("first"), Type::createBoolean());
    Builder.createStoreStackInst(Builder.getLiteralBool(true), firstStack);
  }
  Builder.createBranchInst(loopBlock);

  // The loop starts here.
  Builder.setInsertionBlock(loopBlock);
  Builder.setLocation(loop->_body->getDebugLoc());
  // TODO: create a scope.
  auto *curScope = curFunction()->curScope;

  // Declare the new variables in the new scope and copy the declared variables
  // into the new ones. Change the decls to resolve to the "new" variables, so
  // all further accesses will refer to them.
  assert(
      loop->getScope() && !loop->getScope()->decls.empty() &&
      "for-loop scope can't be empty if there is a scoped init");

  IRBuilder::ScopedLocationChange slc(
      Builder,
      loop->_init ? loop->_init->getDebugLoc() : loop->_body->getDebugLoc());
  for (auto *decl : loop->getScope()->decls) {
    assert(!decl->isKindGlobal(decl->kind) && "for(;;) can't declare globals");
    Variable *oldVar = llvh::cast<Variable>(getDeclData(decl));
    // Create a copy of the variable. Note that it doesn't need TDZ, since we
    // are initializing it here.
    Variable *newVar = Builder.createVariable(
        curFunction()->curScope->getVariableScope(),
        decl->name,
        Type::subtractTy(oldVar->getType(), Type::createEmpty()),
        /* hidden */ false);

    vars.emplace_back(decl, oldVar, newVar);

    // Note that we are using a direct load/store, which doesn't check TDZ. If
    // our thinking is correct, all variables declared in the for(;;) must have
    // been initialized.
    Value *val = Builder.createLoadFrameInst(curScope, oldVar);
    if (val->getType().canBeEmpty())
      val = Builder.createUnionNarrowTrustedInst(val, newVar->getType());
    Builder.createStoreFrameInst(curScope, val, newVar);

    // Update the declaration to resolve to the new variable.
    setDeclData(decl, newVar);
  }

  if (updateExprCaptures) {
    // if first...
    Builder.createCondBranchInst(
        Builder.createLoadStackInst(firstStack),
        firstIterBlock,
        notFirstIterBlock);

    // emit the update.
    Builder.setLocation(loop->_update->getDebugLoc());
    Builder.setInsertionBlock(notFirstIterBlock);
    genExpression(loop->_update);

    Builder.createBranchInst(firstIterBlock);
    Builder.setInsertionBlock(firstIterBlock);
  }

  // Branch out of the loop if the condition is false.
  if (testOrUpdateCapture && loop->_test)
    genExpressionBranch(loop->_test, bodyBlock, exitBlock, nullptr);
  else
    Builder.createBranchInst(bodyBlock);

  // Generate the body.
  Builder.setInsertionBlock(bodyBlock);
  genStatement(loop->_body);
  Builder.createBranchInst(updateBlock);

  Builder.setInsertionBlock(updateBlock);
  // Copy the new vars back into the old ones.
  for (const auto &mapping : vars) {
    Builder.createStoreFrameInst(
        curScope,
        Builder.createLoadFrameInst(curScope, mapping.newVar),
        mapping.oldVar);
    // Update the declaration to resolve to the old variable.
    setDeclData(mapping.decl, mapping.oldVar);
  }

  // Set "first" to false.
  if (updateExprCaptures)
    Builder.createStoreStackInst(Builder.getLiteralBool(false), firstStack);

  if (!updateExprCaptures && loop->_update) {
    genExpression(loop->_update);
  }

  // Loop.
  if (!testOrUpdateCapture && loop->_test) {
    FunctionContext::AllowRecompileRAII allowRecompile(*curFunction());
    genExpressionBranch(loop->_test, loopBlock, exitBlock, nullptr);
  } else {
    Builder.createBranchInst(loopBlock);
  }

  // Following statements are inserted to the exit block.
  Builder.setInsertionBlock(exitBlock);
}

void ESTreeIRGen::genForInStatement(ESTree::ForInStatementNode *ForInStmt) {
  emitScopeDeclarations(ForInStmt->getScope());

  // The state of the enumerator. Notice that the instruction writes to the
  // storage
  // variables just like Load/Store instructions write to stack allocations.
  auto *iteratorStorage = Builder.createAllocStackInst(
      genAnonymousLabelName("iter"), Type::createAnyType());
  auto *baseStorage = Builder.createAllocStackInst(
      genAnonymousLabelName("base"), Type::createAnyType());
  auto *indexStorage = Builder.createAllocStackInst(
      genAnonymousLabelName("idx"), Type::createNumber());
  auto *sizeStorage = Builder.createAllocStackInst(
      genAnonymousLabelName("size"), Type::createNumber());

  // Check for the obscure case "for(var i = init in ....)". We need to
  // initialize the loop variable with the initializer.
  if (auto *VD =
          llvh::dyn_cast<ESTree::VariableDeclarationNode>(ForInStmt->_left)) {
    assert(
        VD->_declarations.size() == 1 && "for-in must have a single binding");
    auto *declarator =
        cast<ESTree::VariableDeclaratorNode>(&VD->_declarations.front());
    if (declarator->_init) {
      // Note that we need to create a separate LReference for the
      // initialization because the loop one must execute inside the loop for
      // cases like "for(a[i++] in ...)".
      LReference initRef = createLRef(VD, true);
      initRef.emitStore(genExpression(declarator->_init));
    }
  }

  // Generate the right hand side of the for-in loop. The result of this
  // expression is the object we iterate on. We use this object as the 'base'
  // of the enumerator.
  Value *object = genExpression(ForInStmt->_right);
  Builder.createStoreStackInst(object, baseStorage);

  // The storage for the property name that the enumerator loads:
  auto *propertyStorage = Builder.createAllocStackInst(
      genAnonymousLabelName("prop"), Type::createAnyType());

  /*
    We generate the following loop structure for the for-in loops:

        [ current block ]
        [   get_pname   ]
               |         \
               |          \
               v           \ (on empty object)
    /----> [get_next]       \
    |          |     \       \
    |          |      \       \
    |          |       \       \ ->[ exit block ]
    |          |        \      /
    |    [ body block ]  \____/
    |          |          (on last iteration)
    \__________/
  */

  auto parent = Builder.getInsertionBlock()->getParent();
  auto *exitBlock = Builder.createBasicBlock(parent);
  auto *getNextBlock = Builder.createBasicBlock(parent);
  auto *bodyBlock = Builder.createBasicBlock(parent);

  // Initialize the goto labels.
  curFunction()->initLabel(ForInStmt, exitBlock, getNextBlock);

  // Create the enumerator:
  Builder.createGetPNamesInst(
      iteratorStorage,
      baseStorage,
      indexStorage,
      sizeStorage,
      exitBlock,
      getNextBlock);

  // Generate the get_next part of the loop:
  Builder.setInsertionBlock(getNextBlock);
  Builder.createGetNextPNameInst(
      propertyStorage,
      baseStorage,
      indexStorage,
      sizeStorage,
      iteratorStorage,
      exitBlock,
      bodyBlock);

  // Emit the loop body and setup the property variable. When done jump into the
  // 'get_next' block and try to do another iteration.
  Builder.setInsertionBlock(bodyBlock);

  // The string property value of the current iteration is saved into this
  // variable.
  auto propertyStringRepr = Builder.createLoadStackInst(propertyStorage);

  // The left hand side of For-In statements can be any lhs expression
  // ("PutValue"). Example:
  //  1. for (x.y in [1,2,3])
  //  2. for (x in [1,2,3])
  //  3. for (var x in [1,2,3])
  // See ES5 $12.6.4 "The for-in Statement"
  LReference lref = createLRef(ForInStmt->_left, false);
  lref.emitStore(propertyStringRepr);

  genStatement(ForInStmt->_body);

  Builder.createBranchInst(getNextBlock);

  Builder.setInsertionBlock(exitBlock);
}

void ESTreeIRGen::genForOfStatement(ESTree::ForOfStatementNode *forOfStmt) {
  if (auto *type = llvh::dyn_cast<flow::ArrayType>(
          flowContext_.getNodeTypeOrAny(forOfStmt->_right)->info)) {
    return genForOfFastArrayStatement(forOfStmt, type);
  }

  emitScopeDeclarations(forOfStmt->getScope());

  auto *function = Builder.getInsertionBlock()->getParent();
  auto *getNextBlock = Builder.createBasicBlock(function);
  auto *bodyBlock = Builder.createBasicBlock(function);
  auto *exitBlock = Builder.createBasicBlock(function);

  // Initialize the goto labels.
  curFunction()->initLabel(forOfStmt, exitBlock, getNextBlock);

  auto *exprValue = genExpression(forOfStmt->_right);
  const IteratorRecord iteratorRecord = emitGetIterator(exprValue);

  Builder.createBranchInst(getNextBlock);

  // Attempt to retrieve the next value. If iteration is complete, finish the
  // loop. This stays outside the SurroundingTry below because exceptions in
  // `.next()` should not call `.return()` on the iterator.
  Builder.setInsertionBlock(getNextBlock);
  auto *nextValue = emitIteratorNext(iteratorRecord);
  auto *done = emitIteratorComplete(iteratorRecord);
  Builder.createCondBranchInst(done, exitBlock, bodyBlock);

  Builder.setInsertionBlock(bodyBlock);

  emitTryCatchScaffolding(
      getNextBlock,
      // emitBody.
      [this, forOfStmt, nextValue, &iteratorRecord, getNextBlock](
          BasicBlock *catchBlock) {
        // Generate IR for the body of Try
        SurroundingTry thisTry{
            curFunction(),
            forOfStmt,
            catchBlock,
            {},
            [this, &iteratorRecord, getNextBlock](
                ESTree::Node *,
                ControlFlowChange cfc,
                BasicBlock *continueTarget) {
              // Only emit the iteratorClose if this is a
              // 'break' or if the target of the control flow
              // change is outside the current loop. If
              // continuing the existing loop, do not close
              // the iterator.
              if (cfc == ControlFlowChange::Break ||
                  continueTarget != getNextBlock)
                emitIteratorClose(iteratorRecord, false);
            }};

        // Note: obtaining the value is not protected, but storing it is.
        createLRef(forOfStmt->_left, false).emitStore(nextValue);

        genStatement(forOfStmt->_body);
        Builder.setLocation(SourceErrorManager::convertEndToLocation(
            forOfStmt->_body->getSourceRange()));
      },
      // emitNormalCleanup.
      []() {},
      // emitHandler.
      [this, &iteratorRecord](BasicBlock *) {
        auto *catchReg = Builder.createCatchInst();
        emitIteratorClose(iteratorRecord, true);
        Builder.createThrowInst(catchReg);
      });

  Builder.setInsertionBlock(exitBlock);
}

void ESTreeIRGen::genForOfFastArrayStatement(
    ESTree::ForOfStatementNode *forOfStmt,
    flow::ArrayType *type) {
  emitScopeDeclarations(forOfStmt->getScope());

  auto *function = Builder.getInsertionBlock()->getParent();
  // Set the element variable and run the body.
  auto *bodyBlock = Builder.createBasicBlock(function);
  // Exit the loop.
  auto *exitBlock = Builder.createBasicBlock(function);
  // Check index < length at the end of the loop.
  auto *postTestBlock = Builder.createBasicBlock(function);
  // ++index.
  auto *incrementBlock = Builder.createBasicBlock(function);

  // Initialize the goto labels.
  // Break goes to the exit, continue goes to the incrementBlock.
  curFunction()->initLabel(forOfStmt, exitBlock, incrementBlock);

  // RHS expression.
  auto *exprValue = genExpression(forOfStmt->_right);
  // Index for iteration.
  auto *idx = Builder.createAllocStackInst(
      genAnonymousLabelName("forOfIndex"), Type::createNumber());
  Builder.createStoreStackInst(Builder.getLiteralNumber(0), idx);
  LoadStackInst *loadStack1 = Builder.createLoadStackInst(idx);
  auto *cond = Builder.createFCompareInst(
      ValueKind::FLessThanInstKind,
      loadStack1,
      Builder.createFastArrayLengthInst(exprValue));
  Builder.createCondBranchInst(cond, bodyBlock, exitBlock);

  Builder.setInsertionBlock(bodyBlock);
  // Load the element from the array.
  auto *nextValue = Builder.createFastArrayLoadInst(
      exprValue,
      Builder.createLoadStackInst(idx),
      flowTypeToIRType(type->getElement()));
  createLRef(forOfStmt->_left, false).emitStore(nextValue);
  // Run the body of the loop.
  genStatement(forOfStmt->_body);
  Builder.createBranchInst(incrementBlock);

  // Increment the index (this is the 'continue' target).
  Builder.setInsertionBlock(incrementBlock);
  Builder.createStoreStackInst(
      Builder.createFBinaryMathInst(
          ValueKind::FAddInstKind,
          Builder.createLoadStackInst(idx),
          Builder.getLiteralNumber(1)),
      idx);
  Builder.createBranchInst(postTestBlock);

  // Use a separate block here for symmetry with other loops.
  Builder.setInsertionBlock(postTestBlock);
  // Branch out of the loop if the index has reached the end of the array.
  LoadStackInst *loadStack2 = Builder.createLoadStackInst(idx);
  auto *cond2 = Builder.createFCompareInst(
      ValueKind::FLessThanInstKind,
      loadStack2,
      Builder.createFastArrayLengthInst(exprValue));
  Builder.createCondBranchInst(cond2, bodyBlock, exitBlock);

  Builder.setInsertionBlock(exitBlock);
}

void ESTreeIRGen::genReturnStatement(ESTree::ReturnStatementNode *RetStmt) {
  LLVM_DEBUG(llvh::dbgs() << "IRGen Return-stmt.\n");

  Value *Value;
  // Generate IR for the return value, or undefined if this is an empty return
  // statement.
  if (auto *A = RetStmt->_argument) {
    Value = genExpression(A);
  } else {
    Value = Builder.getLiteralUndefined();
  }

  genFinallyBeforeControlChange(
      curFunction()->surroundingTry, nullptr, ControlFlowChange::Break);
  Builder.createReturnInst(Value);

  // Code that comes after 'return' is dead code. Let's create a new un-linked
  // basic block and keep IRGen in that block. The optimizer will clean things
  // up.
  auto Parent = Builder.getInsertionBlock()->getParent();
  Builder.setInsertionBlock(Builder.createBasicBlock(Parent));
}

/// \returns true if \p node is the default case.
static inline bool isDefaultCase(ESTree::SwitchCaseNode *caseStmt) {
  // If there is no test field then this is the default block.
  return !caseStmt->_test;
}

bool ESTreeIRGen::areAllCasesConstant(
    ESTree::SwitchStatementNode *switchStmt,
    llvh::SmallVectorImpl<Literal *> &caseLiterals) {
  for (auto &c : switchStmt->_cases) {
    auto *caseStmt = cast<ESTree::SwitchCaseNode>(&c);

    if (isDefaultCase(caseStmt)) {
      caseLiterals.push_back(nullptr);
      continue;
    }

    if (!isConstantExpr(caseStmt->_test))
      return false;

    auto *lit = llvh::dyn_cast<Literal>(genExpression(caseStmt->_test));
    assert(lit && "constant expression must compile to a literal");
    caseLiterals.push_back(lit);
  }

  return true;
}

void ESTreeIRGen::genSwitchStatement(ESTree::SwitchStatementNode *switchStmt) {
  LLVM_DEBUG(llvh::dbgs() << "IRGen 'switch' statement.\n");

  emitScopeDeclarations(switchStmt->getScope());

  {
    llvh::SmallVector<Literal *, 8> caseLiterals{};
    if (areAllCasesConstant(switchStmt, caseLiterals) &&
        caseLiterals.size() > 1) {
      genConstSwitchStmt(switchStmt, caseLiterals);
      return;
    }
  }

  Function *function = Builder.getInsertionBlock()->getParent();
  BasicBlock *exitBlock = Builder.createBasicBlock(function);

  // Unless a default is specified the default case brings us to the exit block.
  BasicBlock *defaultBlock = exitBlock;

  // A BB for each case in the switch statement.
  llvh::SmallVector<BasicBlock *, 8> caseBlocks;

  // Initialize the goto labels.
  curFunction()->initLabel(switchStmt, exitBlock, nullptr);

  // The discriminator expression.
  Value *discr = genExpression(switchStmt->_discriminant);

  // Sequentially allocate a basic block for each case, compare the discriminant
  // against the case value and conditionally jump to the basic block.
  int caseIndex = -1; // running index of the case's basic block.
  BasicBlock *elseBlock = nullptr; // The next case's condition.

  for (auto &c : switchStmt->_cases) {
    auto *caseStmt = cast<ESTree::SwitchCaseNode>(&c);
    ++caseIndex;
    caseBlocks.push_back(Builder.createBasicBlock(function));

    if (isDefaultCase(caseStmt)) {
      defaultBlock = caseBlocks.back();
      continue;
    }

    auto *caseVal = genExpression(caseStmt->_test);
    auto *pred = Builder.createBinaryOperatorInst(
        caseVal, discr, ValueKind::BinaryStrictlyEqualInstKind);

    elseBlock = Builder.createBasicBlock(function);
    Builder.createCondBranchInst(pred, caseBlocks[caseIndex], elseBlock);
    Builder.setInsertionBlock(elseBlock);
  }

  Builder.createBranchInst(defaultBlock);

  // Generate the case bodies.
  bool isFirstCase = true;
  caseIndex = -1;
  for (auto &c : switchStmt->_cases) {
    auto *caseStmt = cast<ESTree::SwitchCaseNode>(&c);
    ++caseIndex;

    // Generate the fall-through from the previous block to this one.
    if (!isFirstCase)
      Builder.createBranchInst(caseBlocks[caseIndex]);

    Builder.setInsertionBlock(caseBlocks[caseIndex]);
    genBody(caseStmt->_consequent);
    isFirstCase = false;
  }

  if (!isFirstCase)
    Builder.createBranchInst(exitBlock);

  Builder.setInsertionBlock(exitBlock);
}

void ESTreeIRGen::genConstSwitchStmt(
    ESTree::SwitchStatementNode *switchStmt,
    llvh::SmallVectorImpl<Literal *> &caseLiterals) {
  Function *function = Builder.getInsertionBlock()->getParent();
  BasicBlock *exitBlock = Builder.createBasicBlock(function);

  // Unless a default is specified the default case brings us to the exit block.
  BasicBlock *defaultBlock = exitBlock;

  curFunction()->initLabel(switchStmt, exitBlock, nullptr);

  // The discriminator expression.
  Value *discr = genExpression(switchStmt->_discriminant);
  // Save the block where we will insert the switch instruction.
  auto *startBlock = Builder.getInsertionBlock();

  // Since this is a constant value switch, duplicates are not allowed and we
  // must filter them. We can conveniently store them in this set.
  llvh::SmallPtrSet<Literal *, 8> valueSet;

  SwitchInst::ValueListType values;
  SwitchInst::BasicBlockListType blocks;

  int caseIndex = -1;
  bool isFirstCase = true;

  for (auto &c : switchStmt->_cases) {
    auto *caseStmt = cast<ESTree::SwitchCaseNode>(&c);
    auto *caseBlock = Builder.createBasicBlock(function);
    ++caseIndex;

    if (isDefaultCase(caseStmt)) {
      defaultBlock = caseBlock;
    } else {
      auto *lit = caseLiterals[caseIndex];

      // Only generate the case and block if this is the first occurence of the
      // value.
      if (valueSet.insert(lit).second) {
        values.push_back(lit);
        blocks.push_back(caseBlock);
      }
    }

    if (!isFirstCase)
      Builder.createBranchInst(caseBlock);

    Builder.setInsertionBlock(caseBlock);
    genBody(caseStmt->_consequent);
    isFirstCase = false;
  }

  if (!isFirstCase)
    Builder.createBranchInst(exitBlock);

  Builder.setInsertionBlock(startBlock);
  Builder.createSwitchInst(discr, defaultBlock, values, blocks);

  Builder.setInsertionBlock(exitBlock);
};

void ESTreeIRGen::genImportDeclaration(
    ESTree::ImportDeclarationNode *importDecl) {
  assert(
      Mod->getContext().getUseCJSModules() &&
      "import/export requires module mode");
  // Modules have these arguments: (this, exports, require, module)
  assert(
      Builder.getFunction()->getJSDynamicParams()[2]->getName().str() ==
          "require" &&
      "CJS module second parameter must be 'require'");
  Value *require = curFunction()->jsParams[2];
  auto *source = genExpression(importDecl->_source);
  auto *exports = Builder.createCallInst(
      require,
      /* newTarget */ Builder.getLiteralUndefined(),
      /* thisValue */ Builder.getLiteralUndefined(),
      {source});
  // An import declaration is a list of import specifiers.
  for (ESTree::Node &spec : importDecl->_specifiers) {
    if (auto *ids = llvh::dyn_cast<ESTree::ImportDefaultSpecifierNode>(&spec)) {
      // import defaultProperty from 'file.js';
      auto *local = resolveIdentifierFromID(ids->_local);
      assert(local && "imported name should have been hoisted");
      emitStore(
          Builder.createLoadPropertyInst(exports, identDefaultExport_),
          local,
          true);
    } else if (
        auto *ins =
            llvh::dyn_cast<ESTree::ImportNamespaceSpecifierNode>(&spec)) {
      // import * as File from 'file.js';
      auto *local = resolveIdentifierFromID(ins->_local);
      assert(local && "imported name should have been hoisted");
      emitStore(exports, local, true);
    } else {
      // import {x as y} as File from 'file.js';
      // import {x} as File from 'file.js';
      auto *is = cast<ESTree::ImportSpecifierNode>(&spec);

      // Store to a local variable with the name is->_local.
      auto *local = resolveIdentifierFromID(is->_local);
      assert(local && "imported name should have been hoisted");

      // Get is->_imported from the exports object, because that's what the
      // other file stored it as.
      emitStore(
          Builder.createLoadPropertyInst(
              exports, getNameFieldFromID(is->_imported)),
          local,
          true);
    }
  }
  return;
}

void ESTreeIRGen::genExportNamedDeclaration(
    ESTree::ExportNamedDeclarationNode *exportDecl) {
  assert(
      Mod->getContext().getUseCJSModules() &&
      "import/export requires module mode");
  // Modules have these arguments: (this, exports, require, module)
  assert(
      Builder.getFunction()->getJSDynamicParams()[1]->getName().str() ==
          "exports" &&
      "CJS module first parameter must be 'exports'");
  assert(
      Builder.getFunction()->getJSDynamicParams()[2]->getName().str() ==
          "require" &&
      "CJS module second parameter must be 'require'");
  Value *exports = curFunction()->jsParams[1];
  Value *require = curFunction()->jsParams[2];

  // Generate IR for exports of declarations.
  if (auto *decl = exportDecl->_declaration) {
    // If we have a declaration, then we cannot have a source or specifiers.
    // We can generate IR for the declaration and immediately return.
    assert(
        !exportDecl->_source &&
        "ExportNamedDeclarationNode with Declaration cannot have a source");
    assert(
        exportDecl->_specifiers.empty() &&
        "ExportNamedDeclarationNode with Declaration cannot have specifiers");
    if (auto *varDecl = llvh::dyn_cast<ESTree::VariableDeclarationNode>(decl)) {
      // export var x = 1, y = 2;
      for (auto &declarator : varDecl->_declarations) {
        auto *variableDeclarator =
            cast<ESTree::VariableDeclaratorNode>(&declarator);
        genVariableDeclarator(varDecl->_kind, variableDeclarator);
        Identifier name = getNameFieldFromID(variableDeclarator->_id);

        Builder.createStorePropertyInst(
            emitLoad(resolveIdentifierFromID(variableDeclarator->_id), false),
            exports,
            name);
      }
    } else if (
        auto *classDecl = llvh::dyn_cast<ESTree::ClassDeclarationNode>(decl)) {
      (void)classDecl;
      // TODO: Support class declarations once we support class IRGen.
      Builder.getModule()->getContext().getSourceErrorManager().error(
          exportDecl->getSourceRange(),
          Twine("class declaration exports are unsupported"));
    } else {
      auto *funDecl = llvh::dyn_cast<ESTree::FunctionDeclarationNode>(decl);
      // export function x() {}
      auto *fun = emitLoad(resolveIdentifierFromID(funDecl->_id), false);
      Builder.createStorePropertyInst(
          fun, exports, getNameFieldFromID(funDecl->_id));
    }

    return;
  }

  auto *source = exportDecl->_source
      ? Builder.createCallInst(
            require,
            /* newTarget */ Builder.getLiteralUndefined(),
            /* thisValue */ Builder.getLiteralUndefined(),
            {genExpression(exportDecl->_source)})
      : nullptr;

  for (ESTree::Node &spec : exportDecl->_specifiers) {
    auto *es = cast<ESTree::ExportSpecifierNode>(&spec);
    auto *localIdent = cast<ESTree::IdentifierNode>(es->_local);
    auto *exportedIdent = cast<ESTree::IdentifierNode>(es->_exported);
    // If we read from a source, load the property from the source exports
    // object, else generate the IdentifierExpression and resolve the variable
    // name.
    auto *local = source
        ? Builder.createLoadPropertyInst(source, getNameFieldFromID(localIdent))
        : genIdentifierExpression(localIdent, false);
    Builder.createStorePropertyInst(
        local, exports, getNameFieldFromID(exportedIdent));
  }
}

void ESTreeIRGen::genExportDefaultDeclaration(
    ESTree::ExportDefaultDeclarationNode *exportDecl) {
  // Modules have these arguments: (this, exports, require, module)
  assert(
      Builder.getFunction()->getJSDynamicParams()[1]->getName().str() ==
          "exports" &&
      "CJS module first parameter must be 'exports'");
  Value *exports = curFunction()->jsParams[1];
  auto *decl = exportDecl->_declaration;
  if (auto *funDecl = llvh::dyn_cast<ESTree::FunctionDeclarationNode>(decl)) {
    assert(
        funDecl->_id &&
        "SemanticValidator should change anonymous FunctionDeclaration");
    // export default function foo() {}
    // The function declaration should have been hoisted,
    // so simply load it and store it in the default slot.
    auto *fun = emitLoad(resolveIdentifierFromID(funDecl->_id), false);
    Builder.createStorePropertyInst(
        fun, exports, getNameFieldFromID(funDecl->_id));
  } else if (
      auto *classDecl = llvh::dyn_cast<ESTree::ClassDeclarationNode>(decl)) {
    (void)classDecl;
    // TODO: Support default class declarations once we support class IRGen.
    Builder.getModule()->getContext().getSourceErrorManager().error(
        exportDecl->getSourceRange(),
        "default class declaration exports are unsupported");
  } else {
    // export default {expression};
    auto *value = genExpression(decl);
    Builder.createStorePropertyInst(value, exports, identDefaultExport_);
  }
}

void ESTreeIRGen::genExportAllDeclaration(
    ESTree::ExportAllDeclarationNode *exportDecl) {
  assert(
      Mod->getContext().getUseCJSModules() &&
      "import/export requires module mode");
  // Modules have these arguments: (this, exports, require, module)
  assert(
      Builder.getFunction()->getJSDynamicParams()[1]->getName().str() ==
          "exports" &&
      "CJS module first parameter must be 'exports'");
  assert(
      Builder.getFunction()->getJSDynamicParams()[2]->getName().str() ==
          "require" &&
      "CJS module second parameter must be 'require'");
  Value *exports = curFunction()->jsParams[1];
  Value *require = curFunction()->jsParams[2];
  // export * from 'file.js';
  auto *source = Builder.createCallInst(
      require,
      /* newTarget */ Builder.getLiteralUndefined(),
      /* thisValue */ Builder.getLiteralUndefined(),
      {genExpression(exportDecl->_source)});
  // Copy all the re-exported properties from the source to the exports object.
  genBuiltinCall(BuiltinMethod::HermesBuiltin_exportAll, {exports, source});
}

} // namespace irgen
} // namespace hermes
