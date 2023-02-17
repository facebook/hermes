/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_IR_ANALYSIS_H
#define HERMES_IR_ANALYSIS_H

#include "hermes/IR/CFG.h"
#include "hermes/IR/IR.h"
#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/Optional.h"
#include "llvh/ADT/SmallPtrSet.h"
#include "llvh/Support/RecyclingAllocator.h"

namespace hermes {

/// This is an implementation of the post-order scan. We use our own
/// implementation and not the LLVM RPO analysis because we currently have
/// basic blocks that are not linked to the entry blocks (catch blocks),
/// and LLVM's graph traits expect all blocks to be reachable from the entry
/// blocks. The analysis does not enumerate unreachable blocks.
class PostOrderAnalysis {
  using BlockList = std::vector<BasicBlock *>;

  /// The AST context, which here is only used by Dump().
  Context &ctx_;

  /// Holds the ordered list of basic blocks.
  BlockList Order;

  /// This function does the recursive scan of the function. \p BB is the basic
  /// block that starts the scan. \p order is the ordered list of blocks, and
  /// the output.
  static void visitPostOrder(BasicBlock *BB, BlockList &order);

 public:
  explicit PostOrderAnalysis(Function *F);

  void dump();

  using iterator = decltype(Order)::iterator;
  using const_iterator = decltype(Order)::const_iterator;
  using reverse_iterator = decltype(Order)::reverse_iterator;
  using const_reverse_iterator = decltype(Order)::const_reverse_iterator;

  using range = llvh::iterator_range<iterator>;
  using const_range = llvh::iterator_range<const_iterator>;
  using reverse_range = llvh::iterator_range<reverse_iterator>;
  using const_reverse_range = llvh::iterator_range<const_reverse_iterator>;

  inline iterator begin() {
    return Order.begin();
  }
  inline iterator end() {
    return Order.end();
  }
  inline reverse_iterator rbegin() {
    return Order.rbegin();
  }
  inline reverse_iterator rend() {
    return Order.rend();
  }
  inline const_iterator begin() const {
    return Order.begin();
  }
  inline const_iterator end() const {
    return Order.end();
  }
  inline const_reverse_iterator rbegin() const {
    return Order.rbegin();
  }
  inline const_reverse_iterator rend() const {
    return Order.rend();
  }
};

/// This analysis finds out which blocks are part of loops, and identifies
/// header and preheader blocks. All blocks in cycles of the CFG are considered
/// part of a loop, but they don't all have a unique header (dominating entry
/// point) and preheader (dominating predecessor of the header). Each loop can
/// have neither, just a header, or both (but not just a preheader).
///
/// Here's an example loop CFG:
/// \code
///      BB0     preheader, not in loop
///       |
///       v
///  +-> BB1     header, in loop
///  |    |
///  |    v
///  +-- BB2     in loop
///       |
///       v
///      BB3     not in loop
/// \endcode
class LoopAnalysis {
  template <typename T>
  using BlockMap = llvh::SmallDenseMap<const BasicBlock *, T, 16>;

  /// Mapping from each block to the header of the enclosing loop, or to null if
  /// the block is in a cycle but has no unique header.
  BlockMap<BasicBlock *> blockToHeader_{};
  /// Mapping from each header block to its preheader block.
  BlockMap<BasicBlock *> headerToPreheader_{};

 public:
  explicit LoopAnalysis(Function *F, const DominanceInfo &dominanceInfo);

  /// \returns True if \p BB is in a loop.
  bool isBlockInLoop(const BasicBlock *BB) const {
    // Note that this also returns true when BB is in a cycle but has no header,
    // since we map it to null in blockToHeader_ for that case.
    return blockToHeader_.count(BB);
  }
  /// \returns True if \p BB is the header block of a loop.
  bool isBlockHeader(const BasicBlock *BB) const {
    return getLoopHeader(BB) == BB;
  }
  /// \returns The header block of the loop enclosing \p BB, or null if \p BB is
  /// not in a loop with a unique header.
  BasicBlock *getLoopHeader(const BasicBlock *BB) const;
  /// \returns The preheader block of the loop enclosing \p BB, or null if \p BB
  /// is not in a loop with a unique header and preheader.
  BasicBlock *getLoopPreheader(const BasicBlock *BB) const;
};

/// This analysis generates the scope info for each function.
/// Global code has scope depth of 0. All other functions
/// have depth bigger than 0.
class FunctionScopeAnalysis {
  struct ScopeData {
    /// The parent of the function for which scope data was computed.
    Function *parent;

    /// The depth in the scope chain. The global scope has depth 0, each
    /// function nesting level increases this by 1. Placeholder functions (which
    /// represent the lexical environment in local eval) have negative depths.
    int32_t depth;

    /// Indicates that the function has no scope data, because while the
    /// function was added to the bytecode module, no instruction could be found
    /// to create it.
    bool orphaned;

    ScopeData(
        Function *parent = nullptr,
        int32_t depth = 0,
        bool orphaned = false)
        : parent(parent), depth(depth), orphaned(orphaned) {}

    /// Convenience function. \return an orphaned ScopeData.
    static ScopeData orphan() {
      return ScopeData(nullptr, 0, true);
    }
  };

  using LexicalScopeDescMap = llvh::DenseMap<const ScopeDesc *, ScopeData>;
  LexicalScopeDescMap lexicalScopeDescMap_{};

  /// Recursively calculate the scope data of \p scopeDesc. \p depth is
  /// specified during analysis initialization so scopes before the top level
  /// can be initialized.
  /// \return the ScopeData of the function.
  ScopeData calculateFunctionScopeData(
      ScopeDesc *scopeDesc,
      llvh::Optional<int> depth = llvh::None);

  static Function *computeParent(
      ScopeDesc *thisScope,
      ScopeDesc *parentScope,
      const ScopeData &sd);

 public:
  explicit FunctionScopeAnalysis(Function *entryPoint) {
    ScopeData data =
        calculateFunctionScopeData(entryPoint->getFunctionScopeDesc(), 0);
    assert(!data.orphaned && data.depth == 0);
    (void)data;
  }

  /// Lazily get the scope depth of \p S.
  llvh::Optional<int32_t> getScopeDepth(ScopeDesc *S);

  /// Lazily get the lexical parent of \p F, or nullptr if none.
  Function *getLexicalParent(Function *F);
};

/// A namespace encapsulating utilities for implementing optimization passes
/// based on a DFS visit of a dominator tree.
namespace DomTreeDFS {

/// StackNode - contains all the needed information to create a stack for
/// doing a depth first traversal of the tree. This includes scopes for values
/// and loads as well as the generation. There is a child iterator so that the
/// children do not need to be stored separately.
template <typename Visitor>
class StackNode {
 public:
  StackNode(const StackNode &) = delete;
  void operator=(const StackNode &) = delete;

  StackNode(Visitor *, const DominanceInfoNode *n)
      : node_(n), childIter_(n->begin()), endIter_(n->end()), done_(false) {}

  // Accessors.
  const DominanceInfoNode *node() {
    return node_;
  }
  // Return nullptr when the end is reached, or the next child otherwise.
  DominanceInfoNode *nextChild() {
    return childIter_ == endIter_ ? nullptr : *childIter_++;
  }
  bool isDone() {
    return done_;
  }
  void markDone() {
    done_ = true;
  }

 private:
  // Members.
  const DominanceInfoNode *node_;
  DominanceInfoNode::const_iterator childIter_;
  const DominanceInfoNode::const_iterator endIter_;
  bool done_;
};

/// A DFS visitor for nodes in a dominator tree. Derived needs to implement:
/// bool processNode(StackNode &);
template <typename Derived, typename StackNode>
class Visitor {
  llvh::RecyclingAllocator<llvh::BumpPtrAllocator, StackNode> nodeAllocator_;

  Derived *derived() {
    return static_cast<Derived *>(this);
  }

  StackNode *newNode(const DominanceInfoNode *n) {
    auto *sn = nodeAllocator_.Allocate();
    return new (sn) StackNode(derived(), n);
  }
  void freeNode(StackNode *n) {
    n->~StackNode();
    nodeAllocator_.Deallocate(n);
  }

 protected:
  const DominanceInfo &DT_;

  Visitor(const DominanceInfo &DT) : DT_(DT) {}

  /// Starting DFS from root node.
  bool DFS() {
    return DFS(DT_.getRootNode());
  }

  /// Starting DFS from a specific node.
  bool DFS(const DominanceInfoNode *DIN) {
    llvh::SmallVector<StackNode *, 4> nodesToProcess;

    bool changed = false;

    // Process the root node.
    nodesToProcess.push_back(newNode(DIN));

    // Process the stack.
    while (!nodesToProcess.empty()) {
      // Grab the first item off the stack. Set the current generation, remove
      // the node from the stack, and process it.
      StackNode *toProcess = nodesToProcess.back();

      // Check if the node needs to be processed.
      if (!toProcess->isDone()) {
        // Process the node.
        changed |= derived()->processNode(toProcess);
        // This node has been processed.
        toProcess->markDone();
      } else if (auto *child = toProcess->nextChild()) {
        // Push the next child onto the stack.
        nodesToProcess.push_back(newNode(child));
      } else {
        // It has been processed, and there are no more children to process,
        // so pop it off the stack
        freeNode(nodesToProcess.pop_back_val());
      }
    }

    return changed;
  }
};

} // namespace DomTreeDFS

} // end namespace hermes

#endif
