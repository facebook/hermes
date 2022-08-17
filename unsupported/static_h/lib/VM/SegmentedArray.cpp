/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/SegmentedArray.h"

#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HermesValue-inline.h"

namespace hermes {
namespace vm {

template <typename HVType>
const VTable SegmentedArrayBase<HVType>::Segment::vt(
    getCellKind(),
    cellSize<SegmentedArrayBase::Segment>(),
    nullptr,
    nullptr,
    nullptr,
    nullptr
#ifdef HERMES_MEMORY_INSTRUMENTATION
    ,
    VTable::HeapSnapshotMetadata {
      HeapSnapshot::NodeType::Array, nullptr, nullptr, nullptr, nullptr
    }
#endif
);

void SegmentBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const SegmentedArray::Segment *>(cell);
  mb.setVTable(&SegmentedArray::Segment::vt);
  mb.addArray("data", self->data_, &self->length_, sizeof(GCHermesValue));
}
void SegmentSmallBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const SegmentedArraySmall::Segment *>(cell);
  mb.setVTable(&SegmentedArraySmall::Segment::vt);
  mb.addArray("data", self->data_, &self->length_, sizeof(GCSmallHermesValue));
}

template <typename HVType>
PseudoHandle<typename SegmentedArrayBase<HVType>::Segment>
SegmentedArrayBase<HVType>::Segment::create(Runtime &runtime) {
  // NOTE: This needs to live in the cpp file instead of the header because it
  // uses PseudoHandle, which requires a specialization of IsGCObject for the
  // type it constructs.
  return createPseudoHandle(runtime.makeAFixed<Segment>());
}

template <typename HVType>
void SegmentedArrayBase<HVType>::Segment::setLength(
    Runtime &runtime,
    uint32_t newLength) {
  const auto len = length();
  if (newLength > len) {
    // Length is increasing, fill with emptys.
    GCHVType::uninitialized_fill(
        data_ + len,
        data_ + newLength,
        HVType::encodeEmptyValue(),
        runtime.getHeap());
    length_.store(newLength, std::memory_order_release);
  } else if (newLength < len) {
    // If length is decreasing a write barrier needs to be done.
    GCHVType::rangeUnreachableWriteBarrier(
        data_ + newLength, data_ + len, runtime.getHeap());
    length_.store(newLength, std::memory_order_release);
  }
}

template <typename HVType>
const VTable SegmentedArrayBase<HVType>::vt(
    getCellKind(),
    /*variableSize*/ 0,
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

void SegmentedArrayBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const SegmentedArray *>(cell);
  mb.setVTable(&SegmentedArray::vt);
  mb.addArray(
      "slots",
      self->inlineStorage(),
      &self->numSlotsUsed_,
      sizeof(GCHermesValue));
}

void SegmentedArraySmallBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const SegmentedArraySmall *>(cell);
  mb.setVTable(&SegmentedArraySmall::vt);
  mb.addArray(
      "slots",
      self->inlineStorage(),
      &self->numSlotsUsed_,
      sizeof(GCSmallHermesValue));
}

template <typename HVType>
CallResult<PseudoHandle<SegmentedArrayBase<HVType>>>
SegmentedArrayBase<HVType>::create(Runtime &runtime, size_type capacity) {
  if (LLVM_UNLIKELY(capacity > maxElements())) {
    return throwExcessiveCapacityError(runtime, capacity);
  }
  // Leave the segments as null. Whenever the size is changed, the segments
  // will be allocated. Note that this means the capacity argument won't be
  // reflected in capacity() if it is larger than the inline storage space.
  // That is in order to avoid having an extra field to track, and the upper
  // bound of "size" can be used instead.
  const auto allocSize = allocationSizeForCapacity(capacity);
  return createPseudoHandle(
      runtime.makeAVariable<SegmentedArrayBase>(allocSize));
}

template <typename HVType>
CallResult<PseudoHandle<SegmentedArrayBase<HVType>>> SegmentedArrayBase<
    HVType>::createLongLived(Runtime &runtime, size_type capacity) {
  if (LLVM_UNLIKELY(capacity > maxElements())) {
    return throwExcessiveCapacityError(runtime, capacity);
  }
  // Leave the segments as null. Whenever the size is changed, the segments
  // will be allocated.
  const auto allocSize = allocationSizeForCapacity(capacity);
  return createPseudoHandle(
      runtime
          .makeAVariable<SegmentedArrayBase, HasFinalizer::No, LongLived::Yes>(
              allocSize));
}

template <typename HVType>
CallResult<PseudoHandle<SegmentedArrayBase<HVType>>> SegmentedArrayBase<
    HVType>::create(Runtime &runtime, size_type capacity, size_type size) {
  auto arrRes = create(runtime, capacity);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  PseudoHandle<SegmentedArrayBase> self = std::move(*arrRes);
  // TODO T25663446: This is potentially optimizable to iterate over the
  // inline storage and the segments separately.
  self = increaseSize(runtime, std::move(self), size);
  return self;
}

template <typename HVType>
typename SegmentedArrayBase<HVType>::size_type
SegmentedArrayBase<HVType>::capacity() const {
  const auto numSlotsUsed = numSlotsUsed_.load(std::memory_order_relaxed);
  if (numSlotsUsed <= kValueToSegmentThreshold) {
    // In the case where the size is less than the number of inline elements,
    // the capacity is at most slotCapacity, or the segment threshold if slot
    // capacity goes beyond that.
    return std::min(slotCapacity(), size_type{kValueToSegmentThreshold});
  } else {
    // Any slot after numSlotsUsed_ is guaranteed to be null.
    return kValueToSegmentThreshold +
        (numSlotsUsed - kValueToSegmentThreshold) * Segment::kMaxLength;
  }
}

template <typename HVType>
typename SegmentedArrayBase<HVType>::size_type
SegmentedArrayBase<HVType>::totalCapacityOfSpine() const {
  const auto slotCap = slotCapacity();
  if (slotCap <= kValueToSegmentThreshold) {
    return slotCap;
  } else {
    return kValueToSegmentThreshold +
        (slotCap - kValueToSegmentThreshold) * Segment::kMaxLength;
  }
}

template <typename HVType>
ExecutionStatus SegmentedArrayBase<HVType>::push_back(
    MutableHandle<SegmentedArrayBase> &self,
    Runtime &runtime,
    Handle<> value) {
  auto oldSize = self->size(runtime);
  if (growRight(self, runtime, 1) == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  const auto shv = HVType::encodeHermesValue(*value, runtime);
  auto &elm = self->atRef(runtime, oldSize);
  new (&elm) GCHVType(shv, runtime.getHeap());
  return ExecutionStatus::RETURNED;
}

template <typename HVType>
ExecutionStatus SegmentedArrayBase<HVType>::resize(
    MutableHandle<SegmentedArrayBase> &self,
    Runtime &runtime,
    size_type newSize) {
  if (newSize > self->size(runtime)) {
    return growRight(self, runtime, newSize - self->size(runtime));
  } else if (newSize < self->size(runtime)) {
    self->shrinkRight(runtime, self->size(runtime) - newSize);
  }
  return ExecutionStatus::RETURNED;
}

template <typename HVType>
ExecutionStatus SegmentedArrayBase<HVType>::resizeLeft(
    MutableHandle<SegmentedArrayBase> &self,
    Runtime &runtime,
    size_type newSize) {
  if (newSize == self->size(runtime)) {
    return ExecutionStatus::RETURNED;
  } else if (newSize > self->size(runtime)) {
    return growLeft(self, runtime, newSize - self->size(runtime));
  } else {
    self->shrinkLeft(runtime, self->size(runtime) - newSize);
    return ExecutionStatus::RETURNED;
  }
}

template <typename HVType>
void SegmentedArrayBase<HVType>::resizeWithinCapacity(
    SegmentedArrayBase *self,
    Runtime &runtime,
    size_type newSize) {
  const size_type currSize = self->size(runtime);
  assert(
      newSize <= self->capacity() &&
      "Cannot resizeWithinCapacity to a size not within capacity");
  if (newSize > currSize) {
    self->increaseSizeWithinCapacity(runtime, newSize - currSize);
  } else if (newSize < currSize) {
    self->shrinkRight(runtime, currSize - newSize);
  }
}

template <typename HVType>
ExecutionStatus SegmentedArrayBase<HVType>::throwExcessiveCapacityError(
    Runtime &runtime,
    size_type capacity) {
  assert(
      capacity > maxElements() &&
      "Shouldn't call this without first checking that capacity is big");
  return runtime.raiseRangeError(
      TwineChar16(
          "Requested an array size larger than the max allowable: Requested elements = ") +
      capacity + ", max elements = " + maxElements());
}

template <typename HVType>
void SegmentedArrayBase<HVType>::allocateSegment(
    Runtime &runtime,
    Handle<SegmentedArrayBase> self,
    SegmentNumber segment) {
  assert(
      self->segmentAtPossiblyUnallocated(segment)->isEmpty() &&
      "Allocating into a non-empty segment");
  PseudoHandle<Segment> c = Segment::create(runtime);
  self->segmentAtPossiblyUnallocated(segment)->set(
      HVType::encodeObjectValue(c.get(), runtime), runtime.getHeap());
}

template <typename HVType>
ExecutionStatus SegmentedArrayBase<HVType>::growRight(
    MutableHandle<SegmentedArrayBase> &self,
    Runtime &runtime,
    size_type amount) {
  if (self->size(runtime) + amount <= self->totalCapacityOfSpine()) {
    increaseSize(runtime, self, amount);
    return ExecutionStatus::RETURNED;
  }
  const auto newSize = self->size(runtime) + amount;
  // Allocate a new SegmentedArray according to the resize policy.
  auto arrRes =
      create(runtime, calculateNewCapacity(self->size(runtime), newSize));
  if (arrRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  PseudoHandle<SegmentedArrayBase> newSegmentedArray = std::move(*arrRes);
  // Copy inline storage and segments over.
  // Do this with raw pointers so that the range write barrier occurs.
  const auto numSlotsUsed = self->numSlotsUsed_.load(std::memory_order_relaxed);
  GCHVType::uninitialized_copy(
      self->inlineStorage(),
      self->inlineStorage() + numSlotsUsed,
      newSegmentedArray->inlineStorage(),
      runtime.getHeap());
  // Set the size of the new array to be the same as the old array's size.
  newSegmentedArray->numSlotsUsed_.store(
      numSlotsUsed, std::memory_order_release);
  newSegmentedArray =
      increaseSize(runtime, std::move(newSegmentedArray), amount);
  // Assign back to self.
  self = newSegmentedArray.get();
  return ExecutionStatus::RETURNED;
}

template <typename HVType>
ExecutionStatus SegmentedArrayBase<HVType>::growLeft(
    MutableHandle<SegmentedArrayBase> &self,
    Runtime &runtime,
    size_type amount) {
  if (self->size(runtime) + amount <= self->totalCapacityOfSpine()) {
    growLeftWithinCapacity(runtime, self, amount);
    return ExecutionStatus::RETURNED;
  }
  const auto newSize = self->size(runtime) + amount;
  auto arrRes = create(
      runtime, calculateNewCapacity(self->size(runtime), newSize), newSize);
  if (arrRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  PseudoHandle<SegmentedArrayBase> newSegmentedArray = std::move(*arrRes);
  // Copy element-by-element, since a shift would need to happen anyway.
  // Since self and newSegmentedArray are distinct, don't need to worry about
  // order.
  GCHVType::copy(
      self->begin(runtime),
      self->end(runtime),
      newSegmentedArray->begin(runtime) + amount,
      runtime.getHeap());
  // Assign back to self.
  self = newSegmentedArray.get();
  return ExecutionStatus::RETURNED;
}

template <typename HVType>
void SegmentedArrayBase<HVType>::growLeftWithinCapacity(
    Runtime &runtime,
    PseudoHandle<SegmentedArrayBase> self,
    size_type amount) {
  assert(
      self->size(runtime) + amount <= self->totalCapacityOfSpine() &&
      "Cannot grow higher than capacity");
  // Fill with empty values at the end to simplify the write barrier.
  self = increaseSize(runtime, std::move(self), amount);
  // Copy the range from the beginning to the end.
  GCHVType::copy_backward(
      self->begin(runtime),
      self->end(runtime) - amount,
      self->end(runtime),
      runtime.getHeap());
  // Fill the beginning with empty values.
  GCHVType::fill(
      self->begin(runtime),
      self->begin(runtime) + amount,
      HVType::encodeEmptyValue(),
      runtime.getHeap());
}

template <typename HVType>
void SegmentedArrayBase<HVType>::shrinkRight(
    Runtime &runtime,
    size_type amount) {
  decreaseSize(runtime, amount);
}

template <typename HVType>
void SegmentedArrayBase<HVType>::shrinkLeft(
    Runtime &runtime,
    size_type amount) {
  // Copy the end values leftwards to the beginning.
  GCHVType::copy(
      begin(runtime) + amount, end(runtime), begin(runtime), runtime.getHeap());
  // Now that all the values are moved down, fill the end with empty values.
  decreaseSize(runtime, amount);
}

template <typename HVType>
void SegmentedArrayBase<HVType>::increaseSizeWithinCapacity(
    Runtime &runtime,
    size_type amount) {
  // This function has the same logic as increaseSize, but removes some
  // complexity from avoiding dealing with alllocations.
  const auto currSize = size(runtime);
  const auto finalSize = currSize + amount;
  assert(
      finalSize <= capacity() &&
      "Cannot use increaseSizeWithinCapacity without checking for capacity first");

  if (finalSize <= kValueToSegmentThreshold) {
    // currSize and finalSize are inside inline storage, bump and fill.
    GCHVType::uninitialized_fill(
        inlineStorage() + currSize,
        inlineStorage() + finalSize,
        HVType::encodeEmptyValue(),
        runtime.getHeap());
    // Set the final size.
    numSlotsUsed_.store(finalSize, std::memory_order_release);
    return;
  }
  // Since this change is within capacity, it is at most filling up a single
  // segment.
  const SegmentNumber segment = toSegment(finalSize - 1);
  const auto segmentLength = toInterior(finalSize - 1) + 1;
  // Fill the inline slots if necessary, and the single segment.
  if (currSize < kValueToSegmentThreshold) {
    GCHVType::uninitialized_fill(
        inlineStorage() + currSize,
        inlineStorage() + kValueToSegmentThreshold,
        HVType::encodeEmptyValue(),
        runtime.getHeap());
  }
  segmentAt(runtime, segment)->setLength(runtime, segmentLength);
}

template <typename HVType>
PseudoHandle<SegmentedArrayBase<HVType>>
SegmentedArrayBase<HVType>::increaseSize(
    Runtime &runtime,
    PseudoHandle<SegmentedArrayBase> self,
    size_type amount) {
  const auto currSize = self->size(runtime);
  const auto finalSize = currSize + amount;

  if (finalSize <= self->capacity()) {
    self->increaseSizeWithinCapacity(runtime, amount);
    return self;
  }

  // Inline slots must be reserved by the caller. Since finalSize is greater
  // than the capacity, we know that it must require adding segments.
  assert(finalSize > kValueToSegmentThreshold);

  // currSize might be in inline storage, but finalSize is definitely in
  // segments.
  // Allocate missing segments after filling inline storage.
  if (currSize <= kValueToSegmentThreshold) {
    // Segments will need to be allocated, if the old size didn't have the
    // inline storage filled up, fill it up now.
    GCHVType::uninitialized_fill(
        self->inlineStorage() + currSize,
        self->inlineStorage() + kValueToSegmentThreshold,
        HVType::encodeEmptyValue(),
        runtime.getHeap());
    // Set the size to the inline storage threshold.
    self->numSlotsUsed_.store(
        kValueToSegmentThreshold, std::memory_order_release);
  }

  // NOTE: during this function, allocations can happen.
  // If one of these allocations triggers a full compacting GC, then the array
  // currently being increased might have its capacity shrunk to match its
  // numSlotsUsed. So, increase numSlotsUsed immediately to its final value
  // before the allocations happen so it isn't shrunk, and also fill with
  // empty values so that any mark passes don't fail. The segments should all
  // have length 0 until allocations are finished, so that uninitialized
  // memory is not scanned inside the segments. Once allocations are finished,
  // go back and fixup the lengths.
  const SegmentNumber startSegment =
      currSize <= kValueToSegmentThreshold ? 0 : toSegment(currSize - 1);
  const SegmentNumber lastSegment = toSegment(finalSize - 1);
  const auto newNumSlotsUsed = numSlotsForCapacity(finalSize);
  // Put empty values into all of the added slots so that the memory is not
  // uninitialized during marking.
  GCHVType::uninitialized_fill(
      self->inlineStorage() +
          self->numSlotsUsed_.load(std::memory_order_relaxed),
      self->inlineStorage() + newNumSlotsUsed,
      HVType::encodeEmptyValue(),
      runtime.getHeap());
  self->numSlotsUsed_.store(newNumSlotsUsed, std::memory_order_release);

  // Allocate a handle to track the current array.
  auto selfHandle = runtime.makeHandle(std::move(self));
  // Allocate each segment.
  if (startSegment <= lastSegment &&
      selfHandle->segmentAtPossiblyUnallocated(startSegment)->isEmpty()) {
    // The start segment might already be allocated if it was half full when
    // we increase the size.
    allocateSegment(runtime, selfHandle, startSegment);
  }
  for (auto i = startSegment + 1; i <= lastSegment; ++i) {
    // All segments except the start need to become allocated.
    allocateSegment(runtime, selfHandle, i);
  }

  // Now that all allocations have occurred, set the lengths inside each
  // segment, and optionally fill.
  for (auto i = startSegment; i <= lastSegment; ++i) {
    // If its the last chunk, set to the length required by any leftover
    // elements.
    const auto segmentLength =
        i == lastSegment ? toInterior(finalSize - 1) + 1 : Segment::kMaxLength;
    selfHandle->segmentAt(runtime, i)->setLength(runtime, segmentLength);
  }
  self = selfHandle;
  return self;
}

template <typename HVType>
void SegmentedArrayBase<HVType>::decreaseSize(
    Runtime &runtime,
    size_type amount) {
  const auto initialSize = size(runtime);
  const auto initialNumSlots = numSlotsUsed_.load(std::memory_order_relaxed);
  assert(amount <= initialSize && "Cannot decrease size past zero");
  const auto finalSize = initialSize - amount;
  const auto finalNumSlots = numSlotsForCapacity(finalSize);
  assert(
      finalNumSlots <= initialNumSlots &&
      "Should not be increasing the number of slots");
  if (finalSize > kValueToSegmentThreshold) {
    // Set the new last used segment's length to be the leftover.
    segmentAt(runtime, toSegment(finalSize - 1))
        ->setLength(runtime, toInterior(finalSize - 1) + 1);
  }
  // Before shrinking, do a snapshot write barrier for the elements being
  // removed.
  GCHVType::rangeUnreachableWriteBarrier(
      inlineStorage() + finalNumSlots,
      inlineStorage() + initialNumSlots,
      runtime.getHeap());
  numSlotsUsed_.store(finalNumSlots, std::memory_order_release);
}

template <typename HVType>
gcheapsize_t SegmentedArrayBase<HVType>::_trimSizeCallback(const GCCell *cell) {
  const auto *self = reinterpret_cast<const SegmentedArrayBase *>(cell);
  // This array will shrink so that it has the same slot capacity as the slot
  // size.
  return allocationSizeForSlots(
      self->numSlotsUsed_.load(std::memory_order_relaxed));
}

template class SegmentedArrayBase<HermesValue>;
template class SegmentedArrayBase<SmallHermesValue>;

} // namespace vm
} // namespace hermes
