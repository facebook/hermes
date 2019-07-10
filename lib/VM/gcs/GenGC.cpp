/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
// Note: this must include GC.h, instead of GenGC.h, because GenGC.h assumes
// it is included only by GC.h.  (For example, it assumes GCBase is declared.)
#include "hermes/VM/GC.h"

#include "hermes/Support/OSCompat.h"
#include "hermes/Support/PerfSection.h"
#include "hermes/VM/AllocSource.h"
#include "hermes/VM/Casting.h"
#include "hermes/VM/CheckHeapWellFormedAcceptor.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HeapSnapshot.h"
#include "hermes/VM/HermesValue-inline.h"
#include "hermes/VM/SlotAcceptorDefault.h"
#include "hermes/VM/SnapshotAcceptor.h"
#include "hermes/VM/Space-inline.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/SweepResult.h"
#include "hermes/VM/SymbolID.h"

#ifdef HERMES_SLOW_DEBUG
#include "llvm/ADT/DenseSet.h"
#endif
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/MathExtras.h"

#include <algorithm>
#include <chrono>
#include <tuple>
#include <utility>

#define DEBUG_TYPE "gc"

using llvm::dbgs;
using std::chrono::steady_clock;
using namespace hermes;

namespace hermes {
namespace vm {

namespace {

/// Round \p size up to the nearest page, and make sure it is at least 2 pages,
/// the minimum size the heap can be.
static gcheapsize_t clampAndPageAlign(gcheapsize_t size) {
  const auto PS = static_cast<gcheapsize_t>(oscompat::page_size());
  return std::max(static_cast<gcheapsize_t>(llvm::alignTo(size, PS)), 2 * PS);
}

} // anonymous namespace

GenGC::Size::Size(const GCConfig &gcConfig)
    : Size(gcConfig.getMinHeapSize(), gcConfig.getMaxHeapSize()) {}

GenGC::Size::Size(gcheapsize_t min, gcheapsize_t max)
    : min_(clampAndPageAlign(min)), max_(clampAndPageAlign(max)) {}

gcheapsize_t GenGC::Size::storageFootprint() const {
  // GenGC uses no storage from the StorageProvider.
  return 0;
}

gcheapsize_t GenGC::Size::minStorageFootprint() const {
  // GenGC uses no storage from the StorageProvider.
  return 0;
}

GenGC::GenGC(
    MetadataTable metaTable,
    GCCallbacks *gcCallbacks,
    PointerBase *pointerBase,
    const GCConfig &gcConfig,
    std::shared_ptr<CrashManager> crashMgr,
    StorageProvider *provider)
    : GCBase(
          metaTable,
          gcCallbacks,
          pointerBase,
          gcConfig,
          std::move(crashMgr),
          provider),
      // The minimum initial size is 2 pages.
      initialSize_(clampAndPageAlign(gcConfig.getInitHeapSize())),
      // The minimum maximum size is 2 pages.
      maxSize_(Size(gcConfig).max()),
      storage_(allocBackingStorage()),
      oldGen_(
          this,
          youngGenHiLim(),
          storage_.hiLim(),
          initialSize_ - youngGenSize(initialSize_)),
      youngGen_(
          this,
          &oldGen_,
          storage_.lowLim(),
          youngGenHiLim(),
          youngGenSize(initialSize_)),
      markBits_(
          storage_.lowLim(),
          youngGen_.start(),
          oldGen_.end(),
          storage_.hiLim()) {}

#ifndef NDEBUG
AllocResult
GenGC::debugAlloc(uint32_t sz, HasFinalizer hasFinalizer, bool fixedSize) {
  AllocResult res;
  // Only variable-sized objects may be allocated in both generations.
  if (shouldRandomizeAllocSpace() && !fixedSize) {
    if (allocInYoung_) {
      res = youngGen_.alloc(sz, hasFinalizer);
    } else {
      res = oldGen_.alloc(sz, hasFinalizer);
    }
    allocInYoung_ = !allocInYoung_;
  } else {
    // Always try first in young.
    res = youngGen_.alloc(sz, hasFinalizer, fixedSize);
  }
  return res;
}
#endif

void GenGC::creditExternalMemory(GCCell *alloc, uint32_t size) {
#ifndef NDEBUG
  numExtAllocs_++;
  curExtAllocBytes_ += size;
  if (curExtAllocBytes_ > maxExtAllocBytes_) {
    maxExtAllocBytes_ = curExtAllocBytes_;
  }
  totalExtAllocBytes_ += size;
#endif
  if (youngGen_.contains(alloc)) {
    youngGen_.creditExternalMemory(size);
  } else {
    oldGen_.creditExternalMemory(size);
  }
}

void GenGC::debitExternalMemory(GCCell *cell, uint32_t size) {
#ifndef NDEBUG
  curExtAllocBytes_ -= size;
#endif
  if (youngGen_.contains(cell)) {
    youngGen_.debitExternalMemory(size);
  } else {
    oldGen_.debitExternalMemory(size);
  }
}

#ifndef NDEBUG
void GenGC::printExtAllocStats(llvm::raw_ostream &os) {
  os << "\nExternal allocation:\n";
  os << "   " << numExtAllocs_ << " allocs for " << totalExtAllocBytes_
     << " bytes; average = "
     << (numExtAllocs_ == 0 ? 0.0
                            : static_cast<double>(totalExtAllocBytes_) /
                 static_cast<double>(numExtAllocs_))
     << ".\n"
     << "   Max allocation at one time: " << maxExtAllocBytes_ << " bytes.\n";
}
#endif

void GenGC::finalizeAll() {
  markBits_.clear();
  // Need to reset numFinalizedObjects before calling the finalizers.
  // The finalize code uses that field to resize the storage for finalizable
  // cells.
  resetNumFinalizedObjectsInGens();
  youngGen_.finalizeUnreachableObjects(this, &markBits_);
  oldGen_.finalizeUnreachableObjects(this, &markBits_);
}

void GenGC::collect() {
  PerfSection fullGCSystraceRegion("Full collection");
  auto wallStart = steady_clock::now();
  auto cpuStart = oscompat::thread_cpu_time();

  size_t usedBefore = used();
  fullGCSystraceRegion.addArg("fullGCUsedBefore", usedBefore);
#ifndef NDEBUG
  resetNumReachableObjectsInGens();
  resetNumHiddenClassesInGens();
  numMarkedSymbols_ = 0;
#endif

  // Used to return memory to the OS.
  char *oldGenLevelBefore = oldGen_.level();

  resetNumFinalizedObjectsInGens();
  cumPreBytes_ += used();

  LLVM_DEBUG(
      dbgs() << "\nStarting (full, young=" << formatSize(youngGen_.size())
             << "; old=" << formatSize(oldGen_.size())
             << ") garbage collection # " << numCollections() << "\n");
  {
    GCCycle cycle{this};

#ifndef NDEBUG
    doAllocCensus();
#endif

    updateTotalAllocStats();
#ifdef HERMES_SLOW_DEBUG
    checkWellFormedHeap();
#endif

    markPhase();

    std::vector<SweepResult> sweepResults = sweepAndInstallForwardingPointers();
    updateReferences(sweepResults);
    compact(sweepResults);

    // First arg indicates whether we were able to completely evacuate the young
    // generation.
    oldGen_.cardTable().updateAfterCompaction(
        youngGen_.used() == 0, oldGen_.level());
#ifdef HERMES_SLOW_DEBUG
    oldGen_.verifyCardObjectTable();
#endif

    oldGen_.levelChangedFrom(oldGenLevelBefore);

    // Record the new levels of the generations, so we can accurately
    // identify new allocations, to count their size and to iterate over
    // allocated objects.
    recordGenLevelsAtEndOfLastGC();

    // Should we increase the total heap size?
    size_t sizeBefore = size();
    fullGCSystraceRegion.addArg("fullGCSizeBefore", sizeBefore);
    if (used() > size() / 2) {
#ifndef NDEBUG
      size_t oldSize = size();
#endif
      uint64_t desiredNewSize = llvm::alignTo(
          static_cast<uint64_t>(
              static_cast<double>(used()) / occupancyTarget()),
          oscompat::page_size());
      OptValue<SizeConfig> sizeConfig = newSizesForDesiredSize(desiredNewSize);
      if (sizeConfig) {
        youngGen_.growHigh(sizeConfig->ygSize - youngGen_.size());
        oldGen_.growHigh(sizeConfig->ogSize - oldGen_.size());

        didResize();
        LLVM_DEBUG(
            dbgs() << "Increased heap size by " << sizeConfig->total() - oldSize
                   << " to " << formatSize(sizeConfig->total()) << "\n");
      }
    }
    size_t sizeAfter = size();
    fullGCSystraceRegion.addArg("fullGCSizeAfter", sizeAfter);

    gcCallbacks_->freeSymbols(markedSymbols_);

    auto cpuEnd = oscompat::thread_cpu_time();
    auto wallEnd = steady_clock::now();

    // Update the statistics.
    unsigned numAllocatedObjectsBefore = recordStats(sweepResults);

    double wallElapsedSecs = GCBase::clockDiffSeconds(wallStart, wallEnd);
    double cpuElapsedSecs = GCBase::clockDiffSeconds(cpuStart, cpuEnd);

    // Record as an overall collection.
    recordGCStats(wallElapsedSecs, cpuElapsedSecs, size());
    // Also record as a full collection.
    recordGCStats(
        wallElapsedSecs, cpuElapsedSecs, size(), &fullCollectionCumStats_);
    size_t usedAfter = used();
    cumPostBytes_ += usedAfter;
    fullGCSystraceRegion.addArg("fullGCUsedAfter", usedAfter);

    LLVM_DEBUG(
        dbgs() << "End (full) garbage collection. numCollected="
               << numCollectedObjects_
               << "; wall time=" << formatSecs(wallElapsedSecs)
               << "; cpu time=" << formatSecs(cpuElapsedSecs) << "\n\n");

#ifdef HERMES_SLOW_DEBUG
    checkWellFormedHeap();
#endif
    checkInvariants(numAllocatedObjectsBefore, usedBefore);

    checkTripwire(usedAfter, steady_clock::now());
  }
}

unsigned GenGC::recordStats(const std::vector<SweepResult> &sweepResults) {
#ifndef NDEBUG
  // Report current status to the GCBase variables.
  recordNumReachableObjects(computeNumReachableObjects());
  recordNumHiddenClasses(
      computeNumHiddenClasses(/*leafOnly*/ false),
      computeNumHiddenClasses(/*leafOnly*/ true));
  unsigned numAllocatedObjectsBefore = computeNumAllocatedObjects();
  recordNumCollectedObjects(numAllocatedObjectsBefore - numReachableObjects_);
  recordNumFinalizedObjects(computeNumFinalizedObjects());

  // Reset the per-generation allocated objects to those found reachable.
  setNumAllocatedObjectsAfterFullCollection(sweepResults);
  return numAllocatedObjectsBefore;
#else
  return 0;
#endif
}

void GenGC::checkInvariants(
    unsigned numAllocatedObjectsBefore,
    size_t usedBefore) {
#ifndef NDEBUG
  size_t usedAfter = used();
  // A full GC should never increase used memory.
  assert(usedAfter <= usedBefore);
  // Check that getDebugHeapInfo returns a well-formed info structure.
  DebugHeapInfo info;
  getDebugHeapInfo(info);
  info.assertInvariants();
  // Assert an additional invariant involving the number of allocated
  // objects before collection.
  assert(
      numAllocatedObjectsBefore - info.numReachableObjects ==
          info.numCollectedObjects &&
      "collected objects computed incorrectly");
#endif
}

bool GenGC::contains(void *ptr) const {
  return storage_.contains(ptr);
}

char *GenGC::lowLim() const {
  return storage_.lowLim();
}

char *GenGC::hiLim() const {
  return storage_.hiLim();
}

#ifndef NDEBUG
bool GenGC::validPointer(const void *ptr) const {
  return youngGen_.validPointer(ptr) || oldGen_.validPointer(ptr);
}

bool GenGC::isMostRecentFinalizableObj(const GCCell *cell) const {
  return GCBase::isMostRecentCellInFinalizerVector(
             youngGen_.cellsWithFinalizers(), cell) ||
      GCBase::isMostRecentCellInFinalizerVector(
             oldGen_.cellsWithFinalizers(), cell);
}
#endif

size_t GenGC::size() const {
  return youngGen_.size() + oldGen_.size();
}

size_t GenGC::used() const {
  return youngGen_.used() + oldGen_.used();
}

#ifdef HERMES_SLOW_DEBUG
/// This is used to detect whether the Runtime::markRoots ever invokes
/// the acceptor on the same root location more than once, which is illegal.
struct FullMSCDuplicateRootsDetectorAcceptor final
    : public SlotAcceptorDefault {
  llvm::DenseSet<void *> markedLocs_;

  using SlotAcceptorDefault::accept;
  using SlotAcceptorDefault::SlotAcceptorDefault;

  void accept(void *&ptr) override {
    assert(markedLocs_.count(&ptr) == 0);
    markedLocs_.insert(&ptr);
  }
  void accept(HermesValue &hv) override {
    assert(markedLocs_.count(&hv) == 0);
    markedLocs_.insert(&hv);
  }
};
#endif

void GenGC::markPhase() {
  struct FullMSCMarkInitialAcceptor final : public SlotAcceptorDefault {
    using SlotAcceptorDefault::accept;
    using SlotAcceptorDefault::SlotAcceptorDefault;

    void accept(void *&ptr) override {
      if (gc.contains(ptr)) {
        size_t ind = gc.markBits_.addressToIndex(ptr);
        assert(ind < gc.markBits_.size());
        gc.markBits_.set(ind, true);
      }
    }
    void accept(HermesValue &hv) override {
      if (hv.isPointer()) {
        void *ptr = hv.getPointer();
        if (gc.contains(ptr)) {
          size_t ind = gc.markBits_.addressToIndex(ptr);
          assert(ind < gc.markBits_.size());
          gc.markBits_.set(ind, true);
        }
      } else if (hv.isSymbol()) {
        gc.markSymbol(hv.getSymbol());
      }
    }
    void accept(SymbolID sym) override {
      gc.markSymbol(sym);
    }
    void accept(WeakRefBase &wr) override {
      gc.markWeakRef(wr);
    }
  };
  markedSymbols_.clear();
  markedSymbols_.resize(gcCallbacks_->getSymbolsEnd(), false);

  // We compute full reachability during marking, so determine reachability of
  // WeakRef slots.
  unmarkWeakReferences();

  FullMSCMarkInitialAcceptor acceptor(*this);
  DroppingAcceptor<FullMSCMarkInitialAcceptor> nameAcceptor{acceptor};
  markBits_.clear();

#ifdef HERMES_SLOW_DEBUG
  {
    // We want to guarantee that markRoots doesn't invoke acceptors on
    // locations more than once.
    FullMSCDuplicateRootsDetectorAcceptor dupDetector(*this);
    DroppingAcceptor<FullMSCDuplicateRootsDetectorAcceptor> nameAcceptor{
        dupDetector};
    markRoots(nameAcceptor, /*markLongLived*/ true);
  }
#endif

  auto markRootsStart = steady_clock::now();
  {
    PerfSection fullGCMarkRootsSystraceRegion("fullGCMarkRoots");
    markRoots(nameAcceptor, /*markLongLived*/ true);
  }
  auto completeMarkingStart = steady_clock::now();
  {
    PerfSection fullGCCompleteMarkingSystraceRegion("fullGCCompleteMarking");
    completeMarking();
  }
  auto completeMarkingEnd = steady_clock::now();
  markRootsSecs_ +=
      GCBase::clockDiffSeconds(markRootsStart, completeMarkingStart);
  markTransitiveSecs_ +=
      GCBase::clockDiffSeconds(completeMarkingStart, completeMarkingEnd);
}

void GenGC::completeMarking() {
  while (true) {
    // If there's mark stack overflow in the old generation, we must restart
    // traversal from the young gen.  (In a more general N-generation
    // system, overflow in any generation after the first should cause
    // immediate restart from the first generation.)  While this can
    // happen several times, the overall marking process is guaranteed
    // to terminate.
    youngGen_.completeMarking(this, &markBits_, &markState_);
    if (oldGen_.completeMarking(this, &markBits_, &markState_)) {
      continue;
    }
    // Otherwise, no overflow:
    break;
  }
}

std::vector<SweepResult> GenGC::sweepAndInstallForwardingPointers() {
  auto sweepStart = steady_clock::now();
  // The sweep results for the generations, to be filled in.
  SweepResult ygSweepRes, ogSweepRes;
  // We compact the old gen into itself.
  ContigAllocGCSpace::CompactionRegion ogCompactRegion(oldGen_);
  ContigAllocGCSpace::CompactionRegion ygCompactRegion(youngGen_);

  // Globally, we compact first into the old gen, then into the young gen.
  std::vector<ContigAllocGCSpace::CompactionRegion *> compactRegions{
      &ogCompactRegion, &ygCompactRegion};

  ogSweepRes.displacedVtablePtrs = oldGen_.sweepAndInstallForwardingPointers(
      this, &markBits_, &compactRegions);
  ygSweepRes.displacedVtablePtrs = youngGen_.sweepAndInstallForwardingPointers(
      this, &markBits_, &compactRegions);
  ogSweepRes.level = ogCompactRegion.start;
  ygSweepRes.level = ygCompactRegion.start;
#ifndef NDEBUG
  ogSweepRes.numAllocated = ogCompactRegion.numAllocated;
  ygSweepRes.numAllocated = ygCompactRegion.numAllocated;
#endif
  sweepSecs_ += GCBase::clockDiffSeconds(sweepStart, steady_clock::now());
  return {ygSweepRes, ogSweepRes};
}

void GenGC::updateReferences(const std::vector<SweepResult> &sweepResults) {
  auto updateRefsStart = steady_clock::now();
  PerfSection fullGCUpdateReferencesSystraceRegion("fullGCUpdateReferences");
  std::unique_ptr<SlotAcceptor> acceptor = getFullMSCUpdateAcceptor(*this);
  DroppingAcceptor<SlotAcceptor> nameAcceptor{*acceptor};
  markRoots(nameAcceptor, /*markLongLived*/ true);

  // Update weak roots references.
  FullMSCUpdateWeakRootsAcceptor weakAcceptor(*this);
  DroppingAcceptor<SlotAcceptor> nameWeakAcceptor{weakAcceptor};
  markWeakRoots(nameWeakAcceptor);

  youngGen_.updateReferences(
      this, markBits_, sweepResults.at(0).displacedVtablePtrs);
  oldGen_.updateReferences(
      this, markBits_, sweepResults.at(1).displacedVtablePtrs);
  updateWeakReferences(/*fullGC*/ true);
  updateReferencesSecs_ +=
      GCBase::clockDiffSeconds(updateRefsStart, steady_clock::now());
}

void GenGC::compact(const std::vector<SweepResult> &sweepResults) {
  auto compactStart = steady_clock::now();
  PerfSection fullGCCompactSystraceRegion("fullGCCompact");
  oldGen_.prepareForCompaction();
  // Compacting the young gen may copy objects into the old gen, so we
  // must compact the old gen before the young gen.
  oldGen_.compact(markBits_, sweepResults.at(1));
  youngGen_.compact(markBits_, sweepResults.at(0));

  uint64_t newYGExtSize = youngGen_.extSizeFromFinalizerList();
  assert(youngGen_.externalMemory() >= newYGExtSize);
  uint64_t transferred = youngGen_.externalMemory() - newYGExtSize;
  youngGen_.setExternalMemory(newYGExtSize);
  oldGen_.creditExternalMemory(transferred);

  // At this point, finalizers have been run, and any unreachable objects with
  // external memory charges have had those charges adjusted.  So the
  // generations' external memory charges should be accurate, and may have
  // changed.  Adjust their effectiveEnd_ values.
  oldGen_.updateEffectiveEndForExternalMemory();
  youngGen_.updateEffectiveEndForExternalMemory();

  compactSecs_ += GCBase::clockDiffSeconds(compactStart, steady_clock::now());
}

void GenGC::markSymbol(SymbolID symbolID) {
  if (LLVM_UNLIKELY(symbolID.isInvalid()))
    return;

  uint32_t index = symbolID.unsafeGetIndex();
  assert(index < markedSymbols_.size() && "symbolID out of reported range");
#ifndef NDEBUG
  if (!markedSymbols_[index]) {
    markedSymbols_[index] = true;
    ++numMarkedSymbols_;
    LLVM_DEBUG(dbgs() << "markSymbol " << index << "\n");
  }
#else
  markedSymbols_[index] = true;
#endif
}

void GenGC::markWeakRef(const WeakRefBase &wr) {
  assert(
      wr.slot_->extra <= WeakSlotState::Marked &&
      "marking a freed weak ref slot");
  wr.slot_->extra = WeakSlotState::Marked;
}

#ifdef HERMES_SLOW_DEBUG
void GenGC::checkWellFormedHeap() const {
  // OK to cast away const here, because in VerifyRoots mode,
  // markRoots has no side effects.
  GenGC *gc = const_cast<GenGC *>(this);
  CheckHeapWellFormedAcceptor acceptor(*gc);
  DroppingAcceptor<CheckHeapWellFormedAcceptor> nameAcceptor{acceptor};
  gc->markRoots(nameAcceptor, /*markLongLived*/ true);
  youngGen_.checkWellFormed(gc);
  oldGen_.checkWellFormed(gc);
}
#endif

BackingStorage GenGC::allocBackingStorage() {
  auto res = shouldSanitizeHandles()
      ? BackingStorage(maxSize_, initialSize_, AllocSource::Malloc)
      : BackingStorage(maxSize_, initialSize_, "hermes-gengc");

  // If the requested maxSize_ was not available, the created backing storage
  // may be less than that request.  Update maxSize_ to the size we actually
  // got.
  maxSize_ = res.size();
  // This result should be page-aligned, and >= initialSize_.
  assert(maxSize_ == llvm::alignTo(maxSize_, oscompat::page_size()));
  assert(maxSize_ >= initialSize_);
  return res;
}

void GenGC::swapToFreshHeap() {
  BackingStorage newStorage = allocBackingStorage();
  const ptrdiff_t delta = newStorage.lowLim() - storage_.lowLim();

  std::unique_ptr<SlotAcceptor> acceptor = getMoveHeapAcceptor(*this, delta);
  DroppingAcceptor<SlotAcceptor> nameAcceptor{*acceptor};

  {
    GCCycle cycle{this};

    markRoots(nameAcceptor, /*markLongLived*/ true);
    // For purposes of heap swapping, treat weak roots as strong.
    markWeakRoots(nameAcceptor);

    // Note: we must move the oldGen_ before the young, because the youngGen_
    // needs to know the oldGen_ start after the move.
    oldGen_.moveHeap(this, delta);
    youngGen_.moveHeap(this, delta);

    moveWeakReferences(delta);
    markBits_.moveParentRegion(delta);
  }

  // Move over the occupied parts of the heap.
  // (Note that in each case, we use the ContigAllocGCSpace definition of
  // "used()" -- the generation-level definition includes external storage which
  // we don't care about here.
  memcpy(
      youngGen_.start(),
      youngGen_.start() - delta,
      youngGen_.ContigAllocGCSpace::used());
  memcpy(
      oldGen_.start(),
      oldGen_.start() - delta,
      oldGen_.ContigAllocGCSpace::used());
  __asan_unpoison_memory_region(
      youngGen_.start() - delta, youngGen_.end() - youngGen_.start());
  __asan_unpoison_memory_region(
      oldGen_.start() - delta, oldGen_.end() - oldGen_.start());
#ifndef NDEBUG
  // Now that the memory has been moved, make sure to clear the empty space.
  youngGen_.clear(youngGen_.level(), youngGen_.end());
  oldGen_.clear(oldGen_.level(), oldGen_.end());
#endif

  std::swap(storage_, newStorage);
}

void GenGC::moveWeakReferences(ptrdiff_t delta) {
  for (auto &slot : weakSlots_) {
    if (slot.extra == WeakSlotState::Free || !slot.value.isPointer())
      continue;

    auto cell = reinterpret_cast<char *>(slot.value.getPointer());
    slot.value = slot.value.updatePointer(cell + delta);
  }
}

char *GenGC::youngGenHiLim() const {
  return storage_.lowLim() + youngGenSize(maxSize_);
}

size_t GenGC::youngGenSize(size_t totalHeapSize) {
  // 1/kYoungGenFractionDenom the max size, up to a max of kMaxYoungGenSize.
  return llvm::alignTo(
      std::min(
          totalHeapSize / kYoungGenFractionDenom,
          static_cast<size_t>(kMaxYoungGenSize)),
      oscompat::page_size());
}

OptValue<GenGC::SizeConfig> GenGC::newSizesForDesiredSize(
    uint64_t desiredSize) const {
  size_t oldOGSize = oldGen_.size();
  size_t oldSize = size();
  size_t newSize;
  if (desiredSize > static_cast<uint64_t>(maxSize())) {
    newSize = maxSize();
  } else {
    // Since maxSize() is a size_t, must be representable as a size_t.
    newSize = static_cast<size_t>(desiredSize);
  }
  if (newSize > oldSize) {
    size_t newYGSize = youngGenSize(newSize);
    size_t newOGSize = newSize - newYGSize;
    // The young gen may have grown to take a larger fraction of the
    // total -- which could actually shrink the old gen.  Prevent that from
    // happening.
    if (newOGSize < oldOGSize) {
      // This increases the total size of the config, but it can't go
      // beyond maxSize() -- the young-gen is increased to a size at
      // most the max young-gen size, and the oldOGSize was already a
      // legal old-gen size.
      newOGSize = oldOGSize;
    }
    return SizeConfig{newYGSize, newOGSize};
  } else {
    return OptValue<SizeConfig>();
  }
}

#ifndef NDEBUG
size_t GenGC::countUsedWeakRefs() const {
  size_t count = 0;
  for (auto &slot : weakSlots_) {
    if (slot.extra != WeakSlotState::Free) {
      ++count;
    }
  }
  return count;
}
#endif

void GenGC::unmarkWeakReferences() {
  for (auto &slot : weakSlots_) {
    if (slot.extra == WeakSlotState::Marked) {
      slot.extra = WeakSlotState::Unmarked;
    }
  }
}

void GenGC::updateWeakReference(WeakRefSlot *slotPtr, bool fullGC) {
  // Skip free slots.
  if (slotPtr->extra == WeakSlotState::Free) {
    return;
  }

  // TODO: re-enable this for young-gen collection.  See T21007593.
  if (fullGC) {
    // A slot which is no longer reachable. Add it to the free list.
    if (slotPtr->extra == WeakSlotState::Unmarked) {
      freeWeakSlot(slotPtr);
      return;
    }

    assert(
        slotPtr->extra == WeakSlotState::Marked && "invalid marked slot state");
  }

  // Skip non-pointer slots.
  if (!slotPtr->value.isPointer()) {
    return;
  }

  GCCell *cell = (GCCell *)slotPtr->value.getPointer();

  if (fullGC) {
    if (isMarked(cell)) {
      slotPtr->value =
          slotPtr->value.updatePointer(cell->getForwardingPointer());
    } else {
      slotPtr->value = HermesValue::encodeEmptyValue();
    }
  } else {
    // Young-gen collection.  If the cell is in the young gen, see if
    // it survived collection.  If so, update the slot.
    if (youngGen_.contains(cell)) {
      if (oldGen_.contains(cell->getForwardingPointer())) {
        slotPtr->value =
            slotPtr->value.updatePointer(cell->getForwardingPointer());
      } else {
        slotPtr->value = HermesValue::encodeEmptyValue();
      }
    }
  }
}

void GenGC::updateWeakReferences(bool fullGC) {
  if (fullGC) {
    for (auto &slot : weakSlots_) {
      updateWeakReference(&slot, fullGC); // fullGC is true.
    }
  } else {
    for (auto slotPtr : weakRefSlotsWithPossibleYoungReferent_) {
      updateWeakReference(slotPtr, fullGC); // fullGC is false.
    }
  }
  // This loop "filters" weakRefSlotsWithPossibleYoungReferent_, removing
  // any entries whose  values are not pointers into the young gen.
  // The "retainedIndex" variable counts the number of such entries below the
  // "i" index variable; those entries are moved downwards in the vector to be
  // contiguous.
  size_t numSlots = weakRefSlotsWithPossibleYoungReferent_.size();
  unsigned retainedIndex = 0;
  for (unsigned i = 0; i < numSlots; i++) {
    // Since numSlots is the size() of the vector, i is definitely in range.
    auto slotPtr = weakRefSlotsWithPossibleYoungReferent_[i];
    if (slotPtr->value.isPointer()) {
      if (youngGen_.contains(slotPtr->value.getPointer())) {
        weakRefSlotsWithPossibleYoungReferent_[retainedIndex] = slotPtr;
        retainedIndex++;
      }
    }
  }
  weakRefSlotsWithPossibleYoungReferent_.resize(retainedIndex);
  weakRefSlotsWithPossibleYoungReferent_.shrink_to_fit();

  if (fullGC) {
    // For now, we only free slots during full GC.
    shrinkWeakSlots();
  }
}

WeakRefSlot *GenGC::allocWeakSlot(HermesValue init) {
  assert(
      !init.isNativeValue() && !init.isEmpty() &&
      "Cannot set a weak reference to empty or native value");

  WeakRefSlot *res;
  if (firstFreeWeak_) {
    assert(
        firstFreeWeak_->extra == WeakSlotState::Free &&
        "invalid free slot state");
    res = firstFreeWeak_;
    firstFreeWeak_ = firstFreeWeak_->value.getNativePointer<WeakRefSlot>();
    res->value = init;
    res->extra = WeakSlotState::Unmarked;
  } else {
    weakSlots_.push_back({init, WeakSlotState::Unmarked});
    res = &weakSlots_.back();
  }
  weakRefSlotsWithPossibleYoungReferent_.push_back(res);
  return res;
}

void GenGC::freeWeakSlot(WeakRefSlot *slot) {
  slot->value = HermesValue::encodeNativePointer(firstFreeWeak_);
  slot->extra = WeakSlotState::Free;
  firstFreeWeak_ = slot;
}

void GenGC::shrinkWeakSlots() {
  // Opportunistically shrink the deque if free slots are found at the end.
  while (!weakSlots_.empty() && firstFreeWeak_ == &weakSlots_.back()) {
    firstFreeWeak_ = firstFreeWeak_->value.getNativePointer<WeakRefSlot>();
    weakSlots_.pop_back();
  }

  // TODO: in the future we might want to keep an ordered list of free
  // regions. The current singly-linked list doesn't guarantee that the
  // deque will be optimally shrunk.
}

void GenGC::didResize() {
  markBits_.resizeParentUsedRegion(youngGen_.start(), oldGen_.effectiveEnd());
}

#ifndef NDEBUG
void GenGC::youngGenCollect() {
  youngGen_.collect();
}

unsigned GenGC::computeNumAllocatedObjects() const {
  return youngGen_.numAllocatedObjects() + oldGen_.numAllocatedObjects();
}

unsigned GenGC::computeNumReachableObjects() const {
  return youngGen_.numReachableObjects() + oldGen_.numReachableObjects();
}

unsigned GenGC::computeNumHiddenClasses(bool leafOnly) const {
  return youngGen_.numHiddenClasses(leafOnly) +
      oldGen_.numHiddenClasses(leafOnly);
}

unsigned GenGC::computeNumFinalizedObjects() const {
  return youngGen_.numFinalizedObjects() + oldGen_.numFinalizedObjects();
}

void GenGC::resetNumReachableObjectsInGens() {
  youngGen_.resetNumReachableObjects();
  oldGen_.resetNumReachableObjects();
}

void GenGC::resetNumHiddenClassesInGens() {
  youngGen_.resetNumHiddenClasses();
  oldGen_.resetNumHiddenClasses();
}

void GenGC::setNumAllocatedObjectsAfterFullCollection(
    const std::vector<SweepResult> &sweepResults) {
  youngGen_.setNumAllocatedObjects(sweepResults.at(0).numAllocated);
  oldGen_.setNumAllocatedObjects(sweepResults.at(1).numAllocated);
}

void GenGC::trackAlloc(CellKind kind, unsigned sz) {
  unsigned kindNum = static_cast<unsigned>(kind);
  allocsByCellKind_[kindNum]++;
  bytesAllocatedByCellKind_[kindNum] += sz;
}

void GenGC::trackReachable(CellKind kind, unsigned sz) {
  unsigned kindNum = static_cast<unsigned>(kind);
  fullGCReachableByCellKind_[kindNum]++;
  fullGCBytesAllocatedByCellKind_[kindNum] += sz;
}
#endif

void GenGC::resetNumFinalizedObjectsInGens() {
  youngGen_.resetNumFinalizedObjects();
  oldGen_.resetNumFinalizedObjects();
}

#ifndef NDEBUG
bool GenGC::needsWriteBarrier(void *loc, void *value) {
  char *locPtr = reinterpret_cast<char *>(loc);
  char *valPtr = reinterpret_cast<char *>(value);
  char *boundary = oldGen_.lowLim();
  return value && locPtr >= boundary && valPtr < boundary;
}
#endif

void GenGC::writeBarrierRange(HermesValue *start, uint32_t numHVs) {
  countRangeWriteBarrier();
  // For now, in this case, we'll just dirty the cards in the range.
  // We could look at the copied contents, or change the interface to
  // take the "from" range, and dirty the cards if the from range has
  // any dirty cards.  But just dirtying the cards will probably be
  // fine.
  char *locPtr = reinterpret_cast<char *>(start);
  char *boundary = oldGen_.lowLim();
  if (locPtr >= boundary) {
    oldGen_.cardTable().dirtyCardsForAddressRange(
        start, reinterpret_cast<char *>(start + numHVs) - 1);
  }
}

void GenGC::writeBarrierRangeFill(
    HermesValue *start,
    uint32_t numHVs,
    HermesValue value) {
  countRangeFillWriteBarrier();
  if (value.isPointer()) {
    char *ptr = reinterpret_cast<char *>(value.getPointer());
    char *locPtr = reinterpret_cast<char *>(start);
    char *boundary = oldGen_.lowLim();
    if (locPtr >= boundary && ptr < boundary) {
      oldGen_.cardTable().dirtyCardsForAddressRange(
          start, reinterpret_cast<char *>(start + numHVs) - 1);
    }
  }
}

void GenGC::getHeapInfo(HeapInfo &info) {
  GCBase::getHeapInfo(info);
  info.allocatedBytes = used();
  info.heapSize = size();
  info.totalAllocatedBytes = totalAllocatedBytes_ + bytesAllocatedSinceLastGC();
  info.va = maxSize();
}

void GenGC::getHeapInfoWithMallocSize(HeapInfo &info) {
  getHeapInfo(info);
  // In case the info is being re-used, ensure the count starts at 0.
  info.mallocSizeEstimate = 0;
  // First add the usage by the runtime's roots.
  info.mallocSizeEstimate += gcCallbacks_->mallocSize();
  // Then add the contributions of the two generations.
  info.mallocSizeEstimate += youngGen_.countMallocSize();
  info.mallocSizeEstimate += oldGen_.countMallocSize();

  // Now include the GC's own data structures.

  // Assume that the vector implementation doesn't use a separate bool for each
  // bool, but groups them together as bits.
  info.mallocSizeEstimate += markedSymbols_.capacity() *
      sizeof(decltype(markedSymbols_)::value_type) / 8;
  // A deque doesn't have a capacity, so the size is the lower bound.
  info.mallocSizeEstimate +=
      weakSlots_.size() * sizeof(decltype(weakSlots_)::value_type);
}

void GenGC::dump(llvm::raw_ostream &os, bool verbose) {
  GCBase::dump(os, verbose);
#ifndef NDEBUG
  // We need to do a final alloc census, to count the objects allocated since
  // the last GC.
  doAllocCensus();
  // Now we can print the census results.
  printCensusByKindStats(os);
#endif
#ifndef NDEBUG
  printExtAllocStats(os);
#endif
#ifndef NDEBUG
  os << "\nWrite barriers executed:\n"
     << "Single field: " << numWriteBarriers_ << "\n"
     << "Range: " << numRangeBarriers_ << "\n"
     << "RangeFill: " << numRangeFillBarriers_ << "\n";
#endif
}

gcheapsize_t GenGC::bytesAllocatedSinceLastGC() const {
  return youngGen_.bytesAllocatedSinceLastGC() +
      oldGen_.bytesAllocatedSinceLastGC();
}

void GenGC::updateTotalAllocStats() {
  totalAllocatedBytes_ += bytesAllocatedSinceLastGC();
}

void GenGC::createSnapshot(llvm::raw_ostream &os, bool compact) {
  // We'll say we're in GC even though we're not, to avoid assertion failures.
  GCCycle cycle{this};
#ifdef HERMES_SLOW_DEBUG
  checkWellFormedHeap();
#endif
  HeapInfo info;
  getHeapInfo(info);
  FacebookHeapSnapshot snap(os, compact, info.allocatedBytes);
  auto ptrToOffset = [this](const void *ptr) -> uint64_t {
    // This encodes all pointers as offsets from the start of the young
    // generation. This relies on the assumption that the young gen always
    // starts lower than the old gen.
    auto comparablePtr = reinterpret_cast<uintptr_t>(ptr);
    auto youngGenStart = reinterpret_cast<uintptr_t>(youngGen_.start());
    assert(
        comparablePtr >= youngGenStart &&
        "Cannot convert to offset if it's not in the old gen");
    return comparablePtr - youngGenStart;
  };
  SnapshotRootAcceptor rootSnapshotAcceptor(*this, &snap, ptrToOffset);
  SnapshotAcceptor snapshotAcceptor(*this, &snap, ptrToOffset);
  SlotVisitorWithNames<SnapshotAcceptor> visitor(snapshotAcceptor);

  snap.beginRoots();
  markRoots(rootSnapshotAcceptor, true);
  snap.endRoots();
  auto scanner =
      [ptrToOffset, &snap, &visitor, this](ContigAllocGCSpace &space) {
        for (const char *curr = space.start(); curr != space.level();) {
          const auto *cell = reinterpret_cast<const GCCell *>(curr);
          FacebookHeapSnapshot::Object obj(
              ptrToOffset(cell), cell->getKind(), cell->getAllocatedSize());
          // If the cell is a string, add a value to be printed.
          // TODO: add other special types here.
          if (const StringPrimitive *str = dyn_vmcast<StringPrimitive>(cell)) {
            snap.startObjectWithValue(std::move(obj), str);
          } else {
            snap.startObject(std::move(obj));
          }
          GCBase::markCellWithNames(
              visitor, const_cast<GCCell *>(cell), cell->getVT(), this);
          snap.endObject();
          curr += cell->getAllocatedSize();
        }
      };

  snap.beginRefs();
  scanner(youngGen_);
  scanner(oldGen_);
  snap.endRefs();

  snap.beginIdTable();
  gcCallbacks_->visitIdentifiers([&snap](UTF16Ref entry, uint32_t id) {
    snap.addIdTableEntry(entry, id);
  });
  snap.endIdTable();

#ifdef HERMES_SLOW_DEBUG
  checkWellFormedHeap();
#endif
}

void GenGC::printStats(llvm::raw_ostream &os, bool trailingComma) {
  if (!recordGcStats_) {
    return;
  }
  GCBase::printStats(os, true);
  os << "\t\"specific\": {\n"
     << "\t\t\"collector\": \"generational\",\n"
     << "\t\t\"stats\": {\n"
     << "\t\t\t\"ygNumCollections\": "
     << youngGenCollectionCumStats_.numCollections << ",\n"
     << "\t\t\t\"ygTotalGCTime\": "
     << formatSecs(youngGenCollectionCumStats_.gcWallTime.sum()).secs << ",\n"
     << "\t\t\t\"ygAvgGCPause\": "
     << formatSecs(youngGenCollectionCumStats_.gcWallTime.average()).secs
     << ",\n"
     << "\t\t\t\"ygMaxGCPause\": "
     << formatSecs(youngGenCollectionCumStats_.gcWallTime.max()).secs << ",\n"
     << "\t\t\t\"ygTotalGCCPUTime\": "
     << formatSecs(youngGenCollectionCumStats_.gcCPUTime.sum()).secs << ",\n"
     << "\t\t\t\"ygAvgGCCPUPause\": "
     << formatSecs(youngGenCollectionCumStats_.gcCPUTime.average()).secs
     << ",\n"
     << "\t\t\t\"ygMaxGCCPUPause\": "
     << formatSecs(youngGenCollectionCumStats_.gcCPUTime.max()).secs << ",\n"
     << "\t\t\t\"ygFinalSize\": "
     << formatSize(youngGenCollectionCumStats_.finalHeapSize).bytes << ",\n";

  youngGen_.printStats(os, /*trailingComma*/ true);

  os << "\t\t\t\"fullNumCollections\": "
     << fullCollectionCumStats_.numCollections << ",\n"
     << "\t\t\t\"fullTotalGCTime\": "
     << formatSecs(fullCollectionCumStats_.gcWallTime.sum()).secs << ",\n"
     << "\t\t\t\"fullAvgGCPause\": "
     << formatSecs(fullCollectionCumStats_.gcWallTime.average()).secs << ",\n"
     << "\t\t\t\"fullMaxGCPause\": "
     << formatSecs(fullCollectionCumStats_.gcWallTime.max()).secs << ",\n"
     << "\t\t\t\"fullTotalGCCPUTime\": "
     << formatSecs(fullCollectionCumStats_.gcCPUTime.sum()).secs << ",\n"
     << "\t\t\t\"fullAvgGCCPUPause\": "
     << formatSecs(fullCollectionCumStats_.gcCPUTime.average()).secs << ",\n"
     << "\t\t\t\"fullMaxGCCPUPause\": "
     << formatSecs(fullCollectionCumStats_.gcCPUTime.max()).secs << ",\n"
     << "\t\t\t\"fullFinalSize\": "
     << formatSize(fullCollectionCumStats_.finalHeapSize).bytes << ",\n";

  printFullCollectionStats(os, /*trailingComma*/ false);
  os << "\t\t}\n"
     << "\t},\n";
  gcCallbacks_->printRuntimeGCStats(os);
  if (trailingComma) {
    os << ",";
  }
  os << "\n";
}

void GenGC::printFullCollectionStats(llvm::raw_ostream &os, bool trailingComma)
    const {
  double fullSurvivalPct = 0.0;
  if (cumPreBytes_ > 0) {
    fullSurvivalPct = 100.0 * static_cast<double>(cumPostBytes_) /
        static_cast<double>(cumPreBytes_);
  }

  os << "\t\t\t\"fullMarkRootsTime\": " << markRootsSecs_ << ",\n"
     << "\t\t\t\"fullMarkTransitiveTime\": " << markTransitiveSecs_ << ",\n"
     << "\t\t\t\"fullSweepTime\": " << sweepSecs_ << ",\n"
     << "\t\t\t\"fullUpdateRefsTime\": " << updateReferencesSecs_ << ",\n"
     << "\t\t\t\"fullCompactTime\": " << compactSecs_ << ",\n"
     << "\t\t\t\"fullSurvivalPct\": " << fullSurvivalPct;

  if (trailingComma) {
    os << ",";
  }
  os << "\n";
}

void GenGC::recordGenLevelsAtEndOfLastGC() {
  youngGen_.recordLevelAtEndOfLastGC();
  oldGen_.recordLevelAtEndOfLastGC();
}

#ifndef NDEBUG
void GenGC::doAllocCensus() {
  if (!recordGcStats_) {
    return;
  }
  GC *gc = this;
  auto callback = [gc](GCCell *cell) {
    gc->trackAlloc(cell->getKind(), cell->getAllocatedSize());
  };
  youngGen_.forObjsAllocatedSinceGC(callback);
  oldGen_.forObjsAllocatedSinceGC(callback);
}

void GenGC::printCensusByKindStats(llvm::raw_ostream &os) const {
  printCensusByKindStatsWork(
      os,
      "Allocs by cell kind, ranked by #allocs.",
      &allocsByCellKind_[0],
      &bytesAllocatedByCellKind_[0]);
  printCensusByKindStatsWork(
      os,
      "Objects reachable in full GCs, by cell kind, ranked by #allocs.",
      &fullGCReachableByCellKind_[0],
      &fullGCBytesAllocatedByCellKind_[0]);
}

void GenGC::printCensusByKindStatsWork(
    llvm::raw_ostream &os,
    const char *msg,
    const uint64_t *allocs,
    const uint64_t *bytes) const {
  // This is <cell-kind, allocs, bytes-allocated>.
  using Elem = std::tuple<unsigned, uint64_t, uint64_t>;
  std::vector<Elem> data;
  for (unsigned i = 0; i < kNKinds; i++) {
    data.push_back(std::make_tuple(i, allocs[i], bytes[i]));
  }
  sort(data.begin(), data.end(), [](const Elem &e0, const Elem &e1) {
    return std::get<1>(e0) > std::get<1>(e1);
  });
  uint64_t totAllocs = 0;
  uint64_t totBytes = 0;
  for (const Elem &elem : data) {
    totAllocs += std::get<1>(elem);
    totBytes += std::get<2>(elem);
  }

  // If there were no allocations, don't print the table.
  if (totAllocs == 0) {
    os << "\n"
       << msg << "\n\n"
       << "No objects.\n";
    return;
  }
  os << "\n"
     << msg << "\n\n"
     << "Total objs: " << llvm::format("%14lld", totAllocs)
     << ", bytes = " << llvm::format("%14lld", totBytes) << ".\n\n"
     << llvm::format(
            "%30s%14s%8s%14s%8s\n",
            static_cast<const char *>("kind"),
            static_cast<const char *>("objects"),
            static_cast<const char *>("(%)"),
            static_cast<const char *>("bytes"),
            static_cast<const char *>("(%)"));
  os << "----------------------------------------------------------------------"
        "----\n";

  double totAllocsD = totAllocs;
  double totBytesD = totBytes;
  for (const Elem &elem : data) {
    // If there were no instances of a cell kind, omit the line.
    if (std::get<1>(elem) == 0) {
      assert(std::get<2>(elem) == 0);
      continue;
    }
    os << llvm::format(
        "%30s%14lld%7.2f%%%14lld%7.2f%%\n",
        cellKindStr(static_cast<CellKind>(std::get<0>(elem))),
        std::get<1>(elem),
        100.0 * static_cast<double>(std::get<1>(elem)) / totAllocsD,
        std::get<2>(elem),
        100.0 * static_cast<double>(std::get<2>(elem)) / totBytesD);
  }
}
#endif

} // namespace vm
} // namespace hermes
