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
#include "hermes/VM/SlotAcceptorDefault-inline.h"
#include "hermes/VM/SlotAcceptorDefault.h"

#include <array>
#include <forward_list>
#include <stack>

namespace hermes {
namespace vm {

static const char kHeapNameForAnalytics[] = "hades";

// A free list cell is always variable-sized.
const VTable HadesGC::OldGen::FreelistCell::vt{CellKind::FreelistKind,
                                               /*variableSize*/ 0};

void FreelistBuildMeta(const GCCell *, Metadata::Builder &) {}

HadesGC::HeapSegment::HeapSegment(AlignedStorage &&storage)
    : AlignedHeapSegment{std::move(storage)} {
  // Make sure end() is at the maxSize.
  growToLimit();
}

void HadesGC::HeapSegment::transitionToFreelist(OldGen &og) {
  assert(bumpAllocMode_ && "This segment has already been transitioned");
  // Add a free list cell that spans the distance from end to level.
  const uint32_t sz = end() - level();
  if (sz < minAllocationSize()) {
    // There's not enough space to add a free list node.
    // That's ok, the segment is still well-formed. To eventually reclaim this
    // space, the sweeper should coalesce this tail space when the last object
    // is free'd.
    bumpAllocMode_ = false;
    return;
  }
  AllocResult res = bumpAlloc(sz);
  assert(res.success && "Failed to bump the level to the end");
  bumpAllocMode_ = false;
  og.addCellToFreelist(res.ptr, sz, /*alreadyFree*/ true);
}

GCCell *HadesGC::OldGen::finishAlloc(FreelistCell *cell, uint32_t sz) {
  // Track the number of allocated bytes in a segment.
  allocatedBytes_ += sz;
  // Unpoison the memory so that the mutator can use it.
  __asan_unpoison_memory_region(cell + 1, sz - sizeof(FreelistCell));
  // Write a mark bit so this entry doesn't get free'd by the sweeper.
  HeapSegment::setCellMarkBit(cell);
  // Could overwrite the VTable, but the allocator will write a new one in
  // anyway.
  return cell;
}

AllocResult HadesGC::HeapSegment::bumpAlloc(uint32_t sz) {
  assert(
      bumpAllocMode_ && "Shouldn't use bumpAlloc except on specific segments");
  // Don't use a free list for bump allocation.
  AllocResult res = AlignedHeapSegment::alloc(sz);
  if (res.success) {
    // Set the cell head for any successful alloc, so that write barriers can
    // move from dirty cards to the head of the object.
    setCellHead(static_cast<GCCell *>(res.ptr));
    // No need to change allocatedBytes_ here, as AlignedHeapSegment::used()
    // already tracks that. Once the cell is transitioned to freelist mode the
    // right value will be stored in OldGen::allocatedBytes_.
  }
  return res;
}

AllocResult HadesGC::HeapSegment::youngGenBumpAlloc(uint32_t sz) {
  assert(bumpAllocMode_ && "Shouldn't use youngGenBumpAlloc on an OG segment");
  // The only difference between bumpAlloc and youngGenBumpAlloc is that the
  // latter doesn't need to set cell heads because YG never has dirty cards that
  // need to be scanned.
  return AlignedHeapSegment::alloc(sz);
}

void HadesGC::OldGen::addCellToFreelist(GCCell *cell) {
  addCellToFreelist(cell, cell->getAllocatedSize(), /*alreadyFree*/ false);
}

void HadesGC::OldGen::addCellToFreelist(
    void *addr,
    uint32_t sz,
    bool alreadyFree) {
  assert(
      sz >= sizeof(FreelistCell) &&
      "Cannot construct a FreelistCell into an allocation in the OG");
  // Get the size bucket for the cell being added;
  const uint32_t bucket = getFreelistBucket(sz);
  // Push onto the size-specific free list and set the corresponding bit in the
  // bit array.
  FreelistCell *newFreeCell =
      new (addr) FreelistCell{sz, freelistBuckets_[bucket]};
  freelistBuckets_[bucket] = newFreeCell;
  freelistBucketBitArray_.set(bucket, true);
  if (!alreadyFree) {
    // We free'd this many bytes.
    assert(
        allocatedBytes_ >= sz &&
        "Free'ing a cell that is larger than what is left allocated");
    allocatedBytes_ -= sz;
  }
  // In ASAN builds, poison the memory outside of the FreelistCell so that
  // accesses are flagged as illegal.
  // Here, and in other places where FreelistCells are poisoned, use +1 on the
  // pointer to skip towards the memory region directly after the FreelistCell
  // header of a cell. This way the header is always intact and readable, and
  // only the contents of the cell are poisoned.
  __asan_poison_memory_region(newFreeCell + 1, sz - sizeof(FreelistCell));
}

/* static */
void HadesGC::HeapSegment::setCellHead(const GCCell *cell) {
  MarkBitArrayNC &heads = cellHeadsCovering(cell);
  heads.mark(heads.addressToIndex(cell));
}

GCCell *HadesGC::HeapSegment::getCellHead(const void *address) {
  MarkBitArrayNC &heads = cellHeads();
  auto ind = heads.addressToIndex(address);
  // Go backwards from the current address looking for a marked bit, which means
  // that the address contains a GCCell header.
  ind = heads.findPrevMarkedBitFrom(ind + 1);
  GCCell *cell = reinterpret_cast<GCCell *>(heads.indexToAddress(ind));
  assert(
      cell->isValid() && "Object heads table doesn't point to a valid object");
  return cell;
}

template <typename CallbackFunction>
void HadesGC::HeapSegment::forAllObjs(CallbackFunction callback) {
  void *const stop = level();
  for (GCCell *cell = reinterpret_cast<GCCell *>(start()); cell < stop;
       cell = cell->nextCell()) {
    // Skip free-list entries.
    if (!vmisa<OldGen::FreelistCell>(cell)) {
      callback(cell);
    }
  }
}

void HadesGC::OldGen::FreelistCell::split(OldGen &og, uint32_t sz) {
  const auto origSize = getAllocatedSize();
  assert(
      origSize >= sz + minAllocationSize() &&
      "Can't split if it would leave too small of a second cell");
  char *nextCellAddress = reinterpret_cast<char *>(this) + sz;
  // We're about to touch some memory in the newly split cell.
  // All other memory should remain poisoned.
  __asan_unpoison_memory_region(nextCellAddress, sizeof(FreelistCell));
  // Adding the newly formed cell to the free list will add it to the
  // appropriate size bucket.
  og.addCellToFreelist(nextCellAddress, origSize - sz, /*alreadyFree*/ true);
  GCCell *const newCell = reinterpret_cast<GCCell *>(nextCellAddress);
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
  HeapSegment::setCellHead(newCell);
}

class HadesGC::CollectionStats final {
 public:
  using Clock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<Clock>;
  using Duration = std::chrono::microseconds;

  CollectionStats(HadesGC *gc, std::string extraInfo = "")
      : gc_{gc}, extraInfo_{std::move(extraInfo)} {}
  ~CollectionStats();

  /// Record the allocated bytes in the heap and its size before a collection
  /// begins.
  void setBeforeSizes(uint64_t allocated, uint64_t sz) {
    allocatedBefore_ = allocated;
    size_ = sz;
  }

  /// Record how many bytes were swept during the collection.
  void setSweptBytes(uint64_t sweptBytes) {
    sweptBytes_ = sweptBytes;
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

  double survivalRatio() const {
    return allocatedBefore_
        ? static_cast<double>(afterAllocatedBytes()) / allocatedBefore_
        : 0;
  }

 private:
  HadesGC *gc_;
  std::string extraInfo_;
  TimePoint beginTime_{};
  TimePoint endTime_{};
  Duration cpuDuration_{};
  uint64_t allocatedBefore_{0};
  uint64_t size_{0};
  uint64_t sweptBytes_{0};
};

HadesGC::CollectionStats::~CollectionStats() {
  gc_->recordGCStats(GCAnalyticsEvent{
      gc_->getName(),
      kHeapNameForAnalytics,
      extraInfo_,
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
  MarkAcceptor(GC &gc) : SlotAcceptorDefault{gc}, byteDrainRate_{0} {
    markedSymbols_.resize(gc.gcCallbacks_->getSymbolsEnd(), false);
  }

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

#ifdef HERMESVM_COMPRESSED_POINTERS
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
#endif

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
    if (sym.isInvalid() || sym.unsafeGetIndex() >= markedSymbols_.size()) {
      // Ignore symbols that aren't valid or are pointing outside of the range
      // when the collection began.
      return;
    }
    markedSymbols_[sym.unsafeGetIndex()] = true;
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
    byteDrainRate_ = rate;
  }

  /// Drain the mark stack of cells to be processed. This function periodically
  /// acquires and releases oldGenMutex_ and weakRefMutex_.
  /// \pre Should only be called for concurrent collections.
  void drainMarkWorklist() {
    assert(
        kConcurrentGC &&
        "drainMarkWorklist should only be called for concurrent GCs");
    // Use do-while to make sure both the global worklist and the local worklist
    // are examined at least once.
    do {
      // Hold the GC lock while setting mark bits. Grab it in big
      // chunks so that we're not constantly acquiring and releasing the lock.
      std::lock_guard<Mutex> ogLk{gc.gcMutex_};
      std::lock_guard<Mutex> wrLk{gc.weakRefMutex()};
      // See the comment in setDrainRate for why the drain rate isn't used here.
      constexpr size_t kConcurrentMarkLimit = 8192;
      drainSomeWork(kConcurrentMarkLimit);
      // It's ok to access localWorklist outside of the lock, since it is local
      // memory to only this thread.
    } while (!localWorklist_.empty());
  }

  /// Same as \c drainMarkWorklist, but assumes locks are already held, and
  /// doesn't release them. Can be used in a non-concurrent GC, or during a STW
  /// pause.
  void drainMarkWorklistNoLocks() {
    // This should only be called during a STW pause. This means no write
    // barriers should occur, and there's no need to check the global worklist
    // more than once.
    drainSomeWork(std::numeric_limits<size_t>::max());
    assert(allWorkIsDrained() && "Some work left that wasn't completed");
  }

  /// Drains some of the worklist, using a drain rate specified by
  /// \c setDrainRate.
  /// \pre Should only be called for non-concurrent collections.
  void drainSomeWork() {
    assert(
        !kConcurrentGC &&
        "drainSomeWork should only be called for non-concurrent GCs");
    drainSomeWork(byteDrainRate_);
  }

  /// Drain some of the work to be done for marking.
  /// \param markLimit Only mark up to this many bytes from the local
  /// worklist.
  ///   NOTE: This does *not* guarantee that the marker thread
  ///   has upper bounds on the amount of work it does before reading from the
  ///   global worklist. Any individual cell can be quite large (such as an
  ///   ArrayStorage), which means there is a possibility of the global worklist
  ///   filling up and blocking the mutator.
  void drainSomeWork(const size_t markLimit) {
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
  }

  /// \return true if there is no more work to be done for marking.
  /// WARN: This should only be called when the mutator is paused, as
  /// otherwise there is a race condition between reading this and new work
  /// getting added.
  bool allWorkIsDrained() {
    return localWorklist_.empty() && !globalWorklist_.hasPendingWork();
  }

  MarkWorklist &globalWorklist() {
    return globalWorklist_;
  }

  std::vector<JSWeakMap *> &reachableWeakMaps() {
    return reachableWeakMaps_;
  }

  std::vector<bool> &markedSymbols() {
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
  std::vector<bool> markedSymbols_;

  /// The number of bytes to drain per call to drainSomeWork. A higher rate
  /// means more objects will be marked.
  size_t byteDrainRate_;

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

HadesGC::OldGen::OldGen(HadesGC *gc)
    // Assume about 70% of the OG will survive initially.
    : gc_(gc), averageSurvivalRatio_{/*weight*/ 0.5, /*init*/ 0.7} {}

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
      youngGen_{createSegment(/*isYoungGen*/ true)},
      promoteYGToOG_{!gcConfig.getAllocInYoung()},
      revertToYGAtTTI_{gcConfig.getRevertToYGAtTTI()},
      occupancyTarget_(gcConfig.getOccupancyTarget()),
      // Assume about 30% of the YG will survive initially.
      ygAverageSurvivalRatio_{/*weight*/ 0.5, /*init*/ 0.3} {
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
  info.totalAllocatedBytes = 0;
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
  json.emitKeyValue("collector", "hades");
  json.emitKey("stats");
  json.openDict();
  json.closeDict();
  json.closeDict();
}

void HadesGC::collect() {
  {
    // Wait for any existing collections to finish before starting a new one.
    std::lock_guard<Mutex> lk{gcMutex_};
    waitForCollectionToFinish();
  }
  // This function should block until a collection finishes.
  // YG needs to be empty in order to do an OG collection.
  youngGenCollection(/*forceOldGenCollection*/ true);
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

  if (!kConcurrentGC) {
    if (oldGenMarker_) {
      // Complete the collection.
      completeNonConcurrentOldGenCollection();
    }
    return;
  }
  std::unique_lock<Mutex> lk{gcMutex_, std::adopt_lock};
  // Before waiting for the OG collection to finish, tell the OG collection
  // that the world is stopped.
  worldStopped_ = true;
  // Notify a waiting OG collection that it can complete.
  lk.unlock();
  stopTheWorldCondVar_.notify_all();
  // Wait for an existing collection to finish.
  oldGenCollectionThread_.join();
  // Undo the worldStopped_ change, as the mutator will now resume. Can't rely
  // on completeMarking to set worldStopped_ to false, as the OG collection
  // might have already passed the stage where it was waiting for STW.
  lk.lock();
  assert(concurrentPhase_ == Phase::None);
  worldStopped_ = false;
  // Check for a tripwire, since we know a collection just completed.
  checkTripwireAndResetStats();

  assert(lk && "Lock must be re-acquired before exiting");
  assert(gcMutex_ && "GC mutex must be re-acquired before exiting");
  // Release association with the mutex to prevent the destructor from unlocking
  // it.
  lk.release();
}

void HadesGC::oldGenCollection() {
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
#ifdef HERMES_SLOW_DEBUG
  checkWellFormed();
#endif
  // Note: This line calls the destructor for the previous CollectionStats (if
  // any) in addition to creating a new CollectionStats. It is desirable to
  // call the destructor here so that the analytics callback is invoked from the
  // mutator thread. This might also be done from checkTripwireAndResetStats.
  ogCollectionStats_ = llvh::make_unique<CollectionStats>(this, "old");
  // NOTE: Leave CPU time as zero if the collection isn't concurrent, as the
  // times aren't useful.
  auto cpuTimeStart = oscompat::thread_cpu_time();
  ogCollectionStats_->setBeginTime();
  ogCollectionStats_->setBeforeSizes(oldGen_.allocatedBytes(), oldGen_.size());

  if (revertToYGAtTTI_) {
    // If we've reached the first OG collection, and reverting behavior is
    // requested, switch back to YG mode.
    promoteYGToOG_ = false;
  }
  // First, clear any mark bits that were set by direct-to-OG allocation, they
  // aren't needed anymore.
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
  oldGen_.setBytesToMark(
      oldGen_.allocatedBytes() * oldGen_.averageSurvivalRatio());
  oldGenMarker_.reset(new MarkAcceptor{*this});
  oldGenMarker_->setDrainRate(getDrainRate());
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
    // NOTE: Nothing is done here, YG will check for this condition and drain
    // directly from the oldGenMarker_.
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
  oldGenMarker_->drainMarkWorklist();
  completeMarking();
  sweep();
  std::lock_guard<Mutex> g{gcMutex_};
  ogCollectionStats_->setEndTime();
  auto cpuTimeEnd = oscompat::thread_cpu_time();
  ogCollectionStats_->incrementCPUTime(cpuTimeEnd - cpuTimeStart);
  // TODO: Should probably check for cancellation, either via a
  // promise/future pair or a condition variable.
  concurrentPhase_ = Phase::None;
}

void HadesGC::completeNonConcurrentOldGenCollection() {
  assert(
      !kConcurrentGC &&
      "Shouldn't call completeNonConcurrentOldGenCollection if concurrency "
      "is enabled");
  completeMarkingAssumeLocks();
  // Sweeping will try to acquire mutexes, but they will be fake mutexes, so
  // there's no performance cost.
  sweep();
  ogCollectionStats_->setEndTime();
  concurrentPhase_ = Phase::None;
}

void HadesGC::completeMarking() {
  // Both locks are held here for a few reasons.
  //  * gcMutex_ is associated with the stopTheWorldCondVar and needs to be held
  //  in order to stop the world. It also synchronises access to GC data
  //  structures and if the mutator were still active, would prevent concurrent
  //  access to them from YG.
  //  * weakRefMutex_ synchronises access to WeakRef data structures and
  //  prevents the mutator from accessing them.
  // This lock order is required for any other lock acquisitions in this file.
  std::unique_lock<Mutex> lk{gcMutex_};
  stopTheWorldRequested_ = true;
  waitForConditionVariable(
      stopTheWorldCondVar_, lk, [this]() { return worldStopped_; });
  assert(inGC() && "inGC_ must be set during the STW pause");
  WeakRefLock weakRefLock{weakRefMutex()};
  completeMarkingAssumeLocks();

  // Let the thread that yielded know that the STW pause has completed.
  stopTheWorldRequested_ = false;
  worldStopped_ = false;
  stopTheWorldCondVar_.notify_all();
}

void HadesGC::completeMarkingAssumeLocks() {
  oldGenMarker_->globalWorklist().flushPushChunk();
  // Drain the marking queue.
  oldGenMarker_->drainMarkWorklistNoLocks();
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

  // In order to free symbols and weak refs, we need to know if any are in use
  // by YG. Iterate through YG's objects and mark their symbols.
  findYoungGenSymbolsAndWeakRefs();

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

void HadesGC::findYoungGenSymbolsAndWeakRefs() {
  class SymbolAndWeakRefAcceptor final : public SlotAcceptor,
                                         public WeakRefAcceptor {
   public:
    explicit SymbolAndWeakRefAcceptor(GC &gc, std::vector<bool> &markedSymbols)
        : markedSymbols_{markedSymbols} {}

    // Do nothing for pointers.
    void accept(void *&) override {}
#ifdef HERMESVM_COMPRESSED_POINTERS
    void accept(BasedPointer &) override {}
#endif
    void accept(GCPointerBase &ptr) override {}

    void accept(HermesValue &hvRef) override {
      const HermesValue hv = hvRef;
      if (hv.isSymbol()) {
        accept(hv.getSymbol());
      }
    }

    void accept(SymbolID sym) override {
      if (sym.isInvalid() || sym.unsafeGetIndex() >= markedSymbols_.size()) {
        // Ignore symbols that aren't valid or are pointing outside of the range
        // when the collection began.
        return;
      }
      markedSymbols_[sym.unsafeGetIndex()] = true;
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

   private:
    std::vector<bool> &markedSymbols_;
  };

  SymbolAndWeakRefAcceptor acceptor{*this, oldGenMarker_->markedSymbols()};
  // We're scanning YG while it might be in the middle of its own collection,
  // if it is waiting for an OG GC to complete.
  // During a YG GC, the VTables might be replaced by fowarding pointers,
  // check the forwarded cell instead of the invalid current cell.
  void *const stop = youngGen().level();
  for (GCCell *cell = reinterpret_cast<GCCell *>(youngGen().start());
       cell < stop;) {
    if (cell->hasMarkedForwardingPointer()) {
      GCCell *const forwardedCell = cell->getMarkedForwardingPointer();
      // Just need to mark symbols and WeakRefs, doesn't need to worry about
      // un-updated pointers.
      GCBase::markCell(forwardedCell, this, acceptor);
      cell = reinterpret_cast<GCCell *>(
          reinterpret_cast<char *>(cell) + forwardedCell->getAllocatedSize());
    } else {
      GCBase::markCell(cell, this, acceptor);
      cell = cell->nextCell();
    }
  }
}

void HadesGC::sweep() {
  // Sweep phase: iterate through dead objects and add them to the
  // free list. Also finalize them at this point.
  size_t segEnd;
  const bool isTracking = isTrackingIDs();
  {
    std::lock_guard<Mutex> lk{gcMutex_};
    segEnd = oldGen_.numSegments();
  }

  uint64_t sweptBytes = 0;
  // The number of segments can change during this loop if a YG collection
  // interleaves. There's no need to sweep those extra segments since they are
  // full of newly promoted objects from YG (which have all their mark bits
  // set).
  for (size_t i = 0; i < segEnd; ++i) {
    // Acquire and release the lock once per loop, to allow YG collections to
    // interrupt the sweeper.
    std::lock_guard<Mutex> lk{gcMutex_};
    oldGen_[i].forAllObjs([this, isTracking, &sweptBytes](GCCell *cell) {
      // forAllObjs skips free list cells, so no need to check for those.
      assert(cell->isValid() && "Invalid cell in sweeping");
      if (HeapSegment::getCellMarkBit(cell)) {
        return;
      }
      sweptBytes += cell->getAllocatedSize();
      // Cell is dead, run its finalizer first if it has one.
      cell->getVT()->finalizeIfExists(cell, this);
      oldGen_.addCellToFreelist(cell);
      if (isTracking) {
        // There is no race condition here, because the object has already been
        // determined to be dead, so nothing can be accessing it, or asking for
        // its ID.
        getIDTracker().untrackObject(cell);
        getAllocationLocationTracker().freeAlloc(cell);
      }
    });

    // Do *not* clear the mark bits. This is important to have a cheaper
    // solution to a race condition in weakRefReadBarrier. If it gets
    // interrupted between reading the concurrentPhase_ and checking the mark
    // bits, the GC might finish. In that case, the mark bits will then be
    // read to determine if the weak ref is live.
    // Mark bits are reset before any new collection occurs, so there's no
    // need to worry about their information being misused.
  }
  std::lock_guard<Mutex> lk{gcMutex_};
  ogCollectionStats_->setSweptBytes(sweptBytes);
  oldGen_.updateAverageSurvivalRatio(ogCollectionStats_->survivalRatio());
  const size_t targetCapacity =
      ogCollectionStats_->afterAllocatedBytes() / occupancyTarget_;
  const size_t targetSegments =
      llvh::divideCeil(targetCapacity, HeapSegment::maxSize());
  const size_t finalSegments =
      std::min(targetSegments, oldGen_.maxNumSegments());
  oldGen_.reserveSegments(finalSegments);
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
  }
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
      calledByBackgroundThread() == worldStopped_ &&
      "Read barrier invoked by background thread outside STW.");
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
  youngGen().forAllObjs(callback);
  for (auto seg = oldGen_.begin(), end = oldGen_.end(); seg != end; ++seg) {
    (*seg)->forAllObjs(callback);
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
  assert(sz <= maxAllocationSize() && "Allocating too large of an object");
  if (kConcurrentGC) {
    HERMES_SLOW_ASSERT(
        !weakRefMutex() &&
        "WeakRef mutex should not be held when alloc is called");
  }
  if (!fixedSize && LLVM_UNLIKELY(sz >= HeapSegment::maxSize() / 2)) {
    return allocLongLived(sz);
  }
  if (shouldSanitizeHandles()) {
    // The best way to sanitize uses of raw pointers outside handles is to force
    // the entire heap to move, and ASAN poison the old heap. That is too
    // expensive to do, even with sampling, for Hades. It also doesn't test the
    // more interesting aspect of Hades which is concurrent background
    // collections. So instead, do a youngGenCollection which force-starts an
    // oldGenCollection if one is not already running.
    youngGenCollection(/*forceOldGenCollection*/ true);
  }
  AllocResult res = youngGen().youngGenBumpAlloc(sz);
  if (LLVM_UNLIKELY(!res.success)) {
    // Failed to alloc in young gen, do a young gen collection.
    youngGenCollection(/*forceOldGenCollection*/ false);
    res = youngGen().youngGenBumpAlloc(sz);
    assert(res.success && "Should never fail to allocate");
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
  // Alloc directly into the old gen.
  std::lock_guard<Mutex> lk{gcMutex_};
  void *res = oldGen_.alloc(heapAlignSize(sz));
  // Need to initialize the memory here to a valid cell to prevent the case
  // where sweeping discovers the uninitialized memory while it's traversing
  // a segment. This only happens at the end of a bump-alloc segment.
  new (res) OldGen::FreelistCell(sz, nullptr);
  return res;
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
  if (segments_.size() < maxNumSegments()) {
    std::unique_ptr<HeapSegment> seg = gc_->createSegment(/*isYoungGen*/ false);
    // Complete this allocation using a bump alloc.
    AllocResult res = seg->bumpAlloc(sz);
    assert(
        res.success &&
        "A newly created segment should always be able to allocate");
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
    if (FreelistCell *cell = freelistBuckets_[bucket]) {
      // There is a free cell for this size class, take it and return.
      assert(
          cell->getAllocatedSize() == sz &&
          "Size bucket should be an exact match");
      freelistBuckets_[bucket] = cell->next_;
      freelistBucketBitArray_.set(bucket, freelistBuckets_[bucket]);
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
    assert(freelistBuckets_[bucket] && "Empty bucket should not have bit set!");
    // Need to track the previous entry in order to change the next pointer.
    FreelistCell **prevLoc = &freelistBuckets_[bucket];
    FreelistCell *cell = freelistBuckets_[bucket];

    while (cell) {
      assert(cell == *prevLoc && "prevLoc should be updated in each iteration");
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
        // Split the free cell. In order to avoid initializing soon-to-be-unused
        // values like the size and the next pointer, copy the return path here.
        *prevLoc = cell->next_;
        // Update the bit in the bit array if freelistBuckets_[bucket] has
        // changed
        freelistBucketBitArray_.set(bucket, freelistBuckets_[bucket]);
        cell->split(*this, sz);
        return finishAlloc(cell, sz);
      } else if (cellSize == sz) {
        // Exact match, take it.
        *prevLoc = cell->next_;
        freelistBucketBitArray_.set(bucket, freelistBuckets_[bucket]);
        return finishAlloc(cell, sz);
      }
      // Non-exact matches, or anything just barely too small to fit, will need
      // to find another block.
      // NOTE: This is due to restrictions on the minimum cell size to keep the
      // heap parseable, especially in debug mode. If this minimum size becomes
      // smaller (smaller header, size becomes part of it automatically, debug
      // magic field is handled differently), this decisions can be re-examined.
      // An example alternative is to make a special fixed-size cell that is
      // only as big as an empty GCCell. That alternative only works if the
      // empty is small enough to fit in any gap in the heap. That's not true in
      // debug modes currently.
      prevLoc = &cell->next_;
      cell = cell->next_;
    }
  }
  return nullptr;
}

void HadesGC::youngGenCollection(bool forceOldGenCollection) {
  CollectionStats stats{this, "young"};
  auto cpuTimeStart = oscompat::thread_cpu_time();
  stats.setBeginTime();
  stats.setBeforeSizes(youngGen().used(), youngGen().size());
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
  uint64_t promotedBytes = 0, usedBefore = youngGen().used();
  if (promoteYGToOG_) {
    promotedBytes = usedBefore;
    promoteYoungGenToOldGen();
  } else {
    auto &yg = youngGen();

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
    promotedBytes = acceptor.promotedBytes();
    {
      WeakRefLock weakRefLock{weakRefMutex_};
      // Now that all YG objects have been marked, update weak references.
      updateWeakReferencesForYoungGen();
    }
    // Inform trackers about objects that died during this YG collection.
    if (isTrackingIDs()) {
      char *ptr = youngGen().start();
      char *const lvl = youngGen().level();
      while (ptr < lvl) {
        GCCell *cell = reinterpret_cast<GCCell *>(ptr);
        if (cell->hasMarkedForwardingPointer()) {
          auto *fptr = cell->getMarkedForwardingPointer();
          ptr += reinterpret_cast<GCCell *>(fptr)->getAllocatedSize();
        } else {
          ptr += cell->getAllocatedSize();
          getIDTracker().untrackObject(cell);
          getAllocationLocationTracker().freeAlloc(cell);
        }
      }
    }
    // Run finalizers for young gen objects.
    finalizeYoungGenObjects();
    // Now the copy list is drained, and all references point to the old
    // gen. Clear the level of the young gen.
    yg.resetLevel();
    assert(
        youngGen().markBitArray().findNextUnmarkedBitFrom(0) ==
            youngGen().markBitArray().size() &&
        "Young gen segment must have all mark bits set");
  }
#ifdef HERMES_SLOW_DEBUG
  checkWellFormed();
  // Check that the card tables are well-formed after the collection.
  verifyCardTable();
#endif
  // Post-allocated has an ambiguous meaning for a young-gen GC, since the
  // young gen must be completely evacuated. Since zeros aren't really
  // useful here, instead put the number of bytes that were promoted into
  // old gen, which is the amount that survived the collection.
  stats.setSweptBytes(usedBefore - promotedBytes);
  // The average survival ratio should be updated before starting an OG
  // collection.
  ygAverageSurvivalRatio_.update(stats.survivalRatio());
  if (concurrentPhase_ == Phase::None) {
    // There is no OG collection running, check the tripwire in case this is the
    // first YG after an OG completed.
    checkTripwireAndResetStats();
    if (forceOldGenCollection) {
      oldGenCollection();
    } else {
      // If the OG is sufficiently full after the collection finishes, begin
      // an OG collection.
      const uint64_t totalAllocated = oldGen_.allocatedBytes();
      const uint64_t totalBytes = oldGen_.capacityBytes();
      constexpr double kCollectionThreshold = 0.75;
      double allocatedRatio = static_cast<double>(totalAllocated) / totalBytes;
      if (allocatedRatio >= kCollectionThreshold) {
        oldGenCollection();
      }
    }
  } else if (oldGenMarker_) {
    oldGenMarker_->setDrainRate(getDrainRate());
  }
  // Give an existing background thread a chance to complete.
  yieldToOldGen();
  stats.setEndTime();
  auto cpuTimeEnd = oscompat::thread_cpu_time();
  stats.incrementCPUTime(cpuTimeEnd - cpuTimeStart);
}

void HadesGC::promoteYoungGenToOldGen() {
  // The flag is on to prevent GC until TTI. Promote the whole YG segment
  // directly into OG.
  // Before promoting it, set the cell heads correctly for the segment
  // going into OG. This could be done at allocation time, but at a cost
  // to YG alloc times for a case that might not come up.
  youngGen_->forAllObjs([this](GCCell *cell) { youngGen_->setCellHead(cell); });
  // It is important that this operation is just a move of pointers to
  // segments. The addresses have to stay the same or else it would
  // require a marking pass through all objects.
  // This will also rename the segment in the crash data.
  oldGen_.addSegment(std::move(youngGen_));
  // Replace YG with a new segment.
  youngGen_ = createSegment(/*isYoungGen*/ true);
  // These objects will be finalized by an OG collection.
  youngGenFinalizables_.clear();
}

void HadesGC::checkTripwireAndResetStats() {
  assert(
      gcMutex_ &&
      "gcMutex must be held before calling checkTripwireAndResetStats");
  if (!ogCollectionStats_) {
    return;
  }
  const auto afterAllocatedBytes = ogCollectionStats_->afterAllocatedBytes();
  // Have to release the lock in case the tripwire callback calls
  // createSnapshot.
  std::unique_lock<Mutex> lk{gcMutex_, std::adopt_lock};
  lk.unlock();
  // We use the amount of live data from after a GC completed as the minimum
  // bound of what is live.
  checkTripwire(afterAllocatedBytes);
  lk.lock();
  // Resetting the stats both runs the destructor (submitting the stats), as
  // well as prevent us from checking the tripwire every YG.
  ogCollectionStats_.reset();
  // Don't run the destructor to unlock the mutex.
  lk.release();
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
  const Phase phase = concurrentPhase_;
  // If an OG collection is active, it will be determining what weak references
  // are live. The YG collection shouldn't modify the state of any weak
  // references that OG is trying to track. Only fixup pointers that are
  // pointing to newly evac'ed YG objects.
  const bool ogCollectionActive = phase != Phase::None;
  for (auto &slot : weakPointers_) {
    switch (slot.state()) {
      case WeakSlotState::Free:
        break;

      case WeakSlotState::Marked:
        if (!ogCollectionActive) {
          // Set all allocated slots to unmarked.
          slot.unmark();
        }
        // fallthrough
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
        acceptor.drainMarkWorklistNoLocks();
        return true;
      },
      /*drainMarkStack*/
      [](MarkAcceptor &acceptor) {
        // The weak ref lock is held throughout this entire section, so no need
        // to re-lock it.
        acceptor.drainMarkWorklistNoLocks();
      },
      /*checkMarkStackOverflow (HadesGC does not have mark stack overflow)*/
      []() { return false; });

  acceptor.reachableWeakMaps().clear();
  (void)weakMapAllocBytes;
}

uint64_t HadesGC::allocatedBytes() const {
  return youngGen().used() + oldGen_.allocatedBytes();
}

uint64_t HadesGC::OldGen::allocatedBytes() const {
  return allocatedBytes_;
}

uint64_t HadesGC::OldGen::size() const {
  return numSegments() * HeapSegment::maxSize();
}

uint64_t HadesGC::OldGen::capacityBytes() const {
  return numCapacitySegments_ * HeapSegment::maxSize();
}

double HadesGC::OldGen::averageSurvivalRatio() const {
  return averageSurvivalRatio_;
}

void HadesGC::OldGen::updateAverageSurvivalRatio(double survivalRatio) {
  averageSurvivalRatio_.update(survivalRatio);
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
  auto res = AlignedStorage::create(
      provider_.get(), isYoungGen ? "young-gen" : "old-gen");
  if (!res) {
    hermes_fatal("Failed to alloc memory segment");
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
  assert(
      seg->isBumpAllocMode() &&
      "Segments added to OG must be in bump alloc mode!");
  segments_.emplace_back(std::move(seg));
  HeapSegment &newSeg = *segments_.back();
  allocatedBytes_ += newSeg.used();
  reserveSegments(segments_.size());
  // Switch the segment from bump alloc mode to free list mode and add any
  // remaining capacity to the free list.
  newSeg.transitionToFreelist(*this);
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
    // A non-concurrent GC needs to check if there's any work to be done from
    // oldGenMarker_.
    if (oldGenMarker_) {
      oldGenMarker_->drainSomeWork();
      if (oldGenMarker_->allWorkIsDrained()) {
        // If the work has finished completely, finish the collection.
        completeNonConcurrentOldGenCollection();
      }
    }
    return;
  }
  assert(
      gcMutex_ &&
      "Must hold the GC mutex when calling yieldToBackgroundThread");

  if (!stopTheWorldRequested_) {
    // If the OG isn't waiting for a STW pause yet, exit immediately.
    return;
  }
  worldStopped_ = true;
  // Notify a waiting OG collection that it can complete.
  stopTheWorldCondVar_.notify_all();
  std::unique_lock<Mutex> lk{gcMutex_, std::adopt_lock};
  waitForConditionVariable(
      stopTheWorldCondVar_, lk, [this]() { return !worldStopped_; });
  assert(
      !stopTheWorldRequested_ &&
      "The request should be set to false after being completed");
  lk.release();
}

size_t HadesGC::getDrainRate() {
  HERMES_SLOW_ASSERT(
      gcMutex_ && "Must hold the GC mutex when calling this function");
  if (kConcurrentGC) {
    // Concurrent collections don't need to use the drain rate because they
    // only yield the lock periodically to be interrupted, but otherwise will
    // continuously churn through work regardless of the rate.
    // Non-concurrent collections, on the other hand, can only make progress
    // at YG collection intervals, so they need to be configured to mark the
    // OG faster than it fills up.
    return 0;
  }
  // Assume this many bytes are live in the OG. We want to make progress so
  // that over all YG collections before the heap fills up, we are able to
  // complete marking before OG fills up.
  const size_t bytesToFill = oldGen_.capacityBytes() - oldGen_.allocatedBytes();
  const double ygSurvivalRatio = ygAverageSurvivalRatio_;
  size_t ygCollectionsUntilFull = ygSurvivalRatio
      ? bytesToFill / (ygSurvivalRatio * HeapSegment::maxSize())
      : std::numeric_limits<size_t>::max();
  if (ygCollectionsUntilFull == 0) {
    // If the bytes to fill is very small, integer division might give 0
    // collections. In this case, round up to 1.
    ygCollectionsUntilFull = 1;
  } else if (ygCollectionsUntilFull > 1) {
    // If all of the average survival ratios match the actual survival ratios,
    // we'll finish marking exactly when OG is full. Instead, to account for
    // both slightly off survival ratio estimates, as well as wanting to finish
    // collecting before OG fills up, aim to finish marking all of OG one YG
    // before we need to.
    --ygCollectionsUntilFull;
  }
  assert(
      ygCollectionsUntilFull != 0 && "Cannot have zero collections until full");
  // If any of the above calculations end up being a tiny drain rate, make the
  // lower limit at least 8 KB, to ensure collections eventually end.
  constexpr size_t byteDrainRateMin = 8192;
  return std::max(
      oldGen_.bytesToMark() / ygCollectionsUntilFull, byteDrainRateMin);
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

#ifdef HERMESVM_COMPRESSED_POINTERS
    void accept(BasedPointer &ptr) override {
      // Don't use the default from SlotAcceptorDefault since the address of
      // the reference is used.
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
  for (auto segit = oldGen_.begin(), end = oldGen_.end(); segit != end;
       ++segit) {
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
