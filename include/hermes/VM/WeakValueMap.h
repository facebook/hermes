/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
//===----------------------------------------------------------------------===//
#ifndef HERMES_VM_WEAKVALUEMAP_H
#define HERMES_VM_WEAKVALUEMAP_H

#include "hermes/VM/WeakRef.h"

#include "llvm/ADT/DenseMap.h"

namespace hermes {
namespace vm {

/// A hash table where the values are weak references. Note that in the "usual"
/// weak map semantics the keys are weak, not the values.
/// The map consists of std::pair<KeyT, WeakRef<ValueT>> and that is what is
/// returned by dereferencing an iterator.
template <class KeyT, class ValueT>
class WeakValueMap {
  /// TODO: this is wasteful in terms of code size because it will be
  /// specialized for different kinds of ValueT while technically the underlying
  /// hash table always stores instances of WeakRefBase.
  using DenseMapT = llvm::SmallDenseMap<KeyT, WeakRef<ValueT>, 2>;
  using InternalIterator = typename DenseMapT::iterator;
  using const_iterator = typename DenseMapT::const_iterator;

 public:
  using key_type = typename DenseMapT::mapped_type;
  using mapped_type = typename DenseMapT::mapped_type;
  using value_type = typename DenseMapT::value_type;
  using size_type = typename DenseMapT::size_type;

  class iterator;

  WeakValueMap() : map_() {}

  WeakValueMap(const WeakValueMap &) = delete;
  WeakValueMap &operator=(const WeakValueMap &) = delete;

  /// \return true if the hash table is known to be empty.
  /// It can report false negatives because it doesn't prune invalid references
  /// - that would be too slow. In other words, if the table contains only
  /// invalid weak refs, it is not knownto be empty, and this will return false.
  bool isKnownEmpty() const {
    return map_.empty();
  }

  iterator begin() {
    return makeIterator(map_.begin(), true);
  }
  iterator end() {
    return makeIterator(map_.end());
  }

  /// Read only iterator. Don't skip or erase invalid entries.
  const_iterator const_begin() const {
    return map_.begin();
  }

  /// Read only iterator. Don't skip or erase invalid entries.
  const_iterator const_end() const {
    return map_.end();
  }

  /// Lookup a value by key. The returned iterator must be used right away,
  /// either to access the found element or to erase it.
  iterator find(const KeyT &key) {
    return makeIterator(internalFind(key));
  }

  /// Look for a key and return the value as Handle<T> if found or llvm::None if
  /// not found.
  llvm::Optional<Handle<ValueT>> lookup(
      HandleRootOwner *runtime,
      const KeyT &key) {
    auto it = internalFind(key);
    if (it == map_.end())
      return llvm::None;
    return it->second.get(runtime);
  }

  /// Remove the element at position \p it.
  void erase(iterator it) {
    map_.erase(it.it_);
    recalcPruneLimit();
  }

  /// Remove the element with key \p key and return true if it was found.
  bool erase(const KeyT &key) {
    auto it = internalFind(key);
    if (it == map_.end())
      return false;
    map_.erase(it);
    recalcPruneLimit();
    return true;
  }

  /// Insert a key/value into the map if the key is not already there.
  /// \return true if the pair was inserted, false if the key was already there.
  bool insertNew(GC *gc, const KeyT &key, Handle<ValueT> value) {
    if (!map_.try_emplace(key, WeakRef<ValueT>(gc, value)).second)
      return false;
    pruneInvalid();
    return true;
  }

  /// Insert key/value into the map. Used by deserialization.
  /// Use WeakRefSlot* to initialize WeakRefs directly. Don't prune entries.
  void insertUnsafe(GC *gc, const KeyT &key, WeakRefSlot *ptr) {
    auto res = map_.try_emplace(key, WeakRef<ValueT>(ptr)).second;
    if (!res) {
      hermes_fatal("shouldn't fail to insert during deserialization");
    }
  }

  /// This method should be invoked during garbage collection. It calls
  /// gc->markWeakRef() with every valid WeakRef in the map.
  void markWeakRefs(WeakRefAcceptor &acceptor) {
    for (auto it = map_.begin(), e = map_.end(); it != e; ++it) {
      if (it->second.isValid())
        acceptor.accept(it->second);
      else
        map_.erase(it);
    }
  }

  size_t getMemorySize() const {
    return map_.getMemorySize();
  }

  class iterator {
    friend class WeakValueMap;
    /// The map we are iterating. We need it so we can skip and delete
    /// invalidated entries. This is sub-optimal since DenseMap::iterator
    /// already stores an "end" pointer.
    DenseMapT *map_;
    InternalIterator it_;

    iterator(DenseMapT &map, InternalIterator it, bool skipInvalid)
        : map_(&map), it_(it) {
      if (skipInvalid)
        skipPastInvalid();
    }

    iterator &skipPastInvalid() {
      auto e = map_->end();
      // Skip over and delete invalid entries.
      while (it_ != e && !it_->second.isValid()) {
        // NOTE: DenseMap's erase() operation doesn't invalidate any iterators,
        // (and has a void return type), so this is safe to do. If we were using
        // a std::map<>, we would have to do "it = map_.erase(it)"
        map_->erase(it_);
        ++it_;
      }
      return *this;
    }

   public:
    value_type *operator->() const {
      assert(
          it_ != map_->end() && it_->second.isValid() &&
          "iterator points to invalid entry");
      return it_.operator->();
    }
    value_type &operator*() const {
      return *this->operator->();
    }

    iterator &operator++() {
      assert(it_ != map_->end() && "iterator points to end()");
      ++it_;
      return skipPastInvalid();
    }

    bool operator==(const iterator &other) const {
      return it_ == other.it_;
    }
    bool operator!=(const iterator &other) const {
      return it_ != other.it_;
    }
  };

 private:
  DenseMapT map_;

  static constexpr size_type kMinPruneLimit = 5;

  /// When the map grows beyond this size, the invalid WeakRef-s are deleted.
  size_type pruneLimit_{kMinPruneLimit};

  /// Construct a public iterator from an internal one.
  iterator makeIterator(InternalIterator it, bool skipInvalid = false) {
    return iterator{map_, it, skipInvalid};
  }

  /// Look for the specified key \p key in the internal map and return it if
  /// it is there and if the value is still valid. If the value is invalid
  /// delete it.
  /// \return the internal iterator or map_.end() if not found.
  InternalIterator internalFind(const KeyT &key) {
    auto it = map_.find(key);
    // Not found?
    if (it == map_.end())
      return it;

    // The value was garbage collected, so pretend that we didn't find it.
    if (!it->second.isValid()) {
      map_.erase(it);
      return map_.end();
    }

    return it;
  }

  /// If the size of the map has exceeded the prune limit, scan the map and
  /// delete the invalid WeakRef-s. Then recalculate the prune limit.
  void pruneInvalid() {
    if (map_.size() <= pruneLimit_)
      return;

    for (auto it = map_.begin(), e = map_.end(); it != e; ++it) {
      if (!it->second.isValid()) {
        // NOTE: DenseMap's erase() operation doesn't invalidate any iterators,
        // (and has a void return type), so this is safe to do. If we were using
        // a std::map<>, we would have to do "it = map_.erase(it)"
        map_.erase(it);
      }
    }

    recalcPruneLimit();
  }

  /// Update the prune limit to be the larger of the minimum or 2*size-1.
  void recalcPruneLimit() {
    pruneLimit_ = std::max(map_.size() * 2 + 1, toRValue(kMinPruneLimit));
  }
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_WEAKVALUEMAP_H
