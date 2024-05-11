/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ADT_OWNINGFOLDINGSET_H
#define HERMES_ADT_OWNINGFOLDINGSET_H

#include "llvh/ADT/FoldingSet.h"

namespace hermes {

/// Default creator for elements in the OwningFoldingSet, which constructs
/// elements with new.
template <typename T>
class OwningFoldingSetDefaultCreator {
 public:
  template <typename... Args>
  static T *create(Args &&...args) {
    return new T(std::forward<Args>(args)...);
  }
};

/// A FoldingSet that owns its entries and deletes them when it is destroyed.
template <
    typename T,
    typename Creator = OwningFoldingSetDefaultCreator<T>,
    class Deleter = std::default_delete<T>>
class OwningFoldingSet {
  llvh::FoldingSet<T> set_;

 public:
  explicit OwningFoldingSet(unsigned Log2InitSize = 6) : set_(Log2InitSize) {}
  OwningFoldingSet(OwningFoldingSet &&Arg) = default;
  OwningFoldingSet &operator=(OwningFoldingSet &&RHS) = default;

  ~OwningFoldingSet() {
    // The folding set is intrusive, so elements cannot be destroyed while they
    // are still inserted in the set. At the same time, removal of individual
    // elements from the set is rather slow as it involves iterating over
    // the bucket to find the previous node. So, instead we just save all nodes,
    // clear the set, and then delete them.
    llvh::SmallVector<T *, 8> toDelete{};
    toDelete.reserve(set_.size());
    for (T &entry : set_)
      toDelete.push_back(&entry);
    set_.clear();
    Deleter deleter;
    for (T *entry : toDelete)
      deleter(entry);
  }

  /// FindNodeOrInsertPos - Look up the node specified by ID.  If it exists,
  /// return it.  If not, return the insertion token that will make insertion
  /// faster.
  T *FindNodeOrInsertPos(const llvh::FoldingSetNodeID &ID, void *&InsertPos) {
    return set_.FindNodeOrInsertPos(ID, InsertPos);
  }

  /// InsertNode - Insert the specified node into the folding set, knowing that
  /// it is not already in the folding set.  InsertPos must be obtained from
  /// FindNodeOrInsertPos.
  T *InsertNode(std::unique_ptr<T, Deleter> N, void *InsertPos) {
    set_.InsertNode(N.get(), InsertPos);
    return N.release();
  }

  /// A helper method wrapping FindNodeOrInsertPos() and InsertNode()
  /// for types where the constructor parameters can be passed to a static
  /// method \c T::Profile() in the same order.
  /// The new node is created using
  /// <code>Creator::create(std::forward<Args>(args)...)</code>.
  ///
  /// \return a pair of the pointer to the node and a boolean indicating whether
  ///     a new node was inserted.
  template <typename... Args>
  std::pair<T *, bool> getOrEmplace(Args &&...args) {
    llvh::FoldingSetNodeID ID;
    T::Profile(ID, std::forward<Args>(args)...);
    void *insertPos = nullptr;
    if (auto *v = FindNodeOrInsertPos(ID, insertPos))
      return {v, false};
    return {
        InsertNode(
            std::unique_ptr<T, Deleter>(
                Creator::create(std::forward<Args>(args)...)),
            insertPos),
        true};
  }
};

} // namespace hermes

#endif
