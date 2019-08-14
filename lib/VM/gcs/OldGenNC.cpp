/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#define DEBUG_TYPE "gc"
#include "hermes/VM/OldGenNC.h"

#include "hermes/Support/OSCompat.h"
#include "hermes/VM/AdviseUnused.h"
#include "hermes/VM/AllocResult.h"
#include "hermes/VM/CompactionResult-inline.h"
#include "hermes/VM/CompleteMarkState-inline.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/GCSegmentRange-inline.h"
#include "hermes/VM/GCSegmentRange.h"
#include "hermes/VM/YoungGenNC-inline.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"

#include <algorithm>
#include <iterator>
#include <vector>

namespace hermes {
namespace vm {

#ifdef HERMES_SLOW_DEBUG
/// These constants allow us to selective turn off some of the
/// slow debugging checks (by modifying this code and recompiling).
/// But they're on by default.
static bool kVerifyCardTableBoundaries = true;
static bool kVerifyCardTable = true;
#endif

/* static */ const char *OldGen::kSegmentName = "hermes-oldgen-segment";

OldGen::Size::Size(gcheapsize_t min, gcheapsize_t max)
    : // The minimum old generation size is 2 pages.
      // Round up the minSize as needed.
      min_(adjustSizeWithBounds(
          min,
          2 * oscompat::page_size(),
          std::numeric_limits<gcheapsize_t>::max())),
      // Round up the maxSize as needed.
      max_(adjustSizeWithBounds(
          max,
          2 * oscompat::page_size(),
          std::numeric_limits<gcheapsize_t>::max())) {}

gcheapsize_t OldGen::Size::storageFootprint() const {
  return maxSegments() * AlignedStorage::size();
}

gcheapsize_t OldGen::Size::minStorageFootprint() const {
  return minSegments() * AlignedStorage::size();
}

/* static */
gcheapsize_t
OldGen::Size::adjustSizeWithBounds(size_t desired, size_t min, size_t max) {
  const size_t PS = hermes::oscompat::page_size();

  // The old generation's size must be at least two pages wide.
  assert(min >= 2 * PS);
  // The max must be at least the min size.
  assert(max >= min);

  // The old generation's size must be
  //  - page-aligned, if it fits within one segment.
  //  - segment-aligned, otherwise.
  //  - at most \c max bytes wide up to alignment.
  const auto clamped = std::max(min, std::min(desired, max));
  const auto alignment = clamped <= AlignedHeapSegment::maxSize()
      ? PS
      : AlignedHeapSegment::maxSize();
  return llvm::alignTo(clamped, alignment);
}

OldGen::OldGen(GenGC *gc, Size sz, bool releaseUnused)
    : GCGeneration(gc), sz_(sz), releaseUnused_(releaseUnused) {
  auto result = AlignedStorage::create(&gc_->storageProvider_, kSegmentName);
  if (!result) {
    gc_->oom(result.getError());
  }
  exchangeActiveSegment({std::move(result.get()), this});
  // Record the initial level, as if we had done a GC before starting.
  didFinishGC();
  updateCardTableBoundary();
}

size_t OldGen::available() const {
  assert(size() >= levelOffset());

  size_t avail = size() - levelOffset();
  size_t trail = trailingExternalMemory();

  // Now delete the external memory.
  return trail <= avail ? avail - trail : 0;
}

void OldGen::growTo(size_t desired) {
  assert(desired == adjustSize(desired) && "Size must be adjusted.");

  if (size() >= desired) {
    return;
  }

  // We only need to grow from a non-segment-aligned size if the current size is
  // less than a segment's worth.
  if (size() < AlignedHeapSegment::maxSize()) {
    assert(filledSegments_.empty());
    activeSegment().growTo(std::min(desired, AlignedHeapSegment::maxSize()));
  }

  size_ = desired;

  // Now update the effective end to the new correct value.
  updateEffectiveEndForExternalMemory();
}

void OldGen::shrinkTo(size_t desired) {
  assert(desired >= used());
  // Note that this assertion implies that desired >= sz_.min().
  assert(desired == adjustSize(desired) && "Size must be adjusted.");

  if (size() <= desired) {
    return;
  }

  // We only shrink to a non-segment-aligned size if the desired size is less
  // than a segment's worth.
  if (desired < AlignedHeapSegment::maxSize()) {
    // We should be justified in asserting that there are no filled segments
    // because we checked earlier that we are using less than the desired size
    // so by transitivity, we are using less than a segment's worth of space.
    assert(filledSegments_.empty());
    activeSegment().shrinkTo(desired);
  }

  size_ = desired;

  // Now update the effective end to the new correct value.
  updateEffectiveEndForExternalMemory();
}

bool OldGen::growToFit(size_t amount) {
  size_t unavailable = levelOffset() + trailingExternalMemory();
  size_t adjusted = adjustSize(unavailable + amount);

  // Insufficient space?
  if (adjusted < unavailable + amount) {
    return false;
  }

  // Could not allocate segments to back growth.
  if (!seedSegmentCacheForSize(levelOffset() + amount)) {
    return false;
  }

  growTo(adjusted);
  return true;
}

gcheapsize_t OldGen::bytesAllocatedSinceLastGC() const {
  auto segs = segmentsSinceLastGC();
  AlignedHeapSegment *seg = segs->next();

  assert(seg && "Must be at least one segment");
  assert(seg->dbgContainsLevel(levelAtEndOfLastGC_.ptr));

  // First do the diff for the alloc segment at the time of last GC.
  gcheapsize_t res = seg->level() - levelAtEndOfLastGC_.ptr;

  // Now add any later segments into which allocation has occurred.
  while ((seg = segs->next())) {
    res += seg->used();
  }

  return res;
}

void OldGen::forAllObjs(const std::function<void(GCCell *)> &callback) {
  forUsedSegments([&callback](AlignedHeapSegment &segment) {
    segment.forAllObjs(callback);
  });
}

#ifndef NDEBUG
bool OldGen::dbgContains(const void *p) const {
  return gc_->dbgContains(p) && !gc_->youngGen_.dbgContains(p);
}

void OldGen::forObjsAllocatedSinceGC(
    const std::function<void(GCCell *)> &callback) {
  auto segs = segmentsSinceLastGC();
  AlignedHeapSegment *seg = segs->next();

  assert(seg && "Must be at least one segment");
  assert(seg->dbgContainsLevel(levelAtEndOfLastGC_.ptr));

  // First do the remainder, if any, of the region containing the first such
  // object.
  seg->forObjsInRange(callback, levelAtEndOfLastGC_.ptr, seg->level());

  // Now do any subsequent segments.
  while ((seg = segs->next())) {
    seg->forAllObjs(callback);
  }
}
#endif // !NDEBUG

void OldGen::creditExternalMemory(uint32_t size) {
  GCGeneration::creditExternalMemory(size);
  updateEffectiveEndForExternalMemory();
}

void OldGen::debitExternalMemory(uint32_t size) {
  GCGeneration::debitExternalMemory(size);
  updateEffectiveEndForExternalMemory();
}

bool OldGen::ensureFits(size_t amount) {
  if (amount > available()) {
    return false;
  }

  return seedSegmentCacheForSize(levelOffset() + amount);
}

size_t OldGen::effectiveSize() {
  size_t trailingExternalMem = trailingExternalMemory();
  if (trailingExternalMem > size()) {
    return 0;
  } else {
    return size() - trailingExternalMem;
  }
}

OldGen::Location OldGen::effectiveEnd() {
  const size_t offset = effectiveSize();
  const size_t segNum = offset / AlignedHeapSegment::maxSize();
  const size_t segOff = offset % AlignedHeapSegment::maxSize();

  // In the common case, the effective end will be nowhere near the used portion
  // of the heap.
  if (LLVM_LIKELY(segNum > filledSegments_.size())) {
    return Location();
  }

  // In the not so common case, we hope that even if the effective end is close
  // to the level, it is still above it.
  if (LLVM_LIKELY(segNum == filledSegments_.size())) {
    return Location(segNum, trueActiveSegment().start() + segOff);
  }

  // Pathologically bad case -- the effective end lies before the level and the
  // heap is overcommitted.
  return Location(segNum, filledSegments_[segNum].start() + segOff);
}

size_t OldGen::trailingExternalMemory() const {
  // We consider the externalMemory_ to "fill up" fragmentation losses in
  // filled segments -- thus, we subtract such those losses from the external
  // memory for purposes of computing the effective size.  Only the remainder
  // is considered to be "allocated at the end".
  size_t fragLoss = fragmentationLoss();
  if (fragLoss > externalMemory_) {
    return 0;
  } else {
    return externalMemory_ - fragLoss;
  }
}

void OldGen::updateEffectiveEndForExternalMemory() {
  Location desiredEnd = effectiveEnd();

  if (LLVM_LIKELY(!desiredEnd)) {
    // The effective size is not in a used segment, so in particular, it cannot
    // be in the active segment.
    trueActiveSegment().clearExternalMemoryCharge();
    return;
  }

  auto clampedEnd = std::max(level(), desiredEnd);
  assert(
      clampedEnd.segmentNum == filledSegments_.size() &&
      "Effective end should be in the active segment.");

  trueActiveSegment().setEffectiveEnd(clampedEnd.ptr);
}

void OldGen::markYoungGenPointers(OldGen::Location originalLevel) {
  if (used() == 0) {
    // Nothing to do if the old gen is empty.
    return;
  }

#ifdef HERMES_SLOW_DEBUG
  struct VerifyCardDirtyAcceptor final : public SlotAcceptorDefault {
    using SlotAcceptorDefault::accept;
    using SlotAcceptorDefault::SlotAcceptorDefault;

    void accept(void *&ptr) override {
      char *valuePtr = reinterpret_cast<char *>(ptr);
      char *locPtr = reinterpret_cast<char *>(&ptr);

      if (gc.youngGen_.contains(valuePtr)) {
        assert(AlignedHeapSegment::cardTableCovering(locPtr)
                   ->isCardForAddressDirty(locPtr));
      }
    }
    void accept(BasedPointer &ptr) override {
      // Don't use the default from SlotAcceptorDefault since the address of the
      // reference is used.
      PointerBase *const base = gc.getPointerBase();
      char *valuePtr = reinterpret_cast<char *>(base->basedToPointer(ptr));
      char *locPtr = reinterpret_cast<char *>(&ptr);

      if (gc.youngGen_.contains(valuePtr)) {
        assert(AlignedHeapSegment::cardTableCovering(locPtr)
                   ->isCardForAddressDirty(locPtr));
      }
    }
    void accept(HermesValue &hv) override {
      if (!hv.isPointer()) {
        return;
      }

      char *valuePtr = reinterpret_cast<char *>(hv.getPointer());
      char *locPtr = reinterpret_cast<char *>(&hv);

      if (gc.youngGen_.contains(valuePtr)) {
        assert(AlignedHeapSegment::cardTableCovering(locPtr)
                   ->isCardForAddressDirty(locPtr));
      }
    }
  };

  if (kVerifyCardTable) {
    VerifyCardDirtyAcceptor acceptor(*gc_);
    GenGC *gc = gc_;
    forAllObjs([gc, &acceptor](GCCell *cell) {
      GCBase::markCell(cell, gc, acceptor);
    });
  }

  verifyCardTableBoundaries();
#endif // HERMES_SLOW_DEBUG

  struct OldGenObjEvacAcceptor final : public SlotAcceptorDefault {
    using SlotAcceptorDefault::accept;
    using SlotAcceptorDefault::SlotAcceptorDefault;

    // NOTE: C++ does not allow templates on local classes, so duplicate the
    // body of \c helper for ensureReferentCopied.
    void helper(GCCell **slotAddr, void *slotContents) {
      if (gc.youngGen_.contains(slotContents)) {
        gc.youngGen_.ensureReferentCopied(slotAddr);
      }
    }
    void helper(HermesValue *slotAddr, void *slotContents) {
      if (gc.youngGen_.contains(slotContents)) {
        gc.youngGen_.ensureReferentCopied(slotAddr);
      }
    }

    void accept(void *&ptr) {
      helper(reinterpret_cast<GCCell **>(&ptr), ptr);
    }
    void accept(HermesValue &hv) {
      if (hv.isPointer()) {
        helper(&hv, hv.getPointer());
      }
    }
  };

  OldGenObjEvacAcceptor acceptor(*gc_);
  SlotVisitor<OldGenObjEvacAcceptor> visitor(acceptor);

  auto segs = GCSegmentRange::concat(
      OldGenFilledSegmentRange::create(this),
      GCSegmentRange::singleton(&activeSegment()));

  size_t i = 0;
  while (AlignedHeapSegment *seg = segs->next()) {
    if (originalLevel.segmentNum < i)
      break;

    const char *const origSegLevel =
        i == originalLevel.segmentNum ? originalLevel.ptr : seg->level();

    auto &cardTable = seg->cardTable();

    size_t from = cardTable.addressToIndex(seg->start());
    size_t to = cardTable.addressToIndex(origSegLevel - 1) + 1;

    while (const auto oiBegin = cardTable.findNextDirtyCard(from, to)) {
      const auto iBegin = *oiBegin;

      const auto oiEnd = cardTable.findNextCleanCard(iBegin, to);
      const auto iEnd = oiEnd ? *oiEnd : to;

      assert(
          (iEnd == to || !cardTable.isCardForIndexDirty(iEnd)) &&
          cardTable.isCardForIndexDirty(iEnd - 1) &&
          "end should either be the end of the card table, or the first "
          "non-dirty card after a sequence of dirty cards");
      assert(iBegin < iEnd && "Indices must be apart by at least one");

      const char *const begin = cardTable.indexToAddress(iBegin);
      const char *const end = cardTable.indexToAddress(iEnd);
      const void *const boundary = std::min(end, origSegLevel);

      GCCell *const firstObj = cardTable.firstObjForCard(iBegin);
      GCCell *obj = firstObj;

#ifdef HERMES_EXTRA_DEBUG
      // We seem to have a bug where firstObjForCard yields an invalid object.
      // Gather extra debug information in that case.
      // Note: the predicate carefully checks first whether the segment contains
      // the firstObj; if not, we probably don't want to dereference it, which
      // the VTable validity check does.
      // TODO(T48709128): remove this when the problem is diagnosed.
      if (LLVM_UNLIKELY(!seg->contains(firstObj) || !firstObj->isValid())) {
        static unsigned numInvalid = 0;
        char detailBuffer[200];
        snprintf(
            detailBuffer,
            sizeof(detailBuffer),
            "CardObjectTable leads to bad first object: seg = [%p, %p), "
            "CT index = %zu, CT value = %d, firstObj = %p.  Num fails = %d.",
            seg->lowLim(),
            seg->hiLim(),
            iBegin,
            cardTable.cardObjectTableValue(iBegin),
            firstObj,
            ++numInvalid);
        hermesLog("HermesGC", "OOM: %s.", detailBuffer);
        // Record the OOM custom data with the crash manager.
        gc_->crashMgr_->setCustomData("HermesGCBadCOTCalc", detailBuffer);
      }
#endif

      // Mark the first object with respect to the dirty card boundaries.
      GCBase::markCellWithinRange(visitor, obj, obj->getVT(), gc_, begin, end);

      // Mark the objects that are entirely contained within the dirty card
      // boundaries.
      for (GCCell *next = obj->nextCell(); next < boundary;
           next = next->nextCell()) {
        obj = next;
        GCBase::markCell(visitor, obj, obj->getVT(), gc_);
      }

      // Mark the final object in the range with respect to the dirty card
      // boundaries, as long as it does not coincide with the first object.
      if (LLVM_LIKELY(obj != firstObj)) {
        GCBase::markCellWithinRange(
            visitor, obj, obj->getVT(), gc_, begin, end);
      }

      from = iEnd;
    }
    cardTable.clear();
    i++;
  }
}

void OldGen::youngGenTransitiveClosure(
    const Location &toScanLoc,
    YoungGen::EvacAcceptor &acceptor) {
  size_t toScanSegmentNum = toScanLoc.segmentNum;
  char *toScanPtr = toScanLoc.ptr;

  // Predicate to check whether the the index \p ix refers to a segment that has
  // already been "filled", which implies that its level will not change.
  const auto isFilled = [this](size_t ix) {
    return ix < filledSegments_.size();
  };

  // We must scan until the to-scan segment number and pointer reach the current
  // allocation point.  This loop nest is maximally specialized for
  // performance, see T26274987 for details.
  while (isFilled(toScanSegmentNum) || toScanPtr < activeSegment().level()) {
    // Now we have two interior loops: we can be faster for already
    // filled segments, since their levels won't change.
    while (isFilled(toScanSegmentNum)) {
      AlignedHeapSegment &curSegment = filledSegments_[toScanSegmentNum];
      const char *const curSegmentLevel = curSegment.level();
      while (toScanPtr < curSegmentLevel) {
        GCCell *cell = reinterpret_cast<GCCell *>(toScanPtr);
        toScanPtr += cell->getAllocatedSize();
        // Ask the object to mark the pointers it owns.
        GCBase::markCell(cell, gc_, acceptor);
      }
      toScanSegmentNum++;
      toScanPtr = isFilled(toScanSegmentNum)
          ? filledSegments_[toScanSegmentNum].start()
          : activeSegment().start();
    }

    // We should have reached the current allocation segment.
    assert(toScanSegmentNum == filledSegments_.size());
    assert(activeSegment().dbgContainsLevel(toScanPtr));
    const char *const activeLowLim = activeSegment().lowLim();

    // Now the level of the region can change as we call markCell, so
    // we must query each time.
    while (activeLowLim == activeSegment().lowLim() &&
           toScanPtr < activeSegment().level()) {
      const char *const activeLevel = activeSegment().level();
      while (toScanPtr < activeLevel) {
        GCCell *cell = reinterpret_cast<GCCell *>(toScanPtr);
        toScanPtr += cell->getAllocatedSize();
        // Ask the object to mark the pointers it owns.
        GCBase::markCell(cell, gc_, acceptor);
      }
    }

    // We're not necessarily done; allocation may have moved to a
    // later segment.  So we go around the loop again to make sure we
    // reach the proper termination condition.
  }
}

#ifdef HERMES_SLOW_DEBUG
void OldGen::verifyCardTableBoundaries() const {
  if (kVerifyCardTableBoundaries) {
    forUsedSegments([](const AlignedHeapSegment &segment) {
      segment.cardTable().verifyBoundaries(segment.start(), segment.level());
    });
  }
}
#endif

void OldGen::sweepAndInstallForwardingPointers(
    GC *gc,
    SweepResult *sweepResult) {
  forUsedSegments([gc, sweepResult](AlignedHeapSegment &segment) {
    segment.sweepAndInstallForwardingPointers(gc, sweepResult);
  });
}

void OldGen::updateReferences(GC *gc, SweepResult::VTablesRemaining &vTables) {
  auto acceptor = getFullMSCUpdateAcceptor(*gc);
  forUsedSegments([&acceptor, gc, &vTables](AlignedHeapSegment &segment) {
    segment.updateReferences(gc, acceptor.get(), vTables);
  });
  updateFinalizableCellListReferences();
}

void OldGen::recordLevelAfterCompaction(
    CompactionResult::ChunksRemaining &chunks) {
  const size_t nChunks = chunks.size();

  usedInFilledSegments_ = 0;
  size_t usedInPrev = 0;
  size_t usedSegs = 0;

  // Some prefix of the used chunks correspond to segments in this generation.
  // The corresponding segments are the used segments after compaction.
  whileUsedSegments([&](AlignedHeapSegment &segment) {
    if (usedSegs >= nChunks || this != chunks.peek().generation()) {
      return false;
    }

    if (releaseUnused_)
      chunks.next().recordLevel<AdviseUnused::Yes>(&segment);
    else
      chunks.next().recordLevel<AdviseUnused::No>(&segment);

    usedInFilledSegments_ += usedInPrev;
    usedInPrev = segment.used();
    usedSegs++;

    return true;
  });

  releaseSegments(usedSegs == 0 ? 1 : usedSegs);
  updateCardTableBoundary();
}

AllocResult OldGen::fullCollectThenAlloc(
    uint32_t allocSize,
    HasFinalizer hasFinalizer) {
  gc_->collect(/* canEffectiveOOM */ true);
  {
    AllocResult res = allocRaw(allocSize, hasFinalizer);
    if (LLVM_LIKELY(res.success)) {
      return res;
    }
  }

  if (growToFit(allocSize)) {
    AllocResult res = allocRaw(allocSize, hasFinalizer);
    assert(res.success && "preceding test should guarantee success.");
    return res;
  }

  gc_->oom(make_error_code(OOMError::MaxHeapReached));
}

void OldGen::moveHeap(GC *gc, ptrdiff_t moveHeapDelta) {
  // TODO (T25686322): implement non-contig version of this.
}

void OldGen::updateCardTablesAfterCompaction(bool youngIsEmpty) {
  forUsedSegments([youngIsEmpty](AlignedHeapSegment &segment) {
    if (youngIsEmpty) {
      segment.cardTable().clear();
    } else {
      segment.cardTable().updateAfterCompaction(segment.level());
    }
  });

#ifdef HERMES_SLOW_DEBUG
  verifyCardTableBoundaries();
#endif
}

void OldGen::recreateCardTableBoundaries() {
  forUsedSegments([](AlignedHeapSegment &segment) {
    segment.recreateCardTableBoundaries();
  });

  updateCardTableBoundary();
#ifdef HERMES_SLOW_DEBUG
  verifyCardTableBoundaries();
#endif
}

bool OldGen::seedSegmentCacheForSize(size_t size) {
  auto committedSegs = [this]() {
    return filledSegments_.size() + segmentCache_.size() + 1;
  };

  const auto segReq = Size::segmentsForSize(size);
  auto segAlloc = committedSegs();

  // Remember how many segments we had cached in case we need to rollback.
  const auto cacheBefore = segmentCache_.size();

  // Try and seed the segment cache with enough segments to fit the request.
  for (; segAlloc < segReq; ++segAlloc) {
    auto result = AlignedStorage::create(&gc_->storageProvider_, kSegmentName);
    if (!result) {
      // We could not allocate all the segments we needed, so give back the ones
      // we were able to allocate.
      segmentCache_.resize(cacheBefore);
      return false;
    }
    segmentCache_.emplace_back(std::move(result.get()), this);
    segmentCache_.back().growToLimit();
  }

  assert(committedSegs() >= segReq);
  return true;
}

bool OldGen::materializeNextSegment() {
  auto usedSegs = filledSegments_.size() + 1;
  auto maxSizeSegs = Size::segmentsForSize(effectiveSize());

  if (usedSegs >= maxSizeSegs) {
    return false;
  }

  // Save the number of used bytes in the currently active segment and TLAB
  const size_t usedInFilledSeg = activeSegment().used();

  // Reserve a slot for the previously active segment
  filledSegments_.emplace_back();
  AlignedHeapSegment *filledSegSlot = &filledSegments_.back();

  // Get a new segment from somewhere
  if (!segmentCache_.empty()) {
    exchangeActiveSegment(std::move(segmentCache_.back()), filledSegSlot);
    segmentCache_.pop_back();
  } else {
    auto result = AlignedStorage::create(&gc_->storageProvider_, kSegmentName);
    if (!result) {
      filledSegments_.pop_back();
      return false;
    }
    exchangeActiveSegment({std::move(result.get()), this}, filledSegSlot);
    activeSegment().growToLimit();
  }

  // The active segment has changed, so we need to update the next card table
  // boundary to align with the start of its allocation region.
  updateCardTableBoundary();
  usedInFilledSegments_ += usedInFilledSeg;
  filledSegSlot->clearExternalMemoryCharge();

  // We may have moved into the segment containing the effective end. Also,
  // declaring the segment full may leave a portion unallocated; we consider
  // external memory as if it fills in such fragmentation losses.  So update
  // the effective end of the generation.
  updateEffectiveEndForExternalMemory();

  return true;
}

void OldGen::releaseSegments(size_t from) {
  assert(from > 0 && "Cannot release every segment.");

  // TODO (T30523258) Experiment with different schemes for deciding
  // how many segments we keep in the cache, trading off the VA cost,
  // against the cost of allocating a fresh segment.
  if (releaseUnused_)
    segmentCache_.clear();

  const auto nSegs = filledSegments_.size() + 1;
  if (from >= nSegs) {
    // Nothing more to do
    return;
  }

  std::vector<const char *> toRelease;
  const auto release = [&toRelease, this](AlignedHeapSegment &unused) {
    toRelease.push_back(unused.lowLim());
    if (!this->releaseUnused_) {
      // Clear the segment and move it to the cache.
      unused.resetLevel();
      assert(unused.used() == 0);
      this->segmentCache_.emplace_back(std::move(unused));
    }
  };

  // Release the current active segment first
  release(activeSegment());

  auto first = filledSegments_.begin() + from;
  auto last = filledSegments_.end();

  std::for_each(first, last, release);

  std::sort(toRelease.begin(), toRelease.end());
  gc_->forgetSegments(toRelease);

  // The new active segment was the last unreleased one.
  exchangeActiveSegment(std::move(filledSegments_[from - 1]));
  filledSegments_.resize(from - 1);
}

AllocResult OldGen::allocSlow(uint32_t size, HasFinalizer hasFinalizer) {
  assert(ownsAllocContext());
  AllocResult result = allocRawSlow(size, hasFinalizer);
  if (LLVM_LIKELY(result.success)) {
    return result;
  }
  return fullCollectThenAlloc(size, hasFinalizer);
}

void OldGen::updateCardTableBoundary() {
  assert(ownsAllocContext());
  cardBoundary_ =
      activeSegment().cardTable().nextBoundary(activeSegment().level());
}

AllocResult OldGen::allocRawSlow(uint32_t size, HasFinalizer hasFinalizer) {
  assert(ownsAllocContext() && "Only called when the context is owned.");
  // The size being allocated must fit in a segment.
  if (LLVM_UNLIKELY(size > AlignedHeapSegment::maxSize())) {
    gc_->oom(make_error_code(OOMError::SuperSegmentAlloc));
  }

  // Allocation failed in the current segment; try the next one, if
  // possible.  Are there more segments to try?
  if (LLVM_UNLIKELY(!materializeNextSegment())) {
    return {nullptr, false};
  }

  // This looks like potentially unbounded recursion, but it is not.  We
  // asserted above that the the size being allocated fits in a fully empty
  // segment.  At this point we have successfully materialized a new empty, and
  // max sized segment, so the allocation is guaranteed to succeed.
  return allocRaw(size, hasFinalizer);
}

#ifdef HERMES_SLOW_DEBUG
void OldGen::checkWellFormed(const GC *gc) const {
  uint64_t totalExtSize = 0;

  forUsedSegments([&totalExtSize, gc](const AlignedHeapSegment &segment) {
    uint64_t extSize = 0;
    segment.checkWellFormed(gc, &extSize);
    totalExtSize += extSize;
  });

  assert(totalExtSize == externalMemory());
  checkFinalizableObjectsListWellFormed();
}
#endif

void OldGen::didFinishGC() {
  levelAtEndOfLastGC_ = levelDirect();
}

} // namespace vm
} // namespace hermes
