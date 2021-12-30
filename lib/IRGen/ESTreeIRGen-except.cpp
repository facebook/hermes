/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ESTreeIRGen.h"

#include "llvh/Support/SaveAndRestore.h"

namespace hermes {
namespace irgen {

void ESTreeIRGen::genTryStatement(ESTree::TryStatementNode *tryStmt) {
  LLVM_DEBUG(dbgs() << "IRGen 'try' statement\n");

  // try-catch-finally statements must have been transformed by the validator
  // into two nested try statements with only "catch" or "finally" each.
  assert(
      (!tryStmt->_handler || !tryStmt->_finalizer) &&
      "Try statement can't have both catch and finally");

  auto *nextBlock = emitTryCatchScaffolding(
      nullptr,
      // emitBody.
      [this, tryStmt]() {
        llvh::Optional<SurroundingTry> thisTry;

        if (tryStmt->_finalizer) {
          thisTry.emplace(
              curFunction(),
              tryStmt,
              tryStmt->_finalizer->getDebugLoc(),
              [this](ESTree::Node *node, ControlFlowChange, BasicBlock *) {
                genStatement(cast<ESTree::TryStatementNode>(node)->_finalizer);
              });
        } else {
          thisTry.emplace(curFunction(), tryStmt);
        }

        genStatement(tryStmt->_block);

        Builder.setLocation(SourceErrorManager::convertEndToLocation(
            tryStmt->_block->getSourceRange()));
      },
      // emitNormalCleanup.
      [this, tryStmt]() {
        if (tryStmt->_finalizer) {
          genStatement(tryStmt->_finalizer);
          Builder.setLocation(SourceErrorManager::convertEndToLocation(
              tryStmt->_finalizer->getSourceRange()));
        }
      },
      // emitHandler.
      [this, tryStmt](BasicBlock *nextBlock) {
        // If we have a catch block.
        if (tryStmt->_handler) {
          auto *catchClauseNode =
              llvh::dyn_cast<ESTree::CatchClauseNode>(tryStmt->_handler);

          // Catch takes a exception variable, hence we need to create a new
          // scope for it.
          NameTableScopeTy newScope(nameTable_);

          Builder.setLocation(tryStmt->_handler->getDebugLoc());
          prepareCatch(catchClauseNode->_param);

          genStatement(catchClauseNode->_body);

          Builder.setLocation(SourceErrorManager::convertEndToLocation(
              tryStmt->_handler->getSourceRange()));
          Builder.createBranchInst(nextBlock);
        } else {
          // A finally block catches the exception and rethrows is.
          Builder.setLocation(tryStmt->_finalizer->getDebugLoc());
          auto *catchReg = Builder.createCatchInst();

          genStatement(tryStmt->_finalizer);

          Builder.setLocation(SourceErrorManager::convertEndToLocation(
              tryStmt->_finalizer->getSourceRange()));
          Builder.createThrowInst(catchReg);
        }
      });

  Builder.setInsertionBlock(nextBlock);
}

CatchInst *ESTreeIRGen::prepareCatch(ESTree::NodePtr catchParam) {
  auto *catchInst = Builder.createCatchInst();

  if (!catchParam) {
    // Optional catch binding allows us to emit no extra code for the catch.
    return catchInst;
  }

  if (!llvh::isa<ESTree::IdentifierNode>(catchParam)) {
    Builder.getModule()->getContext().getSourceErrorManager().error(
        catchParam->getSourceRange(),
        Twine("Destructuring in catch parameters is currently unsupported"));
    return nullptr;
  }

  auto catchVariableName =
      getNameFieldFromID(cast<ESTree::IdentifierNode>(catchParam));

  // Generate a unique catch variable name and use this name for IRGen purpose
  // only. The variable lookup in the catch clause will continue to be done
  // using the declared name.
  auto uniquedCatchVariableName =
      genAnonymousLabelName(catchVariableName.str());

  auto errorVar = Builder.createVariable(
      curFunction()->function->getFunctionScope(),
      Variable::DeclKind::Var,
      uniquedCatchVariableName);

  /// Insert the synthesized variable into the function name table, so it can
  /// be looked up internally.
  nameTable_.insertIntoScope(
      &curFunction()->scope, errorVar->getName(), errorVar);

  // Alias the lexical name to the synthesized variable.
  nameTable_.insert(catchVariableName, errorVar);

  emitStore(Builder, catchInst, errorVar, true);
  return catchInst;
}

void ESTreeIRGen::genFinallyBeforeControlChange(
    SurroundingTry *sourceTry,
    SurroundingTry *targetTry,
    ControlFlowChange cfc,
    BasicBlock *continueTarget) {
  assert(
      (cfc == ControlFlowChange::Break || continueTarget != nullptr) &&
      "Continue ControlFlowChange must have a target");
  // We walk the nested try statements starting from the source, until we reach
  // the target, generating the finally statements on the way.
  for (; sourceTry != targetTry; sourceTry = sourceTry->outer) {
    assert(sourceTry && "invalid try chain");

    // Emit an end of the try statement.
    auto *tryEndBlock = Builder.createBasicBlock(curFunction()->function);
    Builder.createBranchInst(tryEndBlock);
    Builder.setInsertionBlock(tryEndBlock);

    // Make sure we use the correct debug location for tryEndInst.
    if (sourceTry->tryEndLoc.isValid()) {
      hermes::IRBuilder::ScopedLocationChange slc(
          Builder, sourceTry->tryEndLoc);
      Builder.createTryEndInst();
    } else {
      Builder.createTryEndInst();
    }

    if (sourceTry->genFinalizer) {
      // Recreate the state of the try stack on entrance to the finally block.
      llvh::SaveAndRestore<SurroundingTry *> sr{
          curFunction()->surroundingTry, sourceTry->outer};
      sourceTry->genFinalizer(sourceTry->node, cfc, continueTarget);
    }
  }
}

} // namespace irgen
} // namespace hermes
