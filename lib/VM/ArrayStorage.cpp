/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/ArrayStorage.h"

#include "hermes/Support/Algorithms.h"
#include "hermes/Support/ErrorHandling.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/Metadata.h"

namespace hermes {
namespace vm {

VTable ArrayStorage::vt(
    CellKind::ArrayStorageKind,
    0,
    nullptr,
    nullptr,
    nullptr,
    _trimSizeCallback,
    _trimCallback,
    VTable::HeapSnapshotMetadata{V8HeapSnapshot::NodeType::Array,
                                 nullptr,
                                 nullptr});

void ArrayStorageBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const ArrayStorage *>(cell);
  mb.addNonPointerField("@capacity", &self->capacity_);
  mb.addNonPointerField("@size", &self->size_);
  mb.addArray<Metadata::ArrayData::ArrayType::HermesValue>(
      "@storage", self->data(), &self->size_, sizeof(GCHermesValue));
}

ExecutionStatus ArrayStorage::ensureCapacity(
    MutableHandle<ArrayStorage> &selfHandle,
    Runtime *runtime,
    size_type capacity) {
  assert(capacity <= maxElements() && "capacity overflows 32-bit storage");

  if (capacity <= selfHandle->capacity_)
    return ExecutionStatus::RETURNED;

  auto size = selfHandle->size_;
  return reallocateToLarger(selfHandle, runtime, capacity, 0, 0, size);
}

ExecutionStatus ArrayStorage::reallocateToLarger(
    MutableHandle<ArrayStorage> &selfHandle,
    Runtime *runtime,
    size_type capacity,
    size_type fromFirst,
    size_type toFirst,
    size_type toLast) {
  assert(capacity <= maxElements() && "capacity overflows 32-bit storage");

  assert(
      capacity > selfHandle->capacity_ &&
      "reallocateToLarger must be called with a larger capacity");

  auto arrRes = ArrayStorage::create(runtime, capacity);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto newSelfHandle = runtime->makeHandle<ArrayStorage>(*arrRes);

  auto *newSelf = newSelfHandle.get();

  // Copy the existing data.
  auto *self = selfHandle.get();
  size_type copySize = std::min(self->size_ - fromFirst, toLast - toFirst);

  {
    GCHermesValue *from = self->data() + fromFirst;
    GCHermesValue *to = newSelf->data() + toFirst;
    for (size_t n = 0; n < copySize; n++) {
      to[n].set(from[n], &runtime->getHeap());
    }
  }

  // Initialize the elements before the first copied element.
  GCHermesValue::fill(
      newSelf->data(),
      newSelf->data() + toFirst,
      HermesValue::encodeEmptyValue());

  // Initialize the elements after the last copied element and toLast.
  if (toFirst + copySize < toLast) {
    GCHermesValue::fill(
        newSelf->data() + toFirst + copySize,
        newSelf->data() + toLast,
        HermesValue::encodeEmptyValue());
  }

  newSelf->size_ = toLast;

  // Update the handle.
  selfHandle = newSelfHandle.get();

  return ExecutionStatus::RETURNED;
}

ExecutionStatus ArrayStorage::shift(
    MutableHandle<ArrayStorage> &selfHandle,
    Runtime *runtime,
    size_type fromFirst,
    size_type toFirst,
    size_type toLast) {
  assert(toLast <= maxElements() && "size overflows 32-bit storage");

  // If we don't need to expand the capacity.
  if (toLast <= selfHandle->capacity_) {
    auto *self = selfHandle.get();
    size_type copySize = std::min(self->size_ - fromFirst, toLast - toFirst);

    // Copy the values to their final destination.
    GCHermesValue::copy(
        self->data() + fromFirst,
        self->data() + fromFirst + copySize,
        self->data() + toFirst,
        &runtime->getHeap());
    // Initialize the elements which were emptied in front.
    GCHermesValue::fill(
        self->data(), self->data() + toFirst, HermesValue::encodeEmptyValue());

    // Initialize the elements between the last copied element and toLast.
    if (toFirst + copySize < toLast) {
      GCHermesValue::fill(
          self->data() + toFirst + copySize,
          self->data() + toLast,
          HermesValue::encodeEmptyValue());
    }
    self->size_ = toLast;
    return ExecutionStatus::RETURNED;
  }

  // Calculate the new capacity.
  size_type capacity = selfHandle->capacity_;
  if (capacity < maxElements() / 2)
    capacity = std::max(capacity * 2, toLast);
  else
    capacity = maxElements();

  return reallocateToLarger(
      selfHandle, runtime, capacity, fromFirst, toFirst, toLast);
}

ExecutionStatus ArrayStorage::throwExcessiveCapacityError(
    Runtime *runtime,
    size_type capacity) {
  assert(
      capacity > maxElements() &&
      "Shouldn't call this without first checking that capacity is big");
  // Record the fact that this error occurred.
  HERMES_EXTRA_DEBUG(runtime->getCrashManager().setCustomData(
      "Hermes_ArrayStorage_overflow", "1"));
  return runtime->raiseRangeError(
      TwineChar16(
          "Requested an array size larger than the max allowable: Requested elements = ") +
      capacity + ", max elements = " + maxElements());
}

ExecutionStatus ArrayStorage::pushBackSlowPath(
    MutableHandle<ArrayStorage> &selfHandle,
    Runtime *runtime,
    Handle<> value) {
  auto size = selfHandle->size_;
  if (resize(selfHandle, runtime, size + 1) == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  selfHandle->at(size).set(value.get(), &runtime->getHeap());
  return ExecutionStatus::RETURNED;
}

gcheapsize_t ArrayStorage::_trimSizeCallback(const GCCell *cell) {
  const auto *self = reinterpret_cast<const ArrayStorage *>(cell);
  return allocationSize(self->size());
}

void ArrayStorage::_trimCallback(GCCell *cell) {
  auto *self = reinterpret_cast<ArrayStorage *>(cell);
  // Shrink the capacity to the current size.
  self->capacity_ = self->size();
}

} // namespace vm
} // namespace hermes
