/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/HadesGC.h"
#include "hermes/Support/Compiler.h"
#include "hermes/VM/AllocResult.h"
#include "hermes/VM/CheckHeapWellFormedAcceptor.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/SlotAcceptorDefault.h"

#include <array>
#include <functional>
#include <stack>

namespace hermes {
namespace vm {

static const char *kGCName =
    kConcurrentGC ? "hades (concurrent)" : "hades (incremental)";

// A free list cell is always variable-sized.
const VTable HadesGC::OldGen::FreelistCell::vt{CellKind::FreelistKind,
                                               /*variableSize*/ 0};

void FreelistBuildMeta(const GCCell *, Metadata::Builder &) {}

HadesGC::HeapSegment::HeapSegment(AlignedStorage &&storage)
    : AlignedHeapSegment{std::move(storage)} {
  // Make sure end() is at the maxSize.
  growToLimit();
}

GCCell *
HadesGC::OldGen::finishAlloc(GCCell *cell, uint32_t sz, uint16_t segmentIdx) {
  // Track the number of allocated bytes in a segment.
  incrementAllocatedBytes(sz, segmentIdx);
  // Write a mark bit so this entry doesn't get free'd by the sweeper.
  HeapSegment::setCellMarkBit(cell);
  // Could overwrite the VTable, but the allocator will write a new one in
  // anyway.
  return cell;
}

AllocResult HadesGC::HeapSegment::bumpAlloc(uint32_t sz) {
  return AlignedHeapSegment::alloc(sz);
}

void HadesGC::OldGen::addCellToFreelist(
    void *addr,
    uint32_t sz,
    size_t segmentIdx) {
  assert(
      sz >= sizeof(FreelistCell) &&
      "Cannot construct a FreelistCell into an allocation in the OG");
  FreelistCell *newFreeCell = new (addr) FreelistCell{sz};
  HeapSegment::setCellHead(static_cast<GCCell *>(addr), sz);
  addCellToFreelist(newFreeCell, segmentIdx);
}

void HadesGC::OldGen::addCellToFreelist(FreelistCell *cell, size_t segmentIdx) {
  const size_t sz = cell->getAllocatedSize();
  // Get the size bucket for the cell being added;
  const uint32_t bucket = getFreelistBucket(sz);
  // Push onto the size-specific free list for this bucket and segment.
  cell->next_ = freelistSegmentsBuckets_[segmentIdx][bucket];
  freelistSegmentsBuckets_[segmentIdx][bucket] = cell;

  // Set a bit indicating that there are now available blocks in this segment
  // for the given bucket.
  freelistBucketSegmentBitArray_[bucket].set(segmentIdx);
  // Set a bit indicating that there are now available blocks for this bucket.
  freelistBucketBitArray_.set(bucket, true);

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
    bool setHead) {
  assert(
      gc_->concurrentPhase_ == Phase::Sweep &&
      "addCellToFreelistFromSweep should only be called during sweeping.");
  size_t newCellSize = freeRangeEnd - freeRangeStart;
  // While coalescing, sweeping may generate new cells, so make sure the cell
  // head is updated.
  if (setHead)
    HeapSegment::setCellHead(
        reinterpret_cast<GCCell *>(freeRangeStart), newCellSize);
  FreelistCell *newCell = new (freeRangeStart) FreelistCell(newCellSize);
  // Get the size bucket for the cell being added;
  const uint32_t bucket = getFreelistBucket(newCellSize);
  // Push onto the size-specific free list for this bucket and segment.
  newCell->next_ = freelistSegmentsBuckets_[sweepIterator_.segNumber][bucket];
  freelistSegmentsBuckets_[sweepIterator_.segNumber][bucket] = newCell;
  __asan_poison_memory_region(newCell + 1, newCellSize - sizeof(FreelistCell));
}

HadesGC::OldGen::FreelistCell *HadesGC::OldGen::removeCellFromFreelist(
    size_t bucket,
    size_t segmentIdx) {
  return removeCellFromFreelist(
      &freelistSegmentsBuckets_[segmentIdx][bucket], bucket, segmentIdx);
}

HadesGC::OldGen::FreelistCell *HadesGC::OldGen::removeCellFromFreelist(
    FreelistCell **prevLoc,
    size_t bucket,
    size_t segmentIdx) {
  FreelistCell *cell = *prevLoc;
  assert(cell && "Cannot get a null cell from freelist");

  // Update whatever was pointing to the cell we are removing.
  *prevLoc = cell->next_;
  // Update the bit arrays if the given freelist is now empty.
  if (!freelistSegmentsBuckets_[segmentIdx][bucket]) {
    // Set the bit for this segment and bucket to 0.
    freelistBucketSegmentBitArray_[bucket].reset(segmentIdx);
    // If setting the bit to 0 above made this bucket empty for all segments,
    // set the bucket bit to 0 as well.
    freelistBucketBitArray_.set(
        bucket, !freelistBucketSegmentBitArray_[bucket].empty());
  }

  // Unpoison the memory so that the mutator can use it.
  __asan_unpoison_memory_region(
      cell + 1, cell->getAllocatedSize() - sizeof(FreelistCell));
  return cell;
}

void HadesGC::OldGen::clearFreelistForSegment(size_t segmentIdx) {
  for (size_t bucket = 0; bucket < kNumFreelistBuckets; ++bucket) {
    freelistBucketSegmentBitArray_[bucket].reset(segmentIdx);
    freelistBucketBitArray_.set(
        bucket, !freelistBucketSegmentBitArray_[bucket].empty());
    freelistSegmentsBuckets_[segmentIdx][bucket] = nullptr;
  }
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
void HadesGC::HeapSegment::forCompactedObjs(CallbackFunction callback) {
  void *const stop = level();
  GCCell *cell = reinterpret_cast<GCCell *>(start());
  while (cell < stop) {
    if (cell->hasMarkedForwardingPointer()) {
      // This cell has been evacuated, do nothing.
      cell = reinterpret_cast<GCCell *>(
          reinterpret_cast<char *>(cell) +
          cell->getMarkedForwardingPointer()->getAllocatedSize());
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

  CollectionStats(HadesGC *gc, std::string cause, std::string collectionType)
      : gc_{gc},
        cause_{std::move(cause)},
        collectionType_{std::move(collectionType)} {}
  ~CollectionStats();

  void addCollectionType(const std::string &collectionType) {
    collectionType_ += " " + collectionType;
  }

  /// Record the allocated bytes in the heap and its size before a collection
  /// begins.
  void setBeforeSizes(uint64_t allocated, uint64_t external, uint64_t sz) {
    allocatedBefore_ = allocated;
    externalBefore_ = external;
    size_ = sz;
  }

  /// Record how many bytes were swept during the collection.
  void setSweptBytes(uint64_t sweptBytes) {
    sweptBytes_ = sweptBytes;
  }

  void setSweptExternalBytes(uint64_t externalBytes) {
    sweptExternalBytes_ = externalBytes;
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

  /// Record this amount of CPU time was taken.
  /// Call this in each thread that does work to correctly count CPU time.
  void incrementCPUTime(Duration cpuTime) {
    cpuDuration_ += cpuTime;
  }

  /// Since Hades allows allocations during an old gen collection, use the
  /// initially allocated bytes and the swept bytes to determine the actual
  /// impact of the GC.
  uint64_t afterAllocatedBytes() const {
    return allocatedBefore_ - sweptBytes_;
  }

  uint64_t afterExternalBytes() const {
    return externalBefore_ - sweptExternalBytes_;
  }

  double survivalRatio() const {
    return allocatedBefore_
        ? static_cast<double>(afterAllocatedBytes()) / allocatedBefore_
        : 0;
  }

 private:
  HadesGC *gc_;
  std::string cause_;
  std::string collectionType_;
  TimePoint beginTime_{};
  TimePoint endTime_{};
  Duration cpuDuration_{};
  uint64_t allocatedBefore_{0};
  uint64_t externalBefore_{0};
  uint64_t size_{0};
  uint64_t sweptBytes_{0};
  uint64_t sweptExternalBytes_{0};
};

HadesGC::CollectionStats::~CollectionStats() {
  gc_->recordGCStats(GCAnalyticsEvent{
      gc_->getName(),
      kGCName,
      collectionType_,
      std::move(cause_),
      std::chrono::duration_cast<std::chrono::milliseconds>(
          endTime_ - beginTime_),
      std::chrono::duration_cast<std::chrono::milliseconds>(cpuDuration_),
      /*preAllocated*/ allocatedBefore_,
      /*preSize*/ size_,
      /*postAllocated*/ afterAllocatedBytes(),
      // Hades does not currently return segments to the system so the size
      // does not change due to a collection.
      /*postSize*/ size_,
      /*survivalRatio*/ survivalRatio()});
}

class HadesGC::EvacAcceptor final : public SlotAcceptorDefault {
 public:
  struct CopyListCell final : public GCCell {
    // Linked list of cells pointing to the next cell that was copied.
    CopyListCell *next_;
  };

  EvacAcceptor(GC &gc)
      : SlotAcceptorDefault{gc},
        copyListHead_{nullptr},
        isTrackingIDs_{gc.isTrackingIDs()} {}

  ~EvacAcceptor() {}

  using SlotAcceptorDefault::accept;

  void accept(void *&ptr) override {
    if (!ptr || !gc.inYoungGen(ptr)) {
      // Ignore null and OG pointers.
      return;
    }
    GCCell *&cell = reinterpret_cast<GCCell *&>(ptr);
    assert(
        HeapSegment::getCellMarkBit(cell) &&
        "All young gen cells should be marked");
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
    GCCell *const newCell = gc.oldGen_.alloc(sz);
    HERMES_SLOW_ASSERT(
        gc.inOldGen(newCell) && "Evacuated cell not in the old gen");
    assert(
        HeapSegment::getCellMarkBit(newCell) &&
        "Cell must be marked when it is allocated into the old gen");
    // Copy the contents of the existing cell over before modifying it.
    std::memcpy(newCell, cell, sz);
    assert(newCell->isValid() && "Cell was copied incorrectly");
    promotedBytes_ += sz;
    CopyListCell *const copyCell = static_cast<CopyListCell *>(cell);
    // Set the forwarding pointer in the old spot
    copyCell->setMarkedForwardingPointer(newCell);
    if (isTrackingIDs_) {
      gc.getIDTracker().moveObject(cell, newCell);
      gc.getAllocationLocationTracker().moveAlloc(cell, newCell);
    }
    // Push onto the copied list.
    push(copyCell);
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

  uint64_t promotedBytes() const {
    return promotedBytes_;
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
  const bool isTrackingIDs_;
  uint64_t promotedBytes_{0};

  void push(CopyListCell *cell) {
    cell->next_ = copyListHead_;
    copyListHead_ = cell;
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

class HadesGC::MarkAcceptor final : public SlotAcceptorDefault,
                                    public WeakRefAcceptor {
 public:
  MarkAcceptor(GC &gc)
      : SlotAcceptorDefault{gc},
        markedSymbols_{gc.gcCallbacks_->getSymbolsEnd()},
        writeBarrierMarkedSymbols_{gc.gcCallbacks_->getSymbolsEnd()},
        bytesToMark_{gc.oldGen_.allocatedBytes()} {}

  using SlotAcceptorDefault::accept;

  void accept(void *&ptr) override {
    GCCell *const cell = static_cast<GCCell *>(ptr);
    if (!cell) {
      return;
    }
    assert(cell->isValid() && "Encountered an invalid cell");
    if (HeapSegment::getCellMarkBit(cell)) {
      // Points to an already marked object, do nothing.
      return;
    }
    push(cell);
  }

  void accept(BasedPointer &ptrRef) override {
    // Copy into local variable in case it changes during evaluation.
    const BasedPointer ptr = ptrRef;
    if (!ptr) {
      return;
    }
    PointerBase *const base = gc.getPointerBase();
    void *actualizedPointer = base->basedToPointerNonNull(ptr);
#ifndef NDEBUG
    void *const ptrCopy = actualizedPointer;
#endif
    accept(actualizedPointer);
    assert(
        ptrCopy == actualizedPointer &&
        "MarkAcceptor::accept should not modify its argument");
  }

  void accept(HermesValue &hvRef) override {
    const HermesValue hv = hvRef;
    if (hv.isPointer()) {
      void *ptr = hv.getPointer();
#ifndef NDEBUG
      void *const ptrCopy = ptr;
#endif
      accept(ptr);
      // ptr should never be modified by this acceptor, so there's no write-back
      // to do.
      assert(
          ptrCopy == ptr &&
          "ptr shouldn't be modified by accept in MarkAcceptor");
    } else if (hv.isSymbol()) {
      accept(hv.getSymbol());
    }
  }

  void accept(SymbolID sym) override {
    const uint32_t idx = sym.unsafeGetIndex();
    if (sym.isInvalid() || idx >= markedSymbols_.size()) {
      // Ignore symbols that aren't valid or are pointing outside of the range
      // when the collection began.
      return;
    }
    markedSymbols_[idx] = true;
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

  /// \return an upper bound on the number of bytes left to mark.
  uint64_t bytesToMark() const {
    return bytesToMark_;
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

    std::lock_guard<Mutex> wrLk{gc.weakRefMutex()};
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
      // There is a benign data race here, as the GC can read a pointer while
      // it's being modified by the mutator; however, the following rules we
      // obey prevent it from being a problem:
      // * The only things being modified that the GC reads are the GCPointers
      //    and GCHermesValue in an object. All other fields are ignored.
      // * Those fields are fewer than 64 bits.
      // * Therefore, on 64-bit platforms, those fields are atomic
      //    automatically.
      // * On 32-bit platforms, we don't run this code concurrently, and
      //    instead yield cooperatively with the mutator. WARN: This isn't
      //    true yet, will be true later.
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
      GCBase::markCell(cell, &gc, *this);
      TsanIgnoreReadsEnd();
    }
    assert(
        bytesToMark_ >= numMarkedBytes &&
        "Cannot have marked more bytes than were originally in the OG.");
    bytesToMark_ -= numMarkedBytes;
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

  /// This approximates the number of bytes that have not yet been marked by
  /// drainSomeWork. It should be initialised at the start of a collection to
  /// the number of allocated bytes at that point. That value serves as an upper
  /// bound on the number of bytes that can be marked, since any newly added
  /// objects after that will already be marked.
  uint64_t bytesToMark_;

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
    if (cell->getKind() == CellKind::WeakMapKind) {
      reachableWeakMaps_.push_back(vmcast<JSWeakMap>(cell));
    } else {
      localWorklist_.push(cell);
    }
  }
};

/// Mark weak roots separately from the MarkAcceptor since this is done while
/// the world is stopped.
/// Don't use the default weak root acceptor because fine-grained control of
/// writes of compressed pointers is important.
class HadesGC::MarkWeakRootsAcceptor final : public WeakRootAcceptor {
 public:
  MarkWeakRootsAcceptor(GC &gc) : gc_{gc}, pointerBase_{gc.getPointerBase()} {
    // Only used in debug builds.
    (void)gc_;
  }

  void acceptWeak(WeakRootBase &wr) override {
    if (!wr) {
      return;
    }
    GCPointerBase::StorageType &ptrStorage = wr.getNoBarrierUnsafe();
#ifdef HERMESVM_COMPRESSED_POINTERS
    GCCell *const cell =
        static_cast<GCCell *>(pointerBase_->basedToPointerNonNull(ptrStorage));
#else
    GCCell *const cell = static_cast<GCCell *>(ptrStorage);
#endif
    assert(!gc_.inYoungGen(cell) && "Pointer should be into the OG");
    HERMES_SLOW_ASSERT(gc_.dbgContains(cell) && "ptr not in heap");
    if (HeapSegment::getCellMarkBit(cell)) {
      // If the cell is marked, no need to do any writes.
      return;
    }
    // Reset weak root if target GCCell is dead.
    ptrStorage = nullptr;
  }

  void accept(WeakRefBase &wr) override {
    // Duplicated from MarkAcceptor, since some weak roots are also weak refs.
    WeakRefSlot *slot = wr.unsafeGetSlot();
    assert(
        slot->state() != WeakSlotState::Free &&
        "marking a freed weak ref slot");
    if (slot->state() != WeakSlotState::Marked) {
      slot->mark();
    }
  }

 private:
  GC &gc_;
  PointerBase *const pointerBase_;
};

bool HadesGC::OldGen::sweepNext() {
  // Check if there are any more segments to sweep. Note that in the case where
  // OG has zero segments, this also skips updating the stats and survival ratio
  // at the end of this function, since they are not required.
  if (!sweepIterator_.segNumber)
    return false;
  assert(gc_->gcMutex_ && "gcMutex_ must be held while sweeping.");

  sweepIterator_.segNumber--;
  const bool isTracking = gc_->isTrackingIDs();
  // Re-evaluate this start point each time, as releasing the gcMutex_ allows
  // allocations into the old gen, which might boost the credited memory.
  const uint64_t externalBytesBefore = externalBytes();

  // Clear the head pointers so that we can construct a new freelist. The
  // freelist bits will be updated after the segment is swept. The bits will be
  // inconsistent with the actual freelist for the duration of sweeping, but
  // this is fine because gcMutex_ is during the entire period.
  for (auto &head : freelistSegmentsBuckets_[sweepIterator_.segNumber])
    head = nullptr;

  char *freeRangeStart = nullptr, *freeRangeEnd = nullptr;
  size_t mergedCells = 0;
  int32_t segmentSweptBytes = 0;
  for (GCCell *cell : segments_[sweepIterator_.segNumber]->cells()) {
    assert(cell->isValid() && "Invalid cell in sweeping");
    if (HeapSegment::getCellMarkBit(cell)) {
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
            freeRangeStart, freeRangeEnd, mergedCells > 1);

      mergedCells = 0;
      freeRangeEnd = freeRangeStart = cellCharPtr;
    }
    // Expand the current free range to include the current cell.
    freeRangeEnd += sz;
    mergedCells++;

    if (cell->getKind() == CellKind::FreelistKind)
      continue;

    segmentSweptBytes += sz;
    // Cell is dead, run its finalizer first if it has one.
    cell->getVT()->finalizeIfExists(cell, gc_);
    if (isTracking) {
      // FIXME: There could be a race condition here if newAlloc is
      // being called at the same time and using a shared data structure
      // with freeAlloc. freeAlloc relies on the ID, so call it before
      // untrackObject.
      gc_->getAllocationLocationTracker().freeAlloc(cell, sz);
      gc_->getIDTracker().untrackObject(cell);
    }
  }

  // Flush any free range that was left over.
  if (freeRangeStart)
    addCellToFreelistFromSweep(freeRangeStart, freeRangeEnd, mergedCells > 1);

  // Update the freelist bit arrays to match the newly set freelist heads.
  for (size_t bucket = 0; bucket < kNumFreelistBuckets; ++bucket) {
    // For each bucket, set the bit for the current segment based on whether
    // it has a non-null freelist head for that bucket.
    if (freelistSegmentsBuckets_[sweepIterator_.segNumber][bucket])
      freelistBucketSegmentBitArray_[bucket].set(sweepIterator_.segNumber);
    else
      freelistBucketSegmentBitArray_[bucket].reset(sweepIterator_.segNumber);

    // In case the change above has changed the availability of a bucket across
    // all segments, update the overall bit array.
    freelistBucketBitArray_.set(
        bucket, !freelistBucketSegmentBitArray_[bucket].empty());
  }

  // Correct the allocated byte count.
  incrementAllocatedBytes(-segmentSweptBytes, sweepIterator_.segNumber);
  sweepIterator_.sweptBytes += segmentSweptBytes;
  sweepIterator_.sweptExternalBytes += externalBytesBefore - externalBytes();

  // There are more iterations to go.
  if (sweepIterator_.segNumber)
    return true;

  // This was the last sweep iteration, finish the collection.
  auto &stats = *gc_->ogCollectionStats_;
  stats.setSweptBytes(sweepIterator_.sweptBytes);
  stats.setSweptExternalBytes(sweepIterator_.sweptExternalBytes);
  // The formula for occupancyTarget_ is:
  // occupancyTarget_ = (allocatedBytes + externalBytes) / (capacityBytes +
  //  externalBytes)
  // Solving for capacityBytes:
  // capacityBytes = (allocatedBytes + externalBytes) / occupancyTarget_ -
  //  externalBytes
  const size_t targetCapacity =
      (stats.afterAllocatedBytes() + stats.afterExternalBytes()) /
          gc_->occupancyTarget_ -
      stats.afterExternalBytes();
  const size_t targetSegments =
      llvh::divideCeil(targetCapacity, HeapSegment::maxSize());
  const size_t finalSegments = std::min(targetSegments, maxNumSegments());
  reserveSegments(finalSegments);
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

// Assume about 30% of the YG will survive initially.
constexpr double kYGInitialSurvivalRatio = 0.3;

HadesGC::OldGen::OldGen(HadesGC *gc) : gc_(gc) {}

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
              llvh::alignTo<AlignedStorage::size()>(gcConfig.getMaxHeapSize())),
          // At least one YG segment and one OG segment.
          2 * AlignedStorage::size())},
      provider_(std::move(provider)),
      oldGen_{this},
      promoteYGToOG_{!gcConfig.getAllocInYoung()},
      revertToYGAtTTI_{gcConfig.getRevertToYGAtTTI()},
      occupancyTarget_(gcConfig.getOccupancyTarget()),
      ygAverageSurvivalRatio_{/*weight*/ 0.5,
                              /*init*/ kYGInitialSurvivalRatio} {
  crashMgr_->setCustomData("HermesGC", kGCName);
  // createSegment relies on member variables and should not be called until
  // they are initialised.
  if (!(youngGen_ = createSegment(/*isYoungGen*/ true))) {
    hermes_fatal("Failed to initialize the young gen");
  }
  ygStart_ = youngGen_->lowLim();
  const size_t minHeapSegments =
      // Align up first to round up.
      llvh::alignTo<AlignedStorage::size()>(gcConfig.getMinHeapSize()) /
      AlignedStorage::size();
  const size_t requestedInitHeapSegments =
      // Align up first to round up.
      llvh::alignTo<AlignedStorage::size()>(gcConfig.getInitHeapSize()) /
      AlignedStorage::size();

  const size_t initHeapSegments =
      std::max({minHeapSegments,
                requestedInitHeapSegments,
                // At least one YG segment and one OG segment.
                static_cast<size_t>(2)});
  oldGen_.reserveSegments(initHeapSegments);
}

HadesGC::~HadesGC() {
  if (oldGenCollectionThread_.joinable()) {
    oldGenCollectionThread_.join();
  }
}

uint32_t HadesGC::minAllocationSize() {
  return heapAlignSize(std::max(
      sizeof(OldGen::FreelistCell), sizeof(EvacAcceptor::CopyListCell)));
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
}

void HadesGC::getHeapInfoWithMallocSize(HeapInfo &info) {
  // Get the usual heap info.
  getHeapInfo(info);
  std::lock_guard<Mutex> lk{gcMutex_};
  // In case the info is being re-used, ensure the count starts at 0.
  info.mallocSizeEstimate = 0;
  // First add the usage by the runtime's roots.
  info.mallocSizeEstimate += gcCallbacks_->mallocSize();
  // Scan all objects for their malloc size. This operation is what makes
  // getHeapInfoWithMallocSize O(heap size).
  forAllObjs([&info](GCCell *cell) {
    info.mallocSizeEstimate += cell->getVT()->getMallocSize(cell);
  });
  // A deque doesn't have a capacity, so the size is the lower bound.
  info.mallocSizeEstimate +=
      weakPointers_.size() * sizeof(decltype(weakPointers_)::value_type);
}

void HadesGC::getCrashManagerHeapInfo(
    CrashManager::HeapInformation &crashInfo) {
  HeapInfo info;
  getHeapInfo(info);
  crashInfo.size_ = info.heapSize;
  crashInfo.used_ = info.allocatedBytes;
}

void HadesGC::createSnapshot(llvh::raw_ostream &os) {
  std::lock_guard<Mutex> lk{gcMutex_};
  // No allocations are allowed throughout the entire heap snapshot process.
  NoAllocScope scope{this};
  // Let any existing collections complete before taking the snapshot.
  waitForCollectionToFinish();
  {
    GCCycle cycle{this, gcCallbacks_, "Heap Snapshot"};
    WeakRefLock lk{weakRefMutex()};
    GCBase::createSnapshot(this, os);
  }
}

void HadesGC::printStats(JSONEmitter &json) {
  GCBase::printStats(json);
  json.emitKey("specific");
  json.openDict();
  json.emitKeyValue("collector", kGCName);
  json.emitKey("stats");
  json.openDict();
  json.closeDict();
  json.closeDict();
}

void HadesGC::collect(std::string cause) {
  {
    // Wait for any existing collections to finish before starting a new one.
    std::lock_guard<Mutex> lk{gcMutex_};
    // Disable the YG promotion mode. A forced GC via collect will do a full
    // collection immediately anyway, so there's no need to avoid collecting YG.
    // This is especially important when the forced GC is a memory warning.
    promoteYGToOG_ = false;
    waitForCollectionToFinish();
  }
  // This function should block until a collection finishes.
  // YG needs to be empty in order to do an OG collection.
  youngGenCollection(std::move(cause), /*forceOldGenCollection*/ true);
  // Wait for the collection to complete.
  std::lock_guard<Mutex> lk{gcMutex_};
  waitForCollectionToFinish();
}

void HadesGC::waitForCollectionToFinish() {
  assert(
      gcMutex_ &&
      "gcMutex_ must be held before calling waitForCollectionToFinish");
  if (concurrentPhase_ == Phase::None) {
    return;
  }
  GCCycle cycle{this, gcCallbacks_, "Old Gen (Direct)"};
  if (ygCollectionStats_) {
    // If this wait happened during a YG collection, add a "(waiting)" suffix.
    ygCollectionStats_->addCollectionType("(waiting)");
  }

  if (!kConcurrentGC) {
    while (concurrentPhase_ != Phase::None) {
      incrementalCollect();
    }
    return;
  }
  std::unique_lock<Mutex> lk{gcMutex_, std::adopt_lock};
  if (isOldGenMarking_) {
    // The background thread is currently marking, wait until it is time for the
    // STW pause.
    waitForConditionVariable(stopTheWorldCondVar_, lk, [this] {
      return concurrentPhase_ == Phase::CompleteMarking;
    });
    completeMarking();
    // Notify the waiting OG collection that it can complete.
    stopTheWorldCondVar_.notify_one();
  }
  lk.unlock();
  // Wait for the collection to finish.
  oldGenCollectionThread_.join();
  lk.lock();
  assert(concurrentPhase_ == Phase::None);
  // Check for a tripwire, since we know a collection just completed.
  checkTripwireAndResetStats();

  assert(lk && "Lock must be re-acquired before exiting");
  assert(gcMutex_ && "GC mutex must be re-acquired before exiting");
  // Release association with the mutex to prevent the destructor from unlocking
  // it.
  lk.release();
}

void HadesGC::oldGenCollection(std::string cause) {
  // Full collection:
  //  * Mark all live objects by iterating through a worklist.
  //  * Sweep dead objects onto the free lists.
  // This function must be called while the gcMutex_ is held.
  assert(gcMutex_ && "gcMutex_ must be held when starting an OG collection");
  assert(
      concurrentPhase_ == Phase::None &&
      "Starting a second old gen collection");
  if (oldGenCollectionThread_.joinable()) {
    // This is just making sure the leftover thread is completed before starting
    // a new one. Since concurrentPhase_ == None here, there is no collection
    // ongoing.
    oldGenCollectionThread_.join();
  }
  // We know ygCollectionStats_ exists because oldGenCollection is only called
  // by youngGenCollection.
  ygCollectionStats_->addCollectionType("(old gen start)");
#ifdef HERMES_SLOW_DEBUG
  checkWellFormed();
#endif
  // Setup the sweep iterator when collection begins, because the number of
  // segments can change if a YG collection interleaves. There's no need to
  // sweep those extra segments since they are full of newly promoted objects
  // from YG (which have all their mark bits set), thus the sweep iterator
  // doesn't need to be updated. We also don't need to sweep any segments made
  // since the start of the collection, since they won't have any unmarked
  // objects anyway.
  oldGen_.initializeSweep();

  // Note: This line calls the destructor for the previous CollectionStats (if
  // any) in addition to creating a new CollectionStats. It is desirable to
  // call the destructor here so that the analytics callback is invoked from the
  // mutator thread. This might also be done from checkTripwireAndResetStats.
  ogCollectionStats_ =
      llvh::make_unique<CollectionStats>(this, std::move(cause), "old");
  // NOTE: Leave CPU time as zero if the collection isn't concurrent, as the
  // times aren't useful.
  auto cpuTimeStart = oscompat::thread_cpu_time();
  ogCollectionStats_->setBeginTime();
  ogCollectionStats_->setBeforeSizes(
      oldGen_.allocatedBytes(), oldGen_.externalBytes(), oldGen_.size());

  if (revertToYGAtTTI_) {
    // If we've reached the first OG collection, and reverting behavior is
    // requested, switch back to YG mode.
    promoteYGToOG_ = false;
  }
  // First, clear any mark bits that were set by a previous collection or
  // direct-to-OG allocation, they aren't needed anymore.
  for (auto segit = oldGen_.begin(), segitend = oldGen_.end();
       segit != segitend;
       ++segit) {
    HeapSegment &seg = **segit;
    seg.markBitArray().clear();
  }

  // Unmark all symbols in the identifier table, as Symbol liveness will be
  // determined during the collection.
  gcCallbacks_->unmarkSymbols();

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
  isOldGenMarking_ = true;
  // NOTE: Since the "this" value (the HadesGC instance) is implicitly copied to
  // the new thread, the GC cannot be destructed until the new thread completes.
  // This means that before destroying the GC, waitForCollectionToFinish must
  // be called.
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
  auto cpuTimeEnd = oscompat::thread_cpu_time();
  ogCollectionStats_->incrementCPUTime(cpuTimeEnd - cpuTimeStart);
  // 64-bit system: 64-bit HermesValues can be updated in one atomic
  // instruction. Start up a separate thread for doing marking work.
  // NOTE: Since the "this" value (the HadesGC instance) is implicitly copied
  // to the new thread, the GC cannot be destructed until the new thread
  // completes. This means that before destroying the GC,
  // waitForCollectionToFinish must be called.
  oldGenCollectionThread_ = std::thread(&HadesGC::oldGenCollectionWorker, this);
  // Use concurrentPhase_ to be able to tell when the collection finishes.
}

void HadesGC::oldGenCollectionWorker() {
  oscompat::set_thread_name("hades");
  auto cpuTimeStart = oscompat::thread_cpu_time();
  while (true) {
    std::lock_guard<Mutex> lk(gcMutex_);
    incrementalCollect();
    if (concurrentPhase_ == Phase::None) {
      // NOTE: These must be set before the lock is released. This is because
      // the mutator might otherwise observe the change in concurrentPhase_ and
      // attempt to reset ogCollectionStats_.
      auto cpuTimeEnd = oscompat::thread_cpu_time();
      ogCollectionStats_->incrementCPUTime(cpuTimeEnd - cpuTimeStart);
      break;
    }
  }
}

void HadesGC::incrementalCollect() {
  switch (concurrentPhase_) {
    case Phase::None:
      break;
    case Phase::Mark:
      // Drain some work from the mark worklist. If the work has finished
      // completely, move on to CompleteMarking.
      if (!oldGenMarker_->drainSomeWork())
        concurrentPhase_ = Phase::CompleteMarking;
      break;
    case Phase::CompleteMarking:
      if (!kConcurrentGC)
        completeMarking();
      else
        waitForCompleteMarking();
      assert(
          concurrentPhase_ == Phase::Sweep &&
          "completeMarking should advance concurrentPhase_ to sweep");
      break;
    case Phase::Sweep:
      // Calling oldGen_.sweepNext() will sweep the next segment.
      if (!oldGen_.sweepNext()) {
        // Finish any collection bookkeeping.
        ogCollectionStats_->setEndTime();
        concurrentPhase_ = Phase::None;
        if (!kConcurrentGC) {
          // Check the tripwire here since we know the incremental collection
          // will always end on the mutator thread.
          checkTripwireAndResetStats();
        }
      }
      break;
    default:
      llvm_unreachable("No other possible state between iterations");
  }
}

void HadesGC::waitForCompleteMarking() {
  assert(kConcurrentGC);
  assert(
      calledByBackgroundThread() &&
      "Only background thread can block waiting for STW pause.");
  assert(gcMutex_);
  std::unique_lock<Mutex> lk{gcMutex_, std::adopt_lock};
  // If the mutator is in waitForCollectionToFinish, notify it that the OG
  // thread is waiting for a STW pause.
  stopTheWorldCondVar_.notify_one();
  waitForConditionVariable(stopTheWorldCondVar_, lk, [this] {
    return concurrentPhase_ == Phase::Sweep;
  });
  lk.release();
}

void HadesGC::completeMarking() {
  assert(inGC() && "inGC_ must be set during the STW pause");
  if (ygCollectionStats_) {
    ygCollectionStats_->addCollectionType("(complete marking)");
  }
  // No locks are needed here because the world is stopped and there is only 1
  // active thread.
  oldGenMarker_->globalWorklist().flushPushChunk();
  // Drain the marking queue.
  oldGenMarker_->drainAllWork();
  assert(
      oldGenMarker_->globalWorklist().empty() &&
      "Marking worklist wasn't drained");
  // completeWeakMapMarking examines all WeakRefs stored in various WeakMaps and
  // examines them, regardless of whether the object they use is live or not. We
  // don't want to execute any read barriers during that time which would affect
  // the liveness of the object read out of the weak reference.
  concurrentPhase_ = Phase::WeakMapScan;
  // NOTE: We can access this here since the world is stopped.
  isOldGenMarking_ = false;
  completeWeakMapMarking(*oldGenMarker_);
  assert(
      oldGenMarker_->globalWorklist().empty() &&
      "Marking worklist wasn't drained");
  // Reset weak roots to null after full reachability has been
  // determined.
  MarkWeakRootsAcceptor acceptor{*this};
  markWeakRoots(acceptor);

  // Now free symbols and weak refs.
  gcCallbacks_->freeSymbols(oldGenMarker_->markedSymbols());
  // NOTE: If sweeping is done concurrently with YG collection, weak references
  // could be handled during the sweep pass instead of the mark pass. The read
  // barrier will need to be updated to handle the case where a WeakRef points
  // to an now-empty cell.
  updateWeakReferencesForOldGen();
  // Change the phase to sweep here, before the STW lock is released. This
  // ensures that no mutator read barriers observe the WeakMapScan phase.
  concurrentPhase_ = Phase::Sweep;

  // Nothing needs oldGenMarker_ from this point onward.
  oldGenMarker_.reset();
}

void HadesGC::finalizeAll() {
  std::lock_guard<Mutex> lk{gcMutex_};
  finalizeAllLocked();
}

void HadesGC::finalizeAllLocked() {
  // Wait for any existing OG collections to finish.
  // TODO: Investigate sending a cancellation instead.
  waitForCollectionToFinish();
  // Now finalize the heap.
  // We might be in the middle of a YG collection, with some objects promoted to
  // the OG, and some not. Only finalize objects that have not been promoted to
  // OG, and let the OG finalize the promoted objects.
  finalizeYoungGenObjects();
  for (auto seg = oldGen_.begin(), end = oldGen_.end(); seg != end; ++seg) {
    (*seg)->forAllObjs([this](GCCell *cell) {
      assert(cell->isValid() && "Invalid cell in finalizeAll");
      cell->getVT()->finalizeIfExists(cell, this);
    });
  }
}

void HadesGC::creditExternalMemory(GCCell *cell, uint32_t sz) {
  assert(canAllocExternalMemory(sz) && "Precondition");
  if (inYoungGen(cell)) {
    ygExternalBytes_ += sz;
    // Instead of setting the effective end, which forces YG collections to
    // happen sooner, check if the new total external bytes is large enough to
    // maybe warrant an OG GC.
    const uint64_t totalAllocated = allocatedBytes() + externalBytes();
    // Add one heap segment for YG capacity bytes.
    const uint64_t totalBytes =
        (oldGen_.capacityBytes() + HeapSegment::maxSize()) + externalBytes();
    constexpr double kCollectionThreshold = 0.75;
    double allocatedRatio = static_cast<double>(totalAllocated) / totalBytes;
    if (allocatedRatio >= kCollectionThreshold) {
      // Set the effective end to the level, which will force a GC to occur
      // on the next YG alloc.
      youngGen_->setEffectiveEnd(youngGen_->level());
    }
  } else {
    std::lock_guard<Mutex> lk{gcMutex_};
    oldGen_.creditExternalMemory(sz);
  }
}

void HadesGC::debitExternalMemory(GCCell *cell, uint32_t sz) {
  if (inYoungGen(cell)) {
    assert(
        ygExternalBytes_ >= sz &&
        "Debiting more native memory than was credited");
    ygExternalBytes_ -= sz;
    // Don't modify the effective end here. creditExternalMemory is in charge
    // of tracking this. We don't expect many debits to not be from finalizers
    // anyway.
  } else {
    std::lock_guard<Mutex> lk{gcMutex_};
    oldGen_.debitExternalMemory(sz);
  }
}

void HadesGC::writeBarrier(void *loc, HermesValue value) {
  assert(
      !calledByBackgroundThread() &&
      "Write barrier invoked by background thread.");
  if (inYoungGen(loc)) {
    // A pointer that lives in YG never needs any write barriers.
    return;
  }
  if (isOldGenMarking_) {
    snapshotWriteBarrierInternal(*static_cast<HermesValue *>(loc));
  }
  if (!value.isPointer()) {
    return;
  }
  generationalWriteBarrier(loc, value.getPointer());
}

void HadesGC::writeBarrier(void *loc, void *value) {
  assert(
      !calledByBackgroundThread() &&
      "Write barrier invoked by background thread.");
  if (inYoungGen(loc)) {
    // A pointer that lives in YG never needs any write barriers.
    return;
  }
  if (isOldGenMarking_) {
    const GCPointerBase::StorageType oldValueStorage =
        *static_cast<GCPointerBase::StorageType *>(loc);

#ifdef HERMESVM_COMPRESSED_POINTERS
    // TODO: Pass in pointer base? Slows down the non-concurrent-marking case.
    // Or maybe always decode the old value? Also slows down the normal case.
    GCCell *const oldValue = static_cast<GCCell *>(
        getPointerBase()->basedToPointer(oldValueStorage));
#else
    GCCell *const oldValue = static_cast<GCCell *>(oldValueStorage);
#endif
    snapshotWriteBarrierInternal(oldValue);
  }
  // Always do the non-snapshot write barrier in order for YG to be able to
  // scan cards.
  generationalWriteBarrier(loc, value);
}

void HadesGC::writeBarrier(SymbolID symbol) {
  assert(
      !calledByBackgroundThread() &&
      "Write barrier invoked by background thread.");
  if (isOldGenMarking_) {
    snapshotWriteBarrierInternal(symbol);
  }
  // No need for a generational write barrier for symbols, they always point
  // to long-lived strings.
}

void HadesGC::constructorWriteBarrier(void *loc, HermesValue value) {
  if (inYoungGen(loc)) {
    // A pointer that lives in YG never needs any write barriers.
    return;
  }
  // A constructor never needs to execute a SATB write barrier, since its
  // previous value was definitely not live.
  if (!value.isPointer()) {
    return;
  }
  generationalWriteBarrier(loc, value.getPointer());
}

void HadesGC::constructorWriteBarrier(void *loc, void *value) {
  if (inYoungGen(loc)) {
    // A pointer that lives in YG never needs any write barriers.
    return;
  }
  // A constructor never needs to execute a SATB write barrier, since its
  // previous value was definitely not live.
  generationalWriteBarrier(loc, value);
}

void HadesGC::snapshotWriteBarrier(GCHermesValue *loc) {
  if (inYoungGen(loc)) {
    // A pointer that lives in YG never needs any write barriers.
    return;
  }
  if (isOldGenMarking_) {
    snapshotWriteBarrierInternal(*loc);
  }
}

void HadesGC::snapshotWriteBarrierRange(GCHermesValue *start, uint32_t numHVs) {
  if (inYoungGen(start)) {
    // A pointer that lives in YG never needs any write barriers.
    return;
  }
  if (!isOldGenMarking_) {
    return;
  }
  for (uint32_t i = 0; i < numHVs; ++i) {
    snapshotWriteBarrierInternal(start[i]);
  }
}

void HadesGC::snapshotWriteBarrierInternal(GCCell *oldValue) {
  assert(
      (!oldValue || oldValue->isValid()) &&
      "Invalid cell encountered in snapshotWriteBarrier");
  if (oldValue && !inYoungGen(oldValue)) {
    HERMES_SLOW_ASSERT(
        dbgContains(oldValue) &&
        "Non-heap pointer encountered in snapshotWriteBarrier");
    oldGenMarker_->globalWorklist().enqueue(oldValue);
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

void HadesGC::snapshotWriteBarrierInternal(SymbolID symbol) {
  HERMES_SLOW_ASSERT(
      isOldGenMarking_ &&
      "snapshotWriteBarrier should only be called while the OG is marking");
  oldGenMarker_->markSymbol(symbol);
}

void HadesGC::generationalWriteBarrier(void *loc, void *value) {
  assert(!inYoungGen(loc) && "Pre-condition from other callers");
  if (AlignedStorage::containedInSame(loc, value)) {
    return;
  }
  if (inYoungGen(value)) {
    // Only dirty a card if it's an old-to-young pointer.
    // This is fine to do since the GC never modifies card tables outside of
    // allocation.
    // Note that this *only* applies since the boundaries are updated separately
    // from the card table being marked itself.
    HeapSegment::cardTableCovering(loc)->dirtyCardForAddress(loc);
  }
}

void HadesGC::weakRefReadBarrier(void *value) {
  assert(
      !calledByBackgroundThread() &&
      "Read barrier invoked by background thread.");
  if (isOldGenMarking_) {
    // If the GC is marking, conservatively mark the value as live.
    snapshotWriteBarrierInternal(static_cast<GCCell *>(value));
  }
  // Otherwise, if no GC is active at all, the weak ref must be alive.
  // During sweeping there's no special handling either.
}

void HadesGC::weakRefReadBarrier(HermesValue value) {
  // Any non-pointer value is not going to be cleaned up by a GC anyway.
  if (value.isPointer()) {
    weakRefReadBarrier(value.getPointer());
  }
}

bool HadesGC::canAllocExternalMemory(uint32_t size) {
  return size <= maxHeapSize_;
}

// Only exists to prevent linker errors, do not call.
void HadesGC::markSymbol(SymbolID) {}

WeakRefSlot *HadesGC::allocWeakSlot(HermesValue init) {
  assert(
      !calledByBackgroundThread() &&
      "allocWeakSlot should only be called from the mutator");
  // The weak ref mutex doesn't need to be held since weakPointers_ and
  // firstFreeWeak_ are only modified while the world is stopped.
  WeakRefSlot *slot;
  if (firstFreeWeak_) {
    assert(
        firstFreeWeak_->state() == WeakSlotState::Free &&
        "invalid free slot state");
    slot = firstFreeWeak_;
    firstFreeWeak_ = firstFreeWeak_->nextFree();
    slot->reset(init);
  } else {
    weakPointers_.push_back({init});
    slot = &weakPointers_.back();
  }
  if (isOldGenMarking_) {
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
  // Since the lock is held, it's safe to store this phase.
  const Phase phase = concurrentPhase_;
  youngGen().forAllObjs(callback);
  for (auto segit = oldGen_.begin(), end = oldGen_.end(); segit != end;
       ++segit) {
    HeapSegment &seg = **segit;
    if (phase != Phase::Sweep) {
      seg.forAllObjs(callback);
      continue;
    }
    seg.forAllObjs([callback](GCCell *cell) {
      // If we're doing this check during an OG GC, there might be some objects
      // that are dead, and could potentially have garbage in them. There's no
      // need to check the pointers of those objects.
      if (HeapSegment::getCellMarkBit(cell)) {
        callback(cell);
      }
    });
  }
}

void HadesGC::ttiReached() {
  promoteYGToOG_ = false;
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

template <bool fixedSize, HasFinalizer hasFinalizer>
void *HadesGC::allocWork(uint32_t sz) {
  assert(
      isSizeHeapAligned(sz) &&
      "Should be aligned before entering this function");
  assert(sz >= minAllocationSize() && "Allocating too small of an object");
  if (kConcurrentGC) {
    HERMES_SLOW_ASSERT(
        !weakRefMutex() &&
        "WeakRef mutex should not be held when alloc is called");
  }
  if (shouldSanitizeHandles()) {
    // The best way to sanitize uses of raw pointers outside handles is to force
    // the entire heap to move, and ASAN poison the old heap. That is too
    // expensive to do, even with sampling, for Hades. It also doesn't test the
    // more interesting aspect of Hades which is concurrent background
    // collections. So instead, do a youngGenCollection which force-starts an
    // oldGenCollection if one is not already running.
    youngGenCollection(
        kHandleSanCauseForAnalytics, /*forceOldGenCollection*/ true);
  }
  AllocResult res = youngGen().bumpAlloc(sz);
  if (LLVM_UNLIKELY(!res.success)) {
    // Failed to alloc in young gen, do a young gen collection.
    youngGenCollection(
        kNaturalCauseForAnalytics, /*forceOldGenCollection*/ false);
    res = youngGen().bumpAlloc(sz);
    if (LLVM_UNLIKELY(!res.success)) {
      // A YG collection is guaranteed to fully evacuate, leaving all the space
      // available, so the only way this could fail is if sz is greater than
      // a segment size.
      // This would be an error in VM code to ever allow such a size to be
      // allocated, and thus there's an assert at the top of this function to
      // disallow that. This case is for production, if we miss a test case.
      oom(make_error_code(OOMError::SuperSegmentAlloc));
    }
  }
  if (hasFinalizer == HasFinalizer::Yes) {
    youngGenFinalizables_.emplace_back(static_cast<GCCell *>(res.ptr));
  }
  return res.ptr;
}

// Instaniate all versions of allocWork up-front so that this function doesn't
// need to be inlined.
template void *HadesGC::allocWork<true, HasFinalizer::Yes>(uint32_t);
template void *HadesGC::allocWork<false, HasFinalizer::Yes>(uint32_t);
template void *HadesGC::allocWork<true, HasFinalizer::No>(uint32_t);
template void *HadesGC::allocWork<false, HasFinalizer::No>(uint32_t);

void *HadesGC::allocLongLived(uint32_t sz) {
  if (kConcurrentGC) {
    HERMES_SLOW_ASSERT(
        !weakRefMutex() &&
        "WeakRef mutex should not be held when allocLongLived is called");
  }
  assert(gcMutex_ && "GC mutex must be held when calling allocLongLived");
  totalAllocatedBytes_ += heapAlignSize(sz);
  // Alloc directly into the old gen.
  return oldGen_.alloc(heapAlignSize(sz));
}

GCCell *HadesGC::OldGen::alloc(uint32_t sz) {
  assert(
      isSizeHeapAligned(sz) &&
      "Should be aligned before entering this function");
  assert(sz >= minAllocationSize() && "Allocating too small of an object");
  assert(sz <= maxAllocationSize() && "Allocating too large of an object");
  assert(gc_->gcMutex_ && "gcMutex_ must be held before calling oldGenAlloc");
  if (GCCell *cell = search(sz)) {
    return cell;
  }
  // Before waiting for a collection to finish, check if we're below the max
  // heap size and can simply allocate another segment. This will prevent
  // blocking the YG unnecessarily.
  if (std::unique_ptr<HeapSegment> seg =
          gc_->createSegment(/*isYoungGen*/ false)) {
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
    addSegment(std::move(seg));
    GCCell *newObj = static_cast<GCCell *>(res.ptr);
    HeapSegment::setCellMarkBit(newObj);
    return newObj;
  }
  // Can't expand to any more segments, wait for an old gen collection to finish
  // and possibly free up memory.
  gc_->waitForCollectionToFinish();
  // Repeat the search in case the collection did free memory.
  if (GCCell *cell = search(sz)) {
    return cell;
  }

  // The GC didn't recover enough memory, OOM.
  gc_->oom(make_error_code(OOMError::MaxHeapReached));
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
    if (freelistBucketBitArray_.at(bucket)) {
      int segmentIdx = freelistBucketSegmentBitArray_[bucket].find_first();
      assert(
          segmentIdx >= 0 &&
          "Set bit in freelistBucketBitArray_ must correspond to segment index.");
      FreelistCell *cell = removeCellFromFreelist(bucket, segmentIdx);
      assert(
          cell->getAllocatedSize() == sz &&
          "Size bucket should be an exact match");
      return finishAlloc(cell, sz, segmentIdx);
    }
    // Make sure we start searching at the smallest possible size that could fit
    bucket = getFreelistBucket(sz + minAllocationSize());
  }
  // Once we're examining the rest of the free list, it's a first-fit algorithm.
  // This approach approximates finding the smallest possible fit.
  bucket = freelistBucketBitArray_.findNextSetBitFrom(bucket);
  for (; bucket < kNumFreelistBuckets;
       bucket = freelistBucketBitArray_.findNextSetBitFrom(bucket + 1)) {
    for (size_t segmentIdx : freelistBucketSegmentBitArray_[bucket]) {
      assert(
          freelistSegmentsBuckets_[segmentIdx][bucket] &&
          "Empty bucket should not have bit set!");
      // Need to track the previous entry in order to change the next pointer.
      FreelistCell **prevLoc = &freelistSegmentsBuckets_[segmentIdx][bucket];
      FreelistCell *cell = freelistSegmentsBuckets_[segmentIdx][bucket];

      while (cell) {
        assert(
            cell == *prevLoc && "prevLoc should be updated in each iteration");
        assert(
            vmisa<FreelistCell>(cell) &&
            "Non-free-list cell found in the free list");
        assert(
            (!cell->next_ || cell->next_->isValid()) &&
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
          if (getFreelistBucket(cell->getAllocatedSize()) != bucket) {
            removeCellFromFreelist(prevLoc, bucket, segmentIdx);
            addCellToFreelist(cell, segmentIdx);
          }
          // Because we carved newCell out before removing cell from the
          // freelist, newCell is still poisoned (regardless of whether the
          // conditional above executed). Unpoison it.
          __asan_unpoison_memory_region(newCell, sz);
          return finishAlloc(newCell, sz, segmentIdx);
        } else if (cellSize == sz) {
          // Exact match, take it.
          removeCellFromFreelist(prevLoc, bucket, segmentIdx);
          return finishAlloc(cell, sz, segmentIdx);
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
        cell = cell->next_;
      }
    }
  }
  return nullptr;
}

void HadesGC::youngGenCollection(
    std::string cause,
    bool forceOldGenCollection) {
  ygCollectionStats_ = llvh::make_unique<CollectionStats>(this, cause, "young");
  auto cpuTimeStart = oscompat::thread_cpu_time();
  ygCollectionStats_->setBeginTime();
  ygCollectionStats_->setBeforeSizes(
      youngGen().used(), ygExternalBytes_, youngGen().size());
  // Acquire the GC lock for the duration of the YG collection.
  std::lock_guard<Mutex> lk{gcMutex_};
  // The YG is not parseable while a collection is occurring.
  assert(!inGC() && "Cannot be in GC at the start of YG!");
  GCCycle cycle{this, gcCallbacks_, "Young Gen"};
#ifdef HERMES_SLOW_DEBUG
  checkWellFormed();
  // Check that the card tables are well-formed before the collection.
  verifyCardTable();
#endif
  assert(
      youngGen().markBitArray().findNextUnmarkedBitFrom(0) ==
          youngGen().markBitArray().size() &&
      "Young gen segment must have all mark bits set");
  const uint64_t usedBefore = youngGen().used();
  // YG is about to be emptied, add all of the allocations.
  totalAllocatedBytes_ += usedBefore;
  // Attempt to promote the YG segment to OG if the flag is set. If this call
  // fails for any reason, proceed with a GC.
  if (promoteYoungGenToOldGen()) {
    // Leave sweptBytes and sweptExternalBytes as defaults (which are 0).
    // Don't update the average YG survival ratio since no liveness was
    // calculated for the promotion case.
    ygCollectionStats_->addCollectionType("(promotion)");
  } else {
    auto &yg = youngGen();
    const uint64_t externalBytesBefore = ygExternalBytes_;

    // Marking each object puts it onto an embedded free list.
    EvacAcceptor acceptor{*this};
    {
      DroppingAcceptor<EvacAcceptor> nameAcceptor{acceptor};
      markRoots(nameAcceptor, /*markLongLived*/ false);
      // Do not call markWeakRoots here, as all weak roots point to
      // long-lived objects.
    }
    // Find old-to-young pointers, as they are considered roots for YG
    // collection.
    scanDirtyCards(acceptor);
    // Iterate through the copy list to find new pointers.
    while (EvacAcceptor::CopyListCell *const copyCell = acceptor.pop()) {
      assert(
          copyCell->hasMarkedForwardingPointer() &&
          "Discovered unmarked object");
      assert(inYoungGen(copyCell) && "Discovered OG object in YG collection");
      // Update the pointers inside the forwarded object, since the old
      // object is only there for the forwarding pointer.
      GCCell *const cell = copyCell->getMarkedForwardingPointer();
      GCBase::markCell(cell, this, acceptor);
    }
    {
      WeakRefLock weakRefLock{weakRefMutex_};
      // Now that all YG objects have been marked, update weak references.
      updateWeakReferencesForYoungGen();
    }
    // Inform trackers about objects that died during this YG collection.
    if (isTrackingIDs()) {
      youngGen().forCompactedObjs([this](GCCell *cell) {
        // Have to call freeAlloc before untrackObject.
        getAllocationLocationTracker().freeAlloc(
            cell, cell->getAllocatedSize());
        getIDTracker().untrackObject(cell);
      });
    }
    // Run finalizers for young gen objects.
    finalizeYoungGenObjects();
    // This was modified by debitExternalMemoryFromFinalizer, called by
    // finalizers. The difference in the value before to now was the swept bytes
    const uint64_t externalSweptBytes = externalBytesBefore - ygExternalBytes_;
    // Now the copy list is drained, and all references point to the old
    // gen. Clear the level of the young gen.
    yg.resetLevel();
    assert(
        youngGen().markBitArray().findNextUnmarkedBitFrom(0) ==
            youngGen().markBitArray().size() &&
        "Young gen segment must have all mark bits set");
    // Move external memory accounting from YG to OG as well.
    transferExternalMemoryToOldGen();
    // Post-allocated has an ambiguous meaning for a young-gen GC, since the
    // young gen must be completely evacuated. Since zeros aren't really
    // useful here, instead put the number of bytes that were promoted into
    // old gen, which is the amount that survived the collection.
    ygCollectionStats_->setSweptBytes(usedBefore - acceptor.promotedBytes());
    ygCollectionStats_->setSweptExternalBytes(externalSweptBytes);
    // The average survival ratio should be updated before starting an OG
    // collection.
    ygAverageSurvivalRatio_.update(ygCollectionStats_->survivalRatio());
  }
#ifdef HERMES_SLOW_DEBUG
  // Check that the card tables are well-formed after the collection.
  verifyCardTable();
#endif
  // Give an existing background thread a chance to complete.
  // Do this before starting a new collection in case we need collections
  // back-to-back. Also, don't check this after starting a collection to avoid
  // waiting for something that is both unlikely, and will increase the pause
  // time if it does happen.
  yieldToOldGen();
  if (concurrentPhase_ == Phase::None) {
    // There is no OG collection running, check the tripwire in case this is the
    // first YG after an OG completed.
    checkTripwireAndResetStats();
    if (forceOldGenCollection) {
      oldGenCollection(std::move(cause));
    } else {
      // If the OG is sufficiently full after the collection finishes, begin
      // an OG collection.
      // External bytes are part of the numerator and denominator, because they
      // should not be included as part of determining the heap's occupancy, but
      // instead just influence when collections begin.
      const uint64_t totalAllocated =
          oldGen_.allocatedBytes() + oldGen_.externalBytes();
      const uint64_t totalBytes =
          oldGen_.capacityBytes() + oldGen_.externalBytes();
      constexpr double kCollectionThreshold = 0.75;
      double allocatedRatio = static_cast<double>(totalAllocated) / totalBytes;
      if (allocatedRatio >= kCollectionThreshold) {
        oldGenCollection(kNaturalCauseForAnalytics);
      }
    }
  }
#ifdef HERMES_SLOW_DEBUG
  // Run a well-formed check before exiting.
  checkWellFormed();
#endif
  ygCollectionStats_->setEndTime();
  auto cpuTimeEnd = oscompat::thread_cpu_time();
  ygCollectionStats_->incrementCPUTime(cpuTimeEnd - cpuTimeStart);
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
  auto newYoungGen = createSegment(/*isYoungGen*/ true);
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
  youngGen_->forAllObjs([this](GCCell *cell) {
    youngGen_->setCellHead(cell, cell->getAllocatedSize());
  });
  // It is important that this operation is just a move of pointers to
  // segments. The addresses have to stay the same or else it would
  // require a marking pass through all objects.
  // This will also rename the segment in the crash data.
  oldGen_.addSegment(std::move(youngGen_));
  // Replace YG with a new segment.
  youngGen_ = std::move(newYoungGen);
  ygStart_ = youngGen_->lowLim();
  // These objects will be finalized by an OG collection.
  youngGenFinalizables_.clear();
  return true;
}

void HadesGC::checkTripwireAndResetStats() {
  assert(
      gcMutex_ &&
      "gcMutex must be held before calling checkTripwireAndResetStats");
  if (!ogCollectionStats_) {
    return;
  }
  const auto afterAllocatedBytes = ogCollectionStats_->afterAllocatedBytes();
  // We use the amount of live data from after a GC completed as the minimum
  // bound of what is live.
  checkTripwire(afterAllocatedBytes);
  // Resetting the stats both runs the destructor (submitting the stats), as
  // well as prevent us from checking the tripwire every YG.
  ogCollectionStats_.reset();
}

void HadesGC::transferExternalMemoryToOldGen() {
  oldGen_.creditExternalMemory(ygExternalBytes_);
  ygExternalBytes_ = 0;
  youngGen_->clearExternalMemoryCharge();
}

void HadesGC::scanDirtyCards(EvacAcceptor &acceptor) {
  SlotVisitor<EvacAcceptor> visitor{acceptor};
  // The acceptors in this loop can grow the old gen by adding another
  // segment, if there's not enough room to evac the YG objects discovered.
  // Since segments are always placed at the end, we can use indices instead
  // of iterators, which aren't invalidated. It's ok to not scan newly added
  // segments, since they are going to be handled from the rest of YG
  // collection.
  const auto segEnd = oldGen_.numSegments();
  for (size_t i = 0; i < segEnd; ++i) {
    HeapSegment &seg = oldGen_[i];
    const auto &cardTable = seg.cardTable();
    // Use level instead of end in case the OG segment is still in bump alloc
    // mode.
    const char *const origSegLevel = seg.level();
    size_t from = cardTable.addressToIndex(seg.start());
    const size_t to = cardTable.addressToIndex(origSegLevel - 1) + 1;

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
      GCBase::markCellWithinRange(visitor, obj, obj->getVT(), this, begin, end);

      // Mark the objects that are entirely contained within the dirty card
      // boundaries.
      for (GCCell *next = obj->nextCell(); next < boundary;
           next = next->nextCell()) {
        // Use a separate pointer for the loop condition so that the last
        // object in range gets used with markCellWithinRange instead.
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
  assert(gcMutex_ && "gcMutex must be held when updating weak refs");
  for (auto &slot : weakPointers_) {
    switch (slot.state()) {
      case WeakSlotState::Free:
        break;

      case WeakSlotState::Marked:
        // WeakRefSlots may only be marked while an OG collection is in the mark
        // phase or in the STW pause. The OG collection should unmark any slots
        // after it is complete.
        assert(isOldGenMarking_);
        LLVM_FALLTHROUGH;
      case WeakSlotState::Unmarked: {
        // Both marked and unmarked weak ref slots need to be updated.
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
          // Can't free this slot because it might only be used by an OG
          // object.
          slot.clearPointer();
        }
        break;
      }
    }
  }
}

void HadesGC::updateWeakReferencesForOldGen() {
  for (auto &slot : weakPointers_) {
    switch (slot.state()) {
      case WeakSlotState::Free:
        // Skip free weak slots.
        break;
      case WeakSlotState::Marked: {
        // Set all allocated slots to unmarked.
        slot.unmark();
        if (!slot.hasPointer()) {
          // Skip non-pointers.
          break;
        }
        auto *const cell = static_cast<GCCell *>(slot.getPointer());
        // If the object isn't live, clear the weak ref.
        // YG has all of its mark bits set whenever there's no YG collection
        // happening, so this also excludes clearing any pointers to YG objects.
        if (!HeapSegment::getCellMarkBit(cell)) {
          slot.clearPointer();
        }
        break;
      }
      case WeakSlotState::Unmarked: {
        freeWeakSlot(&slot);
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
      // Wrap the call to getCellMarkBit in a lambda to work around potential
      // issues building with Visual Studio
      [](GCCell *cell) { return HeapSegment::getCellMarkBit(cell); },
      /*markFromVal*/
      [&acceptor](GCCell *valCell, HermesValue &valRef) {
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
  return (youngGen_ ? youngGen_->used() : 0) + oldGen_.allocatedBytes();
}

uint64_t HadesGC::externalBytes() const {
  return ygExternalBytes_ + oldGen_.externalBytes();
}

uint64_t HadesGC::heapFootprint() const {
  size_t totalSegments = oldGen_.numSegments() + (youngGen_ ? 1 : 0);
  return totalSegments * AlignedStorage::size() + externalBytes();
}

uint64_t HadesGC::OldGen::allocatedBytes() const {
  return allocatedBytes_;
}

uint64_t HadesGC::OldGen::allocatedBytes(uint16_t segmentIdx) const {
  return segmentAllocatedBytes_[segmentIdx];
}

void HadesGC::OldGen::incrementAllocatedBytes(
    int32_t incr,
    uint16_t segmentIdx) {
  assert(segmentIdx < numSegments());
  allocatedBytes_ += incr;
  segmentAllocatedBytes_[segmentIdx] += incr;
  assert(
      allocatedBytes_ <= size() &&
      segmentAllocatedBytes_[segmentIdx] <= HeapSegment::maxSize() &&
      "Invalid increment");
}

uint64_t HadesGC::OldGen::externalBytes() const {
  return externalBytes_;
}

uint64_t HadesGC::OldGen::size() const {
  return numSegments() * HeapSegment::maxSize();
}

uint64_t HadesGC::OldGen::capacityBytes() const {
  return numCapacitySegments_ * HeapSegment::maxSize();
}

HadesGC::HeapSegment &HadesGC::youngGen() {
  return *youngGen_;
}

const HadesGC::HeapSegment &HadesGC::youngGen() const {
  return *youngGen_;
}

bool HadesGC::inYoungGen(const void *p) const {
  return ygStart_ == AlignedStorage::start(p);
}

std::vector<std::unique_ptr<HadesGC::HeapSegment>>::iterator
HadesGC::OldGen::begin() {
  return segments_.begin();
}

std::vector<std::unique_ptr<HadesGC::HeapSegment>>::iterator
HadesGC::OldGen::end() {
  return segments_.end();
}

std::vector<std::unique_ptr<HadesGC::HeapSegment>>::const_iterator
HadesGC::OldGen::begin() const {
  return segments_.begin();
}

std::vector<std::unique_ptr<HadesGC::HeapSegment>>::const_iterator
HadesGC::OldGen::end() const {
  return segments_.end();
}

size_t HadesGC::OldGen::numSegments() const {
  return segments_.size();
}

size_t HadesGC::OldGen::maxNumSegments() const {
  assert(
      gc_->maxHeapSize_ % AlignedStorage::size() == 0 &&
      "maxHeapSize must be evenly disible by AlignedStorage::size");
  return (gc_->maxHeapSize_ / AlignedStorage::size()) - 1;
}

HadesGC::HeapSegment &HadesGC::OldGen::operator[](size_t i) {
  return *segments_[i];
}

std::unique_ptr<HadesGC::HeapSegment> HadesGC::createSegment(bool isYoungGen) {
  if (heapFootprint() >= maxHeapSize_) {
    return nullptr;
  }
  auto res = AlignedStorage::create(
      provider_.get(), isYoungGen ? "young-gen" : "old-gen");
  if (!res) {
    return nullptr;
  }
  std::unique_ptr<HeapSegment> seg(new HeapSegment{std::move(res.get())});
#ifdef HERMESVM_COMPRESSED_POINTERS
  pointerBase_->setSegment(++numSegments_, seg->lowLim());
#endif
  seg->markBitArray().markAll();
  if (isYoungGen) {
    addSegmentExtentToCrashManager(*seg, "YG");
  }
  return seg;
}

void HadesGC::OldGen::addSegment(std::unique_ptr<HeapSegment> seg) {
  uint16_t newSegIdx = segments_.size();
  reserveSegments(newSegIdx + 1);
  segments_.emplace_back(std::move(seg));
  segmentAllocatedBytes_.push_back(0);
  HeapSegment &newSeg = *segments_.back();
  incrementAllocatedBytes(newSeg.used(), newSegIdx);
  // Add a set of freelist buckets for this segment.
  freelistSegmentsBuckets_.emplace_back();
  assert(
      freelistSegmentsBuckets_.size() == segments_.size() &&
      "Must have as many freelists as segments.");

  // Add the remainder of the segment to the freelist.
  uint32_t sz = newSeg.available();
  if (sz >= minAllocationSize()) {
    auto res = newSeg.bumpAlloc(sz);
    assert(res.success);
    addCellToFreelist(res.ptr, sz, segments_.size() - 1);
  }

  gc_->addSegmentExtentToCrashManager(
      newSeg, oscompat::to_string(numSegments()));
}

void HadesGC::OldGen::reserveSegments(size_t numCapacitySegments) {
  numCapacitySegments_ = std::max(numCapacitySegments, numCapacitySegments_);
}

bool HadesGC::inOldGen(const void *p) const {
  for (auto seg = oldGen_.begin(), end = oldGen_.end(); seg != end; ++seg) {
    if ((*seg)->contains(p)) {
      return true;
    }
  }
  // None of the old gen segments matched the pointer.
  return false;
}

void HadesGC::yieldToOldGen() {
  assert(inGC() && "Must be in GC when yielding to old gen");
  if (!kConcurrentGC) {
    // If there is an ongoing collection, update the drain rate before
    // collecting.
    if (concurrentPhase_ == Phase::Mark)
      oldGenMarker_->setDrainRate(getDrainRate());

    incrementalCollect();
    return;
  }
  assert(gcMutex_ && "Must hold the GC mutex when calling yieldToOldGen");

  if (concurrentPhase_ != Phase::CompleteMarking) {
    // If the OG isn't waiting for a STW pause yet, exit immediately.
    return;
  }
  completeMarking();
  // Notify the waiting OG collection that it can complete.
  stopTheWorldCondVar_.notify_one();
}

size_t HadesGC::getDrainRate() {
  // Concurrent collections don't need to use the drain rate because they
  // only yield the lock periodically to be interrupted, but otherwise will
  // continuously churn through work regardless of the rate.
  // Non-concurrent collections, on the other hand, can only make progress
  // at YG collection intervals, so they need to be configured to mark the
  // OG faster than it fills up.
  assert(!kConcurrentGC);

  // Assume this many bytes are live in the OG. We want to make progress so
  // that over all YG collections before the heap fills up, we are able to
  // complete marking before OG fills up.
  // Don't include external memory since that doesn't need to be marked.
  const size_t bytesToFill = oldGen_.capacityBytes() - oldGen_.allocatedBytes();
  // On average, the number of bytes that survive a YG collection. Round it up
  // to at least 1.
  const uint64_t ygSurvivalBytes =
      std::max(ygAverageSurvivalRatio_ * HeapSegment::maxSize(), 1.0);
  const size_t ygCollectionsUntilFull =
      llvh::divideCeil(bytesToFill, ygSurvivalBytes);
  // We need to account for the STW pause and sweeping. The STW pause requires 1
  // additional YG collection. Sweeping will take exactly
  // oldGen_.sweepSegmentsRemaining() YG collections to fully sweep. Subtract
  // that from the iterations allowed for marking. Leave at least 1 if there
  // aren't enough YG collections remaining.
  const size_t postMarkIterations = oldGen_.sweepSegmentsRemaining() + 1;
  const size_t markIterations = ygCollectionsUntilFull > postMarkIterations
      ? ygCollectionsUntilFull - postMarkIterations
      : 1;
  assert(
      markIterations != 0 &&
      "All of the math above should avoid a 0 markIterations");
  // If any of the above calculations end up being a tiny drain rate, make the
  // lower limit at least 8 KB, to ensure collections eventually end.
  constexpr uint64_t byteDrainRateMin = 8192;
  return std::max(
      oldGenMarker_->bytesToMark() / markIterations, byteDrainRateMin);
}

void HadesGC::addSegmentExtentToCrashManager(
    const HeapSegment &seg,
    std::string extraName) {
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
  assert(inGC() && "Must be in GC to call verifyCardTable");
  struct VerifyCardDirtyAcceptor final : public SlotAcceptorDefault {
    using SlotAcceptorDefault::accept;
    using SlotAcceptorDefault::SlotAcceptorDefault;

    void accept(void *&ptr) override {
      char *valuePtr = reinterpret_cast<char *>(ptr);
      char *locPtr = reinterpret_cast<char *>(&ptr);

      acceptHelper(valuePtr, locPtr);
    }

    void accept(BasedPointer &ptr) override {
      // Don't use the default from SlotAcceptorDefault since the address of
      // the reference is used.
      PointerBase *const base = gc.getPointerBase();
      char *valuePtr = reinterpret_cast<char *>(base->basedToPointer(ptr));
      char *locPtr = reinterpret_cast<char *>(&ptr);

      acceptHelper(valuePtr, locPtr);
    }

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

  for (const auto &segPtr : oldGen_) {
    segPtr->cardTable().verifyBoundaries(segPtr->start(), segPtr->level());
  }
}

#endif

} // namespace vm
} // namespace hermes
