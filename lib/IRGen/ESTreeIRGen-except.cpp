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
  LLVM_DEBUG(llvh::dbgs() << "IRGen 'try' statement\n");

  // try-catch-finally statements must have been transformed by the validator
  // into two nested try statements with only "catch" or "finally" each.
  assert(
      (!tryStmt->_handler || !tryStmt->_finalizer) &&
      "Try statement can't have both catch and finally");

  auto *nextBlock = emitTryCatchScaffolding(
      nullptr,
      // emitBody.
      [this, tryStmt](BasicBlock *catchBlock) {
        llvh::Optional<SurroundingTry> thisTry;

        if (tryStmt->_finalizer) {
          thisTry.emplace(
              curFunction(),
              tryStmt,
              catchBlock,
              tryStmt->_finalizer->getDebugLoc(),
              [this](ESTree::Node *node, ControlFlowChange, BasicBlock *) {
                // This may be invoked multiple times if there are multiple
                // returns/breaks.
                FunctionContext::AllowRecompileRAII allowRecompile(
                    *curFunction());
                genStatement(cast<ESTree::TryStatementNode>(node)->_finalizer);
              });
        } else {
          thisTry.emplace(curFunction(), tryStmt, catchBlock);
        }

        genStatement(tryStmt->_block);

        Builder.setLocation(
            SourceErrorManager::convertEndToLocation(
                tryStmt->_block->getSourceRange()));
      },
      // emitNormalCleanup.
      [this, tryStmt]() {
        if (tryStmt->_finalizer) {
          // We may be recompiling the finally block if there are returns/breaks
          // in the body.
          FunctionContext::AllowRecompileRAII allowRecompile(*curFunction());
          genStatement(tryStmt->_finalizer);
          Builder.setLocation(
              SourceErrorManager::convertEndToLocation(
                  tryStmt->_finalizer->getSourceRange()));
        }
      },
      // emitHandler.
      [this, tryStmt](BasicBlock *nextBlock) {
        // If we have a catch block.
        if (tryStmt->_handler) {
          auto *catchClauseNode =
              llvh::dyn_cast<ESTree::CatchClauseNode>(tryStmt->_handler);

          Builder.setLocation(tryStmt->_handler->getDebugLoc());
          auto *catchInst = Builder.createCatchInst();
          emitScopeDeclarations(catchClauseNode->getScope());
          // Optional catch binding allows us to emit no extra code for the
          // catch.
          if (catchClauseNode->_param)
            createLRef(catchClauseNode->_param, true).emitStore(catchInst);

          genStatement(catchClauseNode->_body);

          Builder.setLocation(
              SourceErrorManager::convertEndToLocation(
                  tryStmt->_handler->getSourceRange()));
          Builder.createBranchInst(nextBlock);
        } else {
          // Indicate for debugging that we are compiling a "finally" handler a
          // second time.
          FunctionContext::AllowRecompileRAII allowRecompile(*curFunction());

          // A finally block catches the exception and rethrows is.
          Builder.setLocation(tryStmt->_finalizer->getDebugLoc());
          auto *catchReg = Builder.createCatchInst();

          genStatement(tryStmt->_finalizer);

          Builder.setLocation(
              SourceErrorManager::convertEndToLocation(
                  tryStmt->_finalizer->getSourceRange()));
          Builder.createThrowInst(catchReg);
        }
      });

  Builder.setInsertionBlock(nextBlock);
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
    auto *finalizerBlock = Builder.createBasicBlock(curFunction()->function);
    // Make sure we use the correct debug location for tryEndInst.
    if (sourceTry->tryEndLoc.isValid()) {
      hermes::IRBuilder::ScopedLocationChange slc(
          Builder, sourceTry->tryEndLoc);
      Builder.createTryEndInst(sourceTry->catchBlock, finalizerBlock);
    } else {
      Builder.createTryEndInst(sourceTry->catchBlock, finalizerBlock);
    }
    Builder.setInsertionBlock(finalizerBlock);

    if (sourceTry->genFinalizer) {
      // Recreate the state of the try stack on entrance to the finally block.
      llvh::SaveAndRestore<SurroundingTry *> trySR{
          curFunction()->surroundingTry, sourceTry->outer};
      // Restore the surrounding scope of the finally block.
      auto *existingScope = curFunction()->curScope();
      restoreScope(sourceTry->scope);
      sourceTry->genFinalizer(sourceTry->node, cfc, continueTarget);
      restoreScope(existingScope);
    }
  }
}

} // namespace irgen
} // namespace hermes
