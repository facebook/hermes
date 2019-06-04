/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "ESTreeIRGen.h"

#include "llvm/Support/SaveAndRestore.h"

namespace hermes {
namespace irgen {

void ESTreeIRGen::genTryStatement(ESTree::TryStatementNode *tryStmt) {
  LLVM_DEBUG(dbgs() << "IRGen 'try' statement\n");
  auto *parent = Builder.getInsertionBlock()->getParent();

  // try-catch-finally statements must have been transformed by the validator
  // into two nested try statements with only "catch" or "finally" each.
  assert(
      (!tryStmt->_handler || !tryStmt->_finalizer) &&
      "Try statement can't have both catch and finally");

  auto *catchBlock = Builder.createBasicBlock(parent);
  auto *continueBlock = Builder.createBasicBlock(parent);
  auto *tryBodyBlock = Builder.createBasicBlock(parent);

  // Start with a TryStartInst, and transition to try body.
  Builder.createTryStartInst(tryBodyBlock, catchBlock);
  Builder.setInsertionBlock(tryBodyBlock);

  // Generate IR for the body of Try
  {
    SurroundingTry thisTry{curFunction(), tryStmt, tryStmt->_finalizer};
    genStatement(tryStmt->_block);
  }

  // Emit TryEnd in a new block.
  Builder.setLocation(SourceErrorManager::convertEndToLocation(
      tryStmt->_block->getSourceRange()));
  auto *tryEndBlock = Builder.createBasicBlock(parent);
  Builder.createBranchInst(tryEndBlock);
  Builder.setInsertionBlock(tryEndBlock);
  Builder.createTryEndInst();

  if (tryStmt->_finalizer) {
    genStatement(tryStmt->_finalizer);
    Builder.setLocation(SourceErrorManager::convertEndToLocation(
        tryStmt->_finalizer->getSourceRange()));
  }

  Builder.createBranchInst(continueBlock);

  // Generate the catch/finally block.
  Builder.setInsertionBlock(catchBlock);

  // If we have a catch block.
  if (tryStmt->_handler) {
    auto *catchClauseNode =
        dyn_cast<ESTree::CatchClauseNode>(tryStmt->_handler);

    // Catch takes a exception variable, hence we need to create a new
    // scope for it.
    NameTableScopeTy newScope(nameTable_);

    Builder.setLocation(tryStmt->_handler->getDebugLoc());
    prepareCatch(catchClauseNode->_param);

    genStatement(catchClauseNode->_body);

    Builder.setLocation(SourceErrorManager::convertEndToLocation(
        tryStmt->_handler->getSourceRange()));
    Builder.createBranchInst(continueBlock);
  } else {
    // A finally block catches the exception and rethrows is.
    Builder.setLocation(tryStmt->_finalizer->getDebugLoc());
    auto *catchReg = Builder.createCatchInst();

    genStatement(tryStmt->_finalizer);

    Builder.setLocation(SourceErrorManager::convertEndToLocation(
        tryStmt->_finalizer->getSourceRange()));
    Builder.createThrowInst(catchReg);
  }

  // Finally transition to continue block.
  Builder.setInsertionBlock(continueBlock);
}

CatchInst *ESTreeIRGen::prepareCatch(ESTree::NodePtr catchParam) {
  auto catchInst = Builder.createCatchInst();

  auto catchVariableName =
      getNameFieldFromID(dyn_cast<ESTree::IdentifierNode>(catchParam));

  // Generate a unique catch variable name and use this name for IRGen purpose
  // only. The variable lookup in the catch clause will continue to be done
  // using the declared name.
  auto uniquedCatchVariableName =
      genAnonymousLabelName(catchVariableName.str());

  auto errorVar = Builder.createVariable(
      curFunction()->function->getFunctionScope(), uniquedCatchVariableName);

  /// Insert the synthesized variable into the function name table, so it can
  /// be looked up internally.
  nameTable_.insertIntoScope(
      &curFunction()->scope, errorVar->getName(), errorVar);

  // Alias the lexical name to the synthesized variable.
  nameTable_.insert(catchVariableName, errorVar);

  emitStore(Builder, catchInst, errorVar);
  return catchInst;
}

void ESTreeIRGen::genFinallyBeforeControlChange(
    SurroundingTry *sourceTry,
    SurroundingTry *targetTry) {
  // We walk the nested try statements starting from the source, until we reach
  // the target, generating the finally statements on the way.
  for (; sourceTry != targetTry; sourceTry = sourceTry->outer) {
    assert(sourceTry && "invalid try chain");

    // Emit an end of the try statement.
    auto *tryEndBlock = Builder.createBasicBlock(curFunction()->function);
    Builder.createBranchInst(tryEndBlock);
    Builder.setInsertionBlock(tryEndBlock);

    // Make sure we use the correct debug location for tryEndInst.
    if (sourceTry->finalizer) {
      hermes::IRBuilder::ScopedLocationChange slc(
          Builder, sourceTry->finalizer->getDebugLoc());
      Builder.createTryEndInst();
    } else {
      Builder.createTryEndInst();
    }

    if (sourceTry->finalizer) {
      // Recreate the state of the try stack on entrance to the finally block.
      llvm::SaveAndRestore<SurroundingTry *> sr{curFunction()->surroundingTry,
                                                sourceTry->outer};
      genStatement(sourceTry->finalizer);
    }
  }
}

} // namespace irgen
} // namespace hermes
