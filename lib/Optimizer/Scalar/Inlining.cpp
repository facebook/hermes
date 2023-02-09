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

#include "llvh/ADT/DenseSet.h"
#include "llvh/ADT/SetVector.h"
#include "llvh/ADT/SmallVector.h"
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

/// \return a list of known callsites of \p F based on its users.
/// It is possible that \p F has additional unknown callsites,
/// read the `allCallsitesKnown` attribute to check that.
static llvh::SmallVector<BaseCallInst *, 2> getKnownCallsites(Function *F) {
  llvh::SmallVector<BaseCallInst *, 2> result{};
  for (Instruction *user : F->getUsers()) {
    if (auto *call = llvh::dyn_cast<BaseCallInst>(user)) {
      assert(
          call->getTarget() == F &&
          "invalid usage of Function as operand of BaseCallInst");
      result.push_back(call);
    }
  }
  return result;
}

/// Iterates all instructions and extracts all populated \c target operands
/// from call instructions.
/// The callees of a function are not stored explicitly outside the insts
/// themselves.
/// \return a list of known callees of \p F.
static llvh::SmallVector<Function *, 2> getKnownCallees(Function *F) {
  // Use a SetVector to avoid duplicate entries.
  // Return the vector to ensure deterministic iteration.
  llvh::SetVector<Function *, llvh::SmallVector<Function *, 2>> result{};
  for (BasicBlock &BB : *F) {
    for (Instruction &I : BB) {
      if (auto *call = llvh::dyn_cast<BaseCallInst>(&I)) {
        if (auto *callee = llvh::dyn_cast<Function>(call->getTarget())) {
          result.insert(callee);
        }
      }
    }
  }
  return result.takeVector();
}

/// Make an order by which to visit functions for inlining.
/// Because the call graph isn't acyclic or connected, we can't necessarily
/// perfectly perform a topological sort, but we start at functions that
/// have calls from no other known functions to ensure they're not going to
/// be inlined anywhere, and once we've processed those,
/// visit all the other functions.
/// \return a list of Functions in a roughly leaf-to-root sorted manner.
static std::vector<Function *> orderFunctions(Module *M) {
  // Resultant ordering of functions.
  std::vector<Function *> order{};

  /// State to push onto the stack.
  class State {
   public:
    Function *func;
    /// All known callees of \c func.
    /// When empty, iteration is complete.
    llvh::SmallVector<Function *, 2> calledFunctions;

    explicit State(Function *func, llvh::SmallVector<Function *, 2> &&called)
        : func(func), calledFunctions(std::move(called)) {}

    State(const State &) = delete;
    State &operator=(const State &) = delete;

    State(State &&) = default;
  };

  llvh::SmallDenseSet<Function *> visited{};

  // Store the stack as a list of states to track whether the callees
  // of each function have already been emplaced onto the stack.
  // Store it outside `visitPostOrder` to avoid reallocating every call.
  llvh::SmallVector<State, 4> stack{};

  /// Run a post-order traversal of the function call graph starting at
  /// \p cur, where the edges are from functions to the functions that they
  /// are known to call.
  /// If \p cur has already been visited, do nothing.
  const auto visitPostOrder =
      [&visited, &order, &stack](Function *cur) -> void {
    if (!visited.insert(cur).second) {
      // Visited this function on a previous invocation of visitPostOrder.
      return;
    }

    assert(
        stack.empty() &&
        "stack must be empty by the end of every visitPostOrder call");

    // Begin with the callees of the current function.
    stack.emplace_back(cur, getKnownCallees(cur));

    do {
      while (!stack.back().calledFunctions.empty()) {
        Function *next = stack.back().calledFunctions.pop_back_val();
        if (visited.insert(next).second) {
          // Haven't visited `next` before, add its callees to the stack.
          stack.emplace_back(next, getKnownCallees(next));
        }
      }

      order.push_back(stack.back().func);
      stack.pop_back();
    } while (!stack.empty());
  };

  // Functions that shouldn't be initial starting points in the BFS.
  std::vector<Function *> functionsWithCallsites{};

  // Run the visitor from each starting point to account for a disconnected
  // graph, deferring functions with callsites until after functions without.
  for (Function &F : *M) {
    if (getKnownCallsites(&F).empty()) {
      visitPostOrder(&F);
    } else {
      functionsWithCallsites.push_back(&F);
    }
  }
  for (Function *F : functionsWithCallsites) {
    visitPostOrder(F);
  }

  assert(
      order.size() == M->getFunctionList().size() &&
      "Didn't order all functions");

  return order;
}

/// \return true if the function \p F satisfies the conditions for being
///   inlined.
static bool canBeInlined(Function *F, Function *intoFunction) {
  // If it's a recursive call, it can't be inlined.
  if (F == intoFunction) {
    LLVM_DEBUG(
        llvh::dbgs() << "Cannot inline function '" << F->getInternalNameStr()
                     << "' into itself\n");
    return false;
  }

  // If it has captured variables, it can't be inlined.
  if (!F->getFunctionScope()->getVariables().empty()) {
    LLVM_DEBUG(
        llvh::dbgs() << "Cannot inline function '" << F->getInternalNameStr()
                     << "': has captured variables\n");
    return false;
  }

  // If the functions have different strictness, we can't inline them, since
  // we don't have strict/non-strict version of instructions (TODO).
  if (F->isStrictMode() != intoFunction->isStrictMode()) {
    LLVM_DEBUG(
        llvh::dbgs() << "Cannot inline function '" << F->getInternalNameStr()
                     << "': strict mode mismatch\n");
    return false;
  }

  for (BasicBlock *oldBB : orderDFS(F)) {
    for (auto &I : *oldBB) {
      switch (I.getKind()) {
        case ValueKind::LIRGetThisNSInstKind:
        case ValueKind::CreateArgumentsInstKind:
        // TODO: we can't deal with changing the scope depth of functions yet.
        case ValueKind::CreateFunctionInstKind:
        case ValueKind::CreateGeneratorInstKind:
          // Fail.
          LLVM_DEBUG(
              llvh::dbgs() << "Cannot inline function '"
                           << F->getInternalNameStr()
                           << "': invalid instruction " << I.getKindStr()
                           << '\n');
          return false;
        case ValueKind::CallBuiltinInstKind:
          if (cast<CallBuiltinInst>(&I)->getBuiltinIndex() ==
              BuiltinMethod::HermesBuiltin_copyRestArgs) {
            LLVM_DEBUG(
                llvh::dbgs()
                << "Cannot inline function '" << F->getInternalNameStr()
                << "': copies rest args\n");
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

      if (llvh::isa<Instruction>(oldOp) || llvh::isa<Parameter>(oldOp) ||
          llvh::isa<BasicBlock>(oldOp)) {
        // Operands must already have been visited.
        newOp = operandMap[oldOp];
        assert(newOp && "operand not visited before instruction");
      } else if (
          llvh::isa<Label>(oldOp) || llvh::isa<Literal>(oldOp) ||
          llvh::isa<Function>(oldOp) || llvh::isa<Variable>(oldOp) ||
          llvh::isa<EmptySentinel>(oldOp)) {
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

  // Copy all instructions to the inlined function. Phi instructions are
  // treated differently and copied with empty operands.
  for (BasicBlock *oldBB : order) {
    BasicBlock *newBB = cast<BasicBlock>(operandMap[oldBB]);
    assert(newBB->empty() && "BB visited more than once");

    builder.setInsertionBlock(newBB);

    for (auto &I : *oldBB) {
      // LoadParamInst is translated to a direct usage of the argument.
      if (auto *LPI = llvh::dyn_cast<LoadParamInst>(&I)) {
        uint32_t index = LPI->getParam()->getIndexInParamList();
        operandMap[&I] = index < CI->getNumArguments()
            ? CI->getArgument(index)
            : builder.getLiteralUndefined();
        continue;
      }
      assert(!llvh::isa<LIRGetThisNSInst>(I) && "Not allowed during inlining");

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

  std::vector<Function *> functionOrder = orderFunctions(M);

  for (Function *FC : functionOrder) {
    LLVM_DEBUG(
        llvh::dbgs() << "Visiting function '" << FC->getInternalNameStr()
                     << "'\n");

    if (!FC->getAttributes().allCallsitesKnown) {
      LLVM_DEBUG(
          llvh::dbgs() << "Cannot inline function '" << FC->getInternalNameStr()
                       << "': has unknown callsites\n");
      continue;
    }

    llvh::SmallVector<BaseCallInst *, 2> callsites = getKnownCallsites(FC);

    if (callsites.size() != 1) {
      LLVM_DEBUG(
          llvh::dbgs() << "Cannot inline function '" << FC->getInternalNameStr()
                       << llvh::format(
                              "': has %u callsites (requires 1)\n",
                              callsites.size()));
      continue;
    }

    auto *CI = llvh::dyn_cast<CallInst>(*callsites.begin());
    if (!CI) {
      LLVM_DEBUG(
          llvh::dbgs() << "Cannot inline function '" << FC->getInternalNameStr()
                       << "': callsite is not a CallInst\n");
      continue;
    }

    Function *intoFunction = CI->getParent()->getParent();

    if (!canBeInlined(FC, intoFunction))
      continue;

    LLVM_DEBUG(llvh::dbgs()
                   << "Inlining function '" << FC->getInternalNameStr() << "' ";
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

  return changed;
}

Pass *createInlining() {
  return new Inlining();
}

} // namespace hermes
#undef DEBUG_TYPE
