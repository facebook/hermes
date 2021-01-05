/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Note: this must include GC.h, instead of GenGC.h, because GenGC.h assumes
// it is included only by GC.h.  (For example, it assumes GCBase is declared.)
#include "hermes/VM/GC.h"

#include "hermes/Platform/Logging.h"
#include "hermes/Support/DebugHelpers.h"
#include "hermes/Support/JSONEmitter.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/Support/PerfSection.h"
#include "hermes/VM/AllocSource.h"
#include "hermes/VM/Casting.h"
#include "hermes/VM/CheckHeapWellFormedAcceptor.h"
#include "hermes/VM/CompactionResult-inline.h"
#include "hermes/VM/CompleteMarkState-inline.h"
#include "hermes/VM/Deserializer.h"
#include "hermes/VM/ExpectedPageSize.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HeapSnapshot.h"
#include "hermes/VM/HermesValue-inline.h"
#include "hermes/VM/JSWeakMapImpl.h"
#include "hermes/VM/Serializer.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/SweepResultNC.h"
#include "hermes/VM/SymbolID.h"

#ifdef HERMES_SLOW_DEBUG
#include "llvh/ADT/DenseSet.h"
#endif
#include "llvh/Support/Debug.h"
#include "llvh/Support/Format.h"
#include "llvh/Support/MathExtras.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cinttypes>
#include <clocale>
#include <cstdint>
#include <tuple>
#include <utility>

#define DEBUG_TYPE "gc"

using llvh::dbgs;
using std::chrono::steady_clock;

namespace hermes {
namespace vm {

static const char *kGCName = "gengc";

GenGC::Size::Size(const GCConfig &gcConfig)
    : Size(gcConfig.getMinHeapSize(), gcConfig.getMaxHeapSize()) {}

GenGC::Size::Size(gcheapsize_t min, gcheapsize_t max)
    : ygs_(min / kYoungGenFractionDenom, max / kYoungGenFractionDenom),
      ogs_(clampDiffNonNeg(min, ygs_.min()), clampDiffNonNeg(max, ygs_.max())) {
}

gcheapsize_t GenGC::Size::storageFootprint() const {
  return ogs_.storageFootprint() + ygs_.storageFootprint();
}

gcheapsize_t GenGC::Size::minStorageFootprint() const {
  return ogs_.minStorageFootprint() + ygs_.minStorageFootprint();
}

llvh::ErrorOr<size_t> GenGC::getVMFootprintForTest() const {
  size_t footprint = 0;
  for (const auto &seg : segmentIndex_) {
    auto segFootprint =
        hermes::oscompat::vm_footprint(seg->start(), seg->hiLim());
    if (!segFootprint)
      return segFootprint;
    footprint += *segFootprint;
  }
  return footprint;
}

std::pair<gcheapsize_t, gcheapsize_t> GenGC::Size::adjustSize(
    gcheapsize_t desired) const {
  const gcheapsize_t ygSize = ygs_.adjustSize(desired / kYoungGenFractionDenom);
  return std::make_pair(
      ygSize, ogs_.adjustSize(std::max(desired, ygSize) - ygSize));
}

GenGC::GenGC(
    MetadataTable metaTable,
    GCCallbacks *gcCallbacks,
    PointerBase *pointerBase,
    const GCConfig &gcConfig,
    std::shared_ptr<CrashManager> crashMgr,
    std::shared_ptr<StorageProvider> provider)
    : GCBase(
          metaTable,
          gcCallbacks,
          pointerBase,
          gcConfig,
          std::move(crashMgr)),
      storageProvider_(std::move(provider)),
      generationSizes_(Size(gcConfig)),
      // ExpectedPageSize.h defines a static value for the expected page size,
      // which we use in sizing and aligning the metadata components
      // of segments. Only do memory protection of those components if
      // the expected value is accurate wrt the dynamic value.
      doMetadataProtection_(
          gcConfig.getProtectMetadata() && pagesize::expectedPageSizeIsSafe()),
      youngGen_(
          this,
          generationSizes_.youngGenSize(),
          &oldGen_,
          gcConfig.getShouldReleaseUnused()),
      oldGen_(
          this,
          generationSizes_.oldGenSize(),
          gcConfig.getShouldReleaseUnused()),
      allocContextFromYG_(gcConfig.getAllocInYoung()),
      revertToYGAtTTI_(gcConfig.getRevertToYGAtTTI()),
      occupancyTarget_(gcConfig.getOccupancyTarget()),
      oomThreshold_(gcConfig.getEffectiveOOMThreshold()),
      weightedUsed_(static_cast<double>(gcConfig.getInitHeapSize())) {
  crashMgr_->setCustomData("HermesGC", kGCName);
  growTo(gcConfig.getInitHeapSize());
  claimAllocContext();
  updateCrashManagerHeapExtents();
}

GenGC::~GenGC() {
#ifdef HERMES_SLOW_DEBUG
  // Verify that the efficient method of counting allocated bytes
  // (updating at YG collections) matches a dead-simple per-allocation
  // counter.
  HeapInfo info;
  getHeapInfo(info);
  assert(info.totalAllocatedBytes == totalAllocatedBytesDebug_);
#endif
}

#ifndef NDEBUG
AllocResult
GenGC::debugAlloc(uint32_t sz, HasFinalizer hasFinalizer, bool fixedSize) {
  assert(noAllocLevel_ == 0 && "no alloc allowed right now");
  AllocResult res;
  // Only variable-sized objects may be allocated in both generations.
  if (shouldRandomizeAllocSpace()) {
    res = debugAllocRandomize(sz, hasFinalizer, fixedSize);
  } else {
    // First try in the claimed allocation context.
    res = allocContext_.alloc(sz, hasFinalizer);
    if (LLVM_LIKELY(res.success)) {
      return res;
    }
    // If that fails, yield the active segment, and try allocating from
    // the proper generation.
    res = {allocSlow(sz, fixedSize, hasFinalizer), true};
  }
  return res;
}

AllocResult GenGC::debugAllocRandomize(
    uint32_t sz,
    HasFinalizer hasFinalizer,
    bool fixedSize) {
  AllocResult res;
  if (fixedSize) {
    res = youngGen_.alloc(sz, hasFinalizer);
  } else {
    if (allocInYoung_) {
      res = youngGen_.alloc(sz, hasFinalizer);
    } else {
      res = oldGen_.alloc(sz, hasFinalizer);
    }
    allocInYoung_ = !allocInYoung_;
  }
  return res;
}
#endif

bool GenGC::canAllocExternalMemory(uint32_t size) {
  const size_t heapMaxSize = youngGen_.maxSize() + oldGen_.maxSize();
  return size <= heapMaxSize;
}

void GenGC::creditExternalMemory(GCCell *alloc, uint32_t size) {
  assert(canAllocExternalMemory(size) && "Precondition");
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
void GenGC::printExtAllocStats(llvh::raw_ostream &os) {
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
  AllocContextYieldThenClaim yielder(this);
  // Must clear the mark bits, so all objects are considered unreachable.
  clearMarkBits();
  finalizeUnreachableObjects();
}

namespace {

template <typename T>
using uintegral = typename std::enable_if<
    std::is_integral<T>::value && std::is_unsigned<T>::value,
    T>::type;

/// If the number of binary digits Tdig in T is greater than the number of
/// mantissa digits in a double (Ddig), returns the difference Tdig - Ddig.
/// Otherwise return 0.
/// TODO (T31421960): if/when we upgrade to C++14, we could use std::max for
/// this instead of a ternary expression.
template <typename T>
constexpr int doubleDigitsDiff() {
  return std::numeric_limits<T>::digits > std::numeric_limits<double>::digits
      ? std::numeric_limits<T>::digits - std::numeric_limits<double>::digits
      : 0;
}

/// Returns the largest value of the given unsigned integral type that is
/// guaranteed to be precisely representable as a double: it can be cast to a
/// double, and back to the type, and yield the original value exactly.
template <typename T>
constexpr uintegral<T> doublePreciseMax() {
  return (std::numeric_limits<T>::max() >> doubleDigitsDiff<T>())
      << doubleDigitsDiff<T>();
}

/// Verify that doublePreciseMax() works correctly for unsigned types we care
/// about.
static_assert(
    static_cast<uint64_t>(static_cast<double>(doublePreciseMax<uint64_t>())) ==
        doublePreciseMax<uint64_t>(),
    "doublePreciseMax should be exactly representable as a double.");
static_assert(
    static_cast<uint32_t>(static_cast<double>(doublePreciseMax<uint32_t>())) ==
        doublePreciseMax<uint32_t>(),
    "doublePreciseMax should be exactly representable as a double.");

} // namespace

gcheapsize_t GenGC::usedToDesiredSize(size_t usedBytes) {
  double desiredSize = static_cast<double>(usedBytes) / occupancyTarget();
  // If occupancyTarget is small (close to 0), and/or usedBytes is large,
  // desiredSize could be too large to cast back into gcheapsize_t, below.
  // Therefore, we take the min of this with the the "double precise maximum
  // value" of gcheapsize_t.  For a given unsigned integral type, this is the
  // largest value that is guaranteed to be precisely representable as a double:
  // it can be cast to a double, and back to the type, and yield the original
  // value exactly.  If gcheapsize_t is uint32_t, the max is representable as a
  // double with no loss of precision, but if gcheapsize_t is uint64_t, this has
  // more bits of precision than the mantissa of a double, so a smaller value
  // (with trailing zeros) must be used.
  desiredSize = std::min(
      desiredSize, static_cast<double>(doublePreciseMax<gcheapsize_t>()));
  return static_cast<gcheapsize_t>(desiredSize);
}

void GenGC::ttiReached() {
  // If we started allocating in the OG, switch back to allocating in the YG.
  if (!allocContextFromYG_ && revertToYGAtTTI_) {
    // First, yield the allocation context, so the heap is well-formed.
    yieldAllocContext();
    // If we were doing direct OG allocation, the card object boundaries
    // will not be valid.  Recreate it.
    oldGen_.recreateCardTableBoundaries();
    // Now switch to doing YG alloc, and claim the alloc context from the YG.
    allocContextFromYG_ = true;
    claimAllocContext();
  }
}

void GenGC::collect(std::string cause, bool canEffectiveOOM) {
  assert(noAllocLevel_ == 0 && "no GC allowed right now");
  if (canEffectiveOOM && ++consecFullGCs_ >= oomThreshold_)
    oom(make_error_code(OOMError::Effective));

  /// Yield, then reclaim, the allocation context.  (This is a noop
  /// if the context has already been yielded.)
  AllocContextYieldThenClaim yielder(this);

  // Make sure the AllocContext been yielded back to its owner.
  assert(!allocContext_.activeSegment);

#ifdef HERMES_EXTRA_DEBUG
  /// Check the old-gen vtable summary, to detect possible mutator writes.
  /// TODO(T56364255): remove these when the problem is diagnosed.
  oldGen_.checkSummarizedOldGenVTables(fullCollectionCumStats_.numCollections);
#endif

#ifdef HERMES_EXTRA_DEBUG
  /// Unprotect the card table boundary table, so we can update it.
  /// TODO(T48709128): remove these when the problem is diagnosed.
  oldGen_.unprotectCardTableBoundaries();
#endif

  const size_t usedBefore = used();
  const size_t sizeBefore = size();
  cumPreBytes_ += used();
  const size_t ygUsedBefore = youngGen_.used();
  const size_t ogUsedBefore = oldGen_.used();

  // To be filled in after collection has happened.
  size_t usedAfter;
  size_t sizeAfter;

#ifndef NDEBUG
  resetNumReachableObjectsInGens();
  resetNumAllHiddenClassesInGens();
  numMarkedSymbols_ = 0;
#endif

  LLVM_DEBUG(
      dbgs() << "\nStarting (full, young=" << formatSize(youngGen_.sizeDirect())
             << "; old=" << formatSize(oldGen_.size())
             << ") garbage collection # " << numGCs() << "\n");

  {
    CollectionSection fullCollection(
        this, "Full collection", std::move(cause), gcCallbacks_);

    fullCollection.addArg("fullGCUsedBefore", usedBefore);
    fullCollection.addArg("fullGCSizeBefore", sizeBefore);

    markPhase();

    finalizeUnreachableObjects();

    auto ygExtMem = youngGen_.externalMemory();
    auto ogExtMem = oldGen_.externalMemory();

    // TODO (T37170733) Treat external charge like any other allocation.
    // Remove external charge for the duration of collection.
    youngGen_.debitExternalMemory(ygExtMem);
    oldGen_.debitExternalMemory(ogExtMem);

    // The sweep results for the generations, to be filled in.
    SweepResult sweepResult({oldGen_.allSegments(), youngGen_.allSegments()});

    sweepAndInstallForwardingPointers(&sweepResult);
    updateReferences(sweepResult);

    // Re-instate the external charge.
    youngGen_.creditExternalMemory(ygExtMem);
    oldGen_.creditExternalMemory(ogExtMem);

    compact(sweepResult);

    oldGen_.updateCardTablesAfterCompaction(
        /* youngGenIsEmpty */ youngGen_.usedDirect() == 0);

    gcCallbacks_->freeSymbols(markedSymbols_);

    // Update the exponential weighted average of live size, which we'll
    // consult if we need to shrink the heap.
    updateWeightedUsed();

    updateHeapSize();

    // In case we started in direct OG allocation, we want to revert to YG alloc
    // if we reach a full collection.  (Usually, a TTI call will have already
    // done this; this is just a backstop.)
    if (!allocContextFromYG_ && revertToYGAtTTI_) {
      allocContextFromYG_ = true;
    }

    usedAfter = usedDirect();
    const size_t usedAfterYG = youngGen_.usedDirect();
    sizeAfter = sizeDirect();
    cumPostBytes_ += usedAfter;

    // Update the statistics.
    const unsigned numAllocatedObjectsBefore =
        recordStatsNC(sweepResult.compactionResult);

    fullCollection.recordGCStats(
        sizeAfter,
        usedBefore,
        sizeBefore,
        usedAfter,
        sizeAfter,
        &fullCollectionCumStats_);

    fullCollection.addArg("fullGCUsedAfter", usedAfter);
    fullCollection.addArg("fullGCUsedAfterYG", usedAfterYG);
    fullCollection.addArg("fullGCSizeAfter", sizeAfter);
    fullCollection.addArg("fullGCNum", fullCollectionCumStats_.numCollections);

    checkInvariants(numAllocatedObjectsBefore, usedBefore);

    execTrace_.addFullGC(
        ygUsedBefore, youngGen_.used(), ogUsedBefore, oldGen_.used());
  }

  /// Update the heap's segments extents in the crash manager data.
  updateCrashManagerHeapExtents();

  checkTripwire(usedAfter);
#ifdef HERMESVM_SIZE_DIAGNOSTIC
  sizeDiagnosticCensus();
#endif

#ifdef HERMES_EXTRA_DEBUG
  /// Summarize the old gen vtables to detect possible mutator writes.
  /// TODO(T56364255): remove these when the problem is diagnosed.
  oldGen_.summarizeOldGenVTables();
#endif

#ifdef HERMES_EXTRA_DEBUG
  /// Protect the card table boundary table, to detect corrupting mutator
  /// writes.
  /// TODO(T48709128): remove these when the problem is diagnosed.
  oldGen_.protectCardTableBoundaries();
#endif
}

unsigned GenGC::recordStatsNC(const CompactionResult &compactionResult) {
#ifndef NDEBUG
  // Report current status to the GCBase variables.
  recordNumReachableObjects(computeNumReachableObjects());
  recordNumHiddenClasses(
      computeNumHiddenClasses(), computeNumLeafHiddenClasses());
  unsigned numAllocatedObjectsBefore = computeNumAllocatedObjects();
  recordNumCollectedObjects(numAllocatedObjectsBefore - numReachableObjects_);
  recordNumFinalizedObjects(computeNumFinalizedObjects());

  // Reset the per-generation allocated objects to those found reachable.
  setNumAllocatedObjectsAfterFullCollectionNC(compactionResult);
  return numAllocatedObjectsBefore;
#else
  return 0;
#endif
}

void GenGC::checkInvariants(
    unsigned numAllocatedObjectsBefore,
    size_t usedBefore) {
#ifndef NDEBUG
  size_t usedAfter = usedDirect();
  // A full GC should never increase used memory.
  assert(usedAfter <= usedBefore);
  // Check that getHeapInfo returns a well-formed info structure.
  DebugHeapInfo info;
  getDebugHeapInfo(info);
  info.assertInvariants();
  // Assert an additional invariant involving the number of allocated
  // objects before collection.
  assert(
      numAllocatedObjectsBefore - info.numReachableObjects ==
          info.numCollectedObjects &&
      "collected objects computed incorrectly");
#endif // !NDEBUG
}

#ifndef NDEBUG
bool GenGC::dbgContains(const void *ptr) const {
  GenGCHeapSegment *segment = segmentIndex_.segmentCovering(ptr);
  assert(!segment || segment->contains(ptr));
  return segment;
}

bool GenGC::validPointer(const void *ptr) const {
  GenGCHeapSegment *segment = segmentIndex_.segmentCovering(ptr);
  return segment && segment->validPointer(ptr);
}

bool GenGC::isMostRecentFinalizableObj(const GCCell *cell) const {
  if (GCBase::isMostRecentCellInFinalizerVector(
          allocContext_.cellsWithFinalizers, cell)) {
    return true;
  } else {
    // If we're allocating in the young gen, it might have been directly
    // allocated in the old gen.
    return allocContextFromYG_ &&
        GCBase::isMostRecentCellInFinalizerVector(
               oldGen_.allocContext().cellsWithFinalizers, cell);
  }
}
#endif

size_t GenGC::size() const {
  return youngGen_.size() + oldGen_.size();
}
size_t GenGC::sizeDirect() const {
  return youngGen_.sizeDirect() + oldGen_.size();
}

size_t GenGC::used() const {
  return youngGen_.used() + oldGen_.used();
}
size_t GenGC::usedDirect() const {
  return youngGen_.usedDirect() + oldGen_.used();
}

#ifdef HERMES_SLOW_DEBUG
/// This is used to detect whether the Runtime::markRoots ever invokes
/// the acceptor on the same root location more than once, which is illegal.
struct FullMSCDuplicateRootsDetectorAcceptor final
    : public RootAndSlotAcceptorDefault {
  llvh::DenseSet<void *> markedLocs_;

  using RootAndSlotAcceptorDefault::accept;
  using RootAndSlotAcceptorDefault::RootAndSlotAcceptorDefault;

  void accept(void *&ptr) override {
    assert(markedLocs_.count(&ptr) == 0);
    markedLocs_.insert(&ptr);
  }
  void acceptHV(HermesValue &hv) override {
    assert(markedLocs_.count(&hv) == 0);
    markedLocs_.insert(&hv);
  }
};
#endif

void GenGC::markPhase() {
  struct FullMSCMarkInitialAcceptor final : public RootAndSlotAcceptorDefault {
    using RootAndSlotAcceptorDefault::accept;
    using RootAndSlotAcceptorDefault::RootAndSlotAcceptorDefault;
    void accept(void *&ptr) override {
      if (ptr) {
        assert(gc.dbgContains(ptr));
        GCCell *cell = reinterpret_cast<GCCell *>(ptr);
#ifdef HERMES_EXTRA_DEBUG
        if (!cell->isValid()) {
          hermes_fatal("HermesGC: marking pointer to invalid object.");
        }
#endif
        GenGCHeapSegment::setCellMarkBit(cell);
      }
    }
    void acceptHV(HermesValue &hv) override {
      if (hv.isPointer()) {
        void *ptr = hv.getPointer();
        if (ptr) {
          assert(gc.dbgContains(ptr));
          GCCell *cell = reinterpret_cast<GCCell *>(ptr);
#ifdef HERMES_EXTRA_DEBUG
          if (!cell->isValid()) {
            hermes_fatal("HermesGC: marking pointer to invalid object.");
          }
#endif
          GenGCHeapSegment::setCellMarkBit(cell);
        }
      } else if (hv.isSymbol()) {
        gc.markSymbol(hv.getSymbol());
      }
    }
    void acceptSym(SymbolID sym) override {
      gc.markSymbol(sym);
    }
  };
  markedSymbols_.clear();
  markedSymbols_.resize(gcCallbacks_->getSymbolsEnd(), false);

  FullMSCMarkInitialAcceptor acceptor(*this);
  DroppingAcceptor<FullMSCMarkInitialAcceptor> nameAcceptor{acceptor};
  clearMarkBits();

#ifdef HERMES_SLOW_DEBUG
  {
    // Weak ref slots should have been unmarked at end of previous collection.
    for (auto slot : weakSlots_)
      assert(slot.state() != WeakSlotState::Marked);

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

  // Clear property maps of hidden classes, to reduce memory usage. But exclude
  // any classes that were marked as roots above, to simplify the Handle-based
  // implementation of HiddenClass.
  youngGen_.clearUnmarkedPropertyMaps();
  oldGen_.clearUnmarkedPropertyMaps();

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

void GenGC::clearMarkBits() {
  for (auto segment : segmentIndex_) {
    segment->markBitArray().clear();
  }
}

void GenGC::completeMarking() {
  // completeMarking returns a boolean that is true if and only if the mark
  // stack overflowed whilst trying to complete marking.  When this happens, we
  // must restart marking from the beginning (in increasing order of virtual
  // memory addresses).  Whilst this can happen several times, the overall
  // marking process is guaranteed to terminate.
  do {
    markState_.markStackOverflow_ = false;
    for (auto *segment : segmentIndex_) {
      segment->completeMarking(this, &markState_);
      if (markState_.markStackOverflow_)
        break;
    }
    // The marking loop above will have accumulated WeakMaps;
    // find things reachable from values of reachable keys.
    // (Note that this can also set markStackOverflow_).
    completeWeakMapMarking();
  } while (markState_.markStackOverflow_);
}

void GenGC::completeWeakMapMarking() {
  CompleteMarkState::FullMSCMarkTransitiveAcceptor acceptor(*this, &markState_);

  // Set the currentParPointer to a maximal value, so all pointers scanned
  // will be pushed on the mark stack.
  markState_.currentParPointer =
      reinterpret_cast<GCCell *>(static_cast<intptr_t>(-1));

  // GCBase::completeWeakMapMarking returns the total size of the reachable
  // WeakMaps, but GenGC computes allocatedBytes in a different way, so we don't
  // use this result.
  (void)GCBase::completeWeakMapMarking(
      this,
      acceptor,
      markState_.reachableWeakMaps_,
      /*objIsMarked*/ GenGCHeapSegment::getCellMarkBit,
      /*checkValIsMarked*/
      [this, &acceptor](GCCell *valCell, GCHermesValue &valRef) {
        if (!GenGCHeapSegment::getCellMarkBit(valCell)) {
          GenGCHeapSegment::setCellMarkBit(valCell);
          markState_.pushCell(valCell);
          markState_.drainMarkStack(this, acceptor);
          return true;
        }
        return false;
      },
      /*drainMarkStack*/
      [this](CompleteMarkState::FullMSCMarkTransitiveAcceptor &acceptor) {
        markState_.drainMarkStack(this, acceptor);
      },
      /*checkMarkStackOverflow*/
      [this]() { return markState_.markStackOverflow_; });

  markState_.currentParPointer = nullptr;
  markState_.reachableWeakMaps_.clear();
}

void GenGC::finalizeUnreachableObjects() {
  youngGen_.finalizeUnreachableObjects();
  oldGen_.finalizeUnreachableObjects();
}

void GenGC::sweepAndInstallForwardingPointers(SweepResult *sweepResult) {
  // TODO (T26749007): use the segment API to iterate over segments in each
  // generation.
  auto sweepStart = steady_clock::now();

  oldGen_.sweepAndInstallForwardingPointers(this, sweepResult);
  youngGen_.sweepAndInstallForwardingPointers(this, sweepResult);

  sweepSecs_ += GCBase::clockDiffSeconds(sweepStart, steady_clock::now());
}

void GenGC::updateReferences(const SweepResult &sweepResult) {
  auto updateRefsStart = steady_clock::now();
  PerfSection fullGCUpdateReferencesSystraceRegion("fullGCUpdateReferences");
  std::unique_ptr<FullMSCUpdateAcceptor> acceptor =
      getFullMSCUpdateAcceptor(*this);
  DroppingAcceptor<RootAndSlotAcceptor> nameAcceptor{*acceptor};
  markRoots(nameAcceptor, /*markLongLived*/ true);
  markWeakRoots(*acceptor);

  SweepResult::VTablesRemaining vTables(
      sweepResult.displacedVtablePtrs.begin(),
      sweepResult.displacedVtablePtrs.end());

  // We swept the old gen into itself before sweeping the young gen.  We must
  // preserve this order here, to match up cells with their displaced VTable
  // pointers.
  oldGen_.updateReferences(this, vTables);
  youngGen_.updateReferences(this, vTables);

  updateWeakReferences(/*fullGC*/ true);
  updateReferencesSecs_ +=
      GCBase::clockDiffSeconds(updateRefsStart, steady_clock::now());
  unmarkWeakReferences();
}

void GenGC::compact(const SweepResult &sweepResult) {
  auto compactStart = steady_clock::now();
  PerfSection fullGCCompactSystraceRegion("fullGCCompact");

  auto &compactionResult = sweepResult.compactionResult;

  SweepResult::VTablesRemaining vTables(
      sweepResult.displacedVtablePtrs.begin(),
      sweepResult.displacedVtablePtrs.end());

  CompactionResult::ChunksRemaining chunks(
      compactionResult.usedChunks().begin(),
      compactionResult.usedChunks().end());

  // We swept the old gen into itself before sweeping the young gen.  We must
  // preserve this order here, so that we re-associate the correct VTable
  // pointers, and match up the chunks we used with the segments they were
  // created from.
  auto doCompaction = [&vTables](GenGCHeapSegment &segment) {
    segment.compact(vTables);
  };

  oldGen_.forUsedSegments(doCompaction);
  youngGen_.forUsedSegments(doCompaction);

  oldGen_.recordLevelAfterCompaction(chunks);
  youngGen_.recordLevelAfterCompaction(chunks);

  assert(!vTables.hasNext() && "Not all vtable pointers replaced.");
  assert(!chunks.hasNext() && "Not all chunks written back to their segments.");

  youngGen_.compactFinalizableObjectList();

  assert(youngGen_.extSizeFromFinalizerList() == youngGen_.externalMemory());
  assert(oldGen_.extSizeFromFinalizerList() == oldGen_.externalMemory());

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

#ifdef HERMES_SLOW_DEBUG
void GenGC::checkWellFormedHeap() const {
  segmentIndex_.checkConsistency();

  // OK to cast away const here, because in VerifyRoots mode,
  // markRoots has no side effects.
  GenGC *gc = const_cast<GenGC *>(this);
  CheckHeapWellFormedAcceptor acceptor(*gc);
  DroppingAcceptor<CheckHeapWellFormedAcceptor> nameAcceptor{acceptor};
  gc->markRoots(nameAcceptor, /*markLongLived*/ true);
  gc->markWeakRoots(acceptor);
  youngGen_.checkWellFormed(gc);
  oldGen_.checkWellFormed(gc);
}
#endif

void GenGC::segmentMoved(GenGCHeapSegment *segment) {
  segmentIndex_.update(segment);
}

void GenGC::forgetSegments(const std::vector<const char *> &lowLims) {
  segmentIndex_.remove(lowLims.begin(), lowLims.end());
}

void GenGC::moveWeakReferences(ptrdiff_t delta) {
  for (auto &slot : weakSlots_) {
    if (slot.state() == WeakSlotState::Free || !slot.hasPointer())
      continue;

    auto cell = reinterpret_cast<char *>(slot.getPointer());
    slot.setPointer(cell + delta);
  }
}

size_t GenGC::youngGenSize(size_t totalHeapSize) const {
  return youngGen_.adjustSize(totalHeapSize / kYoungGenFractionDenom);
}

void GenGC::growTo(size_t hint) {
  // The generations' sizes should be monotonic with respect to the hint.  The
  // Young Generation satisfies this because youngGenSize is monotonic.  The Old
  // Generation satisfies this because it aligns up to a multiple of the Young
  // Generation's alignment boundary.
  const auto sizes = generationSizes_.adjustSize(hint);

  const auto ygSize = sizes.first;
  // Ensure old gen fits.
  const auto ogSize = oldGen_.adjustSize(sizes.second);

  youngGen_.growTo(ygSize);
  oldGen_.growTo(ogSize);
}

void GenGC::shrinkTo(size_t hint) {
  const auto sizes = generationSizes_.adjustSize(hint);
  const auto ygSize = sizes.first;
  // This should only be called when this assertion is guaranteed: for example,
  // when the young gen is empty.
  assert(youngGen_.usedDirect() <= ygSize);
  // The minimim size we can shrink to allows a young generation collection.
  const gcheapsize_t minHeapSize = usedDirect();
  auto ogSize = oldGen_.adjustSize(std::max(sizes.second, minHeapSize));

  youngGen_.shrinkTo(ygSize);
  oldGen_.shrinkTo(ogSize);
}

void GenGC::unmarkWeakReferences() {
  for (auto &slot : weakSlots_) {
    if (slot.state() == WeakSlotState::Marked) {
      slot.unmark();
    }
  }
}

void GenGC::updateWeakReference(WeakRefSlot *slotPtr, bool fullGC) {
  // Skip free slots.
  if (slotPtr->state() == WeakSlotState::Free) {
    return;
  }

  // TODO: re-enable this for young-gen collection.  See T21007593.
  if (fullGC) {
    // A slot which is no longer reachable. Add it to the free list.
    if (slotPtr->state() == WeakSlotState::Unmarked) {
      freeWeakSlot(slotPtr);
      return;
    }

    assert(
        slotPtr->state() == WeakSlotState::Marked &&
        "invalid marked slot state");
  }

  // Skip empty slots.
  if (!slotPtr->hasPointer()) {
    return;
  }

  GCCell *cell = (GCCell *)slotPtr->getPointer();

  if (fullGC) {
    if (GenGCHeapSegment::getCellMarkBit(cell)) {
      slotPtr->setPointer(cell->getForwardingPointer());
    } else {
      slotPtr->clearPointer();
    }
  } else {
    // Young-gen collection.  If the cell is in the young gen, see if
    // it survived collection.  If so, update the slot.
    if (youngGen_.contains(cell)) {
      if (cell->hasMarkedForwardingPointer()) {
        slotPtr->setPointer(cell->getMarkedForwardingPointer());
      } else {
        slotPtr->clearPointer();
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
    if (slotPtr->state() != WeakSlotState::Free && slotPtr->hasPointer()) {
      if (youngGen_.contains(slotPtr->getPointer())) {
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
  WeakRefSlot *res;
  if (firstFreeWeak_) {
    assert(
        firstFreeWeak_->state() == WeakSlotState::Free &&
        "invalid free slot state");
    res = firstFreeWeak_;
    firstFreeWeak_ = firstFreeWeak_->nextFree();
    res->reset(init);
  } else {
    weakSlots_.push_back({init});
    res = &weakSlots_.back();
  }
  weakRefSlotsWithPossibleYoungReferent_.push_back(res);
  return res;
}

void GenGC::freeWeakSlot(WeakRefSlot *slot) {
  slot->free(firstFreeWeak_);
  firstFreeWeak_ = slot;
}

void GenGC::shrinkWeakSlots() {
  // Opportunistically shrink the deque if free slots are found at the end.
  while (!weakSlots_.empty() && firstFreeWeak_ == &weakSlots_.back()) {
    firstFreeWeak_ = firstFreeWeak_->nextFree();
    weakSlots_.pop_back();
  }

  // TODO: in the future we might want to keep an ordered list of free
  // regions. The current singly-linked list doesn't guarantee that the
  // deque will be optimally shrunk.
}

void GenGC::updateWeightedUsed() {
  // Have we just done the first full collection?
  double curUsed = static_cast<double>(usedDirect());
  static_assert(
      0.0 < kWeightedUsedAlpha && kWeightedUsedAlpha < 1.0,
      "kWeightedUsedAlpha must be in the range [0.0..1.0].");
  weightedUsed_ = (kWeightedUsedAlpha * curUsed) +
      ((1.0 - kWeightedUsedAlpha) * weightedUsed_);
}

void GenGC::updateHeapSize() {
  // Should we alter the total heap size?
  if (usedDirect() >
      static_cast<gcheapsize_t>(sizeDirect() * occupancyTarget())) {
    growTo(usedToDesiredSize(usedDirect()));
    // Otherwise, we may wish to shrink the heap.  If we do this, it may shrink
    // the the YG size.  To prevent shrinking past the current level_, we only
    // do this if the YG is empty -- which should be the common case at the end
    // of a full GC.
  } else if (LLVM_LIKELY(youngGen_.usedDirect() == 0)) {
    // Note that the Generations' adjustSize methods set a non-zero lower limit
    // on the generation sizes, so the heap size will not fall to zero, even if
    // the live data is zero.  Note that shrinkTo is a no-op if targetSize >=
    // size().
    shrinkTo(usedToDesiredSize(static_cast<gcheapsize_t>(weightedUsed_)));
  }

  CrashManager::HeapInformation info;
  getCrashManagerHeapInfo(info);
  crashMgr_->setHeapInfo(info);
}

void GenGC::updateCrashManagerHeapExtents() {
  AllocContextYieldThenClaim yielder(this);
  youngGen_.updateCrashManagerHeapExtents(name_, crashMgr_.get());
  oldGen_.updateCrashManagerHeapExtents(name_, crashMgr_.get());
}

void GenGC::forAllObjs(const std::function<void(GCCell *)> &callback) {
  AllocContextYieldThenClaim yielder(this);
  youngGen_.forAllObjs(callback);
  oldGen_.forAllObjs(callback);
}

#ifndef NDEBUG
void GenGC::youngGenCollect() {
  AllocContextYieldThenClaim yielder(this);
  youngGen_.collect();
}

unsigned GenGC::computeNumAllocatedObjects() const {
  return youngGen_.numAllocatedObjects() + oldGen_.numAllocatedObjects();
}

unsigned GenGC::computeNumReachableObjects() const {
  return youngGen_.numReachableObjects() + oldGen_.numReachableObjects();
}

unsigned GenGC::computeNumHiddenClasses() const {
  return youngGen_.numHiddenClasses() + oldGen_.numHiddenClasses();
}

unsigned GenGC::computeNumLeafHiddenClasses() const {
  return youngGen_.numLeafHiddenClasses() + oldGen_.numLeafHiddenClasses();
}

unsigned GenGC::computeNumFinalizedObjects() const {
  return youngGen_.numFinalizedObjects() + oldGen_.numFinalizedObjects();
}

void GenGC::resetNumReachableObjectsInGens() {
  youngGen_.resetNumReachableObjects();
  oldGen_.resetNumReachableObjects();
}

void GenGC::resetNumAllHiddenClassesInGens() {
  youngGen_.resetNumAllHiddenClasses();
  oldGen_.resetNumAllHiddenClasses();
}

void GenGC::setNumAllocatedObjectsAfterFullCollectionNC(
    const CompactionResult &compactionResult) {
  youngGen_.resetNumAllocatedObjects();
  oldGen_.resetNumAllocatedObjects();

  for (auto &chunk : compactionResult.usedChunks()) {
    chunk.recordNumAllocated();
  }
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

#ifndef NDEBUG
bool GenGC::needsWriteBarrier(void *loc, void *value) {
  return !youngGen_.contains(loc) && youngGen_.contains(value);
}
#endif

LLVM_ATTRIBUTE_NOINLINE
void GenGC::writeBarrier(void *loc, HermesValue value) {
  countWriteBarrier(/*hv*/ true, kNumWriteBarrierTotalCountIdx);
  if (!value.isPointer()) {
    return;
  }
  countWriteBarrier(/*hv*/ true, kNumWriteBarrierOfObjectPtrIdx);
  writeBarrierImpl(loc, value.getPointer(), /*hv*/ true);
}

LLVM_ATTRIBUTE_NOINLINE
void GenGC::writeBarrier(void *loc, void *value) {
  countWriteBarrier(/*hv*/ false, kNumWriteBarrierTotalCountIdx);
  writeBarrierImpl(loc, value, /*hv*/ false);
}

LLVM_ATTRIBUTE_NOINLINE
void GenGC::constructorWriteBarrier(void *loc, HermesValue value) {
  // There's no difference for GenGC between the constructor and an assignment.
  writeBarrier(loc, value);
}

LLVM_ATTRIBUTE_NOINLINE
void GenGC::constructorWriteBarrier(void *loc, void *value) {
  // There's no difference for GenGC between the constructor and an assignment.
  writeBarrier(loc, value);
}

void GenGC::writeBarrierRange(GCHermesValue *start, uint32_t numHVs) {
  countRangeWriteBarrier();

  // For now, in this case, we'll just dirty the cards in the range.  We could
  // look at the copied contents, or change the interface to take the "from"
  // range, and dirty the cards if the from range has any dirty cards.  But just
  // dirtying the cards will probably be fine.  We could also check to make sure
  // that the range to dirty is not in the young generation, but expect that in
  // most cases the cost of the check will be similar to the cost of dirtying
  // those addresses.
  char *firstPtr = reinterpret_cast<char *>(start);
  char *lastPtr = reinterpret_cast<char *>(start + numHVs) - 1;

  assert(
      AlignedStorage::start(firstPtr) == AlignedStorage::start(lastPtr) &&
      "Range should be contained in the same segment");

  GenGCHeapSegment::cardTableCovering(firstPtr)->dirtyCardsForAddressRange(
      firstPtr, lastPtr);
}

size_t GenGC::getPeakLiveAfterGC() const {
  // "live" must only be measured after full GCs.
  return fullCollectionCumStats_.usedAfter.max();
}

void GenGC::getHeapInfo(HeapInfo &info) {
  AllocContextYieldThenClaim yielder(this);
  GCBase::getHeapInfo(info);
  info.allocatedBytes = usedDirect();
  info.heapSize = sizeDirect();
  info.totalAllocatedBytes = totalAllocatedBytes_ + bytesAllocatedSinceLastGC();
  info.va = segmentIndex_.size() * AlignedStorage::size();
  info.youngGenStats = youngGenCollectionCumStats_;
  info.fullStats = fullCollectionCumStats_;
  info.numMarkStackOverflows = markState_.numMarkStackOverflows_;
}

void GenGC::getCrashManagerHeapInfo(CrashManager::HeapInformation &info) {
  info.size_ = size();
  info.used_ = used();
}

void GenGC::getHeapInfoWithMallocSize(HeapInfo &info) {
  getHeapInfo(info);
  GCBase::getHeapInfoWithMallocSize(info);
  // In case the info is being re-used, ensure the count starts at 0.
  info.mallocSizeEstimate = 0;

  // First add the usage by the runtime's roots.
  info.mallocSizeEstimate += gcCallbacks_->mallocSize();

  {
    /// Yield, then reclaim, the allocation context.  (This is a noop
    /// if the context has already been yielded.)
    AllocContextYieldThenClaim yielder(this);
    // Add all usage from both generations of the heap.
    info.mallocSizeEstimate += oldGen_.mallocSizeFromFinalizerList();
    info.mallocSizeEstimate += youngGen_.mallocSizeFromFinalizerList();
  }

  info.mallocSizeEstimate += markedSymbols_.getMemorySize();
}

#ifndef NDEBUG
void GenGC::getDebugHeapInfo(DebugHeapInfo &info) {
  // Make sure the allocated object count is written back.
  AllocContextYieldThenClaim yielder(this);
  GCBase::getDebugHeapInfo(info);
}
#endif

void GenGC::dump(llvh::raw_ostream &os, bool verbose) {
  AllocContextYieldThenClaim yielder(this);
  GCBase::dump(os, verbose);
#ifndef NDEBUG
  // We need to do a final alloc census, to count the objects allocated since
  // the last GC.
  doAllocCensus();
  // Now we can print the census results.
  printCensusByKindStats(os);
  // Stats about external memory.
  printExtAllocStats(os);
#endif
#ifndef NDEBUG
  os << "\nWrite barriers executed:\n"
     << "Single field: "
     << (numWriteBarriers_[false][kNumWriteBarrierTotalCountIdx] +
         numWriteBarriers_[true][kNumWriteBarrierTotalCountIdx])
     << "\n"
     << "   HV:    " << numWriteBarriers_[true][kNumWriteBarrierTotalCountIdx]
     << "\n"
     << "      Value is obj ptr:     "
     << numWriteBarriers_[true][kNumWriteBarrierOfObjectPtrIdx] << "\n"
     << "      Val/loc in diff segs: "
     << numWriteBarriers_[true][kNumWriteBarrierWithDifferentSegsIdx] << "\n"
     << "      Value in YG:          "
     << numWriteBarriers_[true][kNumWriteBarrierPtrInYGIdx] << "\n"
     << "   void*: " << numWriteBarriers_[false][kNumWriteBarrierTotalCountIdx]
     << "\n"
     << "      Val/loc in diff segs: "
     << numWriteBarriers_[false][kNumWriteBarrierWithDifferentSegsIdx] << "\n"
     << "      Value in YG:          "
     << numWriteBarriers_[false][kNumWriteBarrierPtrInYGIdx] << "\n"
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

void GenGC::yieldAllocContext() {
  // If we're randomizing the alloc space, we don't use the allocation context.
  if (shouldRandomizeAllocSpace()) {
    return;
  }
  GCGeneration *targetGen = targetGeneration();
  swap(targetGen->allocContext(), allocContext_);
  segmentMoved(&targetGen->allocContext().activeSegment);
  targetGen->resetTrueAllocContext();
}

void GenGC::claimAllocContext() {
  // If we're randomizing the alloc space, we don't use the active segment.
  if (shouldRandomizeAllocSpace()) {
    return;
  }
  GCGeneration *targetGen = targetGeneration();
  swap(targetGen->allocContext(), allocContext_);
  segmentMoved(&allocContext_.activeSegment);
  targetGen->setTrueAllocContext(&allocContext_);
}

void GenGC::createSnapshot(llvh::raw_ostream &os) {
  // We need to yield/claim at outer scope, to cover the calls to
  // forUsedSegments below.
  AllocContextYieldThenClaim yielder(this);
  // We'll say we're in GC even though we're not, to avoid assertion failures.
  GCCycle cycle{this};
  // No allocations are allowed throughout the entire heap snapshot process.
  NoAllocScope scope{this};
#ifdef HERMES_SLOW_DEBUG
  checkWellFormedHeap();
#endif
  GCBase::createSnapshot(this, os);
#ifdef HERMES_SLOW_DEBUG
  checkWellFormedHeap();
#endif
}

#ifdef HERMESVM_SERIALIZE
void GenGC::serializeWeakRefs(Serializer &s) {
  int numWeakRefSlots = weakSlots_.size();
  s.writeInt<uint32_t>(numWeakRefSlots);
  for (auto &slot : weakSlots_) {
    // Serialize WeakRefSlot slot.
    auto state = slot.state();
    s.writeInt<uint32_t>(state);
    // Should not be serializing in the middle of GC.
    assert(state != WeakSlotState::Marked && "marked state would be lost");
    if (state != WeakSlotState::Free) {
      assert(state == WeakSlotState::Unmarked);
      if (WeakRefSlot::kRelocKind == RelocationKind::HermesValue) {
        s.writeHermesValue(
            slot.hasValue() ? slot.value() : HermesValue::encodeEmptyValue());
      } else {
        s.writeRelocation(slot.getPointer());
      }
    }
    // Call endObject() for slot because another free WeakSlot may have a
    // pointer to &slot.
    s.endObject(&slot);
  }
}

void GenGC::deserializeWeakRefs(Deserializer &d) {
  if (weakSlots_.size() != 0) {
    hermes_fatal(
        "must deserialize WeakRefs before there are weak refs in heap");
  }

  uint32_t numWeakRefSlots = d.readInt<uint32_t>();
  weakSlots_.resize(numWeakRefSlots);
  for (auto &slot : weakSlots_) {
    // Deserialize this WeakRefSlot.
    auto state = d.readInt<uint32_t>();
    if (state == WeakSlotState::Free) {
      freeWeakSlot(&slot);
    } else {
      assert(state == WeakSlotState::Unmarked);
      if (WeakRefSlot::kRelocKind == RelocationKind::HermesValue) {
        d.readHermesValue(
            reinterpret_cast<HermesValue *>(slot.deserializeAddr()));
      } else {
        d.readRelocation(slot.deserializeAddr(), WeakRefSlot::kRelocKind);
      }
    }
    d.endObject(&slot);
  }
}

#undef DEBUG_TYPE
#define DEBUG_TYPE "serialize"
void GenGC::serializeHeap(Serializer &s) {
  // We need to yield/claim at outer scope, to cover the calls to
  // forUsedSegments below.
  AllocContextYieldThenClaim yielder(this);
  // We'll say we're in GC even though we're not, to avoid assertion failures.
  GCCycle cycle{this};

  auto serializeObject = [&s](const GCCell *cell) {
    // ArrayStorage will be serialized/deserialized with its "owner" because
    // only its owner knows if this ArrayStorage has native pointer in it.
    if (cell->getKind() != CellKind::ArrayStorageKind) {
      LLVM_DEBUG(
          llvh::dbgs() << "Heap Serialize Cell " << cellKindStr(cell->getKind())
                       << ", id " << cell->getDebugAllocationId() << "\n");
      s.writeInt<uint8_t>((uint8_t)cell->getKind());
      s.serializeCell(cell);
    } else {
      LLVM_DEBUG(
          llvh::dbgs() << "Heap Serialize Skipped Cell "
                       << cellKindStr(cell->getKind()) << ", id "
                       << cell->getDebugAllocationId() << "\n");
    }
  };

  oldGen_.forAllObjs(serializeObject);

  // Write a 255 at the end to signal that we finish serializing heap objects.
  s.writeInt<uint8_t>(255);
}

void GenGC::deserializeHeap(Deserializer &d) {
  uint8_t kind;
  while ((kind = d.readInt<uint8_t>()) != 255) {
    LLVM_DEBUG(
        llvh::dbgs() << "Heap Deserialize Cell " << cellKindStr((CellKind)kind)
                     << "\n");
    // ArrayStorage will be serialized/deserialized with its "owner" because
    // only its owner knows if this ArrayStorage has native pointer in it.
    assert(
        (CellKind)kind != CellKind::ArrayStorageKind &&
        "ArrayStorage should be deserialized with owner.");
    d.deserializeCell(kind);
  }
}
#undef DEBUG_TYPE
#define DEBUG_TYPE "gc"
void GenGC::deserializeStart() {
  GCBase::deserializeStart();
  // First, yield the allocation context, so the heap is well-formed.
  yieldAllocContext();

  allocContextFromYG_ = false;
  claimAllocContext();
}

void GenGC::deserializeEnd() {
  // First, yield the allocation context, so the heap is well-formed.
  yieldAllocContext();
  // If we were doing direct OG allocation, the card object boundaries
  // will not be valid.  Recreate it.
  oldGen_.recreateCardTableBoundaries();
  // Now switch to doing YG alloc, and claim the alloc context from the YG.
  allocContextFromYG_ = true;
  claimAllocContext();
  GCBase::deserializeEnd();
}
#endif

void GenGC::printStats(JSONEmitter &json) {
  GCBase::printStats(json);
  json.emitKey("specific");
  json.openDict();
  json.emitKeyValue("collector", kGCName);

  json.emitKey("stats");
  json.openDict();
  json.emitKeyValue(
      "ygNumCollections", youngGenCollectionCumStats_.numCollections);
  json.emitKeyValue(
      "ygTotalGCTime",
      formatSecs(youngGenCollectionCumStats_.gcWallTime.sum()).secs);
  json.emitKeyValue(
      "ygAvgGCPause",
      formatSecs(youngGenCollectionCumStats_.gcWallTime.average()).secs);
  json.emitKeyValue(
      "ygMaxGCPause",
      formatSecs(youngGenCollectionCumStats_.gcWallTime.max()).secs);
  json.emitKeyValue(
      "ygTotalGCCPUTime",
      formatSecs(youngGenCollectionCumStats_.gcCPUTime.sum()).secs);
  json.emitKeyValue(
      "ygAvgGCCPUPause",
      formatSecs(youngGenCollectionCumStats_.gcCPUTime.average()).secs);
  json.emitKeyValue(
      "ygMaxGCCPUPause",
      formatSecs(youngGenCollectionCumStats_.gcCPUTime.max()).secs);
  json.emitKeyValue(
      "ygFinalSize",
      formatSize(youngGenCollectionCumStats_.finalHeapSize).bytes);
  youngGen_.printStats(json);
  json.emitKeyValue(
      "fullNumCollections", fullCollectionCumStats_.numCollections);
  json.emitKeyValue(
      "fullTotalGCTime",
      formatSecs(fullCollectionCumStats_.gcWallTime.sum()).secs);
  json.emitKeyValue(
      "fullAvgGCPause",
      formatSecs(fullCollectionCumStats_.gcWallTime.average()).secs);
  json.emitKeyValue(
      "fullMaxGCPause",
      formatSecs(fullCollectionCumStats_.gcWallTime.max()).secs);
  json.emitKeyValue(
      "fullTotalGCCPUTime",
      formatSecs(fullCollectionCumStats_.gcCPUTime.sum()).secs);
  json.emitKeyValue(
      "fullAvgGCCPUPause",
      formatSecs(fullCollectionCumStats_.gcCPUTime.average()).secs);
  json.emitKeyValue(
      "fullMaxGCCPUPause",
      formatSecs(fullCollectionCumStats_.gcCPUTime.max()).secs);
  json.emitKeyValue(
      "fullFinalSize", formatSize(fullCollectionCumStats_.finalHeapSize).bytes);
  printFullCollectionStats(json);
  json.closeDict();
  json.closeDict();
}

void GenGC::printFullCollectionStats(JSONEmitter &json) const {
  double fullSurvivalPct = 0.0;
  if (cumPreBytes_ > 0) {
    fullSurvivalPct = 100.0 * static_cast<double>(cumPostBytes_) /
        static_cast<double>(cumPreBytes_);
  }

  json.emitKeyValue("fullMarkRootsTime", markRootsSecs_);
  json.emitKeyValue("fullMarkTransitiveTime", markTransitiveSecs_);
  json.emitKeyValue("fullSweepTime", sweepSecs_);
  json.emitKeyValue("fullUpdateRefsTime", updateReferencesSecs_);
  json.emitKeyValue("fullCompactTime", compactSecs_);
  json.emitKeyValue("fullSurvivalPct", fullSurvivalPct);
}

GenGC::CollectionSection::CollectionSection(
    GenGC *gc,
    const char *name,
    std::string cause,
    OptValue<GCCallbacks *> gcCallbacksOpt)
    : PerfSection(name, gc->getName().c_str()),
      gc_(gc),
      cycle_(gc, gcCallbacksOpt, name),
      cause_(std::move(cause)),
      wallStart_(steady_clock::now()),
      cpuStart_(oscompat::thread_cpu_time()),
      gcUsedBefore_(gc->usedDirect()),
      yielder_(gc) {
#ifdef HERMES_SLOW_DEBUG
  gc_->checkWellFormedHeap();
#endif

#ifndef NDEBUG
  gc_->doAllocCensus();
#endif

  gc_->updateTotalAllocStats();
}

GenGC::CollectionSection::~CollectionSection() {
  gc_->youngGen_.didFinishGC();
  gc_->oldGen_.didFinishGC();

#ifdef HERMES_SLOW_DEBUG
  gc_->checkWellFormedHeap();
#endif

#ifdef HERMESVM_PLATFORM_LOGGING
  assert(
      wallElapsedSecs_ >= 0.0 && "Requires wallElapsedSecs_ to have been set.");
  assert(
      cpuElapsedSecs_ >= 0.0 && "Requires cpuElapsedSecs_ to have been set.");
  hermesLog(
      "HermesGC",
      "%s[%s]: elapsed wall time = %f ms, cpu time = %f ms",
      name_,
      gc_->getName().c_str(),
      wallElapsedSecs_ * 1000.0,
      cpuElapsedSecs_ * 1000.0);
  hermesLog(
      "HermesGC", "    used = %zu, heap size = %zu.", gc_->used(), gc_->size());
#endif
}

void GenGC::CollectionSection::recordGCStats(
    size_t regionSize,
    size_t usedBefore,
    size_t sizeBefore,
    size_t usedAfter,
    size_t sizeAfter,
    CumulativeHeapStats *regionStats) {
  const TimePoint wallEnd = steady_clock::now();
  wallElapsedSecs_ = GCBase::clockDiffSeconds(wallStart_, wallEnd);

  const std::chrono::microseconds cpuEnd = oscompat::thread_cpu_time();
  cpuElapsedSecs_ = GCBase::clockDiffSeconds(cpuStart_, cpuEnd);

  addArgD("gcCPUTime", cpuElapsedSecs_);

  // Record as an overall collection.
  GCAnalyticsEvent event{
      gc_->getName(),
      kGCName,
      cycle_.extraInfo(),
      std::move(cause_),
      std::chrono::duration_cast<std::chrono::milliseconds>(
          wallEnd - wallStart_),
      std::chrono::duration_cast<std::chrono::milliseconds>(cpuEnd - cpuStart_),
      /*preAllocated*/ usedBefore,
      /*preSize*/ sizeBefore,
      /*postAllocated*/ usedAfter,
      /*postSize*/ sizeAfter,
      /*survivalRatio*/ usedBefore ? (usedAfter * 1.0) / usedBefore : 0};

  gc_->recordGCStats(event);
  // Also record as a region-specific collection.
  gc_->recordGCStats(event, regionStats);

  LLVM_DEBUG(
      dbgs() << "End garbage collection. numCollected="
             << gc_->numCollectedObjects_
             << "; wall time=" << formatSecs(wallElapsedSecs_)
             << "; cpu time=" << formatSecs(cpuElapsedSecs_) << "\n\n");
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

void GenGC::printCensusByKindStats(llvh::raw_ostream &os) const {
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
    llvh::raw_ostream &os,
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
     << "Total objs: " << llvh::format("%14lld", totAllocs)
     << ", bytes = " << llvh::format("%14lld", totBytes) << ".\n\n"
     << llvh::format(
            "%35s%14s%8s%14s%8s\n",
            static_cast<const char *>("kind"),
            static_cast<const char *>("objects"),
            static_cast<const char *>("(%)"),
            static_cast<const char *>("bytes"),
            static_cast<const char *>("(%)"));
  os << "--------------------------------------------------------------------------\n";

  double totAllocsD = totAllocs;
  double totBytesD = totBytes;
  for (const Elem &elem : data) {
    // If there were no instances of a cell kind, omit the line.
    if (std::get<1>(elem) == 0) {
      assert(std::get<2>(elem) == 0);
      continue;
    }
    os << llvh::format(
        "%35s%14lld%7.2f%%%14lld%7.2f%%\n",
        cellKindStr(static_cast<CellKind>(std::get<0>(elem))),
        std::get<1>(elem),
        100.0 * static_cast<double>(std::get<1>(elem)) / totAllocsD,
        std::get<2>(elem),
        100.0 * static_cast<double>(std::get<2>(elem)) / totBytesD);
  }
}
#endif

void GenGC::sizeDiagnosticCensus() {
  struct HeapSizeDiagnostic {
    struct HermesValueDiagnostic {
      uint64_t count = 0;
      uint64_t numBool = 0;
      uint64_t numNumber = 0;
      uint64_t numInt8 = 0;
      uint64_t numInt16 = 0;
      uint64_t numInt24 = 0;
      uint64_t numInt32 = 0;
      uint64_t numSymbol = 0;
      uint64_t numNull = 0;
      uint64_t numUndefined = 0;
      uint64_t numEmpty = 0;
      uint64_t numNativeValue = 0;
      uint64_t numString = 0;
      uint64_t numObject = 0;
    };

    struct StringDiagnostic {
      uint64_t count = 0;
      // Count of strings of a given size. Initialize to all zeros.
      // The zeroth index is unused, but left as zero.
      std::array<uint64_t, 8> countPerSize{};
      uint64_t totalChars = 0;
    };

    uint64_t numCell = 0;
    uint64_t numVariableSizedObject = 0;
    uint64_t numPointer = 0;
    uint64_t numSymbol = 0;
    HermesValueDiagnostic hv;
    StringDiagnostic asciiStr;
    StringDiagnostic utf16Str;

    const char *fmts[3] = {
        "\t%-25s : %'10" PRIu64 " [%'10" PRIu64 " B | %4.1f%%]",
        "\t\t%-25s : %'10" PRIu64 " [%'10" PRIu64 " B | %4.1f%%]",
        "\t\t\t%-25s : %'10" PRIu64 " [%'10" PRIu64 " B | %4.1f%%]"};

    void rootsDiagnosticFrame() const {
      // Use this to print commas on large numbers
      char *currentLocale = std::setlocale(LC_NUMERIC, nullptr);
      std::setlocale(LC_NUMERIC, "");

      uint64_t rootSize = hv.count * sizeof(HermesValue) +
          numPointer * sizeof(GCPointerBase) + numSymbol * sizeof(SymbolID);
      hermesLog("HermesGC", "Root size: %'7" PRIu64 " B", rootSize);

      hermesValueDiagnostic(rootSize);
      gcPointerDiagnostic(rootSize);
      symbolDiagnostic(rootSize);

      std::setlocale(LC_NUMERIC, currentLocale);
    }

    void sizeDiagnosticFrame(uint64_t heapSize) const {
      // Use this to print commas on large numbers
      char *currentLocale = std::setlocale(LC_NUMERIC, nullptr);
      std::setlocale(LC_NUMERIC, "");

      hermesLog("HermesGC", "Heap size: %'7" PRIu64 " B", heapSize);
      hermesLog("HermesGC", "\tTotal cells: %'7" PRIu64, numCell);
      hermesLog(
          "HermesGC",
          "\tNum variable size cells: %'7" PRIu64,
          numVariableSizedObject);

      // In theory should use sizeof(VariableSizeRuntimeCell), but that includes
      // padding sometimes. To be conservative, use the field it contains
      // directly instead.
      uint64_t headerSize =
          numVariableSizedObject * (sizeof(GCCell) + sizeof(uint32_t)) +
          (numCell - numVariableSizedObject) * sizeof(GCCell);
      hermesLog(
          "HermesGC",
          fmts[0],
          "Cell headers",
          numCell,
          headerSize,
          getPercent(headerSize, heapSize));

      hermesValueDiagnostic(heapSize);
      gcPointerDiagnostic(heapSize);
      symbolDiagnostic(heapSize);

      {
        hermesLog(
            "HermesGC",
            fmts[0],
            "StringPrimitive (ASCII)",
            asciiStr.count,
            asciiStr.totalChars,
            getPercent(asciiStr.totalChars, heapSize));
        for (decltype(asciiStr.countPerSize.size()) i = 1;
             i < asciiStr.countPerSize.size();
             i++) {
          std::string tag;
          llvh::raw_string_ostream stream(tag);
          stream << llvh::format("StringPrimitive (size %1lu)", i);
          stream.flush();
          hermesLog(
              "HermesGC",
              fmts[1],
              tag.c_str(),
              asciiStr.countPerSize[i],
              asciiStr.countPerSize[i] * i,
              getPercent(asciiStr.countPerSize[i] * i, asciiStr.totalChars));
        }

        hermesLog(
            "HermesGC",
            fmts[0],
            "StringPrimitive (UTF-16)",
            utf16Str.count,
            utf16Str.totalChars,
            getPercent(utf16Str.totalChars, heapSize));
        for (decltype(utf16Str.countPerSize.size()) i = 1;
             i < utf16Str.countPerSize.size();
             i++) {
          std::string tag;
          llvh::raw_string_ostream stream(tag);
          stream << llvh::format("StringPrimitive (size %1lu)", i);
          stream.flush();
          hermesLog(
              "HermesGC",
              fmts[1],
              tag.c_str(),
              utf16Str.countPerSize[i],
              utf16Str.countPerSize[i] * i,
              getPercent(utf16Str.countPerSize[i] * i, utf16Str.totalChars));
        }
      }

      uint64_t leftover = heapSize - (hv.count * sizeof(HermesValue)) -
          (numPointer * sizeof(GCPointerBase)) - (asciiStr.totalChars * 2) -
          utf16Str.totalChars - headerSize;
      hermesLog(
          "HermesGC",
          fmts[0],
          "Other",
          "-",
          leftover,
          getPercent(leftover, heapSize));

      std::setlocale(LC_NUMERIC, currentLocale);
    }

   private:
    void hermesValueDiagnostic(uint64_t heapSize) const {
      constexpr uint64_t bytesHV = sizeof(HermesValue);
      hermesLog(
          "HermesGC",
          fmts[0],
          "HV",
          hv.count,
          hv.count * bytesHV,
          getPercent(hv.count * bytesHV, heapSize));

      hermesLog(
          "HermesGC",
          fmts[1],
          "Bool",
          hv.numBool,
          hv.numBool * bytesHV,
          getPercent(hv.numBool, hv.count));

      {
        hermesLog(
            "HermesGC",
            fmts[1],
            "Number",
            hv.numNumber,
            hv.numNumber * bytesHV,
            getPercent(hv.numNumber, hv.count));
        hermesLog(
            "HermesGC",
            fmts[2],
            "Int8",
            hv.numInt8,
            hv.numInt8 * bytesHV,
            getPercent(hv.numInt8, hv.numNumber));
        hermesLog(
            "HermesGC",
            fmts[2],
            "Int16",
            hv.numInt16,
            hv.numInt16 * bytesHV,
            getPercent(hv.numInt16, hv.numNumber));
        hermesLog(
            "HermesGC",
            fmts[2],
            "Int24",
            hv.numInt24,
            hv.numInt24 * bytesHV,
            getPercent(hv.numInt24, hv.numNumber));
        hermesLog(
            "HermesGC",
            fmts[2],
            "Int32",
            hv.numInt32,
            hv.numInt32 * bytesHV,
            getPercent(hv.numInt32, hv.numNumber));

        uint64_t numDoubles =
            hv.numNumber - hv.numInt32 - hv.numInt24 - hv.numInt16 - hv.numInt8;
        hermesLog(
            "HermesGC",
            fmts[2],
            "Doubles",
            numDoubles,
            numDoubles * bytesHV,
            getPercent(numDoubles, hv.numNumber));
      }

      hermesLog(
          "HermesGC",
          fmts[1],
          "Symbol",
          hv.numSymbol,
          hv.numSymbol * bytesHV,
          getPercent(hv.numSymbol, hv.count));
      hermesLog(
          "HermesGC",
          fmts[1],
          "Null",
          hv.numNull,
          hv.numNull * bytesHV,
          getPercent(hv.numNull, hv.count));
      hermesLog(
          "HermesGC",
          fmts[1],
          "Undefined",
          hv.numUndefined,
          hv.numUndefined * bytesHV,
          getPercent(hv.numUndefined, hv.count));
      hermesLog(
          "HermesGC",
          fmts[1],
          "Empty",
          hv.numEmpty,
          hv.numEmpty * bytesHV,
          getPercent(hv.numEmpty, hv.count));
      hermesLog(
          "HermesGC",
          fmts[1],
          "NativeValue",
          hv.numNativeValue,
          hv.numNativeValue * bytesHV,
          getPercent(hv.numNativeValue, hv.count));
      hermesLog(
          "HermesGC",
          fmts[1],
          "StringPointer",
          hv.numString,
          hv.numString * bytesHV,
          getPercent(hv.numString, hv.count));
      hermesLog(
          "HermesGC",
          fmts[1],
          "ObjectPointer",
          hv.numObject,
          hv.numObject * bytesHV,
          getPercent(hv.numObject, hv.count));
    }

    void gcPointerDiagnostic(uint64_t heapSize) const {
      hermesLog(
          "HermesGC",
          fmts[0],
          "GCPointer",
          numPointer,
          numPointer * sizeof(GCPointerBase),
          getPercent(numPointer * sizeof(GCPointerBase), heapSize));
    }

    void symbolDiagnostic(uint64_t heapSize) const {
      hermesLog(
          "HermesGC",
          fmts[0],
          "Symbol",
          numSymbol,
          numSymbol * sizeof(SymbolID),
          getPercent(numSymbol * sizeof(SymbolID), heapSize));
    }

    static double getPercent(double numer, double denom) {
      return denom != 0 ? 100 * numer / denom : 0.0;
    }
  };

  struct HeapSizeDiagnosticAcceptor final : public RootAndSlotAcceptor {
    // Can't be static in a local class.
    const int64_t HINT8_MIN = -(1 << 7);
    const int64_t HINT8_MAX = (1 << 7) - 1;
    const int64_t HINT16_MIN = -(1 << 15);
    const int64_t HINT16_MAX = (1 << 15) - 1;
    const int64_t HINT24_MIN = -(1 << 23);
    const int64_t HINT24_MAX = (1 << 23) - 1;
    const int64_t HINT32_MIN = -(1LL << 31);
    const int64_t HINT32_MAX = (1LL << 31) - 1;

    HeapSizeDiagnostic diagnostic;

    HeapSizeDiagnosticAcceptor() = default;

    using SlotAcceptor::accept;

    void accept(void *&ptr) override {
      diagnostic.numPointer++;
    }

    void accept(GCPointerBase &ptr) override {
      diagnostic.numPointer++;
    }

    void accept(PinnedHermesValue &hv) override {
      acceptHV(hv);
    }
    void accept(GCHermesValue &hv) override {
      acceptHV(hv);
    }
    void acceptHV(HermesValue &hv) {
      diagnostic.hv.count++;
      if (hv.isBool()) {
        diagnostic.hv.numBool++;
      } else if (hv.isNumber()) {
        diagnostic.hv.numNumber++;

        double val = hv.getNumber();
        double intpart;
        if (std::modf(val, &intpart) == 0.0) {
          if (val >= static_cast<double>(HINT8_MIN) &&
              val <= static_cast<double>(HINT8_MAX)) {
            diagnostic.hv.numInt8++;
          } else if (
              val >= static_cast<double>(HINT16_MIN) &&
              val <= static_cast<double>(HINT16_MAX)) {
            diagnostic.hv.numInt16++;
          } else if (
              val >= static_cast<double>(HINT24_MIN) &&
              val <= static_cast<double>(HINT24_MAX)) {
            diagnostic.hv.numInt24++;
          } else if (
              val >= static_cast<double>(HINT32_MIN) &&
              val <= static_cast<double>(HINT32_MAX)) {
            diagnostic.hv.numInt32++;
          }
        }
      } else if (hv.isString()) {
        diagnostic.hv.numString++;
      } else if (hv.isSymbol()) {
        diagnostic.hv.numSymbol++;
      } else if (hv.isObject()) {
        diagnostic.hv.numObject++;
      } else if (hv.isNull()) {
        diagnostic.hv.numNull++;
      } else if (hv.isUndefined()) {
        diagnostic.hv.numUndefined++;
      } else if (hv.isEmpty()) {
        diagnostic.hv.numEmpty++;
      } else if (hv.isNativeValue()) {
        diagnostic.hv.numNativeValue++;
      } else {
        assert(false && "Should be no other hermes values");
      }
    }

    void accept(PinnedSymbolID sym) override {
      acceptSym(sym);
    }
    void accept(GCSymbolID sym) override {
      acceptSym(sym);
    }
    void acceptSym(SymbolID sym) {
      diagnostic.numSymbol++;
    }
  };

  hermesLog("HermesGC", "%s:", "Roots");
  HeapSizeDiagnosticAcceptor rootAcceptor;
  DroppingAcceptor<HeapSizeDiagnosticAcceptor> namedRootAcceptor{rootAcceptor};
  markRoots(namedRootAcceptor, /* markLongLived */ true);
  rootAcceptor.diagnostic.rootsDiagnosticFrame();

  hermesLog("HermesGC", "%s:", "Heap contents");
  HeapSizeDiagnosticAcceptor acceptor;
  GC *gc = this;
  forAllObjs([gc, &acceptor](GCCell *cell) {
    GCBase::markCell(cell, gc, acceptor);
    acceptor.diagnostic.numCell++;
    acceptor.diagnostic.numVariableSizedObject +=
        static_cast<int>(cell->isVariableSize());

    if (cell->getKind() == CellKind::DynamicASCIIStringPrimitiveKind ||
        cell->getKind() == CellKind::DynamicUniquedASCIIStringPrimitiveKind ||
        cell->getKind() == CellKind::ExternalASCIIStringPrimitiveKind) {
      acceptor.diagnostic.asciiStr.count++;
      auto *strprim = vmcast<StringPrimitive>(cell);
      if (strprim->getStringLength() < 8) {
        acceptor.diagnostic.asciiStr.countPerSize[strprim->getStringLength()]++;
      }
      acceptor.diagnostic.asciiStr.totalChars += strprim->getStringLength();
    } else if (
        cell->getKind() == CellKind::DynamicUTF16StringPrimitiveKind ||
        cell->getKind() == CellKind::DynamicUniquedUTF16StringPrimitiveKind ||
        cell->getKind() == CellKind::ExternalUTF16StringPrimitiveKind) {
      acceptor.diagnostic.utf16Str.count++;
      auto *strprim = vmcast<StringPrimitive>(cell);
      if (strprim->getStringLength() < 8) {
        acceptor.diagnostic.utf16Str.countPerSize[strprim->getStringLength()]++;
      }
      acceptor.diagnostic.utf16Str.totalChars += strprim->getStringLength();
    }
  });

  acceptor.diagnostic.sizeDiagnosticFrame(used());
}

void GenGC::oomDetail(std::error_code reason) {
  AllocContextYieldThenClaim yielder(this);
  GCBase::oomDetail(reason);
  // Could use a stringstream here, but want to avoid dynamic
  // allocation in OOM situations.
  char detailBuffer[100];
  snprintf(
      detailBuffer,
      sizeof(detailBuffer),
      "#segments = %zd, !materialize = %zd, maxHeapSize = %zd",
      segmentIndex_.size(),
      numFailedSegmentMaterializations(),
      maxSize());
  hermesLog("HermesGC", "NCGen OOM: %s", detailBuffer);
  crashMgr_->setCustomData("HermesGCOOMDetailNCGen", detailBuffer);
}

void *GenGC::allocSlow(uint32_t sz, bool fixedSize, HasFinalizer hasFinalizer) {
  AllocContextYieldThenClaim yielder(this);
  AllocResult res;
  if (allocContextFromYG_) {
    res = youngGen_.allocSlow(sz, hasFinalizer, fixedSize);
  } else {
    res = oldGen_.allocSlow(sz, hasFinalizer);
  }
  assert(res.success && "Should never fail to allocate at the top level");
  return res.ptr;
}

} // namespace vm
} // namespace hermes
