/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#define DEBUG_TYPE "gc"
#include "hermes/VM/YoungGen.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"

#include "hermes/Support/OSCompat.h"
#include "hermes/Support/PerfSection.h"
#include "hermes/VM/AllocResult.h"
#include "hermes/VM/FillerCell.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/GCCell-inline.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HermesValue-inline.h"
#include "hermes/VM/HiddenClass.h"
#include "hermes/VM/SlotAcceptorDefault.h"
#include "hermes/VM/StringPrimitive.h"

#include <chrono>

using llvm::dbgs;
using std::chrono::steady_clock;

namespace hermes {
namespace vm {

YoungGen::YoungGen(
    GenGC *gc,
    OldGen *nextGen,
    char *lowLim,
    char *hiLim,
    size_t size)
    : ContigAllocGCSpace(lowLim, lowLim, lowLim + size, hiLim),
      gc_(gc),
      nextGen_(nextGen) {
  // Must initialize the compaction move callback.
  setCompactMoveCallback();
}

void YoungGen::printStats(llvm::raw_ostream &os, bool trailingComma) const {
  double youngGenSurvivalPct = 0.0;
  if (cumPreBytes_ > 0) {
    youngGenSurvivalPct = 100.0 * static_cast<double>(cumPromotedBytes_) /
        static_cast<double>(cumPreBytes_);
  }

  os << "\t\t\t\"ygMarkOldToYoungTime\": " << markOldToYoungSecs_ << ",\n"
     << "\t\t\t\"ygMarkRootsTime\": " << markRootsSecs_ << ",\n"
     << "\t\t\t\"ygScanTransitiveTime\": " << scanTransitiveSecs_ << ",\n"
     << "\t\t\t\"ygUpdateWeakRefsTime\": " << updateWeakRefsSecs_ << ",\n"
     << "\t\t\t\"ygFinalizersTime\": " << finalizersSecs_ << ",\n"
     << "\t\t\t\"ygSurvivalPct\": " << youngGenSurvivalPct;
  if (trailingComma) {
    os << ",";
  }
  os << "\n";
}

AllocResult YoungGen::curGenAllocFailure(
    uint32_t allocSize,
    HasFinalizer hasFinalizer,
    bool fixedSizeAlloc) {
  // Can we do a worst-case collection of the YoungGen?  That is, does the
  // next generation have sufficient space to let the collection complete
  // if everything survives?
  if (LLVM_LIKELY(used() <= nextGen_->available())) {
    // There is enough space; do the young-gen collection.
    collect();
    AllocResult res = allocRaw(allocSize, hasFinalizer);
    if (res.success) {
      return res;
    } else if (!fixedSizeAlloc) {
      // Since the collection above evacuated the young generation,
      // the object being allocated must be too large to fit in the
      // young generation.  Try allocating it directly in the old
      // generation (if that was allowed.
      res = nextGen_->allocRaw(allocSize, hasFinalizer);
      if (res.success) {
        return res;
      }
    }
  }
  // Otherwise: either there wasn't enough space in the old generation
  // to allow worst-case evacuation, or the individual allocation was
  // too large to fit in the young-gen, and it also didn't fit in the
  // old gen, or old-gen allocation was not allowed.  Try collecting
  // the old gen, and see if that allows the collection or allocation.
  return fullCollectThenAlloc(allocSize, hasFinalizer, fixedSizeAlloc);
}

AllocResult YoungGen::fullCollectThenAlloc(
    uint32_t allocSize,
    HasFinalizer hasFinalizer,
    bool fixedSizeAlloc) {
  gc_->collect();
  {
    AllocResult res = allocRaw(allocSize, hasFinalizer);
    if (LLVM_LIKELY(res.success)) {
      return res;
    }
  }

  // Try to grow the next gen to allow young-gen collection, if the allocation
  // can fit into the young generation.
  if (allocSize <= size() && nextGen_->growHigh(used())) {
    collect();
    AllocResult res = allocRaw(allocSize, hasFinalizer);
    assert(res.success && "preceding test should guarantee success.");
    return res;
  }

  // The allocation is not going to fit into the young generation, if it is not
  // a fixed size allocation, try and fit it into the old generation.
  if (!fixedSizeAlloc) {
    if (allocSize <= nextGen_->available() || nextGen_->growHigh(allocSize)) {
      AllocResult res = nextGen_->allocRaw(allocSize, hasFinalizer);
      assert(res.success && "preceding test should guarantee success.");
      return res;
    }
  }

  // We did everything we could, bail.
  gc_->oom(make_error_code(OOMError::MaxHeapReached));
}

void YoungGen::collect() {
  PerfSection ygSystraceSection("YoungGen collection");

  struct YoungGenEvacAcceptor final : public SlotAcceptorDefault {
    YoungGen &gen;
    YoungGenEvacAcceptor(GC &gc, YoungGen &gen)
        : SlotAcceptorDefault(gc), gen(gen) {}

    using SlotAcceptorDefault::accept;

    void accept(void *&ptr) override {
      gen.ensureReferentCopied(reinterpret_cast<GCCell **>(&ptr));
    }

    void accept(HermesValue &hv) override {
      if (hv.isPointer()) {
        gen.ensureReferentCopied(&hv);
      }
    }
  };

  auto wallStart = steady_clock::now();
  auto cpuStart = oscompat::thread_cpu_time();
  {
    GC::GCCycle cycle{gc_};

#ifndef NDEBUG
    gc_->doAllocCensus();
#endif

    // Add bytes since the last GC to the total.
    gc_->updateTotalAllocStats();

// Reset the number of reachable and finalized objects for the young gen.
#ifndef NDEBUG
    resetNumReachableObjects();
    resetNumHiddenClasses();

    // Remember the number of allocated objects, to compute the number
    // collected.
    unsigned numAllocatedObjectsBefore = gc_->computeNumAllocatedObjects();
#endif
    resetNumFinalizedObjects();
    // Track the sum of the total pre-collection sizes of the young gens.
    size_t youngGenUsedBefore = used();
    ygSystraceSection.addArg("ygUsedBefore", youngGenUsedBefore);

    size_t oldGenUsedBefore = nextGen_->used();
    cumPreBytes_ += youngGenUsedBefore;

    LLVM_DEBUG(
        dbgs() << "\nStarting (young-gen, " << formatSize(size())
               << ") garbage collection; collection # " << gc_->numCollections()
               << "\n");
#ifdef HERMES_SLOW_DEBUG
    gc_->checkWellFormedHeap();
#endif

    // Remember the point in the older generation into which we started
    // promoting objects.
    char *toScan = nextGen_->level();

    // We do this first, before marking from the roots, so that we can take
    // a "snapshot" of the level of the old gen, and only iterate over pointers
    // in old-gen objects allocated at the start of the collection.
    auto markOldToYoungStart = steady_clock::now();
    {
      PerfSection ygMarkOldToYoungSystraceRegion("ygMarkOldToYoung");
      nextGen_->markYoungGenPointers(this);
    }

    auto markRootsStart = steady_clock::now();
    YoungGenEvacAcceptor acceptor(*gc_, *this);
    DroppingAcceptor<YoungGenEvacAcceptor> nameAcceptor{acceptor};
    {
      PerfSection ygMarkRootsSystraceRegion("ygMarkRoots");
      gc_->markRoots(nameAcceptor, /*markLongLived*/ false);
    }

    auto scanTransitiveStart = steady_clock::now();
    {
      PerfSection ygScanTransitiveSystraceRegion("ygScanTransitive");
      while (toScan < nextGen_->level()) {
        GCCell *cell = reinterpret_cast<GCCell *>(toScan);
        toScan += cell->getAllocatedSize();

        // Ask the object to mark the pointers it owns.
        GCBase::markCell(cell, gc_, acceptor);
      }
    }

    // We've now determined reachability; find weak refs to young-gen
    // pointers that have become unreachable.
    auto updateWeakRefsStart = steady_clock::now();
    {
      PerfSection ygUpdateWeakRefsSystraceRegion("ygUpdateWeakRefs");
      gc_->updateWeakReferences(/*fullGC*/ false);
    }

    // Call the finalizers of unreachable objects. Assumes all cells that
    // survived the young gen collection are moved to the old gen collection.
    auto finalizersStart = steady_clock::now();
    {
      PerfSection ygFinalizeSystraceRegion("ygFinalize");
      finalizeUnreachableAndTransferReachableObjects();
    }

    // Restart allocation at the bottom of the space.
    level_ = start_;

#ifndef NDEBUG
    clear();
#endif

    // Record the final level in both generations, so we can track later
    // allocations.
    gc_->recordGenLevelsAtEndOfLastGC();

#ifdef HERMES_SLOW_DEBUG
    gc_->checkWellFormedHeap();
#endif

#ifndef NDEBUG
    // Update statistics:

    // Update the "last-gc" stats for the heap as a whole.
    // At this point, all young-gen objects that were reachable have
    // been moved to the old generation, and we're considering the
    // objects already in the old generation to be reachable, so the
    // total number of reachable objects is just the old-gen allocated
    // objects.
    gc_->recordNumReachableObjects(nextGen_->numAllocatedObjects());

    // The hidden classes found reachable in the young gen were moved to
    // the old gen; move the stat, and record it in the GCBase variable.
    nextGen_->incNumHiddenClasses(/*leafOnly*/ false, numHiddenClasses_);
    nextGen_->incNumHiddenClasses(/*leafOnly*/ true, numLeafHiddenClasses_);
    gc_->recordNumHiddenClasses(
        nextGen_->numHiddenClasses(/*leafOnly*/ false),
        nextGen_->numHiddenClasses(/*leafOnly*/ true));

    // Record the number of collected objects.
    gc_->recordNumCollectedObjects(
        numAllocatedObjectsBefore - gc_->numReachableObjects_);
    // Only objects in the young generation were finalized, so we set
    // the heap number to the young-gen's number.
    gc_->recordNumFinalizedObjects(numFinalizedObjects_);

    // Space is free; reset num allocated, reachable.
    numAllocatedObjects_ = 0;
    resetNumReachableObjects();
    resetNumHiddenClasses();
#endif

    auto cpuEnd = oscompat::thread_cpu_time();
    auto wallEnd = steady_clock::now();

    double wallElapsedSecs = GCBase::clockDiffSeconds(wallStart, wallEnd);
    double cpuElapsedSecs = GCBase::clockDiffSeconds(cpuStart, cpuEnd);

    // Record as an overall collection.
    gc_->recordGCStats(wallElapsedSecs, cpuElapsedSecs, gc_->size());
    // Also record as a young-gen collection.
    gc_->recordGCStats(
        wallElapsedSecs,
        cpuElapsedSecs,
        size(),
        &gc_->youngGenCollectionCumStats_);

    markOldToYoungSecs_ +=
        GCBase::clockDiffSeconds(markOldToYoungStart, markRootsStart);
    markRootsSecs_ +=
        GCBase::clockDiffSeconds(markRootsStart, scanTransitiveStart);
    scanTransitiveSecs_ +=
        GCBase::clockDiffSeconds(scanTransitiveStart, updateWeakRefsStart);
    updateWeakRefsSecs_ +=
        GCBase::clockDiffSeconds(updateWeakRefsStart, finalizersStart);
    finalizersSecs_ += GCBase::clockDiffSeconds(finalizersStart, wallEnd);
    // Track the bytes of promoted objects.
    size_t promotedBytes = (nextGen_->used() - oldGenUsedBefore);
    cumPromotedBytes_ += promotedBytes;
    ygSystraceSection.addArg("ygPromoted", promotedBytes);

    LLVM_DEBUG(
        dbgs() << "End (young-gen) garbage collection. numCollected="
               << gc_->numCollectedObjects_
               << "; wall time=" << formatSecs(wallElapsedSecs)
               << "; cpu time=" << formatSecs(cpuElapsedSecs) << "\n\n");

#ifdef HERMES_SLOW_DEBUG
    GCBase::DebugHeapInfo info;
    gc_->getDebugHeapInfo(info);
    info.assertInvariants();
    // Assert an additional invariant involving the number of allocated
    // objects before collection.
    assert(
        numAllocatedObjectsBefore - info.numReachableObjects ==
            info.numCollectedObjects &&
        "collected objects computed incorrectly");
#endif
  }
}

void YoungGen::creditExternalMemory(uint32_t size) {
  externalMemory_ += size;
  ContigAllocGCSpace::creditExternalMemory(size);
}

void YoungGen::debitExternalMemory(uint32_t size) {
  assert(externalMemory_ >= size);
  externalMemory_ -= size;
  ContigAllocGCSpace::debitExternalMemory(size);
}

void YoungGen::updateEffectiveEndForExternalMemory() {
  char *newEffectiveEnd = std::max(level_, end_ - externalMemory_);
  if (newEffectiveEnd > effectiveEnd_) {
#ifndef NDEBUG
    clear(effectiveEnd_, newEffectiveEnd);
#endif
    effectiveEnd_ = newEffectiveEnd;
  }
}

void YoungGen::ensureReferentCopied(HermesValue *hv) {
  assert(hv->isPointer() && "Should only call on pointer HermesValues");
  GCCell *cell = static_cast<GCCell *>(hv->getPointer());
  if (contains(cell)) {
    hv->setInGC(hv->updatePointer(forwardPointer(cell)), gc_);
  }
}

void YoungGen::ensureReferentCopied(GCCell **ptrLoc) {
  GCCell *ptr = *ptrLoc;
  if (contains(ptr)) {
    *ptrLoc = forwardPointer(ptr);
  }
}

GCCell *YoungGen::forwardPointer(GCCell *ptr) {
  assert(contains(ptr));
  GCCell *cell = ptr;

  // If the object has already been moved, in which case 'vt' is the forwarding
  // pointer, we just need to update the source pointer.
  if (isForwardingPointer(cell)) {
    return cell->getForwardingPointer();
  }

  uint32_t size = cell->getAllocatedSize();

  /// The finalizer parameter is always set to no under the assumption that the
  /// reachable cells with finalizers in the finalizer list of Young Gen are
  /// moved to the finalizer list of Old Gen.
  AllocResult res = nextGen_->allocRaw(size, HasFinalizer::No);
  // This assertion is justified because we take pains not to start a
  // young-gen collection unless we can ensure that the worst-case of all
  // data being live can be accommodated in the old generation.
  assert(res.success);
  memcpy(res.ptr, cell, size);
  // We can now consider res.ptr to be a GCCell.
  GCCell *newCell = reinterpret_cast<GCCell *>(res.ptr);
#ifndef NDEBUG
  numReachableObjects_++;
  if (auto *hiddenClass = dyn_vmcast<HiddenClass>(cell)) {
    ++numHiddenClasses_;
    numLeafHiddenClasses_ += hiddenClass->isKnownLeaf();
  }
#endif

  // Store the forwarding pointer in the original object.
  cell->setForwardingPointer(newCell);

  // Update the source pointer.
  return newCell;
}

bool YoungGen::isForwardingPointer(const GCCell *cell) const {
  return nextGen_->contains(cell->getForwardingPointer());
}

void YoungGen::finalizeUnreachableAndTransferReachableObjects() {
  for (const auto &cell : cellsWithFinalizers_) {
    if (isForwardingPointer(cell)) {
      nextGen_->addToFinalizerList(cell->getForwardingPointer());
      continue;
    }
    cell->getVT()->finalize(cell, gc_);
    numFinalizedObjects_++;
  }
  cellsWithFinalizers_.clear();
  // At this point, we've considered YG objects with external memory (and thus
  // finalizers.) If they were unreachable, their finalizers were run, and the
  // YG's external memory credit was debited.  If they were reachable, their
  // charge remains, but the objects have been promoted to the old gen. so
  // their external memory credit must be transferred.
  nextGen_->creditExternalMemory(externalMemory_);
  externalMemory_ = 0;
  /// Since there is no longer any charged external memory, effectiveEnd_ can
  /// return to end_.
  updateEffectiveEndForExternalMemory();
}

void YoungGen::compact(
    const MarkBitArray &markBits,
    const SweepResult &sweepResult) {
  ContigAllocGCSpace::compact(markBits, sweepResult);
  compactFinalizableObjectList();
}

void YoungGen::compactFinalizableObjectList() {
  size_t numCells = cellsWithFinalizers_.size();
  unsigned retainedIndex = 0;
  for (unsigned i = 0; i < numCells; i++) {
    const auto cell = cellsWithFinalizers_.at(i);
    if (!contains(cell)) {
      nextGen_->addToFinalizerList(cell);
    } else {
      // This may overwrite some with the same value, if retainedIndex
      // is the same as the current implicit index of the for loop,
      // but that's OK.
      cellsWithFinalizers_[retainedIndex] = cell;
      retainedIndex++;
    }
  }
  cellsWithFinalizers_.resize(retainedIndex);
  cellsWithFinalizers_.shrink_to_fit();
}

uint64_t YoungGen::extSizeFromFinalizerList() const {
  uint64_t extSize = 0;
  for (const auto cell : cellsWithFinalizers_) {
    extSize += cell->externalMemorySize();
  }
  return extSize;
}

void YoungGen::setCompactMoveCallback() {
  // YounGen objects may be compacted into the old, and must update the
  // CardObjectTable when they do so.
  CardObjectTable *cardObjectTable = nextGen_->cardObjectTable();
  char *oldGenStart = nextGen_->start();
  // TODO (ddetlefs): Ashok Menon noted that we could capture nextGen_
  // here, and get the nextCardBoundary and oldGenStart from that.
  // Then we wouldn't need to reset the callback in "moveHeap",
  // below.  But there might be performance implications because of
  // the extra levels of indirection, so we'd need to investigate that
  // before doing the change.
  compactMoveCallback_ = [cardObjectTable, oldGenStart](
                             char *start, char *end) {
    if (start >= oldGenStart && end > cardObjectTable->nextCardBoundary()) {
      cardObjectTable->updateEntries(start, end);
    }
  };
}

void YoungGen::moveHeap(GC *gc, ptrdiff_t moveHeapDelta) {
  ContigAllocGCSpace::moveHeap(gc, moveHeapDelta);
  // We must reset the move callback, since the start address of the old gen has
  // changed.
  setCompactMoveCallback();
}

#ifdef HERMES_SLOW_DEBUG
void YoungGen::checkWellFormed(const GC *gc) const {
  uint64_t extSize = 0;
  ContigAllocGCSpace::checkWellFormed(gc, &extSize);
  assert(extSize == externalMemory_);
}
#endif

} // namespace vm
} // namespace hermes
