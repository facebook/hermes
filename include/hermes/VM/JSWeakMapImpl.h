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
/// Uses WeakRefs using invalid locations as slots for empty and tombstone keys.
struct WeakRefKey {
  /// Actual weak ref stored as the key.
  WeakMapEntryRef ref;

  /// GC-stable hash value of the JSObject pointed to by ref, while it's alive.
  uint32_t hash;

  WeakRefKey(WeakMapEntryRef ref, uint32_t hash) : ref(ref), hash(hash) {}
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

/// Enable using WeakRef<JSObject> in DenseMap.
struct WeakRefInfo {
  /// \return Empty key which is simply null.
  static inline WeakRefKey getEmptyKey() {
    // Hashes of Empty/Tombstone key do not matter, since the key to be inserted
    // or searched can never be one of these two values. So we just pass a
    // constant 0 here.
    return WeakRefKey{WeakMapEntryRef::createEmptyEntryRef(), 0};
  }
  /// \return Empty key which is simply a pointer to 0x1 - don't dereference.
  static inline WeakRefKey getTombstoneKey() {
    return WeakRefKey{WeakMapEntryRef::createTombstoneEntryRef(), 0};
  }
  /// \return the hash in \p key.
  static inline unsigned getHashValue(const WeakRefKey &key) {
    return key.hash;
  }
  /// \return the hash in lookup key.
  static inline unsigned getHashValue(const WeakRefLookupKey &key) {
    return key.hash;
  }
  /// \return true both arguments are empty, both are tombstone,
  /// or both point to the same JSObject.
  static inline bool isEqual(const WeakRefKey &a, const WeakRefKey &b) {
    return a.ref.isKeyEqual(b.ref);
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
    return b.ref.isKeyEqual(a.refCellPtr);
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
      element.ref.releaseSlot();
    }
    self->~JSWeakMapImplBase();
  }

  /// Mark weak references and set hasFreeableSlots_ if invalidated slots
  /// were found.
  /// \pre The weak ref mutex must be held.
  static void _markWeakImpl(GCCell *cell, WeakRefAcceptor &acceptor);

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
