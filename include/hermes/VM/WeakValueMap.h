/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
#ifndef HERMES_VM_WEAKVALUEMAP_H
#define HERMES_VM_WEAKVALUEMAP_H

#include "hermes/VM/HandleRootOwner.h"
#include "hermes/VM/WeakRef.h"

#include "llvh/ADT/DenseMap.h"

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
  using DenseMapT = llvh::SmallDenseMap<KeyT, WeakRef<ValueT>, 8>;
  using InternalIterator = typename DenseMapT::iterator;

 public:
  using key_type = typename DenseMapT::mapped_type;
  using mapped_type = typename DenseMapT::mapped_type;
  using value_type = typename DenseMapT::value_type;
  using size_type = typename DenseMapT::size_type;

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

  /// Invoke \p callback on each (const) key and value. Values may be invalid.
  template <typename CallbackFunction>
  void forEachEntry(const CallbackFunction &callback) const {
    for (auto it = map_.begin(), e = map_.end(); it != e; ++it) {
      callback(it->first, it->second);
    }
  }

  /// Return true if there is an entry with the given key and a valid value.
  bool containsKey(const KeyT &key, GC *gc) {
    WeakRefLock lk{gc->weakRefMutex()};
    return containsKeyLocked(key, gc);
  }

  bool containsKeyLocked(const KeyT &key, GC *gc) {
    return internalFindLocked(key, gc) != map_.end();
  }

  /// Look for a key and return the value as Handle<T> if found or llvh::None if
  /// not found.
  llvh::Optional<Handle<ValueT>>
  lookup(HandleRootOwner *runtime, GC *gc, const KeyT &key) {
    WeakRefLock lk{gc->weakRefMutex()};
    return lookupLocked(runtime, gc, key);
  }

  llvh::Optional<Handle<ValueT>>
  lookupLocked(HandleRootOwner *runtime, GC *gc, const KeyT &key) {
    auto it = internalFindLocked(key, gc);
    if (it == map_.end())
      return llvh::None;
    return it->second.getLocked(runtime, gc);
  }

  /// Remove the element with key \p key and return true if it was found.
  bool erase(const KeyT &key, GC *gc) {
    WeakRefLock lk{gc->weakRefMutex()};
    auto it = internalFindLocked(key, gc);
    if (it == map_.end())
      return false;
    map_.erase(it);
    recalcPruneLimit();
    return true;
  }

  /// Insert a key/value into the map if the key is not already there.
  /// \return true if the pair was inserted, false if the key was already there.
  bool insertNew(GC *gc, const KeyT &key, Handle<ValueT> value) {
    WeakRefLock lk{gc->weakRefMutex()};
    return insertNewLocked(gc, key, value);
  }

  bool insertNewLocked(GC *gc, const KeyT &key, Handle<ValueT> value) {
    if (!map_.try_emplace(key, WeakRef<ValueT>(gc, value)).second)
      return false;
    pruneInvalid(gc);
    return true;
  }

  /// Insert key/value into the map. Used by deserialization.
  /// Use WeakRefSlot* to initialize WeakRefs directly. Don't prune entries.
  void insertUnsafe(const KeyT &key, WeakRefSlot *ptr) {
    auto res = map_.try_emplace(key, WeakRef<ValueT>(ptr)).second;
    if (!res) {
      hermes_fatal("shouldn't fail to insert during deserialization");
    }
  }

  /// This method should be invoked during garbage collection. It calls
  /// the acceptor with every valid WeakRef in the map.
  void markWeakRefs(WeakRefAcceptor &acceptor) {
    const auto &mtx = acceptor.mutexRef();
    // The weak ref lock should be held here by the GC.
    for (auto it = map_.begin(), e = map_.end(); it != e; ++it) {
      if (it->second.isValid(mtx))
        acceptor.accept(it->second);
      else
        map_.erase(it);
    }
  }

  size_t getMemorySize() const {
    return map_.getMemorySize();
  }

 private:
  DenseMapT map_;

  static constexpr size_type kMinPruneLimit = 5;

  /// When the map grows beyond this size, the invalid WeakRef-s are deleted.
  size_type pruneLimit_{kMinPruneLimit};

  /// Look for the specified key \p key in the internal map and return it if
  /// it is there and if the value is still valid. If the value is invalid
  /// delete it.
  /// \return the internal iterator or map_.end() if not found.
  /// \pre This function must be called while the weak ref mutex is held.
  InternalIterator internalFindLocked(const KeyT &key, GC *gc) {
    assert(
        gc->weakRefMutex() &&
        "WeakRefMutex must be held before entering this function");
    auto it = map_.find(key);
    // Not found?
    if (it == map_.end())
      return it;

    // The value was garbage collected, so pretend that we didn't find it.
    // Accessing isValid requires a lock to be held.
    if (!it->second.isValid(gc->weakRefMutex())) {
      map_.erase(it);
      return map_.end();
    }

    return it;
  }

  /// If the size of the map has exceeded the prune limit, scan the map and
  /// delete the invalid WeakRef-s. Then recalculate the prune limit.
  void pruneInvalid(GC *gc) {
    assert(
        gc->weakRefMutex() &&
        "Weak ref mutex must be held before calling this function");
    if (map_.size() <= pruneLimit_)
      return;

    for (auto it = map_.begin(), e = map_.end(); it != e; ++it) {
      if (!it->second.isValid(gc->weakRefMutex())) {
        // NOTE: DenseMap's erase() operation doesn't invalidate any
        // iterators, (and has a void return type), so this is safe to do. If
        // we were using a std::map<>, we would have to do
        // "it = map_.erase(it)".
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
