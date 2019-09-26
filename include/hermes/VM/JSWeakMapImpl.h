/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_JSWEAKMAPIMPL_H
#define HERMES_VM_JSWEAKMAPIMPL_H

#include "hermes/VM/CallResult.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/WeakRef.h"

namespace hermes {
namespace vm {

namespace detail {

/// Used as the key to the DenseMap in JSWeakMapImpl.
/// Packages the hash with the WeakRef itself.
/// Uses WeakRefs using invalid locations as slots for empty and tombstone keys.
struct WeakRefKey {
  /// Actual weak ref stored as the key.
  WeakRef<JSObject> ref;

  /// GC-stable hash value of the JSObject pointed to by ref, while it's alive.
  uint32_t hash;

  WeakRefKey(WeakRef<JSObject> ref, uint32_t hash) : ref(ref), hash(hash) {}
};

/// Enable using WeakRef<JSObject> in DenseMap.
struct WeakRefInfo {
 public:
  /// \return Empty key which is simply null.
  static inline WeakRefKey getEmptyKey() {
    return WeakRefKey{
        WeakRef<JSObject>(reinterpret_cast<WeakRefSlot *>(kEmptyKey)),
        kEmptyKey};
  }
  /// \return Empty key which is simply a pointer to 0x1 - don't dereference.
  static inline WeakRefKey getTombstoneKey() {
    return WeakRefKey{
        WeakRef<JSObject>(reinterpret_cast<WeakRefSlot *>(kTombstoneKey)),
        kTombstoneKey};
  }
  /// \return the hash in \p key.
  static inline unsigned getHashValue(const WeakRefKey &key) {
    return key.hash;
  }
  /// \return true both arguments are empty, both are tombstone,
  /// or both point to the same JSObject.
  static inline bool isEqual(const WeakRefKey &a, const WeakRefKey &b) {
    const auto *aSlot = a.ref.unsafeGetSlot();
    const auto *bSlot = b.ref.unsafeGetSlot();
    // Check if the keys are empty or tombstones, to avoid deferencing them.
    if (aSlot == bSlot) {
      return true;
    }
    if (reinterpret_cast<uintptr_t>(aSlot) <= kTombstoneKey ||
        reinterpret_cast<uintptr_t>(bSlot) <= kTombstoneKey) {
      // aSlot != bSlot and one or both are empty or tombstone.
      return false;
    }
    // Check if the refs are valid.
    // The underlying object may have been collected already,
    // but markWeakRefs may not have run at this point.
    // The value will be cleared on the next GC cycle, but for now,
    // isValid needs to be checked to avoid an invalid vmcast.
    return WeakRef<JSObject>::isSlotValid(aSlot) &&
        WeakRef<JSObject>::isSlotValid(bSlot) &&
        vmcast<JSObject>(aSlot->value()) == vmcast<JSObject>(bSlot->value());
  }

 private:
  /// The empty key (used as a pointer and a hash).
  static constexpr uint32_t kEmptyKey = 0;

  /// The tombstone key (used as a pointer and a hash).
  static constexpr uint32_t kTombstoneKey = 1;

  static_assert(
      kEmptyKey < kTombstoneKey,
      "empty key must be less than tombstone key");
};

} // namespace detail

/// Base implementation of JSWeakMapImpl methods,
/// used by both WeakMap and WeakSet, with no templating.
class JSWeakMapImplBase : public JSObject {
  using Super = JSObject;
  using WeakRefKey = detail::WeakRefKey;
  using DenseMapT = llvm::DenseMap<WeakRefKey, uint32_t, detail::WeakRefInfo>;

 protected:
#ifdef HERMESVM_SERIALIZE
  JSWeakMapImplBase(Deserializer &d, const VTable *vt);

  friend void serializeJSWeakMapBase(Serializer &s, const GCCell *cell);
#endif

  JSWeakMapImplBase(
      Runtime *runtime,
      const VTable *vtp,
      JSObject *parent,
      HiddenClass *clazz,
      BigStorage *valueStorage)
      : JSObject(runtime, vtp, parent, clazz),
        valueStorage_(runtime, valueStorage, &runtime->getHeap()) {}

 public:
  static const ObjectVTable vt;

  static bool classof(const GCCell *cell) {
    return kindInRange(
        cell->getKind(),
        CellKind::WeakMapImplBaseKind_first,
        CellKind::WeakMapImplBaseKind_last);
  }

  static void WeakMapImplBaseBuildMeta(
      const GCCell *cell,
      Metadata::Builder &mb);

  /// Set a key/value, overwriting the previous value at that key,
  /// or add a new key/value if the key doesn't exist.
  static ExecutionStatus setValue(
      Handle<JSWeakMapImplBase> self,
      Runtime *runtime,
      Handle<JSObject> key,
      Handle<> value);

  /// Delete a key/value in the map.
  /// \return true if the key/value existed and was removed.
  static bool deleteValue(
      Handle<JSWeakMapImplBase> self,
      Runtime *runtime,
      Handle<JSObject> key);

  /// \return true if the \p key exists in the map.
  static bool hasValue(
      Handle<JSWeakMapImplBase> self,
      Runtime *runtime,
      Handle<JSObject> key);

  /// \return the value at \p key, if it exists. Else, return undefined.
  static HermesValue getValue(
      Handle<JSWeakMapImplBase> self,
      Runtime *runtime,
      Handle<JSObject> key);

  /// \return the size of the internal map.
  /// Used for testing purposes.
  /// Note: the returned value may differ from the number of reachable keys.
  static uint32_t debugGetSize(JSWeakMapImplBase *self) {
    return self->map_.size();
  }

 protected:
  static void _finalizeImpl(GCCell *cell, GC *gc) {
    auto *self = vmcast<JSWeakMapImplBase>(cell);
    self->~JSWeakMapImplBase();
  }

  /// Mark weak references and set hasFreeableSlots_ if invalidated slots
  /// were found.
  static void _markWeakImpl(GCCell *cell, WeakRefAcceptor &acceptor);

  static size_t _mallocSizeImpl(GCCell *cell) {
    auto *self = vmcast<JSWeakMapImplBase>(cell);
    return self->getMallocSize();
  }

  /// \return the number of bytes allocated by this object on the heap.
  size_t getMallocSize() const {
    return map_.getMemorySize();
  }

  /// Iterate the slots in map_ and call deleteInternal on any invalid
  /// references, adding all available slots to the free list.
  void findAndDeleteFreeSlots(PointerBase *base);

  /// Erase the map entry and corresponding valueStorage entry
  /// pointed to by the iterator \p it.
  /// Add the newly opened valueStorage slot to the free list.
  void deleteInternal(PointerBase *base, DenseMapT::iterator it);

 private:
  /// Get the index to insert a new value into valueStorage_.
  /// Resize valueStorage_ if no such spots exist.
  /// \return the index into which to insert, or EXCEPTION if resize failed.
  static CallResult<uint32_t> getFreeValueStorageIndex(
      Handle<JSWeakMapImplBase> self,
      Runtime *runtime);

 private:
  /// The underlying weak value map.
  DenseMapT map_;

  /// Value storage, used to keep the HermesValues out of the DenseMap.
  /// If a value is kFreeListInvalid it's free to use.
  /// If a value can be found by following indices starting at freeListHead_,
  /// it's also free to use.
  /// The last member of this linked list of free spots is kFreeListInvalid.
  GCPointer<BigStorage> valueStorage_;

  /// Number indicating the end of the free list.
  static constexpr uint32_t kFreeListInvalid{UINT32_MAX};

  /// Index of the first free spot in the valueStorage_ free list.
  /// If kFreeListInvalid, then the free list is completely empty,
  /// and the nextIndex_ should be used.
  uint32_t freeListHead_{kFreeListInvalid};

  /// Next index to use when the free list runs out of elements.
  uint32_t nextIndex_{0};

  /// This is set to true when markWeakRefs sees an invalid slot which can be
  /// freed later.
  /// When set to true, the WeakMap should look for free slots the next time
  /// the user tries to add an element.
  bool hasFreeableSlots_{false};
};

/// Underlying representation of the WeakMap and WeakSet objects.
///
/// The keys are stored in a `DenseMap<WeakRefKey, uint32_t>`,
/// which allows storing the hashes with the keys,
/// so that the DenseMap can compute the hashes
/// without being given a Runtime.
/// We can't store the hashes separately because the DenseMap
/// needs to be able to compute hashes based on only the KeyT,
/// because the map rehashes when growing itself,
/// and it doesn't store the LookupKeyT if using find_as or insert_as.
///
/// The values in the map_ are indices into valueStorage_,
/// which is where we store the actual values.
/// valueStorage_ is a GC-managed PropStorage,
/// and the indices in map_ are gotten either via the free list
/// or, if that's empty, via the nextIndex_ field.
/// If we run out of slots, we double the size of valueStorage_.
/// Currently, we never shrink it.
template <CellKind C>
class JSWeakMapImpl final : public JSWeakMapImplBase {
  using Super = JSWeakMapImplBase;

 public:
  static const ObjectVTable vt;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == C;
  }

  /// Create a new WeakMap with prototype property \p parentHandle.
  static CallResult<HermesValue> create(
      Runtime *runtime,
      Handle<JSObject> parentHandle);

  static void WeakMapOrSetBuildMeta(const GCCell *cell, Metadata::Builder &mb);

 protected:
#ifdef HERMESVM_SERIALIZE
  explicit JSWeakMapImpl(Deserializer &d);

  friend void WeakMapDeserialize(Deserializer &d, CellKind kind);
  friend void WeakSetDeserialize(Deserializer &d, CellKind kind);
#endif

  JSWeakMapImpl(
      Runtime *runtime,
      JSObject *parent,
      HiddenClass *clazz,
      BigStorage *valueStorage)
      : JSWeakMapImplBase(runtime, &vt.base, parent, clazz, valueStorage) {}
};

using JSWeakMap = JSWeakMapImpl<CellKind::WeakMapKind>;
using JSWeakSet = JSWeakMapImpl<CellKind::WeakSetKind>;

} // namespace vm
} // namespace hermes

#endif
