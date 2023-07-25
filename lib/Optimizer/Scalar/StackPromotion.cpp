/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "stackpromotion"

#include "hermes/Optimizer/Scalar/StackPromotion.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Scalar/Utils.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/DenseSet.h"
#include "llvh/ADT/SetVector.h"
#include "llvh/Support/Debug.h"

using namespace hermes;
using llvh::dbgs;

STATISTIC(NumSP, "Number of stack allocations promoted");
STATISTIC(NumOpsPostponed, "Number of stack ops removed or postponed");

STATISTIC(NumConstProm, "Number of loads of single store constants promoted");
STATISTIC(NumInstProm, "Number of loads of single store instructions promoted");

/// \returns true if the variable is used outside of the current function.
static bool hasExternalUses(Variable *V) {
  auto *parent = V->getParent();

  // For all users:
  for (auto *U : V->getUsers()) {
    auto *I = cast<Instruction>(U);
    // Check if the user is inside the current function.
    if (I->getParent()->getParent() != parent->getFunction())
      return true;
  }

  // No external users.
  return false;
}

static void promoteConstVariable(
    DominanceInfo &DT,
    Variable *V,
    Function *func,
    Value *val) {
  IRBuilder builder(func->getParent());
  BasicBlock &entry = func->front();
  builder.setInsertionBlock(&entry);
  auto *stackVar = builder.createAllocStackInst(V->getName());
  stackVar->moveBefore(&*entry.begin());

  IRBuilder::InstructionDestroyer destroyer;

  bool needToKeepStores = false;

  for (auto *U : V->getUsers()) {
    if (auto *LF = llvh::dyn_cast<LoadFrameInst>(U)) {
      if (auto *P = llvh::dyn_cast<Parameter>(val)) {
        if (P->getParent() != LF->getParent()->getParent()) {
          needToKeepStores = true;
          continue;
        }

        LF->replaceAllUsesWith(val);
        destroyer.add(LF);
        NumConstProm++;
        continue;
      }

      if (llvh::isa<Literal>(val)) {
        LF->replaceAllUsesWith(val);
        destroyer.add(LF);
        NumConstProm++;
        continue;
      }

      if (auto *I = llvh::dyn_cast<Instruction>(val)) {
        // If the stored value dominates the loads then we can use the original
        // stored value instead of the load.
        if (I->getParent() == LF->getParent() && DT.properlyDominates(I, LF)) {
          LF->replaceAllUsesWith(I);
          destroyer.add(LF);
          NumInstProm++;
        }

        // We were not able to eliminate this load. This means that we need to
        // keep the store that initializes the variable.
        needToKeepStores = true;
      }
      continue;
    }

    if (llvh::isa<StoreFrameInst>(U))
      continue;

    llvm_unreachable("invalid user!");
  }

  /// Delete the variable store if we were able to eliminate all loads.
  if (!needToKeepStores) {
    for (auto *U : V->getUsers()) {
      if (llvh::isa<LoadFrameInst>(U))
        continue;

      if (auto *SF = llvh::dyn_cast<StoreFrameInst>(U))
        destroyer.add(SF);
    }
  }

  ++NumSP;
}

namespace {

/// Insert \p toInsert into \p current, returning true if changed.
bool unionSets(
    llvh::DenseSet<Variable *> &current,
    llvh::DenseSet<Variable *> &toInsert) {
  unsigned previousSize = current.size();
  current.insert(toInsert.begin(), toInsert.end());
  return previousSize != current.size();
}

/// Find variables from the Function \p base captured by Function \p current.
/// The variables will be stored in \p captured.
void collectCapturedVariables(
    llvh::DenseSet<Variable *> &captured,
    Function *base,
    Function *current) {
  for (auto &BB : *current) {
    for (auto &I : BB) {
      if (auto *create = llvh::dyn_cast<CreateFunctionInst>(&I)) {
        collectCapturedVariables(captured, base, create->getFunctionCode());
        continue;
      }

      Variable *var = nullptr;
      if (auto *load = llvh::dyn_cast<LoadFrameInst>(&I)) {
        var = load->getLoadVariable();
      } else if (auto *store = llvh::dyn_cast<StoreFrameInst>(&I)) {
        var = store->getVariable();
      }
      if (!var || var->getParent()->getFunction() != base)
        continue;

      captured.insert(var);
    }
  }
}

/// Find which captured variables in \p F need to be stored in the frame at
/// each BasicBlock. \p capturedVariableUsage will be a map from  to set of
/// required variables.
/// When CreateFunctionInst with zero users is encountered, it is removed, and
/// its Function is added to the set of potentially unreachable functions
/// \p maybeUnreachableFuncs. The Function in question has no users in the end,
/// it can be removed.
void determineCapturedVariableUsage(
    Function *F,
    llvh::DenseMap<BasicBlock *, llvh::DenseSet<Variable *>>
        &capturedVariableUsage,
    llvh::SetVector<Function *> &maybeUnreachableFuncs) {
  for (auto &BB : *F) {
    capturedVariableUsage.FindAndConstruct(&BB);
  }

  llvh::DenseSet<BasicBlock *> toPropagate;
  IRBuilder::InstructionDestroyer destroyer{};
  for (auto &BB : *F) {
    for (auto &I : BB) {
      auto *create = llvh::dyn_cast<CreateFunctionInst>(&I);
      if (!create)
        continue;

      /// If the create instruction has no users, it can be deleted. The
      /// function itself becomes potentially unreachable.
      if (!create->hasUsers()) {
        destroyer.add(create);
        maybeUnreachableFuncs.insert(create->getFunctionCode());
        continue;
      }

      llvh::DenseSet<Variable *> variables;
      collectCapturedVariables(variables, F, create->getFunctionCode());

      // A block should be marked as using a captured frame variable if a
      // capturing function may be invoked as a result of that block.
      // We approximate this by just counting any use of the function object.
      for (auto *user : create->getUsers()) {
        auto *block = user->getParent();
        capturedVariableUsage[block].insert(variables.begin(), variables.end());
        toPropagate.insert(block);
      }
    }
  }

  // Iterate until all variable usage in any predecessor has been propagated.
  while (toPropagate.size()) {
    auto *BB = *toPropagate.begin();
    toPropagate.erase(BB);

    for (auto I = succ_begin(BB), E = succ_end(BB); I != E; ++I) {
      if (unionSets(capturedVariableUsage[*I], capturedVariableUsage[BB])) {
        toPropagate.insert(*I);
      }
    }
  }
}

/// The list of variables that should be stored between
/// blocks \p from and \p to.
struct StorePoint {
  BasicBlock *from{};
  BasicBlock *to{};
  llvh::SmallVector<Variable *, 2> variables{};

  StorePoint(BasicBlock *from, BasicBlock *to) : from(from), to(to) {}
};

/// Promote captured variables until they're actually needed.
// Here's an example of how it optimizes a captured variable:
//
// function normalize(arr) {
//   var max = 0;            // Store on stack instead of frame
//   for (i in arr) {
//     if (max < arr[i]) {   // Load from stack instead
//       max = arr[i];       // Store on stack instead
//     }
//   }
//   // Copy from stack to frame
//   arr.map(function(x) { return x/max; });
// }
bool promoteVariables(
    Function *F,
    llvh::SetVector<Function *> &maybeUnreachableFuncs) {
  bool changed = false;

  llvh::DenseMap<BasicBlock *, llvh::DenseSet<Variable *>>
      capturedVariableUsage;
  determineCapturedVariableUsage(
      F, capturedVariableUsage, maybeUnreachableFuncs);

  // Maps scope descriptors to the instructions creating them.
  llvh::DenseMap<ScopeDesc *, ScopeCreationInst *> scopeCreation;
  auto scopeForVariable = [&scopeCreation](Variable *var) {
    ScopeCreationInst *sci = scopeCreation[var->getParent()];
    assert(sci && "missing scope creation for accessing variable.");
    return sci;
  };

  F->forEachScope([F, &scopeCreation](ScopeDesc *currScopeDesc) {
    for (Value *U : currScopeDesc->getUsers()) {
      if (auto *sci = llvh::dyn_cast<ScopeCreationInst>(U)) {
        if (currScopeDesc == sci->getCreatedScopeDesc()) {
          assert(
              F == currScopeDesc->getFunction() &&
              "materializing foreign scope");
          auto res = scopeCreation.insert(std::make_pair(currScopeDesc, sci));
          assert(
              !res.second ||
              res.first->second == sci && "multi instructions creating scope.");
          (void)res;
        }
      }
    }
    (void)F;
  });

  // Find variables that are currently not optimal.
  llvh::DenseSet<Variable *> needsOptimizing;
  F->forEachScope([&](ScopeDesc *currScopeDesc) {
    for (auto *var : currScopeDesc->getVariables()) {
      if (!hasExternalUses(var)) {
        // This variable isn't needed at all, it should be purely on the stack.
        needsOptimizing.insert(var);
        continue;
      }

      // Look for loads/stores in the current function that are not needed.
      for (auto *use : var->getUsers()) {
        if (use->getParent()->getParent() != F)
          continue;
        if (capturedVariableUsage[use->getParent()].count(var))
          continue;

        // This use is not needed yet. We can optimize it.
        needsOptimizing.insert(var);
        break;
      }
    }
  });

  // Replace all variables with stack alloc operations in blocks that don't
  // need real variables. For uncaptured variables, this replaces all uses.
  IRBuilder builder(F);
  llvh::DenseMap<Variable *, AllocStackInst *> stackMap;

  F->forEachScope([&](ScopeDesc *currScopeDesc) {
    for (auto *var : currScopeDesc->getVariables()) {
      if (!needsOptimizing.count(var))
        continue;
      if (!var->getNumUsers())
        continue;

      builder.setInsertionPoint(&*F->begin()->begin());
      auto *stackVar = builder.createAllocStackInst(var->getName());
      builder.createStoreStackInst(builder.getLiteralUndefined(), stackVar);
      stackMap[var] = stackVar;
      changed = true;

      {
        IRBuilder::InstructionDestroyer destroyer;
        for (auto *U : var->getUsers()) {
          // This instruction is not part of this function.
          if (U->getParent()->getParent() != F)
            continue;

          // This block needs the variable in the frame.
          if (capturedVariableUsage[U->getParent()].count(var))
            continue;

          NumOpsPostponed++;

          if (auto *LF = llvh::dyn_cast<LoadFrameInst>(U)) {
            builder.setInsertionPoint(LF);
            auto *LS = builder.createLoadStackInst(stackVar);
            LS->moveBefore(LF);
            LF->replaceAllUsesWith(LS);
            destroyer.add(LF);
            continue;
          }
          if (auto *SF = llvh::dyn_cast<StoreFrameInst>(U)) {
            builder.setInsertionPoint(SF);
            auto *SS = builder.createStoreStackInst(SF->getValue(), stackVar);
            SS->moveBefore(SF);
            destroyer.add(SF);
            continue;
          }
          llvm_unreachable("Invalid user");
        }
      }
      // We were able to remove all uses.
      if (!var->getNumUsers()) {
        ++NumSP;
      }
    }
  });

  // For any block where all incoming arrows require the same stores,
  // insert them into the block itself.
  llvh::DenseSet<std::pair<BasicBlock *, Variable *>> alreadyProcessed;
  for (auto &BB : *F) {
    // No incoming arrows
    if (!pred_count(&BB))
      continue;

    llvh::DenseSet<Variable *> commons = capturedVariableUsage[&BB];
    for (auto *predecessor : predecessors(&BB)) {
      llvh::SmallVector<Variable *, 4> toErase;
      for (auto *var : commons) {
        if (needsOptimizing.count(var) &&
            !capturedVariableUsage[predecessor].count(var))
          continue;
        toErase.push_back(var);
      }
      for (auto *var : toErase) {
        commons.erase(var);
      }
    }
    if (!commons.size())
      continue;

    auto insertionPoint = BB.begin();
    while (llvh::isa<TryEndInst>(*insertionPoint) ||
           llvh::isa<CatchInst>(*insertionPoint) ||
           llvh::isa<PhiInst>(*insertionPoint)) {
      insertionPoint++;
    }
    builder.setInsertionPoint(&*insertionPoint);

    // Loop over the set of common variables, but in a deterministic order.
    F->forEachScope([&](ScopeDesc *currScopeDesc) {
      for (auto *var : currScopeDesc->getVariables()) {
        if (!commons.count(var))
          continue;
        // It could have been the case that this block both initializes and
        // requires the variable. This load would then read an uninitialized
        // value, which is illegal.
        // To avoid this case, the variable is always initialized to undefined
        // so that it's merely unnecessary, and will get optimized away.
        auto *value = builder.createLoadStackInst(stackMap[var]);
        builder.createStoreFrameInst(value, var, scopeForVariable(var));
        alreadyProcessed.insert(std::pair<BasicBlock *, Variable *>(&BB, var));
        changed = true;
      }
    });
  }

  // Find each block transition where one or more variable captures
  // go from inactive to active.
  llvh::SmallVector<StorePoint, 4> storePoints;
  for (auto &BB : *F) {
    TerminatorInst *terminator = BB.getTerminator();
    auto &usedHere = capturedVariableUsage[&BB];

    // Need to store each successor of the block, in order to ensure we don't
    // accidentally consider the same arrow twice:
    //   A corner case that occurs under certain conditions in
    //   empty cases in switch statements
    llvh::SmallPtrSet<BasicBlock *, 16> storeSuccessors;
    for (int i = 0, e = terminator->getNumSuccessors(); i < e; i++) {
      auto *next = terminator->getSuccessor(i);
      // Only proceed if successor not seen before
      if (!storeSuccessors.insert(next).second) {
        continue;
      }
      auto &usedNext = capturedVariableUsage[next];
      StorePoint *point = nullptr;

      F->forEachScope([&](ScopeDesc *currScopeDesc) {
        for (auto *var : currScopeDesc->getVariables()) {
          if (!needsOptimizing.count(var))
            continue;
          // We only care about transitions, i.e. variables
          // that are usedNext but not usedHere.
          if (usedHere.count(var) || !usedNext.count(var))
            continue;
          // We already incorporated this into the block itself
          if (alreadyProcessed.count(
                  std::pair<BasicBlock *, Variable *>(next, var)))
            continue;

          if (!point) {
            storePoints.push_back(StorePoint(&BB, next));
            point = &storePoints.back();
          }
          point->variables.push_back(var);
        }
      });
    }
  }

  // Copy from the stack to the frame at those points.
  for (auto &point : storePoints) {
    splitCriticalEdge(&builder, point.from, point.to);
    for (auto *var : point.variables) {
      auto *value = builder.createLoadStackInst(stackMap[var]);
      builder.createStoreFrameInst(value, var, scopeForVariable(var));
      changed = true;
    }
  }
  return changed;
}

bool runOnFunction(
    Function *F,
    llvh::SetVector<Function *> &maybeUnreachableFuncs) {
  bool changed = false;

  LLVM_DEBUG(
      dbgs() << "Promoting variables in " << F->getInternalNameStr() << "\n");
  DominanceInfo DT(F);

  F->forEachScope([&](ScopeDesc *currScopeDesc) {
    for (auto *V : currScopeDesc->getVariables()) {
      // Promote constant variables.
      if (Value *val = isStoreOnceVariable(V)) {
        promoteConstVariable(DT, V, F, val);
      }
    }
  });
  promoteVariables(F, maybeUnreachableFuncs);

  // Now that we've promoted some variables, remove the unused variables from
  // the list and destroy them.
  F->forEachScope([&](ScopeDesc *currScopeDesc) {
    auto &vars = currScopeDesc->getMutableVariables();
    vars.erase(
        std::remove_if(
            vars.begin(),
            vars.end(),
            [](Variable *V) {
              if (V->getNumUsers())
                return false;
              Value::destroy(V);
              return true;
            }),
        vars.end());
  });

  return changed;
}

} // namespace

bool StackPromotion::runOnModule(Module *M) {
  bool changed = false;
  // Use a SetVector instead of just a set for efficient iteration.
  llvh::SetVector<Function *> maybeUnreachableFuncs{};
  for (Function &func : *M) {
    Function *F = &func;
    // Skip working on functions that have become unreachable.
    if (maybeUnreachableFuncs.count(F) && !F->hasUsers())
      continue;
    changed |= runOnFunction(F, maybeUnreachableFuncs);
  }

  // Destroy all functions that became unreachable by examining all potentially
  // unreachable functions and checking whether they have zero users.
  //
  // Unreachable functions should be destroyed, because stack promotion operates
  // as if they already don't exist - specifically, it eliminates stores to
  // frame variables that may be accessed by unreachable functions - which could
  // make the IR inside the unreachable functions technically incorrect, even if
  // it is never executed.
  while (!maybeUnreachableFuncs.empty()) {
    Function *F = maybeUnreachableFuncs.pop_back_val();
    if (F->hasUsers())
      continue;

    // All functions created by this unreachable function are now potentially
    // unreachable too.
    for (auto &BB : *F) {
      for (auto &I : BB) {
        if (auto *CFI = llvh::dyn_cast<CreateFunctionInst>(&I))
          maybeUnreachableFuncs.insert(CFI->getFunctionCode());
      }
    }
    F->eraseFromParentNoDestroy();
    Value::destroy(F);
  }

  return changed;
}

std::unique_ptr<Pass> hermes::createStackPromotion() {
  return std::make_unique<StackPromotion>();
}

#undef DEBUG_TYPE
