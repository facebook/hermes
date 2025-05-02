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
    /* allowLargeAlloc */ true,
    nullptr,
    nullptr,
    _trimSizeCallback
#ifdef HERMES_MEMORY_INSTRUMENTATION
    ,
    VTable::HeapSnapshotMetadata{
        HeapSnapshot::NodeType::Array,
        nullptr,
        nullptr,
        nullptr,
        nullptr}
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

template <>
void ArrayStorageBase<HermesValue>::staticAsserts() {}

template <>
void ArrayStorageBase<SmallHermesValue>::staticAsserts() {
  static_assert(sizeof(ArrayStorageSmall) == sizeof(SHArrayStorageSmall));
  static_assert(
      offsetof(ArrayStorageSmall, size_) ==
      offsetof(SHArrayStorageSmall, size));
  llvm_unreachable("staticAsserts must never be called.");
}

template <typename HVType>
ExecutionStatus ArrayStorageBase<HVType>::ensureCapacity(
    MutableHandle<ArrayStorageBase<HVType>> &selfHandle,
    Runtime &runtime,
    size_type capacity) {
  if (capacity <= selfHandle->capacity())
    return ExecutionStatus::RETURNED;

  return reallocateToLarger(
      selfHandle, runtime, capacity, 0, 0, selfHandle->size());
}

template <typename HVType>
ExecutionStatus ArrayStorageBase<HVType>::reallocateToLarger(
    MutableHandle<ArrayStorageBase<HVType>> &selfHandle,
    Runtime &runtime,
    size_type minCapacity,
    size_type fromFirst,
    size_type toFirst,
    size_type toLast) {
  assert(minCapacity >= toLast && "Last element outside capacity.");
  assert(
      minCapacity > selfHandle->capacity() &&
      "minCapacity must be larger than current capacity.");

  size_type newCapacity = std::max(
      std::min(selfHandle->capacity(), maxCapacityNoOverflow() / 2) * 2,
      minCapacity);

  auto arrRes = create(runtime, newCapacity);
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
    GCHVType::uninitialized_copy(
        from, from + copySize, to, newSelf, runtime.getHeap());
  }

  // Initialize the elements before the first copied element.
  GCHVType::uninitialized_fill(
      newSelf->data(),
      newSelf->data() + toFirst,
      HVType::encodeEmptyValue(),
      newSelf,
      runtime.getHeap());

  // Initialize the elements after the last copied element and toLast.
  if (toFirst + copySize < toLast) {
    GCHVType::uninitialized_fill(
        newSelf->data() + toFirst + copySize,
        newSelf->data() + toLast,
        HVType::encodeEmptyValue(),
        newSelf,
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
        self,
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
ExecutionStatus ArrayStorageBase<HVType>::resizeLeft(
    MutableHandle<ArrayStorageBase<HVType>> &selfHandle,
    Runtime &runtime,
    size_type newSize) {
  if (selfHandle->size() == newSize)
    return ExecutionStatus::RETURNED;

  if (newSize < selfHandle->size()) {
    // Shrink left, copy from [size-newSize, size) to [0, newSize). Mark the
    // range [newSize, size) as unreachable.
    auto *self = selfHandle.get();
    size_type size = self->size();
    GCHVType::copy(
        self->data() + size - newSize,
        self->data() + size,
        self->data(),
        self,
        runtime.getHeap());
    // Execute write barriers on elements about to be conceptually changed to
    // null. This also means if an array is refilled, it can treat the memory
    // here as uninitialized safely.
    GCHVType::rangeUnreachableWriteBarrier(
        self->data() + newSize, self->data() + size, runtime.getHeap());
    self->size_.store(newSize, std::memory_order_release);
    return ExecutionStatus::RETURNED;
  }

  // Check if we could expand within capacity.
  assert(newSize > selfHandle->size());
  if (newSize <= selfHandle->capacity()) {
    // size < newSize <= capacity.

    auto *self = selfHandle.get();
    size_type size = self->size();
    size_type toFirst = newSize - size;

    if (toFirst >= size) {
      // uninitialized_copy [0, size) to [toFirst, newSize).
      GCHVType::uninitialized_copy(
          self->data(),
          self->data() + size,
          self->data() + toFirst,
          self,
          runtime.getHeap());
      // Set [0, toFirst) to Empty:
      // - fill [0, size).
      // - uninitialized_fill [size, toFirst).
      GCHVType::fill(
          self->data(),
          self->data() + size,
          HVType::encodeEmptyValue(),
          self,
          runtime.getHeap());
      // The previously-unused cells in [size, toFirst) are now part of the
      // array representation; fill them with empty.
      GCHVType::uninitialized_fill(
          self->data() + size,
          self->data() + toFirst,
          HVType::encodeEmptyValue(),
          self,
          runtime.getHeap());
    } else {
      // uninitialized_copy [size-toFirst, size) to [size, newSize).
      GCHVType::uninitialized_copy(
          self->data() + size - toFirst,
          self->data() + size,
          self->data() + size,
          self,
          runtime.getHeap());
      // Backward copy [0, size-toFirst) to [toFirst, size). It has to be
      // backward here because it's possible that size-toFirst > toFirst.
      GCHVType::copy_backward(
          self->data(),
          self->data() + size - toFirst,
          self->data() + size,
          self,
          runtime.getHeap());
      // Set [0, toFirst) to Empty.
      GCHVType::fill(
          self->data(),
          self->data() + toFirst,
          HVType::encodeEmptyValue(),
          self,
          runtime.getHeap());
    }
    self->size_.store(newSize, std::memory_order_release);
    return ExecutionStatus::RETURNED;
  }

  // Reallocate to a capacity of at least newSize.
  return reallocateToLarger(
      selfHandle, runtime, newSize, 0, newSize - selfHandle->size(), newSize);
}

template <typename HVType>
ExecutionStatus ArrayStorageBase<HVType>::throwAllocationFailure(
    Runtime &runtime,
    size_type capacity) {
  // Record the fact that this error occurred.
  HERMES_EXTRA_DEBUG(runtime.getCrashManager().setCustomData(
      "Hermes_ArrayStorage_overflow", "1"));
  return runtime.raiseRangeError(
      TwineChar16(
          "Requested an array size that fails to allocate: Requested elements = ") +
      capacity);
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
ExecutionStatus ArrayStorageBase<HVType>::appendSlowPath(
    MutableHandle<ArrayStorageBase> &selfHandle,
    Runtime &runtime,
    Handle<ArrayStorageBase> other) {
  auto newSize = selfHandle->size() + other->size();
  if (ensureCapacity(selfHandle, runtime, newSize) ==
      ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;
  selfHandle->appendWithinCapacity(runtime, *other);
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
