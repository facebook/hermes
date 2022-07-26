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
  /// State of this slot for the purpose of reusing slots.
  enum State {
    Unmarked = 0, /// Unknown whether this slot is in use by the mutator.
    Marked, /// Proven to be in use by the mutator.
    Free /// Proven to NOT be in use by the mutator.
  };

  // Mutator methods.

  WeakRefSlot(CompressedPointer ptr) {
    reset(ptr);
  }

  /// Return true if this slot stores a non-null pointer to something.
  bool hasValue() const {
    // This assert should be predicated on kConcurrentGC being false, because it
    // is not safe to access the state in a concurrent GC
    assert(
        (kConcurrentGC || (state_ != Free)) &&
        "Should never query a free WeakRef");
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
    if (state_ != State::Free) {
      acceptor.acceptWeak(value_.root);
    }
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

  // GC methods to recycle slots.

  State state() const {
    return state_;
  }

  void mark() {
    assert(state() != Free && "Cannot mark a free slot.");
    state_ = Marked;
  }

  void unmark() {
    assert(state() == Marked && "not yet marked");
    state_ = Unmarked;
  }

  void free(WeakRefSlot *nextFree) {
    assert(state() == Unmarked && "cannot free a reachable slot");
    state_ = Free;
    value_.nextFree = nextFree;
    assert(state() == Free);
  }

  WeakRefSlot *nextFree() const {
    // nextFree is only called during a STW pause, so it's fine to access both
    // state and value here.
    assert(state() == Free);
    return value_.nextFree;
  }

  /// Re-initialize a freed slot.
  void reset(CompressedPointer ptr) {
    state_ = Marked;
    value_.root = ptr;
  }

 private:
  // value_ and state_ are read and written by different threads. We rely on
  // them being independent words so that they can be used without
  // synchronization.
  union WeakRootOrIndex {
    WeakRoot<GCCell> root;
    WeakRefSlot *nextFree;
    WeakRootOrIndex() {}
  } value_;
  State state_;
};

using WeakSlotState = WeakRefSlot::State;

} // namespace vm
} // namespace hermes

#endif
