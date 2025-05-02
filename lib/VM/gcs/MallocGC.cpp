/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/CheckedMalloc.h"
#include "hermes/Support/ErrorHandling.h"
#include "hermes/Support/SlowAssert.h"
#include "hermes/VM/CheckHeapWellFormedAcceptor.h"
#include "hermes/VM/CompressedPointer.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/HermesValue-inline.h"
#include "hermes/VM/HiddenClass.h"
#include "hermes/VM/JSWeakMapImpl.h"
#include "hermes/VM/SmallHermesValue-inline.h"

#include "llvh/Support/Debug.h"

#include <algorithm>

#define DEBUG_TYPE "gc"

namespace hermes {
namespace vm {

static const char *kGCName = "malloc";

struct MallocGC::MarkingAcceptor final : public RootAcceptor,
                                         public WeakRootAcceptor {
  MallocGC &gc;
  std::vector<CellHeader *> worklist_;

  /// markedSymbols_ represents which symbols have been proven live so far in
  /// a collection. True means that it is live, false means that it could
  /// possibly be garbage. At the end of the collection, it is guaranteed that
  /// the falses are garbage.
  llvh::BitVector markedSymbols_;

  PointerBase &pointerBase_;

  MarkingAcceptor(MallocGC &gc)
      : gc(gc),
        markedSymbols_(gc.gcCallbacks_.getSymbolsEnd()),
        pointerBase_(gc.getPointerBase()) {}

  void accept(GCCell *&cell) override {
    if (!cell) {
      return;
    }
    HERMES_SLOW_ASSERT(
        gc.validPointer(cell) &&
        "Marked a pointer that the GC didn't allocate");
    CellHeader *header = CellHeader::from(cell);
#ifdef HERMESVM_SANITIZE_HANDLES
    /// Make the acceptor idempotent: allow it to be called multiple
    /// times on the same slot during a collection.  Do this by
    /// recognizing when the pointer is already a "new" pointer.
    if (gc.newPointers_.count(header)) {
      return;
    }
    // With handle-san on, handle moving pointers here.
    if (header->isMarked()) {
      cell = header->getForwardingPointer()->data();
    } else {
      // It hasn't been seen before, move it.
      // At this point, also trim the object.
      const gcheapsize_t origSize = cell->getAllocatedSizeSlow();
      const gcheapsize_t trimmedSize =
          cell->getVT()->getTrimmedSize(cell, origSize);
      auto *newLocation =
          new (checkedMalloc(trimmedSize + sizeof(CellHeader))) CellHeader();
      newLocation->mark();
      memcpy(newLocation->data(), cell, trimmedSize);
      if (origSize != trimmedSize) {
        auto *newVarCell =
            reinterpret_cast<VariableSizeRuntimeCell *>(newLocation->data());
        newVarCell->setSizeFromGC(trimmedSize);
      }
      // Make sure to put an element on the worklist that is at the updated
      // location. Don't update the stale address that is about to be free'd.
      header->markWithForwardingPointer(newLocation);
      worklist_.push_back(newLocation);
      gc.newPointers_.insert(newLocation);
      if (gc.isTrackingIDs()) {
        gc.moveObject(
            cell,
            cell->getAllocatedSizeSlow(),
            newLocation->data(),
            trimmedSize);
      }
      cell = newLocation->data();
    }
#else
    if (!header->isMarked()) {
      // Only add to the worklist if it hasn't been marked yet.
      header->mark();
      // Trim the cell. This is fine to do with malloc'ed memory because the
      // original size is retained by malloc.
      gcheapsize_t origSize = cell->getAllocatedSizeSlow();
      gcheapsize_t newSize = cell->getVT()->getTrimmedSize(cell, origSize);
      if (newSize != origSize) {
        static_cast<VariableSizeRuntimeCell *>(cell)->setSizeFromGC(newSize);
      }
      worklist_.push_back(header);
      // Move the pointer from the old pointers to the new pointers.
      gc.pointers_.erase(header);
      gc.newPointers_.insert(header);
    }
    // Else the cell is already marked and either on the worklist or already
    // visited entirely, do nothing.
#endif
  }

  void accept(PinnedHermesValue &hv) override {
    assert((!hv.isPointer() || hv.getPointer()) && "Value is not nullable.");
    acceptHV(hv);
  }
  void acceptNullable(PinnedHermesValue &hv) override {
    acceptHV(hv);
  }
  void accept(const RootSymbolID &sym) override {
    acceptSym(sym);
  }

  void accept(GCPointerBase &ptr) {
    auto *p = ptr.get(pointerBase_);
    accept(p);
    // Update the pointer in the slot.
    ptr.setInGC(CompressedPointer::encode(p, pointerBase_));
  }
  void accept(GCHermesValueBase &hv) {
    acceptHV(hv);
  }
  void accept(GCSmallHermesValueBase &hv) {
    acceptSHV(hv);
  }
  void accept(const GCSymbolID &sym) {
    acceptSym(sym);
  }

  void acceptWeak(WeakRootBase &wr) override {
    if (!wr) {
      return;
    }
    auto *ptr = wr.getNonNullNoBarrierUnsafe(pointerBase_);
    CellHeader *header = CellHeader::from(ptr);

    // Reset weak root if target GCCell is dead.
#ifdef HERMESVM_SANITIZE_HANDLES
    ptr = header->isMarked() ? header->getForwardingPointer()->data() : nullptr;
#else
    ptr = header->isMarked() ? ptr : nullptr;
#endif
    wr = CompressedPointer::encode(ptr, pointerBase_);
  }

  void acceptWeakSym(WeakRootSymbolID &ws) override {
    if (ws.isInvalid()) {
      return;
    }
    SymbolID id = ws.getNoBarrierUnsafe();
    assert(
        id.unsafeGetIndex() < markedSymbols_.size() &&
        "Tried to mark a weak symbol not in range");
    if (!markedSymbols_[id.unsafeGetIndex()]) {
      ws = SymbolID::empty();
    }
  }

  void acceptHV(HermesValue &hv) {
    if (hv.isPointer()) {
      GCCell *ptr = static_cast<GCCell *>(hv.getPointer());
      accept(ptr);
      hv.setInGC(hv.updatePointer(ptr), gc);
    } else if (hv.isSymbol()) {
      acceptSym(hv.getSymbol());
    }
  }

  void acceptSHV(SmallHermesValue &hv) {
    if (hv.isPointer()) {
      GCCell *ptr = static_cast<GCCell *>(hv.getPointer(pointerBase_));
      accept(ptr);
      hv.setInGC(hv.updatePointer(ptr, pointerBase_), gc);
    } else if (hv.isSymbol()) {
      acceptSym(hv.getSymbol());
    }
  }

  void acceptSym(SymbolID sym) {
    if (sym.isInvalid()) {
      return;
    }
    assert(
        sym.unsafeGetIndex() < markedSymbols_.size() &&
        "Tried to mark a symbol not in range");
    markedSymbols_.set(sym.unsafeGetIndex());
  }
};

MallocGC::MallocGC(
    GCCallbacks &gcCallbacks,
    PointerBase &pointerBase,
    const GCConfig &gcConfig,
    std::shared_ptr<CrashManager> crashMgr,
    std::shared_ptr<StorageProvider> provider,
    experiments::VMExperimentFlags vmExperimentFlags)
    : GCBase(
          gcCallbacks,
          pointerBase,
          gcConfig,
          std::move(crashMgr),
          HeapKind::MallocGC),
      pointers_(),
      maxSize_(gcConfig.getMaxHeapSize()),
      sizeLimit_(gcConfig.getInitHeapSize()) {
  (void)vmExperimentFlags;
  crashMgr_->setCustomData("HermesGC", kGCName);
}

MallocGC::~MallocGC() {
  for (CellHeader *header : pointers_) {
    free(header);
  }
}

void MallocGC::collectBeforeAlloc(std::string cause, uint32_t size) {
  // Do a collection if the sanitization of handles is requested or if there
  // is memory pressure.
  collect(std::move(cause));

  // We aim for the heap to always be 50% live data, so set the target size
  // limit to double the number of bytes that survived the collection.
  uint64_t targetNewSizeLimit = (uint64_t)allocatedBytes_ * 2;
  // The heap must be at least large enough to allocate the requested object.
  uint64_t minNewSizeLimit = allocatedBytes_ + size;
  // There is not enough room after the allocation, return and let the caller
  // decide what to do.
  if (minNewSizeLimit > maxSize_)
    return;
  sizeLimit_ =
      std::clamp<uint64_t>(targetNewSizeLimit, minNewSizeLimit, maxSize_);
}

#ifdef HERMES_SLOW_DEBUG
void MallocGC::checkWellFormed() {
  GCCycle cycle{*this};
  CheckHeapWellFormedAcceptor acceptor(*this);
  DroppingAcceptor<CheckHeapWellFormedAcceptor> nameAcceptor{acceptor};
  markRoots(nameAcceptor, true);
  markWeakRoots(acceptor, /*markLongLived*/ true);
  for (CellHeader *header : pointers_) {
    GCCell *cell = header->data();
    assert(cell->isValid() && "Invalid cell encountered in heap");
    markCell(acceptor, cell);
  }

  weakMapEntrySlots_.forEach([](WeakMapEntrySlot &slot) {
    if (slot.mappedValue.isEmpty()) {
      assert(
          !slot.key ||
          !slot.owner && "Reachable entry should not have Empty value.");
    }
  });
}

void MallocGC::clearUnmarkedPropertyMaps() {
  for (CellHeader *header : pointers_)
    if (!header->isMarked())
      if (auto hc = dyn_vmcast<HiddenClass>(header->data()))
        hc->clearPropertyMap(*this);
}
#endif

void MallocGC::collect(std::string cause, bool /*canEffectiveOOM*/) {
  assert(noAllocLevel_ == 0 && "no GC allowed right now");
  using std::chrono::steady_clock;
  LLVM_DEBUG(llvh::dbgs() << "Beginning collection");
#ifdef HERMES_SLOW_DEBUG
  checkWellFormed();
#endif
  const auto wallStart = steady_clock::now();
  const auto cpuStart = oscompat::thread_cpu_time();
  const auto allocatedBefore = allocatedBytes_;
  const auto externalBefore = externalBytes_;

  resetStats();

  // Begin the collection phases.
  {
    GCCycle cycle{*this, "GC Full collection"};
    MarkingAcceptor acceptor(*this);
    DroppingAcceptor<MarkingAcceptor> nameAcceptor{acceptor};
    markRoots(nameAcceptor, true);
#ifdef HERMES_SLOW_DEBUG
    clearUnmarkedPropertyMaps();
#endif
    drainMarkStack(acceptor);

    markWeakMapEntrySlots(acceptor);

    // Now free symbols. Note that:
    // 1. We must do this before marking weak symbols, because the mark bits
    //    will be updated by this call to reflect symbols that were not actually
    //    freed.
    // 2. We must do this after we mark the WeakMap entries, because they may
    //    retain additional symbols. However, if we add support for WeakMaps
    //    with symbol keys, we will have to revisit this, because freeSymbols
    //    also updates the mark bits to reflect non-freeable symbols, which is
    //    necessary to know if a symbol key in a WeakMap is reachable.
    gcCallbacks_.freeSymbols(acceptor.markedSymbols_);

    // Update weak roots references.
    markWeakRoots(acceptor, /*markLongLived*/ true);

    // By the end of the marking loop, all pointers left in pointers_ are dead.
    for (CellHeader *header : pointers_) {
#ifndef HERMESVM_SANITIZE_HANDLES
      // If handle sanitization isn't on, these pointers should all be dead.
      assert(!header->isMarked() && "Live pointer left in dead heap section");
#endif
      GCCell *cell = header->data();
      // Extract before running any potential finalizers.
      const auto freedSize = cell->getAllocatedSizeSlow();
      // Run the finalizer if it exists and the cell is actually dead.
      if (!header->isMarked()) {
        cell->getVT()->finalizeIfExists(cell, *this);
#ifndef NDEBUG
        // Update statistics.
        if (cell->getVT()->finalize_) {
          ++numFinalizedObjects_;
        }
#endif
        // Pointers that aren't marked now weren't moved, and are dead instead.
        if (isTrackingIDs()) {
          untrackObject(cell, freedSize);
        }
      }
#ifndef NDEBUG
      // Before free'ing, fill with a dead value for debugging
      std::fill_n(reinterpret_cast<char *>(cell), freedSize, kInvalidHeapValue);
#endif
      free(header);
    }

#ifndef NDEBUG
#ifdef HERMESVM_SANITIZE_HANDLES
    // If handle sanitization is on, pointers_ is unmodified from before the
    // collection, and the number of collected objects is the difference between
    // the pointers before, and the pointers after the collection.
    assert(
        pointers_.size() >= newPointers_.size() &&
        "There cannot be more new pointers than there are old pointers");
    numCollectedObjects_ = pointers_.size() - newPointers_.size();
#else
    // If handle sanitization is not on, live pointers are removed from
    // pointers_ so the number of collected objects is equal to the size of
    // pointers_.
    numCollectedObjects_ = pointers_.size();
#endif
    numReachableObjects_ = newPointers_.size();
    numAllocatedObjects_ = newPointers_.size();
#endif
    pointers_ = std::move(newPointers_);
    assert(
        newPointers_.empty() &&
        "newPointers_ should be empty between collections");
    // Clear all the mark bits in pointers_.
    for (CellHeader *header : pointers_) {
      assert(header->isMarked() && "Should only be live pointers left");
      header->unmark();
      header->inYoungGen = false;
    }
  }

  // End of the collection phases, begin cleanup and stat recording.
#ifdef HERMES_SLOW_DEBUG
  checkWellFormed();
#endif
  // Grow the size limit if the heap is still more than 75% full.
  if (allocatedBytes_ >= sizeLimit_ * 3 / 4) {
    sizeLimit_ = std::min(maxSize_, sizeLimit_ * 2);
  }

  const auto cpuEnd = oscompat::thread_cpu_time();
  const auto wallEnd = steady_clock::now();
  auto cpuTime = cpuEnd - cpuStart;
  auto wallTime = wallEnd - wallStart;

  InternalAnalyticsEvent event{
      GCAnalyticsEvent{
          getName(),
          kGCName,
          "full",
          std::move(cause),
          std::chrono::duration_cast<std::chrono::milliseconds>(wallTime),
          std::chrono::duration_cast<std::chrono::milliseconds>(cpuTime),
          /*allocated*/ BeforeAndAfter{allocatedBefore, allocatedBytes_},
          // MallocGC only allocates memory as it is used so there is no
          // distinction between the allocated bytes and the heap size.
          /*size*/ BeforeAndAfter{allocatedBefore, allocatedBytes_},
          // TODO: MallocGC doesn't yet support credit/debit external memory, so
          // it has no data for these numbers.
          /*external*/ BeforeAndAfter{externalBefore, externalBytes_},
          /*survivalRatio*/
          allocatedBefore ? (allocatedBytes_ * 1.0) / allocatedBefore : 0,
          /*tags*/ {}},
      std::chrono::duration<double>(wallTime).count(),
      std::chrono::duration<double>(cpuTime).count()};

  recordGCStats(event, /* onMutator */ true);
  checkTripwire(allocatedBytes_ + externalBytes_);
}

void MallocGC::drainMarkStack(MarkingAcceptor &acceptor) {
  while (!acceptor.worklist_.empty()) {
    CellHeader *header = acceptor.worklist_.back();
    acceptor.worklist_.pop_back();
    assert(header->isMarked() && "Pointer on the worklist isn't marked");
    GCCell *cell = header->data();
    markCell(acceptor, cell);
    allocatedBytes_ += cell->getAllocatedSizeSlow();
  }
}

void MallocGC::markWeakMapEntrySlots(MarkingAcceptor &acceptor) {
  bool newlyMarkedValue;
  do {
    newlyMarkedValue = false;
    weakMapEntrySlots_.forEach([this, &acceptor](WeakMapEntrySlot &slot) {
      if (!slot.key || !slot.owner)
        return;
      GCCell *ownerMapCell = slot.owner.getNoBarrierUnsafe(getPointerBase());
      // If the owner structure isn't reachable, no need to mark the values.
      if (!CellHeader::from(ownerMapCell)->isMarked())
        return;
      GCCell *cell = slot.key.getNoBarrierUnsafe(getPointerBase());
      // The WeakRef object must be marked for the mapped value to
      // be marked (unless there are other strong refs to the value).
      if (!CellHeader::from(cell)->isMarked())
        return;
      acceptor.accept(slot.mappedValue);
    });
    newlyMarkedValue = !acceptor.worklist_.empty();
    drainMarkStack(acceptor);
  } while (newlyMarkedValue);

  // If either a key or its owning map is dead, set the mapped value to Empty.
  weakMapEntrySlots_.forEach([this](WeakMapEntrySlot &slot) {
    if (!slot.key || !slot.owner) {
      slot.mappedValue = HermesValue::encodeEmptyValue();
      return;
    }
    GCCell *cell = slot.key.getNoBarrierUnsafe(getPointerBase());
    GCCell *ownerMapCell = slot.owner.getNoBarrierUnsafe(getPointerBase());
    if (!CellHeader::from(ownerMapCell)->isMarked() ||
        !CellHeader::from(cell)->isMarked()) {
      slot.mappedValue = HermesValue::encodeEmptyValue();
    }
  });
}

void MallocGC::finalizeAll() {
  for (CellHeader *header : pointers_) {
    GCCell *cell = header->data();
    cell->getVT()->finalizeIfExists(cell, *this);
  }
}

void MallocGC::printStats(JSONEmitter &json) {
  GCBase::printStats(json);
  json.emitKey("specific");
  json.openDict();
  json.emitKeyValue("collector", kGCName);
  json.emitKey("stats");
  json.openDict();
  json.closeDict();
  json.closeDict();
}

std::string MallocGC::getKindAsStr() const {
  return kGCName;
}

void MallocGC::resetStats() {
#ifndef NDEBUG
  numAllocatedObjects_ = 0;
  numReachableObjects_ = 0;
  numCollectedObjects_ = 0;
  numMarkedSymbols_ = 0;
  numHiddenClasses_ = 0;
  numLeafHiddenClasses_ = 0;
#endif
  allocatedBytes_ = 0;
  numFinalizedObjects_ = 0;
}

void MallocGC::getHeapInfo(HeapInfo &info) {
  GCBase::getHeapInfo(info);
  info.allocatedBytes = allocatedBytes_;
  info.totalAllocatedBytes = totalAllocatedBytes_;
  // MallocGC does not have a heap size.
  info.heapSize = 0;
  info.externalBytes = externalBytes_;
}
void MallocGC::getHeapInfoWithMallocSize(HeapInfo &info) {
  getHeapInfo(info);
  GCBase::getHeapInfoWithMallocSize(info);
  // Note that info.mallocSizeEstimate is initialized by the call to
  // GCBase::getHeapInfoWithMallocSize.
  for (CellHeader *header : pointers_) {
    GCCell *cell = header->data();
    info.mallocSizeEstimate += cell->getVT()->getMallocSize(cell);
  }
}

void MallocGC::getCrashManagerHeapInfo(CrashManager::HeapInformation &info) {
  info.used_ = allocatedBytes_;
  // MallocGC does not have a heap size.
  info.size_ = 0;
}

void MallocGC::forAllObjs(const std::function<void(GCCell *)> &callback) {
  for (auto *ptr : pointers_) {
    callback(ptr->data());
  }
}

#ifndef NDEBUG
bool MallocGC::validPointer(const void *p) const {
  return dbgContains(p) && static_cast<const GCCell *>(p)->isValid();
}

bool MallocGC::dbgContains(const void *p) const {
  auto *ptr = reinterpret_cast<GCCell *>(const_cast<void *>(p));
  CellHeader *header = CellHeader::from(ptr);
  bool isValid = pointers_.find(header) != pointers_.end();
  isValid = isValid || newPointers_.find(header) != newPointers_.end();
  return isValid;
}

bool MallocGC::needsWriteBarrier(
    const GCHermesValueBase *loc,
    HermesValue value) const {
  return false;
}
bool MallocGC::needsWriteBarrierInCtor(
    const GCHermesValueBase *loc,
    HermesValue value) const {
  return false;
}
bool MallocGC::needsWriteBarrier(
    const GCSmallHermesValueBase *loc,
    SmallHermesValue value) const {
  return false;
}
bool MallocGC::needsWriteBarrierInCtor(
    const GCSmallHermesValueBase *loc,
    SmallHermesValue value) const {
  return false;
}
bool MallocGC::needsWriteBarrier(const GCPointerBase *loc, GCCell *value)
    const {
  return false;
}
#endif

#ifdef HERMES_MEMORY_INSTRUMENTATION
void MallocGC::createSnapshot(llvh::raw_ostream &os, bool captureNumericValue) {
  GCCycle cycle{*this};
  GCBase::createSnapshot(*this, os, captureNumericValue);
}
#endif

void MallocGC::creditExternalMemory(GCCell *, uint32_t size) {
  externalBytes_ += size;
}
void MallocGC::debitExternalMemory(GCCell *, uint32_t size) {
  externalBytes_ -= size;
}

GCCell *MallocGC::alloc(uint32_t size) {
  assert(noAllocLevel_ == 0 && "no alloc allowed right now");
  assert(
      isSizeHeapAligned(size) &&
      "Call to alloc must use a size aligned to HeapAlign");
  if (shouldSanitizeHandles()) {
    collectBeforeAlloc(kHandleSanCauseForAnalytics, size);
  }
  // Check for memory pressure conditions to do a collection.
  // Use subtraction to prevent overflow.
  if (LLVM_UNLIKELY(size > sizeLimit_ - allocatedBytes_)) {
    collectBeforeAlloc(kNaturalCauseForAnalytics, size);
  }
  // Above collection doesn't free enough memory, we can't allocate this object.
  if (LLVM_UNLIKELY(size > sizeLimit_ - allocatedBytes_)) {
    return nullptr;
  }
  // Add space for the header.
  auto *header = new (checkedMalloc(size + sizeof(CellHeader))) CellHeader();
  GCCell *mem = header->data();
  initCell(mem, size);
  // Add to the set of pointers owned by the GC.
  pointers_.insert(header);
  allocatedBytes_ += size;
  totalAllocatedBytes_ += size;
#ifndef NDEBUG
  ++numAllocatedObjects_;
#endif
  return mem;
}

} // namespace vm
} // namespace hermes
#undef DEBUG_TYPE
