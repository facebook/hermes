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
std::vector<BasicBlock *> postOrderAnalysis(Function *F);

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
  using LexicalScopeMap = llvh::DenseMap<const Function *, ScopeData>;
  LexicalScopeMap lexicalScopeMap_{};

  /// Recursively calculate the scope data of a function \p F.
  /// \return the ScopeData of the function.
  ScopeData calculateFunctionScopeData(Function *F);

 public:
  explicit FunctionScopeAnalysis(const Function *entryPoint) {
    lexicalScopeMap_[entryPoint] = ScopeData(nullptr, 0);
  }

  /// Lazily get the scope depth of \p VS.
  llvh::Optional<int32_t> getScopeDepth(VariableScope *VS);

  /// Lazily get the lexical parent of \p F, or nullptr if none.
  Function *getLexicalParent(Function *F);
};

/// A namespace encapsulating utilities for implementing optimization passes
/// based on a DFS visit of a dominator tree.
namespace DomTreeDFS {

/// StackNode - contains the needed information to create a stack for doing
/// a depth first traversal of a dominator tree.
/// It can be subclassed to attach more information like scoped tables, etc.
class StackNode {
 public:
  StackNode(const StackNode &) = delete;
  void operator=(const StackNode &) = delete;

  StackNode(const DominanceInfoNode *n)
      : node_(n), childIter_(n->begin()), endIter_(n->end()), done_(false) {}
  /// A convenience constructor matching the signature of the derived class.
  StackNode(void *, const DominanceInfoNode *n) : StackNode(n) {}

  /// The dominator tree node associated with this stack node.
  const DominanceInfoNode *node() {
    return node_;
  }

 private:
  template <typename Derived, typename StackNode>
  friend class Visitor;

  /// The dominator tree node associated with this stack node.
  const DominanceInfoNode *node_;
  /// The next child of the dominance tree node to process.
  DominanceInfoNode::const_iterator childIter_;
  /// The end iterator of child dominance tree nodes.
  const DominanceInfoNode::const_iterator endIter_;
  /// This flag indicates that this dominance tree node has been processed
  /// and we have moved onto iterating its children.
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

  StackNode *newStackEntry(const DominanceInfoNode *n) {
    auto *sn = nodeAllocator_.Allocate();
    return new (sn) StackNode(derived(), n);
  }
  void freeStackEntry(StackNode *n) {
    n->~StackNode();
    nodeAllocator_.Deallocate(n);
  }

 protected:
  const DominanceInfo &DT_;

  Visitor(const DominanceInfo &DT) : DT_(DT) {}

  /// Starting DFS from root node.
  /// \return the changed flag.
  bool DFS() {
    return DFS(DT_.getRootNode());
  }

  /// Starting DFS from a specific node.
  /// \return the changed flag.
  bool DFS(const DominanceInfoNode *DIN) {
    llvh::SmallVector<StackNode *, 4> stack{};

    bool changed = false;

    // Process the root node.
    stack.push_back(newStackEntry(DIN));

    // Process the stack.
    while (!stack.empty()) {
      // Grab the first item off the stack. Set the current generation, remove
      // the node from the stack, and process it.
      StackNode *toProcess = stack.back();

      // Check if the node needs to be processed.
      if (!toProcess->done_) {
        // Process the node.
        changed |= derived()->processNode(toProcess);
        // This node has been processed.
        toProcess->done_ = true;
      } else if (toProcess->childIter_ != toProcess->endIter_) {
        auto *dn = *toProcess->childIter_++;
        // Push the next child onto the stack.
        stack.push_back(newStackEntry(dn));
      } else {
        // It has been processed, and there are no more children to process,
        // so pop it off the stack
        freeStackEntry(stack.pop_back_val());
      }
    }

    return changed;
  }
};

} // namespace DomTreeDFS

/// Construct a map from each basic block in \p F to the number of enclosing
/// try's. Note that unreachable blocks, and blocks with a nesting depth of 0,
/// are excluded from the map.
/// \return a pair containing the map, and the maximum nesting depth of try's in
/// this function.
std::pair<llvh::DenseMap<BasicBlock *, size_t>, size_t> getBlockTryDepths(
    Function *F);

} // end namespace hermes

#endif
