/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SEGMENTEDARRAY_H
#define HERMES_VM_SEGMENTEDARRAY_H

#include "hermes/VM/CellKind.h"
#include "hermes/VM/Runtime.h"

#include "llvh/Support/TrailingObjects.h"

#include <limits>

namespace hermes {
namespace vm {

/// A SegmentedArray is a two-layer array implementation.
///
/// It has an API that resembles ArrayStorage, and can be used as a generic
/// growable array.
/// The first layer is called the "spine", it contains elements for the first
/// kValueToSegmentThreshold slots, and then pointers to fixed size Segments
/// of contiguous elements in the slots past kValueToSegmentThreshold.
/// All the segments are the same size. If a number of values are needed that
/// extends past a segment's size, a new one is created.
/// The first kValueToSegmentThreshold values are stored directly in the spine
/// so that small arrays do not need to allocate a whole segment.
/// The inline storage grows by doubling the capacity whenever the max is
/// reached, similar to a std::vector or ArrayStorage.
/// Upon resizing, segments are copied to the newly allocated SegmentedArray,
/// and the elements inside are uncopied.
///
/// The original implementation in the JVM is described in this paper:
/// http://hirzels.com/martin/papers/pldi10-arraylets.pdf
/// This version is an adaptation to suit JS.
///
/// Future potential optimizations:
///   * Sharing segments with multiple spines (copy-on-write)
class SegmentedArray final
    : public VariableSizeRuntimeCell,
      private llvh::TrailingObjects<SegmentedArray, GCHermesValue> {
 public:
  /// A segment is just a blob of raw memory with a fixed size.
  class Segment final : public GCCell {
   public:
    /// The max number of elements that can be held in a segment.
    static constexpr uint32_t kMaxLength = 1024;

    /// Creates an empty segment with zero length
    static PseudoHandle<Segment> create(Runtime *runtime);

    static bool classof(const GCCell *cell) {
      return cell->getKind() == CellKind::SegmentKind;
    }

    GCHermesValue &at(uint32_t index) {
      assert(
          index < length() &&
          "Cannot get an index outside of the length of a segment");
      return data_[index];
    }

    /// \p const version of \p at.
    const GCHermesValue &at(uint32_t index) const {
      assert(
          index < length() &&
          "Cannot get an index outside of the length of a segment");
      return data_[index];
    }

    uint32_t length() const {
      return length_.load(std::memory_order_relaxed);
    }

    /// Increases or decreases the length of the segment, up to a max of
    /// kMaxLength. If the length increases, it fills the newly used portion of
    /// the segment with empty values.
    void setLength(Runtime *runtime, uint32_t newLength);

    /// Same as above, except it doesn't fill with empty values.
    /// It is the caller's responsibility to ensure that the newly used portion
    /// will contain valid values before they are accessed (including accesses
    /// by the GC).
    /// \pre This cannot be called if kConcurrentGC is true, since the GC might
    ///   read uninitialized memory even if the mutator wouldn't.
    void setLengthWithoutFilling(uint32_t newLength) {
      assert(!kConcurrentGC && "Cannot avoid filling for a concurrent GC");
      assert(newLength <= kMaxLength && "Cannot set length to more than size");
      length_.store(newLength, std::memory_order_release);
    }

   private:
#ifdef HERMESVM_SERIALIZE
    explicit Segment(Deserializer &d);

    friend void SegmentSerialize(Serializer &s, const GCCell *cell);
    friend void SegmentDeserialize(Deserializer &d, CellKind kind);
#endif

    friend void SegmentBuildMeta(const GCCell *cell, Metadata::Builder &mb);
    static VTable vt;

    AtomicIfConcurrentGC<uint32_t> length_;
    GCHermesValue data_[kMaxLength];

    explicit Segment(Runtime *runtime)
        : GCCell(&runtime->getHeap(), &vt), length_(0) {}
  };

  using size_type = uint32_t;

  /// The threshold at which the storage changes from values to pointers to
  /// segments.
  ///
  /// The current crossover point is when there are enough inline elements to
  /// fill 4 segments. This limits max memory waste of the segments to 25%.
  static constexpr size_type kValueToSegmentThreshold = 4 * Segment::kMaxLength;

  /// A tag that can be used in a template parameter to say that no branch is
  /// needed, it is known beforehand that an element is in the inline storage.
  /// NOTE: This could be a bool, but using a bool literal in a template
  /// argument is hard to read and understand, so we use a more descriptive name
  /// through this enum.
  enum class Inline { No, Yes };

 private:
  /// A TotalIndex is the client-visible index that hides the implementation of
  /// location of elements. It is meant to be used as any normal array or vector
  /// index, assuming efficient random access and addition/subtraction.
  /// A TotalIndex encodes both the SegmentNumber and the InteriorIndex, see \c
  /// toSegment and \c toInterior for how it is encoded.
  using TotalIndex = uint32_t;
  /// A SegmentNumber is the index within the spine of the SegmentedArray for a
  /// Segment. The first index is zero, and starts after the end of the inline
  /// storage.
  using SegmentNumber = uint32_t;
  /// An InteriorIndex is the index within a Segment of an element. This index
  /// is at most \c Segment::kMaxLength - 1.
  using InteriorIndex = uint32_t;

  /// The number of slots for either inline storage or segments that this
  /// SegmentedArray can hold. This is decided at creation time. In order to
  /// have more slots, a new SegmentedArray must be allocated.
  /// NOTE: This can be changed during compaction.
  size_type slotCapacity_;
  /// The number of slots that are currently valid. The \c size() is a derived
  /// field from this value.
  AtomicIfConcurrentGC<size_type> numSlotsUsed_;

  struct iterator {
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = GCHermesValue;
    using difference_type = ptrdiff_t;
    using pointer = GCHermesValue *;
    using reference = GCHermesValue &;

    /// The SegmentedArray which owns this iterator. This iterator should never
    /// be compared against an iterator with a different owner. This is used to
    /// access APIs from SegmentedArray.
    SegmentedArray *const owner_;
    /// The current index that the iterator points at.
    TotalIndex index_;

    explicit iterator(SegmentedArray *owner, TotalIndex index)
        : owner_(owner), index_(index) {
      assert(
          index_ <= owner_->size() &&
          "Cannot make an iterator that points outside of the storage");
    }

    iterator(const iterator &) = default;

    iterator &operator=(const iterator &that) {
      assert(
          owner_ == that.owner_ &&
          "Cannot assign to an iterator from a different SegmentedArray");
      index_ = that.index_;
      assert(
          index_ <= owner_->size() &&
          "Cannot make an iterator that points outside of the storage");
      return *this;
    }

    bool operator==(const iterator &that) const {
      assert(
          owner_ == that.owner_ &&
          "Cannot compare to an iterator from a different SegmentedArray");
      return index_ == that.index_;
    }
    bool operator!=(const iterator &that) const {
      return !(*this == that);
    }
    iterator operator+(TotalIndex index) const {
      assert(
          index_ <= std::numeric_limits<TotalIndex>::max() - index &&
          "Overflow in addition");
      return iterator(owner_, index_ + index);
    }
    iterator operator-(TotalIndex index) const {
      assert(index_ >= index && "Overflow in subtraction");
      return iterator(owner_, index_ - index);
    }
    iterator &operator+=(TotalIndex index) {
      return *this = *this + index;
    }
    iterator &operator-=(TotalIndex index) {
      return *this = *this - index;
    }
    iterator &operator++() {
      return *this += 1;
    }
    iterator &operator--() {
      return *this -= 1;
    }

    reference operator*() {
      assert(
          index_ < owner_->size() &&
          "Trying to read from an index outside the size");
      // Almost all arrays fit entirely in the inline storage.
      if (LLVM_LIKELY(index_ < kValueToSegmentThreshold)) {
        return owner_->inlineStorage()[index_];
      } else {
        return owner_->segmentAt(toSegment(index_))->at(toInterior(index_));
      }
    }

    pointer operator->() {
      return &**this;
    }
  };

 public:
#ifdef HERMESVM_SERIALIZE
  friend void SegmentSerialize(Serializer &s, const GCCell *cell);
  friend void SegmentDeserialize(Deserializer &d, CellKind kind);
  friend void SegmentedArraySerialize(Serializer &s, const GCCell *cell);
  friend void SegmentedArrayDeserialize(Deserializer &d, CellKind kind);
#endif

  static constexpr size_type maxElements();

  /// Creates a new SegmentedArray that has space for at least the requested \p
  /// capacity number of elements, and has size 0.
  static CallResult<PseudoHandle<SegmentedArray>> create(
      Runtime *runtime,
      size_type capacity);
  static CallResult<PseudoHandle<SegmentedArray>> createLongLived(
      Runtime *runtime,
      size_type capacity);
  /// Same as \c create(runtime, capacity) except fills in the first \p size
  /// elements with \p fill, and sets the size to \p size.
  static CallResult<PseudoHandle<SegmentedArray>>
  create(Runtime *runtime, size_type capacity, size_type size);

  /// Gets the element located at \p index.
  template <Inline inl = Inline::No>
  GCHermesValue &at(TotalIndex index) {
    if (inl == Inline::Yes) {
      assert(
          index < kValueToSegmentThreshold && index < size() &&
          "Using the inline storage accessor when the index is larger than the "
          "inline storage");
      return inlineStorage()[index];
    } else {
      return *(begin() + index);
    }
  }

  /// Get the element located at \p index. \p const function for read.
  const GCHermesValue &at(size_type index) const {
    assert(index < size() && "Invalid index.");
    if (index < kValueToSegmentThreshold) {
      return inlineStorage()[index];
    } else {
      return segmentAt(toSegment(index))->at(toInterior(index));
    }
  }

  /// Gets the size of the SegmentedArray. The size is the number of elements
  /// currently active in the array.
  size_type size() const {
    const auto numSlotsUsed = numSlotsUsed_.load(std::memory_order_relaxed);
    if (LLVM_LIKELY(numSlotsUsed <= kValueToSegmentThreshold)) {
      return numSlotsUsed;
    } else {
      const SegmentNumber numSegments = numSlotsUsed - kValueToSegmentThreshold;
      const size_type numBeforeLastSegment =
          kValueToSegmentThreshold + (numSegments - 1) * Segment::kMaxLength;
      const uint32_t numInLastSegment = segmentAt(numSegments - 1)->length();
      return numBeforeLastSegment + numInLastSegment;
    }
  }

  /// Gets the total number of elements that could fit in the SegmentedArray
  /// before any allocations are required.
  size_type capacity() const;

  /// The total number of elements that can fit in this SegmentedArray if
  /// allocations of new segments are allowed.
  size_type totalCapacityOfSpine() const;

  /// Gets the amount of memory used by this object (just the spine, not
  /// including any segments) for a given capacity.
  static constexpr uint32_t allocationSizeForCapacity(size_type capacity) {
    return allocationSizeForSlots(numSlotsForCapacity(capacity));
  }

  /// Increase the size by one and set the new element to \p value.
  static ExecutionStatus push_back(
      MutableHandle<SegmentedArray> &self,
      Runtime *runtime,
      Handle<> value);

  /// Change the size of the storage to \p newSize. This can increase the size
  /// (in which case the new elements will be initialized to empty), or decrease
  /// the size.
  static ExecutionStatus resize(
      MutableHandle<SegmentedArray> &self,
      Runtime *runtime,
      size_type newSize);

  /// The same as resize, but add elements to the left instead of the right.
  ///
  /// In the case where the capacity is sufficient to hold the \p newSize,
  /// every existing element is copied rightward, a linear time procedure.
  /// If the capacity is not sufficient, then the performance will be the same
  /// as \c resize.
  static ExecutionStatus resizeLeft(
      MutableHandle<SegmentedArray> &self,
      Runtime *runtime,
      size_type newSize);

  /// Set the size to a value <= the capacity. This is a special
  /// case of resize() but has a simpler interface since we know that it doesn't
  /// need to reallocate.
  static void resizeWithinCapacity(
      SegmentedArray *self,
      Runtime *runtime,
      size_type newSize);

  /// Decrease the size to zero.
  void clear(Runtime *runtime) {
    shrinkRight(runtime, size());
  }

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::SegmentedArrayKind;
  }

 private:
  static VTable vt;

  friend TrailingObjects;
  friend void SegmentBuildMeta(const GCCell *cell, Metadata::Builder &mb);
  friend void SegmentedArrayBuildMeta(
      const GCCell *cell,
      Metadata::Builder &mb);

  SegmentedArray(Runtime *runtime, size_type capacity)
      : VariableSizeRuntimeCell(
            &runtime->getHeap(),
            &vt,
            allocationSizeForCapacity(capacity)),
        slotCapacity_(numSlotsForCapacity(capacity)),
        numSlotsUsed_(0) {}

#ifdef HERMESVM_SERIALIZE
  /// Constructor used during deserialization. Takes argument \p slotCapacity
  /// instead of \p capacity like in common constructor.
  /// \param slotCapacity The number of slots for either inline storage or
  /// segments that this SegmentedArray can hold.
  SegmentedArray(
      Runtime *runtime,
      size_type slotCapacity,
      size_type numSlotsUsed)
      : VariableSizeRuntimeCell(
            &runtime->getHeap(),
            &vt,
            allocationSizeForSlots(slotCapacity)),
        slotCapacity_(slotCapacity),
        numSlotsUsed_(numSlotsUsed) {}
#endif

  /// Throws a RangeError with a descriptive message describing the attempted
  /// capacity allocated, and the max that is allowed.
  /// \returns ExecutionStatus::EXCEPTION always.
  static ExecutionStatus throwExcessiveCapacityError(
      Runtime *runtime,
      size_type capacity);

  iterator begin() {
    return iterator(this, 0);
  }
  iterator end() {
    return iterator(this, size());
  }
  iterator inlineStorageEnd() {
    return iterator(this, std::min(size(), toRValue(kValueToSegmentThreshold)));
  }

  /// \return the capacity that should be used for a new SegmentedArray based on
  /// the this SegmentedArray's \p currentSize, and the \p newSize that it needs
  /// to grow to.
  static size_type calculateNewCapacity(
      size_type currentSize,
      size_type newSize) {
    // Either double the current size, or increase to be barely big enough to
    // hold the new size, whichever is larger.
    // It's ok if currentSize * 2 overflows, since we know that the returned
    // capacity will still be >= newSize, which is all that is required.
    return std::max(currentSize * 2, newSize);
  }

  /// \return the number of slots that are needed to hold at least the requested
  /// capacity.
  static constexpr SegmentNumber numSlotsForCapacity(size_type capacity) {
    // TODO(T31421960): Use if statements and local variables to clean this up.
    return capacity <= kValueToSegmentThreshold
        ? capacity
        :
        // Enough segments to hold the capacity without inline storage.
        (llvh::alignTo<Segment::kMaxLength>(
             capacity - kValueToSegmentThreshold) /
         Segment::kMaxLength) +
            // The slots for inline storage plus the slots need to hold the
            // number of segments that can hold the capacity.
            kValueToSegmentThreshold;
  }

  /// Given a TotalIndex \p index, \returns the index of the segment within the
  /// slots of the spine that start after kValueToSegmentThreshold that contains
  /// the element stored at \p index. \pre The \p index has to be past the
  /// inline storage.
  static SegmentNumber toSegment(TotalIndex index) {
    assert(
        index >= kValueToSegmentThreshold &&
        "Cannot get the segment of an element in the inline storage");
    return (index - kValueToSegmentThreshold) / Segment::kMaxLength;
  }

  /// Given a TotalIndex \p index, \returns the index of the element within its
  /// segment.
  /// \pre The \p index has to be past the inline storage.
  static InteriorIndex toInterior(TotalIndex index) {
    assert(
        index >= kValueToSegmentThreshold &&
        "Cannot get the interior index of an element in the inline storage");
    return (index - kValueToSegmentThreshold) % Segment::kMaxLength;
  }

  /// Turns an unallocated segment into an allocated one.
  static void allocateSegment(
      Runtime *runtime,
      Handle<SegmentedArray> self,
      SegmentNumber segment);

  /// Gets the amount of memory used by this object's spine for a given number
  /// of spine slots.
  static constexpr uint32_t allocationSizeForSlots(SegmentNumber numSlots) {
    return totalSizeToAlloc<GCHermesValue>(numSlots);
  }

  /// \return a pointer to the segment from the given \p index in the spine. The
  /// segment is somewhere in the GC heap, so don't store this pointer between
  /// any collections.
  /// \pre The \p segment is within the numSlotsUsed_ in the spine, and it has
  /// been allocated.
  Segment *segmentAt(SegmentNumber segment) {
    return const_cast<Segment *>(
        static_cast<const SegmentedArray *>(this)->segmentAt(segment));
  }
  /// const version of \c segmentAt.
  const Segment *segmentAt(SegmentNumber segment) const {
    assert(
        segment < numUsedSegments() &&
        "Trying to get a segment that does not exist");
    return vmcast<Segment>(*segmentAtPossiblyUnallocated(segment));
  }

  /// Same as \c segmentAt, except for any segment, including ones between
  /// numSlotsUsed_ and slotCapacity_.
  GCHermesValue *segmentAtPossiblyUnallocated(SegmentNumber segment) {
    return const_cast<GCHermesValue *>(
        static_cast<const SegmentedArray *>(this)->segmentAtPossiblyUnallocated(
            segment));
  }
  /// const version of \c segmentAtPossiblyUnallocated.
  const GCHermesValue *segmentAtPossiblyUnallocated(
      SegmentNumber segment) const {
    assert(
        segment < numSegments() &&
        "Trying to get a segment that does not exist");
    return segments() + segment;
  }

  /// \return a raw pointer into the inline storage. Can be used like a C array.
  GCHermesValue *inlineStorage() {
    return getTrailingObjects<GCHermesValue>();
  }

  /// Const version of above.
  const GCHermesValue *inlineStorage() const {
    return getTrailingObjects<GCHermesValue>();
  }

  /// \return a raw pointer into the segment slots. Can be used like a C array.
  GCHermesValue *segments() {
    return getTrailingObjects<GCHermesValue>() + kValueToSegmentThreshold;
  }

  /// Const version of above.
  const GCHermesValue *segments() const {
    return getTrailingObjects<GCHermesValue>() + kValueToSegmentThreshold;
  }

  /// \return the number of segments that could be held by this SegmentedArray.
  /// This number does not represent the number of allocated segments, only the
  /// total number of segments that could exist.
  SegmentNumber numSegments() const {
    return slotCapacity_ <= kValueToSegmentThreshold
        ? 0
        : slotCapacity_ - kValueToSegmentThreshold;
  }

  /// \return the number of segments in active use by this SegmentedArray.
  SegmentNumber numUsedSegments() const {
    const auto numSlotsUsed = numSlotsUsed_.load(std::memory_order_relaxed);
    return numSlotsUsed <= kValueToSegmentThreshold
        ? 0
        : numSlotsUsed - kValueToSegmentThreshold;
  }

  /// @name Resize helper functions
  /// @{

  /// Grow the SegmentedArray by the given \p amount, extending to the right and
  /// adding empty values. If size + \p amount is more than capacity can hold, a
  /// re-allocation will occur.
  static ExecutionStatus growRight(
      MutableHandle<SegmentedArray> &self,
      Runtime *runtime,
      size_type amount);

  /// Same as \c growRight, except the empty values are filled to the left.
  /// This operation is slower than \p growRight, because it needs to shift all
  /// existing elements \p amount spaces to the right to make room for the new
  /// empty values.
  static ExecutionStatus growLeft(
      MutableHandle<SegmentedArray> &self,
      Runtime *runtime,
      size_type amount);

  /// Same as \c growRightWithinCapacity except it fills from the left.
  static void growLeftWithinCapacity(
      Runtime *runtime,
      PseudoHandle<SegmentedArray> self,
      size_type amount);

  /// Shrink the array on the right hand side, removing the existing elements.
  /// \p pre amount <= size().
  void shrinkRight(Runtime *runtime, size_type amount);

  /// Shrink the array on the left hand side, removing the existing elements
  /// from the left.
  /// \p pre amount <= size().
  void shrinkLeft(Runtime *runtime, size_type amount);

  /// Increases the size by \p amount, without doing any allocation.
  /// \param fill If true, fill the newly usable space with empty HermesValues.
  void
  increaseSizeWithinCapacity(Runtime *runtime, size_type amount, bool fill);

  /// Increases the size by \p amount, and adjusts segment sizes
  /// accordingly.
  /// NOTE: increasing size can potentially allocate new segments.
  template <bool Fill>
  static PseudoHandle<SegmentedArray> increaseSize(
      Runtime *runtime,
      PseudoHandle<SegmentedArray> self,
      size_type amount);

  /// Decreases the size by \p amount, and no longer tracks the elements past
  /// the new size limit.
  void decreaseSize(Runtime *runtime, size_type amount);

  /// @}

  /// \return the maximum number of segments that can fit in a single allocation
  /// of a SegmentedArray.
  static constexpr SegmentNumber maxNumSegments();
  /// \return the maximum number of segments such that they hold fewer elements
  /// than the max size_type.
  static constexpr SegmentNumber maxNumSegmentsWithoutOverflow();

  static gcheapsize_t _trimSizeCallback(const GCCell *self);
  static void _trimCallback(GCCell *self);
};

constexpr SegmentedArray::size_type SegmentedArray::maxElements() {
  // TODO(T31421960): Use local variables and std::min to clean this up.
  return (maxNumSegments() <= maxNumSegmentsWithoutOverflow()
              ? maxNumSegments()
              : maxNumSegmentsWithoutOverflow()) *
      Segment::kMaxLength +
      kValueToSegmentThreshold;
}

constexpr SegmentedArray::SegmentNumber SegmentedArray::maxNumSegments() {
  return (
      ((GC::maxAllocationSize() - allocationSizeForCapacity(0)) /
       sizeof(GCHermesValue)) -
      kValueToSegmentThreshold);
}

constexpr SegmentedArray::SegmentNumber
SegmentedArray::maxNumSegmentsWithoutOverflow() {
  return (
      (std::numeric_limits<uint32_t>::max() - kValueToSegmentThreshold) /
      Segment::kMaxLength);
}

template <>
struct IsGCObject<SegmentedArray::Segment> : public std::true_type {};

static_assert(
    SegmentedArray::allocationSizeForCapacity(SegmentedArray::maxElements()) <=
        GC::maxAllocationSize(),
    "maxElements() is too big");

} // namespace vm
} // namespace hermes
#endif
