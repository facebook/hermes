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

#define DEBUG_TYPE "mem2reg"

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

using namespace hermes;
using llvh::dbgs;
using llvh::SmallPtrSet;
using llvh::SmallVector;
using llvh::SmallVectorImpl;

static const int kFrameSizeThreshold = 128;

STATISTIC(NumPhi, "Number of Phi inserted");
STATISTIC(NumAlloc, "Number of AllocStack removed");

STATISTIC(NumLoad, "Number of loads eliminated");
STATISTIC(NumStore, "Number of stores eliminated");
STATISTIC(NumSOL, "Number of store only locations");

using BlockSet = llvh::DenseSet<BasicBlock *>;
using BlockToInstMap = llvh::DenseMap<BasicBlock *, Instruction *>;

/// Add the variables that the function \p F is capturing into \p capturedVars.
/// Notice that the function F may have sub-closures that capture variables.
/// This method does a recursive scan and collects all captured variables.
static void collectCapturedVariables(
    llvh::DenseSet<Variable *> &capturedLoads,
    llvh::DenseSet<Variable *> &capturedStores,
    Function *F) {
  // For all instructions in the function:
  for (auto blockIter = F->begin(), e = F->end(); blockIter != e; ++blockIter) {
    BasicBlock *BB = &*blockIter;
    for (auto &instIter : *BB) {
      Instruction *II = &instIter;

      // Recursively check capturing functions by inspecting the created
      // closure.
      if (auto *CF = llvh::dyn_cast<BaseCreateLexicalChildInst>(II)) {
        collectCapturedVariables(
            capturedLoads, capturedStores, CF->getFunctionCode());
        continue;
      }

      if (auto *LF = llvh::dyn_cast<LoadFrameInst>(II)) {
        Variable *V = LF->getLoadVariable();
        if (V->getParent()->getFunction() != F) {
          capturedLoads.insert(V);
        }
      }

      if (auto *SF = llvh::dyn_cast<StoreFrameInst>(II)) {
        auto *V = SF->getVariable();
        if (V->getParent()->getFunction() != F) {
          capturedStores.insert(V);
        }
      }
    }
  }
}

static bool promoteLoads(BasicBlock *BB) {
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

    // If the instructions writes to memory and one of its operands is
    // an alloca (any alloca), remove that alloca from the known stack
    // values.
    if (II->mayWriteMemory()) {
      for (unsigned i = 0, e = II->getNumOperands(); i != e; ++i) {
        if (auto *ASI = llvh::dyn_cast<AllocStackInst>(II->getOperand(i)))
          knownStackValues.erase(ASI);
      }
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

static bool eliminateStores(
    BasicBlock *BB,
    llvh::ArrayRef<AllocStackInst *> unsafeAllocas) {
  // Check if this block is the entry block.
  Function *F = BB->getParent();
  bool isEntryBlock = (BB == &*F->begin());

  // A list of un-clobbered frame stored values in flight.
  llvh::DenseMap<Variable *, StoreFrameInst *> prevStoreFrame;

  // A list of un-clobbered stack store instructions.
  llvh::DenseMap<AllocStackInst *, StoreStackInst *> prevStoreStack;

  // Deletes instructions when we leave the function.
  IRBuilder::InstructionDestroyer destroyer;

  // A list of variables that are known to be captured.
  llvh::DenseSet<Variable *> capturedVariables;

  // In the entry block we can keep track of which variables have been captured
  // by inspecting the closures that we generate.
  bool usePreciseCaptureAnalysis = isEntryBlock;

  bool changed = false;

  for (auto &it : *BB) {
    Instruction *II = &it;

    // Try to delete the previous store based on the current store.
    if (auto *SF = llvh::dyn_cast<StoreFrameInst>(II)) {
      auto *V = SF->getVariable();
      auto entry = prevStoreFrame.find(V);

      if (entry != prevStoreFrame.end()) {
        // Found store-after-store. Mark the previous store for deletion.
        if (entry->second) {
          destroyer.add(entry->second);
          ++NumStore;
          changed = true;
        }

        entry->second = SF;
        continue;
      }

      prevStoreFrame[V] = SF;
      continue;
    }

    // Try to delete the previous store based on the current store.
    if (auto *SS = llvh::dyn_cast<StoreStackInst>(II)) {
      auto *AS = SS->getPtr();
      auto entry = prevStoreStack.find(AS);

      if (entry != prevStoreStack.end()) {
        // Found store-after-store. Mark the previous store for deletion.
        if (entry->second) {
          destroyer.add(entry->second);
          ++NumStore;
          changed = true;
        }

        entry->second = SS;
        continue;
      }

      prevStoreStack[AS] = SS;
      continue;
    }

    // Invalidate the frame store storage.
    if (auto *LF = llvh::dyn_cast<LoadFrameInst>(II)) {
      auto *V = LF->getLoadVariable();
      prevStoreFrame[V] = nullptr;
      continue;
    }

    // Invalidate the stack store storage.
    if (auto *LS = llvh::dyn_cast<LoadStackInst>(II)) {
      AllocStackInst *AS = LS->getPtr();
      prevStoreStack[AS] = nullptr;
      continue;
    }

    if (II->mayExecute()) {
      for (auto *A : unsafeAllocas) {
        prevStoreStack[A] = nullptr;
      }
    }

    // Invalidate the store frame storage if we can't be sure that the
    // instruction is side-effect free and can't touch our variables.
    if (II->mayReadMemory()) {
      // In no-capture mode the local variables are preserved because they have
      // not been captured. This means that we only need to invalidate the
      // variables that don't belong to this function.
      // limit the size of knownFrameValues in case a function is large, as
      // large functions slow down considerably here
      if (usePreciseCaptureAnalysis &&
          prevStoreFrame.size() < kFrameSizeThreshold) {
        // Erase all non-local variables.
        for (auto &I : prevStoreFrame) {
          if (I.first->getParent()->getFunction() != F ||
              capturedVariables.count(I.first)) {
            I.second = nullptr;
          }
        }
      } else {
        // Invalidate all variables.
        prevStoreFrame.clear();
      }
    }

    if (auto *CF = llvh::dyn_cast<BaseCreateLexicalChildInst>(II)) {
      // Collect the captured variables.
      if (usePreciseCaptureAnalysis) {
        collectCapturedVariables(
            capturedVariables, capturedVariables, CF->getFunctionCode());
      }
    }
  }

  return changed;
}

/// \returns true if the instruction has non-store uses, like loads.
static bool hasNonStoreUses(AllocStackInst *ASI) {
  for (auto *U : ASI->getUsers()) {
    if (!llvh::isa<StoreStackInst>(U))
      return true;
  }

  return false;
}

static bool eliminateStoreOnlyLocations(BasicBlock *BB) {
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

/// \returns true if \p ASI is used in a catch block, or is used by an
/// instruction other than LoadStackInst/StoreStackInst (like GetPNamesInst).
/// In that case it is not subject to SSA conversion.
///
/// "catch" blocks are special because most instructions (all throwing ones)
/// have an implicit edge to them and we can't analyze the control flow, thus
/// we must be conservative when dealing with variables accessed there.
static bool isUnsafeStackLocation(
    AllocStackInst *ASI,
    DominanceInfo *DT,
    BlockSet &exceptionHandlingBlocks) {
  // For all users of the stack allocation:
  for (auto *U : ASI->getUsers()) {
    if (llvh::isa<LoadStackInst>(U) || llvh::isa<StoreStackInst>(U)) {
      // If the load/store is used inside of a catch block then we consider this
      // variable as captured.
      for (auto *BB : exceptionHandlingBlocks) {
        if (DT->dominates(BB, U->getParent()))
          return true;
      }
      // Instruction is not in catch blocks.
      continue;
    }

    // Some other instruction is capturing the ASI.
    return true;
  }

  return false;
}

/// Collect all of the allocas in the program in two lists. \p allocas that are
/// optimizable, and \p unsafe, which are allocas that we can't optimize because
/// they are used by catch blocks or non-load/store instructions.
static void collectStackAllocations(
    Function *F,
    DominanceInfo *DT,
    SmallVectorImpl<AllocStackInst *> &allocas,
    SmallVectorImpl<AllocStackInst *> &unsafe) {
  // Collect all of the blocks that are roots of catch regions.
  BlockSet exceptionHandlingBlocks;
  for (auto &BB : *F) {
    Instruction *I = &*BB.begin();
    if (llvh::isa<TryStartInst>(BB.getTerminator()) ||
        llvh::isa<CatchInst>(I)) {
      exceptionHandlingBlocks.insert(&BB);
    }
  }

  // For each instruction in the basic block:
  for (auto &BB : *F) {
    for (auto &it : BB) {
      auto *ASI = llvh::dyn_cast<AllocStackInst>(&it);
      if (!ASI)
        continue;

      // Don't touch captured stack allocations that are used by non-load/store
      // instructions directly.
      if (isUnsafeStackLocation(ASI, DT, exceptionHandlingBlocks)) {
        unsafe.push_back(ASI);
        continue;
      }

      allocas.push_back(ASI);
    }
  }
}

using DomTreeNode = llvh::DomTreeNodeBase<BasicBlock>;
using DomTreeLevelMap = llvh::DenseMap<DomTreeNode *, unsigned>;

using DomTreeNodePair = std::pair<DomTreeNode *, unsigned>;
using NodePriorityQueue = std::priority_queue<
    DomTreeNodePair,
    SmallVector<DomTreeNodePair, 32>,
    llvh::less_second>;

/// Compute the dominator tree levels for our graph.
static void computeDomTreeLevels(
    DominanceInfo *DT,
    DomTreeLevelMap &DomTreeLevels) {
  SmallVector<DomTreeNode *, 32> worklist;
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

static Value *getLiveOutValue(
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
static Value *getLiveInValue(
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

static void promoteAllocStackToSSA(
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

  SmallVector<DomTreeNode *, 32> worklist;

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

    SmallVector<BasicBlock *, 4> preds(predecessors(BB));
    SmallPtrSet<BasicBlock *, 4> processed{};
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

static bool mem2reg(Function *F) {
  bool changed = false;
  DominanceInfo D(F);

  // Compute dominator tree node levels for the function.
  DomTreeLevelMap domTreeLevels;
  computeDomTreeLevels(&D, domTreeLevels);

  // a list of stack allocations to promote.
  SmallVector<AllocStackInst *, 16> allocations;

  // a list of stack allocations that are unsafe to optimize.
  SmallVector<AllocStackInst *, 16> unsafeAllocations;

  collectStackAllocations(F, &D, allocations, unsafeAllocations);

  LLVM_DEBUG(
      dbgs() << "Optimizing loads and stores in " << F->getInternalNameStr()
             << "\n");

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
  collectStackAllocations(F, &D, allocations, unsafeAllocations);

  for (auto *ASI : allocations) {
    promoteAllocStackToSSA(ASI, D, domTreeLevels);
  }

  return changed;
}

Pass *hermes::createMem2Reg() {
  class Mem2Reg : public FunctionPass {
   public:
    explicit Mem2Reg() : hermes::FunctionPass("Mem2Reg") {}
    ~Mem2Reg() override = default;

    bool runOnFunction(Function *F) override {
      return mem2reg(F);
    }
  };
  return new Mem2Reg();
}

#undef DEBUG_TYPE
