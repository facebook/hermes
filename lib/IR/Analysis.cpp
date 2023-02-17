/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IR.h"
#include "hermes/Utils/Dumper.h"
#include "llvh/ADT/PriorityQueue.h"
#include "llvh/Support/Debug.h"

#include <utility>

#ifdef DEBUG_TYPE
#undef DEBUG_TYPE
#endif
#define DEBUG_TYPE "IR Analysis"

using namespace hermes;

using llvh::dbgs;
using llvh::Optional;
using llvh::outs;

void PostOrderAnalysis::visitPostOrder(BasicBlock *BB, BlockList &order) {
  struct State {
    BasicBlock *BB;
    succ_iterator cur, end;
    explicit State(BasicBlock *BB)
        : BB(BB), cur(succ_begin(BB)), end(succ_end(BB)) {}
  };

  llvh::SmallPtrSet<BasicBlock *, 16> visited{};
  llvh::SmallVector<State, 32> stack{};

  stack.emplace_back(BB);
  do {
    while (stack.back().cur != stack.back().end) {
      BB = *stack.back().cur++;
      if (visited.insert(BB).second)
        stack.emplace_back(BB);
    }

    order.push_back(stack.back().BB);
    stack.pop_back();
  } while (!stack.empty());
}

PostOrderAnalysis::PostOrderAnalysis(Function *F) : ctx_(F->getContext()) {
  assert(Order.empty() && "vector must be empty");

  BasicBlock *entry = &*F->begin();

  // Finally, do an PO scan from the entry block.
  visitPostOrder(entry, Order);

  assert(
      !Order.empty() && Order[Order.size() - 1] == entry &&
      "Entry block must be the last element in the vector");
}

void PostOrderAnalysis::dump() {
  IRPrinter D(ctx_, outs());
  D.visit(*Order[0]->getParent());

  outs() << "Blocks: ";

  for (auto &BB : Order) {
    outs() << "BB" << D.BBNamer.getNumber(BB) << " ";
  }

  outs() << "\n";
}

// Perform depth-first search to identify loops. The loop header of a block B is
// its DFS ancestor H such that a descendent of B has a back edge to H. (In the
// case of a self-loop, the ancestor and descendant are B itself.) When there
// are multiple such blocks, we take the one with the maximum DFS discovery time
// to get the innermost loop. The preheader is the DFS parent of H. We use
// DominanceInfo to ensure that preheaders dominate headers and headers dominate
// all blocks in the loop.
LoopAnalysis::LoopAnalysis(Function *F, const DominanceInfo &dominanceInfo) {
  // BlockMap (defined in Analysis.h) and BlockSet are used for most cases.
  // TinyBlockSet is only used for headerSets (defined below); it has a smaller
  // inline size because we store BLOCK -> {SET OF HEADERS} for every block, and
  // very deeply nested loops (leading to many headers) are not that common.
  using BlockSet = llvh::SmallPtrSet<const BasicBlock *, 16>;
  using TinyBlockSet = llvh::SmallPtrSet<BasicBlock *, 2>;

  int dfsTime = 0;
  // Maps each block to its DFS discovery time (value of dfsTime).
  BlockMap<int> discovered;
  // Set of blocks we have finished visiting.
  BlockSet finished;
  // Maps each block to its parent in the DFS tree.
  BlockMap<BasicBlock *> parent;
  // Maps each block to a set of header blocks of loops that enclose it.
  BlockMap<TinyBlockSet> headerSets;

  // Explicit stack for depth-first search.
  llvh::SmallVector<BasicBlock *, 16> stack;
  stack.push_back(&*F->begin());
  while (stack.size()) {
    BasicBlock *BB = stack.back();
    // If it's the first time visiting, record the discovery time and push all
    // undiscovered successors onto the stack. Leave BB on the stack so that
    // after visiting all descendants, we come back to it and resume below.
    if (discovered.try_emplace(BB, dfsTime).second) {
      ++dfsTime;
      for (auto it = succ_begin(BB), e = succ_end(BB); it != e; ++it) {
        BasicBlock *succ = *it;
        if (!discovered.count(succ)) {
          stack.push_back(succ);
          parent[succ] = BB;
        }
      }
      continue;
    }

    stack.pop_back();
    if (finished.count(BB)) {
      // BB was duplicated on the stack and we already finished visiting it.
      continue;
    }

    // Check back/forward/cross edges to find loops BB is in.
    TinyBlockSet headers;
    for (auto it = succ_begin(BB), e = succ_end(BB); it != e; ++it) {
      BasicBlock *succ = *it;
      assert(discovered.count(succ) && "Unexpected undiscovered successor");
      if (!finished.count(succ)) {
        // Found a back edge to a header block.
        headers.insert(succ);
      } else {
        // Either a forward edge or cross edge. Headers of succ are also headers
        // of BB if we haven't finished visiting them.
        auto entry = headerSets.find(succ);
        if (entry != headerSets.end()) {
          for (BasicBlock *headerOfSucc : entry->second) {
            if (!finished.count(headerOfSucc)) {
              headers.insert(headerOfSucc);
            }
          }
        }
      }
    }
    if (!headers.empty()) {
      auto insert = headerSets.try_emplace(BB, std::move(headers));
      (void)insert;
      assert(insert.second && "Inserting headers for same block twice!");
    }
    finished.insert(BB);
  }

  // Determine which headers are good/bad and populate headerToPreheader_ using
  // the parent mapping. A header is good if it dominates every block in the
  // loop (that is, it is the only entry point).
  BlockSet badHeaders;
  for (auto &entry : headerSets) {
    const BasicBlock *BB = entry.first;
    for (const BasicBlock *header : entry.second) {
      if (badHeaders.count(header)) {
        continue;
      }
      if (!dominanceInfo.dominates(header, BB)) {
        badHeaders.insert(header);
      } else if (!headerToPreheader_.count(header)) {
        BasicBlock *preheader = parent[header];
        if (dominanceInfo.properlyDominates(preheader, header)) {
          headerToPreheader_[header] = preheader;
        }
      }
    }
  }

  // Populate blockToHeader_ with the innermost loop header for each block.
  for (auto &entry : headerSets) {
    const BasicBlock *BB = entry.first;
    TinyBlockSet &headers = entry.second;
    if (!headers.empty()) {
      BasicBlock *innerHeader = nullptr;
      int maxDiscovery = -1;
      for (BasicBlock *header : headers) {
        int discovery = discovered[header];
        if (discovery > maxDiscovery && !badHeaders.count(header)) {
          maxDiscovery = discovery;
          innerHeader = header;
        }
      }
      blockToHeader_[BB] = innerHeader;
    }
  }
}

BasicBlock *LoopAnalysis::getLoopHeader(const BasicBlock *BB) const {
  return blockToHeader_.lookup(BB);
}

BasicBlock *LoopAnalysis::getLoopPreheader(const BasicBlock *BB) const {
  BasicBlock *header = getLoopHeader(BB);
  if (header) {
    return headerToPreheader_.lookup(header);
  }
  return nullptr;
}

static llvh::Optional<int> &nextScopeDepth(llvh::Optional<int> &depth) {
  if (depth) {
    *depth -= 1;
  }
  return depth;
}

Function *FunctionScopeAnalysis::computeParent(
    ScopeDesc *thisScope,
    ScopeDesc *parentScope,
    const ScopeData &sd) {
  return (parentScope->hasFunction() &&
          parentScope->getFunction() != thisScope->getFunction())
      ? parentScope->getFunction()
      : sd.parent;
}

FunctionScopeAnalysis::ScopeData
FunctionScopeAnalysis::calculateFunctionScopeData(
    ScopeDesc *scopeDesc,
    llvh::Optional<int> depth) {
  auto entry = lexicalScopeDescMap_.find(scopeDesc);
  if (entry != lexicalScopeDescMap_.end()) {
    return entry->second;
  }

  if (!scopeDesc->hasFunction()) {
    // The only scope that doesn't have a function is the Module's initial
    // scope.
    assert(scopeDesc->getParent() == nullptr);
  } else {
    // If the function is a CommonJS module,
    // then it won't have a CreateFunctionInst, so calculate the depth manually.
    Function *F = scopeDesc->getFunction();
    Module *module = F->getParent();
    if (module->findCJSModule(F)) {
      return ScopeData{module->getTopLevelFunction(), 1, false};
    }
  }

  ScopeData ret = ScopeData::orphan();
  if (ScopeDesc *parentScope = scopeDesc->getParent()) {
    ScopeData parentData =
        calculateFunctionScopeData(parentScope, nextScopeDepth(depth));
    if (!parentData.orphaned && (parentData.depth >= 0 || depth)) {
      assert(!depth || (depth == parentData.depth));
      ret = ScopeData(
          computeParent(scopeDesc, parentScope, parentData),
          parentData.depth + 1);
    }
  } else if (depth) {
    ret = ScopeData(nullptr, *depth);
  }

  LLVM_DEBUG({
    if (ret.orphaned) {
      dbgs() << "Orphaned scope in function \""
             << scopeDesc->getFunction()->getInternalNameStr() << "\"\n";
    }
  });

  lexicalScopeDescMap_[scopeDesc] = ret;
  return ret;
}

Optional<int32_t> FunctionScopeAnalysis::getScopeDepth(ScopeDesc *S) {
  ScopeData sd = calculateFunctionScopeData(S);
  if (sd.orphaned)
    return llvh::None;
  return sd.depth;
}

Function *FunctionScopeAnalysis::getLexicalParent(Function *F) {
  return calculateFunctionScopeData(F->getFunctionScopeDesc()).parent;
}

#undef DEBUG_TYPE
