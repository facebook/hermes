/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/CheckedMalloc.h"
#include "hermes/Support/ErrorHandling.h"
#include "hermes/Support/SlowAssert.h"
#include "hermes/VM/CheckHeapWellFormedAcceptor.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HermesValue-inline.h"
#include "hermes/VM/HiddenClass.h"
#include "hermes/VM/JSWeakMapImpl.h"
#include "hermes/VM/SlotAcceptorDefault.h"

#include "llvh/Support/Debug.h"

#include <algorithm>

#define DEBUG_TYPE "gc"

namespace hermes {
namespace vm {

struct MallocGC::MarkingAcceptor final : public SlotAcceptorDefault,
                                         public WeakRootAcceptorDefault {
  std::vector<CellHeader *> worklist_;

  /// The WeakMap objects that have been discovered to be reachable.
  std::vector<JSWeakMap *> reachableWeakMaps_;

  /// markedSymbols_ represents which symbols have been proven live so far in
  /// a collection. True means that it is live, false means that it could
  /// possibly be garbage. At the end of the collection, it is guaranteed that
  /// the falses are garbage.
  std::vector<bool> markedSymbols_;

  MarkingAcceptor(GC &gc)
      : SlotAcceptorDefault(gc), WeakRootAcceptorDefault(gc) {
    markedSymbols_.resize(gc.gcCallbacks_->getSymbolsEnd(), false);
  }

  using SlotAcceptorDefault::accept;

  void accept(void *&ptr) override {
    if (!ptr) {
      return;
    }
    GCCell *&cell = reinterpret_cast<GCCell *&>(ptr);
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
      const bool canBeTrimmed = cell->getVT()->canBeTrimmed();
      const gcheapsize_t trimmedSize =
          cell->getVT()->getTrimmedSize(cell, cell->getAllocatedSize());
      auto *newLocation =
          new (checkedMalloc(trimmedSize + sizeof(CellHeader))) CellHeader();
      newLocation->mark();
      memcpy(newLocation->data(), cell, trimmedSize);
      if (canBeTrimmed) {
        auto *newVarCell =
            reinterpret_cast<VariableSizeRuntimeCell *>(newLocation->data());
        newVarCell->setSizeFromGC(trimmedSize);
        newVarCell->getVT()->trim(newVarCell);
      }
      // Make sure to put an element on the worklist that is at the updated
      // location. Don't update the stale address that is about to be free'd.
      header->markWithForwardingPointer(newLocation);
      auto *newCell = newLocation->data();
      if (newCell->getKind() == CellKind::WeakMapKind) {
        reachableWeakMaps_.push_back(vmcast<JSWeakMap>(newCell));
      } else {
        worklist_.push_back(newLocation);
      }
      gc.newPointers_.insert(newLocation);
      if (gc.isTrackingIDs()) {
        gc.idTracker_.moveObject(cell, newLocation->data());
        gc.allocationLocationTracker_.moveAlloc(cell, newLocation->data());
      }
      cell = newLocation->data();
    }
#else
    if (!header->isMarked()) {
      // Only add to the worklist if it hasn't been marked yet.
      header->mark();
      // Trim the cell. This is fine to do with malloc'ed memory because the
      // original size is retained by malloc.
      if (cell->getVT()->canBeTrimmed()) {
        cell->getVT()->trim(cell);
      }
      if (cell->getKind() == CellKind::WeakMapKind) {
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

  void acceptWeak(void *&ptr) override {
    if (ptr == nullptr) {
      return;
    }
    auto *cell = reinterpret_cast<GCCell *>(ptr);
    CellHeader *header = CellHeader::from(cell);

    // Reset weak root if target GCCell is dead.
#ifdef HERMESVM_SANITIZE_HANDLES
    ptr = header->isMarked() ? header->getForwardingPointer()->data() : nullptr;
#else
    ptr = header->isMarked() ? ptr : nullptr;
#endif
  }

  void accept(HermesValue &hv) override {
    if (hv.isPointer()) {
      void *ptr = hv.getPointer();
      accept(ptr);
      hv.setInGC(hv.updatePointer(ptr), &gc);
    } else if (hv.isSymbol()) {
      accept(hv.getSymbol());
    }
  }

  void accept(SymbolID sym) override {
    if (sym.isInvalid()) {
      return;
    }
    assert(
        sym.unsafeGetIndex() < markedSymbols_.size() &&
        "Tried to mark a symbol not in range");
    markedSymbols_[sym.unsafeGetIndex()] = true;
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
      pointers_(),
      weakPointers_(),
      maxSize_(Size(gcConfig).max()),
      sizeLimit_(gcConfig.getInitHeapSize()) {}

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
  GCCycle cycle{this};
  CheckHeapWellFormedAcceptor acceptor(*this);
  DroppingAcceptor<CheckHeapWellFormedAcceptor> nameAcceptor{acceptor};
  markRoots(nameAcceptor, true);
  markWeakRoots(acceptor);
  for (CellHeader *header : pointers_) {
    GCCell *cell = header->data();
    assert(cell->isValid() && "Invalid cell encountered in heap");
    GCBase::markCell(cell, this, acceptor);
  }
}

void MallocGC::clearUnmarkedPropertyMaps() {
  for (CellHeader *header : pointers_)
    if (!header->isMarked())
      if (auto hc = dyn_vmcast<HiddenClass>(header->data()))
        hc->clearPropertyMap(this);
}
#endif

void MallocGC::collect(std::string cause) {
  assert(noAllocLevel_ == 0 && "no GC allowed right now");
  using std::chrono::steady_clock;
  LLVM_DEBUG(llvh::dbgs() << "Beginning collection");
#ifdef HERMES_SLOW_DEBUG
  checkWellFormed();
#endif
  const auto wallStart = steady_clock::now();
  const auto cpuStart = oscompat::thread_cpu_time();
  auto allocatedBefore = allocatedBytes_;

  resetStats();

  // Begin the collection phases.
  {
    GCCycle cycle{this, gcCallbacks_, "Full collection"};
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
    markWeakRoots(acceptor);

    // Update and remove weak references.
    updateWeakReferences();
    resetWeakReferences();
    // Free the unused symbols.
    gcCallbacks_->freeSymbols(acceptor.markedSymbols_);
    if (idTracker_.isTrackingIDs()) {
      idTracker_.untrackUnmarkedSymbols(acceptor.markedSymbols_);
    }
    // By the end of the marking loop, all pointers left in pointers_ are dead.
    for (CellHeader *header : pointers_) {
#ifndef HERMESVM_SANITIZE_HANDLES
      // If handle sanitization isn't on, these pointers should all be dead.
      assert(!header->isMarked() && "Live pointer left in dead heap section");
#endif
      GCCell *cell = header->data();
#ifndef NDEBUG
      // Extract before running any potential finalizers.
      const auto freedSize = cell->getAllocatedSize();
#endif
      // Run the finalizer if it exists and the cell is actually dead.
      if (!header->isMarked()) {
        cell->getVT()->finalizeIfExists(cell, this);
#ifndef NDEBUG
        // Update statistics.
        if (cell->getVT()->finalize_) {
          ++numFinalizedObjects_;
        }
#endif
      }
      if (idTracker_.isTrackingIDs() ||
          allocationLocationTracker_.isEnabled()) {
        idTracker_.untrackObject(cell);
        allocationLocationTracker_.freeAlloc(cell);
      }
      if (allocationLocationTracker_.isEnabled()) {
        allocationLocationTracker_.freeAlloc(cell);
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
      "malloc",
      "full",
      std::move(cause),
      std::chrono::duration_cast<std::chrono::milliseconds>(
          wallEnd - wallStart),
      std::chrono::duration_cast<std::chrono::milliseconds>(cpuEnd - cpuStart),
      // MallocGC only allocates memory as it is used so there is no distinction
      // between the allocated bytes and the heap size.
      /*preAllocated*/ allocatedBefore,
      /*preSize*/ allocatedBefore,
      /*postAllocated*/ allocatedBytes_,
      /*postSize*/ allocatedBytes_,
      /*survivalRatio*/
      allocatedBefore ? (allocatedBytes_ * 1.0) / allocatedBefore : 0};

  recordGCStats(event);
  checkTripwire(allocatedBytes_);
}

void MallocGC::drainMarkStack(MarkingAcceptor &acceptor) {
  while (!acceptor.worklist_.empty()) {
    CellHeader *header = acceptor.worklist_.back();
    acceptor.worklist_.pop_back();
    assert(header->isMarked() && "Pointer on the worklist isn't marked");
    GCCell *cell = header->data();
    GCBase::markCell(cell, this, acceptor);
    allocatedBytes_ += cell->getAllocatedSize();
  }
}

void MallocGC::completeWeakMapMarking(MarkingAcceptor &acceptor) {
  gcheapsize_t weakMapAllocBytes = GCBase::completeWeakMapMarking(
      this,
      acceptor,
      acceptor.reachableWeakMaps_,
      /*objIsMarked*/
      [](GCCell *cell) { return CellHeader::from(cell)->isMarked(); },
      /*markFromVal*/
      [this, &acceptor](GCCell *valCell, HermesValue &valRef) {
        CellHeader *valHeader = CellHeader::from(valCell);
        if (valHeader->isMarked()) {
#ifdef HERMESVM_SANITIZE_HANDLES
          valRef.setInGC(
              HermesValue::encodeObjectValue(
                  valHeader->getForwardingPointer()->data()),
              this);
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
    cell->getVT()->finalizeIfExists(cell, this);
  }
}

void MallocGC::printStats(JSONEmitter &json) {
  GCBase::printStats(json);
  json.emitKey("specific");
  json.openDict();
  json.emitKeyValue("collector", "malloc");
  json.emitKey("stats");
  json.openDict();
  json.closeDict();
  json.closeDict();
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
}
void MallocGC::getHeapInfoWithMallocSize(HeapInfo &info) {
  getHeapInfo(info);
  info.mallocSizeEstimate = 0;
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

#ifndef NDEBUG
size_t MallocGC::countUsedWeakRefs() const {
  size_t count = 0;
  for (auto &slot : weakPointers_) {
    if (slot.state() != WeakSlotState::Free) {
      ++count;
    }
  }
  return count;
}
#endif

void MallocGC::forAllObjs(const std::function<void(GCCell *)> &callback) {
  for (auto *ptr : pointers_) {
    callback(ptr->data());
  }
}

void MallocGC::resetWeakReferences() {
  for (auto &slot : weakPointers_) {
    // Set all allocated slots to unmarked.
    if (slot.state() == WeakSlotState::Marked)
      slot.unmark();
  }
}

void MallocGC::updateWeakReferences() {
  for (auto &slot : weakPointers_) {
    switch (slot.state()) {
      case WeakSlotState::Free:
        break;
      case WeakSlotState::Unmarked:
        freeWeakSlot(&slot);
        break;
      case WeakSlotState::Marked:
        // If it's not a pointer, nothing to do.
        if (!slot.hasPointer()) {
          break;
        }
        auto *cell = reinterpret_cast<GCCell *>(slot.getPointer());
        HERMES_SLOW_ASSERT(
            validPointer(cell) &&
            "Got a pointer out of a weak reference slot that is not owned by "
            "the GC");
        CellHeader *header = CellHeader::from(cell);
        if (!header->isMarked()) {
          // This pointer is no longer live, zero it out
          slot.clearPointer();
        } else {
#ifdef HERMESVM_SANITIZE_HANDLES
          // Update the value to point to the new location
          GCCell *nextCell = header->getForwardingPointer()->data();
          HERMES_SLOW_ASSERT(
              validPointer(cell) &&
              "Forwarding weak ref must be to a valid cell");
          slot.setPointer(nextCell);
#endif
        }
        break;
    }
  }
}

WeakRefSlot *MallocGC::allocWeakSlot(HermesValue init) {
  weakPointers_.push_back({init});
  return &weakPointers_.back();
}

void MallocGC::freeWeakSlot(WeakRefSlot *slot) {
  slot->free(nullptr);
}

#ifndef NDEBUG
bool MallocGC::validPointer(const void *p) const {
  auto *ptr = reinterpret_cast<GCCell *>(const_cast<void *>(p));
  CellHeader *header = CellHeader::from(ptr);
  bool isValid = pointers_.find(header) != pointers_.end();
  isValid = isValid || newPointers_.find(header) != newPointers_.end();
  return isValid;
}

bool MallocGC::isMostRecentFinalizableObj(const GCCell *cell) const {
  // We don't keep track of the sequence of finalizable objects in
  // MallocGC; rather, it looks directly at whether a freed cell's vtable has
  // a finalizer method. So we just return whether \p cell has a finalizer.
  // This won't detect errors, but it also won't give false positives.
  return cell->getVT()->finalize_ != nullptr;
}
#endif

void MallocGC::createSnapshot(llvh::raw_ostream &os) {
  GCCycle cycle{this};
  GCBase::createSnapshot(this, os);
}

#ifdef HERMESVM_SERIALIZE
void MallocGC::serializeWeakRefs(Serializer &s) {
  hermes_fatal("serializeWeakRefs not implemented for current GC");
}

void MallocGC::deserializeWeakRefs(Deserializer &d) {
  hermes_fatal("deserializeWeakRefs not implemented for current GC");
}

void MallocGC::serializeHeap(Serializer &s) {
  hermes_fatal("serializeHeap not implemented for current GC");
}

void MallocGC::deserializeHeap(Deserializer &d) {
  hermes_fatal("serializeHeap not implemented for current GC");
}

void MallocGC::deserializeStart() {
  hermes_fatal("Serialization/Deserialization not allowed with MallocGC");
}

void MallocGC::deserializeEnd() {
  hermes_fatal("Serialization/Deserialization not allowed with MallocGC");
}
#endif

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
