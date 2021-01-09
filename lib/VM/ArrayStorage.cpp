/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

const VTable ArrayStorage::vt(
    CellKind::ArrayStorageKind,
    0,
    nullptr,
    nullptr,
    nullptr,
    _trimSizeCallback,
    _trimCallback,
    nullptr,
    VTable::HeapSnapshotMetadata{
        HeapSnapshot::NodeType::Array,
        nullptr,
        nullptr,
        nullptr,
        nullptr});

void ArrayStorageBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const ArrayStorage *>(cell);
  mb.addArray("storage", self->data(), &self->size_, sizeof(GCHermesValue));
}

#ifdef HERMESVM_SERIALIZE
void ArrayStorageSerialize(Serializer &s, const GCCell *cell) {
  hermes_fatal("ArrayStorage should be serialized with its owner");
}

void ArrayStorageDeserialize(Deserializer &d, CellKind kind) {
  hermes_fatal("ArrayStorage should be deserialized with its owner");
}

void ArrayStorage::serializeArrayStorage(
    Serializer &s,
    const ArrayStorage *cell) {
  s.writeInt<size_type>(cell->capacity_);
  s.writeInt<size_type>(cell->size());

  // Serialize HermesValue in storage. There is no native pointer.
  for (size_type i = 0; i < cell->size(); i++) {
    s.writeHermesValue(cell->data()[i]);
  }

  s.endObject(cell);
}

ArrayStorage *ArrayStorage::deserializeArrayStorage(Deserializer &d) {
  uint32_t capacity = d.readInt<size_type>();
  assert(capacity <= ArrayStorage::maxElements() && "invalid capacity");
  auto *cell = d.getRuntime()->makeAVariable<ArrayStorage>(
      ArrayStorage::allocationSize(capacity), d.getRuntime(), capacity);
  assert(cell->size() <= capacity && "size cannot be greater than capacity");
  cell->size_.store(d.readInt<size_type>(), std::memory_order_release);

  // Deserialize HermesValue in storage. There are no native pointers.
  for (size_type i = 0; i < cell->size(); i++) {
    d.readHermesValue(&cell->data()[i]);
  }

  d.endObject(cell);
  return cell;
}
#endif

ExecutionStatus ArrayStorage::ensureCapacity(
    MutableHandle<ArrayStorage> &selfHandle,
    Runtime *runtime,
    size_type capacity) {
  assert(capacity <= maxElements() && "capacity overflows 32-bit storage");

  if (capacity <= selfHandle->capacity_)
    return ExecutionStatus::RETURNED;

  return reallocateToLarger(
      selfHandle, runtime, capacity, 0, 0, selfHandle->size());
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
  size_type copySize = std::min(self->size() - fromFirst, toLast - toFirst);

  {
    GCHermesValue *from = self->data() + fromFirst;
    GCHermesValue *to = newSelf->data() + toFirst;
    GCHermesValue::uninitialized_copy(
        from, from + copySize, to, &runtime->getHeap());
  }

  // Initialize the elements before the first copied element.
  GCHermesValue::uninitialized_fill(
      newSelf->data(),
      newSelf->data() + toFirst,
      HermesValue::encodeEmptyValue(),
      &runtime->getHeap());

  // Initialize the elements after the last copied element and toLast.
  if (toFirst + copySize < toLast) {
    GCHermesValue::uninitialized_fill(
        newSelf->data() + toFirst + copySize,
        newSelf->data() + toLast,
        HermesValue::encodeEmptyValue(),
        &runtime->getHeap());
  }

  newSelf->size_.store(toLast, std::memory_order_release);

  // Update the handle.
  selfHandle = newSelfHandle.get();

  return ExecutionStatus::RETURNED;
}

void ArrayStorage::resizeWithinCapacity(
    ArrayStorage *self,
    Runtime *runtime,
    size_type newSize) {
  assert(
      newSize <= self->capacity_ &&
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
    GCHermesValue::uninitialized_fill(
        self->data() + sz,
        self->data() + newSize,
        HermesValue::encodeEmptyValue(),
        &runtime->getHeap());
  } else if (newSize < sz) {
    // Execute write barriers on elements about to be conceptually changed to
    // null.
    // This also means if an array is refilled, it can treat the memory here
    // as uninitialized safely.
    GCHermesValue::rangeUnreachableWriteBarrier(
        self->data() + newSize, self->data() + sz, &runtime->getHeap());
  }
  self->size_.store(newSize, std::memory_order_release);
}

ExecutionStatus ArrayStorage::shift(
    MutableHandle<ArrayStorage> &selfHandle,
    Runtime *runtime,
    size_type fromFirst,
    size_type toFirst,
    size_type toLast) {
  assert(toLast <= maxElements() && "size overflows 32-bit storage");
  assert(toFirst <= toLast && "First must be before last");
  assert(fromFirst <= selfHandle->size() && "fromFirst must be before size");

  // If we don't need to expand the capacity.
  if (toLast <= selfHandle->capacity_) {
    auto *self = selfHandle.get();
    size_type copySize = std::min(self->size() - fromFirst, toLast - toFirst);

    // Copy the values to their final destination.
    if (fromFirst > toFirst) {
      GCHermesValue::copy(
          self->data() + fromFirst,
          self->data() + fromFirst + copySize,
          self->data() + toFirst,
          &runtime->getHeap());
    } else if (fromFirst < toFirst) {
      // Copying to the right, need to copy backwards to avoid overwriting what
      // is being copied.
      GCHermesValue::copy_backward(
          self->data() + fromFirst,
          self->data() + fromFirst + copySize,
          self->data() + toFirst + copySize,
          &runtime->getHeap());
    }

    // Initialize the elements which were emptied in front.
    GCHermesValue::fill(
        self->data(),
        self->data() + toFirst,
        HermesValue::encodeEmptyValue(),
        &runtime->getHeap());

    // Initialize the elements between the last copied element and toLast.
    if (toFirst + copySize < toLast) {
      GCHermesValue::uninitialized_fill(
          self->data() + toFirst + copySize,
          self->data() + toLast,
          HermesValue::encodeEmptyValue(),
          &runtime->getHeap());
    }
    if (toLast < self->size()) {
      // Some elements are becoming unreachable, let the GC know.
      GCHermesValue::rangeUnreachableWriteBarrier(
          self->data() + toLast,
          self->data() + self->size(),
          &runtime->getHeap());
    }
    self->size_.store(toLast, std::memory_order_release);
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
  const auto size = selfHandle->size();
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
