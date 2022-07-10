/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/HadesGC.h"

#include "GCBase-WeakMap.h"
#include "hermes/Support/Compiler.h"
#include "hermes/Support/ErrorHandling.h"
#include "hermes/VM/AllocResult.h"
#include "hermes/VM/CheckHeapWellFormedAcceptor.h"
#include "hermes/VM/FillerCell.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/RootAndSlotAcceptorDefault.h"

#include <array>
#include <functional>
#include <stack>

namespace hermes {
namespace vm {

static const char *kGCName =
    kConcurrentGC ? "hades (concurrent)" : "hades (incremental)";

static const char *kCompacteeNameForCrashMgr = "COMPACT";

// We have a target max pause time of 50ms.
static constexpr size_t kTargetMaxPauseMs = 50;

// A free list cell is always variable-sized.
const VTable HadesGC::OldGen::FreelistCell::vt{
    CellKind::FreelistKind,
    /*variableSize*/ 0};

void FreelistBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.setVTable(&HadesGC::OldGen::FreelistCell::vt);
}

HadesGC::HeapSegment::HeapSegment(AlignedStorage storage)
    : AlignedHeapSegment{std::move(storage)} {
  // Make sure end() is at the maxSize.
  growToLimit();
}

GCCell *HadesGC::OldGen::finishAlloc(GCCell *cell, uint32_t sz) {
  // Track the number of allocated bytes in a segment.
  incrementAllocatedBytes(sz);
  // Write a mark bit so this entry doesn't get free'd by the sweeper.
  HeapSegment::setCellMarkBit(cell);
  // Could overwrite the VTable, but the allocator will write a new one in
  // anyway.
  return cell;
}

void HadesGC::OldGen::SegmentBucket::addToFreelist(SegmentBucket *dummyHead) {
  auto *oldHead = dummyHead->next;
  if (oldHead)
    oldHead->prev = this;
  prev = dummyHead;
  next = oldHead;
  dummyHead->next = this;
}

void HadesGC::OldGen::SegmentBucket::removeFromFreelist() const {
  if (next)
    next->prev = prev;
  prev->next = next;
}

void HadesGC::OldGen::addCellToFreelist(
    void *addr,
    uint32_t sz,
    SegmentBucket *segBucket) {
  assert(
      sz >= sizeof(FreelistCell) &&
      "Cannot construct a FreelistCell into an allocation in the OG");
  FreelistCell *newFreeCell = constructCell<FreelistCell>(addr, sz);
  HeapSegment::setCellHead(static_cast<GCCell *>(addr), sz);
  addCellToFreelist(newFreeCell, segBucket);
}

void HadesGC::OldGen::addCellToFreelist(
    FreelistCell *cell,
    SegmentBucket *segBucket) {
  const size_t sz = cell->getAllocatedSize();
  // Push onto the size-specific free list for this bucket and segment.
  CompressedPointer oldHead = segBucket->head;
  cell->next_ = oldHead;
  segBucket->head =
      CompressedPointer::encodeNonNull(cell, gc_.getPointerBase());

  // If this SegmentBucket was not already in the freelist, add it.
  if (!oldHead) {
    uint32_t bucket = getFreelistBucket(sz);
    auto *dummyHead = &buckets_[bucket];
    segBucket->addToFreelist(dummyHead);

    // Set a bit indicating that there are now available blocks for this bucket.
    freelistBucketBitArray_.set(bucket, true);
  }

  // In ASAN builds, poison the memory outside of the FreelistCell so that
  // accesses are flagged as illegal while it is in the freelist.
  // Here, and in other places where FreelistCells are poisoned, use +1 on the
  // pointer to skip towards the memory region directly after the FreelistCell
  // header of a cell. This way the header is always intact and readable, and
  // only the contents of the cell are poisoned.
  __asan_poison_memory_region(cell + 1, sz - sizeof(FreelistCell));
}

void HadesGC::OldGen::addCellToFreelistFromSweep(
    char *freeRangeStart,
    char *freeRangeEnd,
    SegmentBuckets &segBuckets,
    bool setHead) {
  assert(
      gc_.concurrentPhase_ == Phase::Sweep &&
      "addCellToFreelistFromSweep should only be called during sweeping.");
  size_t newCellSize = freeRangeEnd - freeRangeStart;
  // While coalescing, sweeping may generate new cells, so make sure the cell
  // head is updated.
  if (setHead)
    HeapSegment::setCellHead(
        reinterpret_cast<GCCell *>(freeRangeStart), newCellSize);
  auto *newCell = constructCell<FreelistCell>(freeRangeStart, newCellSize);
  // Get the size bucket for the cell being added;
  const uint32_t bucket = getFreelistBucket(newCellSize);
  // Push onto the size-specific free list for this bucket and segment.
  auto *segBucket = &segBuckets[bucket];
  newCell->next_ = segBucket->head;
  segBucket->head =
      CompressedPointer::encodeNonNull(newCell, gc_.getPointerBase());
  __asan_poison_memory_region(newCell + 1, newCellSize - sizeof(FreelistCell));
}

HadesGC::OldGen::FreelistCell *HadesGC::OldGen::removeCellFromFreelist(
    size_t bucket,
    SegmentBucket *segBucket) {
  return removeCellFromFreelist(&segBucket->head, bucket, segBucket);
}

HadesGC::OldGen::FreelistCell *HadesGC::OldGen::removeCellFromFreelist(
    AssignableCompressedPointer *prevLoc,
    size_t bucket,
    SegmentBucket *segBucket) {
  FreelistCell *cell =
      vmcast<FreelistCell>(prevLoc->getNonNull(gc_.getPointerBase()));
  assert(cell && "Cannot get a null cell from freelist");

  // Update whatever was pointing to the cell we are removing.
  *prevLoc = cell->next_;
  // Update the bit arrays if the given freelist is now empty.
  if (!segBucket->head) {
    segBucket->removeFromFreelist();

    // If setting the bit to 0 above made this bucket empty for all segments,
    // set the bucket bit to 0 as well.
    freelistBucketBitArray_.set(bucket, buckets_[bucket].next);
  }

  // Unpoison the memory so that the mutator can use it.
  __asan_unpoison_memory_region(
      cell + 1, cell->getAllocatedSize() - sizeof(FreelistCell));
  return cell;
}

/* static */
void HadesGC::HeapSegment::setCellHead(
    const GCCell *cellStart,
    const size_t sz) {
  const char *start = reinterpret_cast<const char *>(cellStart);
  const char *end = start + sz;
  CardTable *cards = cardTableCovering(start);
  auto boundary = cards->nextBoundary(start);
  // If this object crosses a card boundary, then update boundaries
  // appropriately.
  if (boundary.address() < end) {
    cards->updateBoundaries(&boundary, start, end);
  }
}

GCCell *HadesGC::HeapSegment::getFirstCellHead(size_t cardIdx) {
  CardTable &cards = cardTable();
  GCCell *cell = cards.firstObjForCard(cardIdx);
  assert(cell->isValid() && "Object head doesn't point to a valid object");
  return cell;
}

template <typename CallbackFunction>
void HadesGC::HeapSegment::forAllObjs(CallbackFunction callback) {
  for (GCCell *cell : cells()) {
    // Skip free-list entries.
    if (!vmisa<OldGen::FreelistCell>(cell)) {
      callback(cell);
    }
  }
}

template <typename CallbackFunction>
void HadesGC::HeapSegment::forCompactedObjs(
    CallbackFunction callback,
    PointerBase &base) {
  void *const stop = level();
  GCCell *cell = reinterpret_cast<GCCell *>(start());
  while (cell < stop) {
    if (cell->hasMarkedForwardingPointer()) {
      // This cell has been evacuated, do nothing.
      cell = reinterpret_cast<GCCell *>(
          reinterpret_cast<char *>(cell) +
          cell->getMarkedForwardingPointer()
              .getNonNull(base)
              ->getAllocatedSize());
    } else {
      // This cell is being compacted away, call the callback on it.
      // NOTE: We do not check if it is a FreelistCell here in order to avoid
      // the extra overhead of that check in YG. The callback should add that
      // check if required.
      callback(cell);
      cell = cell->nextCell();
    }
  }
}

GCCell *HadesGC::OldGen::FreelistCell::carve(uint32_t sz) {
  const auto origSize = getAllocatedSize();
  assert(
      origSize >= sz + minAllocationSize() &&
      "Can't split if it would leave too small of a second cell");
  const auto finalSize = origSize - sz;
  char *newCellAddress = reinterpret_cast<char *>(this) + finalSize;
  GCCell *const newCell = reinterpret_cast<GCCell *>(newCellAddress);
  setSizeFromGC(finalSize);
  HeapSegment::setCellHead(newCell, sz);
  return newCell;
}

class HadesGC::CollectionStats final {
 public:
  using Clock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<Clock>;
  using Duration = std::chrono::microseconds;

  CollectionStats(HadesGC &gc, std::string cause, std::string collectionType)
      : gc_{gc},
        cause_{std::move(cause)},
        collectionType_{std::move(collectionType)} {}
  ~CollectionStats() {
    assert(usedDbg_ && "Stats not submitted.");
  }

  void addCollectionType(std::string collectionType) {
    if (std::find(tags_.begin(), tags_.end(), collectionType) == tags_.end())
      tags_.emplace_back(std::move(collectionType));
  }

  /// Record the allocated bytes in the heap and its size before a collection
  /// begins.
  void setBeforeSizes(uint64_t allocated, uint64_t external, uint64_t sz) {
    allocatedBefore_ = allocated;
    externalBefore_ = external;
    sizeBefore_ = sz;
  }

  /// Record how many bytes were swept during the collection.
  void setSweptBytes(uint64_t sweptBytes) {
    sweptBytes_ = sweptBytes;
  }

  void setSweptExternalBytes(uint64_t externalBytes) {
    sweptExternalBytes_ = externalBytes;
  }

  void setAfterSize(uint64_t sz) {
    sizeAfter_ = sz;
  }

  /// Record that a collection is beginning right now.
  void setBeginTime() {
    assert(beginTime_ == Clock::time_point{} && "Begin time already set");
    beginTime_ = Clock::now();
  }

  /// Record that a collection is ending right now.
  void setEndTime() {
    assert(endTime_ == Clock::time_point{} && "End time already set");
    endTime_ = Clock::now();
  }

  std::chrono::milliseconds getElapsedTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        Clock::now() - beginTime_);
  }

  /// Record this amount of CPU time was taken.
  /// Call begin/end in each thread that does work to correctly count CPU time.
  /// NOTE: Can only be used by one thread at a time.
  void beginCPUTimeSection() {
    assert(
        cpuTimeSectionStart_ == Duration{} &&
        "Must end cpu time section before starting another");
    cpuTimeSectionStart_ = oscompat::thread_cpu_time();
  }
  void endCPUTimeSection() {
    cpuDuration_ += oscompat::thread_cpu_time() - cpuTimeSectionStart_;
    cpuTimeSectionStart_ = {};
  }

  uint64_t beforeAllocatedBytes() const {
    return allocatedBefore_;
  }

  /// Since Hades allows allocations during an old gen collection, use the
  /// initially allocated bytes and the swept bytes to determine the actual
  /// impact of the GC.
  uint64_t afterAllocatedBytes() const {
    assert(sweptBytes_ <= allocatedBefore_);
    return allocatedBefore_ - sweptBytes_;
  }

  uint64_t afterExternalBytes() const {
    assert(sweptExternalBytes_ <= externalBefore_);
    return externalBefore_ - sweptExternalBytes_;
  }

  double survivalRatio() const {
    return allocatedBefore_
        ? static_cast<double>(afterAllocatedBytes()) / allocatedBefore_
        : 0;
  }

  void markUsed() {
    assert(!std::exchange(usedDbg_, true) && "markUsed called twice");
  }

  GCAnalyticsEvent getEvent() && {
    markUsed();
    return GCAnalyticsEvent{
        gc_.getName(),
        gc_.getKindAsStr(),
        collectionType_,
        std::move(cause_),
        std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime_ - beginTime_),
        std::chrono::duration_cast<std::chrono::milliseconds>(cpuDuration_),
        /*allocated*/ BeforeAndAfter{allocatedBefore_, afterAllocatedBytes()},
        /*size*/ BeforeAndAfter{sizeBefore_, sizeAfter_},
        /*external*/ BeforeAndAfter{externalBefore_, afterExternalBytes()},
        /*survivalRatio*/ survivalRatio(),
        /*tags*/ std::move(tags_)};
  }

 private:
  HadesGC &gc_;
  std::string cause_;
  std::string collectionType_;
  std::vector<std::string> tags_;
  TimePoint beginTime_{};
  TimePoint endTime_{};
  Duration cpuTimeSectionStart_{};
  Duration cpuDuration_{};
  uint64_t allocatedBefore_{0};
  uint64_t externalBefore_{0};
  uint64_t sizeBefore_{0};
  uint64_t sizeAfter_{0};
  uint64_t sweptBytes_{0};
  uint64_t sweptExternalBytes_{0};

#ifndef NDEBUG
  bool usedDbg_{false};
#endif
};

template <typename T>
static T convertPtr(PointerBase &, CompressedPointer cp) {
  return cp;
}
template <>
/* static */ GCCell *convertPtr(PointerBase &base, CompressedPointer cp) {
  return cp.get(base);
}
template <typename T>
static T convertPtr(PointerBase &, GCCell *p) {
  return p;
}
template <>
/* static */ CompressedPointer convertPtr(PointerBase &base, GCCell *a) {
  return CompressedPointer::encodeNonNull(a, base);
}

template <bool CompactionEnabled>
class HadesGC::EvacAcceptor final : public RootAndSlotAcceptor,
                                    public WeakRootAcceptor {
 public:
  EvacAcceptor(HadesGC &gc)
      : gc{gc},
        pointerBase_{gc.getPointerBase()},
        copyListHead_{nullptr},
        isTrackingIDs_{gc.isTrackingIDs()} {}

  ~EvacAcceptor() override {}

  // TODO: Implement a purely CompressedPointer version of this. That will let
  // us avoid decompressing pointers altogether if they point outside the
  // YG/compactee.
  inline bool shouldForward(const void *ptr) const {
    return gc.inYoungGen(ptr) ||
        (CompactionEnabled && gc.compactee_.evacContains(ptr));
  }
  inline bool shouldForward(CompressedPointer ptr) const {
    return gc.inYoungGen(ptr) ||
        (CompactionEnabled && gc.compactee_.evacContains(ptr));
  }

  LLVM_NODISCARD GCCell *acceptRoot(GCCell *ptr) {
    if (shouldForward(ptr))
      return forwardCell<GCCell *>(ptr);
    return ptr;
  }

  LLVM_NODISCARD GCCell *acceptHeap(GCCell *ptr, void *heapLoc) {
    if (shouldForward(ptr)) {
      assert(
          HeapSegment::getCellMarkBit(ptr) &&
          "Should only evacuate marked objects.");
      return forwardCell<GCCell *>(ptr);
    }
    if (CompactionEnabled && gc.compactee_.contains(ptr)) {
      // If a compaction is about to take place, dirty the card for any newly
      // evacuated cells, since the marker may miss them.
      HeapSegment::cardTableCovering(heapLoc)->dirtyCardForAddress(heapLoc);
    }
    return ptr;
  }

  LLVM_NODISCARD CompressedPointer
  acceptHeap(CompressedPointer cptr, void *heapLoc) {
    if (shouldForward(cptr)) {
      GCCell *ptr = cptr.getNonNull(pointerBase_);
      assert(
          HeapSegment::getCellMarkBit(ptr) &&
          "Should only evacuate marked objects.");
      return forwardCell<CompressedPointer>(ptr);
    }
    if (CompactionEnabled && gc.compactee_.contains(cptr)) {
      // If a compaction is about to take place, dirty the card for any newly
      // evacuated cells, since the marker may miss them.
      HeapSegment::cardTableCovering(heapLoc)->dirtyCardForAddress(heapLoc);
    }
    return cptr;
  }

  template <typename T>
  LLVM_NODISCARD T forwardCell(GCCell *const cell) {
    assert(
        HeapSegment::getCellMarkBit(cell) && "Cannot forward unmarked object");
    if (cell->hasMarkedForwardingPointer()) {
      // Get the forwarding pointer from the header of the object.
      CompressedPointer forwardedCell = cell->getMarkedForwardingPointer();
      assert(
          forwardedCell.getNonNull(pointerBase_)->isValid() &&
          "Cell was forwarded incorrectly");
      return convertPtr<T>(pointerBase_, forwardedCell);
    }
    assert(cell->isValid() && "Encountered an invalid cell");
    const auto cellSize = cell->getAllocatedSize();
    // Newly discovered cell, first forward into the old gen.
    GCCell *const newCell = gc.oldGen_.alloc(cellSize);
    HERMES_SLOW_ASSERT(
        gc.inOldGen(newCell) && "Evacuated cell not in the old gen");
    assert(
        HeapSegment::getCellMarkBit(newCell) &&
        "Cell must be marked when it is allocated into the old gen");
    // Copy the contents of the existing cell over before modifying it.
    std::memcpy(newCell, cell, cellSize);
    assert(newCell->isValid() && "Cell was copied incorrectly");
    evacuatedBytes_ += cellSize;
    CopyListCell *const copyCell = static_cast<CopyListCell *>(cell);
    // Set the forwarding pointer in the old spot
    copyCell->setMarkedForwardingPointer(
        CompressedPointer::encodeNonNull(newCell, pointerBase_));
    if (isTrackingIDs_) {
      gc.moveObject(cell, cellSize, newCell, cellSize);
    }
    // Push onto the copied list.
    push(copyCell);
    return convertPtr<T>(pointerBase_, newCell);
  }

  void accept(GCCell *&ptr) override {
    ptr = acceptRoot(ptr);
  }

  void accept(GCPointerBase &ptr) override {
    ptr.setInGC(acceptHeap(ptr, &ptr));
  }

  void accept(PinnedHermesValue &hv) override {
    assert((!hv.isPointer() || hv.getPointer()) && "Value is not nullable.");
    acceptNullable(hv);
  }

  void acceptNullable(PinnedHermesValue &hv) override {
    if (hv.isPointer()) {
      GCCell *forwardedPtr = acceptRoot(static_cast<GCCell *>(hv.getPointer()));
      hv.setInGC(hv.updatePointer(forwardedPtr), gc);
    }
  }

  void accept(GCHermesValue &hv) override {
    if (hv.isPointer()) {
      GCCell *forwardedPtr =
          acceptHeap(static_cast<GCCell *>(hv.getPointer()), &hv);
      hv.setInGC(hv.updatePointer(forwardedPtr), gc);
    }
  }

  void accept(GCSmallHermesValue &hv) override {
    if (hv.isPointer()) {
      CompressedPointer forwardedPtr = acceptHeap(hv.getPointer(), &hv);
      hv.setInGC(hv.updatePointer(forwardedPtr), gc);
    }
  }

  void acceptWeak(WeakRootBase &wr) override {
    // It's safe to not do a read barrier here since this is happening in the GC
    // and does not extend the lifetime of the referent.
    GCCell *const ptr = wr.getNoBarrierUnsafe(pointerBase_);

    if (!shouldForward(ptr))
      return;

    if (ptr->hasMarkedForwardingPointer()) {
      // Get the forwarding pointer from the header of the object.
      CompressedPointer forwardedCell = ptr->getMarkedForwardingPointer();
      assert(
          forwardedCell.getNonNull(pointerBase_)->isValid() &&
          "Cell was forwarded incorrectly");
      // Assign back to the input pointer location.
      wr = forwardedCell;
    } else {
      wr = nullptr;
    }
  }

  void accept(const RootSymbolID &sym) override {}
  void accept(const GCSymbolID &sym) override {}

  uint64_t evacuatedBytes() const {
    return evacuatedBytes_;
  }

  CopyListCell *pop() {
    if (!copyListHead_) {
      return nullptr;
    } else {
      CopyListCell *const cell =
          static_cast<CopyListCell *>(copyListHead_.getNonNull(pointerBase_));
      assert(HeapSegment::getCellMarkBit(cell) && "Discovered unmarked object");
      copyListHead_ = cell->next_;
      return cell;
    }
  }

 private:
  HadesGC &gc;
  PointerBase &pointerBase_;
  /// The copy list is managed implicitly in the body of each copied YG object.
  AssignableCompressedPointer copyListHead_;
  const bool isTrackingIDs_;
  uint64_t evacuatedBytes_{0};

  void push(CopyListCell *cell) {
    cell->next_ = copyListHead_;
    copyListHead_ = CompressedPointer::encodeNonNull(cell, pointerBase_);
  }
};

class MarkWorklist {
 private:
  /// Like std::vector but has a fixed capacity specified by N to reduce memory
  /// allocation.
  template <typename T, size_t N>
  class FixedCapacityVector {
   public:
    explicit FixedCapacityVector() : FixedCapacityVector(0) {}
    explicit FixedCapacityVector(size_t sz) : size_(sz) {}
    explicit FixedCapacityVector(const FixedCapacityVector &vec)
        : data_(vec.data_), size_(vec.size_) {}

    size_t size() const {
      return size_;
    }

    constexpr size_t capacity() const {
      return N;
    }

    bool empty() const {
      return size_ == 0;
    }

    void push_back(T elem) {
      assert(
          size_ < N && "Trying to push off the end of a FixedCapacityVector");
      data_[size_++] = elem;
    }

    T *data() {
      return data_.data();
    }

    void clear() {
#ifndef NDEBUG
      // Fill the push chunk with bad memory in debug modes to catch invalid
      // reads.
      std::memset(data_.data(), kInvalidHeapValue, sizeof(GCCell *) * N);
#endif
      size_ = 0;
    }

   private:
    std::array<T, N> data_;
    size_t size_;
  };

 public:
  /// Adds an element to the end of the queue.
  void enqueue(GCCell *cell) {
    pushChunk_.push_back(cell);
    if (pushChunk_.size() == pushChunk_.capacity()) {
      // Once the chunk has reached its max size, move it to the pull chunks.
      flushPushChunk();
    }
  }

  /// Empty and return the current worklist
  llvh::SmallVector<GCCell *, 0> drain() {
    llvh::SmallVector<GCCell *, 0> cells;
    // Move the list (in O(1) time) to a local variable, and then release the
    // lock. This unblocks the mutator faster.
    std::lock_guard<Mutex> lk{mtx_};
    std::swap(cells, worklist_);
    assert(worklist_.empty() && "worklist isn't cleared out");
    // Keep the previously allocated size to minimise allocations
    worklist_.reserve(cells.size());
    return cells;
  }

  /// While the world is stopped, move the push chunk to the list of pull chunks
  /// to finish draining the mark worklist.
  /// WARN: This can only be called by the mutator or when the world is stopped.
  void flushPushChunk() {
    std::lock_guard<Mutex> lk{mtx_};
    worklist_.insert(
        worklist_.end(),
        pushChunk_.data(),
        pushChunk_.data() + pushChunk_.size());
    // Set the size back to 0 and refill the same buffer.
    pushChunk_.clear();
  }

  /// \return true if there is still some work to be drained, with the exception
  /// of the push chunk.
  bool hasPendingWork() {
    std::lock_guard<Mutex> lk{mtx_};
    return !worklist_.empty();
  }

#ifndef NDEBUG
  /// WARN: This can only be called when the world is stopped.
  bool empty() {
    std::lock_guard<Mutex> lk{mtx_};
    return pushChunk_.empty() && worklist_.empty();
  }
#endif

 private:
  Mutex mtx_;
  static constexpr size_t kChunkSize = 128;
  using Chunk = FixedCapacityVector<GCCell *, kChunkSize>;
  Chunk pushChunk_;
  // Use a SmallVector of size 0 since it is more aggressive with PODs
  llvh::SmallVector<GCCell *, 0> worklist_;
};

class HadesGC::MarkAcceptor final : public RootAndSlotAcceptor,
                                    public WeakRefAcceptor {
 public:
  MarkAcceptor(HadesGC &gc)
      : gc{gc},
        pointerBase_{gc.getPointerBase()},
        markedSymbols_{gc.gcCallbacks_.getSymbolsEnd()},
        writeBarrierMarkedSymbols_{gc.gcCallbacks_.getSymbolsEnd()} {}

  void acceptHeap(GCCell *cell, const void *heapLoc) {
    assert(cell && "Cannot pass null pointer to acceptHeap");
    assert(!gc.inYoungGen(heapLoc) && "YG slot found in OG marking");
    if (gc.compactee_.contains(cell) && !gc.compactee_.contains(heapLoc)) {
      // This is a pointer in the heap pointing into the compactee, dirty the
      // corresponding card.
      HeapSegment::cardTableCovering(heapLoc)->dirtyCardForAddress(heapLoc);
    }
    if (HeapSegment::getCellMarkBit(cell)) {
      // Points to an already marked object, do nothing.
      return;
    }
    // This must be done after checking the cell mark bit, to avoid reading the
    // metadata of cells in the YG, which we do not have the lock for.
    assert(cell->isValid() && "Encountered an invalid cell");
    push(cell);
  }

  void acceptRoot(GCCell *cell) {
    assert(cell->isValid() && "Encountered an invalid cell");
    if (!HeapSegment::getCellMarkBit(cell))
      push(cell);
  }

  void accept(GCCell *&ptr) override {
    if (ptr)
      acceptRoot(ptr);
  }

  void accept(GCPointerBase &ptr) override {
    if (auto cp = concurrentRead<CompressedPointer>(ptr))
      acceptHeap(cp.getNonNull(pointerBase_), &ptr);
  }

  void accept(GCHermesValue &hvRef) override {
    HermesValue hv = concurrentRead<HermesValue>(hvRef);
    if (hv.isPointer()) {
      acceptHeap(static_cast<GCCell *>(hv.getPointer()), &hvRef);
    } else if (hv.isSymbol()) {
      acceptSym(hv.getSymbol());
    }
  }

  void accept(PinnedHermesValue &hv) override {
    // PinnedHermesValue is a root type and cannot live in the heap. Therefore
    // there's no risk of a concurrent access.
    if (hv.isPointer()) {
      acceptRoot(static_cast<GCCell *>(hv.getPointer()));
    } else if (hv.isSymbol()) {
      acceptSym(hv.getSymbol());
    }
  }
  void acceptNullable(PinnedHermesValue &hv) override {
    if (hv.isPointer()) {
      if (void *ptr = hv.getPointer())
        acceptRoot(static_cast<GCCell *>(ptr));
    } else if (hv.isSymbol()) {
      acceptSym(hv.getSymbol());
    }
  }

  void accept(GCSmallHermesValue &hvRef) override {
    const SmallHermesValue hv = concurrentRead<SmallHermesValue>(hvRef);
    if (hv.isPointer()) {
      acceptHeap(hv.getPointer(pointerBase_), &hvRef);
    } else if (hv.isSymbol()) {
      acceptSym(hv.getSymbol());
    }
  }

  void acceptSym(SymbolID sym) {
    const uint32_t idx = sym.unsafeGetIndex();
    if (sym.isInvalid() || idx >= markedSymbols_.size()) {
      // Ignore symbols that aren't valid or are pointing outside of the range
      // when the collection began.
      return;
    }
    markedSymbols_[idx] = true;
  }

  void accept(const RootSymbolID &sym) override {
    acceptSym(sym);
  }
  void accept(const GCSymbolID &sym) override {
    acceptSym(concurrentRead<SymbolID>(sym));
  }

  /// Interface for symbols marked by a write barrier.
  void markSymbol(SymbolID sym) {
    assert(
        !gc.calledByBackgroundThread() &&
        "Write barrier invoked by background thread.");
    const uint32_t idx = sym.unsafeGetIndex();
    if (sym.isInvalid() || idx >= writeBarrierMarkedSymbols_.size()) {
      // Ignore symbols that aren't valid or are pointing outside of the range
      // when the collection began.
      return;
    }
    writeBarrierMarkedSymbols_[idx] = true;
  }

  void accept(WeakRefBase &wr) override {
    assert(
        gc.weakRefMutex() &&
        "Must hold weak ref mutex when marking a WeakRef.");
    WeakRefSlot *slot = wr.unsafeGetSlot();
    assert(
        slot->state() != WeakSlotState::Free &&
        "marking a freed weak ref slot");
    if (slot->state() != WeakSlotState::Marked) {
      slot->mark();
    }
  }

  /// Set the drain rate that'll be used for any future calls to drain APIs.
  void setDrainRate(size_t rate) {
    assert(!kConcurrentGC && "Drain rate is only used by incremental GC.");
    byteDrainRate_ = rate;
  }

  /// \return the total number of bytes marked so far.
  uint64_t markedBytes() const {
    return markedBytes_;
  }

  /// Drain the mark stack of cells to be processed.
  /// \post localWorklist is empty. Any flushed values in the global worklist at
  /// the start of the call are drained.
  void drainAllWork() {
    // This should only be called from the mutator. This means no write barriers
    // should occur, and there's no need to check the global worklist more than
    // once.
    drainSomeWork(std::numeric_limits<size_t>::max());
    assert(localWorklist_.empty() && "Some work left that wasn't completed");
  }

  /// Drains some of the worklist, using a drain rate specified by
  /// \c setDrainRate or kConcurrentMarkLimit.
  /// \return true if there is any remaining work in the local worklist.
  bool drainSomeWork() {
    // See the comment in setDrainRate for why the drain rate isn't used for
    // concurrent collections.
    constexpr size_t kConcurrentMarkLimit = 8192;
    return drainSomeWork(kConcurrentGC ? kConcurrentMarkLimit : byteDrainRate_);
  }

  /// Drain some of the work to be done for marking.
  /// \param markLimit Only mark up to this many bytes from the local
  /// worklist.
  ///   NOTE: This does *not* guarantee that the marker thread
  ///   has upper bounds on the amount of work it does before reading from the
  ///   global worklist. Any individual cell can be quite large (such as an
  ///   ArrayStorage).
  /// \return true if there is any remaining work in the local worklist.
  bool drainSomeWork(const size_t markLimit) {
    assert(gc.gcMutex_ && "Must hold the GC lock while accessing mark bits.");
    // Pull any new items off the global worklist.
    auto cells = globalWorklist_.drain();
    for (GCCell *cell : cells) {
      assert(
          cell->isValid() && "Invalid cell received off the global worklist");
      assert(
          !gc.inYoungGen(cell) &&
          "Shouldn't ever traverse a YG object in this loop");
      HERMES_SLOW_ASSERT(
          gc.dbgContains(cell) && "Non-heap cell found in global worklist");
      if (!HeapSegment::getCellMarkBit(cell)) {
        // Cell has not yet been marked.
        push(cell);
      }
    }

    size_t numMarkedBytes = 0;
    assert(markLimit && "markLimit must be non-zero!");
    while (!localWorklist_.empty() && numMarkedBytes < markLimit) {
      GCCell *const cell = localWorklist_.top();
      localWorklist_.pop();
      assert(cell->isValid() && "Invalid cell in marking");
      assert(HeapSegment::getCellMarkBit(cell) && "Discovered unmarked object");
      assert(
          !gc.inYoungGen(cell) &&
          "Shouldn't ever traverse a YG object in this loop");
      HERMES_SLOW_ASSERT(
          gc.dbgContains(cell) && "Non-heap object discovered during marking");
      const auto sz = cell->getAllocatedSize();
      numMarkedBytes += sz;
      gc.markCell(cell, *this);
    }
    markedBytes_ += numMarkedBytes;
    return !localWorklist_.empty();
  }

  MarkWorklist &globalWorklist() {
    return globalWorklist_;
  }

  std::vector<JSWeakMap *> &reachableWeakMaps() {
    return reachableWeakMaps_;
  }

  /// Merge the symbols marked by the MarkAcceptor and by the write barrier,
  /// then return a reference to it.
  /// WARN: This should only be called when the mutator is paused, as
  /// otherwise there is a race condition between reading this and a symbol
  /// write barrier getting executed.
  llvh::BitVector &markedSymbols() {
    assert(gc.gcMutex_ && "Cannot call markedSymbols without a lock");
    markedSymbols_ |= writeBarrierMarkedSymbols_;
    // No need to clear writeBarrierMarkedSymbols_, or'ing it again won't change
    // the bit vector.
    return markedSymbols_;
  }

 private:
  HadesGC &gc;
  PointerBase &pointerBase_;

  /// A worklist local to the marking thread, that is only pushed onto by the
  /// marking thread. If this is empty, the global worklist must be consulted
  /// to ensure that pointers modified in write barriers are handled.
  std::stack<GCCell *, std::vector<GCCell *>> localWorklist_;

  /// A worklist that other threads may add to as objects to be marked and
  /// considered alive. These objects will *not* have their mark bits set,
  /// because the mutator can't be modifying mark bits at the same time as the
  /// marker thread.
  MarkWorklist globalWorklist_;

  /// The WeakMap objects that have been discovered to be reachable.
  std::vector<JSWeakMap *> reachableWeakMaps_;

  /// markedSymbols_ represents which symbols have been proven live so far in
  /// a collection. True means that it is live, false means that it could
  /// possibly be garbage. The SymbolID's internal value is used as the index
  /// into this vector. Once the collection is finished, this vector is passed
  /// to IdentifierTable so that it can free symbols. If any new symbols are
  /// allocated after the collection began, assume they are live.
  llvh::BitVector markedSymbols_;

  /// A vector the same size as markedSymbols_ that will collect all symbols
  /// marked by write barriers. Merge this with markedSymbols_ to have complete
  /// information about marked symbols. Kept separate to avoid synchronization.
  llvh::BitVector writeBarrierMarkedSymbols_;

  /// The number of bytes to drain per call to drainSomeWork. A higher rate
  /// means more objects will be marked.
  /// Only used by incremental collections.
  size_t byteDrainRate_{0};

  /// The number of bytes that have been marked so far.
  uint64_t markedBytes_{0};

  void push(GCCell *cell) {
    assert(
        !HeapSegment::getCellMarkBit(cell) &&
        "A marked object should never be pushed onto a worklist");
    assert(
        !gc.inYoungGen(cell) &&
        "Shouldn't ever push a YG object onto the worklist");
    HeapSegment::setCellMarkBit(cell);
    // There could be a race here: however, the mutator will never change a
    // cell's kind after initialization. The GC thread might to a free cell, but
    // only during sweeping, not concurrently with this operation. Therefore
    // there's no need for any synchronization here.
    if (vmisa<JSWeakMap>(cell)) {
      reachableWeakMaps_.push_back(vmcast<JSWeakMap>(cell));
    } else {
      localWorklist_.push(cell);
    }
  }

  template <typename T>
  T concurrentReadImpl(const T &valRef) {
    using Storage =
        typename std::conditional<sizeof(T) == 4, uint32_t, uint64_t>::type;
    static_assert(sizeof(T) == sizeof(Storage), "Sizes must match");
    union {
      Storage storage;
      T val;
    } ret{};

    // There is a benign data race here, as the GC can read a pointer while
    // it's being modified by the mutator; however, the following rules we
    // obey prevent it from being a problem:
    // * The only things being modified that the GC reads are the GCPointers
    //    and GCHermesValue in an object. All other fields are ignored.
    // * Those fields are fewer than 64 bits.
    // * Therefore, on 64-bit platforms, those fields are atomic
    //    automatically.
    // * On 32-bit platforms, we don't run this code concurrently, and
    //    instead yield cooperatively with the mutator.
    // * Thanks to the write barrier, if something is modified its old value
    //    is placed in the globalWorklist, so we don't miss marking it.
    // * Since the global worklist is pushed onto *before* the write
    //    happens, we know that there's no way the loop will exit unless it
    //    reads the old value.
    // * If it observes the old value (pre-write-barrier value) here, the
    //    new value will still be live, either by being pre-marked by the
    //    allocator, or because something else's write barrier will catch
    //    the modification.
    TsanIgnoreReadsBegin();

    // The cast to a volatile variable forces a read from valRef, since
    // reads from volatile variables are considered observable behaviour. This
    // prevents the compiler from optimizing away the returned value,
    // guaranteeing that we will not observe changes to the underlying value
    // past this point. Not using volatile here could lead to a TOCTOU bug,
    // because the underlying value may change after a pointer check (in the
    // case of HermesValue) or a null check (for pointers).
    ret.storage = *reinterpret_cast<Storage const volatile *>(&valRef);
    TsanIgnoreReadsEnd();
    return ret.val;
  }

  template <typename T>
  T concurrentRead(const T &ref) {
    return kConcurrentGC ? concurrentReadImpl<T>(ref) : ref;
  }
};

/// Mark weak roots separately from the MarkAcceptor since this is done while
/// the world is stopped.
/// Don't use the default weak root acceptor because fine-grained control of
/// writes of compressed pointers is important.
class HadesGC::MarkWeakRootsAcceptor final : public WeakRootAcceptor {
 public:
  MarkWeakRootsAcceptor(HadesGC &gc) : gc_{gc} {}

  void acceptWeak(WeakRootBase &wr) override {
    if (!wr) {
      return;
    }
    GCCell *const cell = wr.getNoBarrierUnsafe(gc_.getPointerBase());
    HERMES_SLOW_ASSERT(gc_.dbgContains(cell) && "ptr not in heap");
    if (HeapSegment::getCellMarkBit(cell)) {
      // If the cell is marked, no need to do any writes.
      return;
    }
    // Reset weak root if target GCCell is dead.
    wr = nullptr;
  }

 private:
  HadesGC &gc_;
};

class HadesGC::Executor {
 public:
  Executor() : thread_([this] { worker(); }) {}
  ~Executor() {
    {
      std::lock_guard<std::mutex> lk(mtx_);
      shutdown_ = true;
      cv_.notify_one();
    }
    thread_.join();
  }

  std::future<void> add(std::function<void()> fn) {
    std::lock_guard<std::mutex> lk(mtx_);
    // Use a shared_ptr because we cannot std::move the promise into the
    // lambda in C++11.
    auto promise = std::make_shared<std::promise<void>>();
    auto ret = promise->get_future();
    queue_.push_back([promise, fn] {
      fn();
      promise->set_value();
    });
    cv_.notify_one();
    return ret;
  }

  std::thread::id getThreadId() const {
    return thread_.get_id();
  }

 private:
  void worker() {
    oscompat::set_thread_name("hades");
    std::unique_lock<std::mutex> lk(mtx_);
    while (!shutdown_) {
      cv_.wait(lk, [this]() { return !queue_.empty() || shutdown_; });
      while (!queue_.empty()) {
        auto fn = std::move(queue_.front());
        queue_.pop_front();
        lk.unlock();
        fn();
        lk.lock();
      }
    }
  }

  std::mutex mtx_;
  std::condition_variable cv_;
  std::deque<std::function<void()>> queue_;
  bool shutdown_{false};
  std::thread thread_;
};

bool HadesGC::OldGen::sweepNext(bool backgroundThread) {
  // Check if there are any more segments to sweep. Note that in the case where
  // OG has zero segments, this also skips updating the stats and survival ratio
  // at the end of this function, since they are not required.
  if (!sweepIterator_.segNumber)
    return false;
  assert(gc_.gcMutex_ && "gcMutex_ must be held while sweeping.");

  sweepIterator_.segNumber--;

  const bool isTracking = gc_.isTrackingIDs();
  // Re-evaluate this start point each time, as releasing the gcMutex_ allows
  // allocations into the old gen, which might boost the credited memory.
  const uint64_t externalBytesBefore = externalBytes();

  auto &segBuckets = segmentBuckets_[sweepIterator_.segNumber];

  // Clear the head pointers and remove this segment from the segment level
  // freelists, so that we can construct a new freelist. The
  // freelistBucketBitArray_ will be updated after the segment is swept. The
  // bits will be inconsistent with the actual freelist for the duration of
  // sweeping, but this is fine because gcMutex_ is held during the entire
  // period.
  for (size_t bucket = 0; bucket < kNumFreelistBuckets; bucket++) {
    auto *segBucket = &segBuckets[bucket];
    if (segBucket->head) {
      segBucket->removeFromFreelist();
      segBucket->head = nullptr;
    }
  }

  char *freeRangeStart = nullptr, *freeRangeEnd = nullptr;
  size_t mergedCells = 0;
  int32_t segmentSweptBytes = 0;
  for (GCCell *cell : segments_[sweepIterator_.segNumber].cells()) {
    assert(cell->isValid() && "Invalid cell in sweeping");
    if (HeapSegment::getCellMarkBit(cell)) {
      // Cannot concurrently trim storage. Technically just checking
      // backgroundThread would suffice, but the kConcurrentGC lets us compile
      // away this check in incremental mode.
      if (kConcurrentGC && backgroundThread)
        continue;
      const uint32_t cellSize = cell->getAllocatedSize();
      const uint32_t trimmedSize =
          cell->getVT()->getTrimmedSize(cell, cellSize);
      assert(cellSize >= trimmedSize && "Growing objects is not supported.");
      assert(
          trimmedSize >= minAllocationSize() &&
          "Trimmed object must still meet minimum size.");
      assert(
          isSizeHeapAligned(trimmedSize) &&
          "Trimmed size must also be heap aligned");
      const uint32_t trimmableBytes = cellSize - trimmedSize;

      // If this cell has extra space we can trim, trim it.
      if (LLVM_UNLIKELY(trimmableBytes >= minAllocationSize())) {
        static_cast<VariableSizeRuntimeCell *>(cell)->setSizeFromGC(
            trimmedSize);
        GCCell *newCell = cell->nextCell();
        // Just create a FillerCell, the next iteration will free it.
        constructCell<FillerCell>(newCell, trimmableBytes);
        assert(
            !HeapSegment::getCellMarkBit(newCell) &&
            "Trimmed space cannot be marked");
        HeapSegment::setCellHead(newCell, trimmableBytes);
#ifndef NDEBUG
        sweepIterator_.trimmedBytes += trimmableBytes;
#endif
      }
      continue;
    }

    const auto sz = cell->getAllocatedSize();
    char *const cellCharPtr = reinterpret_cast<char *>(cell);

    if (freeRangeEnd != cellCharPtr) {
      assert(
          freeRangeEnd < cellCharPtr &&
          "Should not overshoot the start of an object");
      // We are starting a new free range, flush the previous one.
      if (LLVM_LIKELY(freeRangeStart))
        addCellToFreelistFromSweep(
            freeRangeStart, freeRangeEnd, segBuckets, mergedCells > 1);

      mergedCells = 0;
      freeRangeEnd = freeRangeStart = cellCharPtr;
    }
    // Expand the current free range to include the current cell.
    freeRangeEnd += sz;
    mergedCells++;

    if (vmisa<FreelistCell>(cell))
      continue;

    segmentSweptBytes += sz;
    // Cell is dead, run its finalizer first if it has one.
    cell->getVT()->finalizeIfExists(cell, gc_);
    if (isTracking && !vmisa<FillerCell>(cell)) {
      gc_.untrackObject(cell, sz);
    }
  }

  // Flush any free range that was left over.
  if (freeRangeStart)
    addCellToFreelistFromSweep(
        freeRangeStart, freeRangeEnd, segBuckets, mergedCells > 1);

  // Update the segment level freelists for any buckets that this segment has
  // free cells for.
  for (size_t bucket = 0; bucket < kNumFreelistBuckets; ++bucket) {
    auto *segBucket = &segBuckets[bucket];
    if (segBucket->head)
      segBucket->addToFreelist(&buckets_[bucket]);

    // In case sweeping has changed the availability of a bucket, update the
    // overall bit array. Note that this is necessary even if segBucket->head is
    // null, as the bits were not updated when the freelist for this segment was
    // erased prior to sweeping.
    freelistBucketBitArray_.set(bucket, buckets_[bucket].next);
  }

  // Correct the allocated byte count.
  incrementAllocatedBytes(-segmentSweptBytes);
  sweepIterator_.sweptBytes += segmentSweptBytes;
  sweepIterator_.sweptExternalBytes += externalBytesBefore - externalBytes();

  // There are more iterations to go.
  if (sweepIterator_.segNumber)
    return true;

  // This was the last sweep iteration, finish the collection.
  auto &stats = *gc_.ogCollectionStats_;

  auto sweptBytes = sweepIterator_.sweptBytes;
  auto preAllocated = stats.beforeAllocatedBytes();
  if (sweptBytes > preAllocated) {
    // Only trimming can result in freeing more memory than was allocated at the
    // start of the collection, since we may trim cells that were allocated
    // after the collection started.
    assert(sweepIterator_.trimmedBytes >= (sweptBytes - preAllocated));
    // We can't precisely calculate how much of the trimmed memory came from
    // cells allocated during the collection, so just cap the swept bytes at the
    // number of initially allocated bytes.
    sweptBytes = preAllocated;
  }
  stats.setSweptBytes(sweptBytes);
  stats.setSweptExternalBytes(sweepIterator_.sweptExternalBytes);
  const uint64_t targetSizeBytes =
      (stats.afterAllocatedBytes() + stats.afterExternalBytes()) /
      gc_.occupancyTarget_;

  // In a very large heap, use the configured max heap size as a backstop to
  // prevent the target size crossing it (which would delay collection and cause
  // an OOM). This is just an approximation, a precise accounting would subtract
  // segment metadata and YG memory.
  uint64_t clampedSizeBytes = std::min(targetSizeBytes, gc_.maxHeapSize_);
  targetSizeBytes_.update(clampedSizeBytes);
  sweepIterator_ = {};
  return false;
}

void HadesGC::OldGen::initializeSweep() {
  assert(
      !sweepIterator_.segNumber && !sweepIterator_.sweptBytes &&
      !sweepIterator_.sweptExternalBytes && "Sweep is already in progress.");
  sweepIterator_.segNumber = segments_.size();
}

size_t HadesGC::OldGen::sweepSegmentsRemaining() const {
  return sweepIterator_.segNumber;
}

size_t HadesGC::OldGen::getMemorySize() const {
  size_t memorySize = segments_.size() * sizeof(HeapSegment);
  memorySize += segmentBuckets_.size() * sizeof(SegmentBuckets);
  return memorySize;
}

// Assume about 30% of the YG will survive initially.
constexpr double kYGInitialSurvivalRatio = 0.3;

HadesGC::OldGen::OldGen(HadesGC &gc) : gc_(gc) {}

HadesGC::HadesGC(
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
          HeapKind::HadesGC),
      maxHeapSize_{std::max<uint64_t>(
          gcConfig.getMaxHeapSize(),
          // At least one YG segment and one OG segment.
          2 * AlignedStorage::size())},
      provider_(std::move(provider)),
      oldGen_{*this},
      backgroundExecutor_{
          kConcurrentGC ? std::make_unique<Executor>() : nullptr},
      promoteYGToOG_{!gcConfig.getAllocInYoung()},
      revertToYGAtTTI_{gcConfig.getRevertToYGAtTTI()},
      occupancyTarget_(gcConfig.getOccupancyTarget()),
      ygAverageSurvivalBytes_{
          /*weight*/ 0.5,
          /*init*/ kYGInitialSizeFactor * HeapSegment::maxSize() *
              kYGInitialSurvivalRatio} {
  (void)vmExperimentFlags;
  std::lock_guard<Mutex> lk(gcMutex_);
  crashMgr_->setCustomData("HermesGC", getKindAsStr().c_str());
  // createSegment relies on member variables and should not be called until
  // they are initialised.
  llvh::ErrorOr<HeapSegment> newYoungGen = createSegment();
  if (!newYoungGen)
    hermes_fatal("Failed to initialize the young gen", newYoungGen.getError());
  setYoungGen(std::move(newYoungGen.get()));
  const size_t initHeapSize = std::max<uint64_t>(
      {gcConfig.getMinHeapSize(),
       gcConfig.getInitHeapSize(),
       HeapSegment::maxSize()});
  oldGen_.setTargetSizeBytes(initHeapSize - HeapSegment::maxSize());
}

HadesGC::~HadesGC() {
  // finalizeAll calls waitForCollectionToFinish, so there should be no ongoing
  // collection.
  assert(
      concurrentPhase_ == Phase::None &&
      "Must call finalizeAll before destructor.");
}

void HadesGC::getHeapInfo(HeapInfo &info) {
  std::lock_guard<Mutex> lk{gcMutex_};
  GCBase::getHeapInfo(info);
  info.allocatedBytes = allocatedBytes();
  // Heap size includes fragmentation, which means every segment is fully used.
  info.heapSize = (oldGen_.numSegments() + 1) * AlignedStorage::size();
  // If YG isn't empty, its bytes haven't been accounted for yet, add them here.
  info.totalAllocatedBytes = totalAllocatedBytes_ + youngGen().used();
  info.va = info.heapSize;
  info.externalBytes = oldGen_.externalBytes() + getYoungGenExternalBytes();
}

void HadesGC::getHeapInfoWithMallocSize(HeapInfo &info) {
  // Get the usual heap info.
  getHeapInfo(info);
  // Get GCBase's malloc size estimate.
  GCBase::getHeapInfoWithMallocSize(info);
  std::lock_guard<Mutex> lk{gcMutex_};
  // First add the usage by the runtime's roots.
  info.mallocSizeEstimate += gcCallbacks_.mallocSize();
  // Scan all objects for their malloc size. This operation is what makes
  // getHeapInfoWithMallocSize O(heap size).
  forAllObjs([&info](GCCell *cell) {
    info.mallocSizeEstimate += cell->getVT()->getMallocSize(cell);
  });
}

void HadesGC::getCrashManagerHeapInfo(
    CrashManager::HeapInformation &crashInfo) {
  HeapInfo info;
  getHeapInfo(info);
  crashInfo.size_ = info.heapSize;
  crashInfo.used_ = info.allocatedBytes;
}

#ifdef HERMES_MEMORY_INSTRUMENTATION
void HadesGC::createSnapshot(llvh::raw_ostream &os) {
  std::lock_guard<Mutex> lk{gcMutex_};
  // No allocations are allowed throughout the entire heap snapshot process.
  NoAllocScope scope{*this};
  // Let any existing collections complete before taking the snapshot.
  waitForCollectionToFinish("snapshot");
  {
    GCCycle cycle{*this, "GC Heap Snapshot"};
    WeakRefLock lk{weakRefMutex()};
    GCBase::createSnapshot(*this, os);
  }
}

void HadesGC::snapshotAddGCNativeNodes(HeapSnapshot &snap) {
  GCBase::snapshotAddGCNativeNodes(snap);
  if (nativeIDs_.ygFinalizables == IDTracker::kInvalidNode) {
    nativeIDs_.ygFinalizables = getIDTracker().nextNativeID();
  }
  if (nativeIDs_.og == IDTracker::kInvalidNode) {
    nativeIDs_.og = getIDTracker().nextNativeID();
  }
  snap.beginNode();
  snap.endNode(
      HeapSnapshot::NodeType::Native,
      "std::vector<GCCell *>",
      nativeIDs_.ygFinalizables,
      youngGenFinalizables_.capacity() * sizeof(GCCell *),
      0);
  snap.beginNode();
  snap.endNode(
      HeapSnapshot::NodeType::Native,
      "OldGen",
      nativeIDs_.og,
      sizeof(oldGen_) + oldGen_.getMemorySize(),
      0);
}

void HadesGC::snapshotAddGCNativeEdges(HeapSnapshot &snap) {
  GCBase::snapshotAddGCNativeEdges(snap);
  // All native ids should be lazily initialized in snapshotAddGCNativeNodes,
  // because that is called first.
  assert(nativeIDs_.ygFinalizables != IDTracker::kInvalidNode);
  assert(nativeIDs_.og != IDTracker::kInvalidNode);
  snap.addNamedEdge(
      HeapSnapshot::EdgeType::Internal,
      "youngGenFinalizables",
      nativeIDs_.ygFinalizables);
  snap.addNamedEdge(HeapSnapshot::EdgeType::Internal, "oldGen", nativeIDs_.og);
}

void HadesGC::enableHeapProfiler(
    std::function<void(
        uint64_t,
        std::chrono::microseconds,
        std::vector<GCBase::AllocationLocationTracker::HeapStatsUpdate>)>
        fragmentCallback) {
  std::lock_guard<Mutex> lk{gcMutex_};
  // Let any existing collections complete before enabling the profiler.
  waitForCollectionToFinish("heap profiler enable");
  GCBase::enableHeapProfiler(std::move(fragmentCallback));
}

void HadesGC::disableHeapProfiler() {
  std::lock_guard<Mutex> lk{gcMutex_};
  // Let any existing collections complete before disabling the profiler.
  waitForCollectionToFinish("heap profiler disable");
  GCBase::disableHeapProfiler();
}

void HadesGC::enableSamplingHeapProfiler(
    size_t samplingInterval,
    int64_t seed) {
  std::lock_guard<Mutex> lk{gcMutex_};
  // Let any existing collections complete before enabling the profiler.
  waitForCollectionToFinish("sampling heap profiler enable");
  GCBase::enableSamplingHeapProfiler(samplingInterval, seed);
}

void HadesGC::disableSamplingHeapProfiler(llvh::raw_ostream &os) {
  std::lock_guard<Mutex> lk{gcMutex_};
  // Let any existing collections complete before disabling the profiler.
  waitForCollectionToFinish("sampling heap profiler disable");
  GCBase::disableSamplingHeapProfiler(os);
}
#endif // HERMES_MEMORY_INSTRUMENTATION

void HadesGC::printStats(JSONEmitter &json) {
  GCBase::printStats(json);
  json.emitKey("specific");
  json.openDict();
  json.emitKeyValue("collector", getKindAsStr());
  json.emitKey("stats");
  json.openDict();
  json.closeDict();
  json.closeDict();
}

std::string HadesGC::getKindAsStr() const {
  return kGCName;
}

void HadesGC::collect(std::string cause, bool /*canEffectiveOOM*/) {
  {
    // Wait for any existing collections to finish before starting a new one.
    std::lock_guard<Mutex> lk{gcMutex_};
    // Disable the YG promotion mode. A forced GC via collect will do a full
    // collection immediately anyway, so there's no need to avoid collecting YG.
    // This is especially important when the forced GC is a memory warning.
    promoteYGToOG_ = false;
    waitForCollectionToFinish(cause);
  }
  // This function should block until a collection finishes.
  // YG needs to be empty in order to do an OG collection.
  youngGenCollection(cause, /*forceOldGenCollection*/ true);
  {
    // Wait for the collection to complete.
    std::lock_guard<Mutex> lk{gcMutex_};
    waitForCollectionToFinish(cause);
  }
  // Start a second YG collection to complete any pending compaction.
  // Since YG is empty, this will only be evacuating the compactee.
  // Note that it's possible for this call to start another OG collection if the
  // occupancy target is >= 75%. That doesn't break the contract of this
  // function though, and we don't want to bother with waiting for that
  // collection to complete because it won't find any garbage anyway.
  youngGenCollection(std::move(cause), /*forceOldGenCollection*/ false);
}

void HadesGC::waitForCollectionToFinish(std::string cause) {
  assert(
      gcMutex_ &&
      "gcMutex_ must be held before calling waitForCollectionToFinish");
  if (concurrentPhase_ == Phase::None) {
    return;
  }
  GCCycle cycle{*this, "GC Old Gen (Direct)"};

  assert(!ygCollectionStats_ && "Cannot collect OG during a YG collection");
  CollectionStats waitingStats(*this, std::move(cause), "waiting");
  waitingStats.beginCPUTimeSection();
  waitingStats.setBeginTime();

  while (concurrentPhase_ != Phase::None)
    incrementalCollect(false);

  waitingStats.endCPUTimeSection();
  waitingStats.setEndTime();
  recordGCStats(std::move(waitingStats).getEvent(), true);
}

void HadesGC::oldGenCollection(std::string cause, bool forceCompaction) {
  // Full collection:
  //  * Mark all live objects by iterating through a worklist.
  //  * Sweep dead objects onto the free lists.
  // This function must be called while the gcMutex_ is held.
  assert(gcMutex_ && "gcMutex_ must be held when starting an OG collection");
  assert(
      gcMutex_.depth() == 1 &&
      "Need ability to release mutex in oldGenCollection.");
  assert(
      concurrentPhase_ == Phase::None &&
      "Starting a second old gen collection");
  // Wait for any lingering background task to finish.
  if (kConcurrentGC && ogThreadStatus_.valid()) {
    // This is just making sure that any leftover work is completed before
    // starting a new collection. Since concurrentPhase_ == None here, there is
    // no collection ongoing. However, the background task may need to acquire
    // the lock in order to observe the value of concurrentPhase_.
    std::unique_lock<Mutex> lk{gcMutex_, std::adopt_lock};
    lk.unlock();
    ogThreadStatus_.get();
    lk.lock();
    lk.release();
  }
  // We know ygCollectionStats_ exists because oldGenCollection is only called
  // by youngGenCollection.
  ygCollectionStats_->addCollectionType("old gen start");
#ifdef HERMES_SLOW_DEBUG
  checkWellFormed();
#endif
  if (ogCollectionStats_)
    recordGCStats(std::move(*ogCollectionStats_).getEvent(), false);
  ogCollectionStats_ =
      std::make_unique<CollectionStats>(*this, std::move(cause), "old");
  // NOTE: Leave CPU time as zero if the collection isn't concurrent, as the
  // times aren't useful.
  if (kConcurrentGC)
    ogCollectionStats_->beginCPUTimeSection();
  ogCollectionStats_->setBeginTime();
  ogCollectionStats_->setBeforeSizes(
      oldGen_.allocatedBytes(), oldGen_.externalBytes(), segmentFootprint());

  if (revertToYGAtTTI_) {
    // If we've reached the first OG collection, and reverting behavior is
    // requested, switch back to YG mode.
    promoteYGToOG_ = false;
  }
  // First, clear any mark bits that were set by a previous collection or
  // direct-to-OG allocation, they aren't needed anymore.
  for (HeapSegment &seg : oldGen_)
    seg.markBitArray().clear();

  // Unmark all symbols in the identifier table, as Symbol liveness will be
  // determined during the collection.
  gcCallbacks_.unmarkSymbols();

  // Mark phase: discover all pointers that are live.
  // This assignment will reset any leftover memory from the last collection. We
  // leave the last marker alive to avoid a race condition with setting
  // concurrentPhase_, oldGenMarker_ and the write barrier.
  oldGenMarker_.reset(new MarkAcceptor{*this});
  {
    // Roots are marked before a marking thread is spun up, so that the root
    // marking is atomic.
    DroppingAcceptor<MarkAcceptor> nameAcceptor{*oldGenMarker_};
    markRoots(nameAcceptor, /*markLongLived*/ true);
    // Do not call markWeakRoots here, as weak roots can only be cleared
    // after liveness is known.
  }

  concurrentPhase_ = Phase::Mark;
  // Before the thread starts up, make sure that any write barriers are aware
  // that concurrent marking is happening.
  ogMarkingBarriers_ = true;
  // prepareCompactee must be called before the new thread is spawned, in order
  // to ensure that write barriers start operating immediately, and that any
  // objects promoted during an intervening YG collection are correctly scanned.
  prepareCompactee(forceCompaction);

  // Setup the sweep iterator when collection begins, because the number of
  // segments can change if a YG collection interleaves. There's no need to
  // sweep those extra segments since they are full of newly promoted
  // objects from YG (which have all their mark bits set), thus the sweep
  // iterator doesn't need to be updated. We also don't need to sweep any
  // segments made since the start of the collection, since they won't have
  // any unmarked objects anyway.
  // NOTE: this must be called after prepareCompactee, which may remove segments
  // from the heap.
  oldGen_.initializeSweep();

  if (!kConcurrentGC) {
    // 32-bit system: 64-bit HermesValues cannot be updated in one atomic
    // instruction. Have YG collections interleave marking work.
    // In this version of the system, the mark stack will be drained in batches
    // during each YG collection. Once the mark stack is fully drained, the rest
    // of the collection finishes while blocking a YG GC. This allows
    // incremental work without actually using multiple threads.

    // Initialize the drain rate.
    oldGenMarker_->setDrainRate(getDrainRate());
    return;
  }
  ogCollectionStats_->endCPUTimeSection();
  // 64-bit system: 64-bit HermesValues can be updated in one atomic
  // instruction. Start up a separate thread for doing marking work.
  // NOTE: Since the "this" value (the HadesGC instance) is copied to the
  // executor, the GC cannot be destructed until the new thread completes. This
  // means that before destroying the GC, waitForCollectionToFinish must be
  // called.
  collectOGInBackground();
  // Use concurrentPhase_ to be able to tell when the collection finishes.
}

void HadesGC::collectOGInBackground() {
  assert(gcMutex_ && "Must hold GC mutex when scheduling background work.");
  assert(
      !backgroundTaskActive_ && "Should only have one active task at a time");
  assert(kConcurrentGC && "Background work can only be done in concurrent GC");
#ifndef NDEBUG
  backgroundTaskActive_ = true;
#endif

  ogThreadStatus_ = backgroundExecutor_->add([this]() {
    std::unique_lock<Mutex> lk(gcMutex_);
    while (true) {
      // If the mutator has requested the background task to stop and yield
      // gcMutex_, wait on ogPauseCondVar_ until the mutator acquires the mutex
      // and signals that we may resume.
      ogPauseCondVar_.wait(
          lk, [this] { return !ogPaused_.load(std::memory_order_relaxed); });
      assert(
          backgroundTaskActive_ &&
          "backgroundTaskActive_ must be true when the background task is in the loop.");
      incrementalCollect(true);
      if (concurrentPhase_ == Phase::None ||
          concurrentPhase_ == Phase::CompleteMarking) {
#ifndef NDEBUG
        backgroundTaskActive_ = false;
#endif
        break;
      }
    }
  });
}

std::lock_guard<Mutex> HadesGC::pauseBackgroundTask() {
  assert(kConcurrentGC && "Should not be called in incremental mode");
  assert(!calledByBackgroundThread() && "Must be called from mutator");
  // Signal to the background thread that it should stop and wait on
  // ogPauseCondVar_.
  ogPaused_.store(true, std::memory_order_relaxed);
  // Acquire gcMutex_ as soon as it is released by the background thread.
  gcMutex_.lock();
  // Signal to the background thread that it may resume. Note that it will just
  // go to wait on gcMutex_, since it is currently held by this thread.
  ogPaused_.store(false, std::memory_order_relaxed);
  ogPauseCondVar_.notify_one();
  return std::lock_guard(gcMutex_, std::adopt_lock);
}

void HadesGC::incrementalCollect(bool backgroundThread) {
  assert(gcMutex_ && "Must hold the GC mutex when calling incrementalCollect");
  switch (concurrentPhase_) {
    case Phase::None:
      break;
    case Phase::Mark:
      if (!kConcurrentGC && ygCollectionStats_)
        ygCollectionStats_->addCollectionType("marking");
      // Drain some work from the mark worklist. If the work has finished
      // completely, move on to CompleteMarking.
      if (!oldGenMarker_->drainSomeWork())
        concurrentPhase_ = Phase::CompleteMarking;
      break;
    case Phase::CompleteMarking:
      // Background task should exit, the mutator will restart it after the STW
      // pause.
      if (!backgroundThread) {
        if (ygCollectionStats_)
          ygCollectionStats_->addCollectionType("complete marking");
        completeMarking();
        concurrentPhase_ = Phase::Sweep;
      }
      break;
    case Phase::Sweep:
      if (!kConcurrentGC && ygCollectionStats_)
        ygCollectionStats_->addCollectionType("sweeping");
      // Calling oldGen_.sweepNext() will sweep the next segment.
      if (!oldGen_.sweepNext(backgroundThread)) {
        // Finish any collection bookkeeping.
        ogCollectionStats_->setEndTime();
        ogCollectionStats_->setAfterSize(segmentFootprint());
        compacteeHandleForSweep_.reset();
        concurrentPhase_ = Phase::None;
        if (!backgroundThread)
          checkTripwireAndSubmitStats();
      }
      break;
    default:
      llvm_unreachable("No other possible state between iterations");
  }
}

void HadesGC::prepareCompactee(bool forceCompaction) {
  assert(gcMutex_);
  assert(
      compactee_.empty() &&
      "Ongoing compaction at the start of an OG collection.");
  if (promoteYGToOG_)
    return;

#ifdef HERMESVM_SANITIZE_HANDLES
  // Handle-SAN forces a compaction to move some OG objects.
  if (sanitizeRate_)
    forceCompaction = true;
#endif

  // To avoid compacting too often, keep a buffer of one segment or 5% of the
  // heap (whichever is greater). Since the selected segment will be removed
  // from the heap, we only want to compact if there are at least 2 segments in
  // the OG.
  uint64_t buffer = std::max<uint64_t>(
      oldGen_.targetSizeBytes() / 20, HeapSegment::maxSize());
  uint64_t threshold = oldGen_.targetSizeBytes() + buffer;
  uint64_t totalBytes = oldGen_.size() + oldGen_.externalBytes();
  if ((forceCompaction || totalBytes > threshold) &&
      oldGen_.numSegments() > 1) {
    compactee_.segment = std::make_shared<HeapSegment>(oldGen_.popSegment());
    addSegmentExtentToCrashManager(
        *compactee_.segment, kCompacteeNameForCrashMgr);
    compactee_.start = compactee_.segment->lowLim();
    compactee_.startCP = CompressedPointer::encodeNonNull(
        reinterpret_cast<GCCell *>(compactee_.segment->lowLim()),
        getPointerBase());
    compacteeHandleForSweep_ = compactee_.segment;
  }
}

void HadesGC::finalizeCompactee() {
  char *stop = compactee_.segment->level();
  char *cur = compactee_.segment->start();
  PointerBase &base = getPointerBase();
  // Calculate the total number of bytes that were allocated in the compactee at
  // the start of compaction.
  int32_t preAllocated = 0;
  while (cur < stop) {
    auto *cell = reinterpret_cast<GCCell *>(cur);
    if (cell->hasMarkedForwardingPointer()) {
      auto size = cell->getMarkedForwardingPointer()
                      .getNonNull(base)
                      ->getAllocatedSize();
      preAllocated += size;
      cur += size;
    } else {
      auto size = cell->getAllocatedSize();
      if (!vmisa<OldGen::FreelistCell>(cell)) {
        cell->getVT()->finalizeIfExists(cell, *this);
        preAllocated += size;
      }
      cur += size;
    }
  }
  // At this point, any cells that survived compaction are already accounted for
  // separately in the counter, so we just need to subtract the number of bytes
  // allocated in the compactee.
  oldGen_.incrementAllocatedBytes(-preAllocated);

  const size_t segIdx =
      SegmentInfo::segmentIndexFromStart(compactee_.segment->lowLim());
  segmentIndices_.push_back(segIdx);
  removeSegmentExtentFromCrashManager(std::to_string(segIdx));
  removeSegmentExtentFromCrashManager(kCompacteeNameForCrashMgr);
  compactee_ = {};
}

void HadesGC::updateOldGenThreshold() {
  // TODO: Dynamic threshold is not used in incremental mode because
  // getDrainRate computes the mark rate directly based on the threshold. This
  // means that increasing the threshold would operate like a one way ratchet.
  if (!kConcurrentGC)
    return;

  const double markedBytes = oldGenMarker_->markedBytes();
  const double preAllocated = ogCollectionStats_->beforeAllocatedBytes();
  assert(markedBytes <= preAllocated && "Cannot mark more than was allocated");
  const double postAllocated = oldGen_.allocatedBytes();
  assert(postAllocated >= preAllocated && "Cannot free memory during marking");

  // Calculate the number of bytes marked for each byte allocated into the old
  // generation. Note that this is skewed heavily by the number of young gen
  // collections that occur during marking, and therefore the size of the heap.
  // 1. In small heaps, this underestimates the true mark rate, because marking
  // may finish shortly after it starts, but we have to wait until the next YG
  // collection is complete. This is desirable, because we need a more
  // conservative margin in small heaps to avoid running over the heap limit.
  // 2. In large heaps, this approaches the true mark rate, because there will
  // be several young gen collections, giving us more accurate and finer grained
  // information on the allocation rate.
  const auto concurrentMarkRate =
      markedBytes / std::max(postAllocated - preAllocated, 1.0);

  // If the heap is completely full and we are running into blocking
  // collections, then it is possible that almost nothing is allocated into the
  // OG during the mark phase. Without correction, can become a self-reinforcing
  // cycle because it will cause the mark rate to be overestimated, making
  // collections start later, further increasing the odds of a blocking
  // collection. Empirically, the mark rate can be much higher than the below
  // limit, but we get diminishing returns with increasing mark rate, since the
  // threshold just asymptotically approaches 1.
  const auto clampedRate = std::min(concurrentMarkRate, 20.0);

  // Update the collection threshold using the newly computed mark rate. To add
  // a margin of safety, we assume everything in the heap at the time we hit the
  // threshold needs to be marked. This margin allows for variance in the
  // marking rate, and gives time for sweeping to start. The update is
  // calculated by viewing the threshold as the bytes to mark and the remainder
  // after the threshold as the bytes to fill. We can then solve for it using:
  // MarkRate = Threshold / (1 - Threshold)
  // This has some nice properties:
  // 1. As the threshold increases, the safety margin also increases (since the
  // safety margin is just the difference between the threshold and the
  // occupancy ratio).
  // 2. It neatly fits the range [0, 1) for a mark rate in [0, infinity). There
  // is no risk of division by zero.
  ogThreshold_.update(clampedRate / (clampedRate + 1));
}

void HadesGC::completeMarking() {
  assert(inGC() && "inGC_ must be set during the STW pause");
  // Update the collection threshold before marking anything more, so that only
  // the concurrently marked bytes are part of the calculation.
  updateOldGenThreshold();
  ogMarkingBarriers_ = false;
  // No locks are needed here because the world is stopped and there is only 1
  // active thread.
  oldGenMarker_->globalWorklist().flushPushChunk();
  {
    // Remark any roots that may have changed without executing barriers.
    DroppingAcceptor<MarkAcceptor> nameAcceptor{*oldGenMarker_};
    gcCallbacks_.markRootsForCompleteMarking(nameAcceptor);
  }
  // Drain the marking queue.
  oldGenMarker_->drainAllWork();
  assert(
      oldGenMarker_->globalWorklist().empty() &&
      "Marking worklist wasn't drained");
  completeWeakMapMarking(*oldGenMarker_);
  // Update the compactee tracking pointers so that the next YG collection will
  // do a compaction.
  compactee_.evacStart = compactee_.start;
  compactee_.evacStartCP = compactee_.startCP;
  assert(
      oldGenMarker_->globalWorklist().empty() &&
      "Marking worklist wasn't drained");
  // Reset weak roots to null after full reachability has been
  // determined.
  MarkWeakRootsAcceptor acceptor{*this};
  markWeakRoots(acceptor, /*markLongLived*/ true);

  // Now free symbols and weak refs.
  gcCallbacks_.freeSymbols(oldGenMarker_->markedSymbols());
  // NOTE: If sweeping is done concurrently with YG collection, weak references
  // could be handled during the sweep pass instead of the mark pass. The read
  // barrier will need to be updated to handle the case where a WeakRef points
  // to an now-empty cell.
  updateWeakReferencesForOldGen();

  // Nothing needs oldGenMarker_ from this point onward.
  oldGenMarker_.reset();
}

void HadesGC::finalizeAll() {
  std::lock_guard<Mutex> lk{gcMutex_};
  // Terminate any existing OG collection.
  concurrentPhase_ = Phase::None;
  if (ogCollectionStats_)
    ogCollectionStats_->markUsed();
  // In case of an OOM, we may be in the middle of a YG collection.
  if (ygCollectionStats_)
    ygCollectionStats_->markUsed();
  // Now finalize the heap.
  // We might be in the middle of a YG collection, with some objects promoted to
  // the OG, and some not. Only finalize objects that have not been promoted to
  // OG, and let the OG finalize the promoted objects.
  finalizeYoungGenObjects();

  // If we are in the middle of a YG collection, some objects may have already
  // been promoted to the OG. Assume that any remaining external memory in the
  // YG belongs to those objects.
  transferExternalMemoryToOldGen();

  const auto finalizeCallback = [this](GCCell *cell) {
    assert(cell->isValid() && "Invalid cell in finalizeAll");
    cell->getVT()->finalizeIfExists(cell, *this);
  };
  if (compactee_.segment)
    compactee_.segment->forCompactedObjs(finalizeCallback, getPointerBase());

  for (HeapSegment &seg : oldGen_)
    seg.forAllObjs(finalizeCallback);
}

void HadesGC::creditExternalMemory(GCCell *cell, uint32_t sz) {
  assert(canAllocExternalMemory(sz) && "Precondition");
  if (inYoungGen(cell)) {
    size_t newYGExtBytes = getYoungGenExternalBytes() + sz;
    setYoungGenExternalBytes(newYGExtBytes);
    auto adj = std::min<size_t>(sz, youngGen_.available());
    youngGen_.setEffectiveEnd(youngGen_.effectiveEnd() - adj);
  } else {
    std::lock_guard<Mutex> lk{gcMutex_};
    oldGen_.creditExternalMemory(sz);
    uint64_t totalBytes = oldGen_.allocatedBytes() + oldGen_.externalBytes();
    if (totalBytes > oldGen_.targetSizeBytes())
      youngGen_.setEffectiveEnd(youngGen_.level());
  }
}

void HadesGC::debitExternalMemory(GCCell *cell, uint32_t sz) {
  if (inYoungGen(cell)) {
    size_t oldYGExtBytes = getYoungGenExternalBytes();
    assert(
        oldYGExtBytes >= sz && "Debiting more native memory than was credited");
    setYoungGenExternalBytes(oldYGExtBytes - sz);
    // Don't modify the effective end here. creditExternalMemory is in charge
    // of tracking this. We don't expect many debits to not be from finalizers
    // anyway.
  } else {
    std::lock_guard<Mutex> lk{gcMutex_};
    oldGen_.debitExternalMemory(sz);
  }
}

void HadesGC::writeBarrierSlow(const GCHermesValue *loc, HermesValue value) {
  if (ogMarkingBarriers_) {
    snapshotWriteBarrierInternal(*loc);
  }
  if (!value.isPointer()) {
    return;
  }
  relocationWriteBarrier(loc, value.getPointer());
}

void HadesGC::writeBarrierSlow(
    const GCSmallHermesValue *loc,
    SmallHermesValue value) {
  if (ogMarkingBarriers_) {
    snapshotWriteBarrierInternal(*loc);
  }
  if (!value.isPointer()) {
    return;
  }
  relocationWriteBarrier(loc, value.getPointer(getPointerBase()));
}

void HadesGC::writeBarrierSlow(const GCPointerBase *loc, const GCCell *value) {
  if (*loc && ogMarkingBarriers_)
    snapshotWriteBarrierInternal(*loc);
  // Always do the non-snapshot write barrier in order for YG to be able to
  // scan cards.
  relocationWriteBarrier(loc, value);
}

void HadesGC::constructorWriteBarrierSlow(
    const GCHermesValue *loc,
    HermesValue value) {
  // A constructor never needs to execute a SATB write barrier, since its
  // previous value was definitely not live.
  if (!value.isPointer()) {
    return;
  }
  relocationWriteBarrier(loc, value.getPointer());
}

void HadesGC::constructorWriteBarrierSlow(
    const GCSmallHermesValue *loc,
    SmallHermesValue value) {
  // A constructor never needs to execute a SATB write barrier, since its
  // previous value was definitely not live.
  if (!value.isPointer()) {
    return;
  }
  relocationWriteBarrier(loc, value.getPointer(getPointerBase()));
}

void HadesGC::constructorWriteBarrierRangeSlow(
    const GCHermesValue *start,
    uint32_t numHVs) {
  assert(
      AlignedStorage::containedInSame(start, start + numHVs) &&
      "Range must start and end within a heap segment.");

  // Most constructors should be running in the YG, so in the common case, we
  // can avoid doing anything for the whole range. If the range is in the OG,
  // then just dirty all the cards corresponding to it, and we can scan them for
  // pointers later. This is less precise but makes the write barrier faster.

  AlignedHeapSegment::cardTableCovering(start)->dirtyCardsForAddressRange(
      start, start + numHVs);
}

void HadesGC::constructorWriteBarrierRangeSlow(
    const GCSmallHermesValue *start,
    uint32_t numHVs) {
  assert(
      AlignedStorage::containedInSame(start, start + numHVs) &&
      "Range must start and end within a heap segment.");
  AlignedHeapSegment::cardTableCovering(start)->dirtyCardsForAddressRange(
      start, start + numHVs);
}

void HadesGC::snapshotWriteBarrierRangeSlow(
    const GCHermesValue *start,
    uint32_t numHVs) {
  for (uint32_t i = 0; i < numHVs; ++i) {
    snapshotWriteBarrierInternal(start[i]);
  }
}

void HadesGC::snapshotWriteBarrierRangeSlow(
    const GCSmallHermesValue *start,
    uint32_t numHVs) {
  for (uint32_t i = 0; i < numHVs; ++i) {
    snapshotWriteBarrierInternal(start[i]);
  }
}

void HadesGC::snapshotWriteBarrierInternal(GCCell *oldValue) {
  assert(
      (oldValue->isValid()) &&
      "Invalid cell encountered in snapshotWriteBarrier");
  if (!inYoungGen(oldValue)) {
    HERMES_SLOW_ASSERT(
        dbgContains(oldValue) &&
        "Non-heap pointer encountered in snapshotWriteBarrier");
    oldGenMarker_->globalWorklist().enqueue(oldValue);
  }
}

void HadesGC::snapshotWriteBarrierInternal(CompressedPointer oldValue) {
  assert(
      (oldValue.get(getPointerBase())->isValid()) &&
      "Invalid cell encountered in snapshotWriteBarrier");
  if (!inYoungGen(oldValue)) {
    GCCell *ptr = oldValue.get(getPointerBase());
    HERMES_SLOW_ASSERT(
        dbgContains(ptr) &&
        "Non-heap pointer encountered in snapshotWriteBarrier");
    oldGenMarker_->globalWorklist().enqueue(ptr);
  }
}

void HadesGC::snapshotWriteBarrierInternal(HermesValue oldValue) {
  if (oldValue.isPointer()) {
    snapshotWriteBarrierInternal(static_cast<GCCell *>(oldValue.getPointer()));
  } else if (oldValue.isSymbol()) {
    // Symbols need snapshot write barriers.
    snapshotWriteBarrierInternal(oldValue.getSymbol());
  }
}

void HadesGC::snapshotWriteBarrierInternal(SmallHermesValue oldValue) {
  if (oldValue.isPointer()) {
    snapshotWriteBarrierInternal(oldValue.getPointer());
  } else if (oldValue.isSymbol()) {
    // Symbols need snapshot write barriers.
    snapshotWriteBarrierInternal(oldValue.getSymbol());
  }
}

void HadesGC::snapshotWriteBarrierInternal(SymbolID symbol) {
  HERMES_SLOW_ASSERT(
      ogMarkingBarriers_ &&
      "snapshotWriteBarrier should only be called while the OG is marking");
  oldGenMarker_->markSymbol(symbol);
}

void HadesGC::relocationWriteBarrier(const void *loc, const void *value) {
  assert(!inYoungGen(loc) && "Pre-condition from other callers");
  // Do not dirty cards for compactee->compactee, yg->yg, or yg->compactee
  // pointers. But do dirty cards for compactee->yg pointers, since compaction
  // may not happen in the next YG.
  if (AlignedStorage::containedInSame(loc, value)) {
    return;
  }
  if (inYoungGen(value) || compactee_.contains(value)) {
    // Only dirty a card if it's an old-to-young or old-to-compactee pointer.
    // This is fine to do since the GC never modifies card tables outside of
    // allocation.
    // Note that this *only* applies since the boundaries are updated separately
    // from the card table being marked itself.
    HeapSegment::cardTableCovering(loc)->dirtyCardForAddress(loc);
  }
}

void HadesGC::weakRefReadBarrier(GCCell *value) {
  assert(
      !calledByBackgroundThread() &&
      "Read barrier invoked by background thread.");
  // If the GC is marking, conservatively mark the value as live.
  if (ogMarkingBarriers_)
    snapshotWriteBarrierInternal(value);

  // Otherwise, if no GC is active at all, the weak ref must be alive.
  // During sweeping there's no special handling either.
}

bool HadesGC::canAllocExternalMemory(uint32_t size) {
  return size <= maxHeapSize_;
}

WeakRefSlot *HadesGC::allocWeakSlot(CompressedPointer ptr) {
  assert(
      !calledByBackgroundThread() &&
      "allocWeakSlot should only be called from the mutator");
  // The weak ref mutex doesn't need to be held since weakSlots_ and
  // firstFreeWeak_ are only modified while the world is stopped.
  WeakRefSlot *slot;
  if (firstFreeWeak_) {
    assert(
        firstFreeWeak_->state() == WeakSlotState::Free &&
        "invalid free slot state");
    slot = firstFreeWeak_;
    firstFreeWeak_ = firstFreeWeak_->nextFree();
    slot->reset(ptr);
  } else {
    weakSlots_.push_back({ptr});
    slot = &weakSlots_.back();
  }
  if (ogMarkingBarriers_) {
    // During the mark phase, if a WeakRef is created, it might not be marked
    // if the object holding this new WeakRef has already been visited.
    // This doesn't need the WeakRefMutex because nothing is using this slot
    // yet.
    slot->mark();
  }
  return slot;
}

void HadesGC::freeWeakSlot(WeakRefSlot *slot) {
  // Sets the given WeakRefSlot to point to firstFreeWeak_ instead of a cell.
  slot->free(firstFreeWeak_);
  firstFreeWeak_ = slot;
}

void HadesGC::forAllObjs(const std::function<void(GCCell *)> &callback) {
  std::lock_guard<Mutex> lk{gcMutex_};
  youngGen().forAllObjs(callback);

  const auto skipGarbageCallback = [callback](GCCell *cell) {
    // If we're doing this check during sweeping, or between sweeping and
    // compaction, there might be some objects that are dead, and could
    // potentially have garbage in them. There's no need to check the
    // pointers of those objects.
    if (HeapSegment::getCellMarkBit(cell)) {
      callback(cell);
    }
  };
  for (HeapSegment &seg : oldGen_) {
    if (concurrentPhase_ != Phase::Sweep)
      seg.forAllObjs(callback);
    else
      seg.forAllObjs(skipGarbageCallback);
  }
  if (compactee_.segment) {
    if (!compactee_.evacActive())
      compactee_.segment->forAllObjs(callback);
    else
      compactee_.segment->forAllObjs(skipGarbageCallback);
  }
}

void HadesGC::ttiReached() {
  promoteYGToOG_ = false;
}

#ifndef NDEBUG

bool HadesGC::calledByBackgroundThread() const {
  // If the background thread is active, check if this thread matches the
  // background thread.
  return kConcurrentGC &&
      backgroundExecutor_->getThreadId() == std::this_thread::get_id();
}

bool HadesGC::validPointer(const void *p) const {
  return dbgContains(p) && static_cast<const GCCell *>(p)->isValid();
}

bool HadesGC::dbgContains(const void *p) const {
  return inYoungGen(p) || inOldGen(p);
}

void HadesGC::trackReachable(CellKind kind, unsigned sz) {}
#endif

void *HadesGC::allocSlow(uint32_t sz) {
  AllocResult res;
  // Failed to alloc in young gen, do a young gen collection.
  youngGenCollection(
      kNaturalCauseForAnalytics, /*forceOldGenCollection*/ false);
  res = youngGen().bumpAlloc(sz);
  if (res.success)
    return res.ptr;

  // Still fails after YG collection, perhaps it is a large alloc, try growing
  // the YG to full size.
  youngGen().clearExternalMemoryCharge();
  res = youngGen().bumpAlloc(sz);
  if (res.success)
    return res.ptr;

  // A YG collection is guaranteed to fully evacuate, leaving all the space
  // available, so the only way this could fail is if sz is greater than
  // a segment size.
  // This would be an error in VM code to ever allow such a size to be
  // allocated, and thus there's an assert at the top of this function to
  // disallow that. This case is for production, if we miss a test case.
  oom(make_error_code(OOMError::SuperSegmentAlloc));
}

void *HadesGC::allocLongLived(uint32_t sz) {
  assert(
      isSizeHeapAligned(sz) &&
      "Call to allocLongLived must use a size aligned to HeapAlign");
  if (kConcurrentGC) {
    HERMES_SLOW_ASSERT(
        !weakRefMutex() &&
        "WeakRef mutex should not be held when allocLongLived is called");
  }
  assert(gcMutex_ && "GC mutex must be held when calling allocLongLived");
  totalAllocatedBytes_ += sz;
  // Alloc directly into the old gen.
  return oldGen_.alloc(sz);
}

GCCell *HadesGC::OldGen::alloc(uint32_t sz) {
  assert(
      isSizeHeapAligned(sz) &&
      "Should be aligned before entering this function");
  assert(sz >= minAllocationSize() && "Allocating too small of an object");
  assert(sz <= maxAllocationSize() && "Allocating too large of an object");
  assert(gc_.gcMutex_ && "gcMutex_ must be held before calling oldGenAlloc");
  if (GCCell *cell = search(sz)) {
    return cell;
  }
  // Before waiting for a collection to finish, check if we're below the max
  // heap size and can simply allocate another segment. This will prevent
  // blocking the YG unnecessarily.
  llvh::ErrorOr<HeapSegment> seg = gc_.createSegment();
  if (seg) {
    // Complete this allocation using a bump alloc.
    AllocResult res = seg->bumpAlloc(sz);
    assert(
        res.success &&
        "A newly created segment should always be able to allocate");
    // Set the cell head for any successful alloc, so that write barriers can
    // move from dirty cards to the head of the object.
    seg->setCellHead(static_cast<GCCell *>(res.ptr), sz);
    // Add the segment to segments_ and add the remainder of the segment to the
    // free list.
    addSegment(std::move(seg.get()));
    GCCell *newObj = static_cast<GCCell *>(res.ptr);
    HeapSegment::setCellMarkBit(newObj);
    return newObj;
  }

  // TODO(T109282643): Block on any pending OG collections here in case they
  // free up space.

  // Repeat the search in case the collection did free memory.
  if (GCCell *cell = search(sz)) {
    return cell;
  }

  // The GC didn't recover enough memory, OOM.
  // Re-use the error code from the earlier heap segment allocation, because
  // it's either that the max heap size was reached, or that segment failed to
  // allocate.
  gc_.oom(seg.getError());
}

uint32_t HadesGC::OldGen::getFreelistBucket(uint32_t size) {
  // If the size corresponds to the "small" portion of the freelist, then the
  // bucket is just (size) / (heap alignment)
  if (size < kMinSizeForLargeBlock) {
    auto bucket = size >> LogHeapAlign;
    assert(
        bucket < kNumSmallFreelistBuckets &&
        "Small blocks must be within the small free list range");
    return bucket;
  }
  // Otherwise, index into the large portion of the freelist, which is based on
  // powers of 2

  auto bucket =
      kNumSmallFreelistBuckets + llvh::Log2_32(size) - kLogMinSizeForLargeBlock;
  assert(
      bucket < kNumFreelistBuckets &&
      "Block size must be within the freelist range!");
  return bucket;
}

GCCell *HadesGC::OldGen::search(uint32_t sz) {
  size_t bucket = getFreelistBucket(sz);
  if (bucket < kNumSmallFreelistBuckets) {
    // Fast path: There already exists a size bucket for this alloc. Check if
    // there's a free cell to take and exit.
    if (auto *segBucket = buckets_[bucket].next) {
      assert(freelistBucketBitArray_.at(bucket));
      FreelistCell *cell = removeCellFromFreelist(bucket, segBucket);
      assert(
          cell->getAllocatedSize() == sz &&
          "Size bucket should be an exact match");
      return finishAlloc(cell, sz);
    }
    // Make sure we start searching at the smallest possible size that could fit
    bucket = getFreelistBucket(sz + minAllocationSize());
  }
  // Once we're examining the rest of the free list, it's a first-fit algorithm.
  // This approach approximates finding the smallest possible fit.
  bucket = freelistBucketBitArray_.findNextSetBitFrom(bucket);
  for (; bucket < kNumFreelistBuckets;
       bucket = freelistBucketBitArray_.findNextSetBitFrom(bucket + 1)) {
    auto *segBucket = buckets_[bucket].next;
    do {
      // Need to track the previous entry in order to change the next pointer.
      AssignableCompressedPointer *prevLoc = &segBucket->head;
      AssignableCompressedPointer cellCP = segBucket->head;

      do {
        auto *cell =
            vmcast<FreelistCell>(cellCP.getNonNull(gc_.getPointerBase()));
        assert(
            cellCP == *prevLoc &&
            "prevLoc should be updated in each iteration");
        assert(
            (!cell->next_ ||
             cell->next_.getNonNull(gc_.getPointerBase())->isValid()) &&
            "Next pointer points to an invalid cell");
        const auto cellSize = cell->getAllocatedSize();
        assert(
            getFreelistBucket(cellSize) == bucket &&
            "Found an incorrectly sized block in this bucket");
        // Check if the size is large enough that the cell could be split.
        if (cellSize >= sz + minAllocationSize()) {
          // Split the free cell. In order to avoid initializing
          // soon-to-be-unused values like the size and the next pointer, copy
          // the return path here.
          auto newCell = cell->carve(sz);
          // Since the size of cell has changed, we may need to add it to a
          // different free list bucket.
          auto newBucket = getFreelistBucket(cell->getAllocatedSize());
          assert(newBucket <= bucket && "Split cell must be smaller.");
          if (newBucket != bucket) {
            removeCellFromFreelist(prevLoc, bucket, segBucket);
            // Since the buckets for each segment are stored contiguously in
            // memory, we can compute the address of the SegmentBucket for
            // newBucket in this segment relative to the current SegmentBucket.
            auto diff = bucket - newBucket;
            addCellToFreelist(cell, segBucket - diff);
          }
          // Because we carved newCell out before removing cell from the
          // freelist, newCell is still poisoned (regardless of whether the
          // conditional above executed). Unpoison it.
          __asan_unpoison_memory_region(newCell, sz);
          return finishAlloc(newCell, sz);
        } else if (cellSize == sz) {
          // Exact match, take it.
          removeCellFromFreelist(prevLoc, bucket, segBucket);
          return finishAlloc(cell, sz);
        }
        // Non-exact matches, or anything just barely too small to fit, will
        // need to find another block.
        // NOTE: This is due to restrictions on the minimum cell size to keep
        // the heap parseable, especially in debug mode. If this minimum size
        // becomes smaller (smaller header, size becomes part of it
        // automatically, debug magic field is handled differently), this
        // decisions can be re-examined. An example alternative is to make a
        // special fixed-size cell that is only as big as an empty GCCell. That
        // alternative only works if the empty is small enough to fit in any gap
        // in the heap. That's not true in debug modes currently.
        prevLoc = &cell->next_;
        cellCP = cell->next_;
      } while (cellCP);
      segBucket = segBucket->next;
    } while (segBucket);
  }
  return nullptr;
}

template <typename Acceptor>
void HadesGC::youngGenEvacuateImpl(Acceptor &acceptor, bool doCompaction) {
  // Marking each object puts it onto an embedded free list.
  {
    DroppingAcceptor<Acceptor> nameAcceptor{acceptor};
    markRoots(nameAcceptor, /*markLongLived*/ doCompaction);
  }
  // Find old-to-young pointers, as they are considered roots for YG
  // collection.
  scanDirtyCards(acceptor);
  // Iterate through the copy list to find new pointers.
  while (CopyListCell *const copyCell = acceptor.pop()) {
    assert(
        copyCell->hasMarkedForwardingPointer() && "Discovered unmarked object");
    assert(
        (inYoungGen(copyCell) || compactee_.evacContains(copyCell)) &&
        "Unexpected object in YG collection");
    // Update the pointers inside the forwarded object, since the old
    // object is only there for the forwarding pointer.
    GCCell *const cell =
        copyCell->getMarkedForwardingPointer().getNonNull(getPointerBase());
    markCell(cell, acceptor);
  }

  // Mark weak roots. We only need to update the long lived weak roots if we are
  // evacuating part of the OG.
  markWeakRoots(acceptor, /*markLongLived*/ doCompaction);
}

void HadesGC::youngGenCollection(
    std::string cause,
    bool forceOldGenCollection) {
  ygCollectionStats_ = std::make_unique<CollectionStats>(*this, cause, "young");
  ygCollectionStats_->beginCPUTimeSection();
  ygCollectionStats_->setBeginTime();
  // Acquire the GC lock for the duration of the YG collection.
  auto lk = ensureBackgroundTaskPaused();
  // The YG is not parseable while a collection is occurring.
  assert(!inGC() && "Cannot be in GC at the start of YG!");
  GCCycle cycle{*this, "GC Young Gen"};
#ifdef HERMES_SLOW_DEBUG
  checkWellFormed();
  // Check that the card tables are well-formed before the collection.
  verifyCardTable();
#endif
  assert(
      youngGen().markBitArray().findNextUnmarkedBitFrom(0) ==
          youngGen().markBitArray().size() &&
      "Young gen segment must have all mark bits set");
  struct {
    uint64_t before{0};
    uint64_t after{0};
  } heapBytes, externalBytes;
  heapBytes.before = youngGen().used();
  externalBytes.before = getYoungGenExternalBytes();
  // YG is about to be emptied, add all of the allocations.
  totalAllocatedBytes_ += youngGen().used();
  const bool doCompaction = compactee_.evacActive();
  // Attempt to promote the YG segment to OG if the flag is set. If this call
  // fails for any reason, proceed with a GC.
  if (promoteYoungGenToOldGen()) {
    // Leave sweptBytes and sweptExternalBytes as defaults (which are 0).
    // Don't update the average YG survival ratio since no liveness was
    // calculated for the promotion case.
    ygCollectionStats_->setBeforeSizes(
        heapBytes.before, externalBytes.before, segmentFootprint());
    ygCollectionStats_->addCollectionType("promotion");
    assert(!doCompaction && "Cannot do compactions during YG promotions.");
  } else {
    auto &yg = youngGen();

    if (compactee_.segment) {
      EvacAcceptor<true> acceptor{*this};
      youngGenEvacuateImpl(acceptor, doCompaction);
      // The remaining bytes after the collection is just the number of bytes
      // that were evacuated.
      heapBytes.after = acceptor.evacuatedBytes();
    } else {
      EvacAcceptor<false> acceptor{*this};
      youngGenEvacuateImpl(acceptor, false);
      heapBytes.after = acceptor.evacuatedBytes();
    }
    // Inform trackers about objects that died during this YG collection.
    if (isTrackingIDs()) {
      auto trackerCallback = [this](GCCell *cell) {
        // The compactee might have free list cells, which are not tracked.
        // untrackObject requires the object to have been tracked previously.
        // So skip free list cells here.
        if (!vmisa<OldGen::FreelistCell>(cell)) {
          untrackObject(cell, cell->getAllocatedSize());
        }
      };
      yg.forCompactedObjs(trackerCallback, getPointerBase());
      if (doCompaction) {
        compactee_.segment->forCompactedObjs(trackerCallback, getPointerBase());
      }
    }
    // Run finalizers for young gen objects.
    finalizeYoungGenObjects();
    // This was modified by debitExternalMemoryFromFinalizer, called by
    // finalizers. The difference in the value before to now was the swept bytes
    externalBytes.after = getYoungGenExternalBytes();
    // Now the copy list is drained, and all references point to the old
    // gen. Clear the level of the young gen.
    yg.resetLevel();
    assert(
        yg.markBitArray().findNextUnmarkedBitFrom(0) ==
            yg.markBitArray().size() &&
        "Young gen segment must have all mark bits set");

    if (doCompaction) {
      ygCollectionStats_->addCollectionType("compact");
      // We can use the total amount of external memory in the OG before and
      // after running finalizers to measure how much external memory has been
      // released.
      uint64_t ogExternalBefore = oldGen_.externalBytes();
      // Similarly, finalizeCompactee will update the allocated bytes counter to
      // remove bytes allocated in the compactee.
      uint64_t ogAllocatedBefore = oldGen_.allocatedBytes();
      finalizeCompactee();
      heapBytes.before += ogAllocatedBefore - oldGen_.allocatedBytes();
      const uint64_t externalCompactedBytes =
          ogExternalBefore - oldGen_.externalBytes();
      // Since we can't track the actual number of external bytes that were in
      // this segment, just use the swept external byte count.
      externalBytes.before += externalCompactedBytes;
      externalBytes.after += externalCompactedBytes;
    }

    // Move external memory accounting from YG to OG as well.
    transferExternalMemoryToOldGen();

    // Potentially resize the YG if this collection did not meet our pause time
    // goals. Exclude compacting collections and the portion of YG time spent on
    // incremental OG collections, since they distort pause times and are
    // unaffected by YG size.
    if (!doCompaction)
      updateYoungGenSizeFactor();

    // The effective end of our YG is no longer accurate for multiple reasons:
    // 1. transferExternalMemoryToOldGen resets the effectiveEnd to be the end.
    // 2. Creating a large alloc in the YG can increase the effectiveEnd.
    // 3. The duration of this collection may not have met our pause time goals.
    youngGen().setEffectiveEnd(
        youngGen().start() +
        static_cast<size_t>(ygSizeFactor_ * HeapSegment::maxSize()));

    // We have to set these after the collection, in case a compaction took
    // place and updated these metrics.
    ygCollectionStats_->setBeforeSizes(
        heapBytes.before, externalBytes.before, segmentFootprint());

    // The swept bytes are just the bytes that were not evacuated.
    ygCollectionStats_->setSweptBytes(heapBytes.before - heapBytes.after);
    ygCollectionStats_->setSweptExternalBytes(
        externalBytes.before - externalBytes.after);
    ygCollectionStats_->setAfterSize(segmentFootprint());
    // If this is not a compacting YG, update the average survival bytes.
    // In a compacting YG, since the evacuatedBytes counter tracks both
    // segments, this value is not a useful predictor of future collections.
    if (!doCompaction)
      ygAverageSurvivalBytes_.update(
          ygCollectionStats_->afterAllocatedBytes() +
          ygCollectionStats_->afterExternalBytes());
  }
#ifdef HERMES_SLOW_DEBUG
  // Check that the card tables are well-formed after the collection.
  verifyCardTable();
#endif
  // Perform any pending work for an ongoing OG collection.
  // Do this before starting a new collection in case we need collections
  // back-to-back. Also, don't check this after starting a collection to avoid
  // waiting for something that is both unlikely, and will increase the pause
  // time if it does happen.
  yieldToOldGen();
  // We can only consider starting a new OG collection if any previous OG
  // collection is fully completed and has not left a pending compaction. This
  // handles the rare case where an OG collection was completed during this YG
  // collection, and the compaction will therefore only be completed in the next
  // collection.
  if (concurrentPhase_ == Phase::None && !compactee_.evacActive()) {
    // There is no OG collection running, check the tripwire in case this is the
    // first YG after an OG completed.
    checkTripwireAndSubmitStats();
    if (forceOldGenCollection) {
      // If an OG collection is being forced, it's because something called
      // collect directly, most likely from a memory warning. In order to
      // respond to memory pressure effectively, the OG should compact.
      oldGenCollection(std::move(cause), /*forceCompaction*/ true);
    } else {
      // If the OG is sufficiently full after the collection finishes, begin
      // an OG collection.
      // External bytes are part of the numerator and denominator, because they
      // should not be included as part of determining the heap's occupancy, but
      // instead just influence when collections begin.
      const uint64_t totalAllocated =
          oldGen_.allocatedBytes() + oldGen_.externalBytes();
      const uint64_t totalBytes = oldGen_.targetSizeBytes();
      double allocatedRatio = static_cast<double>(totalAllocated) / totalBytes;
      if (allocatedRatio >= ogThreshold_) {
        oldGenCollection(kNaturalCauseForAnalytics, /*forceCompaction*/ false);
      }
    }
  }
#ifdef HERMES_SLOW_DEBUG
  // Run a well-formed check before exiting.
  checkWellFormed();
#endif
  ygCollectionStats_->setEndTime();
  ygCollectionStats_->endCPUTimeSection();
  recordGCStats(std::move(*ygCollectionStats_).getEvent(), true);
  ygCollectionStats_.reset();
}

bool HadesGC::promoteYoungGenToOldGen() {
  if (!promoteYGToOG_) {
    return false;
  }

  // Attempt to create a new segment, if that fails, turn off the flag to
  // disable GC and return false so we proceed with a GC.
  // TODO: Add more stringent criteria for turning off this flag, for instance,
  // once the heap reaches a certain size. That would avoid growing the heap to
  // the maximum possible size before stopping promotions.
  llvh::ErrorOr<HeapSegment> newYoungGen = createSegment();
  if (!newYoungGen) {
    promoteYGToOG_ = false;
    return false;
  }

  // Move the external memory costs to the OG. Needs to happen here so that the
  // YG segment moved to OG is not left with an effective end.
  transferExternalMemoryToOldGen();
  // The flag is on to prevent GC until TTI. Promote the whole YG segment
  // directly into OG.
  // Before promoting it, set the cell heads correctly for the segment
  // going into OG. This could be done at allocation time, but at a cost
  // to YG alloc times for a case that might not come up.
  youngGen_.forAllObjs([this](GCCell *cell) {
    youngGen_.setCellHead(cell, cell->getAllocatedSize());
  });
  // It is important that this operation is just a move of pointers to
  // segments. The addresses have to stay the same or else it would
  // require a marking pass through all objects.
  // This will also rename the segment in the crash data.
  oldGen_.addSegment(setYoungGen(std::move(newYoungGen.get())));

  return true;
}

HadesGC::HeapSegment HadesGC::setYoungGen(HeapSegment seg) {
  addSegmentExtentToCrashManager(seg, "YG");
  youngGenFinalizables_.clear();
  std::swap(youngGen_, seg);
  youngGenCP_ = CompressedPointer::encodeNonNull(
      reinterpret_cast<GCCell *>(youngGen_.lowLim()), getPointerBase());
  return seg;
}

void HadesGC::checkTripwireAndSubmitStats() {
  assert(
      gcMutex_ &&
      "gcMutex must be held before calling checkTripwireAndSubmitStats");
  assert(
      concurrentPhase_ == Phase::None &&
      "Cannot check stats while OG collection is ongoing.");
  if (!ogCollectionStats_) {
    return;
  }
  const auto usedBytes = ogCollectionStats_->afterAllocatedBytes() +
      ogCollectionStats_->afterExternalBytes();
  // We use the amount of live data from after a GC completed as the minimum
  // bound of what is live.
  checkTripwire(usedBytes);
  recordGCStats(std::move(*ogCollectionStats_).getEvent(), false);
  ogCollectionStats_.reset();
}

void HadesGC::transferExternalMemoryToOldGen() {
  oldGen_.creditExternalMemory(getYoungGenExternalBytes());
  setYoungGenExternalBytes(0);
  youngGen_.clearExternalMemoryCharge();
}

void HadesGC::updateYoungGenSizeFactor() {
  assert(
      ygSizeFactor_ <= 1.0 && ygSizeFactor_ >= 0.25 && "YG size out of range.");
  const auto ygDuration = ygCollectionStats_->getElapsedTime().count();
  // If the YG collection has taken less than 20% of our budgeted time, increase
  // the size of the YG by 10%.
  if (ygDuration < kTargetMaxPauseMs * 0.2)
    ygSizeFactor_ = std::min(ygSizeFactor_ * 1.1, 1.0);
  // If the YG collection has taken more than 40% of our budgeted time, decrease
  // the size of the YG by 10%. This is meant to leave some time for OG work.
  // However, don't let the YG size drop below 25% of the segment size.
  else if (ygDuration > kTargetMaxPauseMs * 0.4)
    ygSizeFactor_ = std::max(ygSizeFactor_ * 0.9, 0.25);
}

template <bool CompactionEnabled>
void HadesGC::scanDirtyCardsForSegment(
    SlotVisitor<EvacAcceptor<CompactionEnabled>> &visitor,
    HeapSegment &seg) {
  const auto &cardTable = seg.cardTable();
  // Use level instead of end in case the OG segment is still in bump alloc
  // mode.
  const char *const origSegLevel = seg.level();
  size_t from = cardTable.addressToIndex(seg.start());
  const size_t to = cardTable.addressToIndex(origSegLevel - 1) + 1;

  // If a compaction is taking place during sweeping, we may scan cards that
  // contain dead objects which in turn point to dead objects in the compactee.
  // In order to avoid promoting these dead objects, we should skip unmarked
  // objects altogether when compaction and sweeping happen at the same time.
  const bool visitUnmarked =
      !CompactionEnabled || concurrentPhase_ != Phase::Sweep;

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
    // Don't try to mark any cell past the original boundary of the segment.
    const void *const boundary = std::min(end, origSegLevel);

    // Use the object heads rather than the card table to discover the head
    // of the object.
    GCCell *const firstObj = seg.getFirstCellHead(iBegin);
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
    if (visitUnmarked || HeapSegment::getCellMarkBit(obj))
      markCellWithinRange(visitor, obj, obj->getKind(), begin, end);

    obj = obj->nextCell();
    // If there are additional objects in this card, scan them.
    if (LLVM_LIKELY(obj < boundary)) {
      // Mark the objects that are entirely contained within the dirty card
      // boundaries. In a given iteration, obj is the start of a given object,
      // and next is the start of the next object. Iterate until the last
      // object where next is within the card.
      for (GCCell *next = obj->nextCell(); next < boundary;
           next = next->nextCell()) {
        if (visitUnmarked || HeapSegment::getCellMarkBit(obj))
          markCell(visitor, obj, obj->getKind());
        obj = next;
      }

      // Mark the final object in the range with respect to the dirty card
      // boundaries.
      assert(
          obj < boundary && obj->nextCell() >= boundary &&
          "Last object in card must touch or cross cross the card boundary");
      if (visitUnmarked || HeapSegment::getCellMarkBit(obj))
        markCellWithinRange(visitor, obj, obj->getKind(), begin, end);
    }

    from = iEnd;
  }
}

template <bool CompactionEnabled>
void HadesGC::scanDirtyCards(EvacAcceptor<CompactionEnabled> &acceptor) {
  SlotVisitor<EvacAcceptor<CompactionEnabled>> visitor{acceptor};
  const bool preparingCompaction =
      CompactionEnabled && !compactee_.evacActive();
  // The acceptors in this loop can grow the old gen by adding another
  // segment, if there's not enough room to evac the YG objects discovered.
  // Since segments are always placed at the end, we can use indices instead
  // of iterators, which aren't invalidated. It's ok to not scan newly added
  // segments, since they are going to be handled from the rest of YG
  // collection.
  const auto segEnd = oldGen_.numSegments();
  for (size_t i = 0; i < segEnd; ++i) {
    // It is safe to hold this reference across a push_back into
    // oldGen_.segments_ since references into a deque are not invalidated.
    HeapSegment &seg = oldGen_[i];
    scanDirtyCardsForSegment(visitor, seg);
    // Do not clear the card table if the OG thread is currently marking to
    // prepare for a compaction. Note that we should clear the card tables if
    // the compaction is currently ongoing.
    if (!preparingCompaction)
      seg.cardTable().clear();
  }

  // No need to search dirty cards in the compactee segment if it is
  // currently being evacuated, since it will be scanned fully.
  if (preparingCompaction)
    scanDirtyCardsForSegment(visitor, *compactee_.segment);
}

void HadesGC::finalizeYoungGenObjects() {
  for (GCCell *cell : youngGenFinalizables_) {
    if (!cell->hasMarkedForwardingPointer()) {
      cell->getVT()->finalize(cell, *this);
    }
  }
  youngGenFinalizables_.clear();
}

void HadesGC::updateWeakReferencesForOldGen() {
  for (auto &slot : weakSlots_) {
    switch (slot.state()) {
      case WeakSlotState::Free:
        // Skip free weak slots.
        break;
      case WeakSlotState::Marked:
        // Set all allocated slots to unmarked.
        slot.unmark();
        break;
      case WeakSlotState::Unmarked:
        freeWeakSlot(&slot);
        break;
    }
  }
}

void HadesGC::completeWeakMapMarking(MarkAcceptor &acceptor) {
  gcheapsize_t weakMapAllocBytes = GCBase::completeWeakMapMarking(
      *this,
      acceptor,
      acceptor.reachableWeakMaps(),
      /*objIsMarked*/
      HeapSegment::getCellMarkBit,
      /*markFromVal*/
      [&acceptor](GCCell *valCell, GCHermesValue &valRef) {
        if (HeapSegment::getCellMarkBit(valCell)) {
          return false;
        }
        acceptor.accept(valRef);
        // The weak ref lock is held throughout this entire section, so no need
        // to re-lock it.
        acceptor.drainAllWork();
        return true;
      },
      /*drainMarkStack*/
      [](MarkAcceptor &acceptor) {
        // The weak ref lock is held throughout this entire section, so no need
        // to re-lock it.
        acceptor.drainAllWork();
      },
      /*checkMarkStackOverflow (HadesGC does not have mark stack overflow)*/
      []() { return false; });

  acceptor.reachableWeakMaps().clear();
  (void)weakMapAllocBytes;
}

uint64_t HadesGC::allocatedBytes() const {
  // This can be called very early in initialization, before YG is initialized.
  return (youngGen_ ? youngGen_.used() : 0) + oldGen_.allocatedBytes();
}

uint64_t HadesGC::externalBytes() const {
  return getYoungGenExternalBytes() + oldGen_.externalBytes();
}

uint64_t HadesGC::segmentFootprint() const {
  size_t totalSegments = oldGen_.numSegments() + (youngGen_ ? 1 : 0) +
      (compactee_.segment ? 1 : 0);
  return totalSegments * AlignedStorage::size();
}

uint64_t HadesGC::heapFootprint() const {
  return segmentFootprint() + externalBytes();
}

uint64_t HadesGC::OldGen::allocatedBytes() const {
  return allocatedBytes_;
}

void HadesGC::OldGen::incrementAllocatedBytes(int32_t incr) {
  allocatedBytes_ += incr;
  assert(allocatedBytes_ <= size() && "Invalid increment");
}

uint64_t HadesGC::OldGen::externalBytes() const {
  assert(gc_.gcMutex_ && "OG external bytes must be accessed under gcMutex_.");
  return externalBytes_;
}

uint64_t HadesGC::OldGen::size() const {
  size_t totalSegments = numSegments() + (gc_.compactee_.segment ? 1 : 0);
  return totalSegments * HeapSegment::maxSize();
}

uint64_t HadesGC::OldGen::targetSizeBytes() const {
  assert(gc_.gcMutex_ && "Must hold gcMutex_ when accessing targetSizeBytes_.");
  return targetSizeBytes_;
}

size_t HadesGC::getYoungGenExternalBytes() const {
  assert(
      !calledByBackgroundThread() &&
      "ygExternalBytes_ is only accessible from the mutator.");
  return ygExternalBytes_;
}
void HadesGC::setYoungGenExternalBytes(size_t sz) {
  assert(
      !calledByBackgroundThread() &&
      "ygExternalBytes_ is only accessible from the mutator.");
  ygExternalBytes_ = sz;
}

llvh::ErrorOr<size_t> HadesGC::getVMFootprintForTest() const {
  // Start by adding the YG.
  size_t footprint = 0;
  auto ygFootprint =
      hermes::oscompat::vm_footprint(youngGen_.start(), youngGen_.hiLim());
  // Check if the call failed.
  if (!ygFootprint)
    return ygFootprint;

  // Add each OG segment.
  for (const HeapSegment &seg : oldGen_) {
    auto segFootprint =
        hermes::oscompat::vm_footprint(seg.start(), seg.hiLim());
    if (!segFootprint)
      return segFootprint;
    footprint += *segFootprint;
  }
  return footprint;
}

std::deque<HadesGC::HeapSegment>::iterator HadesGC::OldGen::begin() {
  return segments_.begin();
}

std::deque<HadesGC::HeapSegment>::iterator HadesGC::OldGen::end() {
  return segments_.end();
}

std::deque<HadesGC::HeapSegment>::const_iterator HadesGC::OldGen::begin()
    const {
  return segments_.begin();
}

std::deque<HadesGC::HeapSegment>::const_iterator HadesGC::OldGen::end() const {
  return segments_.end();
}

size_t HadesGC::OldGen::numSegments() const {
  return segments_.size();
}

HadesGC::HeapSegment &HadesGC::OldGen::operator[](size_t i) {
  return segments_[i];
}

llvh::ErrorOr<HadesGC::HeapSegment> HadesGC::createSegment() {
  // No heap size limit when Handle-SAN is on, to allow the heap enough room to
  // keep moving things around.
  if (!sanitizeRate_ && heapFootprint() >= maxHeapSize_)
    return make_error_code(OOMError::MaxHeapReached);

  auto res = AlignedStorage::create(provider_.get(), "hades-segment");
  if (!res) {
    return res.getError();
  }
  HeapSegment seg(std::move(res.get()));
  // Even if compressed pointers are off, we still use the segment index for
  // crash manager indices.
  size_t segIdx;
  if (segmentIndices_.size()) {
    segIdx = segmentIndices_.back();
    segmentIndices_.pop_back();
  } else {
    segIdx = ++numSegments_;
  }
  pointerBase_.setSegment(segIdx, seg.lowLim());
  addSegmentExtentToCrashManager(seg, std::to_string(segIdx));
  seg.markBitArray().markAll();
  return llvh::ErrorOr<HadesGC::HeapSegment>(std::move(seg));
}

void HadesGC::OldGen::addSegment(HeapSegment seg) {
  segments_.emplace_back(std::move(seg));
  HeapSegment &newSeg = segments_.back();
  incrementAllocatedBytes(newSeg.used());
  // Add a set of freelist buckets for this segment.
  segmentBuckets_.emplace_back();

  assert(
      segmentBuckets_.size() == segments_.size() &&
      "Must have as many freelists as segments.");

  // Add the remainder of the segment to the freelist.
  uint32_t sz = newSeg.available();
  if (sz >= minAllocationSize()) {
    auto res = newSeg.bumpAlloc(sz);
    assert(res.success);
    auto bucket = getFreelistBucket(sz);
    addCellToFreelist(res.ptr, sz, &segmentBuckets_.back()[bucket]);
  }

  gc_.addSegmentExtentToCrashManager(newSeg, std::to_string(numSegments()));
}

HadesGC::HeapSegment HadesGC::OldGen::popSegment() {
  const auto &segBuckets = segmentBuckets_.back();
  for (size_t bucket = 0; bucket < kNumFreelistBuckets; ++bucket) {
    if (segBuckets[bucket].head) {
      segBuckets[bucket].removeFromFreelist();
      freelistBucketBitArray_.set(bucket, buckets_[bucket].next);
    }
  }
  segmentBuckets_.pop_back();

  auto oldSeg = std::move(segments_.back());
  segments_.pop_back();
  return oldSeg;
}

void HadesGC::OldGen::setTargetSizeBytes(size_t targetSizeBytes) {
  assert(gc_.gcMutex_ && "Must hold gcMutex_ when accessing targetSizeBytes_.");
  assert(!targetSizeBytes_ && "Should only initialise targetSizeBytes_ once.");
  targetSizeBytes_ = ExponentialMovingAverage(0.5, targetSizeBytes);
}

bool HadesGC::inOldGen(const void *p) const {
  // If it isn't in any OG segment or the compactee, then this pointer is not in
  // the OG.
  return compactee_.contains(p) ||
      std::any_of(oldGen_.begin(), oldGen_.end(), [p](const HeapSegment &seg) {
           return seg.contains(p);
         });
}

void HadesGC::yieldToOldGen() {
  assert(inGC() && "Must be in GC when yielding to old gen");
  if (!kConcurrentGC && concurrentPhase_ != Phase::None) {
    // If there is an ongoing collection, update the drain rate before
    // collecting.
    if (concurrentPhase_ == Phase::Mark)
      oldGenMarker_->setDrainRate(getDrainRate());

    constexpr uint32_t kYGIncrementalCollectBudget = kTargetMaxPauseMs / 2;
    const auto initialPhase = concurrentPhase_;
    // If the phase hasn't changed and we are still under 25ms after the first
    // iteration, then we can be reasonably sure that the next iteration will
    // also be <25ms, keeping us within 50ms even in the worst case.
    do {
      incrementalCollect(false);
    } while (concurrentPhase_ == initialPhase &&
             ygCollectionStats_->getElapsedTime().count() <
                 kYGIncrementalCollectBudget);

  } else if (concurrentPhase_ == Phase::CompleteMarking) {
    incrementalCollect(false);
    collectOGInBackground();
  }
}

size_t HadesGC::getDrainRate() {
  // Concurrent collections don't need to use the drain rate because they
  // only yield the lock periodically to be interrupted, but otherwise will
  // continuously churn through work regardless of the rate.
  // Non-concurrent collections, on the other hand, can only make progress
  // at YG collection intervals, so they need to be configured to mark the
  // OG faster than it fills up.
  assert(!kConcurrentGC);

  // We want to make progress so that we are able to complete marking over all
  // YG collections before OG fills up.
  uint64_t totalAllocated = oldGen_.allocatedBytes() + oldGen_.externalBytes();
  // Must be >0 to avoid division by zero below.
  uint64_t bytesToFill =
      std::max(oldGen_.targetSizeBytes(), totalAllocated + 1) - totalAllocated;
  uint64_t preAllocated = ogCollectionStats_->beforeAllocatedBytes();
  uint64_t markedBytes = oldGenMarker_->markedBytes();
  assert(
      markedBytes <= preAllocated &&
      "Cannot mark more bytes than were initially allocated");
  uint64_t bytesToMark = preAllocated - markedBytes;
  // The drain rate is calculated from:
  //   bytesToMark / (collections until full)
  // = bytesToMark / (bytesToFill / ygAverageSurvivalBytes_)
  uint64_t drainRate = bytesToMark * ygAverageSurvivalBytes_ / bytesToFill;
  // If any of the above calculations end up being a tiny drain rate, make
  // the lower limit at least 8 KB, to ensure collections eventually end.
  constexpr uint64_t byteDrainRateMin = 8192;
  return std::max(drainRate, byteDrainRateMin);
}

void HadesGC::addSegmentExtentToCrashManager(
    const HeapSegment &seg,
    const std::string &extraName) {
  assert(!extraName.empty() && "extraName can't be empty");
  if (!crashMgr_) {
    return;
  }
  const std::string segmentName = name_ + ":HeapSegment:" + extraName;
  // Pointers need at most 18 characters for 0x + 16 digits for a 64-bit
  // pointer.
  constexpr unsigned kAddressMaxSize = 18;
  char segmentAddressBuffer[kAddressMaxSize];
  snprintf(segmentAddressBuffer, kAddressMaxSize, "%p", seg.lowLim());
  crashMgr_->setContextualCustomData(segmentName.c_str(), segmentAddressBuffer);

#ifdef HERMESVM_PLATFORM_LOGGING
  hermesLog(
      "HermesGC",
      "Added segment extent: %s = %s",
      segmentName.c_str(),
      segmentAddressBuffer);
#endif
}

void HadesGC::removeSegmentExtentFromCrashManager(
    const std::string &extraName) {
  assert(!extraName.empty() && "extraName can't be empty");
  if (!crashMgr_) {
    return;
  }
  const std::string segmentName = name_ + ":HeapSegment:" + extraName;
  crashMgr_->removeContextualCustomData(segmentName.c_str());
}

#ifdef HERMES_SLOW_DEBUG

void HadesGC::checkWellFormed() {
  WeakRefLock lk{weakRefMutex()};
  CheckHeapWellFormedAcceptor acceptor(*this);
  {
    DroppingAcceptor<CheckHeapWellFormedAcceptor> nameAcceptor{acceptor};
    markRoots(nameAcceptor, true);
  }
  markWeakRoots(acceptor, /*markLongLived*/ true);
  forAllObjs([this, &acceptor](GCCell *cell) {
    assert(cell->isValid() && "Invalid cell encountered in heap");
    markCell(cell, acceptor);
  });
}

void HadesGC::verifyCardTable() {
  assert(inGC() && "Must be in GC to call verifyCardTable");
  struct VerifyCardDirtyAcceptor final : public SlotAcceptor {
    HadesGC &gc;

    explicit VerifyCardDirtyAcceptor(HadesGC &gc) : gc(gc) {}

    void acceptHelper(void *valuePtr, void *locPtr) {
      const bool crossRegionCompacteePtr =
          !gc.compactee_.evacContains(locPtr) &&
          gc.compactee_.evacContains(valuePtr);
      if (!gc.inYoungGen(locPtr) &&
          (gc.inYoungGen(valuePtr) || crossRegionCompacteePtr)) {
        assert(HeapSegment::cardTableCovering(locPtr)->isCardForAddressDirty(
            locPtr));
      }
    }

    void accept(GCPointerBase &ptr) override {
      acceptHelper(ptr.get(gc.getPointerBase()), &ptr);
    }

    void accept(GCHermesValue &hv) override {
      if (hv.isPointer())
        acceptHelper(hv.getPointer(), &hv);
    }
    void accept(GCSmallHermesValue &hv) override {
      if (hv.isPointer())
        acceptHelper(hv.getPointer(gc.getPointerBase()), &hv);
    }

    void accept(const GCSymbolID &hv) override {}
  };

  VerifyCardDirtyAcceptor acceptor{*this};
  forAllObjs([this, &acceptor](GCCell *cell) { markCell(cell, acceptor); });

  for (const HeapSegment &seg : oldGen_) {
    seg.cardTable().verifyBoundaries(seg.start(), seg.level());
  }
}

#endif

} // namespace vm
} // namespace hermes
