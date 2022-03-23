/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_WEAKREFSLOT_H
#define HERMES_VM_WEAKREFSLOT_H

#include "hermes/VM/HermesValue.h"

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

  WeakRefSlot(HermesValue v) {
    reset(v);
  }

  bool hasValue() const {
    // An empty value means the pointer has been cleared, and a native value
    // means it is free.
    // Don't use state_ here since that can be modified concurrently by the GC.
    assert(!value_.isNativeValue() && "Should never query a free WeakRef");
    return !value_.isEmpty();
  }

  /// Return the object as a HermesValue.
  const HermesValue value() const {
    // Cannot check state() here because it can race with marking code.
    assert(hasValue() && "tried to access collected referent");
    return value_;
  }

  // GC methods to update slot when referent moves/dies.

  /// Return true if this slot stores a non-null pointer to something. For any
  /// slot reachable by the mutator, that something is a GCCell.
  bool hasPointer() const {
    return value_.isPointer();
  }

  /// Return the pointer to a GCCell, whether or not this slot is marked.
  GCCell *getPointer() const {
    // Cannot check state() here because it can race with marking code.
    return static_cast<GCCell *>(value_.getPointer());
  }

  /// Update the stored pointer (because the object moved).
  void setPointer(void *newPtr) {
    // Cannot check state() here because it can race with marking code.
    value_ = value_.updatePointer(newPtr);
  }

  /// Clear the pointer (because the object died).
  void clearPointer() {
    value_ = HermesValue::encodeEmptyValue();
  }

  // GC methods to recycle slots.

  State state() const {
    return state_;
  }

  void mark() {
    assert(state() == Unmarked && "already marked");
    state_ = Marked;
  }

  void unmark() {
    assert(state() == Marked && "not yet marked");
    state_ = Unmarked;
  }

  void free(WeakRefSlot *nextFree) {
    assert(state() == Unmarked && "cannot free a reachable slot");
    state_ = Free;
    value_ = HermesValue::encodeNativePointer(nextFree);
    assert(state() == Free);
  }

  WeakRefSlot *nextFree() const {
    // nextFree is only called during a STW pause, so it's fine to access both
    // state and value here.
    assert(state() == Free);
    return value_.getNativePointer<WeakRefSlot>();
  }

  /// Re-initialize a freed slot.
  void reset(HermesValue v) {
    static_assert(Unmarked == 0, "unmarked state should not need tagging");
    state_ = Unmarked;
    assert(v.isPointer() && "Only pointers are currently supported");
    value_ = v;
    assert(state() == Unmarked && "initial state should be unmarked");
  }

 private:
  // value_ and state_ are read and written by different threads. We rely on
  // them being independent words so that they can be used without
  // synchronization.
  PinnedHermesValue value_;
  State state_;
};
using WeakSlotState = WeakRefSlot::State;

} // namespace vm
} // namespace hermes

#endif
