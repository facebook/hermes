/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SEGMENT_H
#define HERMES_VM_SEGMENT_H

#include "hermes/Support/OSCompat.h"
#include "hermes/VM/AdviseUnused.h"
#include "hermes/VM/AlignedStorage.h"
#include "hermes/VM/AllocResult.h"
#include "hermes/VM/AllocSource.h"
#include "hermes/VM/CardTableNC.h"
#include "hermes/VM/GCBase.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/HeapAlign.h"
#include "hermes/VM/MarkBitArrayNC.h"
#include "hermes/VM/PointerBase.h"
#include "hermes/VM/SegmentInfo.h"

#include "llvh/Support/MathExtras.h"

#include <cstdint>
#include <vector>

namespace hermes {
namespace vm {

// In this class:
// TODO (T25527350): Debug Dump
// TODO (T25527350): Heap Moving

/// An \c AlignedHeapSegment is a contiguous chunk of memory aligned to its own
/// storage size (which is a fixed power of two number of bytes).  The storage
/// is further split up according to the diagram below:
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
/// The tables in (1), and (2) cover the contiguous allocation space (3)
/// into which GCCells are bump allocated.
class AlignedHeapSegment {
 public:
  explicit AlignedHeapSegment(AlignedStorage storage);

  /// Construct a null AlignedHeapSegment (one that does not own memory).
  AlignedHeapSegment() = default;
  AlignedHeapSegment(AlignedHeapSegment &&) = default;
  AlignedHeapSegment &operator=(AlignedHeapSegment &&) = default;

  ~AlignedHeapSegment();

  /// Contents of the memory region managed by this segment.
  class Contents {
    friend class AlignedHeapSegment;

    /// Note that because of the Contents object, the first few bytes of the
    /// card table are unused, we instead use them to store a small SegmentInfo
    /// struct.
    CardTable cardTable_;

    MarkBitArrayNC markBitArray_;

    static constexpr size_t kMetadataSize =
        sizeof(cardTable_) + sizeof(markBitArray_);
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

   public:
    /// Set the protection mode of paddedGuardPage_ (if system page size allows
    /// it).
    void protectGuardPage(oscompat::ProtectMode mode);
  };

  static_assert(
      offsetof(Contents, paddedGuardPage_) == Contents::kMetadataSize,
      "Should not need padding after metadata.");

  /// The total space at the start of the CardTable taken up by the metadata and
  /// guard page in the Contents struct.
  static constexpr size_t kCardTableUnusedPrefixBytes =
      Contents::kMetadataAndGuardSize / CardTable::kHeapBytesPerCardByte;
  static_assert(
      sizeof(SegmentInfo) < kCardTableUnusedPrefixBytes,
      "SegmentInfo does not fit in available unused CardTable space.");

  /// The offset from the beginning of a segment of the allocatable region.
  static constexpr size_t offsetOfAllocRegion{offsetof(Contents, allocRegion_)};

  static_assert(
      isSizeHeapAligned(offsetOfAllocRegion),
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

  /// Attempt an allocation of the given size in the segment.  If there is
  /// sufficent space, cast the space as a GCCell, and returns an uninitialized
  /// pointer to that cell (with success = true).  If there is not sufficient
  /// space, returns {nullptr, false}.
  inline AllocResult alloc(uint32_t size);

  /// Given the \p lowLim of some valid AlignedStorage's memory region, returns
  /// a pointer to the AlignedHeapSegment::Contents laid out in that storage,
  /// assuming it exists.
  inline static Contents *contents(void *lowLim);
  inline static const Contents *contents(const void *lowLim);

  /// Given a \p ptr into the memory region of some valid AlignedStorage \c s,
  /// returns a pointer to the CardTable covering the segment containing the
  /// pointer.
  ///
  /// \pre There exists a currently alive heap that claims to contain \c ptr.
  inline static CardTable *cardTableCovering(const void *ptr);

  /// Given a \p ptr into the memory region of some valid AlignedStorage \c s,
  /// returns a pointer to the MarkBitArrayNC covering the segment containing
  /// the pointer.
  ///
  /// \pre There exists a currently alive heap that claims to contain \c ptr.
  inline static MarkBitArrayNC *markBitArrayCovering(const void *ptr);

  /// Mark the given \p cell.  Assumes the given address is a valid heap object.
  inline static void setCellMarkBit(const GCCell *cell);

  /// Return whether the given \p cell is marked.  Assumes the given address is
  /// a valid heap object.
  inline static bool getCellMarkBit(const GCCell *cell);

  /// The largest size the allocation region of an aligned heap segment could
  /// be.
  inline static constexpr size_t maxSize();

  /// The size of the allocation region in this aligned heap segment. (Static
  /// override of \c AlignedStorage::size()).
  inline size_t size() const;

  /// The number of bytes in the segment that are currently allocated.
  inline size_t used() const;

  /// The number of bytes in the segment that are available for allocation.
  inline size_t available() const;

  /// Returns the address that is the lower bound of the segment.
  /// \post The returned pointer is guaranteed to be aligned to a segment
  ///   boundary.
  inline char *lowLim() const;

  /// Returns the address that is the upper bound of the segment.
  inline char *hiLim() const;

  /// Returns the address at which the first allocation in this segment would
  /// occur.
  /// Disable UB sanitization because 'this' may be null during the tests.
  inline char *start() const LLVM_NO_SANITIZE("undefined");

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

  /// Returns the address at which the next allocation, if any, will occur.
  inline char *level() const;

  /// Returns an iterator range corresponding to the cells in this segment.
  inline llvh::iterator_range<HeapCellIterator> cells();

  /// Returns whether \p a and \p b are contained in the same
  /// AlignedHeapSegment.
  inline static bool containedInSame(const void *a, const void *b);

  /// Return a reference to the card table covering the memory region managed by
  /// this segment.
  /// Disable sanitization because 'this' may be null in the tests.
  inline CardTable &cardTable() const LLVM_NO_SANITIZE("null");

  /// Return a reference to the mark bit array covering the memory region
  /// managed by this segment.
  inline MarkBitArrayNC &markBitArray() const;

  explicit inline operator bool() const;

  /// \return \c true if and only if \p ptr is within the memory range owned by
  ///     this \c AlignedHeapSegment.
  inline bool contains(const void *ptr) const;

  /// Assumes that the segment's card object boundaries may not have been
  /// maintained, and recreates it, ensuring that it's valid.
  void recreateCardTableBoundaries();

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

  /// Increase the size of the allocation region in this segment by the minimum
  /// amount such that this.size() >= desired.
  ///
  /// \pre \p desired is page-aligned.
  /// \pre desired <= AlignedHeapSegment::maxSize()
  void growTo(size_t desired);

  /// Decrease the size of the allocation region in this segment by the minimum
  /// amount such that this.size() <= desired.
  ///
  /// \pre \p desired is page-aligned.
  /// \pre 0 < desired <= AlignedHeapSegment::maxSize()
  void shrinkTo(size_t desired);

  /// Grow the allocation region as big as possible.
  inline void growToLimit();

  /// Try to increase the size of the allocation region in this segment so that
  /// at least \p amount bytes become available. Updates the bounds of the
  /// segment and \returns true on success, and \returns false otherwise (If the
  /// request would cause the allocation region to grow bigger than the
  /// underlying storage).
  bool growToFit(size_t amount);

#ifndef NDEBUG
  /// Returns true iff \p lvl could refer to a level within this segment.
  bool dbgContainsLevel(const void *lvl) const;

  /// Returns true iff \p p is located within a valid section of the segment,
  /// and not at dead memory.
  bool validPointer(const void *p) const;

  /// Set the contents of the segment to a dead value.
  void clear();
  /// Set the given range [start, end) to a dead value.
  static void clear(char *start, char *end);
  /// Checks that dead values are present in the [start, end) range.
  static void checkUnwritten(char *start, char *end);
#endif

 protected:
  /// Return a pointer to the contents of the memory region managed by this
  /// segment.
  inline Contents *contents() const;

  AlignedStorage storage_;

  char *level_{start()};

  /// The upper limit of the space that we can currently allocated into;
  /// this may be decreased when externally allocated memory is credited to
  /// the generation owning this space.
  char *effectiveEnd_{start()};

  /// The end of the allocation region.  Initially set to the start (making the
  /// allocation region empty), but fixed up in the body of the constructor.
  char *end_{start()};

#ifdef HERMES_EXTRA_DEBUG
  /// Support summarization of the vtables in the segment.
  /// TODO(T56364255): remove this when the problem is diagnosed.

  /// The level at the time we last summarized, or start(), if we
  /// haven't previously summarized.
  char *lastVTableSummaryLevel_{start()};
  /// The value of the last summary, or else 0 if there has been no summary.
  size_t lastVTableSummary_{0};
#endif
};

AllocResult AlignedHeapSegment::alloc(uint32_t size) {
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

/*static*/
MarkBitArrayNC *AlignedHeapSegment::markBitArrayCovering(const void *ptr) {
  return &contents(AlignedStorage::start(ptr))->markBitArray_;
}

/*static*/
void AlignedHeapSegment::setCellMarkBit(const GCCell *cell) {
  MarkBitArrayNC *markBits = markBitArrayCovering(cell);
  size_t ind = markBits->addressToIndex(cell);
  markBits->mark(ind);
}

/*static*/
bool AlignedHeapSegment::getCellMarkBit(const GCCell *cell) {
  MarkBitArrayNC *markBits = markBitArrayCovering(cell);
  size_t ind = markBits->addressToIndex(cell);
  return markBits->at(ind);
}

/* static */ AlignedHeapSegment::Contents *AlignedHeapSegment::contents(
    void *lowLim) {
  return reinterpret_cast<Contents *>(lowLim);
}

/* static */ const AlignedHeapSegment::Contents *AlignedHeapSegment::contents(
    const void *lowLim) {
  return reinterpret_cast<const Contents *>(lowLim);
}

/* static */ CardTable *AlignedHeapSegment::cardTableCovering(const void *ptr) {
  return &AlignedHeapSegment::contents(AlignedStorage::start(ptr))->cardTable_;
}

/* static */ constexpr size_t AlignedHeapSegment::maxSize() {
  return AlignedStorage::size() - offsetof(Contents, allocRegion_);
}

size_t AlignedHeapSegment::size() const {
  return end() - start();
}

size_t AlignedHeapSegment::used() const {
  return level() - start();
}

size_t AlignedHeapSegment::available() const {
  return effectiveEnd() - level();
}

char *AlignedHeapSegment::lowLim() const {
  return storage_.lowLim();
}

char *AlignedHeapSegment::hiLim() const {
  return storage_.hiLim();
}

char *AlignedHeapSegment::start() const {
  return contents()->allocRegion_;
}

char *AlignedHeapSegment::effectiveEnd() const {
  return effectiveEnd_;
}

char *AlignedHeapSegment::end() const {
  return end_;
}

char *AlignedHeapSegment::level() const {
  return level_;
}

llvh::iterator_range<AlignedHeapSegment::HeapCellIterator>
AlignedHeapSegment::cells() {
  return {
      HeapCellIterator(reinterpret_cast<GCCell *>(start())),
      HeapCellIterator(reinterpret_cast<GCCell *>(level()))};
}

/* static */
bool AlignedHeapSegment::containedInSame(const void *a, const void *b) {
  return AlignedStorage::containedInSame(a, b);
}

CardTable &AlignedHeapSegment::cardTable() const {
  return contents()->cardTable_;
}

MarkBitArrayNC &AlignedHeapSegment::markBitArray() const {
  return contents()->markBitArray_;
}

AlignedHeapSegment::Contents *AlignedHeapSegment::contents() const {
  return contents(lowLim());
}

AlignedHeapSegment::operator bool() const {
  return static_cast<bool>(storage_);
}

bool AlignedHeapSegment::contains(const void *ptr) const {
  return storage_.contains(ptr);
}

void AlignedHeapSegment::growToLimit() {
  growTo(AlignedHeapSegment::maxSize());
  clearExternalMemoryCharge();
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SEGMENT_H
