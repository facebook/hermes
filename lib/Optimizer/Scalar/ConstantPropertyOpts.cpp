/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#define DEBUG_TYPE "constantpropertyopts"

#include "hermes/Optimizer/Scalar/ConstantPropertyOpts.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Optimizer/Scalar/CLACallGraphProvider.h"
#include "hermes/Optimizer/Scalar/ClosureAnalysis.h"
#include "hermes/Optimizer/Scalar/SimpleCallGraphProvider.h"
#include "hermes/Support/Statistic.h"

#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"

using namespace hermes;
using llvm::dbgs;
using llvm::isa;

STATISTIC(
    NumCPO,
    "Number of load property instructions whose users are given "
    "a primitive const");

// TODO: there is code duplication between this file and TypeInference.

/// Does a given prop belong in the owned set?
static bool isOwnedProperty(AllocObjectInst *I, Value *prop) {
  for (auto *J : I->getUsers()) {
    if (auto *SOPI = dyn_cast<StoreOwnPropertyInst>(J)) {
      if (SOPI->getObject() == I) {
        if (prop == SOPI->getProperty())
          return true;
      }
    }
  }
  return false;
}

Literal *ConstantPropertyOpts::simplifyLoadPropertyInst(LoadPropertyInst *LPI) {
  bool first = true;
  Value *retVal = nullptr;

  // Bail out if there are unknown receivers.
  if (cgp_->hasUnknownReceivers(LPI))
    return nullptr;

  // Go over each known receiver R (can be empty)
  for (auto *R : cgp_->getKnownReceivers(LPI)) {
    // Note: currently Array analysis is purposely disabled.
    assert(isa<AllocObjectInst>(R) && "Currently Arrays not handled.");

    // Bail out if there are unknown stores.
    if (cgp_->hasUnknownStores(R))
      return nullptr;

    Value *prop = LPI->getProperty();

    // If the property being requested is NOT an owned prop, bail out.
    if (isa<AllocObjectInst>(R)) {
      if (!isOwnedProperty(cast<AllocObjectInst>(R), prop))
        return nullptr;
    }

    // Go over each store of R (can be empty).
    for (auto *S : cgp_->getKnownStores(R)) {
      Value *storeVal = nullptr;
      assert(isa<StoreOwnPropertyInst>(S) || isa<StorePropertyInst>(S));

      if (auto *SI = dyn_cast<StorePropertyInst>(S)) {
        // If the property in the store isn't what this LPI wants, skip the
        // store.
        if (prop != SI->getProperty()) {
          continue;
        }
        // The property is relevant, but,
        // we only work with StoreOwnPropertyInst.
        return nullptr;
      }

      if (auto *SOI = dyn_cast<StoreOwnPropertyInst>(S)) {
        // If the property in the store isn't what this LPI wants, skip the
        // storeown.
        if (prop != SOI->getProperty())
          continue;
        // Found storeOwn of the property LPI is looking for.
        storeVal = SOI->getStoredValue();
      }

      if (first) {
        retVal = storeVal;
        first = false;
      } else {
        // There should not be more than one defining storeOwn!
        retVal = nullptr;
      }
    }
  }

  if (!retVal)
    return nullptr;

  if (!retVal->getType().isPrimitive())
    return nullptr;

  assert(retVal && "Expected retVal not null at this point.");

  if (retVal->getKind() == ValueKind::LiteralNumberKind ||
      retVal->getKind() == ValueKind::LiteralBoolKind ||
      retVal->getKind() == ValueKind::LiteralStringKind ||
      retVal->getKind() == ValueKind::LiteralNullKind ||
      retVal->getKind() == ValueKind::LiteralUndefinedKind) {
    LLVM_DEBUG(
        dbgs() << LPI->getProperty()->getKindStr()
               << " gets value from a unique " << retVal->getKindStr() << "\n");
    NumCPO++;
    return (Literal *)retVal;
  }

  // retVal is not one of the literals we can splice!
  return nullptr;
}

bool ConstantPropertyOpts::runOnFunction(Function *F) {
  LLVM_DEBUG(
      dbgs() << "Starting to CPO on Function " << F->getInternalName() << "\n");
  bool changed = false;
  IRBuilder::InstructionDestroyer destroyer;

  // For all blocks in the function:
  for (auto &blockIter : *F) {
    BasicBlock *BB = &blockIter;

    // For all instructions:
    for (auto instIter = BB->begin(), e = BB->end(); instIter != e;) {
      Instruction *II = &*instIter;
      ++instIter;

      if (!isa<LoadPropertyInst>(II))
        continue;

      LoadPropertyInst *LPI = cast<LoadPropertyInst>(II);

      Literal *newVal = simplifyLoadPropertyInst(LPI);
      if (!newVal)
        continue;

      // We have a better and simpler instruction. Replace the original
      // instruction and mark it for deletion.
      LPI->replaceAllUsesWith(newVal);
      destroyer.add(LPI);

      changed = true;
    }
  }
  return changed;
}

bool ConstantPropertyOpts::runOnModule(Module *M) {
  if (!M->getContext().getOptimizationSettings().constantPropertyOptimizations)
    return false;

  bool changed = false;

  LLVM_DEBUG(
      dbgs() << "\nStart Constant Property opts on module "
             << "\n");

  // Perform the closure analysis based call graph.

  ClosureAnalysis CA;
  CA.analyzeModule(M);

  // Here we need to do any work only if CLA results are available.

  // Process the function nests nested in the top level.
  for (auto &R : CA.analysisRoots_) {
    LLVM_DEBUG(
        dbgs() << "Working with root " << R->getInternalName().c_str() << "\n");

    auto analysis = CA.analysisMap_.find(R);
    assert(analysis != CA.analysisMap_.end());

    auto nested = CA.nestedMap_.find(R);
    assert(nested != CA.nestedMap_.end());

    if (analysis->second != nullptr) {
      CLACallGraphProvider cgp(analysis->second);
      cgp_ = &cgp;
      for (auto &G : nested->second) {
        changed |= runOnFunction(G);
      }
    }
  }

  return changed;
}

Pass *hermes::createConstantPropertyOpts() {
  return new ConstantPropertyOpts();
}
