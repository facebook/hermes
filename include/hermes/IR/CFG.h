/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_IR_CFG_H
#define HERMES_IR_CFG_H

#include "llvh/ADT/GraphTraits.h"
#include "llvh/Support/GenericDomTree.h"

#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"

using llvh::cast;

namespace hermes {

//===----------------------------------------------------------------------===//
// BasicBlock pred_iterator definition
//===----------------------------------------------------------------------===//

template <class Ptr, class USER_iterator> // Predecessor Iterator
class PredIterator {
  typedef PredIterator<Ptr, USER_iterator> Self;
  USER_iterator It;
  USER_iterator ItEnd;

  inline void advancePastNonTerminators() {
    // Loop to ignore non-terminator uses (for example BlockAddresses).
    while (It != ItEnd && !llvh::dyn_cast<TerminatorInst>(*It))
      ++It;
  }

 public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = Ptr;
  using difference_type = std::ptrdiff_t;
  using pointer = Ptr *;
  using reference = Ptr *;

  PredIterator() = default;

  explicit inline PredIterator(Ptr *bb)
      : It(bb->users_begin()), ItEnd(bb->users_end()) {
    advancePastNonTerminators();
  }

  inline PredIterator(Ptr *bb, bool)
      : It(bb->users_end()), ItEnd(bb->users_end()) {}

  inline bool operator==(const Self &x) const {
    return It == x.It;
  }

  inline bool operator!=(const Self &x) const {
    return !operator==(x);
  }

  inline reference operator*() const {
    assert(It != ItEnd && "pred_iterator out of range!");
    return cast<TerminatorInst>(*It)->getParent();
  }

  inline pointer *operator->() const {
    return &operator*();
  }

  inline Self &operator++() { // Preincrement
    assert(It != ItEnd && "pred_iterator out of range!");
    ++It;
    advancePastNonTerminators();
    return *this;
  }

  inline Self operator++(int) { // Postincrement
    Self tmp = *this;
    ++*this;
    return tmp;
  }
};

typedef PredIterator<BasicBlock, Value::iterator> pred_iterator;
typedef PredIterator<const BasicBlock, Value::const_iterator>
    const_pred_iterator;
using pred_range = llvh::iterator_range<pred_iterator>;
using pred_const_range = llvh::iterator_range<const_pred_iterator>;

inline pred_iterator pred_begin(BasicBlock *BB) {
  return pred_iterator(BB);
}

inline const_pred_iterator pred_begin(const BasicBlock *BB) {
  return const_pred_iterator(BB);
}

inline pred_iterator pred_end(BasicBlock *BB) {
  return pred_iterator(BB, true);
}

inline const_pred_iterator pred_end(const BasicBlock *BB) {
  return const_pred_iterator(BB, true);
}

inline bool pred_empty(const BasicBlock *BB) {
  return pred_begin(BB) == pred_end(BB);
}

inline pred_range predecessors(BasicBlock *BB) {
  return pred_range(pred_begin(BB), pred_end(BB));
}

inline pred_const_range predecessors(const BasicBlock *BB) {
  return pred_const_range(pred_begin(BB), pred_end(BB));
}

inline bool pred_contains(const BasicBlock *BB, const BasicBlock *Search) {
  return std::find(pred_begin(BB), pred_end(BB), Search) != pred_end(BB);
}

inline unsigned pred_count(const BasicBlock *BB) {
  return std::distance(pred_begin(BB), pred_end(BB));
}

inline unsigned pred_count_unique(const BasicBlock *BB) {
  llvh::SmallPtrSet<const BasicBlock *, 8> preds(pred_begin(BB), pred_end(BB));
  return preds.size();
}

//===----------------------------------------------------------------------===//
// BasicBlock succ_iterator helpers
//===----------------------------------------------------------------------===//

typedef llvh::SuccIterator<TerminatorInst, BasicBlock> succ_iterator;
typedef llvh::SuccIterator<const TerminatorInst, const BasicBlock>
    succ_const_iterator;
using succ_range = llvh::iterator_range<succ_iterator>;
using succ_const_range = llvh::iterator_range<succ_const_iterator>;

inline succ_iterator succ_begin(BasicBlock *BB) {
  return succ_iterator(BB->getTerminator());
}

inline succ_const_iterator succ_begin(const BasicBlock *BB) {
  return succ_const_iterator(BB->getTerminator());
}

inline succ_iterator succ_end(BasicBlock *BB) {
  return succ_iterator(BB->getTerminator(), true);
}

inline succ_const_iterator succ_end(const BasicBlock *BB) {
  return succ_const_iterator(BB->getTerminator(), true);
}

inline bool succ_empty(const BasicBlock *BB) {
  return succ_begin(BB) == succ_end(BB);
}

inline succ_range successors(BasicBlock *BB) {
  return succ_range(succ_begin(BB), succ_end(BB));
}

inline succ_const_range successors(const BasicBlock *BB) {
  return succ_const_range(succ_begin(BB), succ_end(BB));
}

inline bool succ_contains(const BasicBlock *BB, const BasicBlock *Search) {
  return std::find(succ_begin(BB), succ_end(BB), Search) != succ_end(BB);
}

inline unsigned succ_count(const BasicBlock *BB) {
  return std::distance(succ_begin(BB), succ_end(BB));
}

} // end namespace hermes

namespace llvh {

//===----------------------------------------------------------------------===//
// GraphTraits for BasicBlock
//===----------------------------------------------------------------------===//
template <>
struct GraphTraits<hermes::BasicBlock *> {
  using NodeType = hermes::BasicBlock;
  using NodeRef = hermes::BasicBlock *;

  using ChildIteratorType = hermes::succ_iterator;

  static NodeType *getEntryNode(NodeType *BB) {
    return BB;
  }

  static ChildIteratorType child_begin(NodeType *N) {
    return succ_begin(N);
  }
  static ChildIteratorType child_end(NodeType *N) {
    return succ_end(N);
  }
};

template <>
struct GraphTraits<Inverse<hermes::BasicBlock *>> {
  using NodeType = hermes::BasicBlock;
  using NodeRef = hermes::BasicBlock *;

  using ChildIteratorType = hermes::pred_iterator;

  static NodeType *getEntryNode(Inverse<NodeType *> G) {
    return G.Graph;
  }
  static inline ChildIteratorType child_begin(NodeType *N) {
    return pred_begin(N);
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return pred_end(N);
  }
};

template <>
struct GraphTraits<hermes::Function *>
    : public GraphTraits<hermes::BasicBlock *> {
  using GraphType = hermes::Function *;

  static NodeType *getEntryNode(GraphType F) {
    return &F->front();
  }

  typedef pointer_iterator<hermes::Function::iterator> nodes_iterator;
  static nodes_iterator nodes_begin(GraphType F) {
    return nodes_iterator(F->begin());
  }
  static nodes_iterator nodes_end(GraphType F) {
    return nodes_iterator(F->end());
  }
  static unsigned size(GraphType F) {
    return F->size();
  }
};

template <>
struct GraphTraits<Inverse<hermes::Function *>>
    : public GraphTraits<Inverse<hermes::BasicBlock *>> {
  using GraphType = Inverse<hermes::Function *>;

  static NodeType *getEntryNode(GraphType F) {
    return &F.Graph->front();
  }

  typedef pointer_iterator<hermes::Function::iterator> nodes_iterator;
  static nodes_iterator nodes_begin(GraphType F) {
    return nodes_iterator(F.Graph->begin());
  }
  static nodes_iterator nodes_end(GraphType F) {
    return nodes_iterator(F.Graph->end());
  }
  static unsigned size(GraphType F) {
    return F.Graph->size();
  }
};

} // namespace llvh

//===----------------------------------------------------------------------===//
// Dominators
//===----------------------------------------------------------------------===//

extern template class llvh::DominatorTreeBase<hermes::BasicBlock, false>;
extern template class llvh::DomTreeNodeBase<hermes::BasicBlock>;

namespace hermes {

using DominanceInfoNode = llvh::DomTreeNodeBase<BasicBlock>;

/// A class for computing basic dominance info.
class DominanceInfo : public llvh::DominatorTreeBase<BasicBlock, false> {
 public:
  explicit DominanceInfo(Function *F);

  bool properlyDominates(const Instruction *A, const Instruction *B) const;

  using DominatorTreeBase::properlyDominates;

  bool isValid(Function *F) const {
    return getNode(&F->front());
  }

  void reset() {
    llvh::DominatorTreeBase<BasicBlock, false>::reset();
  }
};

} // end namespace hermes

#endif
