/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "GCBase-WeakMap.h"
#include "hermes/Support/CheckedMalloc.h"
#include "hermes/Support/ErrorHandling.h"
#include "hermes/Support/SlowAssert.h"
#include "hermes/VM/CheckHeapWellFormedAcceptor.h"
#include "hermes/VM/CompressedPointer.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/HiddenClass.h"
#include "hermes/VM/JSWeakMapImpl.h"
#include "hermes/VM/RootAndSlotAcceptorDefault.h"
#include "hermes/VM/SmallHermesValue-inline.h"

#include "llvh/Support/Debug.h"

#include <algorithm>

#define DEBUG_TYPE "gc"

namespace hermes {
namespace vm {

static const char *kGCName = "malloc";

struct MallocGC::MarkingAcceptor final : public RootAndSlotAcceptorDefault,
                                         public WeakAcceptorDefault {
  MallocGC &gc;
  std::vector<CellHeader *> worklist_;

  /// The WeakMap objects that have been discovered to be reachable.
  std::vector<JSWeakMap *> reachableWeakMaps_;

  /// markedSymbols_ represents which symbols have been proven live so far in
  /// a collection. True means that it is live, false means that it could
  /// possibly be garbage. At the end of the collection, it is guaranteed that
  /// the falses are garbage.
  llvh::BitVector markedSymbols_;

  MarkingAcceptor(MallocGC &gc)
      : RootAndSlotAcceptorDefault(gc.getPointerBase()),
        WeakAcceptorDefault(gc.getPointerBase()),
        gc(gc),
        markedSymbols_(gc.gcCallbacks_.getSymbolsEnd()) {}

  using RootAndSlotAcceptorDefault::accept;

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
      const gcheapsize_t origSize = cell->getAllocatedSize();
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
      auto *newCell = newLocation->data();
      if (vmisa<JSWeakMap>(newCell)) {
        reachableWeakMaps_.push_back(vmcast<JSWeakMap>(newCell));
      } else {
        worklist_.push_back(newLocation);
      }
      gc.newPointers_.insert(newLocation);
      if (gc.isTrackingIDs()) {
        gc.moveObject(
            cell, cell->getAllocatedSize(), newLocation->data(), trimmedSize);
      }
      cell = newLocation->data();
    }
#else
    if (!header->isMarked()) {
      // Only add to the worklist if it hasn't been marked yet.
      header->mark();
      // Trim the cell. This is fine to do with malloc'ed memory because the
      // original size is retained by malloc.
      gcheapsize_t origSize = cell->getAllocatedSize();
      gcheapsize_t newSize = cell->getVT()->getTrimmedSize(cell, origSize);
      if (newSize != origSize) {
        static_cast<VariableSizeRuntimeCell *>(cell)->setSizeFromGC(newSize);
      }
      if (vmisa<JSWeakMap>(cell)) {
        reachableWeakMaps_.push_back(vmcast<JSWeakMap>(cell));
      } else {
        worklist_.push_back(header);
      }
      // Move the pointer from the old pointers to the new pointers.
      gc.pointers_.erase(header);
      gc.newPointers_.insert(header);
    }
    // Else the cell is already marked and either on the worklist or already
    // visited entirely, do nothing.
#endif
  }

  void acceptWeak(GCCell *&ptr) override {
    if (ptr == nullptr) {
      return;
    }
    CellHeader *header = CellHeader::from(ptr);

    // Reset weak root if target GCCell is dead.
#ifdef HERMESVM_SANITIZE_HANDLES
    ptr = header->isMarked() ? header->getForwardingPointer()->data() : nullptr;
#else
    ptr = header->isMarked() ? ptr : nullptr;
#endif
  }

  void acceptHV(HermesValue &hv) override {
    if (hv.isPointer()) {
      GCCell *ptr = static_cast<GCCell *>(hv.getPointer());
      accept(ptr);
      hv.setInGC(hv.updatePointer(ptr), gc);
    } else if (hv.isSymbol()) {
      acceptSym(hv.getSymbol());
    }
  }

  void acceptSHV(SmallHermesValue &hv) override {
    if (hv.isPointer()) {
      GCCell *ptr = static_cast<GCCell *>(hv.getPointer(pointerBase_));
      accept(ptr);
      hv.setInGC(hv.updatePointer(ptr, pointerBase_), gc);
    } else if (hv.isSymbol()) {
      acceptSym(hv.getSymbol());
    }
  }

  void acceptSym(SymbolID sym) override {
    if (sym.isInvalid()) {
      return;
    }
    assert(
        sym.unsafeGetIndex() < markedSymbols_.size() &&
        "Tried to mark a symbol not in range");
    markedSymbols_.set(sym.unsafeGetIndex());
  }

  void accept(WeakRefBase &wr) override {
    wr.unsafeGetSlot()->mark();
  }
};

gcheapsize_t MallocGC::Size::storageFootprint() const {
  // MallocGC uses no storage from the StorageProvider.
  return 0;
}

gcheapsize_t MallocGC::Size::minStorageFootprint() const {
  // MallocGC uses no storage from the StorageProvider.
  return 0;
}

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
      maxSize_(Size(gcConfig).max()),
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
  const auto growSizeLimit = [this, size](gcheapsize_t sizeLimit) {
    // Either double the size limit, or increase to size, at a max of maxSize_.
    return std::min(maxSize_, std::max(sizeLimit * 2, size));
  };
  if (size > sizeLimit_) {
    sizeLimit_ = growSizeLimit(sizeLimit_);
  }
  if (size > maxSize_) {
    // No way to handle the allocation no matter what.
    oom(make_error_code(OOMError::MaxHeapReached));
  }
  assert(
      size <= sizeLimit_ &&
      "Should be guaranteed not to be asking for more space than the heap can "
      "provide");
  // Check for memory pressure conditions to do a collection.
  // Use subtraction to prevent overflow.
#ifndef HERMESVM_SANITIZE_HANDLES
  if (allocatedBytes_ < sizeLimit_ - size) {
    return;
  }
#endif
  // Do a collection if the sanitization of handles is requested or if there
  // is memory pressure.
  collect(std::move(cause));
  // While we still can't fill the allocation, keep growing.
  while (allocatedBytes_ >= sizeLimit_ - size) {
    if (sizeLimit_ == maxSize_) {
      // Can't grow memory any higher, OOM.
      oom(make_error_code(OOMError::MaxHeapReached));
    }
    sizeLimit_ = growSizeLimit(sizeLimit_);
  }
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
    markCell(cell, acceptor);
  }
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

    // The marking loop above will have accumulated WeakMaps;
    // find things reachable from values of reachable keys.
    completeWeakMapMarking(acceptor);

    // Update weak roots references.
    markWeakRoots(acceptor, /*markLongLived*/ true);

    // Update and remove weak references.
    updateWeakReferences();
    resetWeakReferences();
    // Free the unused symbols.
    gcCallbacks_.freeSymbols(acceptor.markedSymbols_);
    // By the end of the marking loop, all pointers left in pointers_ are dead.
    for (CellHeader *header : pointers_) {
#ifndef HERMESVM_SANITIZE_HANDLES
      // If handle sanitization isn't on, these pointers should all be dead.
      assert(!header->isMarked() && "Live pointer left in dead heap section");
#endif
      GCCell *cell = header->data();
      // Extract before running any potential finalizers.
      const auto freedSize = cell->getAllocatedSize();
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

  GCAnalyticsEvent event{
      getName(),
      kGCName,
      "full",
      std::move(cause),
      std::chrono::duration_cast<std::chrono::milliseconds>(
          wallEnd - wallStart),
      std::chrono::duration_cast<std::chrono::milliseconds>(cpuEnd - cpuStart),
      /*allocated*/ BeforeAndAfter{allocatedBefore, allocatedBytes_},
      // MallocGC only allocates memory as it is used so there is no distinction
      // between the allocated bytes and the heap size.
      /*size*/ BeforeAndAfter{allocatedBefore, allocatedBytes_},
      // TODO: MallocGC doesn't yet support credit/debit external memory, so
      // it has no data for these numbers.
      /*external*/ BeforeAndAfter{externalBefore, externalBytes_},
      /*survivalRatio*/
      allocatedBefore ? (allocatedBytes_ * 1.0) / allocatedBefore : 0,
      /*tags*/ {}};

  recordGCStats(event, /* onMutator */ true);
  checkTripwire(allocatedBytes_ + externalBytes_);
}

void MallocGC::drainMarkStack(MarkingAcceptor &acceptor) {
  while (!acceptor.worklist_.empty()) {
    CellHeader *header = acceptor.worklist_.back();
    acceptor.worklist_.pop_back();
    assert(header->isMarked() && "Pointer on the worklist isn't marked");
    GCCell *cell = header->data();
    markCell(cell, acceptor);
    allocatedBytes_ += cell->getAllocatedSize();
  }
}

void MallocGC::completeWeakMapMarking(MarkingAcceptor &acceptor) {
  gcheapsize_t weakMapAllocBytes = GCBase::completeWeakMapMarking(
      *this,
      acceptor,
      acceptor.reachableWeakMaps_,
      /*objIsMarked*/
      [](GCCell *cell) { return CellHeader::from(cell)->isMarked(); },
      /*markFromVal*/
      [this, &acceptor](GCCell *valCell, GCHermesValue &valRef) {
        CellHeader *valHeader = CellHeader::from(valCell);
        if (valHeader->isMarked()) {
#ifdef HERMESVM_SANITIZE_HANDLES
          valRef.setInGC(
              HermesValue::encodeObjectValue(
                  valHeader->getForwardingPointer()->data()),
              *this);
#endif
          return false;
        }
        acceptor.accept(valRef);
        drainMarkStack(acceptor);
        return true;
      },
      /*drainMarkStack*/
      [this](MarkingAcceptor &acceptor) { drainMarkStack(acceptor); },
      /*checkMarkStackOverflow (MallocGC does not have mark stack overflow)*/
      []() { return false; });

  acceptor.reachableWeakMaps_.clear();
  // drainMarkStack will have added the size of every object popped
  // from the mark stack.  WeakMaps are never pushed on that stack,
  // but the call above returns their total size.  So add that.
  allocatedBytes_ += weakMapAllocBytes;
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

void MallocGC::resetWeakReferences() {
  for (auto &slot : weakSlots_) {
    // Set all allocated slots to unmarked.
    if (slot.state() == WeakSlotState::Marked)
      slot.unmark();
  }
}

void MallocGC::updateWeakReferences() {
  for (auto &slot : weakSlots_) {
    if (slot.state() == WeakSlotState::Unmarked) {
      freeWeakSlot(&slot);
    }
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

bool MallocGC::needsWriteBarrier(void *loc, GCCell *value) {
  return false;
}
#endif

#ifdef HERMES_MEMORY_INSTRUMENTATION
void MallocGC::createSnapshot(llvh::raw_ostream &os) {
  GCCycle cycle{*this};
  GCBase::createSnapshot(*this, os);
}
#endif

void MallocGC::creditExternalMemory(GCCell *, uint32_t size) {
  externalBytes_ += size;
}
void MallocGC::debitExternalMemory(GCCell *, uint32_t size) {
  externalBytes_ -= size;
}

/// @name Forward instantiations
/// @{

template void *MallocGC::alloc</*FixedSize*/ true, HasFinalizer::Yes>(
    uint32_t size);
template void *MallocGC::alloc</*FixedSize*/ false, HasFinalizer::Yes>(
    uint32_t size);
template void *MallocGC::alloc</*FixedSize*/ true, HasFinalizer::No>(
    uint32_t size);
template void *MallocGC::alloc</*FixedSize*/ false, HasFinalizer::No>(
    uint32_t size);
/// @}

} // namespace vm
} // namespace hermes
#undef DEBUG_TYPE
