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

/// This pass tries to deduplicate loads and delete unobservable stores to frame
/// variables.
///
/// For loads, the key idea is that if there are no instructions that may write
/// a variable between two loads or a store and a load to it, the second load
/// may be eliminated.
/// For stores, the idea is that if there are no instructions that may read a
/// variable between two stores, then the first store is redundant.
///
/// In both cases, the analysis is refined to allow instructions with
/// side-effects in the middle, by checking whether a variable has capturing
/// loads/stores that may manipulate it from such an instruction. For example,
/// this means that we can deduplicate loads across a function call, as long as
/// we know that there are no capturing stores.
///
/// This analysis is further refined in the entry basic block of a function. We
/// know that none of the variables owned by the function are captured at the
/// start of it, so as we iterate through the entry block, we incrementally
/// build information about which variables are captured. This allows us to
/// optimise variables before the closure that captures them is created.

namespace hermes {
namespace {

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
    for (auto &I : BB) {
      // Recursively check capturing functions by inspecting the created
      // closure.
      if (auto *CF = llvh::dyn_cast<BaseCreateLexicalChildInst>(&I)) {
        collectCapturedVariables(cv, CF->getFunctionCode(), src);
        continue;
      }

      if (auto *LF = llvh::dyn_cast<LoadFrameInst>(&I)) {
        if (LF->getLoadVariable()->getParent()->getFunction() == src)
          cv.loads.insert(LF->getLoadVariable());
        continue;
      }

      if (auto *SF = llvh::dyn_cast<StoreFrameInst>(&I)) {
        if (SF->getVariable()->getParent()->getFunction() == src)
          cv.stores.insert(SF->getVariable());
        continue;
      }
    }
  }
}

/// Attempts to replace loads from the frame in \p BB with values from a
/// previous load or store to the same variable. Uses capture information from
/// \p globalCV to determine whether intervening operations may store to the
/// variable and prevent forwarding.
bool eliminateLoads(BasicBlock *BB, const CapturedVariables &globalCV) {
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
  llvh::DenseMap<Variable *, Value *> knownValues;
  IRBuilder::InstructionDestroyer destroyer;

  bool changed = false;

  for (Instruction &I : *BB) {
    if (auto *SF = llvh::dyn_cast<StoreFrameInst>(&I)) {
      // Record the value stored to the frame:
      knownValues[SF->getVariable()] = SF->getValue();
      continue;
    }

    // Try to replace the LoadFrame with a recently saved value.
    if (auto *LF = llvh::dyn_cast<LoadFrameInst>(&I)) {
      // Check if we already have a known value for the load. If we do, use it,
      // otherwise, populate it.
      auto [it, first] = knownValues.try_emplace(LF->getLoadVariable(), LF);
      if (first)
        continue;

      // Replace all uses of the load with the known value.
      LF->replaceAllUsesWith(it->second);

      // We have no use of this load now. Remove it.
      destroyer.add(LF);
      changed = true;
      continue;
    }

    // Collect the captured variables of newly created closures if we're in
    // the entry block.
    if (auto *CLCI = llvh::dyn_cast<BaseCreateLexicalChildInst>(&I)) {
      if (entryCV)
        collectCapturedVariables(*entryCV, CLCI->getFunctionCode(), F);
      continue;
    }

    // Invalidate the variable storage if the instruction may execute capturing
    // stores that write the variable.
    if (I.mayExecute()) {
      for (auto it = knownValues.begin(); it != knownValues.end(); it++) {
        // Use incremental capture information in the entry block for owned
        // variables, and global information for everything else.
        bool ownedVar = it->first->getParent()->getFunction() == F;
        const auto &cv = entryCV && ownedVar ? *entryCV : globalCV;

        // If there are any captured stores of the variable, the value may be
        // updated, so preceding stores cannot be propagated.
        if (cv.stores.count(it->first))
          knownValues.erase(it);
      }
    }
  }

  return changed;
}

/// Attempts to remove redundant stores to the frame in \p BB when we can
/// determine they cannot be observed.
bool eliminateStores(BasicBlock *BB) {
  Function *F = BB->getParent();

  // See comment in eliminateLoads above.
  // Note that for store elimination, being in the entry block is also relevant
  // because it implies that the block is not in a try, which means that stores
  // to variables owned by the current function can be eliminated across
  // instructions that may throw.
  llvh::Optional<CapturedVariables> entryCV;
  if (BB == &*F->begin())
    entryCV.emplace();

  // Map from a Variable to the last store to it that we have not found to be
  // observable yet. If an entry exists when a subsequent store is
  // encountered, the entry's store is not observable and may be eliminated.
  llvh::DenseMap<Variable *, StoreFrameInst *> prevStores;

  // Deletes instructions when we leave the function.
  IRBuilder::InstructionDestroyer destroyer;

  bool changed = false;

  for (Instruction &I : *BB) {
    // Try to delete the previous store based on the current store.
    if (auto *SF = llvh::dyn_cast<StoreFrameInst>(&I)) {
      auto [it, inserted] = prevStores.try_emplace(SF->getVariable(), SF);

      if (!inserted) {
        // There is a previous store, delete it and make this the previous
        // store.
        destroyer.add(it->second);
        changed = true;
        it->second = SF;
      }
      continue;
    }

    // If we are reading from the variable, the last known store cannot be
    // eliminated.
    if (auto *LF = llvh::dyn_cast<LoadFrameInst>(&I)) {
      prevStores.erase(LF->getLoadVariable());
      continue;
    }

    // Collect the captured variables of newly created closures if we're in
    // the entry block.
    if (auto *CLCI = llvh::dyn_cast<BaseCreateLexicalChildInst>(&I)) {
      if (entryCV)
        collectCapturedVariables(*entryCV, CLCI->getFunctionCode(), F);
      continue;
    }

    // Invalidate the store frame storage if the instruction may execute
    // capturing loads that observe this store, or throw an exception that
    // allows prior stores to be observed.
    auto sideEffect = I.getSideEffect();
    if (sideEffect.getExecuteJS() || sideEffect.getThrow()) {
      for (auto it = prevStores.begin(); it != prevStores.end(); it++) {
        // If this variable is owned by the current function, and we are in the
        // entry block, we know that throwing an exception will not make the
        // store observable. So if the variable does not have any captures that
        // load it, we know the previous store is not observable yet, and can
        // leave it in prevStores.
        bool ownedVar = it->first->getParent()->getFunction() == F;
        if (entryCV && ownedVar && !entryCV->loads.count(it->first))
          continue;

        prevStores.erase(it);
      }
    }
  }
  return changed;
}

bool runFrameLoadStoreOpts(Module *M) {
  CapturedVariables cv;
  // Collect information about all capturing loads and stores for every
  // variable in the module.
  for (Function &F : *M) {
    for (Variable *V : F.getFunctionScope()->getVariables()) {
      for (Instruction *I : V->getUsers()) {
        if (I->getParent()->getParent() != &F) {
          if (llvh::isa<LoadFrameInst>(I)) {
            cv.loads.insert(V);
          } else {
            assert(llvh::isa<StoreFrameInst>(I) && "No other valid user");
            cv.stores.insert(V);
          }
        }
      }
    }
  }

  bool changed = false;
  for (auto &F : *M) {
    for (auto &BB : F) {
      changed |= eliminateLoads(&BB, cv);
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
