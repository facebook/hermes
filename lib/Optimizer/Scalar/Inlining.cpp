/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "inline"
#include "hermes/Optimizer/Scalar/Inlining.h"

#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/Optimizer/Scalar/Utils.h"
#include "hermes/Support/Statistic.h"

#include "llvh/Support/Debug.h"

STATISTIC(NumInlinedCalls, "Number of inlined calls");

namespace hermes {

/// Generate a list of basic blocks in simple depth-first-search order.
/// Unreachable blocks are not included since we don't want to inline them.
static llvh::SmallVector<BasicBlock *, 4> orderDFS(Function *F) {
  llvh::SmallVector<BasicBlock *, 4> order{};
  llvh::SmallVector<BasicBlock *, 4> stack{};
  llvh::SmallDenseSet<BasicBlock *> visited{};

  stack.push_back(&*F->begin());
  while (!stack.empty()) {
    BasicBlock *BB = stack.back();
    stack.pop_back();
    if (!visited.insert(BB).second)
      continue;

    order.push_back(BB);

    for (auto *succ : successors(BB))
      stack.push_back(succ);
  }

  return order;
}

/// \return true if the function \p F satisfies the conditions for being
///   inlined.
static bool canBeInlined(Function *F, Function *intoFunction) {
  // If it has captured variables, it can't be inlined.
  if (!F->getFunctionScopeDesc()->getVariables().empty()) {
    return false;
  }

  // If it has nested scopes, it can't be inlined. This should be possible, but
  // given that hermes doesn't currently support multi-scope functions it is
  // impossible to test it.
  for (const ScopeDesc *inner : F->getFunctionScopeDesc()->getInnerScopes()) {
    if (inner->getFunction() == F) {
      return false;
    }
  }

  // If the functions have different strictness, we can't inline them, since
  // we don't have strict/non-strict version of instructions (TODO).
  if (F->isStrictMode() != intoFunction->isStrictMode())
    return false;

  for (BasicBlock *oldBB : orderDFS(F)) {
    for (auto &I : *oldBB) {
      switch (I.getKind()) {
        case ValueKind::CreateArgumentsInstKind:
        // TODO: we can't deal with changing the scope depth of functions yet.
        case ValueKind::CreateFunctionInstKind:
        case ValueKind::CreateGeneratorInstKind:
          // Fail.
          return false;
        case ValueKind::CallBuiltinInstKind:
          if (cast<CallBuiltinInst>(&I)->getBuiltinIndex() ==
              BuiltinMethod::HermesBuiltin_copyRestArgs) {
            return false;
          }
          break;
        default:
          break;
      }
    }
  }

  return true;
}

/// Inline a function into the current insertion point, which must be at the
/// end of a basic block because a branch will be inserted.
/// \param F the function to inline
/// \param CI the call instruction being replaced. Note that this call
///   will not actually replace it.
/// \param nextBlock the block to branch after the inlining
/// \return the return value of the inlined function
static Value *inlineFunction(
    IRBuilder &builder,
    Function *F,
    CallInst *CI,
    BasicBlock *nextBlock) {
  Function *intoFunction = builder.getInsertionBlock()->getParent();

  // Map from operands in the original function to the operands in the inlined
  // copy.
  llvh::DenseMap<Value *, Value *> operandMap{};

  // We build the operand list here for every instruction.
  llvh::SmallVector<Value *, 8> translatedOperands{};

  // We collect all phi-s during the first pass in this set.
  llvh::SmallVector<PhiInst *, 4> phis{};

  // Create a return block.
  BasicBlock *returnBlock = builder.createBasicBlock(intoFunction);
  Value *returnValue = nullptr;
  // In cases where we have exactly one return value, this is the block we are
  // returning from.
  BasicBlock *returnFrom = nullptr;

  // The statement index offset that we will add to every inlined instruction.
  const uint32_t statementIndexOffset = intoFunction->getStatementCount()
      ? *intoFunction->getStatementCount()
      : 0;
  // Increment the statement count of the function we are inlining into.
  const uint32_t inlineStatementCount =
      F->getStatementCount() ? *F->getStatementCount() : 0;
  intoFunction->setStatementCount(statementIndexOffset + inlineStatementCount);

  // Map the parameters.

  Value *thisParam;
  if (!F->isStrictMode()) {
    // If the callee is non-strict, we need to coerce "this" to an object.
    thisParam = builder.createCoerceThisNSInst(CI->getThis());
  } else {
    thisParam = CI->getThis();
  }

  operandMap[F->getThisParameter()] = thisParam;
  {
    unsigned argIndex = 1;
    for (Parameter *param : F->getParameters()) {
      operandMap[param] = argIndex < CI->getNumArguments()
          ? CI->getArgument(argIndex)
          : cast<Value>(builder.getLiteralUndefined());
      ++argIndex;
    }
  }

  auto order = orderDFS(F);

  // Map the basic blocks.
  for (BasicBlock *oldBB : order) {
    operandMap[oldBB] = builder.createBasicBlock(intoFunction);
  }

  // Branch to the entry block.
  builder.createBranchInst(cast<BasicBlock>(operandMap[order[0]]));

  // Translate all operands of the passed instruction and store them into
  // translatedOperands[].
  auto translateOperands = [&](Instruction *I) {
    translatedOperands.clear();

    for (unsigned i = 0, e = I->getNumOperands(); i != e; ++i) {
      Value *oldOp = I->getOperand(i);
      Value *newOp = nullptr;

      if (auto *sc = llvh::dyn_cast<ScopeDesc>(oldOp)) {
        ScopeDesc *newSC = intoFunction->getFunctionScopeDesc();
        newOp = newSC;

        for (const ScopeDesc *inner : sc->getInnerScopes()) {
          assert(
              inner->getFunction() != F && "canBeInlined should have said no!");
          (void)inner;
        }

        assert(
            sc->getVariables().empty() && "canBeInlined should have said no!");
      } else if (
          llvh::isa<Instruction>(oldOp) || llvh::isa<Parameter>(oldOp) ||
          llvh::isa<BasicBlock>(oldOp)) {
        // Operands must already have been visited.
        newOp = operandMap[oldOp];
        assert(newOp && "operand not visited before instruction");
      } else if (
          llvh::isa<Label>(oldOp) || llvh::isa<Literal>(oldOp) ||
          llvh::isa<Variable>(oldOp) || llvh::isa<EmptySentinel>(oldOp)) {
        // Labels, literals and variables are unchanged.
        newOp = oldOp;
      } else {
        llvh::errs() << "INVALID OPERAND FOR : " << I->getKindStr() << '\n';
        llvh::errs() << "INVALID OPERAND     : " << oldOp->getKindStr() << '\n';
        llvm_unreachable("unexpected operand kind");
      }

      translatedOperands.push_back(newOp);
    }
  };

  CreateScopeInst *intoFunctionScopeCreation{};

  // Returns the instruction materializing intoFunction's scope, creating it if
  // needed.
  auto getIntoFunctionScopeCreation = [&]() {
    // Simple case: this function has already been called, so just return the
    // cached value.
    if (intoFunctionScopeCreation) {
      return intoFunctionScopeCreation;
    }

    // This is the first time this function is invoked, so iterate over the
    // instructions in intoFunction trying to find its scope's creation inst.
    for (BasicBlock &BB : *intoFunction) {
      for (Instruction &I : BB) {
        if (auto *csi = llvh::dyn_cast<CreateScopeInst>(&I)) {
          assert(
              csi->getCreatedScopeDesc() ==
                  intoFunction->getFunctionScopeDesc() &&
              "CreateScopeInst creating the wrong scope");
          intoFunctionScopeCreation = csi;
          return csi;
        }
      }
    }

    // intoFunction's didn't have its scope materialized, so add that
    // instruction.
    IRBuilder::SaveRestore sr{builder};
    builder.setInsertionPoint(&*intoFunction->begin()->begin());
    intoFunctionScopeCreation =
        builder.createCreateScopeInst(intoFunction->getFunctionScopeDesc());
    return intoFunctionScopeCreation;
  };

  // Copy all instructions to the inlined function. Phi instructions are
  // treated differently and copied with empty operands.
  for (BasicBlock *oldBB : order) {
    BasicBlock *newBB = cast<BasicBlock>(operandMap[oldBB]);
    assert(newBB->empty() && "BB visited more than once");

    builder.setInsertionBlock(newBB);

    for (auto &I : *oldBB) {
      // Translate the operands.

      if (auto *phi = llvh::dyn_cast<PhiInst>(&I)) {
        // We cannot translate phi operands yet because the instruction is not
        // dominated by its operands (unlike all others). So, use empty
        // operands and save the Phi for later.
        translatedOperands.clear();
        translatedOperands.resize(phi->getNumOperands(), nullptr);
        phis.push_back(phi);
      } else {
        translateOperands(&I);
      }

      Instruction *newInst;

      if (llvh::isa<ReturnInst>(I)) {
        // Handle return by jumping to the return block and adjusting the phi.
        assert(
            translatedOperands.size() == 1 &&
            "ReturnInst expected to have 1 operand");
        newInst = builder.createBranchInst(returnBlock);
        builder.setInsertionBlock(returnBlock);

        if (!returnValue) {
          // if this is the first return value we have seen, we don't need a
          // phi instruction. Just save it.
          returnValue = translatedOperands[0];
          returnFrom = newBB;
        } else {
          if (returnBlock->empty()) {
            // We now have two return values, so create a phi instruction in
            // the return block.
            auto *phi = builder.createPhiInst();
            phi->addEntry(returnValue, returnFrom);
            returnValue = phi;
            returnFrom = nullptr;
          }

          // Append to the existing phi.
          cast<PhiInst>(returnValue)->addEntry(translatedOperands[0], newBB);
        }
      } else if (auto *csi = llvh::dyn_cast<CreateScopeInst>(&I)) {
        assert(
            csi->getCreatedScopeDesc() == F->getFunctionScopeDesc() &&
            "CreateScopeInst creating the wrong scope");
        newInst = getIntoFunctionScopeCreation();
        // TODO: ensure newInst dominates the builder's current position.
      } else {
        newInst = builder.cloneInst(&I, translatedOperands);
      }

      operandMap[&I] = newInst;

      // Update the statement index so it is relative to the new function.
      newInst->setStatementIndex(
          newInst->getStatementIndex() + statementIndexOffset);
    }
  }

  // Finish the job by translating the operands of the phi instructions we
  // saved earlier.
  for (PhiInst *oldPhi : phis) {
    translateOperands(oldPhi);

    auto *newPhi = cast<PhiInst>(operandMap[oldPhi]);
    for (unsigned i = 0, e = translatedOperands.size(); i != e; ++i)
      newPhi->setOperand(translatedOperands[i], i);
  }

  builder.setInsertionBlock(returnBlock);
  builder.createBranchInst(nextBlock);

  return returnValue ? returnValue : cast<Value>(builder.getLiteralUndefined());
}

bool Inlining::runOnModule(Module *M) {
  if (!M->getContext().getOptimizationSettings().inlining)
    return false;

  bool changed = false;

  for (Function &F : *M) {
    for (Instruction *I : F.getUsers()) {
      auto *CFI = llvh::dyn_cast<CreateFunctionInst>(I);
      if (!CFI)
        continue;

      // Check if the function is used only once directly by a CallInst.
      // We can't use getCallSites() (yet) because it also considers constructor
      // calls as well usages through environment variables.

      if (!CFI->hasOneUser() ||
          CFI->getUsers()[0]->getKind() != ValueKind::CallInstKind) {
        continue;
      }
      auto *CI = cast<CallInst>(CFI->getUsers()[0]);
      if (!isDirectCallee(CFI, CI))
        continue;

      Function *intoFunction = CI->getParent()->getParent();

      auto *FC = CFI->getFunctionCode();
      if (!canBeInlined(FC, intoFunction))
        continue;

      LLVM_DEBUG(llvh::dbgs() << "Inlining function '"
                              << FC->getInternalNameStr() << "' ";
                 FC->getContext().getSourceErrorManager().dumpCoords(
                     llvh::dbgs(), FC->getSourceRange().Start);
                 llvh::dbgs() << " into function '"
                              << intoFunction->getInternalNameStr() << "' ";
                 FC->getContext().getSourceErrorManager().dumpCoords(
                     llvh::dbgs(), intoFunction->getSourceRange().Start);
                 llvh::dbgs() << "\n";);

      IRBuilder builder(M);

      // Split the block in two and move all instructions following the call
      // to the new block.
      BasicBlock *nextBlock = builder.createBasicBlock(intoFunction);
      builder.setInsertionBlock(nextBlock);

      // Move the rest of the instructions.
      auto it = CI->getIterator();
      ++it; // Skip over the call.
      auto e = CI->getParent()->end();
      while (it != e)
        builder.transferInstructionToCurrentBlock(&*it++);

      // Perform the inlining.
      builder.setInsertionPointAfter(CI);

      auto *returnValue = inlineFunction(builder, FC, CI, nextBlock);
      CI->replaceAllUsesWith(returnValue);
      CI->eraseFromParent();

      ++NumInlinedCalls;
      changed = true;
    }
  }

  return changed;
}

Pass *createInlining() {
  return new Inlining();
}

} // namespace hermes
#undef DEBUG_TYPE
