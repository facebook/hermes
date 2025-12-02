/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SERIALIZATIONMANAGEDVALUE_H
#define HERMES_VM_SERIALIZATIONMANAGEDVALUE_H
#include "hermes/VM/HermesValue.h"
namespace hermes {
namespace vm {
/// During serialization and deserialization, we keep track of HermesValues we
/// have already serialized/deserialized. In both cases, GC needs to keep
/// track of these values across collections. We do this by adding
/// an occupied SerializationManagedValue entry to the ManagedChunkList for each
/// HermesValue we are tracking. When GC marks roots, it will iterate through
/// the list and mark every occupied element. At the end of
/// serialization/deserialization, the corresponding entries are freed.
struct SerializationManagedValue {
 private:
  bool isFree_;
  uint32_t gcHash_;
  union {
    PinnedHermesValue hv_;
    SerializationManagedValue *nextFree_;
  };

 public:
  SerializationManagedValue() : isFree_(true) {}

  /// Returns whether this element is currently free.
  bool isFree() const {
    return isFree_;
  }

  /// Stores the HermesValue and the hash at this element and mark this entry as
  /// occupied.
  void emplace(HermesValue value, uint32_t hash) {
    assert(isFree_ && "Cannot emplace an occupied element");
    isFree_ = false;
    gcHash_ = hash;
    hv_ = PinnedHermesValue(value);
  }

  /// Get the next free element after this entry. This should not be called
  /// unless this element is free.
  SerializationManagedValue *getNextFree() {
    assert(
        isFree_ && "Current element must be free to access next free element");
    return nextFree_;
  }

  /// Set the next free element after this entry to add to the free list. This
  /// should not be called unless this element is free.
  void setNextFree(SerializationManagedValue *nextFree) {
    assert(isFree_ && "Current element be free to set next free element");
    nextFree_ = nextFree;
  }

  /// Mark this element as free so that the element may be garbage collected as
  /// needed.
  void markAsFree() {
    assert(!isFree_ && "Current element is already free");
    isFree_ = true;
  }

  const PinnedHermesValue &value() const {
    assert(!isFree_ && "Current element not present");
    return hv_;
  }

  PinnedHermesValue &value() {
    assert(!isFree_ && "Current element not present");
    return hv_;
  }

  uint32_t getHash() const {
    return gcHash_;
  }
};
} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SERIALIZATIONMANAGEDVALUE_H
