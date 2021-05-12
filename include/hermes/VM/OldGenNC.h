/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_OLDGEN_H
#define HERMES_VM_OLDGEN_H

#include "hermes/VM/AlignedStorage.h"
#include "hermes/VM/AllocOptions.h"
#include "hermes/VM/AllocResult.h"
#include "hermes/VM/CardTableNC.h"
#include "hermes/VM/CompactionResult.h"
#include "hermes/VM/CompleteMarkState.h"
#include "hermes/VM/GCGeneration.h"
#include "hermes/VM/GCSegmentRange-inline.h"
#include "hermes/VM/GCSegmentRange.h"
#include "hermes/VM/GenGCHeapSegment.h"
#include "hermes/VM/OldGenSegmentRanges.h"
#include "hermes/VM/SweepResultNC.h"
#include "hermes/VM/YoungGenNC.h"

#include "llvh/ADT/DenseSet.h"
#include "llvh/ADT/iterator_range.h"

#include <cstddef>
#include <deque>
#include <vector>

namespace hermes {
namespace vm {

/// A generation that can function as the old generation in a two-generation
/// system.  Supports finding old-to-young pointers.  This version is
/// "segmented": it is organized as a sequence of "segments" allocated within
/// separately-allocated virtual memory regions that are of size and alignment
/// 2^K, for some suitable K.
///
/// The starting addresses of the segments' virtual memory regions do not
/// necessarily increase in the order the segments are used by the generation.
/// Thus there are two orderings we commonly apply over memory regions owned by
/// this generation: Given regions M and N:
///
/// - The address ordering: Region M is ordered before region N if M's
///   starting address occurs before N's.
///
/// - The logical ordering: Region M is ordered before region N if when filling
///   the generation's heap byte-by-byte, we start filling M before N.
///
/// @name External Memory
/// @{
///
/// We want to treat external memory "as if" it had been allocated in the heap.
/// If we don't do this, small amounts of JS heap space could root large amounts
/// of external memory, and the overall space usage could grow very large.
/// Further, we want the generations to be "credited" with the memory associated
/// with objects they contain, so that young- and old-generation collections
/// happen at times similar to when they would if the external memory were
/// allocated within the heap.  We do this by conceptually considering the
/// external memory to be allocated at the logical high end of the generation,
/// reducing the "effective end" of the generation. In the non-contiguous
/// OldGen, this presents a subtle problem.  When we decide whether to attempt a
/// young-gen collection, we query the available memory of the old generation,
/// to see if, in the worst case, all the data in the young-gen could fit.
/// We need the answer to be accurate, since we have no way of allowing a
/// promotion allocation to fail.  In the non-contiguous heap, this available
/// memory may be split across multiple segments, raising the possibility of
/// fragmentation decreasing the "effective" available memory.  We solve that by
/// (a) limiting the size of the young gen to a single segment, and (2) ensuring
/// that if the old generation has more than one segment, the last segment's
/// size is the maximum segment size.  In the absence of external memory, this
/// solves the problem: either the old generation has a single segment, and
/// available() is accurate, or it has more than one segment, and the last
/// segment is available as a contiguous space.
///
/// External memory messes this up by decreasing the effective size of this last
/// generation, reintroducing the "fragmentation loss" issue.  We solve this by
/// conceptually "allocating" the external memory within the "fragmentation"
/// left in regions prior to the last.  Consider a situation in which there is E
/// external memory, and there are two regions in the old gen, with space S
/// available in first region, and Max - E in the second.  Available is S + (Max
/// - E); assume this is exactly equal to the used() of the young generation.
/// We allocate in the first segment, but then an allocation attempt fails,
/// leaving F space unused.  We allocated S - F in that segment.  We consider F
/// bytes of the external memory to be "allocated" in the first segment, and now
/// consider only (E - F) of external memory to be allocated at the end.  So the
/// available memory in the last segment is now Max - (E - F); the sum of these
/// is S + Max - E, exactly the available space reported, and all the promotion
/// allocation is guaranteed to succeed.
///
/// @}
class OldGen : public GCGeneration {
 public:
  /// See comment in GCGeneration.
  class Size final {
   public:
    Size(gcheapsize_t min, gcheapsize_t max);

    gcheapsize_t min() const {
      return min_;
    }

    gcheapsize_t max() const {
      return max_;
    }

    gcheapsize_t storageFootprint() const;
    gcheapsize_t minStorageFootprint() const;

    gcheapsize_t adjustSize(gcheapsize_t desired) const {
      return adjustSizeWithBounds(desired, min_, max_);
    }

   private:
    friend class OldGen;

    gcheapsize_t min_;
    gcheapsize_t max_;

    /// The minimum number of segments that can be allocated.
    inline size_t minSegments() const;
    /// The maximum number of segments that can be allocated.
    inline size_t maxSegments() const;

    /// The number of segments needed to allocate the given size.  (Rounds up,
    /// conservatively, if necessary.)
    inline static size_t segmentsForSize(size_t size);

    static gcheapsize_t
    adjustSizeWithBounds(size_t desired, size_t min, size_t max);
  };

  /// Initialize the OldGen as a generation in the given GenGC.  The \p minSize
  /// and \p maxSize arguments are hints for the minimum and maximum generation
  /// size, respectively, in bytes.
  OldGen(GenGC *gc, Size ogs, bool releaseUnused);

  /// @name GCGeneration API
  /// @{

  /// See GCGeneration.h for more information.
  AllocResult allocSlow(uint32_t size, HasFinalizer hasFinalizer);
  inline AllocResult allocRaw(uint32_t size, HasFinalizer hasFinalizer);
  inline size_t used() const;
  inline size_t size() const;
  inline size_t minSize() const;
  inline size_t maxSize() const;
  size_t available() const;
  inline size_t adjustSize(size_t size) const;
  void growTo(size_t desired);
  void shrinkTo(size_t desired);
  bool growToFit(size_t amount);
  inline GCSegmentRange::Ptr allSegments();
  gcheapsize_t bytesAllocatedSinceLastGC() const;
  template <typename F>
  inline void forUsedSegments(F callback);
  template <typename F>
  inline void forUsedSegments(F callback) const;
  template <typename F>
  inline bool whileUsedSegments(F callback);
  template <typename F>
  inline bool whileUsedSegments(F callback) const;
  void forAllObjs(const std::function<void(GCCell *)> &callback);
  void creditExternalMemory(uint32_t size);
  void debitExternalMemory(uint32_t size);
  void updateEffectiveEndForExternalMemory();

#ifndef NDEBUG
  bool dbgContains(const void *p) const final override;
  void forObjsAllocatedSinceGC(const std::function<void(GCCell *)> &callback);
#endif

#ifdef HERMES_EXTRA_DEBUG
  /// Extra debugging: at the end of GC we "summarize" the vtable pointers of
  /// old-gen objects.  Conceptually, this means treat them as if they
  /// were concatenated, and take the has of a string form.
  /// TODO(T56364255): remove these when the problem is diagnosed.

  /// Summarize the vtables of old-gen objects, and record the results
  /// (including the last object summarized).
  void summarizeOldGenVTables();

  /// Resummarize the vtables of the old-gen objects, and check whether the
  /// result is the same as the last summary.  Does logging and/or adds
  /// CrashManager custom breakpad data, if it is not.  The \p fullGCNum is
  /// the ordinal number of the full GC that is doing the check (at its start).
  void checkSummarizedOldGenVTables(unsigned fullGCNum);
#endif

  /// @}

  /// Assumes the generation owns its allocation context.  Attempts to allocate
  /// in that; if that fails, calls allocSlow.
  inline AllocResult alloc(uint32_t size, HasFinalizer hasFinalizer);

  /// A location in the old gen: a pair of a segment index, and a
  /// pointer into that segment.
  struct Location {
    size_t segmentNum;
    char *ptr;

    Location() : segmentNum(-1), ptr(nullptr) {}

    Location(size_t segmentNum, char *ptr) : segmentNum(segmentNum), ptr(ptr) {}

    explicit operator bool() const {
      return ptr != nullptr;
    }

    /// We order segments lexicographically, first comparing segment
    /// numbers, then pointers for Locations in the same segment.
    bool operator<(const Location &that) const {
      assert(*this && that && "Locations must be valid.");
      return segmentNum < that.segmentNum ||
          (segmentNum == that.segmentNum && ptr < that.ptr);
    }

    bool operator>=(const Location &that) const {
      return !(*this < that);
    }
  };

  /// An allocation yielded \p alloc, and \p nextAlloc is one byte after the
  /// end of the allocated object.  This allocation extended into a new card.
  /// Update the card boundary table.
  void updateBoundariesAfterAlloc(char *alloc, char *nextAlloc);

  /// The current allocation position.  The first version may be used always;
  /// the availableDirect version may only be used when the generation owns its
  /// allocation context, but is faster.
  inline Location level() const;
  inline Location levelDirect() const;

  /// The distance of the current level from the start of the generation's
  /// allocation region, in logical order.
  inline size_t levelOffset() const;

  /// Make sure a sequence of allocations is guaranteed to succeed.
  ///
  /// \p amount The sum of sizes in the sequence of allocations.
  ///
  /// \pre The largest allocation size is smaller than
  ///     GenGCHeapSegment::maxSize()
  ///
  /// \return true if it was possible to guarantee the requested allocation
  ///     quota could be met, immediately following this call.
  bool ensureFits(size_t amount);

  /// Find all pointers in the current generation that point into
  /// youngGen, and apply the current mark function to them.
  void markYoungGenPointers(Location originalLevel);

  /// Complete an in-progress young-gen collection.  Some number of
  /// young-gen objects have been found reachable and promoted into
  /// the current generation (e.g., by root or card scanning).  The
  /// first of these is at \p toScan.  Apply the given \p acceptor to
  /// these promoted objects.  This may, in turn, promote more
  /// objects; continue until all of the promoted objects have been
  /// scanned.
  void youngGenTransitiveClosure(
      const Location &toScan,
      YoungGen::EvacAcceptor &acceptor);

  /// Called after the GC's heap has been copied to a new location, in order to
  /// update the references in this space (and the space's own limits) to the
  /// new location.
  void moveHeap(GenGC *gc, ptrdiff_t moveHeapDelta);

  /// If youngIsEmpty is true, assume that full GC has completely emptied the
  /// young generation, and clear the card table.  Otherwise, the young
  /// generation is not empty, so we modify the card table to be
  /// (conservatively) valid after old generation compaction: dirty all cards
  /// containing objects after compaction (up to the new level of the
  /// generation), clean all now-unoccupied cards.  This keeps us from having to
  /// track what cards contain old-to-young pointers after compaction -- we
  /// assume any card might.
  void updateCardTablesAfterCompaction(bool youngIsEmpty);

  /// If the owning GC has been allocating directly in the OG, it will
  /// not have maintained the card object boundaries.  This call recreates
  /// the boundaries, making it valid for future YG collections.
  void recreateCardTableBoundaries();

#ifdef HERMES_SLOW_DEBUG
  /// Verify the accuracy of the object boundaries in each segment's card table:
  /// For each card, find the object crossing its lower limit and verify that it
  /// is valid.
  void verifyCardTableBoundaries() const;
#endif

  /// See GCGeneration.h for more information.
  void sweepAndInstallForwardingPointers(GenGC *gc, SweepResult *sweepResult);

  /// See GCGeneration.h for more information.
  void updateReferences(
      GenGC *gc,
      SweepResult::KindAndSizesRemaining &kindAndSizes);

  /// See GCGeneration.h for more information.
  void recordLevelAfterCompaction(
      CompactionResult::ChunksRemaining &usedChunks);

#ifdef HERMES_SLOW_DEBUG
  void checkWellFormed(const GenGC *gc) const;
#endif

#ifdef HERMES_EXTRA_DEBUG
  /// These methods protect and unprotect, respectively, the memory
  /// that comprises the card boundary tables of all the segments in
  /// the old generation.  They require that the starts of the
  /// boundary tables are page-aligned, and the table size is a
  /// multiple of the page size.
  /// TODO(T48709128): remove this when the problem is diagnosed.
  void protectCardTableBoundaries();
  void unprotectCardTableBoundaries();

  /// These methods protect and unprotect, respectively, the memory
  /// that comprises the card boundary table of the active segment of
  /// the old generation.  They require that the start of the boundary
  /// tables are page-aligned, and the table size is a multiple of the
  /// page size.
  /// TODO(T48709128): remove this when the problem is diagnosed.
  void protectActiveSegCardTableBoundaries();
  void unprotectActiveSegCardTableBoundaries();
#endif

  /// Static override of GCGeneration::didFinishGC().
  /// Records the level at the end of a GC.  At the start of next
  /// young-gen GC, objects at addresses at or greater than this were
  /// allocated directly in the OldGen.
  void didFinishGC();

  /// Update the extents of the old-gen segments with \p crashMgr.  Labels
  /// the crash manager key with the given \p runtimeName.
  void updateCrashManagerHeapExtents(
      const std::string &runtimeName,
      CrashManager *crashMgr);

 private:
  friend class OldGenFilledSegmentRange;
  friend class OldGenMaterializingRange;

  AllocResult fullCollectThenAlloc(
      uint32_t allocSize,
      HasFinalizer hasFinalizer);

  /// The number of bytes wasted due to space at the end of segments we could
  /// not allocate in.
  inline size_t fragmentationLoss() const;

  /// The maximum number of segments that can be allocated.
  size_t maxSegments() const;

  /// The number of segments needed to allocate the given size.  (Rounds up,
  /// conservatively, if necessary.)
  inline static size_t segmentsForSize(size_t size);

  /// The sequence of segments starting from the segment that contained the
  /// level at the end of the last GC, up to and including the segment that will
  /// be allocated into next.
  inline GCSegmentRange::Ptr segmentsSinceLastGC() const;

  /// Allocate and store enough segments into the cache to allocate up to \p
  /// size bytes in this heap (including existing allocations).
  ///
  /// \return true if all the requisite segments were allocated, and false
  ///     otherwise.
  bool seedSegmentCacheForSize(size_t size);

  /// Mark the current active segment as filled and find a fresh segment to be
  /// made active. \return true if a new segment was successfully materialized,
  /// and false otherwise.
  bool materializeNextSegment();

  /// Indicate that the materialised segments at index \p from onwards, in the
  /// order they were allocated into, are no longer in use.
  ///
  /// \pre from > 0, because it is not possible to release every segment.
  ///
  /// \post The segments at indices \p from and above have been removed from
  ///     the global segment index.
  ///
  /// \post A subsequent call to \c forUsedSegments will not iterate over the
  ///     segments previously at index \p from onwards.
  void releaseSegments(size_t from);

  /// Synchronise \c cardBoundary_ with the boundary that follows the active
  /// segment's current level.  This does not need to be called every time the
  /// segment's level changes due to allocations, but must do so before the next
  /// allocation if the level changes for some other reason (e.g. new active
  /// segment, collection, etc).
  void updateCardTableBoundary();

  /// Slow path for allocation: allocation in the current allocation
  /// segment failed.  Attempt in the next segment, if one exists,
  /// returning a failure if no it does not.
  AllocResult allocRawSlow(uint32_t size, HasFinalizer hasFinalizer);

  /// See GCGeneration.h for more information.
  size_t effectiveSize();

  /// Returns the Location where the heap effectively ends, if that location
  /// points to a used segment, or a null sentinel Location otherwise.
  Location effectiveEnd();

  /// The external memory that is considered conceptually allocated at the end
  /// of the generation.  This is the total external memory less the
  /// fragmentation loss, but with a min of zero.
  size_t trailingExternalMemory() const;

  /// The name given to the memory mapping for segments owned by this
  /// generation.
  static const char *kSegmentName;

  /// The next boundary between cards that an allocation in the Old Gen is going
  /// to cross.  We keep track of this so that we can note the address of the
  /// object that crosses the card boundary.
  CardTable::Boundary cardBoundary_;

  /// The amount of memory currently available in the generation, including
  /// external allocation; exhausting this triggers a collection.  All segments
  /// before the last contribute their maximum size; the last (most recently
  /// allocated) segment may have a size smaller than the maximum.
  size_t size_{0};

  /// The minimum and maximum sizes of the current generation.
  const Size sz_;

  /// The segments in the old generation that are filled with allocations, but
  /// are not currently being allocated into.
  std::deque<GenGCHeapSegment> filledSegments_;

  /// Segments in the old generation that are not currently in use, but we have
  /// retained to serve requests to materialize segments in the future.
  std::vector<GenGCHeapSegment> segmentCache_;

  /// We allocate in segments in "logical" order, and compact to low "logical"
  /// addresses.  This member holds the sum of the used portions of all but the
  /// last "used segment".
  size_t usedInFilledSegments_{0};

  /// The "level" (i.e., allocation point) at the end of the most
  /// recent (young-gen or full) GC.  The current level may be
  /// different because of direct allocation in the old generation.
  /// (Note that the Location type is a pair of a segment index and a
  /// pointer within the segment.)  The initial value of this is the
  /// start of the first segment.
  Location levelAtEndOfLastGC_;

  /// Whether to return unused memory to OS.
  bool releaseUnused_;

  /// The number of old-gen segments recorded with the crash manager.
  unsigned crashMgrRecordedSegments_{0};

#ifdef HERMES_EXTRA_DEBUG
  /// The set of addresses of card tables whose boundary tables have been
  /// protected.
  /// TODO(T48709128): remove this when the problem is diagnosed.
  llvh::DenseSet<void *> protectedCardTables_;
#endif

#ifdef HERMES_EXTRA_DEBUG
  /// The number of vtable summary errors detected so far.
  /// TODO(T56364255): remove these when the problem is diagnosed.
  unsigned numVTableSummaryErrors_{0};
#endif
};

size_t OldGen::Size::maxSegments() const {
  return segmentsForSize(max());
}

size_t OldGen::Size::minSegments() const {
  return segmentsForSize(min());
}

/*static*/ size_t OldGen::Size::segmentsForSize(size_t size) {
  return size > 0 ? (size - 1) / GenGCHeapSegment::maxSize() + 1 : 1;
}

AllocResult OldGen::alloc(uint32_t size, HasFinalizer hasFinalizer) {
  assert(ownsAllocContext());
  AllocResult result = allocRaw(size, hasFinalizer);
  if (LLVM_LIKELY(result.success)) {
    return result;
  }
  return fullCollectThenAlloc(size, hasFinalizer);
}

AllocResult OldGen::allocRaw(uint32_t size, HasFinalizer hasFinalizer) {
  assert(ownsAllocContext());
  AllocResult result = GCGeneration::allocRaw(size, hasFinalizer);

  if (LLVM_UNLIKELY(!result.success)) {
    return allocRawSlow(size, hasFinalizer);
  }

  char *resPtr = reinterpret_cast<char *>(result.ptr);
  char *nextAllocPtr = activeSegment().level();
  if (cardBoundary_.address() < nextAllocPtr) {
    updateBoundariesAfterAlloc(resPtr, nextAllocPtr);
  }

  return {resPtr, true};
}

size_t OldGen::used() const {
  return usedInFilledSegments_ + activeSegment().used() + externalMemory();
}

size_t OldGen::size() const {
  return size_;
}

size_t OldGen::minSize() const {
  return sz_.min();
}

size_t OldGen::maxSize() const {
  return sz_.max();
}

size_t OldGen::adjustSize(size_t desired) const {
  // The desired size must be at least levelOffset(), to maintain the invariant
  // levelOffset() <= size().
  desired = std::max(desired, levelOffset());
  return sz_.adjustSize(desired);
}

GCSegmentRange::Ptr OldGen::allSegments() {
  assert(ownsAllocContext());
  return GCSegmentRange::concat(
      OldGenFilledSegmentRange::create(this),
      GCSegmentRange::singleton(&activeSegment()),
      GCSegmentRange::fuse(OldGenMaterializingRange::create(this)));
}

template <typename F>
inline void OldGen::forUsedSegments(F callback) {
  assert(ownsAllocContext());

  for (auto &filled : filledSegments_) {
    callback(filled);
  }

  callback(activeSegment());
}

template <typename F>
inline void OldGen::forUsedSegments(F callback) const {
  assert(ownsAllocContext());

  for (const auto &filled : filledSegments_) {
    callback(filled);
  }

  callback(activeSegment());
}

template <typename F>
inline bool OldGen::whileUsedSegments(F callback) {
  assert(ownsAllocContext());

  for (auto &filled : filledSegments_) {
    if (LLVM_UNLIKELY(!callback(filled)))
      return false;
  }

  if (LLVM_UNLIKELY(!callback(activeSegment())))
    return false;

  return true;
}

template <typename F>
inline bool OldGen::whileUsedSegments(F callback) const {
  assert(ownsAllocContext());

  for (const auto &filled : filledSegments_) {
    if (!LLVM_UNLIKELY(callback(filled)))
      return false;
  }

  if (!LLVM_UNLIKELY(callback(activeSegment())))
    return false;

  return true;
}

OldGen::Location OldGen::level() const {
  return {filledSegments_.size(), trueActiveSegment().level()};
}
OldGen::Location OldGen::levelDirect() const {
  return {filledSegments_.size(), activeSegment().level()};
}

size_t OldGen::levelOffset() const {
  return filledSegments_.size() * GenGCHeapSegment::maxSize() +
      activeSegment().used();
}

size_t OldGen::fragmentationLoss() const {
  // Take the max size of all the filled segments, subtract the used to get
  // the fragmentation loss.
  size_t filledSegMaxSize =
      filledSegments_.size() * GenGCHeapSegment::maxSize();
  assert(filledSegMaxSize >= usedInFilledSegments_);
  return filledSegMaxSize - usedInFilledSegments_;
}

GCSegmentRange::Ptr OldGen::segmentsSinceLastGC() const {
  assert(ownsAllocContext());
  // This API is only used in scenarios where the segments will not be mutated,
  // but the range API is not const-polymorphic, so we have to const_cast the
  // \c this.
  auto _this = const_cast<OldGen *>(this);
  return GCSegmentRange::concat(
      OldGenFilledSegmentRange::create(_this, levelAtEndOfLastGC_.segmentNum),
      GCSegmentRange::singleton(&_this->activeSegment()));
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_OLDGEN_H
