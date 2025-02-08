/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SEGMENT_H
#define HERMES_VM_SEGMENT_H

#include "hermes/ADT/BitArray.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/AdviseUnused.h"
#include "hermes/VM/AllocResult.h"
#include "hermes/VM/AllocSource.h"
#include "hermes/VM/CardTableNC.h"
#include "hermes/VM/GCBase.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/HeapAlign.h"

#include "llvh/Support/MathExtras.h"

#include <cstdint>
#include <vector>

namespace hermes {
namespace vm {

class StorageProvider;

#ifndef HERMESVM_LOG_HEAP_SEGMENT_SIZE
#error Heap segment size must be defined.
#endif

// In this class:
// TODO (T25527350): Debug Dump
// TODO (T25527350): Heap Moving

/// An \c AlignedHeapSegment manages a contiguous chunk of memory aligned to
/// kSegmentUnitSize. The storage is further split up according to the diagram
/// below:
///
/// +----------------------------------------+
/// | (1) Card Table                         |
/// +----------------------------------------+
/// | (2) Mark Bit Array                     |
/// +----------------------------------------+
/// | (3) Allocation Space                   |
/// |                                        |
/// | ...                                    |
/// |                                        |
/// | (End)                                  |
/// +----------------------------------------+
///
/// The tables in (1), and (2) cover the contiguous allocation space (3) into
/// which GCCells are bump allocated. They have fixed size computed from
/// kSegmentUnitSize. For segments whose size is some non-unit multiple of
/// kSegmentUnitSize, card table allocates its internal arrays separately
/// instead. Only one GCCell is allowed in each such segment, so the inline
/// Mark Bit Array is large enough. Any segment size smaller than
/// kSegmentUnitSize is not supported. The headers of all GCCells, in any
/// segment type, must reside in the first region of kSegmentUnitSize. This
/// invariant ensures that we can always get the card table from a valid GCCell
/// pointer.
class AlignedHeapSegment {
 public:
  /// Heap segment size in log of 2.
  static constexpr size_t kLogSize = HERMESVM_LOG_HEAP_SEGMENT_SIZE;
  /// The unit segment size, in bytes. Any valid heap segment must have a size
  /// equal to or be a multiple of it.
  static constexpr size_t kSegmentUnitSize = (1 << kLogSize);

  /// Contents of the memory region managed by this segment.
  class Contents {
   public:
    /// The number of bits representing the total number of heap-aligned
    /// addresses in the segment storage.
    static constexpr size_t kMarkBitArraySize =
        kSegmentUnitSize >> LogHeapAlign;
    /// BitArray for marking allocation region of a segment.
    using MarkBitArray = BitArray<kMarkBitArraySize>;

    /// Set the protection mode of paddedGuardPage_ (if system page size allows
    /// it).
    void protectGuardPage(oscompat::ProtectMode mode);

   private:
    friend class FixedSizeHeapSegment;
    friend class AlignedHeapSegment;

    /// Pass segment size to CardTable constructor to allocate its data
    /// separately if \p sz > kSegmentUnitSize.
    Contents(size_t segmentSize) : cardTable_(segmentSize) {}

    /// Note that because of the Contents object, the first few bytes of the
    /// card table are unused, we instead use them to store a small
    /// SHSegmentInfo struct.
    CardTable cardTable_;

    MarkBitArray markBitArray_;

    static constexpr size_t kMetadataSize =
        sizeof(cardTable_) + sizeof(MarkBitArray);
    /// Padding to ensure that the guard page is aligned to a page boundary.
    static constexpr size_t kGuardPagePadding =
        llvh::alignTo<pagesize::kExpectedPageSize>(kMetadataSize) -
        kMetadataSize;

    /// Memory made inaccessible through protectGuardPage, for security and
    /// earlier detection of corruption. Padded to contain at least one full
    /// aligned page.
    char paddedGuardPage_[pagesize::kExpectedPageSize + kGuardPagePadding];

    static constexpr size_t kMetadataAndGuardSize =
        kMetadataSize + sizeof(paddedGuardPage_);

    /// The first byte of the allocation region, which extends past the "end" of
    /// the struct, to the end of the memory region that contains it.
    char allocRegion_[1];
  };

  static_assert(
      offsetof(Contents, paddedGuardPage_) == Contents::kMetadataSize,
      "Should not need padding after metadata.");

  /// The total space at the start of the CardTable taken up by the metadata and
  /// guard page in the Contents struct.
  static constexpr size_t kCardTableUnusedPrefixBytes =
      Contents::kMetadataAndGuardSize / CardTable::kHeapBytesPerCardByte;
  static_assert(
      sizeof(SHSegmentInfo) < kCardTableUnusedPrefixBytes,
      "SHSegmentInfo does not fit in available unused CardTable space.");

  /// The offset from the beginning of a segment of the allocatable region.
  static constexpr size_t kOffsetOfAllocRegion{
      offsetof(Contents, allocRegion_)};

  static_assert(
      isSizeHeapAligned(kOffsetOfAllocRegion),
      "Allocation region must start at a heap aligned offset");

  static_assert(
      (offsetof(Contents, paddedGuardPage_) + Contents::kGuardPagePadding) %
              pagesize::kExpectedPageSize ==
          0,
      "Guard page must be aligned to likely page size");

  class HeapCellIterator : public llvh::iterator_facade_base<
                               HeapCellIterator,
                               std::forward_iterator_tag,
                               GCCell *> {
   public:
    HeapCellIterator(GCCell *cell) : cell_(cell) {}

    bool operator==(const HeapCellIterator &R) const {
      return cell_ == R.cell_;
    }

    GCCell *const &operator*() const {
      return cell_;
    }

    HeapCellIterator &operator++() {
      cell_ = cell_->nextCell();
      return *this;
    }

   private:
    GCCell *cell_{nullptr};
  };

  /// Returns the address that is the lower bound of the segment.
  /// \post The returned pointer is guaranteed to be aligned to
  /// kSegmentUnitSize.
  char *lowLim() const {
    return lowLim_;
  }

  /// Get the size of this segment.
  size_t storageSize() const {
#ifdef HERMES_SLOW_DEBUG
    auto *segmentInfo = reinterpret_cast<const SHSegmentInfo *>(lowLim_);
    auto sz = (size_t)segmentInfo->shiftedSegmentSize
        << HERMESVM_LOG_HEAP_SEGMENT_SIZE;
    assert(
        sz == segmentSize_ &&
        "segmentSize_ in FixedSizeHeapSegment must always be equal to the size stored in SHSegmentInfo");
#endif
    return segmentSize_;
  }

  /// The largest size the allocation region of an aligned heap segment could
  /// be.
  size_t maxSize() const {
    return storageSize() - kOffsetOfAllocRegion;
  }

  /// Returns the address that is the upper bound of the segment.
  char *hiLim() const {
    return lowLim() + storageSize();
  }

  /// Returns the address at which the first allocation in this segment would
  /// occur.
  /// Disable UB sanitization because 'this' may be null during the tests.
  char *start() const LLVM_NO_SANITIZE("undefined") {
    return contents()->allocRegion_;
  }

  /// Returns the address at which the next allocation, if any, will occur.
  char *level() const {
    return level_;
  }

  /// Return a reference to the card table covering the memory region managed by
  /// this segment.
  CardTable &cardTable() const {
    return contents()->cardTable_;
  }

  /// Given a \p cell lives in the memory region of some valid segment \c s,
  /// returns a pointer to the CardTable covering the segment containing the
  /// cell. Note that this takes a GCCell pointer in order to correctly get
  /// the segment starting address for JumboHeapSegment.
  ///
  /// \pre There exists a currently alive heap in which \p cell is allocated.
  static CardTable *cardTableCovering(const GCCell *cell) {
    return &contents(alignedStorageStart(cell))->cardTable_;
  }

  /// Find the head of the first cell that extends into the card at index
  /// \p cardIdx.
  /// \return A cell such that
  /// cell <= indexToAddress(cardIdx) < cell->nextCell().
  GCCell *getFirstCellHead(size_t cardIdx) {
    CardTable &cards = cardTable();
    GCCell *cell = cards.firstObjForCard(cardIdx);
    assert(cell->isValid() && "Object head doesn't point to a valid object");
    return cell;
  }

  /// Record the head of this cell so it can be found by the card scanner.
  static void setCellHead(const GCCell *cellStart, const size_t sz) {
    const char *start = reinterpret_cast<const char *>(cellStart);
    const char *end = start + sz;
    CardTable *cards = cardTableCovering(cellStart);
    auto boundary = cards->nextBoundary(start);
    // If this object crosses a card boundary, then update boundaries
    // appropriately.
    if (boundary.address() < end) {
      cards->updateBoundaries(&boundary, start, end);
    }
  }

  /// Return a reference to the mark bit array covering the memory region
  /// managed by this segment.
  Contents::MarkBitArray &markBitArray() const {
    return contents()->markBitArray_;
  }

  /// Mark the given \p cell. Assumes the given address is a valid heap object.
  static void setCellMarkBit(const GCCell *cell) {
    auto *markBits = markBitArrayCovering(cell);
    size_t ind = addressToMarkBitArrayIndex(cell);
    markBits->set(ind, true);
  }

  /// Return whether the given \p cell is marked. Assumes the given address is
  /// a valid heap object.
  static bool getCellMarkBit(const GCCell *cell) {
    auto *markBits = markBitArrayCovering(cell);
    size_t ind = addressToMarkBitArrayIndex(cell);
    return markBits->at(ind);
  }

  /// Translate the given address to a 0-based index in the MarkBitArray of its
  /// segment. The base address is the start of the storage of this segment. For
  /// JumboSegment, this should always return a constant index
  /// kOffsetOfAllocRegion >> LogHeapAlign.
  static size_t addressToMarkBitArrayIndex(const GCCell *cell) {
    auto *cp = reinterpret_cast<const char *>(cell);
    auto *base = reinterpret_cast<const char *>(alignedStorageStart(cell));
    return (cp - base) >> LogHeapAlign;
  }

  /// Return true if object \p a and \p b live in the same segment. This is used
  /// to check if a pointer field in \p a may points to an object in the same
  /// segment (so that we don't need to dirty the cards). This also works for
  /// large segment, since there is only one cell in those segments (i.e., \p a
  /// and \p b would be the same).
  static bool containedInSame(const GCCell *a, const GCCell *b) {
    return (reinterpret_cast<uintptr_t>(a) ^ reinterpret_cast<uintptr_t>(b)) <
        kSegmentUnitSize;
  }

#ifndef NDEBUG
  /// Get the storage end of segment that \p cell resides in.
  static char *storageEnd(const GCCell *cell) {
    auto *start = alignedStorageStart(cell);
    auto *segmentInfo = reinterpret_cast<const SHSegmentInfo *>(start);
    return start +
        (segmentInfo->shiftedSegmentSize << HERMESVM_LOG_HEAP_SEGMENT_SIZE);
  }
#endif

 protected:
  AlignedHeapSegment() = default;

  /// Construct Contents() at the address of \p lowLim.
  AlignedHeapSegment(void *lowLim, size_t segmentSize)
      : lowLim_(reinterpret_cast<char *>(lowLim)), segmentSize_(segmentSize) {
    new (contents()) Contents(segmentSize);
    contents()->protectGuardPage(oscompat::ProtectMode::None);
  }

  /// Return a pointer to the contents of the memory region managed by this
  /// segment.
  Contents *contents() const {
    return reinterpret_cast<Contents *>(lowLim_);
  }

  /// Given the \p lowLim of some valid segment's memory region, returns a
  /// pointer to the Contents laid out in the storage, assuming it exists.
  static Contents *contents(void *lowLim) {
    return reinterpret_cast<Contents *>(lowLim);
  }

  /// The start of the aligned segment.
  char *lowLim_{nullptr};

  /// The current address in this segment to allocate new object. This must be
  /// positioned after lowLim_ to be correctly initialized.
  char *level_{start()};

  /// The size of this segment.
  size_t segmentSize_;

 private:
  /// Return the starting address for aligned region of size kSegmentUnitSize
  /// that \p cell resides in. If \c cell resides in a JumboSegment, it's the
  /// only cell there, this essentially returns its segment starting address.
  static char *alignedStorageStart(const GCCell *cell) {
    return reinterpret_cast<char *>(
        reinterpret_cast<uintptr_t>(cell) & ~(kSegmentUnitSize - 1));
  }

  /// Given a \p cell, returns a pointer to the MarkBitArray covering the
  /// segment that \p cell resides in.
  ///
  /// \pre There exists a currently alive heap that claims to contain \c cell.
  static Contents::MarkBitArray *markBitArrayCovering(const GCCell *cell) {
    auto *segStart = alignedStorageStart(cell);
    return &contents(segStart)->markBitArray_;
  }
};

/// JumboHeapSegment has custom storage size that must be a multiple of
/// kSegmentUnitSize. Each such segment can only allocate a single object that
/// occupies the entire allocation space. Therefore, the inline MarkBitArray is
/// large enough, while CardTable needs to allocate its cards and boundaries
/// arrays separately.
class JumboHeapSegment : public AlignedHeapSegment {};

/// FixedSizeHeapSegment has fixed storage size kSegmentUnitSize. Its CardTable
/// and MarkBitArray are stored inline right before the allocation space. This
/// is used for all allocations in YoungGen and normal object allocations in
/// OldGen.
class FixedSizeHeapSegment : public AlignedHeapSegment {
 public:
  /// @name Constants and utility functions for the aligned storage of \c
  /// FixedSizeHeapSegment.
  ///
  /// @{
  /// The size and the alignment of the storage, in bytes.
  static constexpr size_t kSize = kSegmentUnitSize;
  /// Mask for isolating the offset into a storage for a pointer.
  static constexpr size_t kLowMask{kSize - 1};
  /// Mask for isolating the storage being pointed into by a pointer.
  static constexpr size_t kHighMask{~kLowMask};

  /// Returns the storage size, in bytes, of a \c FixedSizeHeapSegment. This is
  /// a static override of AlignedHeapSegment::storageSize(), since here the
  /// segment size is a constant.
  static constexpr size_t storageSize() {
    return kSize;
  }

  /// Returns the pointer to the beginning of the storage containing \p ptr
  /// (inclusive). Assuming such a storage exists. Note that
  ///
  ///     storageStart(seg.hiLim()) != seg.lowLim()
  ///
  /// as \c seg.hiLim() is not contained in the bounds of \c seg -- it
  /// is the first address not in the bounds.
  static void *storageStart(const void *ptr) {
    return reinterpret_cast<char *>(
        reinterpret_cast<uintptr_t>(ptr) & kHighMask);
  }

  /// Returns the pointer to the end of the storage containing \p ptr
  /// (exclusive). Assuming such a storage exists. Note that
  ///
  ///     storageEnd(seg.hiLim()) != seg.hiLim()
  ///
  /// as \c seg.hiLim() is not contained in the bounds of \c seg -- it
  /// is the first address not in the bounds.
  static void *storageEnd(const void *ptr) {
    return reinterpret_cast<char *>(storageStart(ptr)) + kSize;
  }

  /// Returns the offset in bytes to \p ptr from the start of its containing
  /// storage. Assuming such a storage exists. Note that
  ///
  ///     offset(seg.hiLim()) != seg.size()
  ///
  /// as \c seg.hiLim() is not contained in the bounds of \c seg -- it
  /// is the first address not in the bounds.
  static size_t offset(const char *ptr) {
    return reinterpret_cast<size_t>(ptr) & kLowMask;
  }
  /// @}

  /// Construct a null FixedSizeHeapSegment (one that does not own memory).
  FixedSizeHeapSegment() = default;
  /// \c FixedSizeHeapSegment is movable and assignable, but not copyable.
  FixedSizeHeapSegment(FixedSizeHeapSegment &&);
  FixedSizeHeapSegment &operator=(FixedSizeHeapSegment &&);
  FixedSizeHeapSegment(const FixedSizeHeapSegment &) = delete;
  FixedSizeHeapSegment &operator=(const FixedSizeHeapSegment &) = delete;

  ~FixedSizeHeapSegment();

  /// Create a FixedSizeHeapSegment by allocating memory with \p provider.
  static llvh::ErrorOr<FixedSizeHeapSegment> create(
      StorageProvider *provider,
      const char *name = nullptr);

  /// Returns the index of the segment containing \p lowLim, which is required
  /// to be the start of its containing segment.  (This can allow extra
  /// efficiency, in cases where the segment start has already been computed.)
  static unsigned getSegmentIndexFromStart(const void *lowLim) {
    assert(lowLim == storageStart(lowLim) && "Precondition.");
    auto *segInfo = reinterpret_cast<const SHSegmentInfo *>(lowLim);
    return segInfo->index;
  }

  /// Requires that \p lowLim is the start address of a segment, and sets
  /// that segment's index to \p index.
  static void setSegmentIndexFromStart(void *lowLim, unsigned index) {
    assert(lowLim == storageStart(lowLim) && "Precondition.");
    auto *segInfo = reinterpret_cast<SHSegmentInfo *>(lowLim);
    segInfo->index = index;
  }

  /// Attempt an allocation of the given size in the segment.  If there is
  /// sufficent space, cast the space as a GCCell, and returns an uninitialized
  /// pointer to that cell (with success = true).  If there is not sufficient
  /// space, returns {nullptr, false}.
  inline AllocResult alloc(uint32_t size);

  /// Given a \p ptr into the memory region of some valid segment \c s, returns
  /// a pointer to the CardTable covering the segment containing the pointer.
  ///
  /// \pre There exists a currently alive heap that claims to contain \c ptr.
  inline static CardTable *cardTableCovering(const void *ptr);

  /// The largest size the allocation region of an aligned heap segment could
  /// be. This is a static override of AlignedHeapSegment::maxSize().
  inline static constexpr size_t maxSize();

  /// The size of the allocation region in this aligned heap segment.
  inline size_t size() const;

  /// The number of bytes in the segment that are currently allocated.
  inline size_t used() const;

  /// The number of bytes in the segment that are available for allocation.
  inline size_t available() const;

  /// Returns the first address after the region in which allocations can occur,
  /// taking external memory credits into a account (they decrease the effective
  /// end).
  inline char *effectiveEnd() const;

  /// The external memory charge of the generation owning this segment may have
  /// changed; set the effective end of the segment to the given \p
  /// effectiveEnd, which is required to be a valid effective end for the
  /// segment.
  void setEffectiveEnd(char *effectiveEnd);

  /// Clear any external memory charge for this segment.  This has the effect of
  /// equating the effective end to the real end.
  void clearExternalMemoryCharge();

  /// Returns the first address after the region in which allocations may occur,
  /// ignoring external memory credits.
  inline char *end() const;

  /// Returns an iterator range corresponding to the cells in this segment.
  inline llvh::iterator_range<HeapCellIterator> cells();

  /// Returns whether \p a and \p b are contained in the same
  /// FixedSizeHeapSegment.
  inline static bool containedInSame(const void *a, const void *b);

  explicit operator bool() const {
    return lowLim();
  }

  /// \return \c true if and only if \p ptr is within the memory range owned by
  ///     this \c FixedSizeHeapSegment.
  bool contains(const void *ptr) const {
    return storageStart(ptr) == lowLim();
  }

  /// Mark a set of pages as unused.
  ///
  /// \pre start and end must be aligned to the page boundary.
  void markUnused(char *start, char *end);

  /// Clear allocations after \p level in this segment.
  ///
  /// \p MU Indicate whether the newly freed pages should be returned to the OS.
  ///
  /// \post this->level() == level.
  template <AdviseUnused MU = AdviseUnused::No>
  void setLevel(char *level);

  /// Clear allocations in this segment.
  ///
  /// \p MU Indicate whether the newly freed pages should be returned to the OS.
  ///
  /// \post this->level() == this->start();
  template <AdviseUnused MU = AdviseUnused::No>
  void resetLevel();

#ifndef NDEBUG
  /// Returns true iff \p lvl could refer to a level within this segment.
  bool dbgContainsLevel(const void *lvl) const;

  /// Returns true iff \p p is located within a valid section of the segment,
  /// and not at dead memory.
  bool validPointer(const void *p) const;

  /// Set the contents of the segment to a dead value.
  void clear();
  /// Checks that dead values are present in the [start, end) range.
  static void checkUnwritten(char *start, char *end);
#endif

 private:
  /// The provider that created this segment. It will be used to properly
  /// destroy this.
  StorageProvider *provider_{nullptr};

  /// The upper limit of the space that we can currently allocated into;
  /// this may be decreased when externally allocated memory is credited to
  /// the generation owning this space.
  char *effectiveEnd_{end()};

  /// Used in move constructor and move assignment operator following the copy
  /// and swap idiom.
  friend void swap(FixedSizeHeapSegment &a, FixedSizeHeapSegment &b);

  FixedSizeHeapSegment(StorageProvider *provider, void *lowLim);
};

AllocResult FixedSizeHeapSegment::alloc(uint32_t size) {
  assert(lowLim() != nullptr && "Cannot allocate in a null segment");
  assert(size >= sizeof(GCCell) && "cell must be larger than GCCell");
  assert(isSizeHeapAligned(size) && "size must be heap aligned");

  char *cellPtr; // Initialized in the if below.

  // On 64-bit systems, we know that we can't allocate a size large enough to
  // cause a pointer value to overflow. This is not true on 32-bit systems. This
  // test should be decided statically.
  // TODO: Is there a portable way of expressing this in the preprocessor?
  if (sizeof(void *) == 8) {
    // Calculate the new level_ once.
    char *newLevel = level_ + size;
    if (LLVM_UNLIKELY(newLevel > effectiveEnd())) {
      return {nullptr, false};
    }
    cellPtr = level_;
    level_ = newLevel;
  } else {
    if (LLVM_UNLIKELY(available() < size)) {
      return {nullptr, false};
    }
    cellPtr = level_;
    level_ += size;
  }

  __asan_unpoison_memory_region(cellPtr, size);
#ifndef NDEBUG
  checkUnwritten(cellPtr, cellPtr + size);
#endif

  auto *cell = reinterpret_cast<GCCell *>(cellPtr);
  return {cell, true};
}

/* static */ CardTable *FixedSizeHeapSegment::cardTableCovering(
    const void *ptr) {
  return &FixedSizeHeapSegment::contents(storageStart(ptr))->cardTable_;
}

/* static */ constexpr size_t FixedSizeHeapSegment::maxSize() {
  return storageSize() - kOffsetOfAllocRegion;
}

size_t FixedSizeHeapSegment::size() const {
  return end() - start();
}

size_t FixedSizeHeapSegment::used() const {
  return level() - start();
}

size_t FixedSizeHeapSegment::available() const {
  return effectiveEnd() - level();
}

char *FixedSizeHeapSegment::effectiveEnd() const {
  return effectiveEnd_;
}

char *FixedSizeHeapSegment::end() const {
  return start() + maxSize();
}

llvh::iterator_range<FixedSizeHeapSegment::HeapCellIterator>
FixedSizeHeapSegment::cells() {
  return {
      HeapCellIterator(reinterpret_cast<GCCell *>(start())),
      HeapCellIterator(reinterpret_cast<GCCell *>(level()))};
}

/* static */
bool FixedSizeHeapSegment::containedInSame(const void *a, const void *b) {
  return (reinterpret_cast<uintptr_t>(a) ^ reinterpret_cast<uintptr_t>(b)) <
      storageSize();
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SEGMENT_H
