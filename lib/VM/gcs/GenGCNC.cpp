/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
// Note: this must include GC.h, instead of GenGC.h, because GenGC.h assumes
// it is included only by GC.h.  (For example, it assumes GCBase is declared.)
#include "hermes/VM/GC.h"

#include "hermes/Platform/Logging.h"
#include "hermes/Support/DebugHelpers.h"
#include "hermes/Support/JSONEmitter.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/Support/PerfSection.h"
#include "hermes/Support/StringSetVector.h"
#include "hermes/VM/AllocSource.h"
#include "hermes/VM/Casting.h"
#include "hermes/VM/CheckHeapWellFormedAcceptor.h"
#include "hermes/VM/CompactionResult-inline.h"
#include "hermes/VM/CompleteMarkState-inline.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HeapSnapshot.h"
#include "hermes/VM/HermesValue-inline.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/SweepResultNC.h"
#include "hermes/VM/SymbolID.h"

#ifdef HERMES_SLOW_DEBUG
#include "llvm/ADT/DenseSet.h"
#endif
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/MathExtras.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cinttypes>
#include <clocale>
#include <cstdint>
#include <tuple>
#include <unordered_map>
#include <utility>

#define DEBUG_TYPE "gc"

using llvm::dbgs;
using std::chrono::steady_clock;

namespace hermes {
namespace vm {

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
    StorageProvider *provider)
    : GCBase(
          metaTable,
          gcCallbacks,
          pointerBase,
          gcConfig,
          std::move(crashMgr),
          provider),
      storageProvider_(provider),
      generationSizes_(Size(gcConfig)),
      youngGen_(this, generationSizes_.youngGenSize(), &oldGen_),
      oldGen_(
          this,
          generationSizes_.oldGenSize(),
          gcConfig.getShouldReleaseUnused()),
      allocContextFromYG_(gcConfig.getAllocInYoung()),
      revertToYGAtTTI_(gcConfig.getRevertToYGAtTTI()),
      oomThreshold_(gcConfig.getEffectiveOOMThreshold()),
      weightedUsed_(static_cast<double>(gcConfig.getInitHeapSize())) {
  growTo(gcConfig.getInitHeapSize());
  claimAllocContext();
}

#ifndef NDEBUG
AllocResult
GenGC::debugAlloc(uint32_t sz, HasFinalizer hasFinalizer, bool fixedSize) {
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

void GenGC::collect(bool canEffectiveOOM) {
  if (canEffectiveOOM && ++consecFullGCs_ >= oomThreshold_)
    oom(make_error_code(OOMError::Effective));

  /// Yield, then reclaim, the allocation context.  (This is a noop
  /// if the context has already been yielded.)
  AllocContextYieldThenClaim yielder(this);

  // Make sure the AllocContext been yielded back to its owner.
  assert(!allocContext_.activeSegment);

  const size_t usedBefore = used();
  const size_t sizeBefore = size();
  cumPreBytes_ += used();

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
    CollectionSection fullCollection(this, "Full collection");

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
    sizeAfter = sizeDirect();
    cumPostBytes_ += usedAfter;

    // Update the statistics.
    const unsigned numAllocatedObjectsBefore =
        recordStatsNC(sweepResult.compactionResult);

    fullCollection.recordGCStats(sizeDirect(), &fullCollectionCumStats_);

    fullCollection.addArg("fullGCUsedAfter", usedAfter);
    fullCollection.addArg("fullGCSizeAfter", sizeAfter);
    fullCollection.addArg("fullGCNum", fullCollectionCumStats_.numCollections);

    checkInvariants(numAllocatedObjectsBefore, usedBefore);
  }

  checkTripwire(usedAfter, steady_clock::now());
#ifdef HERMESVM_SIZE_DIAGNOSTIC
  sizeDiagnosticCensus();
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
  AlignedHeapSegment *segment = segmentIndex_.segmentCovering(ptr);
  assert(!segment || segment->contains(ptr));
  return segment;
}

bool GenGC::validPointer(const void *ptr) const {
  AlignedHeapSegment *segment = segmentIndex_.segmentCovering(ptr);
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
      if (ptr) {
        assert(gc.dbgContains(ptr));
        GCCell *cell = reinterpret_cast<GCCell *>(ptr);
        AlignedHeapSegment::setCellMarkBit(cell);
      }
    }
    void accept(HermesValue &hv) override {
      if (hv.isPointer()) {
        void *ptr = hv.getPointer();
        if (ptr) {
          assert(gc.dbgContains(ptr));
          GCCell *cell = reinterpret_cast<GCCell *>(ptr);
          AlignedHeapSegment::setCellMarkBit(cell);
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
  clearMarkBits();

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
  } while (markState_.markStackOverflow_);
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
  DroppingAcceptor<SlotAcceptor> nameAcceptor{*acceptor};
  markRoots(nameAcceptor, /*markLongLived*/ true);

  // Update weak roots references.
  FullMSCUpdateWeakRootsAcceptor weakAcceptor(*this);
  DroppingAcceptor<SlotAcceptor> nameWeakAcceptor{weakAcceptor};
  markWeakRoots(nameWeakAcceptor);

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
  auto doCompaction = [&vTables](AlignedHeapSegment &segment) {
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

void GenGC::markWeakRef(const WeakRefBase &wr) {
  assert(
      wr.slot_->extra <= WeakSlotState::Marked &&
      "marking a freed weak ref slot");
  wr.slot_->extra = WeakSlotState::Marked;
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
  youngGen_.checkWellFormed(gc);
  oldGen_.checkWellFormed(gc);
}
#endif

void GenGC::segmentMoved(AlignedHeapSegment *segment) {
  segmentIndex_.update(segment);
}

void GenGC::forgetSegments(const std::vector<const char *> &lowLims) {
  segmentIndex_.remove(lowLims.begin(), lowLims.end());
}

void GenGC::moveWeakReferences(ptrdiff_t delta) {
  for (auto &slot : weakSlots_) {
    if (slot.extra == WeakSlotState::Free || !slot.value.isPointer())
      continue;

    auto cell = reinterpret_cast<char *>(slot.value.getPointer());
    slot.value = slot.value.updatePointer(cell + delta);
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
  youngGen_.growTo(sizes.first);
  oldGen_.growTo(sizes.second);
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
    if (AlignedHeapSegment::getCellMarkBit(cell)) {
      slotPtr->value =
          slotPtr->value.updatePointer(cell->getForwardingPointer());
    } else {
      slotPtr->value = HermesValue::encodeEmptyValue();
    }
  } else {
    // Young-gen collection.  If the cell is in the young gen, see if
    // it survived collection.  If so, update the slot.
    if (youngGen_.contains(cell)) {
      if (cell->hasMarkedForwardingPointer()) {
        slotPtr->value =
            slotPtr->value.updatePointer(cell->getMarkedForwardingPointer());
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
}

void GenGC::forAllObjs(const std::function<void(GCCell *)> &callback) {
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

void GenGC::writeBarrierRange(HermesValue *start, uint32_t numHVs) {
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

  AlignedHeapSegment::cardTableCovering(firstPtr)->dirtyCardsForAddressRange(
      firstPtr, lastPtr);
}

void GenGC::writeBarrierRangeFill(
    HermesValue *start,
    uint32_t numHVs,
    HermesValue value) {
  countRangeFillWriteBarrier();
  if (!value.isPointer()) {
    return;
  }

  char *firstPtr = reinterpret_cast<char *>(start);
  char *lastPtr = reinterpret_cast<char *>(start + numHVs) - 1;
  char *valuePtr = reinterpret_cast<char *>(value.getPointer());

  assert(
      AlignedStorage::start(firstPtr) == AlignedStorage::start(lastPtr) &&
      "Range should be contained in the same segment");

  if (youngGen_.contains(valuePtr)) {
    AlignedHeapSegment::cardTableCovering(firstPtr)->dirtyCardsForAddressRange(
        firstPtr, lastPtr);
  }
}

void GenGC::getHeapInfo(HeapInfo &info) {
  AllocContextYieldThenClaim yielder(this);
  GCBase::getHeapInfo(info);
  info.allocatedBytes = usedDirect();
  info.heapSize = sizeDirect();
  info.totalAllocatedBytes = totalAllocatedBytes_ + bytesAllocatedSinceLastGC();
  info.va = segmentIndex_.size() * AlignedStorage::size();
  info.fullStats = fullCollectionCumStats_;
  info.youngGenStats = youngGenCollectionCumStats_;
}

void GenGC::getHeapInfoWithMallocSize(HeapInfo &info) {
  getHeapInfo(info);
  // In case the info is being re-used, ensure the count starts at 0.
  info.mallocSizeEstimate = 0;

  // First add the usage by the runtime's roots.
  info.mallocSizeEstimate += gcCallbacks_->mallocSize();

  // Then add the contributions from all segments.
  for (auto *segment : segmentIndex_) {
    info.mallocSizeEstimate += segment->countMallocSize();
  }

  // Assume that the vector implementation doesn't use a separate bool for each
  // bool, but groups them together as bits.
  info.mallocSizeEstimate += markedSymbols_.capacity() / CHAR_BIT;
  // A deque doesn't have a capacity, so the size is the lower bound.
  info.mallocSizeEstimate +=
      weakSlots_.size() * sizeof(decltype(weakSlots_)::value_type);
}

#ifndef NDEBUG
void GenGC::getDebugHeapInfo(DebugHeapInfo &info) {
  // Make sure the allocated object count is written back.
  AllocContextYieldThenClaim yielder(this);
  GCBase::getDebugHeapInfo(info);
}
#endif

void GenGC::dump(llvm::raw_ostream &os, bool verbose) {
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

namespace {

// Abstract base class for all snapshot acceptors.
struct SnapshotAcceptor : public SlotAcceptorWithNamesDefault {
  using SlotAcceptorWithNamesDefault::accept;
  using SlotAcceptorWithNamesDefault::SlotAcceptorWithNamesDefault;

  // Sub-classes must override this
  void accept(void *&ptr, const char *name) override = 0;

  void accept(HermesValue &hv, const char *name) override {
    if (hv.isPointer()) {
      auto ptr = hv.getPointer();
      accept(ptr, name);
    }
  }

  void accept(uint64_t, const char *) { /* nop */
  }
};

struct EdgeAddingAcceptor : public SnapshotAcceptor {
  using SnapshotAcceptor::accept;

  EdgeAddingAcceptor(
      GC &gc,
      V8HeapSnapshot &snap,
      bool filterDuplicates,
      bool count)
      : SnapshotAcceptor(gc),
        snap_(snap),
        filterDuplicates_(filterDuplicates),
        count_(count) {}

  void accept(void *&ptr, const char *name) override {
    if (!ptr) {
      return;
    }
    const auto id = gc.getObjectID(ptr);

    if (!filterDuplicates_ || !seenIDs_.count(id)) {
      if (count_) {
        edgeCount_++;
      } else {
        snap_.addNamedEdge(
            V8HeapSnapshot::EdgeType::Internal,
            llvm::StringRef::withNullAsEmpty(name),
            id);
      }
      if (filterDuplicates_) {
        seenIDs_.insert(id);
      }
    }
  }

  unsigned edgeCount() const {
    return edgeCount_;
  }

 private:
  V8HeapSnapshot &snap_;
  bool filterDuplicates_;
  // If true, only count the edges, don't emit them.
  // If false, actually emit the edges.
  bool count_;
  unsigned edgeCount_{0};
  llvm::DenseSet<uint64_t> seenIDs_;
};

} // namespace

void GenGC::createSnapshot(llvm::raw_ostream &os, bool compact) {
  // We need to yield/claim at outer scope, to cover the calls to
  // forUsedSegments below.
  AllocContextYieldThenClaim yielder(this);

  // We'll say we're in GC even though we're not, to avoid assertion failures.
  GCCycle cycle{this};

#ifdef HERMES_SLOW_DEBUG
  checkWellFormedHeap();
#endif

  JSONEmitter json(os, !compact);
  V8HeapSnapshot snap(json);

  snap.beginSection(V8HeapSnapshot::Section::Nodes);

  // Count the number of roots.
  EdgeAddingAcceptor rootAcceptor(
      *this, snap, /*filterDuplicates*/ true, /*count*/ true);
  markRoots(rootAcceptor, true);
  snap.addNode(
      V8HeapSnapshot::NodeType::Synthetic,
      "(GC Roots)",
      static_cast<V8HeapSnapshot::NodeID>(IDTracker::ReservedObjectID::Roots),
      0,
      rootAcceptor.edgeCount());

  // Add a node for each object in the heap.
  forAllObjs([&snap, this](GCCell *cell) {
    EdgeAddingAcceptor acceptor(
        *this, snap, /*filterDuplicates*/ false, /*count*/ true);
    SlotVisitorWithNames<EdgeAddingAcceptor> visitor(acceptor);

    GCBase::markCellWithNames(visitor, cell, this);

    std::string str;
    // If the cell is a string, add a value to be printed.
    // TODO: add other special types here.
    if (const StringPrimitive *sp = dyn_vmcast<StringPrimitive>(cell)) {
      str = converter(sp);
    } else {
      str = cellKindStr(cell->getKind());
    }

    snap.addNode(
        V8HeapSnapshot::cellKindToNodeType(cell->getKind()),
        str,
        getObjectID(cell),
        cell->getAllocatedSize(),
        acceptor.edgeCount());
  });
  snap.endSection(V8HeapSnapshot::Section::Nodes);

  EdgeAddingAcceptor rootEdgeAcceptor(
      *this, snap, /*filterDuplicates*/ true, /*count*/ false);
  EdgeAddingAcceptor edgeAcceptor(
      *this, snap, /*filterDuplicates*/ false, /*count*/ false);
  SlotVisitorWithNames<EdgeAddingAcceptor> edgeVisitor(edgeAcceptor);

  snap.beginSection(V8HeapSnapshot::Section::Edges);

  // Add an edge from each synthetic root node to the real object it refers to.
  markRoots(rootEdgeAcceptor, true);

  // Add edges between objects in the heap.
  forAllObjs([&edgeVisitor, this](GCCell *cell) {
    GCBase::markCellWithNames(edgeVisitor, cell, this);
  });
  snap.endSection(V8HeapSnapshot::Section::Edges);

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
     << "\t\t\"collector\": \"noncontig-generational\",\n"
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

GenGC::CollectionSection::CollectionSection(GenGC *gc, const char *name)
    : PerfSection(name, gc->getName().c_str()),
      gc_(gc),
      cycle_(gc),
      wallStart_(steady_clock::now()),
      cpuStart_(oscompat::thread_cpu_time()),
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
#endif
}

void GenGC::CollectionSection::recordGCStats(
    size_t regionSize,
    CumulativeHeapStats *regionStats) {
  const TimePoint wallEnd = steady_clock::now();
  wallElapsedSecs_ = GCBase::clockDiffSeconds(wallStart_, wallEnd);

  const std::chrono::microseconds cpuEnd = oscompat::thread_cpu_time();
  cpuElapsedSecs_ = GCBase::clockDiffSeconds(cpuStart_, cpuEnd);

  addArgD("gcCPUTime", cpuElapsedSecs_);

  // Record as an overall collection.
  gc_->recordGCStats(wallElapsedSecs_, cpuElapsedSecs_, gc_->sizeDirect());
  // Also record as a region-specific collection.
  gc_->recordGCStats(
      wallElapsedSecs_, cpuElapsedSecs_, regionSize, regionStats);

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
  os << "--------------------------------------------------------------------------\n";

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

void GenGC::sizeDiagnosticCensus() {
  struct HeapSizeDiagnostic {
    struct HermesValueDiagnostic {
      size_t count = 0;
      size_t numBool = 0;
      size_t numNumber = 0;
      size_t numInt8 = 0;
      size_t numInt16 = 0;
      size_t numInt24 = 0;
      size_t numInt32 = 0;
      size_t numSymbol = 0;
      size_t numNull = 0;
      size_t numUndefined = 0;
      size_t numEmpty = 0;
      size_t numNativeValue = 0;
      size_t numString = 0;
      size_t numObject = 0;
    };

    struct StringDiagnostic {
      size_t count = 0;
      // Count of strings of a given size. Initialize to all zeros.
      // The zeroth index is unused, but left as zero.
      std::array<size_t, 8> countPerSize{};
      size_t totalChars = 0;
    };

    size_t numCell = 0;
    size_t numVariableSizedObject = 0;
    size_t numPointer = 0;
    size_t numSymbol = 0;
    HermesValueDiagnostic hv;
    StringDiagnostic asciiStr;
    StringDiagnostic utf16Str;

    const char *fmts[3] = {
        "\t%-25s : %'10" PRIdPTR " [%'10" PRIdPTR " B | %4.1f%%]",
        "\t\t%-25s : %'10" PRIdPTR " [%'10" PRIdPTR " B | %4.1f%%]",
        "\t\t\t%-25s : %'10" PRIdPTR " [%'10" PRIdPTR " B | %4.1f%%]"};

    void rootsDiagnosticFrame(size_t bytesHV, size_t bytesGCPointer) const {
      // Use this to print commas on large numbers
      char *currentLocale = std::setlocale(LC_NUMERIC, nullptr);
      std::setlocale(LC_NUMERIC, "");

      size_t rootSize = hv.count * bytesHV + numPointer * bytesGCPointer +
          numSymbol * sizeof(SymbolID);
      hermesLog(
          "HermesGC",
          "Root size with %" PRIdPTR "-byte GCPointer: %'7" PRIdPTR " B",
          bytesGCPointer,
          rootSize);

      hermesValueDiagnostic(bytesHV, rootSize);
      gcPointerDiagnostic(bytesGCPointer, rootSize);
      symbolDiagnostic(rootSize);

      std::setlocale(LC_NUMERIC, currentLocale);
    }

    void sizeDiagnosticFrame(
        size_t bytesHV,
        size_t bytesGCPointer,
        size_t heapSize) const {
      // Use this to print commas on large numbers
      char *currentLocale = std::setlocale(LC_NUMERIC, nullptr);
      std::setlocale(LC_NUMERIC, "");

      hermesLog(
          "HermesGC",
          "Heap size with %" PRIdPTR "-byte GCPointer: %'7" PRIdPTR " B",
          bytesGCPointer,
          heapSize);

      // In theory should use sizeof(VariableSizeRuntimeCell), but that includes
      // padding sometimes. To be conservative, use the field it contains
      // directly instead.
      size_t headerSize =
          numVariableSizedObject * (sizeof(GCCell) + sizeof(uint32_t)) +
          (numCell - numVariableSizedObject) * sizeof(GCCell);
      hermesLog(
          "HermesGC",
          fmts[0],
          "Cell headers",
          numCell,
          headerSize,
          getPercent(headerSize, heapSize));

      hermesValueDiagnostic(bytesHV, heapSize);
      gcPointerDiagnostic(bytesGCPointer, heapSize);
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
          llvm::raw_string_ostream stream(tag);
          stream << llvm::format("StringPrimitive (size %1lu)", i);
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
          llvm::raw_string_ostream stream(tag);
          stream << llvm::format("StringPrimitive (size %1lu)", i);
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

      size_t leftover = heapSize - (hv.count * bytesHV) -
          (numPointer * bytesGCPointer) - (asciiStr.totalChars * 2) -
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
    void hermesValueDiagnostic(size_t bytesHV, size_t heapSize) const {
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

        size_t numDoubles =
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

    void gcPointerDiagnostic(size_t bytesGCPointer, size_t heapSize) const {
      hermesLog(
          "HermesGC",
          fmts[0],
          "GCPointer",
          numPointer,
          numPointer * bytesGCPointer,
          getPercent(numPointer * bytesGCPointer, heapSize));
    }

    void symbolDiagnostic(size_t heapSize) const {
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

  struct HeapSizeDiagnosticAcceptor final : public SlotAcceptor {
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
    void accept(BasedPointer &ptr) override {
      diagnostic.numPointer++;
    }
    void accept(GCPointerBase &ptr) override {
      diagnostic.numPointer++;
    }
    void accept(HermesValue &hv) override {
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
    void accept(SymbolID sym) override {
      diagnostic.numSymbol++;
    }
  };

  const size_t ogBytesHV = 8;
  const size_t normalGCPointerSize = sizeof(void *);
  const size_t bigGCPointerSize = sizeof(uint64_t);
  const size_t smallGCPointerSize = sizeof(uint32_t);
  const size_t oppositeGCPointerSize = normalGCPointerSize == bigGCPointerSize
      ? smallGCPointerSize
      : bigGCPointerSize;
  const size_t ogHeapSize = this->used();

  hermesLog(
      "HermesGC", "GCPointers are %" PRIdPTR "-bytes long", sizeof(void *));
  hermesLog("HermesGC", "%s:", "Roots");
  HeapSizeDiagnosticAcceptor rootAcceptor;
  DroppingAcceptor<HeapSizeDiagnosticAcceptor> namedRootAcceptor{rootAcceptor};
  markRoots(namedRootAcceptor, /* markLongLived */ true);
  rootAcceptor.diagnostic.rootsDiagnosticFrame(ogBytesHV, normalGCPointerSize);

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

  acceptor.diagnostic.sizeDiagnosticFrame(
      ogBytesHV, normalGCPointerSize, ogHeapSize);
  acceptor.diagnostic.sizeDiagnosticFrame(
      ogBytesHV,
      oppositeGCPointerSize,
      // Adjust the heap size to account for a different GCPointer.
      ogHeapSize - acceptor.diagnostic.numPointer * normalGCPointerSize +
          acceptor.diagnostic.numPointer * oppositeGCPointerSize);
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
