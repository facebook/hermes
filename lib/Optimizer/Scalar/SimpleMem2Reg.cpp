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
/// The algorithm is based on:
///
/// Sreedhar and Gao. A linear time algorithm for placing phi-nodes. POPL '95.
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
#include "llvh/ADT/DenseSet.h"
#include "llvh/ADT/STLExtras.h"
#include "llvh/Support/Debug.h"

#include <queue>

STATISTIC(NumPhi, "Number of Phi inserted");
STATISTIC(NumAlloc, "Number of AllocStack removed");

STATISTIC(NumLoad, "Number of loads eliminated");
STATISTIC(NumStore, "Number of stores eliminated");
STATISTIC(NumSOL, "Number of store only locations");

namespace hermes {
namespace {

using BlockSet = llvh::DenseSet<BasicBlock *>;
using BlockToInstMap = llvh::DenseMap<BasicBlock *, Instruction *>;

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

/// \returns true if \p ASI is used in a try block, or is used by an
/// instruction other than LoadStackInst/StoreStackInst (like GetPNamesInst).
/// In that case it is not subject to SSA conversion.
bool isUnsafeStackLocation(
    AllocStackInst *ASI,
    const llvh::DenseMap<BasicBlock *, size_t> &blockTryDepths) {
  // For all users of the stack allocation:
  for (auto *U : ASI->getUsers()) {
    if (llvh::isa<LoadStackInst>(U))
      continue;

    // If the location is stored to from a try block, it cannot be safely
    // promoted to SSA, since an exception thrown prior to the store may be
    // caught in the same function, making the store observable.
    if (llvh::isa<StoreStackInst>(U)) {
      if (blockTryDepths.count(U->getParent()))
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
  auto [blockTryDepths, maxTryDepth] = getBlockTryDepths(F);

  // For each instruction in the basic block:
  for (auto &BB : *F) {
    for (auto &it : BB) {
      auto *ASI = llvh::dyn_cast<AllocStackInst>(&it);
      if (!ASI)
        continue;

      // Check if the stack location is safe for SSA conversion.
      if (isUnsafeStackLocation(ASI, blockTryDepths))
        unsafe.push_back(ASI);
      else
        allocas.push_back(ASI);
    }
  }
}

using DomTreeNode = llvh::DomTreeNodeBase<BasicBlock>;
using DomTreeLevelMap = llvh::DenseMap<DomTreeNode *, unsigned>;

using DomTreeNodePair = std::pair<DomTreeNode *, unsigned>;
using NodePriorityQueue = std::priority_queue<
    DomTreeNodePair,
    llvh::SmallVector<DomTreeNodePair, 32>,
    llvh::less_second>;

/// Compute the dominator tree levels for our graph.
void computeDomTreeLevels(DominanceInfo *DT, DomTreeLevelMap &DomTreeLevels) {
  llvh::SmallVector<DomTreeNode *, 32> worklist;
  DomTreeNode *root = DT->getRootNode();

  // Root starts at zero.
  DomTreeLevels[root] = 0;
  worklist.push_back(root);

  // For all nodes in the tree (this is a DFS scan).
  while (!worklist.empty()) {
    DomTreeNode *Node = worklist.pop_back_val();
    unsigned ChildLevel = DomTreeLevels[Node] + 1;
    // Assign tree level to children.
    for (auto &CI : *Node) {
      DomTreeLevels[CI] = ChildLevel;
      worklist.push_back(CI);
    }
  }
}

Value *getLiveOutValue(
    BasicBlock *startBB,
    BlockToInstMap &phiLoc,
    DominanceInfo &DT,
    BlockToInstMap &stores) {
  LLVM_DEBUG(llvh::dbgs() << "Searching for a value definition.\n");

  // Walk the Dom tree in search of a defining value:
  for (DomTreeNode *Node = DT.getNode(startBB); Node; Node = Node->getIDom()) {
    BasicBlock *BB = Node->getBlock();

    // If there is a store (that must come after the phi), use its value.
    auto it = stores.find(BB);
    if (it != stores.end()) {
      return cast<StoreStackInst>(it->second)->getValue();
    }

    // If there is a Phi definition in this block:
    auto pit = phiLoc.find(BB);
    if (pit != phiLoc.end()) {
      return pit->second;
    }
    // Move to the next dominating block.
  }
  LLVM_DEBUG(llvh::dbgs() << "Could not find a def. Using undefined.\n");
  IRBuilder builder(startBB->getParent());
  return builder.getLiteralUndefined();
}

/// \returns the live-in value for basic block \p BB knowing that we've placed
/// phi nodes in the blocks \p phiLoc.
Value *getLiveInValue(
    BasicBlock *BB,
    BlockToInstMap &phiLoc,
    DominanceInfo &DT,
    BlockToInstMap &stores) {
  // If we are looking for a value in the current basic block then it must be
  // the phi node of the current block.
  auto it = phiLoc.find(BB);
  if (it != phiLoc.end())
    return it->second;

  auto *node = DT.getNode(BB);
  if (!node) {
    // If the node is not in the dom tree then it means that the current value
    // is unreachable. Just return undef.
    IRBuilder builder(BB->getParent());
    return builder.getLiteralUndefined();
  }

  assert(DT.getNode(BB) && "Block must be a part of the dom tree");

  // If the value is not defined in the current basic block then it means that
  // it is defined in one of the dominating basic blocks that are flowing into
  // the current basic block.

  DomTreeNode *IDom = node->getIDom();
  assert(IDom && "Reached the top of the tree!");

  return getLiveOutValue(IDom->getBlock(), phiLoc, DT, stores);
}

void promoteAllocStackToSSA(
    AllocStackInst *ASI,
    DominanceInfo &DT,
    DomTreeLevelMap &domTreeLevels) {
  // A list of blocks that will require new Phi values.
  BlockSet phiBlocks;

  // The data-structure that we use for processing the dom-tree bottom-up.
  NodePriorityQueue PQ;

  // Collect all of the stores into ASI. We know that at this point we have at
  // most one store per block.
  for (auto *U : ASI->getUsers()) {
    // We need to place Phis for this block.
    if (llvh::isa<StoreStackInst>(U)) {
      // If the block is in the dom tree (dominated by the entry block).
      if (DomTreeNode *Node = DT.getNode(U->getParent()))
        PQ.push(std::make_pair(Node, domTreeLevels[Node]));
    }
  }

  LLVM_DEBUG(llvh::dbgs() << " Found: " << PQ.size() << " Defs\n");

  // A list of nodes for which we already calculated the dominator frontier.
  llvh::SmallPtrSet<DomTreeNode *, 32> visited;

  llvh::SmallVector<DomTreeNode *, 32> worklist;

  // Scan all of the definitions in the function bottom-up using the priority
  // queue.
  while (!PQ.empty()) {
    DomTreeNodePair rootPair = PQ.top();
    PQ.pop();
    DomTreeNode *root = rootPair.first;
    unsigned rootLevel = rootPair.second;

    // Walk all dom tree children of Root, inspecting their successors. Only
    // J-edges, whose target level is at most root's level are added to the
    // dominance frontier.
    worklist.clear();
    worklist.push_back(root);

    while (!worklist.empty()) {
      DomTreeNode *node = worklist.pop_back_val();
      BasicBlock *BB = node->getBlock();

      // For all successors of the node:
      for (auto succ : successors(BB)) {
        DomTreeNode *succNode = DT.getNode(succ);

        // Skip D-edges (edges that are dom-tree edges).
        if (succNode->getIDom() == node)
          continue;

        // Ignore J-edges that point to nodes that are not smaller or equal
        // to the root level.
        unsigned succLevel = domTreeLevels[succNode];
        if (succLevel > rootLevel)
          continue;

        // Ignore visited nodes.
        if (!visited.insert(succNode).second)
          continue;

        // If the new PHInode is not dominated by the allocation then it's dead.
        if (!DT.dominates(ASI->getParent(), succNode->getBlock()))
          continue;

        // The successor node is a new PHINode. If this is a new PHI node
        // then it may require additional definitions, so add it to the PQ.
        if (phiBlocks.insert(succ).second)
          PQ.push(std::make_pair(succNode, succLevel));
      }

      // Add the children in the dom-tree to the worklist.
      for (auto &CI : *node)
        if (!visited.count(CI))
          worklist.push_back(CI);
    }
  }

  LLVM_DEBUG(llvh::dbgs() << " Found: " << phiBlocks.size() << " new PHIs\n");

  // At this point we've calculated the locations of all of the new phi nodes.
  // Next, create the phi nodes and promote all of the loads and stores into the
  // new locations.

  BlockToInstMap phiLoc;

  IRBuilder builder(ASI->getParent()->getParent());

  // For all phi nodes we've decided to insert:
  for (auto *BB : phiBlocks) {
    builder.setInsertionPoint(&*BB->begin());
    // Create an empty phi node and register it as the phi for ASI for block BB.
    phiLoc[BB] = builder.createPhiInst();
  }

  BlockToInstMap stores;
  llvh::SmallVector<LoadStackInst *, 16> loads;

  // Collect all loads/stores for the ASI.
  for (auto *U : ASI->getUsers()) {
    if (auto *L = llvh::dyn_cast<LoadStackInst>(U)) {
      loads.push_back(L);
      continue;
    }
    if (auto *S = llvh::dyn_cast<StoreStackInst>(U)) {
      assert(!stores.count(S->getParent()) && "multiple stores per block!");
      stores[S->getParent()] = S;
      continue;
    }
    llvm_unreachable("Invalid use");
  }

  LLVM_DEBUG(llvh::dbgs() << " Finished placing Phis \n");

  // For all phi nodes we've decided to insert:
  for (auto *BB : phiBlocks) {
    NumPhi++;
    auto *phi = cast<PhiInst>(phiLoc[BB]);

    llvh::SmallVector<BasicBlock *, 4> preds(predecessors(BB));
    llvh::SmallPtrSet<BasicBlock *, 4> processed{};
    for (auto *pred : preds) {
      // The predecessor list can contain duplicates. Just skip them.
      if (!processed.insert(pred).second)
        continue;

      auto *val = getLiveOutValue(pred, phiLoc, DT, stores);
      phi->addEntry(val, pred);
    }

    phi->setType(ASI->getType());
  }

  {
    IRBuilder::InstructionDestroyer destroyer;

    // Replace all loads with the new phi nodes.
    for (auto &ld : loads) {
      auto *repl = getLiveInValue(ld->getParent(), phiLoc, DT, stores);
      ld->replaceAllUsesWith(repl);
      destroyer.add(ld);
    }

    // Delete all stores.
    for (auto &st : stores) {
      destroyer.add(st.second);
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
  DominanceInfo D(F);

  // Compute dominator tree node levels for the function.
  DomTreeLevelMap domTreeLevels;
  computeDomTreeLevels(&D, domTreeLevels);

  // a list of stack allocations to promote.
  llvh::SmallVector<AllocStackInst *, 16> allocations;

  // a list of stack allocations that are unsafe to optimize.
  llvh::SmallVector<AllocStackInst *, 16> unsafeAllocations;

  collectStackAllocations(F, allocations, unsafeAllocations);

  LLVM_DEBUG(
      llvh::dbgs() << "Optimizing loads and stores in "
                   << F->getInternalNameStr() << "\n");

  for (auto &it : *F) {
    BasicBlock *BB = &it;
    changed |= promoteLoads(BB);
    changed |= eliminateStores(BB, unsafeAllocations);
  }

  for (auto &it : *F) {
    BasicBlock *BB = &it;
    changed |= eliminateStoreOnlyLocations(BB);
  }

  allocations.clear();
  unsafeAllocations.clear();
  collectStackAllocations(F, allocations, unsafeAllocations);

  for (auto *ASI : allocations) {
    promoteAllocStackToSSA(ASI, D, domTreeLevels);
  }

  simplifyPhiInsts(F);

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
