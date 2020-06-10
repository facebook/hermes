/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/HadesGC.h"
#include "hermes/VM/AllocResult.h"
#include "hermes/VM/CheckHeapWellFormedAcceptor.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/SlotAcceptorDefault-inline.h"
#include "hermes/VM/SlotAcceptorDefault.h"

#include <stack>

namespace hermes {
namespace vm {

/// Similar to AlignedHeapSegment except it uses a free list.
class HadesGC::HeapSegment final : public AlignedHeapSegment {
 public:
  explicit HeapSegment(AlignedStorage &&storage, bool bumpAllocMode);
  ~HeapSegment() = default;

  /// Allocates space to place an object of size \p sz.
  /// \param sz The amount of memory, in bytes, required for this allocation.
  /// \return A pointer into this segment that can have \p sz bytes written into
  ///   it. If no space is available to satisfy this request, null is
  ///   returned.
  /// \post If this allocation succeeds, a GCCell must be constructed in it
  ///   before the next allocation occurs.
  AllocResult alloc(uint32_t sz);

  /// Allocate space by bumping a limit.
  /// NOTE: Usable only by YG.
  AllocResult bumpAlloc(uint32_t sz);

  /// Adds the given cell to the free list for this segment.
  /// \pre this->contains(cell) is true.
  void addCellToFreelist(GCCell *cell);

  /// Record the head of this cell so it can be found by the card scanner.
  void setCellHead(const GCCell *cell);

  /// For a given address, find the head of the cell.
  /// \return A cell such that cell <= address < cell->nextCell().
  GCCell *getCellHead(const void *address);

  /// Call \p callback on every cell allocated in this segment.
  /// NOTE: Overridden to skip free list entries.
  template <typename CallbackFunction>
  void forAllObjs(CallbackFunction callback);
  template <typename CallbackFunction>
  void forAllObjs(CallbackFunction callback) const;

  /// \return the number of bytes in this segment that are in active use by the
  /// program, and are not part of free cells.
  uint64_t allocatedBytes() const {
    return allocatedBytes_;
  }

  class FreelistCell final : public VariableSizeRuntimeCell {
   private:
    static const VTable vt;

   public:
    // If null, this is the tail of the free list.
    FreelistCell *next_;

    explicit FreelistCell(uint32_t sz, FreelistCell *next)
        : VariableSizeRuntimeCell{&vt, sz}, next_{next} {}

    /// Split this cell into two FreelistCells. The first cell will be the
    /// requested size \p sz, and will have a next pointer to the second cell.
    /// The second cell will have the remainder that was left from the original.
    /// \param sz The size that the newly-split cell should be.
    /// \pre getAllocatedSize() >= sz + minAllocationSize()
    /// \post this will now point to the first cell, but without modifying this.
    ///   this should no longer be used as a FreelistCell, and something else
    ///   should be constructed into it immediately.
    FreelistCell *split(HeapSegment &seg, uint32_t sz);

    static bool classof(const GCCell *cell) {
      return cell->getKind() == CellKind::FreelistKind;
    }
  };

 private:
  /// Head of the free list. Null if the free list is empty.
  FreelistCell *freelistHead_;

  uint64_t allocatedBytes_{0};
  bool bumpAllocMode_;
};

// A free list cell is always variable-sized.
const VTable HadesGC::HeapSegment::FreelistCell::vt{CellKind::FreelistKind,
                                                    /*variableSize*/ 0};

void FreelistBuildMeta(const GCCell *, Metadata::Builder &) {}

HadesGC::HeapSegment::HeapSegment(AlignedStorage &&storage, bool bumpAllocMode)
    : AlignedHeapSegment{std::move(storage)}, bumpAllocMode_{bumpAllocMode} {
  // Make sure end() is at the maxSize.
  growToLimit();
  if (bumpAllocMode) {
    return;
  }
  // The segment starts off as one large free cell.
  const uint32_t sz = end() - start();
  AllocResult res = AlignedHeapSegment::alloc(sz);
  assert(res.success && "Failed to bump the level to the end");
  freelistHead_ = new (res.ptr) FreelistCell(sz, nullptr);
  assert(freelistHead_->isValid() && "Invalid free list head");

  setCellHead(freelistHead_);
  // Here, and in other places where FreelistCells are poisoned, use +1 on the
  // pointer to skip towards the memory region directly after the FreelistCell
  // header of a cell. This way the header is always intact and readable, and
  // only the contents of the cell are poisoned.
  __asan_poison_memory_region(freelistHead_ + 1, sz - sizeof(FreelistCell));
}

AllocResult HadesGC::HeapSegment::alloc(uint32_t sz) {
  assert(
      !bumpAllocMode_ && "Shouldn't use bumpAlloc except on specific segments");
  sz = heapAlignSize(sz);
  assert(
      sz >= minAllocationSize() &&
      "Allocating too small of an object into old gen");
  // Need to track the previous entry in order to change the next pointer.
  FreelistCell **prevLoc = &freelistHead_;
  FreelistCell *cell = freelistHead_;
  // TODO: This algorithm is first-fit. The world of free-list allocation
  // algorithms is vast, and the best one can be explored later.
  while (true) {
    if (!cell) {
      // Free list exhausted, and nothing had enough space.
      return {nullptr, false};
    }
    assert(
        vmisa<FreelistCell>(cell) &&
        "Non-free-list cell found in the free list");
    assert(
        (!cell->next_ || cell->next_->isValid()) &&
        "Next pointer points to an invalid cell");
    // Check if the size is large enough that the cell could be split.
    if (cell->getAllocatedSize() >= sz + minAllocationSize()) {
      // Split the free cell. In order to avoid initializing soon-to-be-unused
      // values like the size and the next pointer, copy the return path here.
      *prevLoc = cell->split(*this, sz);
      allocatedBytes_ += sz;
      // Unpoison the memory so that the mutator can use it.
      __asan_unpoison_memory_region(cell + 1, sz - sizeof(FreelistCell));
      return {cell, true};
    } else if (cell->getAllocatedSize() == sz) {
      // Exact match, take it.
      break;
    }
    // Non-exact matches, or anything just barely too small to fit, will need
    // to find another block.
    // NOTE: This is due to restrictions on the minimum cell size to keep the
    // heap parseable, especially in debug mode. If this minimum size becomes
    // smaller (smaller header, size becomes part of it automatically, debug
    // magic field is handled differently), this decisions can be re-examined.
    // An example alternative is to make a special fixed-size cell that is only
    // as big as an empty GCCell. That alternative only works if the empty
    // is small enough to fit in any gap in the heap. That's not true in debug
    // modes currently.
    prevLoc = &cell->next_;
    cell = cell->next_;
  }
  assert(
      cell->getAllocatedSize() == sz &&
      "Cell found in free list should have exactly enough bytes to contain the "
      "allocation");
  assert(
      (!cell->next_ || cell->next_->isValid()) &&
      "Next pointer points to an invalid cell");

  // Remove from free list.
  *prevLoc = cell->next_;
  // Track the number of allocated bytes in a segment.
  allocatedBytes_ += sz;
  // Unpoison the memory so that the mutator can use it.
  __asan_unpoison_memory_region(cell + 1, sz - sizeof(FreelistCell));
  // Could overwrite the VTable, but the allocator will write a new one in
  // anyway.
  return {cell, true};
}

AllocResult HadesGC::HeapSegment::bumpAlloc(uint32_t sz) {
  assert(
      bumpAllocMode_ && "Shouldn't use bumpAlloc except on specific segments");
  // Don't use a free list for bump allocation.
  // NOTE: This is only used for the YG segment.
  return AlignedHeapSegment::alloc(sz);
}

void HadesGC::HeapSegment::addCellToFreelist(GCCell *cell) {
  assert(contains(cell) && "This segment doesn't contain the cell");
  // Turn this into a FreelistCell by constructing in-place.
  const auto sz = cell->getAllocatedSize();
  assert(
      sz >= sizeof(FreelistCell) &&
      "Cannot construct a FreelistCell into an allocation in the OG");
  assert(
      allocatedBytes_ >= sz &&
      "Free'ing a cell that is larger than what is left allocated");
  // TODO: For concurrent access it's probably better to append to the
  // tail instead. Only requires writing a single next pointer instead
  // of the two-phase head swap + next pointer change.
  auto *const newFreeCell = new (cell) FreelistCell{sz, freelistHead_};
  freelistHead_ = newFreeCell;
  // We free'd this many bytes.
  allocatedBytes_ -= sz;
  // In ASAN builds, poison the memory outside of the FreelistCell so that
  // accesses are flagged as illegal.
  __asan_poison_memory_region(newFreeCell + 1, sz - sizeof(FreelistCell));
}

void HadesGC::HeapSegment::setCellHead(const GCCell *cell) {
  cellHeads().mark(cellHeads().addressToIndex(cell));
}

GCCell *HadesGC::HeapSegment::getCellHead(const void *address) {
  MarkBitArrayNC &heads = cellHeads();
  auto ind = heads.addressToIndex(address);
  // Go backwards from the current address looking for a marked bit, which means
  // that the address contains a GCCell header.
  // TODO: Optimize this with zero scanning.
  while (!heads.at(ind)) {
    // There is guaranteed to be a marked cell head before ind reaches 0,
    // because the intial free list creation of the segment sets the first head.
    assert(ind && "About to walk off the end of the object heads table");
    --ind;
  }
  GCCell *cell = reinterpret_cast<GCCell *>(heads.indexToAddress(ind));
  assert(
      cell->isValid() && "Object heads table doesn't point to a valid object");
  return cell;
}

template <typename CallbackFunction>
void HadesGC::HeapSegment::forAllObjs(CallbackFunction callback) {
  // YG doesn't have a FillerCell at the end.
  void *const stop = bumpAllocMode_ ? level() : end();
  for (GCCell *cell = reinterpret_cast<GCCell *>(start()); cell < stop;
       cell = cell->nextCell()) {
    // Skip free-list entries.
    if (!vmisa<FreelistCell>(cell)) {
      callback(cell);
    }
  }
}

HadesGC::HeapSegment::FreelistCell *HadesGC::HeapSegment::FreelistCell::split(
    HeapSegment &seg,
    uint32_t sz) {
  const auto origSize = getAllocatedSize();
  assert(
      origSize >= sz + minAllocationSize() &&
      "Can't split if it would leave too small of a second cell");
  char *nextCellAddress = reinterpret_cast<char *>(this) + sz;
  // We're about to touch some memory in the newly split cell.
  // All other memory should remain poisoned.
  __asan_unpoison_memory_region(nextCellAddress, sizeof(FreelistCell));
  // Construct a new FreelistCell in the empty space.
  FreelistCell *const newCell =
      new (nextCellAddress) FreelistCell(origSize - sz, next_);
#ifndef NDEBUG
  const auto newSize = newCell->getAllocatedSize();
  assert(
      isSizeHeapAligned(newSize) && newSize >= minAllocationSize() &&
      "Invalid size for a split cell");
  assert(newSize + sz == origSize && "Space was wasted during a split");
#endif
  // TODO: Right now the card table boundaries are unused, because creating all
  // of them is too expensive on every split, especially if the free list cell
  // is huge (such as after compaction).
  // Some reasonable options to speed this up:
  // * Split free list cells at higher addresses instead of the lower addresses.
  //    This requires updating fewer card table boundaries.
  // * If a split cell is huge, consider updating only the closest boundaries,
  //    taking advantage of the exponential encoding.
  // Using cell heads as a MarkBitArray was chosen because it's the simplest
  // code that is correct, and under the assumption that searching for the head
  // of a cell extending into a dirty card is not a critical operation.
  // This is the only operation in a segment that actually creates new cells,
  // all other cells are already present.
  seg.setCellHead(newCell);
  return newCell;
}

class HadesGC::CollectionSection final {
 public:
  CollectionSection(HadesGC *gc);
  ~CollectionSection();

 private:
  HadesGC *gc_;
  GCCycle cycle_;
  uint64_t usedBefore_;
  TimePoint wallStart_;
  std::chrono::microseconds cpuStart_;
  std::chrono::microseconds wallElapsed_;
  std::chrono::microseconds cpuElapsed_;
};

HadesGC::CollectionSection::CollectionSection(HadesGC *gc)
    : gc_{gc},
      cycle_{gc},
      usedBefore_{gc->allocatedBytes()},
      wallStart_{std::chrono::steady_clock::now()},
      cpuStart_{oscompat::thread_cpu_time()} {
#ifdef HERMES_SLOW_DEBUG
  gc_->checkWellFormed();
#endif
}

HadesGC::CollectionSection::~CollectionSection() {
  wallElapsed_ = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::steady_clock::now() - wallStart_);
  cpuElapsed_ = oscompat::thread_cpu_time() - cpuStart_;

  std::chrono::duration<double> wallElapsedSeconds = wallElapsed_;
  std::chrono::duration<double> cpuElapsedSeconds = cpuElapsed_;
  gc_->recordGCStats(
      wallElapsedSeconds.count(),
      cpuElapsedSeconds.count(),
      0,
      usedBefore_,
      gc_->allocatedBytes());
#ifdef HERMES_SLOW_DEBUG
  gc_->checkWellFormed();
#endif
}

class HadesGC::EvacAcceptor final : public SlotAcceptorDefault {
 public:
  struct CopyListCell final : public GCCell {
    // Linked list of cells pointing to the next cell that was copied.
    CopyListCell *next_;
  };

  EvacAcceptor(GC &gc) : SlotAcceptorDefault{gc}, copyListHead_{nullptr} {}

  using SlotAcceptorDefault::accept;

  void accept(void *&ptr) override {
    if (!ptr || !gc.inYoungGen(ptr)) {
      // Ignore null and OG pointers.
      return;
    }
    GCCell *&cell = reinterpret_cast<GCCell *&>(ptr);
    if (cell->hasMarkedForwardingPointer()) {
      // Get the forwarding pointer from the header of the object.
      GCCell *const forwardedCell = cell->getMarkedForwardingPointer();
      assert(forwardedCell->isValid() && "Cell was forwarded incorrectly");
      cell = forwardedCell;
      return;
    }
    assert(cell->isValid() && "Encountered an invalid cell");
    // Newly discovered cell, first forward into the old gen.
    const auto sz = cell->getAllocatedSize();
    GCCell *const newCell = gc.oldGenAlloc(sz);
    // Copy the contents of the existing cell over before modifying it.
    std::memcpy(newCell, cell, sz);
    assert(newCell->isValid() && "Cell was copied incorrectly");
    CopyListCell *const copyCell = static_cast<CopyListCell *>(cell);
    // Set the forwarding pointer in the old spot
    copyCell->setMarkedForwardingPointer(newCell);
    // Push onto the copied list.
    push(copyCell);
    // Mark the cell's bit in the mark bit array as well, so that OG can rely on
    // that instead of checking the cell header.
    HeapSegment::setCellMarkBit(cell);
    // Fixup the pointer.
    cell = newCell;
  }

  void accept(HermesValue &hv) override {
    if (hv.isPointer()) {
      void *ptr = hv.getPointer();
      accept(ptr);
      hv.setInGC(hv.updatePointer(ptr), &gc);
    }
  }

  CopyListCell *pop() {
    if (!copyListHead_) {
      return nullptr;
    } else {
      CopyListCell *const cell = copyListHead_;
      assert(HeapSegment::getCellMarkBit(cell) && "Discovered unmarked object");
      copyListHead_ = copyListHead_->next_;
      return cell;
    }
  }

 private:
  /// The copy list is managed implicitly in the body of each copied YG object.
  CopyListCell *copyListHead_;

  void push(CopyListCell *cell) {
    cell->next_ = copyListHead_;
    copyListHead_ = cell;
  }
};

class HadesGC::MarkAcceptor final : public SlotAcceptorDefault,
                                    public WeakRefAcceptor {
 public:
  MarkAcceptor(GC &gc)
      : SlotAcceptorDefault{gc}, weakRefLock_{gc.weakRefMutex()} {
    markedSymbols_.resize(gc.gcCallbacks_->getSymbolsEnd(), false);
  }

  using SlotAcceptorDefault::accept;

  void accept(void *&ptr) override {
    if (!ptr) {
      return;
    }
    assert(
        !gc.inYoungGen(ptr) &&
        "OG collections should only be started immediately following a "
        "YG collection");
    GCCell *cell = static_cast<GCCell *>(ptr);
    assert(cell->isValid() && "Encountered an invalid cell");
    if (HeapSegment::getCellMarkBit(cell)) {
      // Points to an already marked object, do nothing.
      return;
    }
    push(cell);
  }

  void accept(HermesValue &hv) override {
    if (hv.isPointer()) {
      void *ptr = hv.getPointer();
      accept(ptr);
      // ptr should never be modified by this acceptor, so there's no write-back
      // to do.
      assert(
          ptr == hv.getPointer() &&
          "Pointer shouldn't be modified in MarkAcceptor");
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
    // Unfortunately, this weak ref marking must be done during the initial
    // mark phase, because sweeping has to happen after weak references have
    // been nulled out in a concurrent sweeper.
    WeakRefSlot *slot = wr.unsafeGetSlot(mutexRef());
    assert(
        slot->state() != WeakSlotState::Free &&
        "marking a freed weak ref slot");
    slot->mark();
  }

  void drainMarkWorklist(GC *gc) {
    while (!worklist_.empty()) {
      GCCell *const cell = worklist_.top();
      worklist_.pop();
      assert(cell->isValid() && "Invalid cell in marking");
      assert(HeapSegment::getCellMarkBit(cell) && "Discovered unmarked object");
      GCBase::markCell(cell, gc, *this);
    }
  }

  std::vector<JSWeakMap *> &reachableWeakMaps() {
    return reachableWeakMaps_;
  }

  const std::vector<bool> &markedSymbols() {
    return markedSymbols_;
  }

  const WeakRefMutex &mutexRef() override {
    return gc.weakRefMutex();
  }

 private:
  std::stack<GCCell *, std::vector<GCCell *>> worklist_;

  /// The WeakMap objects that have been discovered to be reachable.
  std::vector<JSWeakMap *> reachableWeakMaps_;

  /// markedSymbols_ represents which symbols have been proven live so far in
  /// a collection. True means that it is live, false means that it could
  /// possibly be garbage. At the end of the collection, it is guaranteed that
  /// the falses are garbage.
  std::vector<bool> markedSymbols_;

  WeakRefLock weakRefLock_;

  void push(GCCell *cell) {
    HeapSegment::setCellMarkBit(cell);
    // Add it to the worklist to recurse on that cell.
    if (cell->getKind() == CellKind::WeakMapKind) {
      reachableWeakMaps_.push_back(vmcast<JSWeakMap>(cell));
    } else {
      worklist_.push(cell);
    }
  }
};

class HadesGC::WeakRootAcceptor final : public WeakRootAcceptorDefault {
 public:
  WeakRootAcceptor(GC &gc)
      : WeakRootAcceptorDefault(gc), weakRefLock_(gc.weakRefMutex()) {}

  void acceptWeak(void *&ptr) override {
    if (ptr == nullptr) {
      return;
    }
    auto *cell = static_cast<GCCell *>(ptr);
    assert(gcForWeakRootDefault.dbgContains(ptr) && "ptr not in heap");
    assert(
        !gcForWeakRootDefault.inYoungGen(ptr) &&
        "Pointer should be into the OG");
    // Reset weak root if target GCCell is dead.
    if (!HeapSegment::getCellMarkBit(cell)) {
      ptr = nullptr;
    }
  }

  void accept(WeakRefBase &wr) override {
    // Duplicated from MarkAcceptor, since some weak roots are also weak refs.
    WeakRefSlot *slot = wr.unsafeGetSlot(mutexRef());
    assert(
        slot->state() != WeakSlotState::Free &&
        "marking a freed weak ref slot");
    slot->mark();
  }

  const WeakRefMutex &mutexRef() override {
    return gcForWeakRootDefault.weakRefMutex();
  }

 private:
  WeakRefLock weakRefLock_;
};

HadesGC::HadesGC(
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
      maxHeapSize_{std::max(
          static_cast<size_t>(
              llvm::alignTo<AlignedStorage::size()>(gcConfig.getMaxHeapSize())),
          // At least one YG segment and one OG segment.
          2 * AlignedStorage::size())},
      provider_(std::move(provider)),
      youngGen_{new HeapSegment{
          std::move(AlignedStorage::create(provider_.get(), "young-gen").get()),
          /*bumpAllocMode*/ true}} {
  const size_t minHeapSegments =
      // Align up first to round up.
      llvm::alignTo<AlignedStorage::size()>(gcConfig.getMinHeapSize()) /
      AlignedStorage::size();
  const size_t requestedInitHeapSegments =
      // Align up first to round up.
      llvm::alignTo<AlignedStorage::size()>(gcConfig.getInitHeapSize()) /
      AlignedStorage::size();

  const size_t initHeapSegments =
      std::max({minHeapSegments,
                requestedInitHeapSegments,
                // At least one YG segment and one OG segment.
                static_cast<size_t>(2)});

  for (size_t i = 0; i < initHeapSegments; ++i) {
    createOldGenSegment();
  }
}

HadesGC::~HadesGC() {}

uint32_t HadesGC::minAllocationSize() {
  return heapAlignSize(std::max(
      sizeof(HeapSegment::FreelistCell), sizeof(EvacAcceptor::CopyListCell)));
}

void HadesGC::getHeapInfo(HeapInfo &info) {
  GCBase::getHeapInfo(info);
  info.allocatedBytes = allocatedBytes();
  // Heap size includes fragmentation, which means every segment is fully used.
  info.heapSize = (oldGenEnd() - oldGenBegin() + 1) * AlignedStorage::size();
  info.totalAllocatedBytes = 0;
  info.va = info.heapSize;
}

// TODO: Fill these out
void HadesGC::getHeapInfoWithMallocSize(HeapInfo &info) {}
void HadesGC::getCrashManagerHeapInfo(CrashManager::HeapInformation &info) {}
void HadesGC::createSnapshot(llvm::raw_ostream &os) {}

void HadesGC::printStats(llvm::raw_ostream &os, bool trailingComma) {
  if (!recordGcStats_) {
    return;
  }
  GCBase::printStats(os, true);
  os << "\t\"specific\": {\n"
     << "\t\t\"collector\": \"hades\",\n"
     << "\t\t\"stats\": {\n"
     << "\t\t}\n"
     << "\t},\n";
  gcCallbacks_->printRuntimeGCStats(os);
  if (trailingComma) {
    os << ",";
  }
  os << "\n";
}

void HadesGC::collect() {
  // YG needs to be empty in order to do an OG collection.
  youngGenCollection(false);
  oldGenCollection();
}

void HadesGC::oldGenCollection() {
  // Full collection:
  //  * Mark all live objects by iterating through a worklist.
  //  * Sweep dead objects onto the free lists.
  // TODO: Make this concurrent.
  CollectionSection section{this};
  {
    // Mark phase: discover all pointers that are live.
    MarkAcceptor acceptor{*this};
    {
      DroppingAcceptor<MarkAcceptor> nameAcceptor{acceptor};
      markRoots(nameAcceptor, /*markLongLived*/ true);
      // Do not call markWeakRoots here, as weak roots can only be cleared after
      // liveness is known.
    }
    acceptor.drainMarkWorklist(this);
    completeWeakMapMarking(acceptor);
    // Free symbols that were found to be unused during the collection.
    gcCallbacks_->freeSymbols(acceptor.markedSymbols());
  }
  {
    WeakRootAcceptor acceptor{*this};
    // Reset weak roots to null after full reachability has been determined.
    markWeakRoots(acceptor);
    // Remove weak references that are no longer pointing to a live object.
    updateWeakReferencesForOldGen();
    // Reset all weak references to an unmarked state.
    resetWeakReferences();
  }
  {
    // Sweep phase: iterate through dead objects and add them to the free list.
    // Also finalize them at this point.
    for (auto segit = oldGenBegin(), segitend = oldGenEnd(); segit != segitend;
         ++segit) {
      HeapSegment &seg = **segit;
      if (seg.allocatedBytes() == 0) {
        // Quickly skip empty segments.
        continue;
      }
      seg.forAllObjs([this, &seg](GCCell *cell) {
        // forAllObjs skips free list cells, so no need to check for those.
        assert(cell->isValid() && "Invalid cell in sweeping");
        if (HeapSegment::getCellMarkBit(cell)) {
          return;
        }
        // Cell is dead, run its finalizer first if it has one.
        cell->getVT()->finalizeIfExists(cell, this);
        // Now add it to the head of the free list for the segment it's in.
        seg.addCellToFreelist(cell);
      });
      // Now since this segment has been swept, reset its mark bits.
      // This way when another collection starts, there's no leftover mark bits
      // from the last collection.
      seg.markBitArray().clear();
    }
  }
}

void HadesGC::finalizeAll() {
  const auto finalizeCell = [this](GCCell *cell) {
    assert(cell->isValid() && "Invalid cell in finalizeAll");
    cell->getVT()->finalizeIfExists(cell, this);
  };
  forAllObjs(finalizeCell);
}

void HadesGC::writeBarrier(void *loc, HermesValue value) {
  if (!value.isPointer()) {
    return;
  }
  writeBarrier(loc, value.getPointer());
}

void HadesGC::writeBarrier(void *loc, void *value) {
  // Just a generational write barrier for now.
  // Once the GC is concurrent, add the SATB barrier.
  if (AlignedStorage::containedInSame(loc, value)) {
    return;
  }
  if (youngGen().contains(value) && !youngGen().contains(loc)) {
    // Only dirty a card if it's an old-to-young pointer.
    // Dirtying the young gen card table requires doing a different cast, which
    // isn't necessary anyway.
    HeapSegment::cardTableCovering(loc)->dirtyCardForAddress(loc);
  }
}

void HadesGC::writeBarrierRange(GCHermesValue *start, uint32_t numHVs) {
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

  HeapSegment::cardTableCovering(firstPtr)->dirtyCardsForAddressRange(
      firstPtr, lastPtr);
}

void HadesGC::constructorWriteBarrier(void *loc, HermesValue value) {
  // For now, Hades doesn't do anything special with a constructor write
  // barrier.
  writeBarrier(loc, value);
}

void HadesGC::constructorWriteBarrier(void *loc, void *value) {
  // For now, Hades doesn't do anything special with a constructor write
  // barrier.
  writeBarrier(loc, value);
}

void HadesGC::constructorWriteBarrierRange(
    GCHermesValue *start,
    uint32_t numHVs) {
  writeBarrierRange(start, numHVs);
}

bool HadesGC::canAllocExternalMemory(uint32_t size) {
  return size <= maxHeapSize_;
}

void HadesGC::markSymbol(SymbolID symbolID) {}

WeakRefSlot *HadesGC::allocWeakSlot(HermesValue init) {
  assert(weakRefMutex() && "Mutex must be held");
  weakPointers_.push_back({init});
  return &weakPointers_.back();
}

void HadesGC::freeWeakSlot(WeakRefSlot *slot) {
  slot->free(nullptr);
}

void HadesGC::forAllObjs(const std::function<void(GCCell *)> &callback) {
  youngGen().forAllObjs(callback);
  for (auto seg = oldGenBegin(), end = oldGenEnd(); seg != end; ++seg) {
    (*seg)->forAllObjs(callback);
  }
}

#ifndef NDEBUG

bool HadesGC::validPointer(const void *p) const {
  return dbgContains(p) && static_cast<const GCCell *>(p)->isValid();
}

bool HadesGC::dbgContains(const void *p) const {
  return inYoungGen(p) || inOldGen(p);
}

void HadesGC::trackReachable(CellKind kind, unsigned sz) {}

size_t HadesGC::countUsedWeakRefs() const {
  size_t count = 0;
  for (auto &slot : weakPointers_) {
    if (slot.state() != WeakSlotState::Free) {
      ++count;
    }
  }
  return count;
}

bool HadesGC::isMostRecentFinalizableObj(const GCCell *cell) const {
  if (inYoungGen(cell)) {
    return youngGenFinalizables_.back() == cell;
  } else {
    // Hades doesn't have a most recent finalizable object list for the old
    // generation, it iterates over all dead objects during sweeping. So any
    // object with a finalize pointer set will be finalized.
    return cell->getVT()->finalize_ != nullptr;
  }
}

#endif

void *HadesGC::allocWork(
    uint32_t sz,
    bool longLived,
    bool fixedSize,
    HasFinalizer hasFinalizer) {
  sz = heapAlignSize(sz);
  assert(sz >= minAllocationSize() && "Allocating too small of an object");
  if (longLived) {
    // Alloc directly into the old gen.
    return oldGenAlloc(sz);
  }
  AllocResult res = youngGen().bumpAlloc(sz);
  if (!res.success) {
    // Failed to alloc in young gen, do a young gen collection.
    youngGenCollection(true);
    res = youngGen().bumpAlloc(sz);
    assert(res.success && "Should never fail to allocate");
  }
  if (hasFinalizer == HasFinalizer::Yes) {
    youngGenFinalizables_.emplace_back(static_cast<GCCell *>(res.ptr));
  }
  return res.ptr;
}

GCCell *HadesGC::oldGenAlloc(uint32_t sz) {
  if (GCCell *cell = oldGenSearch(sz)) {
    return cell;
  }
  // Wait for an old gen collection to finish and possibly free up memory.
  // TODO: waitForCollection();
  // Repeat the search in case the collection did free memory.
  // if (GCCell *cell = oldGenSearch(sz)) {
  //   return cell;
  // }
  // Else the collection didn't free enough memory, allocate a new segment.
  const auto maxNumOldGenSegments = (maxHeapSize_ / AlignedStorage::size()) - 1;
  if (static_cast<uint64_t>(oldGenEnd() - oldGenBegin()) <
      maxNumOldGenSegments) {
    HeapSegment &seg = createOldGenSegment();
    AllocResult res = seg.alloc(sz);
    assert(
        res.success &&
        "A newly created segment should always be able to allocate");
    return static_cast<GCCell *>(res.ptr);
  }

  // The GC didn't recover enough memory, OOM.
  // Before OOMing, finalize everything to avoid reporting leaks.
  finalizeAll();
  oom(make_error_code(OOMError::MaxHeapReached));
}

GCCell *HadesGC::oldGenSearch(uint32_t sz) {
  // TODO: Should do something better than iterating through all segments.
  for (auto seg = oldGenBegin(), end = oldGenEnd(); seg != end; ++seg) {
    const AllocResult res = (*seg)->alloc(sz);
    if (!res.success) {
      continue;
    }
    return static_cast<GCCell *>(res.ptr);
  }
  return nullptr;
}

void HadesGC::youngGenCollection(bool allowOGBegin) {
#ifdef HERMES_SLOW_DEBUG
  // Check that the card tables are well-formed before the collection.
  verifyCardTable();
#endif
  {
    CollectionSection section{this};
    // Marking each object puts it onto an embedded free list
    EvacAcceptor acceptor{*this};
    {
      DroppingAcceptor<EvacAcceptor> nameAcceptor{acceptor};
      markRoots(nameAcceptor, /*markLongLived*/ false);
      // Do not call markWeakRoots here, as all weak roots point to long-lived
      // objects.
      // Find old-to-young pointers, as they are considered roots for YG
      // collection.
      scanDirtyCards(acceptor);
    }
    // Iterate through the copy list to find new pointers.
    while (EvacAcceptor::CopyListCell *const copyCell = acceptor.pop()) {
      // Update the pointers inside the forwarded object, since the old object
      // is only there for the forwarding pointer.
      assert(
          copyCell->hasMarkedForwardingPointer() &&
          "Discovered unmarked object");
      GCCell *const cell = copyCell->getMarkedForwardingPointer();
      GCBase::markCell(cell, this, acceptor);
    }
    // Now that all YG objects have been marked, update weak references.
    updateWeakReferencesForYoungGen();
    // Reset all weak references to an unmarked state.
    resetWeakReferences();
    // Run finalizers for young gen objects.
    finalizeYoungGenObjects();
    // Now the copy list is drained, and all references point to the old gen.
    auto &yg = youngGen();
    // Clear the level of the young gen.
    yg.resetLevel();
    // Clear the mark bits in the young gen.
    yg.markBitArray().clear();
  }
#ifdef HERMES_SLOW_DEBUG
  // Check that the card tables are well-formed after the collection.
  verifyCardTable();
#endif

  if (allowOGBegin) {
    // If the OG is sufficiently full after the collection finishes, begin an
    // OG collection.
    const uint64_t totalAllocated = oldGenAllocatedBytes();
    const uint64_t totalBytes =
        (oldGenEnd() - oldGenBegin()) * HeapSegment::maxSize();
    constexpr double collectionThreshold = 0.75;
    if (static_cast<double>(totalAllocated) / totalBytes >=
        collectionThreshold) {
      oldGenCollection();
    }
  }
}

void HadesGC::scanDirtyCards(EvacAcceptor &acceptor) {
  SlotVisitor<EvacAcceptor> visitor{acceptor};
  // The acceptors in this loop can grow the old gen by adding another segment,
  // if there's not enough room to evac the YG objects discovered.
  // Since segments are always placed at the end, we can use indices instead of
  // iterators, which aren't invalidated.
  // It's ok to not scan newly added segments, since they are going to be
  // handled from the rest of YG collection.
  const auto segEnd = oldGen_.size();
  for (decltype(oldGen_.size()) i = 0; i < segEnd; ++i) {
    HeapSegment &seg = *oldGen_[i];
    const auto &cardTable = seg.cardTable();
    size_t from = cardTable.addressToIndex(seg.start());
    const size_t to = cardTable.addressToIndex(seg.end() - 1) + 1;

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

      // Use the object heads rather than the card table to discover the head
      // of the object.
      GCCell *const firstObj = seg.getCellHead(begin);
      GCCell *obj = firstObj;
      // Throughout this loop, objects are being marked which could promote
      // other objects into the OG. Such objects might be promoted onto a dirty
      // card, and be visited a second time. This is only a problem if the
      // acceptor isn't idempotent. Luckily, EvacAcceptor happens to be
      // idempotent, and so there's no correctness issue with visiting an object
      // multiple times. If EvacAcceptor wasn't idempotent, we'd have to be able
      // to identify objects promoted from YG in this loop, which would be
      // expensive.

      // Mark the first object with respect to the dirty card boundaries.
      GCBase::markCellWithinRange(visitor, obj, obj->getVT(), this, begin, end);

      // Mark the objects that are entirely contained within the dirty card
      // boundaries.
      for (GCCell *next = obj->nextCell();
           next < static_cast<const void *>(end);
           next = next->nextCell()) {
        // Use a separate pointer for the loop condition so that the last object
        // in range gets used with markCellWithinRange instead.
        obj = next;
        // Note that this object might be a FreelistCell. We could explicitly
        // check for this, but since FreelistCell has an empty metadata this
        // call ends up doing nothing anyway.
        GCBase::markCell(visitor, obj, obj->getVT(), this);
      }

      // Mark the final object in the range with respect to the dirty card
      // boundaries, as long as it does not coincide with the first object.
      if (LLVM_LIKELY(obj != firstObj)) {
        GCBase::markCellWithinRange(
            visitor, obj, obj->getVT(), this, begin, end);
      }

      from = iEnd;
    }
    seg.cardTable().clear();
  }
}

void HadesGC::finalizeYoungGenObjects() {
  for (GCCell *cell : youngGenFinalizables_) {
    if (!cell->hasMarkedForwardingPointer()) {
      cell->getVT()->finalize(cell, this);
    }
  }
  youngGenFinalizables_.clear();
}

void HadesGC::updateWeakReferencesForYoungGen() {
  for (auto &slot : weakPointers_) {
    switch (slot.state()) {
      case WeakSlotState::Free:
        break;
      case WeakSlotState::Unmarked: {
        if (!slot.hasPointer()) {
          // Non-pointers need no work.
          break;
        }
        auto *const cell = static_cast<GCCell *>(slot.getPointer());
        if (!inYoungGen(cell)) {
          break;
        }
        // A young-gen GC doesn't know if a weak ref is reachable via old gen
        // references, so be conservative and do nothing to the slot.
        // The value must also be forwarded.
        if (cell->hasMarkedForwardingPointer()) {
          HERMES_SLOW_ASSERT(
              validPointer(cell->getMarkedForwardingPointer()) &&
              "Forwarding weak ref must be to a valid cell");
          slot.setPointer(cell->getMarkedForwardingPointer());
        } else {
          // Can't free this slot because it might only be used by an OG object.
          slot.clearPointer();
        }
        break;
      }
      case WeakSlotState::Marked:
        assert(false && "No weak references are marked during YG GC");
        break;
    }
  }
}

void HadesGC::updateWeakReferencesForOldGen() {
  // Here, we know that YG is empty, so all weak refs must point to the OG.
  // NOTE: This only applies to the sequential collector. Re-evaluate once this
  // is concurrent.
  assert(youngGen().level() == youngGen().start() && "YG must be empty");
  for (auto &slot : weakPointers_) {
    switch (slot.state()) {
      case WeakSlotState::Free:
        // Skip free weak slots.
        break;
      case WeakSlotState::Unmarked:
#ifndef NDEBUG
        if (slot.hasPointer()) {
          auto *const cell = static_cast<GCCell *>(slot.getPointer());
          assert(!inYoungGen(cell) && "Can't point into YG, it must be empty");
        }
#endif
        // Free any unmarked weak slots.
        freeWeakSlot(&slot);
        break;
      case WeakSlotState::Marked: {
        if (!slot.hasPointer()) {
          // Skip non-pointers.
          break;
        }
        auto *const cell = static_cast<GCCell *>(slot.getPointer());
        assert(!inYoungGen(cell) && "Can't point into YG, it must be empty");
        // If the object isn't live, clear the weak ref.
        if (!HeapSegment::getCellMarkBit(cell)) {
          slot.clearPointer();
        }
        break;
      }
    }
  }
}

void HadesGC::completeWeakMapMarking(MarkAcceptor &acceptor) {
  gcheapsize_t weakMapAllocBytes = GCBase::completeWeakMapMarking(
      this,
      acceptor,
      acceptor.reachableWeakMaps(),
      /*objIsMarked*/
      HeapSegment::getCellMarkBit,
      /*markFromVal*/
      [this, &acceptor](GCCell *valCell, HermesValue &valRef) {
        if (HeapSegment::getCellMarkBit(valCell)) {
          return false;
        }
        acceptor.accept(valRef);
        acceptor.drainMarkWorklist(this);
        return true;
      },
      /*drainMarkStack*/
      [this](MarkAcceptor &acceptor) { acceptor.drainMarkWorklist(this); },
      /*checkMarkStackOverflow (HadesGC does not have mark stack overflow)*/
      []() { return false; });

  acceptor.reachableWeakMaps().clear();
  (void)weakMapAllocBytes;
}

void HadesGC::resetWeakReferences() {
  for (auto &slot : weakPointers_) {
    // Set all allocated slots to unmarked.
    if (slot.state() == WeakSlotState::Marked)
      slot.unmark();
  }
}

uint64_t HadesGC::allocatedBytes() {
  return youngGen().used() + oldGenAllocatedBytes();
}

uint64_t HadesGC::oldGenAllocatedBytes() {
  uint64_t totalAllocated = 0;
  for (auto segit = oldGenBegin(), end = oldGenEnd(); segit != end; ++segit) {
    // TODO: Should a fragmentation measurement be used as well?
    totalAllocated += (*segit)->allocatedBytes();
  }
  return totalAllocated;
}

HadesGC::HeapSegment &HadesGC::youngGen() {
  return *youngGen_;
}

const HadesGC::HeapSegment &HadesGC::youngGen() const {
  return *youngGen_;
}

bool HadesGC::inYoungGen(const void *p) const {
  return youngGen().contains(p);
}

std::vector<std::unique_ptr<HadesGC::HeapSegment>>::iterator
HadesGC::oldGenBegin() {
  assert(oldGen_.size() >= 1 && "No old gen segments");
  return oldGen_.begin();
}

std::vector<std::unique_ptr<HadesGC::HeapSegment>>::const_iterator
HadesGC::oldGenBegin() const {
  assert(oldGen_.size() >= 1 && "No old gen segments");
  return oldGen_.begin();
}

std::vector<std::unique_ptr<HadesGC::HeapSegment>>::iterator
HadesGC::oldGenEnd() {
  assert(oldGen_.size() >= 1 && "No old gen segments");
  return oldGen_.end();
}

std::vector<std::unique_ptr<HadesGC::HeapSegment>>::const_iterator
HadesGC::oldGenEnd() const {
  assert(oldGen_.size() >= 1 && "No old gen segments");
  return oldGen_.end();
}

HadesGC::HeapSegment &HadesGC::createOldGenSegment() {
  auto res = AlignedStorage::create(provider_.get(), "old-gen");
  if (!res) {
    hermes_fatal("Failed to alloc old gen");
  }
  oldGen_.emplace_back(
      new HeapSegment{std::move(res.get()), /*bumpAllocMode*/ false});
  return **oldGen_.rbegin();
}

bool HadesGC::inOldGen(const void *p) const {
  for (auto seg = oldGenBegin(), end = oldGenEnd(); seg != end; ++seg) {
    if ((*seg)->contains(p)) {
      return true;
    }
  }
  // None of the old gen segments matched the pointer.
  return false;
}

#ifdef HERMES_SLOW_DEBUG

void HadesGC::checkWellFormed() {
  WeakRefLock lk{weakRefMutex()};
  CheckHeapWellFormedAcceptor acceptor(*this);
  {
    DroppingAcceptor<CheckHeapWellFormedAcceptor> nameAcceptor{acceptor};
    markRoots(nameAcceptor, true);
  }
  markWeakRoots(acceptor);
  forAllObjs([this, &acceptor](GCCell *cell) {
    assert(cell->isValid() && "Invalid cell encountered in heap");
    GCBase::markCell(cell, this, acceptor);
  });
}

void HadesGC::verifyCardTable() {
  GCCycle cycle{this};
  struct VerifyCardDirtyAcceptor final : public SlotAcceptorDefault {
    using SlotAcceptorDefault::accept;
    using SlotAcceptorDefault::SlotAcceptorDefault;

    void accept(void *&ptr) override {
      char *valuePtr = reinterpret_cast<char *>(ptr);
      char *locPtr = reinterpret_cast<char *>(&ptr);

      acceptHelper(valuePtr, locPtr);
    }

#ifdef HERMESVM_COMPRESSED_POINTERS
    void accept(BasedPointer &ptr) override {
      // Don't use the default from SlotAcceptorDefault since the address of the
      // reference is used.
      PointerBase *const base = gc.getPointerBase();
      char *valuePtr = reinterpret_cast<char *>(base->basedToPointer(ptr));
      char *locPtr = reinterpret_cast<char *>(&ptr);

      acceptHelper(valuePtr, locPtr);
    }
#endif

    void accept(HermesValue &hv) override {
      if (!hv.isPointer()) {
        return;
      }

      char *valuePtr = reinterpret_cast<char *>(hv.getPointer());
      char *locPtr = reinterpret_cast<char *>(&hv);
      acceptHelper(valuePtr, locPtr);
    }

    void acceptHelper(char *valuePtr, char *locPtr) {
      if (gc.inYoungGen(valuePtr) && !gc.inYoungGen(locPtr)) {
        assert(HeapSegment::cardTableCovering(locPtr)->isCardForAddressDirty(
            locPtr));
      }
    }
  };

  VerifyCardDirtyAcceptor acceptor{*this};
  forAllObjs([this, &acceptor](GCCell *cell) {
    GCBase::markCell(cell, this, acceptor);
  });

  verifyCardTableBoundaries();
}

void HadesGC::verifyCardTableBoundaries() const {
  for (auto segit = oldGenBegin(), end = oldGenEnd(); segit != end; ++segit) {
    // The old gen heap segments use the object heads array instead of card
    // table boundaries to track where objects start.
    HeapSegment &seg = **segit;
    seg.forAllObjs([&seg](GCCell *cell) {
      // Check that each cell has a bit set.
      MarkBitArrayNC &heads = seg.cellHeads();
      assert(heads.at(heads.addressToIndex(cell)) && "Unmarked head");
      // Also check that no other bits are set until the next object.
      // Check in heap-aligned pointers since mark bits only work for aligned
      // pointers.
      uint64_t *ptr = reinterpret_cast<uint64_t *>(cell) + 1;
      uint64_t *const nextCell = reinterpret_cast<uint64_t *>(cell->nextCell());
      while (ptr < nextCell) {
        assert(
            !heads.at(heads.addressToIndex(ptr)) &&
            "Non-cell has a head marked");
        ++ptr;
      }
    });
  }
}

#endif

} // namespace vm
} // namespace hermes
