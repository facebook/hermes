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

/// Visit basic blocks in post order starting from \p BB, appending them into \p
/// order.
static void visitPostOrder(BasicBlock *BB, std::vector<BasicBlock *> &order) {
  struct State {
    BasicBlock *BB;
    succ_iterator cur, end;
    explicit State(BasicBlock *BB)
        : BB(BB), cur(succ_begin(BB)), end(succ_end(BB)) {}
  };

  llvh::SmallPtrSet<BasicBlock *, 16> visited{};
  llvh::SmallVector<State, 32> stack{};

  stack.emplace_back(BB);
  visited.insert(BB);
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

std::vector<BasicBlock *> hermes::postOrderAnalysis(Function *F) {
  std::vector<BasicBlock *> order;
  BasicBlock *entry = &*F->begin();

  // Finally, do an PO scan from the entry block.
  visitPostOrder(entry, order);

  assert(
      !order.empty() && order[order.size() - 1] == entry &&
      "Entry block must be the last element in the vector");
  return order;
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

std::pair<llvh::DenseMap<BasicBlock *, size_t>, size_t>
hermes::getBlockTryDepths(Function *F) {
  // Map basic blocks inside a try to the number of try statements they are
  // nested in.
  llvh::DenseMap<BasicBlock *, size_t> blockTryDepths;

  // The maximum nesting depth we have observed so far.
  size_t maxDepth = 0;

  // Stack of basic blocks to visit. The second element in the pair represents
  // the nesting depth on entry to that block.
  llvh::SmallVector<std::pair<BasicBlock *, size_t>, 4> stack;

  llvh::DenseSet<BasicBlock *> visited;
  visited.insert(&F->front());
  stack.push_back({&F->front(), 0});

  while (!stack.empty()) {
    auto [BB, depth] = stack.pop_back_val();

    // If the block starts with a TryEndInst or CatchInst, it ends the nearest
    // try, decrement the depth for this block and all successors.
    if (llvh::isa<TryEndInst>(&BB->front()) ||
        llvh::isa<CatchInst>(&BB->front()))
      --depth;

    if (depth) {
      maxDepth = std::max(maxDepth, depth);
      blockTryDepths.try_emplace(BB, depth);
    }

    // If the block ends with a TryStartInst, increment the depth
    size_t newDepth =
        llvh::isa<TryStartInst>(BB->getTerminator()) ? depth + 1 : depth;
    for (auto *succ : successors(BB))
      if (visited.insert(succ).second)
        stack.push_back({succ, newDepth});
  }
  return {std::move(blockTryDepths), maxDepth};
}

#undef DEBUG_TYPE
