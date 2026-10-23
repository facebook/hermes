/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/VM/Runtime.h"
#include "hermes/VM/WeakRef.h"

namespace hermes::vm {

/// Specialize this for every ValueT stored in a WeakValueObjectMap.
/// It must provide:
///   static JSObject *get(ValueT *value, Runtime &runtime);
/// which returns the key that \p value was inserted with.
/// The key must never change for the lifetime of \p value. A non-null key must
/// be kept alive by \p value. \p value is an unrooted weak target read without
/// a read barrier, so get() must not allocate and must only read the key.
template <class ValueT>
struct WeakValueObjectMapKey;

/// Impl class for WeakValueObjectMap that contains the template-independent
/// parts. Users must create WeakValueObjectMap directly.
class WeakValueObjectMapImpl {
 public:
  using size_type = uint32_t;

  /// Return the key of a live value. See WeakValueObjectMapKey.
  using KeyOfFn = JSObject *(*)(GCCell * value, Runtime &runtime);

 private:
  /// Minimum capacity of a non-empty map.
  static constexpr size_type kMinCapacity = 8;

  /// The hash table, allocated with calloc. Keys are not stored: the key of a
  /// live entry is computed from its value with a KeyOfFn. Storing the keys
  /// strongly would keep a key alive for as long as the map, and a key that
  /// can reach its own value would keep that value alive too.
  /// Raw pointer to avoid calling a WeakRef constructor for unused entries.
  /// All the memory management is manual.
  /// Entries have three states:
  /// - empty: null slot_. Ends a probe sequence.
  /// - dead: non-null slot_ whose value has been collected. Acts as a
  ///   tombstone and is reused by inserts.
  /// - live: valid slot_.
  WeakRef<GCCell> *values_{nullptr};

  /// The number of allocated entries in values_.
  size_type capacity_{0};

  /// Number of non-empty entries (live + dead).
  /// Used in shouldRehashBeforeInsert to ensure there are always empty entries
  /// for probing termination. Dead entries are only removed by a rehash, which
  /// recomputes this from scratch; nothing else ever decrements it.
  size_type nonEmptyEntryCount_{0};

 protected:
  WeakValueObjectMapImpl() = default;

 public:
  ~WeakValueObjectMapImpl();

  WeakValueObjectMapImpl(const WeakValueObjectMapImpl &) = delete;
  WeakValueObjectMapImpl &operator=(const WeakValueObjectMapImpl &) = delete;

  /// Return true if the map is known to be empty.
  /// May return false negatives: once storage has been allocated the map is
  /// never "known empty" again, even if every value has since been collected.
  bool isKnownEmpty() const {
    return capacity() == 0;
  }

  /// Return the current capacity of the hash table.
  size_type capacity() const {
    return capacity_;
  }

  /// \return the size of the malloc'd storage. The owner must report it as
  /// part of its own malloc size.
  size_t getMemorySize() const {
    return sizeof(values_[0]) * capacity_;
  }

#ifdef HERMES_MEMORY_INSTRUMENTATION
  /// Add a weak edge to every live value, named \p name followed by an index.
  /// The owner must call this from its own snapshotAddEdges.
  void snapshotAddEdges(GC &gc, HeapSnapshot &snap, llvh::StringRef name) const;
#endif

 protected:
  /// Find the index for \p key in the hash table.
  /// If the table has no storage yet (capacity_ == 0), return capacity_.
  /// Otherwise a slot is always returned: either the live one holding \p key,
  /// or the one \p key should be inserted into (the first dead entry on the
  /// probe sequence, or else the empty entry that ended it). The caller can
  /// check whether the entry is valid to see if the key was found.
  size_type findIndex(Runtime &runtime, JSObject *key, KeyOfFn keyOf) const;

  /// Look up the live value for \p key.
  /// \return nullptr if \p key is not in the map.
  GCCell *lookup(Runtime &runtime, JSObject *key, KeyOfFn keyOf) const;

  /// Map \p key to \p value, overwriting any live value for \p key.
  /// \pre keyOf(value) == key.
  void insert(Runtime &runtime, JSObject *key, GCCell *value, KeyOfFn keyOf);

 private:
  /// The number of non-empty entries (live + dead) that the table may hold at
  /// \p capacity. Keeps at least a quarter of the entries empty so that
  /// probing always terminates.
  static constexpr size_type rehashThreshold(size_type capacity) {
    return capacity / 4 * 3;
  }

  /// \return true if the table must be rehashed before another entry is added.
  bool shouldRehashBeforeInsert() const {
    return (nonEmptyEntryCount_ + 1) > rehashThreshold(capacity_);
  }

  /// Rehash the live entries into a table sized for them plus one more, with
  /// room to grow. This grows, shrinks, or just drops dead entries, as needed.
  void rehash(Runtime &runtime, KeyOfFn keyOf);
};

/// A map from JSObject keys to weakly held values.
/// Note that in the "usual" weak map semantics the keys are weak, not the
/// values.
/// Keys are not stored; WeakValueObjectMapKey<ValueT> recovers the key from a
/// live value. So an entry keeps neither its key nor its value alive, and is
/// dropped once its value is collected.
/// Null is a valid key.
/// It's implemented as a quadratically probed hash table in malloc'd memory,
/// and nothing in it lives on the GC heap, so it can be embedded in a GCCell.
/// The owner must report getMemorySize() in its malloc size.
template <class ValueT>
class WeakValueObjectMap : public WeakValueObjectMapImpl {
  /// Adapt WeakValueObjectMapKey<ValueT> to KeyOfFn.
  static JSObject *keyOf(GCCell *value, Runtime &runtime) {
    return WeakValueObjectMapKey<ValueT>::get(vmcast<ValueT>(value), runtime);
  }

 public:
  /// Create an empty map. The hash table storage is allocated lazily on the
  /// first insert.
  WeakValueObjectMap() = default;

  /// Look up the value for the given key.
  /// \return nullptr if key not found or if the weak value has been freed.
  ValueT *lookup(Runtime &runtime, JSObject *key) const {
    return vmcast_or_null<ValueT>(
        WeakValueObjectMapImpl::lookup(runtime, key, keyOf));
  }

  /// \return true if the map contains the given key with a valid value.
  bool containsKey(Runtime &runtime, JSObject *key) const {
    return lookup(runtime, key) != nullptr;
  }

  /// Map \p key to \p value, overwriting any live value for \p key.
  /// \pre WeakValueObjectMapKey<ValueT>::get(value) == key.
  void insert(Runtime &runtime, JSObject *key, ValueT *value) {
    WeakValueObjectMapImpl::insert(runtime, key, value, keyOf);
  }
};

} // namespace hermes::vm
