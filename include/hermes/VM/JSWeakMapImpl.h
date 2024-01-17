/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSWEAKMAPIMPL_H
#define HERMES_VM_JSWEAKMAPIMPL_H

#include "hermes/VM/CallResult.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/WeakRef.h"

namespace hermes {
namespace vm {

namespace detail {

/// Used as the key to the DenseSet in JSWeakMapImpl.
/// Packages the hash with the WeakRef itself.
/// Uses invalid locations as slots for empty and tombstone keys.
class WeakRefKey {
  /// Used to construct Empty/Tombstone WeakRefKey.
  static constexpr uint32_t kEmptyKey = 0;
  static constexpr uint32_t kTombstoneKey = 1;

  /// Slot to the key object (packaged with its mapped value and owning map).
  WeakMapEntrySlot *slot_;

  /// GC-stable hash value of the JSObject pointed to by ref, while it's alive.
  uint32_t hash_;

  explicit WeakRefKey(WeakMapEntrySlot *slot, uint32_t hash)
      : slot_(slot), hash_(hash) {}

 public:
  WeakRefKey(
      Runtime &runtime,
      Handle<JSObject> key,
      HermesValue value,
      JSWeakMapImplBase *ownerMapPtr)
      : slot_(
            runtime.getHeap().allocWeakMapEntrySlot(*key, value, ownerMapPtr)),
        hash_(runtime.gcStableHashHermesValue(key)) {}

  /// \return a pointer to the key object with read barrier. This should not be
  /// called when the weak ref key object is collected.
  JSObject *getKeyNonNull(PointerBase &base, GC &gc) const {
    assert(isKeyValid() && "tried to access collected weak ref object");
    return static_cast<JSObject *>(slot_->key.getNonNull(base, gc));
  }

  /// \return The mapped value by the the key object.
  HermesValue getMappedValue(GC &gc) const {
    // During marking phase in Hades, mutator thread may read a mapped value A
    // and store it to a marked object B, then deletes the entry. In
    // completeMarking(), A may become unreachable and gets swept, leaving a
    // dangling reference in B. To address it, we add a read barrier on every
    // access to mapped value. Adding writer barrier should also work but this
    // is cleaner.
    gc.weakRefReadBarrier(slot_->mappedValue);
    return slot_->mappedValue;
  }

  /// Set mapped value in slot_ to \p value.
  void setMappedValue(HermesValue value) {
    slot_->mappedValue = value;
  }

  /// Create an empty key to be used in DenseMap.
  static WeakRefKey createEmptyKey() {
    return WeakRefKey{
        reinterpret_cast<WeakMapEntrySlot *>(kEmptyKey), kEmptyKey};
  }

  /// Create a tombstone key to be used in DenseMap.
  static WeakRefKey createTombstoneKey() {
    return WeakRefKey{
        reinterpret_cast<WeakMapEntrySlot *>(kTombstoneKey), kTombstoneKey};
  }

  /// \return true if the WeakRoot to the key object is still alive.
  bool isKeyValid() const {
    return !!slot_->key;
  }

  /// \return true if the underlying slots are equal, or point to the same key
  /// object.
  bool isKeyEqual(const WeakRefKey &other) const {
    // Comparing when the slots are equal should always succeed. This also
    // handles comparison with tombstone and empty.
    if (slot_ == other.slot_)
      return true;
    // If the slots don't match and either one is tombstone or empty,
    // the comparison has failed (so we won't dereference them below).
    if (reinterpret_cast<uintptr_t>(slot_) <= kTombstoneKey ||
        reinterpret_cast<uintptr_t>(other.slot_) <= kTombstoneKey)
      return false;

    // If either key has been garbage collected, it is impossible for them to
    // compare equal. Otherwise, check for reference equality between the keys.
    return slot_->key && other.slot_->key &&
        slot_->key.getNoBarrierUnsafe() ==
        other.slot_->key.getNoBarrierUnsafe();
  }

  /// \return true if the underlying slot points to the same key object as
  /// \p keyObject. This is only used by WeakRefLookupKey, so \p keyObject
  /// should never be null.
  bool isKeyEqual(CompressedPointer keyObject) const {
    if (reinterpret_cast<uintptr_t>(slot_) <= kTombstoneKey)
      return false;
    return slot_->key.getNoBarrierUnsafe() == keyObject;
  }

  /// Free the WeakMapEntrySlot held by this reference.
  void releaseSlot() {
    slot_->free();
  }

  uint32_t getHash() const {
    return hash_;
  }
};

/// Used as a lookup key to the DenseSet in JSWeakMapImpl, so that we don't
/// need to allocate a new WeakMapEntrySlot for query operations. With this,
/// the only case that we will allocate a new slot is when inserting new values.
struct WeakRefLookupKey {
  /// Pointer of the key object.
  CompressedPointer refCellPtr;

  /// GC-stable hash value of the JSObject pointed to by ref, while it's alive.
  uint32_t hash;

  WeakRefLookupKey(Runtime &runtime, Handle<JSObject> keyObj)
      : refCellPtr(CompressedPointer::encode(*keyObj, runtime)),
        hash(runtime.gcStableHashHermesValue(keyObj)) {}
};

/// Enable using WeakRefKey in DenseMap.
struct WeakRefInfo {
  /// \return Empty key which is simply null.
  static inline WeakRefKey getEmptyKey() {
    return WeakRefKey::createEmptyKey();
  }
  /// \return Empty key which is simply a pointer to 0x1 - don't dereference.
  static inline WeakRefKey getTombstoneKey() {
    return WeakRefKey::createTombstoneKey();
  }
  /// \return the hash in \p key.
  static inline unsigned getHashValue(const WeakRefKey &key) {
    return key.getHash();
  }
  /// \return the hash in lookup key.
  static inline unsigned getHashValue(const WeakRefLookupKey &key) {
    return key.hash;
  }
  /// \return true both arguments are empty, both are tombstone,
  /// or both point to the same JSObject.
  static inline bool isEqual(const WeakRefKey &a, const WeakRefKey &b) {
    return a.isKeyEqual(b);
  }
  /// \return true if \p b is neither empty nor tombstone, and points to the
  /// same JSObject as refCellPtr of \p a (which should never be null pointer).
  /// In case that the key object in \p b is already garbage collected, but the
  /// WeakMapEntrySlot owned by \p b is not freed yet, this function is still
  /// correct. The hash stored in \p b does not change after the key object is
  /// collected, and the equality check in isKeyEqual() always fails since no
  /// object will ever compare equal to an object that has been freed.
  static inline bool isEqual(const WeakRefLookupKey &a, const WeakRefKey &b) {
    assert(a.refCellPtr && "LookupKey should not use a null object pointer");
    return b.isKeyEqual(a.refCellPtr);
  }
};

} // namespace detail

/// Base implementation of JSWeakMapImpl methods,
/// used by both WeakMap and WeakSet, with no templating.
class JSWeakMapImplBase : public JSObject {
  using Super = JSObject;
  using WeakRefKey = detail::WeakRefKey;
  using DenseSetT = llvh::DenseSet<WeakRefKey, detail::WeakRefInfo>;

 protected:
  JSWeakMapImplBase(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz)
      : JSObject(runtime, *parent, *clazz),
        targetSize_(/* sizingWeight */ 0.5, /* initSize */ 8) {}

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
      Runtime &runtime,
      Handle<JSObject> key,
      Handle<> value);

  /// Delete a key/value in the map.
  /// \return true if the key/value existed and was removed.
  static bool deleteValue(
      Handle<JSWeakMapImplBase> self,
      Runtime &runtime,
      Handle<JSObject> key);

  /// \return true if the \p key exists in the map.
  static bool hasValue(
      Handle<JSWeakMapImplBase> self,
      Runtime &runtime,
      Handle<JSObject> key);

  /// \return the value at \p key, if it exists. Else, return undefined.
  static HermesValue getValue(
      Handle<JSWeakMapImplBase> self,
      Runtime &runtime,
      Handle<JSObject> key);

  /// \return the size of the internal set, after freeing any freeable slots
  /// and erasing their owning keys. Used for testing purposes.
  static uint32_t debugFreeSlotsAndGetSize(
      Runtime &runtime,
      JSWeakMapImplBase *self);

  /// Iterate every slot owned by entries in `set_`, free invalid ones, and
  /// erase the owning entry in `set_` immediately. In the end, recompute
  /// `targetSize_` using the new size of `set_`.
  void clearFreeableEntries();

  /// An iterator over the keys of the map.
  using KeyIterator = DenseSetT::iterator;

 protected:
  static void _finalizeImpl(GCCell *cell, GC &gc) {
    auto *self = vmcast<JSWeakMapImplBase>(cell);
    for (auto &element : self->set_) {
      // No need to explicitly erase the owning entry of this slot since the
      // whole map/set is to be deleted.
      element.releaseSlot();
    }
    self->~JSWeakMapImplBase();
  }

  static size_t _mallocSizeImpl(GCCell *cell) {
    auto *self = vmcast<JSWeakMapImplBase>(cell);
    return self->getMallocSize();
  }

#ifdef HERMES_MEMORY_INSTRUMENTATION
  static void _snapshotAddEdgesImpl(GCCell *cell, GC &gc, HeapSnapshot &snap);
  static void _snapshotAddNodesImpl(GCCell *cell, GC &gc, HeapSnapshot &snap);
#endif

 public:
  // Public for tests.

  /// Lazily fetches the ID for the DenseMap used in this class. After it has
  /// been assigned once it'll stay constant.
  HeapSnapshot::NodeID getMapID(GC &gc);

  /// \return the number of bytes allocated by this object on the heap.
  size_t getMallocSize() const {
    return set_.getMemorySize();
  }

 private:
  /// The underlying weak value map.
  DenseSetT set_;

  /// When the size of `set_` reaches targetSize_, clearFreeableEntries() will
  /// be called to remove freeable entries and recompute targetSize_.
  ExponentialMovingAverage targetSize_;

  /// Target occupancy ration of the map storage.
  static constexpr double kOccupancyTarget = 0.5;
};

/// Underlying representation of the WeakMap and WeakSet objects.
///
/// The key/value pairs are stored in a `DenseSet<WeakRefKey>`. Each
/// `WeakRefKey` includes the underlying `WeakMapEntryRef` and hash of the key
/// object, which allows the DenseSet to compute the hash without relying on
/// the Runtime, and keep the set internally consistent even when the referenced
/// object is freed. Each `WeakMapEntryRef` uniquely owns a `WeakMapEntrySlot`,
/// which stores three things: the WeakRoot to key object, the mapped value,
/// and WeakRoot to the owning map.
/// The size of `set_` keeps growing when new key/value pairs are inserted,
/// until reaching `targetSize_`, at which point all freeable entries are
/// removed from `set_`, the WeakMapEntrySlots owned by them are freed, and
/// `targetSize_` is recomputed.
/// Note that when an entry is removed from `set_`, its underlying slot must be
/// freed together, otherwise, that slot could be used by another key and
/// causing insertion failure.
/// Currently, we only free slots at three places:
/// 1. In deleteValue(), if the key exists.
/// 2. In clearFreeableEntries(), which is called only in setValue() and
/// debugFreeSlotsAndGetSize(). All invalid slots are freed and owning entries
/// are erased from `set_`.
/// 3. In finalizer. All slots owned by entries in `set_` are freed and the
/// entire map/set is destructed.
template <CellKind C>
class JSWeakMapImpl final : public JSWeakMapImplBase {
  using Super = JSWeakMapImplBase;

 public:
  static const ObjectVTable vt;

  static constexpr CellKind getCellKind() {
    return C;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == C;
  }

  /// Create a new WeakMap with prototype property \p parentHandle.
  static CallResult<PseudoHandle<JSWeakMapImpl<C>>> create(
      Runtime &runtime,
      Handle<JSObject> parentHandle);

  static void WeakMapOrSetBuildMeta(const GCCell *cell, Metadata::Builder &mb);

  JSWeakMapImpl(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz)
      : JSWeakMapImplBase(runtime, parent, clazz) {}
};

using JSWeakMap = JSWeakMapImpl<CellKind::JSWeakMapKind>;
using JSWeakSet = JSWeakMapImpl<CellKind::JSWeakSetKind>;

} // namespace vm
} // namespace hermes

#endif
