/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/ArrayStorage.h"

#include "hermes/Support/Algorithms.h"
#include "hermes/Support/ErrorHandling.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/Metadata.h"

namespace hermes {
namespace vm {

template <typename HVType>
const VTable ArrayStorageBase<HVType>::vt(
    ArrayStorageBase<HVType>::getCellKind(),
    0,
    nullptr,
    nullptr,
    nullptr,
    _trimSizeCallback
#ifdef HERMES_MEMORY_INSTRUMENTATION
    ,
    VTable::HeapSnapshotMetadata {
      HeapSnapshot::NodeType::Array, nullptr, nullptr, nullptr, nullptr
    }
#endif
);

void ArrayStorageBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const ArrayStorage *>(cell);
  mb.setVTable(&ArrayStorage::vt);
  mb.addArray("storage", self->data(), &self->size_, sizeof(GCHermesValue));
}

void ArrayStorageSmallBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const ArrayStorageSmall *>(cell);
  mb.setVTable(&ArrayStorageSmall::vt);
  mb.addArray(
      "storage", self->data(), &self->size_, sizeof(GCSmallHermesValue));
}

template <typename HVType>
ExecutionStatus ArrayStorageBase<HVType>::ensureCapacity(
    MutableHandle<ArrayStorageBase<HVType>> &selfHandle,
    Runtime &runtime,
    size_type capacity) {
  assert(capacity <= maxElements() && "capacity overflows 32-bit storage");

  if (capacity <= selfHandle->capacity())
    return ExecutionStatus::RETURNED;

  return reallocateToLarger(
      selfHandle, runtime, capacity, 0, 0, selfHandle->size());
}

template <typename HVType>
ExecutionStatus ArrayStorageBase<HVType>::reallocateToLarger(
    MutableHandle<ArrayStorageBase<HVType>> &selfHandle,
    Runtime &runtime,
    size_type capacity,
    size_type fromFirst,
    size_type toFirst,
    size_type toLast) {
  assert(capacity <= maxElements() && "capacity overflows 32-bit storage");

  assert(
      capacity > selfHandle->capacity() &&
      "reallocateToLarger must be called with a larger capacity");

  auto arrRes = create(runtime, capacity);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto newSelfHandle = runtime.makeHandle<ArrayStorageBase<HVType>>(*arrRes);

  auto *newSelf = newSelfHandle.get();

  // Copy the existing data.
  auto *self = selfHandle.get();
  size_type copySize = std::min(self->size() - fromFirst, toLast - toFirst);

  {
    GCHVType *from = self->data() + fromFirst;
    GCHVType *to = newSelf->data() + toFirst;
    GCHVType::uninitialized_copy(from, from + copySize, to, runtime.getHeap());
  }

  // Initialize the elements before the first copied element.
  GCHVType::uninitialized_fill(
      newSelf->data(),
      newSelf->data() + toFirst,
      HVType::encodeEmptyValue(),
      runtime.getHeap());

  // Initialize the elements after the last copied element and toLast.
  if (toFirst + copySize < toLast) {
    GCHVType::uninitialized_fill(
        newSelf->data() + toFirst + copySize,
        newSelf->data() + toLast,
        HVType::encodeEmptyValue(),
        runtime.getHeap());
  }

  newSelf->size_.store(toLast, std::memory_order_release);

  // Update the handle.
  selfHandle = newSelfHandle.get();

  return ExecutionStatus::RETURNED;
}

template <typename HVType>
void ArrayStorageBase<HVType>::resizeWithinCapacity(
    ArrayStorageBase<HVType> *self,
    GC &gc,
    size_type newSize) {
  assert(
      newSize <= self->capacity() &&
      "newSize must be <= capacity in resizeWithinCapacity()");
  // If enlarging, clear the new elements.
  const auto sz = self->size();
  if (newSize > sz) {
    // Treat the memory as uninitialized when growing.
    // This applies even in the case where the length has been decreased and
    // increased again. When the length is decreased, it executes a write
    // barrier to mark all of the values between the new length and the old
    // length as unreachable. Since the GC will then be aware of those values,
    // it doesn't matter if we overwrite them here again.
    GCHVType::uninitialized_fill(
        self->data() + sz,
        self->data() + newSize,
        HVType::encodeEmptyValue(),
        gc);
  } else if (newSize < sz) {
    // Execute write barriers on elements about to be conceptually changed to
    // null.
    // This also means if an array is refilled, it can treat the memory here
    // as uninitialized safely.
    GCHVType::rangeUnreachableWriteBarrier(
        self->data() + newSize, self->data() + sz, gc);
  }
  self->size_.store(newSize, std::memory_order_release);
}

template <typename HVType>
ExecutionStatus ArrayStorageBase<HVType>::shift(
    MutableHandle<ArrayStorageBase<HVType>> &selfHandle,
    Runtime &runtime,
    size_type fromFirst,
    size_type toFirst,
    size_type toLast) {
  assert(toLast <= maxElements() && "size overflows 32-bit storage");
  assert(toFirst <= toLast && "First must be before last");
  assert(fromFirst <= selfHandle->size() && "fromFirst must be before size");

  // If we don't need to expand the capacity.
  if (toLast <= selfHandle->capacity()) {
    auto *self = selfHandle.get();
    size_type copySize = std::min(self->size() - fromFirst, toLast - toFirst);

    // Copy the values to their final destination.
    if (fromFirst > toFirst) {
      GCHVType::copy(
          self->data() + fromFirst,
          self->data() + fromFirst + copySize,
          self->data() + toFirst,
          runtime.getHeap());
    } else if (fromFirst < toFirst) {
      // Copying to the right, need to copy backwards to avoid overwriting what
      // is being copied.
      GCHVType::copy_backward(
          self->data() + fromFirst,
          self->data() + fromFirst + copySize,
          self->data() + toFirst + copySize,
          runtime.getHeap());
    }

    // Initialize the elements which were emptied in front.
    GCHVType::fill(
        self->data(),
        self->data() + toFirst,
        HVType::encodeEmptyValue(),
        runtime.getHeap());

    // Initialize the elements between the last copied element and toLast.
    if (toFirst + copySize < toLast) {
      GCHVType::uninitialized_fill(
          self->data() + toFirst + copySize,
          self->data() + toLast,
          HVType::encodeEmptyValue(),
          runtime.getHeap());
    }
    if (toLast < self->size()) {
      // Some elements are becoming unreachable, let the GC know.
      GCHVType::rangeUnreachableWriteBarrier(
          self->data() + toLast,
          self->data() + self->size(),
          runtime.getHeap());
    }
    self->size_.store(toLast, std::memory_order_release);
    return ExecutionStatus::RETURNED;
  }

  // Calculate the new capacity.
  size_type capacity = selfHandle->capacity();
  if (capacity < maxElements() / 2)
    capacity = std::max(capacity * 2, toLast);
  else
    capacity = maxElements();

  return reallocateToLarger(
      selfHandle, runtime, capacity, fromFirst, toFirst, toLast);
}

template <typename HVType>
ExecutionStatus ArrayStorageBase<HVType>::throwExcessiveCapacityError(
    Runtime &runtime,
    size_type capacity) {
  assert(
      capacity > maxElements() &&
      "Shouldn't call this without first checking that capacity is big");
  // Record the fact that this error occurred.
  HERMES_EXTRA_DEBUG(runtime.getCrashManager().setCustomData(
      "Hermes_ArrayStorage_overflow", "1"));
  return runtime.raiseRangeError(
      TwineChar16(
          "Requested an array size larger than the max allowable: Requested elements = ") +
      capacity + ", max elements = " + maxElements());
}

template <typename HVType>
ExecutionStatus ArrayStorageBase<HVType>::pushBackSlowPath(
    MutableHandle<ArrayStorageBase<HVType>> &selfHandle,
    Runtime &runtime,
    Handle<> value) {
  const auto size = selfHandle->size();
  if (resize(selfHandle, runtime, size + 1) == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto hv = HVType::encodeHermesValue(*value, runtime);
  selfHandle->set(size, hv, runtime.getHeap());
  return ExecutionStatus::RETURNED;
}

template <typename HVType>
gcheapsize_t ArrayStorageBase<HVType>::_trimSizeCallback(const GCCell *cell) {
  const auto *self = reinterpret_cast<const ArrayStorageBase<HVType> *>(cell);
  return allocationSize(self->size());
}

template class ArrayStorageBase<HermesValue>;
template class ArrayStorageBase<SmallHermesValue>;

} // namespace vm
} // namespace hermes
