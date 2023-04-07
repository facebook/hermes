/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "frameloadstoreopts"

#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Support/Statistic.h"

#include "llvh/Support/Debug.h"

namespace hermes {
namespace {

/// \returns the single initializer if the variable \p V is initializes once
/// (in the lexical scope that it belongs to).
static bool getSingleInitializer(Variable *V) {
  StoreFrameInst *singleStore = nullptr;

  for (auto *U : V->getUsers()) {
    if (auto *S = llvh::dyn_cast<StoreFrameInst>(U)) {
      // This is not the first store.
      if (singleStore)
        return false;

      // Initialization happens not in the lexical scope.
      if (S->getParent()->getParent() != V->getParent()->getFunction())
        return false;

      singleStore = S;
    }
  }

  return singleStore;
}

class CapturedVariables {
 public:
  llvh::DenseSet<Variable *> loads;
  llvh::DenseSet<Variable *> stores;
};

/// Add the variables owned by \p src that the function \p F is capturing into
/// \p cv. Notice that the function F may have sub-closures that capture
/// variables. This method does a recursive scan and collects all captured
/// variables.
void collectCapturedVariables(
    CapturedVariables &cv,
    Function *F,
    Function *src) {
  assert(F != src && "Cannot collect captured variables from src itself.");
  // For all instructions in the function:
  for (auto &BB : *F) {
    for (auto &instIter : BB) {
      Instruction *II = &instIter;

      // Recursively check capturing functions by inspecting the created
      // closure.
      if (auto *CF = llvh::dyn_cast<BaseCreateLexicalChildInst>(II)) {
        collectCapturedVariables(cv, CF->getFunctionCode(), src);
        continue;
      }

      if (auto *LF = llvh::dyn_cast<LoadFrameInst>(II)) {
        Variable *V = LF->getLoadVariable();
        if (V->getParent()->getFunction() == src) {
          cv.loads.insert(V);
        }
      }

      if (auto *SF = llvh::dyn_cast<StoreFrameInst>(II)) {
        auto *V = SF->getVariable();
        if (V->getParent()->getFunction() == src) {
          cv.stores.insert(V);
        }
      }
    }
  }
}

bool eliminateLoads(BasicBlock *BB) {
  Function *F = BB->getParent();

  // In the entry block, we can perform more precise analysis by tracking
  // exactly when variables owned by F are actually captured. Create an empty
  // CapturedVariables that will be incrementally updated as we progress through
  // the entry block. This is set to None in all subsequent blocks.
  // TODO: Factor out analysis of owned variables, and analyse them across the
  // entire function.
  llvh::Optional<CapturedVariables> entryCV;
  if (BB == &*F->begin())
    entryCV.emplace();

  // Map from a Variable to its current known value that can be forwarded to a
  // load. If no entry exists, the value is unknown and must be loaded from the
  // frame.
  llvh::DenseMap<Variable *, Value *> knownFrameValues;

  /// A list of variables that are known to stay constant during the lifetime
  /// of the current function.
  llvh::DenseMap<Variable *, Value *> constFrameValues;

  IRBuilder::InstructionDestroyer destroyer;

  bool changed = false;

  for (auto &it : *BB) {
    Instruction *II = &it;
    if (auto *SF = llvh::dyn_cast<StoreFrameInst>(II)) {
      Variable *var = SF->getVariable();

      // Record the value stored to the frame:
      knownFrameValues[var] = SF->getValue();
      continue;
    }

    // Try to replace the LoadFrame with a recently saved value.
    if (auto *LF = llvh::dyn_cast<LoadFrameInst>(II)) {
      Variable *dest = LF->getLoadVariable();

      // If this variable is known to be constant during the lifetime of the
      // function then use a previous load.
      auto constEntry = constFrameValues.find(dest);
      if (constEntry != constFrameValues.end() &&
          dest->getParent()->getFunction() != LF->getParent()->getParent()) {
        // Replace all uses of the load with the recently stored value.
        LF->replaceAllUsesWith(constEntry->second);

        // We have no use of this load now. Remove it.
        destroyer.add(LF);
        changed = true;
        continue;
      }

      // The first time we load from a constant variable we need to save the
      // content we are loading.
      if (getSingleInitializer(dest)) {
        constFrameValues[dest] = LF;
      }

      // Check if we already have a known value for the load. If we do, use it,
      // otherwise, populate it.
      auto [it, first] = knownFrameValues.try_emplace(dest, LF);
      if (first)
        continue;

      // Replace all uses of the load with the known value.
      LF->replaceAllUsesWith(it->second);

      // We have no use of this load now. Remove it.
      destroyer.add(LF);
      changed = true;
      continue;
    }

    if (auto *CF = llvh::dyn_cast<BaseCreateLexicalChildInst>(II)) {
      // Collect the captured variables.
      if (entryCV) {
        collectCapturedVariables(*entryCV, CF->getFunctionCode(), F);
      }
      continue;
    }

    // Invalidate the variable storage if the instruction may execute capturing
    // stores that write the variable.
    if (II->mayExecute()) {
      if (entryCV) {
        for (auto it = knownFrameValues.begin(); it != knownFrameValues.end();
             it++) {
          bool ownedVar = it->first->getParent()->getFunction() == F;

          // If there are any captured stores of the variable, the value may be
          // updated, so preceding stores cannot be propagated.
          if (!ownedVar || entryCV->stores.count(it->first))
            knownFrameValues.erase(it);
        }
      } else {
        knownFrameValues.clear();
      }
    }
  }

  return changed;
}

bool eliminateStores(BasicBlock *BB) {
  Function *F = BB->getParent();

  // See comment in eliminateLoads above.
  llvh::Optional<CapturedVariables> entryCV;
  if (BB == &*F->begin())
    entryCV.emplace();

  // Map from a Variable to the last store to it that we have not found to be
  // observable yet. If an entry exists when a subsequent store is
  // encountered, the entry's store is not observable and may be eliminated.
  llvh::DenseMap<Variable *, StoreFrameInst *> prevStoreFrame;

  // Deletes instructions when we leave the function.
  IRBuilder::InstructionDestroyer destroyer;

  bool changed = false;

  for (auto &it : *BB) {
    Instruction *II = &it;

    // Try to delete the previous store based on the current store.
    if (auto *SF = llvh::dyn_cast<StoreFrameInst>(II)) {
      auto *V = SF->getVariable();

      auto [it, first] = prevStoreFrame.try_emplace(V, SF);

      if (!first) {
        // Found store-after-store. Mark the previous store for deletion.
        destroyer.add(it->second);
        changed = true;
        it->second = SF;
      }
      continue;
    }

    // If we are reading from the variable, the last known store cannot be
    // eliminated.
    if (auto *LF = llvh::dyn_cast<LoadFrameInst>(II)) {
      auto *V = LF->getLoadVariable();
      prevStoreFrame.erase(V);
      continue;
    }

    // Invalidate the store frame storage if the instruction may execute
    // capturing loads that observe this store.
    if (II->mayExecute()) {
      // In no-capture mode the local variables are preserved because they have
      // not been captured. This means that we only need to invalidate the
      // variables that don't belong to this function.
      if (entryCV) {
        // Erase all non-local variables.
        for (auto it = prevStoreFrame.begin(); it != prevStoreFrame.end();
             it++) {
          bool ownedVar = it->first->getParent()->getFunction() == F;
          if (!ownedVar || entryCV->loads.count(it->first)) {
            prevStoreFrame.erase(it);
          }
        }
      } else {
        // Invalidate all variables.
        prevStoreFrame.clear();
      }
    }

    if (auto *CF = llvh::dyn_cast<BaseCreateLexicalChildInst>(II)) {
      // Collect the captured variables.
      if (entryCV) {
        collectCapturedVariables(*entryCV, CF->getFunctionCode(), F);
      }
    }
  }

  return changed;
}

bool runFrameLoadStoreOpts(Module *M) {
  bool changed = false;
  for (auto &F : *M) {
    for (auto &BB : F) {
      changed |= eliminateLoads(&BB);
      changed |= eliminateStores(&BB);
    }
  }
  return changed;
}

} // namespace

Pass *createFrameLoadStoreOpts() {
  class ThisPass : public ModulePass {
   public:
    explicit ThisPass() : ModulePass("FrameLoadStoreOpts") {}
    ~ThisPass() override = default;

    bool runOnModule(Module *M) override {
      return runFrameLoadStoreOpts(M);
    }
  };
  return new ThisPass();
}

} // namespace hermes
#undef DEBUG_TYPE
