/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_WEAKREFSLOT_H
#define HERMES_VM_WEAKREFSLOT_H

#include "hermes/VM/CompressedPointer.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/GCConcurrency.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/PointerBase.h"
#include "hermes/VM/WeakRoot.h"

namespace hermes {
namespace vm {

/// This is a single slot in the weak reference table. It contains a pointer to
/// a GC managed object. The GC will make sure it is updated when the object is
/// moved; if the object is garbage-collected, the pointer will be cleared.
class WeakRefSlot {
 public:
  // Mutator methods.

  /// Return true if this slot stores a non-null pointer to something.
  bool hasValue() const {
    assert(!isFree() && "Should never query a free WeakRef");
    return value_.root != CompressedPointer(nullptr);
  }

  /// Return the object as a GCCell *, with a read barrier
  inline GCCell *get(PointerBase &base, GC &gc) const;

  /// Same as get, but without a read barrier
  GCCell *getNoBarrierUnsafe(PointerBase &base) const {
    // Cannot check state() here because it can race with marking code.
    assert(hasValue() && "tried to access collected referent");
    return value_.root.getNonNullNoBarrierUnsafe(base);
  }

  CompressedPointer getNoBarrierUnsafe() const {
    assert(hasValue() && "tried to access collected referent");
    return value_.root.getNoBarrierUnsafe();
  }

  void markWeakRoots(WeakRootAcceptor &acceptor) {
    assert(!isFree() && "Cannot mark the weak root of a freed slot.");
    acceptor.acceptWeak(value_.root);
  }

  // GC methods to update slot when referent moves/dies.

  /// Update the stored pointer (because the object moved).
  void setPointer(CompressedPointer ptr) {
    // Cannot check state() here because it can race with marking code.
    value_.root = ptr;
  }

  /// Clear the pointer (because the object died).
  void clearPointer() {
    value_.root = CompressedPointer(nullptr);
  }

  /// GC methods to recycle slots.

  /// Set the slot to free, this can be called from either the mutator (when
  /// destroying a WeakRef) or the background thread (in finalizer).
  void free() {
    /// There are three operations related to this atomic variable:
    /// 1. Freeing the slot on background thread.
    /// 2. Checking if the slot is free on the mutator.
    /// 3. Freeing and reusing the slot on the mutator.
    /// Since the only operation that background thread can do with the slot is
    /// freeing it, we don't need a barrier to order the store.
    free_.store(true, std::memory_order_relaxed);
  }

  /// Methods required by ManagedChunkedList.

  WeakRefSlot() = default;

  /// \return true if the slot has been freed. As noted in free(), since
  /// background thread can only free a slot, we don't need a stricter order.
  bool isFree() const {
    return free_.load(std::memory_order_relaxed);
  }

  WeakRefSlot *getNextFree() {
    assert(isFree() && "Can only get nextFree on a free slot");
    return value_.nextFree;
  }

  void setNextFree(WeakRefSlot *nextFree) {
    assert(isFree() && "can only set nextFree on a free slot");
    value_.nextFree = nextFree;
  }

  /// Emplace new value to this slot.
  void emplace(CompressedPointer ptr) {
    assert(isFree() && "Slot must be free.");
    value_.root = ptr;
    free_.store(false, std::memory_order_relaxed);
  }

 private:
  /// When the slot is not free, root points to the referenced object, and is
  /// updated by GC when it moves/dies (which happens either on the mutator or
  /// background thread at the STW phase). When it's freed, nextFree is set to
  /// another freed slot and added to a freelist for reuse (currently this is
  /// delegated to ManagedChunkedList).
  union WeakRootOrIndex {
    WeakRoot<GCCell> root;
    WeakRefSlot *nextFree;
    WeakRootOrIndex() {}
  } value_;
  /// Atomic state represents whether the slot is free. It can be written by
  /// background thread (in finalizer) and read/written by mutator.
  std::atomic<bool> free_{true};
};

class JSObject;
class JSWeakMapImplBase;

/// Slot used by each entry in a WeakMap or WeakSet. The mapped value is stored
/// here so that GC can directly mark it if both its key and owner are alive,
/// eliminating the complexity of managing a separate values array. And the
/// WeakRoot to the owner is needed to correctly collect cycles, e.g.,
/// ```
/// var m = WeakMap();
/// var o = {};
/// m.set(o, m);
/// m = undefined;
/// ```
/// Note that all these fields must only be directly accessed by the GC, or
/// through the corresponding WeakMapEntryRef.
class WeakMapEntrySlot {
  /// The state of the slot.
  std::atomic_bool freed_{true};

 public:
  /// The owner (WeakMap/WeakSet) of this entry.
  WeakRoot<GCCell> owner;
  /// The WeakRoot to the key (or next free slot).
  union {
    WeakRoot<GCCell> key;
    WeakMapEntrySlot *nextFree;
  };
  /// The value mapped by the WeakRef key.
  /// NOTE: It's set to Empty only if either the key or the owner is null.
  PinnedHermesValue mappedValue;

  void markWeakRoots(WeakRootAcceptor &acceptor) {
    acceptor.acceptWeak(key);
    acceptor.acceptWeak(owner);
  }

  void free() {
    freed_.store(true, std::memory_order_relaxed);
  }

  /// Methods required by ManagedChunkedList

  WeakMapEntrySlot() {}

  bool isFree() const {
    return freed_.load(std::memory_order_relaxed);
  }

  WeakMapEntrySlot *getNextFree() const {
    assert(isFree() && "Can only get nextFree on a free slot");
    return nextFree;
  }

  void setNextFree(WeakMapEntrySlot *nextFree) {
    assert(isFree() && "can only set nextFree on a free slot");
    this->nextFree = nextFree;
  }

  /// Emplace new value to this slot.
  void emplace(
      CompressedPointer keyPtr,
      HermesValue value,
      CompressedPointer ownerPtr) {
    freed_.store(false, std::memory_order_relaxed);
    key = keyPtr;
    mappedValue = value;
    owner = ownerPtr;
  }
};

} // namespace vm
} // namespace hermes

#endif
