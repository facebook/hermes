/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/SegmentedArray.h"

#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HermesValue-inline.h"

#include "llvh/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

const VTable SegmentedArray::Segment::vt(
    CellKind::SegmentKind,
    cellSize<SegmentedArray::Segment>(),
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr, // externalMemorySize
    VTable::HeapSnapshotMetadata{
        HeapSnapshot::NodeType::Array,
        nullptr,
        nullptr,
        nullptr,
        nullptr});

void SegmentBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const SegmentedArray::Segment *>(cell);
  mb.setVTable(&SegmentedArray::Segment::vt);
  mb.addArray("data", self->data_, &self->length_, sizeof(GCHermesValue));
}

#ifdef HERMESVM_SERIALIZE
SegmentedArray::Segment::Segment(Deserializer &d)
    : GCCell(&d.getRuntime()->getHeap(), &vt) {
  length_.store(d.readInt<uint32_t>(), std::memory_order_release);
  for (uint32_t i = 0; i < length(); i++) {
    d.readHermesValue(&data_[i]);
  }
}

void SegmentSerialize(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const SegmentedArray::Segment>(cell);
  s.writeInt<uint32_t>(self->length());
  for (uint32_t i = 0; i < self->length(); i++) {
    s.writeHermesValue(self->data_[i]);
  }
  s.endObject(cell);
}

void SegmentDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::SegmentKind && "Expected Segment");
  auto *cell = d.getRuntime()->makeAFixed<SegmentedArray::Segment>(d);
  d.endObject(cell);
}
#endif

PseudoHandle<SegmentedArray::Segment> SegmentedArray::Segment::create(
    Runtime *runtime) {
  // NOTE: This needs to live in the cpp file instead of the header because it
  // uses PseudoHandle, which requires a specialization of IsGCObject for the
  // type it constructs.
  return createPseudoHandle(runtime->makeAFixed<Segment>(runtime));
}

void SegmentedArray::Segment::setLength(Runtime *runtime, uint32_t newLength) {
  const auto len = length();
  if (newLength > len) {
    // Length is increasing, fill with emptys.
    GCHermesValue::uninitialized_fill(
        data_ + len,
        data_ + newLength,
        HermesValue::encodeEmptyValue(),
        &runtime->getHeap());
    length_.store(newLength, std::memory_order_release);
  } else if (newLength < len) {
    // If length is decreasing a write barrier needs to be done.
    GCHermesValue::rangeUnreachableWriteBarrier(
        data_ + newLength, data_ + len, &runtime->getHeap());
    length_.store(newLength, std::memory_order_release);
  }
}

const VTable SegmentedArray::vt(
    CellKind::SegmentedArrayKind,
    /*variableSize*/ 0,
    nullptr,
    nullptr,
    nullptr,
    _trimSizeCallback,
    nullptr, // externalMemorySize
    VTable::HeapSnapshotMetadata{
        HeapSnapshot::NodeType::Array,
        nullptr,
        nullptr,
        nullptr,
        nullptr});

void SegmentedArrayBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const SegmentedArray *>(cell);
  mb.setVTable(&SegmentedArray::vt);
  mb.addArray(
      "slots",
      self->inlineStorage(),
      &self->numSlotsUsed_,
      sizeof(GCHermesValue));
}

#ifdef HERMESVM_SERIALIZE
void SegmentedArraySerialize(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const SegmentedArray>(cell);
  s.writeInt<SegmentedArray::size_type>(self->slotCapacity());
  s.writeInt<SegmentedArray::size_type>(
      self->numSlotsUsed_.load(std::memory_order_relaxed));

  for (uint32_t i = 0; i < self->numSlotsUsed_.load(std::memory_order_relaxed);
       i++) {
    s.writeHermesValue(self->at(i));
  }

  s.endObject(cell);
}

void SegmentedArrayDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::SegmentedArrayKind && "Expected SegmentedArray");
  SegmentedArray::size_type slotCapacity =
      d.readInt<SegmentedArray::size_type>();
  SegmentedArray::size_type numSlotsUsed =
      d.readInt<SegmentedArray::size_type>();
  const auto allocSize = SegmentedArray::allocationSizeForSlots(slotCapacity);
  SegmentedArray *cell = d.getRuntime()->makeAVariable<SegmentedArray>(
      allocSize, d.getRuntime(), allocSize, numSlotsUsed);
  for (auto it = cell->begin(); it != cell->end(); ++it) {
    d.readHermesValue(&*it);
  }
  d.endObject(cell);
}
#endif

CallResult<PseudoHandle<SegmentedArray>> SegmentedArray::create(
    Runtime *runtime,
    size_type capacity) {
  if (LLVM_UNLIKELY(capacity > maxElements())) {
    return throwExcessiveCapacityError(runtime, capacity);
  }
  // Leave the segments as null. Whenever the size is changed, the segments will
  // be allocated.
  // Note that this means the capacity argument won't be reflected in capacity()
  // if it is larger than the inline storage space. That is in order to avoid
  // having an extra field to track, and the upper bound of "size" can be used
  // instead.
  const auto allocSize = allocationSizeForCapacity(capacity);
  return createPseudoHandle(
      runtime->makeAVariable<SegmentedArray>(allocSize, runtime, allocSize));
}

CallResult<PseudoHandle<SegmentedArray>> SegmentedArray::createLongLived(
    Runtime *runtime,
    size_type capacity) {
  if (LLVM_UNLIKELY(capacity > maxElements())) {
    return throwExcessiveCapacityError(runtime, capacity);
  }
  // Leave the segments as null. Whenever the size is changed, the segments will
  // be allocated.
  const auto allocSize = allocationSizeForCapacity(capacity);
  return createPseudoHandle(
      runtime->makeAVariable<SegmentedArray, HasFinalizer::No, LongLived::Yes>(
          allocSize, runtime, allocSize));
}

CallResult<PseudoHandle<SegmentedArray>>
SegmentedArray::create(Runtime *runtime, size_type capacity, size_type size) {
  auto arrRes = create(runtime, capacity);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  PseudoHandle<SegmentedArray> self = std::move(*arrRes);
  // TODO T25663446: This is potentially optimizable to iterate over the inline
  // storage and the segments separately.
  self = increaseSize(runtime, std::move(self), size);
  return self;
}

SegmentedArray::size_type SegmentedArray::capacity() const {
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

SegmentedArray::size_type SegmentedArray::totalCapacityOfSpine() const {
  const auto slotCap = slotCapacity();
  if (slotCap <= kValueToSegmentThreshold) {
    return slotCap;
  } else {
    return kValueToSegmentThreshold +
        (slotCap - kValueToSegmentThreshold) * Segment::kMaxLength;
  }
}

ExecutionStatus SegmentedArray::push_back(
    MutableHandle<SegmentedArray> &self,
    Runtime *runtime,
    Handle<> value) {
  auto oldSize = self->size();
  if (growRight(self, runtime, 1) == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto &elm = self->atRef(oldSize);
  new (&elm) GCHermesValue(*value, &runtime->getHeap());
  return ExecutionStatus::RETURNED;
}

ExecutionStatus SegmentedArray::resize(
    MutableHandle<SegmentedArray> &self,
    Runtime *runtime,
    size_type newSize) {
  if (newSize > self->size()) {
    return growRight(self, runtime, newSize - self->size());
  } else if (newSize < self->size()) {
    self->shrinkRight(runtime, self->size() - newSize);
  }
  return ExecutionStatus::RETURNED;
}

ExecutionStatus SegmentedArray::resizeLeft(
    MutableHandle<SegmentedArray> &self,
    Runtime *runtime,
    size_type newSize) {
  if (newSize == self->size()) {
    return ExecutionStatus::RETURNED;
  } else if (newSize > self->size()) {
    return growLeft(self, runtime, newSize - self->size());
  } else {
    self->shrinkLeft(runtime, self->size() - newSize);
    return ExecutionStatus::RETURNED;
  }
}

void SegmentedArray::resizeWithinCapacity(
    SegmentedArray *self,
    Runtime *runtime,
    size_type newSize) {
  const size_type currSize = self->size();
  assert(
      newSize <= self->capacity() &&
      "Cannot resizeWithinCapacity to a size not within capacity");
  if (newSize > currSize) {
    self->increaseSizeWithinCapacity(runtime, newSize - currSize);
  } else if (newSize < currSize) {
    self->shrinkRight(runtime, currSize - newSize);
  }
}

ExecutionStatus SegmentedArray::throwExcessiveCapacityError(
    Runtime *runtime,
    size_type capacity) {
  assert(
      capacity > maxElements() &&
      "Shouldn't call this without first checking that capacity is big");
  return runtime->raiseRangeError(
      TwineChar16(
          "Requested an array size larger than the max allowable: Requested elements = ") +
      capacity + ", max elements = " + maxElements());
}

void SegmentedArray::allocateSegment(
    Runtime *runtime,
    Handle<SegmentedArray> self,
    SegmentNumber segment) {
  assert(
      self->segmentAtPossiblyUnallocated(segment)->isEmpty() &&
      "Allocating into a non-empty segment");
  PseudoHandle<Segment> c = Segment::create(runtime);
  self->segmentAtPossiblyUnallocated(segment)->set(
      c.getHermesValue(), &runtime->getHeap());
}

ExecutionStatus SegmentedArray::growRight(
    MutableHandle<SegmentedArray> &self,
    Runtime *runtime,
    size_type amount) {
  if (self->size() + amount <= self->totalCapacityOfSpine()) {
    increaseSize(runtime, self, amount);
    return ExecutionStatus::RETURNED;
  }
  const auto newSize = self->size() + amount;
  // Allocate a new SegmentedArray according to the resize policy.
  auto arrRes = create(runtime, calculateNewCapacity(self->size(), newSize));
  if (arrRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  PseudoHandle<SegmentedArray> newSegmentedArray = std::move(*arrRes);
  // Copy inline storage and segments over.
  // Do this with raw pointers so that the range write barrier occurs.
  const auto numSlotsUsed = self->numSlotsUsed_.load(std::memory_order_relaxed);
  GCHermesValue::uninitialized_copy(
      self->inlineStorage(),
      self->inlineStorage() + numSlotsUsed,
      newSegmentedArray->inlineStorage(),
      &runtime->getHeap());
  // Set the size of the new array to be the same as the old array's size.
  newSegmentedArray->numSlotsUsed_.store(
      numSlotsUsed, std::memory_order_release);
  newSegmentedArray =
      increaseSize(runtime, std::move(newSegmentedArray), amount);
  // Assign back to self.
  self = newSegmentedArray.get();
  return ExecutionStatus::RETURNED;
}

ExecutionStatus SegmentedArray::growLeft(
    MutableHandle<SegmentedArray> &self,
    Runtime *runtime,
    size_type amount) {
  if (self->size() + amount <= self->totalCapacityOfSpine()) {
    growLeftWithinCapacity(runtime, self, amount);
    return ExecutionStatus::RETURNED;
  }
  const auto newSize = self->size() + amount;
  auto arrRes =
      create(runtime, calculateNewCapacity(self->size(), newSize), newSize);
  if (arrRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  PseudoHandle<SegmentedArray> newSegmentedArray = std::move(*arrRes);
  // Copy element-by-element, since a shift would need to happen anyway.
  // Since self and newSegmentedArray are distinct, don't need to worry about
  // order.
  GCHermesValue::copy(
      self->begin(),
      self->end(),
      newSegmentedArray->begin() + amount,
      &runtime->getHeap());
  // Assign back to self.
  self = newSegmentedArray.get();
  return ExecutionStatus::RETURNED;
}

void SegmentedArray::growLeftWithinCapacity(
    Runtime *runtime,
    PseudoHandle<SegmentedArray> self,
    size_type amount) {
  assert(
      self->size() + amount <= self->totalCapacityOfSpine() &&
      "Cannot grow higher than capacity");
  // Fill with empty values at the end to simplify the write barrier.
  self = increaseSize(runtime, std::move(self), amount);
  // Copy the range from the beginning to the end.
  GCHermesValue::copy_backward(
      self->begin(), self->end() - amount, self->end(), &runtime->getHeap());
  // Fill the beginning with empty values.
  GCHermesValue::fill(
      self->begin(),
      self->begin() + amount,
      HermesValue::encodeEmptyValue(),
      &runtime->getHeap());
}

void SegmentedArray::shrinkRight(Runtime *runtime, size_type amount) {
  decreaseSize(runtime, amount);
}

void SegmentedArray::shrinkLeft(Runtime *runtime, size_type amount) {
  // Copy the end values leftwards to the beginning.
  GCHermesValue::copy(begin() + amount, end(), begin(), &runtime->getHeap());
  // Now that all the values are moved down, fill the end with empty values.
  decreaseSize(runtime, amount);
}

void SegmentedArray::increaseSizeWithinCapacity(
    Runtime *runtime,
    size_type amount) {
  // This function has the same logic as increaseSize, but removes some
  // complexity from avoiding dealing with alllocations.
  const auto empty = HermesValue::encodeEmptyValue();
  const auto currSize = size();
  const auto finalSize = currSize + amount;
  assert(
      finalSize <= capacity() &&
      "Cannot use increaseSizeWithinCapacity without checking for capacity first");

  if (finalSize <= kValueToSegmentThreshold) {
    // currSize and finalSize are inside inline storage, bump and fill.
    GCHermesValue::uninitialized_fill(
        inlineStorage() + currSize,
        inlineStorage() + finalSize,
        empty,
        &runtime->getHeap());
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
    GCHermesValue::uninitialized_fill(
        inlineStorage() + currSize,
        inlineStorage() + kValueToSegmentThreshold,
        empty,
        &runtime->getHeap());
  }
  segmentAt(segment)->setLength(runtime, segmentLength);
}

PseudoHandle<SegmentedArray> SegmentedArray::increaseSize(
    Runtime *runtime,
    PseudoHandle<SegmentedArray> self,
    size_type amount) {
  const auto empty = HermesValue::encodeEmptyValue();
  const auto currSize = self->size();
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
    GCHermesValue::uninitialized_fill(
        self->inlineStorage() + currSize,
        self->inlineStorage() + kValueToSegmentThreshold,
        empty,
        &runtime->getHeap());
    // Set the size to the inline storage threshold.
    self->numSlotsUsed_.store(
        kValueToSegmentThreshold, std::memory_order_release);
  }

  // NOTE: during this function, allocations can happen.
  // If one of these allocations triggers a full compacting GC, then the array
  // currently being increased might have its capacity shrunk to match its
  // numSlotsUsed. So, increase numSlotsUsed immediately to its final value
  // before the allocations happen so it isn't shrunk, and also fill with empty
  // values so that any mark passes don't fail.
  // The segments should all have length 0 until allocations are finished, so
  // that uninitialized memory is not scanned inside the segments. Once
  // allocations are finished, go back and fixup the lengths.
  const SegmentNumber startSegment =
      currSize <= kValueToSegmentThreshold ? 0 : toSegment(currSize - 1);
  const SegmentNumber lastSegment = toSegment(finalSize - 1);
  const auto newNumSlotsUsed = numSlotsForCapacity(finalSize);
  // Put empty values into all of the added slots so that the memory is not
  // uninitialized during marking.
  GCHermesValue::uninitialized_fill(
      self->inlineStorage() +
          self->numSlotsUsed_.load(std::memory_order_relaxed),
      self->inlineStorage() + newNumSlotsUsed,
      empty,
      &runtime->getHeap());
  self->numSlotsUsed_.store(newNumSlotsUsed, std::memory_order_release);

  // Allocate a handle to track the current array.
  auto selfHandle = runtime->makeHandle(std::move(self));
  // Allocate each segment.
  if (startSegment <= lastSegment &&
      selfHandle->segmentAtPossiblyUnallocated(startSegment)->isEmpty()) {
    // The start segment might already be allocated if it was half full when we
    // increase the size.
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
    selfHandle->segmentAt(i)->setLength(runtime, segmentLength);
  }
  self = selfHandle;
  return self;
}

void SegmentedArray::decreaseSize(Runtime *runtime, size_type amount) {
  const auto initialSize = size();
  const auto initialNumSlots = numSlotsUsed_.load(std::memory_order_relaxed);
  assert(amount <= initialSize && "Cannot decrease size past zero");
  const auto finalSize = initialSize - amount;
  const auto finalNumSlots = numSlotsForCapacity(finalSize);
  assert(
      finalNumSlots <= initialNumSlots &&
      "Should not be increasing the number of slots");
  if (finalSize > kValueToSegmentThreshold) {
    // Set the new last used segment's length to be the leftover.
    segmentAt(toSegment(finalSize - 1))
        ->setLength(runtime, toInterior(finalSize - 1) + 1);
  }
  // Before shrinking, do a snapshot write barrier for the elements being
  // removed.
  GCHermesValue::rangeUnreachableWriteBarrier(
      inlineStorage() + finalNumSlots,
      inlineStorage() + initialNumSlots,
      &runtime->getHeap());
  numSlotsUsed_.store(finalNumSlots, std::memory_order_release);
}

gcheapsize_t SegmentedArray::_trimSizeCallback(const GCCell *cell) {
  const auto *self = reinterpret_cast<const SegmentedArray *>(cell);
  // This array will shrink so that it has the same slot capacity as the slot
  // size.
  return allocationSizeForSlots(
      self->numSlotsUsed_.load(std::memory_order_relaxed));
}

} // namespace vm
} // namespace hermes

#undef DEBUG_TYPE
