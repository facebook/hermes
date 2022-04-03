/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_ARRAYSTORAGE_H
#define HERMES_VM_ARRAYSTORAGE_H

#include "hermes/VM/HermesValue-inline.h"
#include "hermes/VM/Metadata.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/SmallHermesValue-inline.h"

#include "llvh/Support/TrailingObjects.h"

namespace hermes {
namespace vm {

/// A GC-managed resizable vector of values. It is used for storage of property
/// values in objects and also indexed property values in arrays. It supports
/// resizing on both ends which is necessary for the simplest implementation of
/// JavaScript arrays (using a base offset and length).
template <typename HVType>
class ArrayStorageBase final
    : public VariableSizeRuntimeCell,
      private llvh::
          TrailingObjects<ArrayStorageBase<HVType>, GCHermesValueBase<HVType>> {
  using GCHVType = GCHermesValueBase<HVType>;
  friend llvh::TrailingObjects<ArrayStorageBase<HVType>, GCHVType>;
  friend void ArrayStorageBuildMeta(const GCCell *cell, Metadata::Builder &mb);
  friend void ArrayStorageSmallBuildMeta(
      const GCCell *cell,
      Metadata::Builder &mb);

  friend void ArrayStorageSerialize(Serializer &s, const GCCell *cell);
  friend void ArrayStorageDeserialize(Deserializer &d, CellKind kind);
  friend void ArrayStorageSmallSerialize(Serializer &s, const GCCell *cell);
  friend void ArrayStorageSmallDeserialize(Deserializer &d, CellKind kind);

 public:
  using size_type = uint32_t;
  using iterator = GCHVType *;

  static const VTable vt;

  /// Gets the amount of memory used by this object for a given \p capacity.
  static constexpr uint32_t allocationSize(size_type capacity) {
    return ArrayStorageBase::template totalSizeToAlloc<GCHVType>(capacity);
  }

  /// \return The the maximum number of elements that will fit in an
  /// ArrayStorage with allocated size \p allocSize.
  static constexpr size_type capacityForAllocationSize(uint32_t allocSize) {
    return (allocSize - allocationSize(0)) / sizeof(HVType);
  }

  /// \return The maximum number of elements we can fit in a single array in the
  /// current GC.
  static constexpr size_type maxElements() {
    return capacityForAllocationSize(GC::maxAllocationSize());
  }

  static bool classof(const GCCell *cell) {
    return cell->getKind() == getCellKind();
  }

  static constexpr CellKind getCellKind() {
    static_assert(
        std::is_same<HVType, HermesValue>::value ||
            std::is_same<HVType, SmallHermesValue>::value,
        "Illegal HVType.");
    return std::is_same<HVType, HermesValue>::value
        ? CellKind::ArrayStorageKind
        : CellKind::ArrayStorageSmallKind;
  }

  /// Create a new instance with at least the specified \p capacity.
  static CallResult<HermesValue> create(Runtime &runtime, size_type capacity) {
    if (LLVM_UNLIKELY(capacity > maxElements())) {
      return throwExcessiveCapacityError(runtime, capacity);
    }
    const auto allocSize = allocationSize(capacity);
    auto *cell = runtime.makeAVariable<ArrayStorageBase<HVType>>(allocSize);
    return HermesValue::encodeObjectValue(cell);
  }

#ifdef UNIT_TEST
  /// Make it possible to construct an ArrayStorage with just a GC* that is
  /// immediately resized to its capacity. This is used only in tests, where we
  /// have a GC* but not a Runtime*.
  static ArrayStorageBase *createForTest(GC &gc, size_type capacity) {
    assert(capacity <= maxElements());
    const auto allocSize = allocationSize(capacity);
    auto *cell = gc.makeAVariable<ArrayStorageBase>(allocSize);
    ArrayStorageBase::resizeWithinCapacity(cell, gc, capacity);
    return cell;
  }
#endif

  /// Create a new long-lived instance with at least the specified \p capacity.
  static CallResult<HermesValue> createLongLived(
      Runtime &runtime,
      size_type capacity) {
    if (LLVM_UNLIKELY(capacity > maxElements())) {
      return throwExcessiveCapacityError(runtime, capacity);
    }
    const auto allocSize = allocationSize(capacity);
    return HermesValue::encodeObjectValue(runtime.makeAVariable<
                                          ArrayStorageBase<HVType>,
                                          HasFinalizer::No,
                                          LongLived::Yes>(allocSize));
  }

  /// Create a new instance with at least the specified \p capacity and a size
  /// of \p size. Requires that \p size <= \p capacity.
  static CallResult<HermesValue>
  create(Runtime &runtime, size_type capacity, size_type size) {
    auto arrRes = create(runtime, capacity);
    if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    resizeWithinCapacity(
        vmcast<ArrayStorageBase<HVType>>(*arrRes), runtime, size);
    return arrRes;
  }

  /// \return a pointer to the underlying data storage.
  GCHVType *data() {
    return ArrayStorageBase<HVType>::template getTrailingObjects<GCHVType>();
  }
  const GCHVType *data() const {
    return ArrayStorageBase<HVType>::template getTrailingObjects<GCHVType>();
  }

  /// This enum is not needed here but is used for compatibility with
  /// SegmentedArray. It is intended to indicate that we know beforehand that
  /// an element is in the "inline storage". All storage here is "inline".
  enum class Inline { No, Yes };

  /// \return the element at index \p index
  template <Inline inl = Inline::No>
  HVType at(size_type index) const {
    assert(index < size() && "index out of range");
    return data()[index];
  }

  /// \return the element at index \p index
  template <Inline inl = Inline::No>
  void set(size_type index, HVType val, GC &gc) {
    assert(index < size() && "index out of range");
    data()[index].set(val, gc);
  }

  /// \return the element at index \p index
  template <Inline inl = Inline::No>
  void setNonPtr(size_type index, HVType val, GC &gc) {
    assert(index < size() && "index out of range");
    data()[index].setNonPtr(val, gc);
  }

  size_type capacity() const {
    return capacityForAllocationSize(getAllocatedSize());
  }
  size_type size() const {
    return size_.load(std::memory_order_relaxed);
  }

  iterator begin() {
    return data();
  }
  iterator end() {
    return data() + size();
  }

  /// Append the given element to the end (increasing size by 1).
  static ExecutionStatus push_back(
      MutableHandle<ArrayStorageBase<HVType>> &selfHandle,
      Runtime &runtime,
      Handle<> value) {
    auto *self = selfHandle.get();
    const auto currSz = self->size();
    // This must be done before the capacity check, because encodeHermesValue
    // may allocate, which could cause trimming of the ArrayStorage. If the
    // capacity check fails, then this work is wasted, but that's okay because
    // it's a slow path.
    auto hv = HVType::encodeHermesValue(value.get(), runtime);
    // For SmallHermesValue, the above may allocate, so update self.
    if (std::is_same<HVType, SmallHermesValue>::value)
      self = selfHandle.get();
    if (LLVM_LIKELY(currSz < self->capacity())) {
      // Use the constructor of GCHermesValue to use the correct write barrier
      // for uninitialized memory.
      new (&self->data()[currSz]) GCHVType(hv, runtime.getHeap());
      self->size_.store(currSz + 1, std::memory_order_release);
      return ExecutionStatus::RETURNED;
    }
    return pushBackSlowPath(selfHandle, runtime, value);
  }

  /// Pop the last element off the array and return it.
  HVType pop_back(Runtime &runtime) {
    const size_type sz = size();
    assert(sz > 0 && "Can't pop from empty ArrayStorage");
    HVType val = data()[sz - 1];
    // In Hades, a snapshot write barrier must be executed on the value that is
    // conceptually being changed to null. The write doesn't need to occur, but
    // it is the only correct way to use the write barrier.
    data()[sz - 1].unreachableWriteBarrier(runtime.getHeap());
    // The background thread can't mutate size, so we don't need fetch_sub here.
    // Relaxed is fine, because the GC doesn't care about the order of seeing
    // the length and the individual elements, as long as illegal HermesValues
    // aren't written there (which they won't be).
    size_.store(sz - 1, std::memory_order_relaxed);
    return val;
  }

  /// Ensure that the capacity of the array is at least \p capacity,
  /// reallocating if needed.
  static ExecutionStatus ensureCapacity(
      MutableHandle<ArrayStorageBase<HVType>> &selfHandle,
      Runtime &runtime,
      size_type capacity);

  /// Change the size of the storage to \p newSize. This can increase the size
  /// (in which case the new elements will be initialized to empty), or decrease
  /// the size.
  static ExecutionStatus resize(
      MutableHandle<ArrayStorageBase<HVType>> &selfHandle,
      Runtime &runtime,
      size_type newSize) {
    return shift(selfHandle, runtime, 0, 0, newSize);
  }

  /// The same as resize, but add elements to the left instead of the right.
  ///
  /// In the case where the capacity is sufficient to hold the \p newSize,
  /// every existing element is copied rightward, a linear time procedure.
  /// If the capacity is not sufficient, then the performance will be the same
  /// as \c resize.
  static ExecutionStatus resizeLeft(
      MutableHandle<ArrayStorageBase<HVType>> &selfHandle,
      Runtime &runtime,
      size_type newSize) {
    return shift(selfHandle, runtime, 0, newSize - selfHandle->size(), newSize);
  }

  /// Set the size to a value <= the capacity. This is a special
  /// case of resize() but has a simpler interface since we know that it doesn't
  /// need to reallocate.
  static void resizeWithinCapacity(
      ArrayStorageBase<HVType> *self,
      GC &gc,
      size_type newSize);

  static void resizeWithinCapacity(
      ArrayStorageBase<HVType> *self,
      Runtime &runtime,
      size_type newSize) {
    resizeWithinCapacity(self, runtime.getHeap(), newSize);
  }

 private:
  AtomicIfConcurrentGC<size_type> size_{0};

 public:
  ArrayStorageBase() = default;
  ArrayStorageBase(const ArrayStorageBase &) = delete;
  void operator=(const ArrayStorageBase &) = delete;
  ~ArrayStorageBase() = delete;

 private:
  /// Throws a RangeError with a descriptive message describing the attempted
  /// capacity allocated, and the max that is allowed.
  /// \returns ExecutionStatus::EXCEPTION always.
  static ExecutionStatus throwExcessiveCapacityError(
      Runtime &runtime,
      size_type capacity);

  /// Append the given element to the end when the capacity has been exhausted
  /// and a reallocation is needed.
  static ExecutionStatus pushBackSlowPath(
      MutableHandle<ArrayStorageBase<HVType>> &selfHandle,
      Runtime &runtime,
      Handle<> value);

  /// Shrinks \p self during GC compaction, so that it's capacity is equal to
  /// its size.
  /// \return the size the object will have when compaction is complete.
  static gcheapsize_t _trimSizeCallback(const GCCell *self);

  /// Reallocate to a larger storage capacity of the storage and copy the
  /// specified portion of the data to the new storage. The length of the data
  /// to be copied is
  ///   length = min(size - fromFirst, toLast - toFirst).
  /// "length" number of elements are copied from "fromFirst" to "toFirst".
  static ExecutionStatus reallocateToLarger(
      MutableHandle<ArrayStorageBase<HVType>> &selfHandle,
      Runtime &runtime,
      size_type capacity,
      size_type fromFirst,
      size_type toFirst,
      size_type toLast);

// Mangling scheme used by MSVC encode public/private into the name.
// As a result, vanilla "ifdef public" trick leads to link errors.
#if defined(UNIT_TEST) || defined(_MSC_VER)
 public:
#endif
  /// This is a flexible function which can be used to extend the array by
  /// creating or removing elements in front or in the back. New elements are
  /// initialized to empty. Intuitively it shifts a specified number of elements
  /// to a new position and clears the rest. More precisely, it can be described
  /// as follows:
  /// 1. Resize the storage to contain `toLast` elements.
  /// 2. Copy the elements `[fromFirst..min(fromFirst+size, toLast-toFirst))` to
  ///       position 'toFirst'.
  /// 3. Set all elements before `toFirst` and after the last copied element to
  ///   "empty".
  static ExecutionStatus shift(
      MutableHandle<ArrayStorageBase<HVType>> &selfHandle,
      Runtime &runtime,
      size_type fromFirst,
      size_type toFirst,
      size_type toLast);
};

using ArrayStorage = ArrayStorageBase<HermesValue>;
using ArrayStorageSmall = ArrayStorageBase<SmallHermesValue>;

static_assert(
    ArrayStorage::allocationSize(ArrayStorage::maxElements()) <=
        GC::maxAllocationSize(),
    "maxElements() is too big");

static_assert(
    GC::maxAllocationSize() -
            ArrayStorage::allocationSize(ArrayStorage::maxElements()) <
        HeapAlign,
    "maxElements() is too small");

static_assert(
    ArrayStorageSmall::allocationSize(ArrayStorageSmall::maxElements()) <=
        GC::maxAllocationSize(),
    "maxElements() is too big");

static_assert(
    GC::maxAllocationSize() -
            ArrayStorageSmall::allocationSize(
                ArrayStorageSmall::maxElements()) <
        HeapAlign,
    "maxElements() is too small");

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_ARRAYSTORAGE_H
