/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
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
#include "hermes/VM/PointerBase.h"
#include "hermes/VM/YoungGenNC-inline.h"

#include "llvh/Support/Debug.h"
#include "llvh/Support/MathExtras.h"

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
  const auto alignment =
      clamped <= GenGCHeapSegment::maxSize() ? PS : GenGCHeapSegment::maxSize();
  return llvh::alignTo(clamped, alignment);
}

OldGen::OldGen(GenGC *gc, Size sz, bool releaseUnused)
    : GCGeneration(gc), sz_(sz), releaseUnused_(releaseUnused) {
  auto result =
      AlignedStorage::create(gc_->storageProvider_.get(), kSegmentName);
  if (!result) {
    // Do not invoke oom() from here because the heap is not fully initialised.
    hermes_fatal("Failed to initialize the old gen", result.getError());
  }
  exchangeActiveSegment({std::move(result.get()), this});
#ifdef HERMESVM_COMPRESSED_POINTERS
  // The initial active segment gets the first OG index.
  gc->pointerBase_->setSegment(
      PointerBase::kFirstOGSegmentIndex, activeSegment().lowLim());
#endif
  // Record the initial level, as if we had done a GC before starting.
  didFinishGC();
  updateCardTableBoundary();
#ifdef HERMES_EXTRA_DEBUG
  // Protect the initial active segment.
  protectActiveSegCardTableBoundaries();
#endif
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
  assert(isSizeHeapAligned(desired) && "Size must be heap aligned.");

  if (size() >= desired) {
    return;
  }

  // We only need to grow from a non-segment-aligned size if the current size is
  // less than a segment's worth.
  if (size() < GenGCHeapSegment::maxSize()) {
    assert(filledSegments_.empty());
    activeSegment().growTo(std::min(desired, GenGCHeapSegment::maxSize()));
  }

  size_ = desired;

  // Now update the effective end to the new correct value.
  updateEffectiveEndForExternalMemory();
}

void OldGen::shrinkTo(size_t desired) {
  assert(desired >= used());
  // Note that this assertion implies that desired >= sz_.min().
  assert(desired == adjustSize(desired) && "Size must be adjusted.");
  assert(isSizeHeapAligned(desired) && "Size must be heap aligned.");

  if (size() <= desired) {
    return;
  }

  // We only shrink to a non-segment-aligned size if the desired size is less
  // than a segment's worth.
  if (desired < GenGCHeapSegment::maxSize()) {
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
  assert(isSizeHeapAligned(amount) && "Amount must be heap aligned.");
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
  GenGCHeapSegment *seg = segs->next();

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
  forUsedSegments(
      [&callback](GenGCHeapSegment &segment) { segment.forAllObjs(callback); });
}

#ifndef NDEBUG
bool OldGen::dbgContains(const void *p) const {
  return gc_->dbgContains(p) && !gc_->youngGen_.dbgContains(p);
}

void OldGen::forObjsAllocatedSinceGC(
    const std::function<void(GCCell *)> &callback) {
  auto segs = segmentsSinceLastGC();
  GenGCHeapSegment *seg = segs->next();

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
  const size_t segNum = offset / GenGCHeapSegment::maxSize();
  const size_t segOff = offset % GenGCHeapSegment::maxSize();

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
  struct VerifyCardDirtyAcceptor final : public SlotAcceptor {
    GenGC &gc;
    VerifyCardDirtyAcceptor(GenGC &gc) : gc(gc) {}

    void accept(void *valuePtr, void *locPtr) {
      if (gc.youngGen_.contains(valuePtr)) {
        assert(
            GenGCHeapSegment::cardTableCovering(locPtr)->isCardForAddressDirty(
                locPtr));
      }
    }

    void accept(GCPointerBase &ptr) override {
      accept(ptr.get(gc.getPointerBase()), &ptr);
    }

    void accept(GCHermesValue &hv) override {
      if (hv.isPointer())
        accept(hv.getPointer(), &hv);
    }

    void accept(GCSmallHermesValue &hv) override {
      if (hv.isPointer())
        accept(hv.getPointer(gc.getPointerBase()), &hv);
    }

    void accept(const GCSymbolID &hv) override {}
  };

  if (kVerifyCardTable) {
    VerifyCardDirtyAcceptor acceptor(*gc_);
    GenGC *gc = gc_;
    forAllObjs([gc, &acceptor](GCCell *cell) { gc->markCell(cell, acceptor); });
  }

  verifyCardTableBoundaries();
#endif // HERMES_SLOW_DEBUG

  struct OldGenObjEvacAcceptor final : public RootAndSlotAcceptorDefault {
    GenGC &gc;
    OldGenObjEvacAcceptor(GenGC &gc)
        : RootAndSlotAcceptorDefault(gc.getPointerBase()), gc(gc) {}

    using RootAndSlotAcceptorDefault::accept;

    void accept(BasedPointer &ptr) {
      gc.youngGen_.ensureReferentCopied(&ptr);
    }
    void accept(GCCell *&ptr) {
      gc.youngGen_.ensureReferentCopied(&ptr);
    }
    void acceptHV(HermesValue &hv) {
      if (hv.isPointer()) {
        gc.youngGen_.ensureReferentCopied(&hv);
      }
    }
    void acceptSHV(SmallHermesValue &hv) {
      if (hv.isPointer()) {
        gc.youngGen_.ensureReferentCopied(&hv);
      }
    }
  };

  OldGenObjEvacAcceptor acceptor(*gc_);
  SlotVisitor<OldGenObjEvacAcceptor> visitor(acceptor);

  auto segs = GCSegmentRange::concat(
      OldGenFilledSegmentRange::create(this),
      GCSegmentRange::singleton(&activeSegment()));

  size_t i = 0;
  while (GenGCHeapSegment *seg = segs->next()) {
    if (originalLevel.segmentNum < i)
      break;

    const char *const origSegLevel =
        i == originalLevel.segmentNum ? originalLevel.ptr : seg->level();

    auto &cardTable = seg->cardTable();
#ifdef HERMES_EXTRA_DEBUG
    // Capture the bounds of the segment at the start.  If seg points to the
    // active segment, and we allocate a new active segment *seg, can change, so
    // don't access after this.
    // TODO(T48709128): remove this when the problem is diagnosed.
    const char *segLo = seg->lowLim();
    const char *segHi = seg->hiLim();
#endif

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
      const char *firstObjCharStar = reinterpret_cast<const char *>(firstObj);
      if (LLVM_UNLIKELY(
              !(segLo <= firstObjCharStar && firstObjCharStar < segHi) ||
              !firstObj->isValid())) {
        static unsigned numInvalid = 0;
        char detailBuffer[200];
        snprintf(
            detailBuffer,
            sizeof(detailBuffer),
            "CardObjectTable leads to bad first object: seg = [%p, %p), "
            "CT index = %zu, CT value = %d, firstObj = %p.  Num fails = %d",
            segLo,
            segHi,
            iBegin,
            cardTable.cardObjectTableValue(iBegin),
            firstObj,
            ++numInvalid);
        hermesLog("HermesGC", "Error: %s.", detailBuffer);
        // Record the custom data with the crash manager.
        gc_->crashMgr_->setCustomData("HermesGCBadCOTCalc", detailBuffer);
      }
#endif

      // Mark the first object with respect to the dirty card boundaries.
      gc_->markCellWithinRange(visitor, obj, obj->getKind(), begin, end);

      obj = obj->nextCell();
      // If there are additional objects in this card, scan them.
      if (LLVM_LIKELY(obj < boundary)) {
        // Mark the objects that are entirely contained within the dirty card
        // boundaries. In a given iteration, obj is the start of a given object,
        // and next is the start of the next object. Iterate until the last
        // object where next is within the card.
        for (GCCell *next = obj->nextCell(); next < boundary;
             next = next->nextCell()) {
          gc_->markCell(visitor, obj, obj->getKind());
          obj = next;
        }

        // Mark the final object in the range with respect to the dirty card
        // boundaries.
        assert(
            obj < boundary && obj->nextCell() >= boundary &&
            "Last object in card must touch or cross cross the card boundary");
        gc_->markCellWithinRange(visitor, obj, obj->getKind(), begin, end);
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
      GenGCHeapSegment &curSegment = filledSegments_[toScanSegmentNum];
      const char *const curSegmentLevel = curSegment.level();
      while (toScanPtr < curSegmentLevel) {
        GCCell *cell = reinterpret_cast<GCCell *>(toScanPtr);
        toScanPtr += cell->getAllocatedSize();
        // Ask the object to mark the pointers it owns.
        gc_->markCell(cell, acceptor);
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
        gc_->markCell(cell, acceptor);
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
    forUsedSegments([](const GenGCHeapSegment &segment) {
      segment.cardTable().verifyBoundaries(segment.start(), segment.level());
    });
  }
}
#endif

void OldGen::sweepAndInstallForwardingPointers(
    GenGC *gc,
    SweepResult *sweepResult) {
  forUsedSegments([gc, sweepResult](GenGCHeapSegment &segment) {
    segment.sweepAndInstallForwardingPointers(gc, sweepResult);
  });
}

void OldGen::updateReferences(
    GenGC *gc,
    SweepResult::KindAndSizesRemaining &kindAndSizes) {
  auto acceptor = getFullMSCUpdateAcceptor(*gc);
  forUsedSegments([&acceptor, gc, &kindAndSizes](GenGCHeapSegment &segment) {
    segment.updateReferences(gc, acceptor.get(), kindAndSizes);
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
  whileUsedSegments([&](GenGCHeapSegment &segment) {
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
  gc_->collect(GCBase::kNaturalCauseForAnalytics, /* canEffectiveOOM */ true);
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

void OldGen::moveHeap(GenGC *gc, ptrdiff_t moveHeapDelta) {
  // TODO (T25686322): implement non-contig version of this.
}

void OldGen::updateCardTablesAfterCompaction(bool youngIsEmpty) {
  forUsedSegments([youngIsEmpty](GenGCHeapSegment &segment) {
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
  assert(!gc_->allocContext_.activeSegment);
#ifdef HERMES_EXTRA_DEBUG
  unprotectCardTableBoundaries();
#endif
  forUsedSegments(
      [](GenGCHeapSegment &segment) { segment.recreateCardTableBoundaries(); });

  updateCardTableBoundary();
#ifdef HERMES_SLOW_DEBUG
  verifyCardTableBoundaries();
#endif
#ifdef HERMES_EXTRA_DEBUG
  protectCardTableBoundaries();
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
    auto result =
        AlignedStorage::create(gc_->storageProvider_.get(), kSegmentName);
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
  GenGCHeapSegment *filledSegSlot = &filledSegments_.back();

  // Get a new segment from somewhere
  if (!segmentCache_.empty()) {
    exchangeActiveSegment(std::move(segmentCache_.back()), filledSegSlot);
    segmentCache_.pop_back();
  } else {
    auto result =
        AlignedStorage::create(gc_->storageProvider_.get(), kSegmentName);
    if (!result) {
      filledSegments_.pop_back();
      return false;
    }
    exchangeActiveSegment({std::move(result.get()), this}, filledSegSlot);
    activeSegment().growToLimit();
  }
#ifdef HERMESVM_COMPRESSED_POINTERS
  gc_->pointerBase_->setSegment(
      filledSegments_.size() + PointerBase::kFirstOGSegmentIndex,
      activeSegment().lowLim());
#endif

  // The active segment has changed, so we need to update the next card table
  // boundary to align with the start of its allocation region.
  updateCardTableBoundary();
#ifdef HERMES_EXTRA_DEBUG
  if (!gc_->inGC()) {
    protectActiveSegCardTableBoundaries();
  }
#endif
  usedInFilledSegments_ += usedInFilledSeg;
  filledSegSlot->clearExternalMemoryCharge();

  // We may have moved into the segment containing the effective end. Also,
  // declaring the segment full may leave a portion unallocated; we consider
  // external memory as if it fills in such fragmentation losses.  So update
  // the effective end of the generation.
  updateEffectiveEndForExternalMemory();

  /// Update the old-gen segment extents recorded in the crash manager.
  updateCrashManagerHeapExtents(gc_->name_, gc_->crashMgr_.get());

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
  const auto release = [&toRelease, this](GenGCHeapSegment &unused) {
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
  if (LLVM_UNLIKELY(size > GenGCHeapSegment::maxSize())) {
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

void OldGen::updateBoundariesAfterAlloc(char *alloc, char *nextAlloc) {
  // If we're allocating directly in the OG, we don't need to update the
  // boundaries.
  if (!gc_->allocContextFromYG_)
    return;

#ifdef HERMES_EXTRA_DEBUG
  // The allocation may update the boundary table of the old gen's
  // active segment, so unprotect it (and reprotect below).
  bool doUnprotect = !gc_->inGC();
  if (doUnprotect) {
    unprotectActiveSegCardTableBoundaries();
  }
#endif
  activeSegment().cardTable().updateBoundaries(
      &cardBoundary_, alloc, nextAlloc);
#ifdef HERMES_EXTRA_DEBUG
  if (doUnprotect) {
    protectActiveSegCardTableBoundaries();
  }
#endif
}

#ifdef HERMES_SLOW_DEBUG
void OldGen::checkWellFormed(const GenGC *gc) const {
  uint64_t totalExtSize = 0;

  forUsedSegments([&totalExtSize, gc](const GenGCHeapSegment &segment) {
    uint64_t extSize = 0;
    segment.checkWellFormed(gc, &extSize);
    totalExtSize += extSize;
  });

  assert(totalExtSize == externalMemory());
  checkFinalizableObjectsListWellFormed();
}
#endif

#ifdef HERMES_EXTRA_DEBUG
/// For all of these, we're commenting out the bodies until we can figure
/// out the crashes that the extra diasnostic code seems to have caused.
void OldGen::protectCardTableBoundaries() {
  if (gc_->doMetadataProtection_) {
    forUsedSegments([this](GenGCHeapSegment &segment) {
      segment.cardTable().protectBoundaryTable();
      auto res = protectedCardTables_.insert(&segment.cardTable());
      (void)res;
      assert(res.second);
    });
  }
}

void OldGen::unprotectCardTableBoundaries() {
  if (gc_->doMetadataProtection_) {
    forUsedSegments([this](GenGCHeapSegment &segment) {
      assert(
          protectedCardTables_.find(&segment.cardTable()) !=
          protectedCardTables_.end());
      segment.cardTable().unprotectBoundaryTable();
      protectedCardTables_.erase(&segment.cardTable());
    });
  }
}

void OldGen::protectActiveSegCardTableBoundaries() {
  if (gc_->doMetadataProtection_) {
    activeSegment().cardTable().protectBoundaryTable();
    auto res = protectedCardTables_.insert(&activeSegment().cardTable());
    (void)res;
    assert(res.second);
  }
}

void OldGen::unprotectActiveSegCardTableBoundaries() {
  if (gc_->doMetadataProtection_) {
    assert(
        protectedCardTables_.find(&activeSegment().cardTable()) !=
        protectedCardTables_.end());
    activeSegment().cardTable().unprotectBoundaryTable();
    protectedCardTables_.erase(&activeSegment().cardTable());
  }
}
#endif

#ifdef HERMES_EXTRA_DEBUG
void OldGen::summarizeOldGenVTables() {
  forUsedSegments(
      [](GenGCHeapSegment &segment) { segment.summarizeVTables(); });
}

void OldGen::checkSummarizedOldGenVTables(unsigned fullGCNum) {
  forUsedSegments([this, fullGCNum](const GenGCHeapSegment &segment) {
    if (!segment.checkSummarizedVTables()) {
      numVTableSummaryErrors_++;
      char detailBuffer[100];
      snprintf(
          detailBuffer,
          sizeof(detailBuffer),
          "VTable summary changed since last GC for "
          "[%p, %p).  (Full GC %d; last of %d errors)",
          segment.lowLim(),
          segment.hiLim(),
          fullGCNum,
          numVTableSummaryErrors_);
      hermesLog("HermesGC", "Error: %s.", detailBuffer);
      // Record the OOM custom data with the crash manager.
      if (gc_->crashMgr_) {
        gc_->crashMgr_->setCustomData(
            "HermesVTableSummaryErrors", detailBuffer);
      }
    }
  });
}
#endif

void OldGen::didFinishGC() {
  levelAtEndOfLastGC_ = levelDirect();
}

void OldGen::updateCrashManagerHeapExtents(
    const std::string &runtimeName,
    CrashManager *crashMgr) {
  if (!crashMgr) {
    return;
  }
  /// The number of segments we describe with a single key.
  constexpr unsigned kSegmentsPerKey = 10;
  constexpr unsigned N = 1000;
  char keyBuffer[N];
  char valueBuffer[N];
  /// +1 for the active segment.
  unsigned numCurSegments = filledSegments_.size() + 1;

  const char *kOgKeyFormat = "%s:HeapSegments_OG:%d";

  // First erase any keys that are no longer necessary, if the old gen shrank.
  // (This will only happen after a full GC, and the segments that
  // remain will retain their recorded extents.)
  if (numCurSegments < crashMgrRecordedSegments_) {
    for (unsigned toDel = llvh::alignTo(numCurSegments, kSegmentsPerKey);
         toDel < crashMgrRecordedSegments_;
         toDel += kSegmentsPerKey) {
      (void)snprintf(keyBuffer, N, kOgKeyFormat, runtimeName.c_str(), toDel);
      crashMgr->removeCustomData(keyBuffer);
#ifdef HERMESVM_PLATFORM_LOGGING
      hermesLog("HermesGC", "Removed OG heap extents for %d", toDel);
#endif
    }
  }

  // Now redo any keys whose segment sequences might have changed.
  // (They might have gotten smaller, because of GC, or larger, because
  // of segment allocation.)
  const unsigned firstKeyWithPossiblyChangedSegments = llvh::alignDown(
      std::min(numCurSegments, crashMgrRecordedSegments_), kSegmentsPerKey);
  for (unsigned toRedo = firstKeyWithPossiblyChangedSegments;
       toRedo < numCurSegments;
       toRedo += kSegmentsPerKey) {
    (void)snprintf(keyBuffer, N, kOgKeyFormat, runtimeName.c_str(), toRedo);
    char *buf = valueBuffer;
    buf[0] = '\0';
    int sz = N;
    bool first = true;
    {
      int n = snprintf(buf, sz, "[");
      buf += n;
      sz -= n;
    }
    for (unsigned segIndex = toRedo;
         segIndex < std::min(numCurSegments, toRedo + kSegmentsPerKey);
         segIndex++) {
      if (first) {
        first = false;
      } else {
        int n = snprintf(buf, sz, ",");
        buf += n;
        sz -= n;
      }
      if (segIndex < filledSegments_.size()) {
        filledSegments_[segIndex].addExtentToString(&buf, &sz);
      } else {
        assert(
            segIndex == filledSegments_.size() &&
            "segIndex should only go one past filledSegments_, to activeSegment()");
        activeSegment().addExtentToString(&buf, &sz);
      }
    }
    {
      int n = snprintf(buf, sz, "]");
      buf += n;
      sz -= n;
    }
    crashMgr->setCustomData(keyBuffer, valueBuffer);
#ifdef HERMESVM_PLATFORM_LOGGING
    hermesLog(
        "HermesGC",
        "Added OG heap extent: %s = %s",
        &keyBuffer[0],
        &valueBuffer[0]);
#endif
  }
  crashMgrRecordedSegments_ = numCurSegments;
}

} // namespace vm
} // namespace hermes
#undef DEBUG_TYPE
