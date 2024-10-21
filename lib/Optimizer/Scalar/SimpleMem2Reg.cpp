/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
///
/// This optimization promotes stack allocations into virtual registers.
/// The algorithm is loosely based on:
///
/// Braun, et al.: Simple and efficient construction of static single assignment
/// form
///
/// The pass will take the following steps:
/// 1. Prepare the IR such that each block only has at most a single load and
///    store for a given stack location. If both are present, the load must
///    precede the store. This means that any load must resolve to a value in a
///    predecessor block, and any store must be the definition from its block
///    for that stack location.
/// 2. Replace every load with a placeholder Phi placed at the start of its
///    block.
/// 3. Populate every inserted Phi with the definitions from its predecessors.
///    If a block does not already have a definition, insert a placeholder
///    Phi in that block to serve that purpose.
/// 4. Delete the (many) redundant Phis inserted by the previous steps.
/// 5. Delete store-only stack locations.
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "simplemem2reg"

#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Optimizer/Scalar/Utils.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/SetVector.h"
#include "llvh/Support/Debug.h"

STATISTIC(NumAlloc, "Number of AllocStack removed");

STATISTIC(NumLoad, "Number of loads eliminated");
STATISTIC(NumStore, "Number of stores eliminated");
STATISTIC(NumSOL, "Number of store only locations");

namespace hermes {
namespace {

/// Eliminate loads in \p BB by coalescing multiple loads from the same stack
/// location, and eliminating loads from locations that are stored to in the
/// same basic block.
bool promoteLoads(BasicBlock *BB) {
  // Uncaptured AllocStack instructions don't alias with other memory locations
  // and may only be accessed by LoadStack/StoreStack instructions. We can
  // optimize them without inspecting side effects. Even 'call' instructions
  // can't change the value of AllocStack allocations. This map saves the most
  // recently known values for the stack slots. We can learn about the value by
  // inspecting loads or by recording stores. Captured allocas (allocas in
  // try-catch blocks) must be wiped out on side effects because exceptions
  // modify the expected control flow.
  llvh::DenseMap<AllocStackInst *, Value *> knownStackValues;
  IRBuilder::InstructionDestroyer destroyer;

  bool changed = false;

  for (auto &it : *BB) {
    Instruction *II = &it;

    if (auto *SS = llvh::dyn_cast<StoreStackInst>(II)) {
      // Record the value stored to the stack:
      knownStackValues[SS->getPtr()] = SS->getValue();
      continue;
    }

    // If the instructions writes to the stack and one of its operands is
    // an alloca (any alloca), remove that alloca from the known stack
    // values.
    if (II->getSideEffect().getWriteStack()) {
      for (unsigned i = 0, e = II->getNumOperands(); i != e; ++i) {
        if (auto *ASI = llvh::dyn_cast<AllocStackInst>(II->getOperand(i)))
          knownStackValues.erase(ASI);
      }
      continue;
    }

    // Try to replace the LoadStack with a recently saved value.
    if (auto *LS = llvh::dyn_cast<LoadStackInst>(II)) {
      AllocStackInst *dest = LS->getPtr();
      auto entry = knownStackValues.find(dest);

      // We don't have any information on this stack slot. This means that
      // the result of this load is the most recent value that is known to be
      // in the stack slot. Record this information and move on.
      if (entry == knownStackValues.end()) {
        knownStackValues[dest] = LS;
        continue;
      }

      // Replace all uses of the load with the last known value.
      LS->replaceAllUsesWith(entry->second);
      ++NumLoad;

      // We have no use of this load now. Remove it.
      destroyer.add(LS);
      changed = true;
      continue;
    }
  }

  return changed;
}

/// Eliminate stores in \p BB by coalescing multiple stores to the same stack
/// location.
bool eliminateStores(
    BasicBlock *BB,
    llvh::ArrayRef<AllocStackInst *> unsafeAllocas) {
  // A list of un-clobbered stack store instructions.
  llvh::DenseMap<AllocStackInst *, StoreStackInst *> prevStoreStack;

  // Deletes instructions when we leave the function.
  IRBuilder::InstructionDestroyer destroyer;

  bool changed = false;

  for (auto &it : *BB) {
    Instruction *II = &it;

    // Try to delete the previous store based on the current store.
    if (auto *SS = llvh::dyn_cast<StoreStackInst>(II)) {
      auto *AS = SS->getPtr();

      auto [entry, inserted] = prevStoreStack.try_emplace(AS, SS);

      if (!inserted) {
        // Found store-after-store. Mark the previous store for deletion.
        destroyer.add(entry->second);
        ++NumStore;
        changed = true;
        entry->second = SS;
      }

      continue;
    }

    auto sideEffect = II->getSideEffect();

    // If this instruction can read from the stack, we should invalidate all of
    // its stack operands. Note that this should only affect non-LoadStack
    // instructions (and therefore only affect unsafe allocas), since any
    // LoadStack instruction that accesses a location that has been stored to
    // will have been eliminated by promoteLoads.
    if (sideEffect.getReadStack()) {
      for (size_t i = 0, e = II->getNumOperands(); i < e; ++i)
        if (auto *AS = llvh::dyn_cast<AllocStackInst>(II->getOperand(i)))
          prevStoreStack.erase(AS);
    }

    // Note that we deliberately fall through to the below check since reading
    // from the stack and throwing are not mutually exclusive.

    // If this instruction may throw, we cannot coalesce stores to unsafe
    // allocas across it, since the stored value may be observed if the thrown
    // exception is caught.
    if (sideEffect.getThrow())
      for (auto *A : unsafeAllocas)
        prevStoreStack.erase(A);
  }

  return changed;
}

/// \returns true if the instruction has non-store uses, like loads.
bool hasNonStoreUses(AllocStackInst *ASI) {
  for (auto *U : ASI->getUsers()) {
    if (!llvh::isa<StoreStackInst>(U))
      return true;
  }

  return false;
}

bool eliminateStoreOnlyLocations(BasicBlock *BB) {
  bool changed = false;
  IRBuilder::InstructionDestroyer destroyer;

  for (auto &it : *BB) {
    auto *ASI = llvh::dyn_cast<AllocStackInst>(&it);
    if (!ASI)
      continue;

    if (hasNonStoreUses(ASI)) {
      continue;
    }

    // Add any stores that use the allocation to the deletion list.
    for (auto *U : ASI->getUsers()) {
      destroyer.add(cast<Instruction>(U));
    }

    ++NumSOL;
    destroyer.add(ASI);
    changed = true;
  }

  return changed;
}

/// \returns true if \p ASI is stored to from multiple try blocks, or is used by
/// an instruction other than LoadStackInst/StoreStackInst (like GetPNamesInst).
/// In that case it is not subject to SSA conversion.
bool isUnsafeStackLocation(
    AllocStackInst *ASI,
    const llvh::DenseMap<BasicBlock *, TryStartInst *> &enclosingTrys) {
  // Upon reaching the first store, this is set to either the TryStartInst
  // immediately enclosing that store, or nullptr if no such TryStartInst
  // exists. This is used to track whether any subsequent stores are in a
  // different try block than the first one, which would make this location
  // "unsafe".
  llvh::Optional<TryStartInst *> storeTry;

  // For all users of the stack allocation:
  for (auto *U : ASI->getUsers()) {
    if (llvh::isa<LoadStackInst>(U))
      continue;

    // If the location is stored to from multiple try blocks, it cannot be
    // promoted to SSA, since an exception thrown prior to the store may be
    // caught in the same function, making the store observable. Note that this
    // is not a problem if all stores are in the same try block, because our IR
    // invariants would guarantee that no loads can exist in the catch or after
    // the try block.
    if (llvh::isa<StoreStackInst>(U)) {
      if (!storeTry)
        storeTry = enclosingTrys.lookup(U->getParent());
      else if (*storeTry != enclosingTrys.lookup(U->getParent()))
        return true;
      continue;
    }

    // Some other instruction is capturing the ASI.
    return true;
  }

  return false;
}

/// Collect all of the allocas in the program in two lists. \p allocas that are
/// optimizable, and \p unsafe, which are allocas that we can't optimize because
/// they are used in try blocks or non-load/store instructions.
void collectStackAllocations(
    Function *F,
    llvh::SmallVectorImpl<AllocStackInst *> &allocas,
    llvh::SmallVectorImpl<AllocStackInst *> &unsafe) {
  // Collect all of the basic blocks that are enclosed by try's.
  auto enclosingTrys = findEnclosingTrysPerBlock(F).getValueOr(
      llvh::DenseMap<BasicBlock *, TryStartInst *>{});

  // For each instruction in the basic block:
  for (auto &BB : *F) {
    for (auto &it : BB) {
      auto *ASI = llvh::dyn_cast<AllocStackInst>(&it);
      if (!ASI)
        continue;

      // Check if the stack location is safe for SSA conversion.
      if (isUnsafeStackLocation(ASI, enclosingTrys))
        unsafe.push_back(ASI);
      else
        allocas.push_back(ASI);
    }
  }
}

/// Resolve all loads from the given stack location \p ASI to preceding stores.
/// The location must be safe to promote, and must have been prepared such that:
///   1. There is at most one store to the location per basic block.
///   2. There is at most one load from the location per basic block.
///   3. If a block has both a load and a store, the store comes later.
/// This means that all stores are serve as the outgoing definition of the
/// location for their block, and all loads resolve to the outgoing definitions
/// of their predecessor blocks.
void promoteAllocStackToSSA(AllocStackInst *ASI) {
  // Keep track of all Phis that have been added so we can populate them later.
  // This is used as the work queue for inserting and populating Phis.
  llvh::SmallVector<PhiInst *, 8> phis;

  IRBuilder builder(ASI->getFunction());

  {
    IRBuilder::InstructionDestroyer destroyer;

    // Replace every load from this stack location with a placeholder Phi.
    for (auto *U : ASI->getUsers()) {
      if (llvh::isa<StoreStackInst>(U))
        continue;
      assert(llvh::isa<LoadStackInst>(U) && "No other user allowed here.");

      auto *BB = U->getParent();
      builder.setInsertionPoint(&BB->front());
      builder.setLocation(U->getLocation());
      auto *phi = builder.createPhiInst();
      phis.push_back(phi);
      U->replaceAllUsesWith(phi);
      destroyer.add(U);
    }
  }

  // Track the definition of the stack location that is the outgoing definition
  // for this stack location from each block.
  llvh::DenseMap<BasicBlock *, Value *> blockDefs;

  // Any store in a block is the definition for that block.
  for (auto *U : ASI->getUsers())
    if (auto *SSI = llvh::dyn_cast<StoreStackInst>(U))
      blockDefs.try_emplace(SSI->getParent(), SSI->getValue());

  // Any Phi inserted in a block is the definition for that block unless there
  // is a store in it. Note that try_emplace will not overwrite an existing
  // entry from a store.
  for (auto *phi : phis)
    blockDefs.try_emplace(phi->getParent(), phi);

  // Populate the Phis with the definitions from their predecessors. Note that
  // we can add Phis as we iterate.
  for (size_t i = 0; i < phis.size(); ++i) {
    auto *phi = phis[i];
    auto *BB = phi->getParent();
    phi->setType(ASI->getType());

    // Collect all predecessors first, since iterating predecessors iterates the
    // user list of BB, which may change as we add Phis.
    llvh::SmallSetVector<BasicBlock *, 4> preds;
    preds.insert(pred_begin(BB), pred_end(BB));
    for (auto *pred : preds) {
      // If a predecessor does not have a definition, insert a placeholder Phi
      // there that we will visit in a later iteration.
      auto [it, inserted] = blockDefs.try_emplace(pred);
      if (inserted) {
        builder.setInsertionPoint(&pred->front());
        auto *newPhi = builder.createPhiInst();
        phis.push_back(newPhi);
        it->second = newPhi;
      }

      phi->addEntry(it->second, pred);
    }
  }

  NumAlloc++;
  LLVM_DEBUG(llvh::dbgs() << " Finished placing Phis \n");
}

/// Optimize PHI nodes in \p F where all incoming values that are not self-edges
/// are the same, by replacing them with that single source value.
bool simplifyPhiInsts(Function *F) {
  bool changed = false;
  bool localChanged;
  do {
    localChanged = false;
    for (auto &BB : *F) {
      IRBuilder::InstructionDestroyer destroyer;
      for (auto &I : BB) {
        auto *P = llvh::dyn_cast<PhiInst>(&I);
        if (!P)
          break;

        // The PHI has a single incoming value. Replace all uses of the PHI with
        // the incoming value.
        if (auto *incoming = getSinglePhiValue(P)) {
          localChanged = true;
          P->replaceAllUsesWith(incoming);
          destroyer.add(P);
        }
      }
    }
    changed |= localChanged;
  } while (localChanged);

  return changed;
}

bool mem2reg(Function *F) {
  bool changed = false;

  // a list of stack allocations to promote.
  llvh::SmallVector<AllocStackInst *, 16> allocations;

  // a list of stack allocations that are unsafe to optimize.
  llvh::SmallVector<AllocStackInst *, 16> unsafeAllocations;

  collectStackAllocations(F, allocations, unsafeAllocations);

  LLVM_DEBUG(
      llvh::dbgs() << "Optimizing loads and stores in "
                   << F->getInternalNameStr() << "\n");

  // Prepare the IR to meet the preconditions of promoteAllocStackToSSA.
  for (auto &it : *F) {
    BasicBlock *BB = &it;
    changed |= promoteLoads(BB);
    changed |= eliminateStores(BB, unsafeAllocations);
  }

  for (auto *ASI : allocations) {
    promoteAllocStackToSSA(ASI);
  }

  simplifyPhiInsts(F);

  // Eliminate the stack locations which were promoted.
  for (auto &BB : *F)
    changed |= eliminateStoreOnlyLocations(&BB);

  return changed;
}

} // namespace

Pass *createSimpleMem2Reg() {
  class ThisPass : public FunctionPass {
   public:
    explicit ThisPass() : hermes::FunctionPass("SimpleMem2Reg") {}
    ~ThisPass() override = default;

    bool runOnFunction(Function *F) override {
      return mem2reg(F);
    }
  };
  return new ThisPass();
}

} // namespace hermes

#undef DEBUG_TYPE
