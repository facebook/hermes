/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/VM/ArrayStorage.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/Handle.h"
#include "hermes/VM/Metadata.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/WeakRef.h"

#include "llvh/ADT/SmallVector.h"
#include "llvh/Support/ErrorHandling.h"

namespace hermes::vm {

/// Impl class for WeakValueObjectMap that handles GC integration and metadata.
/// This class contains the template-independent parts.
/// Users must create WeakValueObjectMap directly.
class WeakValueObjectMapImpl : public GCCell {
 public:
  friend void WeakValueObjectMapBuildMeta(
      const GCCell *cell,
      Metadata::Builder &mb);
  using size_type = uint32_t;

 private:
  /// Minimum capacity of a non-empty map.
  static constexpr size_type kMinCapacity = 8;

  /// Maximum capacity of a non-empty map.
  static constexpr size_type kMaxCapacity =
      ArrayStorage::maxCapacityNoOverflow();

  /// The multiplier that the hash table grows or shrinks by each rehash.
  static constexpr size_type kGrowOrShrinkFactor = 2;

  /// The initial (minimum) pruneLimit_.
  static constexpr size_type kMinPruneLimit = 5;

 protected:
  /// Storage for the GC-managed keys (strong references).
  GCPointer<ArrayStorage> keys_{nullptr};

  /// Storage for the WeakRef values, allocated with calloc.
  /// The capacity_ of the WeakValueObjectMap is the number of elements of the
  /// values_ array.
  /// Raw pointer to avoid calling a WeakRef constructor for unused entries.
  /// All the memory management is manual.
  WeakRef<GCCell> *values_{nullptr};

  /// Bit vector of the same size as the values_ array, indicating
  /// whether a WeakRef has been allocated at the given index.
  /// Store this explicitly because keys_ will not be accessible from the
  /// finalizer to check for validity.
  llvh::BitVector allocatedWeakRefs_{};

  /// The number of allocated slots in the keys_ and values_ arrays.
  size_type capacity_{0};

  /// Number of elements known to be stored in the map.
  /// NOTE: May overcount, because weak references may have been freed at some
  /// point without the size_ being updated.
  /// Used only to compute the pruneLimit_.
  size_type size_{0};

  /// When the map grows beyond this size, the invalid WeakRef-s are deleted.
  size_type pruneLimit_{kMinPruneLimit};

 protected:
  WeakValueObjectMapImpl() = default;

 public:
  ~WeakValueObjectMapImpl();

  WeakValueObjectMapImpl(const WeakValueObjectMapImpl &) = delete;
  WeakValueObjectMapImpl &operator=(const WeakValueObjectMapImpl &) = delete;

  static const VTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::WeakValueObjectMapKind;
  }

  static bool classof(const GCCell *cell) {
    return cell->getKind() == getCellKind();
  }

  /// Return true if the map is known to be empty.
  /// May return false negatives if it contains only invalid weak references.
  bool isKnownEmpty() const {
    return capacity() == 0;
  }

  /// Return the current capacity of the hash table.
  size_type capacity() const {
    return capacity_;
  }

  /// Return the memory size of the base components.
  size_t getMemorySize() const {
    return sizeof(values_[0]) * capacity_;
  }

 protected:
  /// Find the index for \p key in the hash table.
  /// If no slot is found, return capacity_.
  /// If a slot is found, the caller can check the keys_ element at that slot
  /// to see if it contains an Object HermesValue to see if the key was already
  /// inserted.
  size_type findIndex(Runtime &runtime, JSObject *key) const;

  /// Rehash the hash table with the specified new capacity.
  static void rehash(
      Handle<WeakValueObjectMapImpl> self,
      Runtime &runtime,
      size_type newCapacity);

  /// Insert a new key-value pair.
  /// \return true if inserted, false if insertion failed (either the key
  /// already exists or we failed to allocate enough in the ArrayStorage).
  static bool insertNew(
      Handle<WeakValueObjectMapImpl> self,
      Runtime &runtime,
      JSObject *key,
      GCCell *value);

  /// Remove the entry with the given key.
  /// \return true if the key was found and removed.
  static bool erase(
      Handle<WeakValueObjectMapImpl> self,
      Runtime &runtime,
      JSObject *key,
      GC &gc);

 private:
  /// Return true if the table should grow.
  bool shouldGrow(size_type capacity, size_type size) const {
    // Grow when load factor exceeds 0.75.
    return capacity == 0 || size >= capacity / 4 * 3;
  }

  /// Return true if the table should shrink.
  bool shouldShrink(size_type capacity, size_type size) const {
    // Shrink when size is 1/8 of capacity, but don't shrink below minimum.
    return capacity_ > kMinCapacity && size_ <= capacity_ / 8;
  }

  /// Update the prune limit to be the larger of the minimum or 2*size-1.
  void recalcPruneLimit() {
    pruneLimit_ = std::max(size_ * 2 + 1, toRValue(kMinPruneLimit));
  }

  /// Prune invalid references from the hash table.
  static void
  pruneInvalid(Handle<WeakValueObjectMapImpl> self, Runtime &runtime, GC &gc);

  /// Free all non-GC managed resources associated with the object.
  static void _finalizeImpl(GCCell *cell, GC &gc);

  /// \return the amount of non-GC memory being used by the given \p cell, which
  /// is assumed to be a HiddenClass.
  static size_t _mallocSizeImpl(GCCell *cell);
};

/// A map where the values are weak references and the keys are JSObject.
/// Note that in the "usual" weak map semantics the keys are weak, not the
/// values.
/// This is a type-specialized wrapper around WeakValueObjectMapImpl,
/// so users must instantiate this directly.
///
/// The keys are stored in ArrayStorage as HermesValue and the values
/// are stored as std::vector of WeakRefs in the base class.
/// It's implemented as a quadratically probed hash table.
template <class ValueT>
class WeakValueObjectMap : public WeakValueObjectMapImpl {
 public:
  WeakValueObjectMap() = default;

  /// Base class destructor handles cleanup.
  ~WeakValueObjectMap() = default;

  /// Create a new WeakValueObjectMap with the specified initial capacity.
  static WeakValueObjectMap<ValueT> *create(Runtime &runtime) {
    return runtime.makeAFixed<WeakValueObjectMap, HasFinalizer::Yes>();
  }

  /// Look up the value for the given key.
  /// \return nullptr if key not found or if the weak value has been freed.
  ValueT *lookup(Runtime &runtime, JSObject *key) const {
    NoAllocScope noAlloc{runtime};
    size_type idx = WeakValueObjectMapImpl::findIndex(runtime, key);
    if (idx < capacity()) {
      ArrayStorage *keys = keys_.getNonNull(runtime);
      HermesValue storedKey = keys->at(idx);
      if (storedKey.isObject() && values_[idx].isValid()) {
        return vmcast<ValueT>(values_[idx].get(runtime));
      }
    }
    return nullptr;
  }

  /// \return true if the map contains the given key with a valid value.
  bool containsKey(Runtime &runtime, JSObject *key) const {
    NoAllocScope noAlloc{runtime};
    return lookup(runtime, key) != nullptr;
  }

  /// Insert a new key-value pair.
  /// \return true if inserted, false if insertion failed (either the key
  /// already exists or we failed to allocate enough in the ArrayStorage).
  static bool insertNew(
      Handle<WeakValueObjectMap> self,
      Runtime &runtime,
      JSObject *key,
      ValueT *value) {
    return WeakValueObjectMapImpl::insertNew(self, runtime, key, value);
  }

  /// Remove the entry with the given key.
  /// \return true if the key was found and removed.
  static bool erase(
      Handle<WeakValueObjectMap> self,
      Runtime &runtime,
      JSObject *key,
      GC &gc) {
    return WeakValueObjectMapImpl::erase(self, runtime, key, gc);
  }
};

} // namespace hermes::vm
