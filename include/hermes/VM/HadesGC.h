/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_HADESGC_H
#define HERMES_VM_HADESGC_H

#include "hermes/ADT/BitArray.h"
#include "hermes/ADT/ExponentialMovingAverage.h"
#include "hermes/Support/SlowAssert.h"
#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/GCBase.h"

#include "llvh/Support/PointerLikeTypeTraits.h"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace hermes {
namespace vm {

class WeakRefBase;
template <class T>
class WeakRef;

/// A GC with a young and old generation, that does concurrent marking and
/// sweeping of the old generation.
///
/// The young gen is a single contiguous-allocation space.  A
/// young-gen collection completely evacuates live objects into the
/// older generation.
///
/// The old generation is a collection of heap segments, and allocations in the
/// old gen are done with a freelist (not a bump-pointer). When the old
/// collection is nearly full, it starts a backthround thread that will mark all
/// objects in the old gen, and then sweep the dead ones onto freelists.
///
/// Compaction is done in the old gen on a per-segment basis.
///
/// NOTE: Currently HadesGC is only a stub, meant to get things to compile.
/// It will be filled out to actually work later.
class HadesGC final : public GCBase {
 public:
  /// Initialize the GC with the give \p gcCallbacks and \p gcConfig.
  /// maximum size.
  /// \param gcCallbacks A callback interface enabling the garbage collector to
  ///   mark roots and free symbols.
  /// \param gcConfig A struct giving, e.g., minimum, initial, and maximum heap
  /// sizes.
  /// \param provider A provider of storage to be used by segments.
  HadesGC(
      MetadataTable metaTable,
      GCCallbacks *gcCallbacks,
      PointerBase *pointerBase,
      const GCConfig &gcConfig,
      std::shared_ptr<CrashManager> crashMgr,
      std::shared_ptr<StorageProvider> provider);

  ~HadesGC();

  static uint32_t minAllocationSize();

  static constexpr uint32_t maxAllocationSize() {
    // The largest allocation allowable in Hades is the max size a single
    // segment supports.
    return HeapSegment::maxSize();
  }

  /// \name GCBase overrides
  /// \{

  void getHeapInfo(HeapInfo &info) override;
  void getHeapInfoWithMallocSize(HeapInfo &info) override;
  void getCrashManagerHeapInfo(CrashManager::HeapInformation &info) override;
  void createSnapshot(llvh::raw_ostream &os) override;
  void printStats(JSONEmitter &json) override;

  /// \}

  /// \name GC non-virtual API
  /// \{

  /// Allocate a new cell of the specified size \p size.
  /// If necessary perform a GC cycle, which may potentially move allocated
  /// objects.
  /// \tparam fixedSize Indicates whether the allocation is for a fixed-size
  ///   cell, which can assumed to be small if true.
  /// \tparam hasFinalizer Indicates whether the object being allocated will
  ///   have a finalizer.
  template <bool fixedSize = true, HasFinalizer hasFinalizer = HasFinalizer::No>
  inline void *alloc(uint32_t sz);

  /// Like alloc above, but the resulting object is expected to be long-lived.
  /// Allocate directly in the old generation (doing a full collection if
  /// necessary to create room).
  /// \tparam hasFinalizer Indicates whether the object being allocated will
  ///   have a finalizer. Unused by Hades, but used by other GCs.
  template <HasFinalizer hasFinalizer = HasFinalizer::No>
  inline void *allocLongLived(uint32_t sz);

  /// Force a garbage collection cycle.
  /// (Part of general GC API defined in GCBase.h).
  void collect(std::string cause);

  /// Run the finalizers for all heap objects.
  void finalizeAll() override;

  /// Add some external memory cost to a cell.
  /// (Part of general GC API defined in GCBase.h).
  /// \pre canAllocExternalMemory(size) is true.
  void creditExternalMemory(GCCell *alloc, uint32_t size);

  /// Remove some external memory cost from a cell.
  /// (Part of general GC API defined in GCBase.h).
  void debitExternalMemory(GCCell *alloc, uint32_t size);
  void debitExternalMemoryFromFinalizer(GCCell *alloc, uint32_t size);

  /// \name Write Barriers
  /// \{

  /// NOTE: For all write barriers and read barriers:
  /// The call to writeBarrier/readBarrier must happen *before* the write/read
  /// to memory occurs.

  /// The given value is being written at the given loc (required to
  /// be in the heap). If value is a pointer, execute a write barrier.
  /// NOTE: The write barrier call must be placed *before* the write to the
  /// pointer, so that the current value can be fetched.
  void writeBarrier(void *loc, HermesValue value);

  /// The given pointer value is being written at the given loc (required to
  /// be in the heap). The value is may be null. Execute a write barrier.
  /// NOTE: The write barrier call must be placed *before* the write to the
  /// pointer, so that the current value can be fetched.
  void writeBarrier(void *loc, void *value);

  /// Special versions of \p writeBarrier for when there was no previous value
  /// initialized into the space.
  void constructorWriteBarrier(void *loc, HermesValue value);
  void constructorWriteBarrier(void *loc, void *value);

  void snapshotWriteBarrier(GCHermesValue *loc);
  void snapshotWriteBarrierRange(GCHermesValue *start, uint32_t numHVs);

  void weakRefReadBarrier(void *value);
  void weakRefReadBarrier(HermesValue value);

  /// \}

  /// Returns whether an external allocation of the given \p size fits
  /// within the maximum heap size. (Note that this does not guarantee that the
  /// allocation will "succeed" -- the size plus the used() of the heap may
  /// still exceed the max heap size. But if it fails, the allocation can never
  /// succeed.)
  bool canAllocExternalMemory(uint32_t size);

  /// Mark a symbol id as being used.
  void markSymbol(SymbolID symbolID);

  WeakRefSlot *allocWeakSlot(HermesValue init);

  /// Iterate over all objects in the heap, and call \p callback on them.
  /// \param callback A function to call on each found object.
  void forAllObjs(const std::function<void(GCCell *)> &callback);

  /// Inform the GC that TTI has been reached. This will transition the GC mode,
  /// if the GC was currently allocating directly into OG.
  void ttiReached();

  /// \}

  /// \return true if the pointer lives in the young generation.
  bool inYoungGen(const void *p) const;

#ifndef NDEBUG
  /// \name Debug APIs
  /// \{

  bool calledByBackgroundThread() const {
    // If the background thread is active, check if this thread matches the
    // background thread.
    return kConcurrentGC &&
        oldGenCollectionThread_.get_id() == std::this_thread::get_id();
  }

  /// See comment in GCBase.
  bool calledByGC() const {
    // Check if this is called by the background thread or the inGC flag is
    // set.
    return calledByBackgroundThread() || inGC();
  }

  /// Return true if \p ptr is currently pointing at valid accessable memory,
  /// allocated to an object.
  bool validPointer(const void *ptr) const;

  /// Return true if \p ptr is within one of the virtual address ranges
  /// allocated for the heap. Not intended for use in normal production GC
  /// operation, debug mode only.
  bool dbgContains(const void *ptr) const;

  /// Record that a cell of the given \p kind and size \p sz has been
  /// found reachable in a full GC.
  void trackReachable(CellKind kind, unsigned sz);

  /// \return Number of weak ref slots currently in use.
  /// Inefficient. For testing/debugging.
  size_t countUsedWeakRefs() const;

  /// Returns true if \p cell is the most-recently allocated finalizable object.
  bool isMostRecentFinalizableObj(const GCCell *cell) const;

  /// \}
#endif

  class CollectionStats;
  class EvacAcceptor;
  class MarkAcceptor;
  class MarkWeakRootsAcceptor;
  class OldGen;

  /// Similar to AlignedHeapSegment except it uses a free list.
  class HeapSegment final : private AlignedHeapSegment {
   public:
    explicit HeapSegment(AlignedStorage &&storage);
    ~HeapSegment() = default;

    /// Allocate space by bumping a level.
    /// \pre isBumpAllocMode() must be true.
    AllocResult bumpAlloc(uint32_t sz);

    /// YG has a much simpler alloc path, which shortcuts some steps the normal
    /// \p alloc takes.
    AllocResult youngGenBumpAlloc(uint32_t sz);

    /// Record the head of this cell so it can be found by the card scanner.
    static void setCellHead(const GCCell *start, const size_t sz);

    /// Find the head of the first cell that extends into the card at index
    /// \p cardIdx.
    /// \return A cell such that
    /// cell <= indexToAddress(cardIdx) < cell->nextCell().
    GCCell *getFirstCellHead(const size_t cardIdx);

    /// Call \p callback on every cell allocated in this segment.
    /// NOTE: Overridden to skip free list entries.
    template <typename CallbackFunction>
    void forAllObjs(CallbackFunction callback);
    template <typename CallbackFunction>
    void forAllObjs(CallbackFunction callback) const;

    bool isBumpAllocMode() const {
      return bumpAllocMode_;
    }

    /// Transitions this segment from bump-alloc mode to freelist mode.
    /// Can only be called once, when the segment is in bump-alloc mode. There
    /// is no transitioning from freelist mode back to bump-alloc mode.
    void transitionToFreelist(OldGen &og);

    // APIs from AlignedHeapSegment that are safe to use on a HeapSegment.
    using AlignedHeapSegment::available;
    using AlignedHeapSegment::cardTable;
    using AlignedHeapSegment::cardTableCovering;
    using AlignedHeapSegment::clearExternalMemoryCharge;
    using AlignedHeapSegment::contains;
    using AlignedHeapSegment::effectiveEnd;
    using AlignedHeapSegment::end;
    using AlignedHeapSegment::getCellMarkBit;
    using AlignedHeapSegment::level;
    using AlignedHeapSegment::lowLim;
    using AlignedHeapSegment::markBitArray;
    using AlignedHeapSegment::maxSize;
    using AlignedHeapSegment::resetLevel;
    using AlignedHeapSegment::setCellMarkBit;
    using AlignedHeapSegment::setEffectiveEnd;
    using AlignedHeapSegment::size;
    using AlignedHeapSegment::start;
    using AlignedHeapSegment::used;

   private:
    /// If true, then allocations into this segment increment a level inside the
    /// segment. Once the level reaches the end of the segment, no more
    /// allocations can occur.
    /// All segments begin in bumpAllocMode. If an OG segment has this mode set,
    /// and sweeping frees an object, this mode will be unset.
    bool bumpAllocMode_{true};
  };

  class OldGen final {
   public:
    explicit OldGen(HadesGC *gc);

    std::vector<std::unique_ptr<HeapSegment>>::iterator begin();
    std::vector<std::unique_ptr<HeapSegment>>::iterator end();
    std::vector<std::unique_ptr<HeapSegment>>::const_iterator begin() const;
    std::vector<std::unique_ptr<HeapSegment>>::const_iterator end() const;

    size_t numSegments() const;
    size_t maxNumSegments() const;

    HeapSegment &operator[](size_t i);

    /// Take ownership of the given segment.
    void addSegment(std::unique_ptr<HeapSegment> seg);

    /// Indicate that OG has a capacity of up to \p numCapacitySegments, some of
    /// which may be allocated as needed.
    void reserveSegments(size_t numCapacitySegments);

    /// Allocate into OG. Returns a pointer to the newly allocated space. That
    /// space must be filled before releasing the gcMutex_.
    /// \return A non-null pointer to memory in the old gen that should have a
    ///   constructor run in immediately.
    /// \pre gcMutex_ must be held before calling this function.
    /// \post This function either successfully allocates, or reports OOM.
    GCCell *alloc(uint32_t sz);

    /// Adds the given cell to the free list for this segment.
    /// \pre this->contains(cell) is true.
    void addCellToFreelist(GCCell *cell);

    /// Version of addCellToFreelist when nothing is initialized at the address
    /// yet.
    /// \param alreadyFree If true, this location is not currently allocated.
    void addCellToFreelist(void *addr, uint32_t sz, bool alreadyFree);

    /// \return the total number of bytes that are in use by the OG section of
    /// the JS heap, excluding free list entries.
    uint64_t allocatedBytes() const;

    /// \return the total number of bytes that are held in external memory, kept
    /// alive by objects in the OG.
    uint64_t externalBytes() const;

    /// \return the total number of bytes that are in use by the OG section of
    /// the JS heap, including free list entries.
    uint64_t size() const;

    /// \return the total number of bytes that we are willing to use in the OG
    /// section of the JS heap, including free list entries.
    uint64_t capacityBytes() const;

    /// \return the average survival ratio of the OG over time.
    double averageSurvivalRatio() const;

    /// Update the average survival ratio with a new instance.
    void updateAverageSurvivalRatio(double survivalRatio);

    /// \return the number of bytes that we expect to need to mark before an
    ///   OG collection finishes.
    /// NOTE: This is determined once at the start of collection, and does not
    /// change during a collection.
    size_t bytesToMark() const {
      return bytesToMark_;
    }

    void setBytesToMark(size_t bytesToMark) {
      bytesToMark_ = bytesToMark;
    }

    /// Add some external memory cost to the OG.
    void creditExternalMemory(uint32_t size) {
      externalBytes_ += size;
    }

    /// Remove some external memory cost from the OG.
    void debitExternalMemory(uint32_t size) {
      assert(
          externalBytes_ >= size && "Debiting more memory than was credited");
      externalBytes_ -= size;
    }

    class FreelistCell final : public VariableSizeRuntimeCell {
     private:
      static const VTable vt;

     public:
      // If null, this is the tail of the free list.
      FreelistCell *next_{nullptr};

      explicit FreelistCell(uint32_t sz) : VariableSizeRuntimeCell{&vt, sz} {}

      /// Shrink this cell by carving out a region of size \p sz bytes. Unpoison
      /// the carved out region if necessary and return it (without any
      /// initialisation).
      /// \param sz The size that the newly-split cell should be.
      /// \pre getAllocatedSize() >= sz + minAllocationSize()
      GCCell *carve(uint32_t sz);

      static bool classof(const GCCell *cell) {
        return cell->getKind() == CellKind::FreelistKind;
      }
    };

    /// Adds the given cell to the free list for this segment.
    /// \pre this->contains(cell) is true.
    void addCellToFreelist(FreelistCell *cell);

    /// Remove the cell pointed to by the pointer at \p prevLoc from
    /// the given \p bucket in the freelist.
    /// \return a pointer to the removed cell.
    FreelistCell *removeCellFromFreelist(FreelistCell **prevLoc, size_t bucket);

    /// Remove the first cell from the given \p bucket in the
    /// freelist.
    /// \return a pointer to the removed cell.
    FreelistCell *removeCellFromFreelist(size_t bucket);

   private:
    /// \return the index of the bucket in freelistBuckets_ corresponding to
    /// \p size.
    /// \post The returned index is less than kNumFreelistBuckets.
    static uint32_t getFreelistBucket(uint32_t size);

    HadesGC *gc_;
    std::vector<std::unique_ptr<HeapSegment>> segments_;

    /// This is the number of segments we should allow the OG to grow to before
    /// needing to wait on an OG collection. This represents the effective size
    /// of the OG but the actual segments can be allocated lazily.
    size_t numCapacitySegments_{0};

    /// This is the sum of all bytes currently allocated in the heap, excluding
    /// bump-allocated segments. Use \c allocatedBytes() to include
    /// bump-allocated segments.
    uint64_t allocatedBytes_{0};

    /// Tracks the average survival ratio over time of the OG.
    ExponentialMovingAverage averageSurvivalRatio_;

    /// An estimate of the number of bytes that need to be marked for a
    /// collection to complete.
    size_t bytesToMark_{0};

    /// The amount of bytes of external memory credited to objects in the OG.
    uint64_t externalBytes_{0};

    /// The freelist buckets are split into two sections. In the "small"
    /// section, there is one bucket for each size, in multiples of heapAlign.
    /// In the "large" section, there is a bucket for each power of 2. The
    /// bucket for a large block is obtained by rounding down to the nearest
    /// power of 2.

    /// So for instance, with a heap alignment of 8 bytes, 256 small buckets,
    /// and a maximum allocation size of 2^21, we would get:

    /// |    Small section      |  Large section   |
    /// +----+----+----+   +--------------+   +----+
    /// | 0  | 8  | 16 |...|2040|2048|4096|...|2^21|
    /// +----+----+----+   +--------------+   +----+

    static constexpr size_t kLogNumSmallFreelistBuckets = 8;
    static constexpr size_t kNumSmallFreelistBuckets = 1
        << kLogNumSmallFreelistBuckets;
    static constexpr size_t kLogMinSizeForLargeBlock =
        kLogNumSmallFreelistBuckets + LogHeapAlign;
    static constexpr size_t kMinSizeForLargeBlock = 1
        << kLogMinSizeForLargeBlock;
    static constexpr size_t kNumLargeFreelistBuckets =
        llvh::detail::ConstantLog2<HeapSegment::maxSize()>::value -
        kLogMinSizeForLargeBlock + 1;
    static constexpr size_t kNumFreelistBuckets =
        kNumSmallFreelistBuckets + kNumLargeFreelistBuckets;

    std::array<FreelistCell *, kNumFreelistBuckets> freelistBuckets_{};
    /// Keep track of which freelist buckets have valid elements to make search
    /// fast.
    BitArray<kNumFreelistBuckets> freelistBucketBitArray_;

    /// Searches the OG for a space to allocate memory into.
    /// \return A pointer to uninitialized memory that can be written into, null
    ///   if no such space exists.
    GCCell *search(uint32_t sz);

    /// Common path for when an allocation has succeeded.
    /// \param cell The free memory that will soon have an object allocated into
    ///   it.
    /// \param sz The number of bytes associated with the free memory.
    GCCell *finishAlloc(GCCell *cell, uint32_t sz);
  };

 private:
  /// The maximum number of bytes that the heap can hold. Once this amount has
  /// been filled up, OOM will occur.
  const uint64_t maxHeapSize_;

#ifdef HERMESVM_COMPRESSED_POINTERS
  /// This needs to be placed before youngGen_ and oldGen_, because those
  /// members use numSegments_ as part of being constructed.
  uint64_t numSegments_{0};
#endif

  /// Keeps the storage provider alive until after the GC is fully destructed.
  std::shared_ptr<StorageProvider> provider_;

  /// youngGen is a bump-pointer space, so it can re-use AlignedHeapSegment.
  /// Protected by gcMutex_.
  std::unique_ptr<HeapSegment> youngGen_;

  /// List of cells in YG that have finalizers. Iterate through this to clean
  /// them out.
  /// Protected by gcMutex_.
  std::vector<GCCell *> youngGenFinalizables_;

  /// oldGen_ is a free list space, so it needs a different segment
  /// representation.
  /// Protected by gcMutex_.
  OldGen oldGen_{this};

  /// weakPointers_ is a list of all the weak pointers in the system. They are
  /// invalidated if they point to an object that is dead, and do not count
  /// towards whether an object is live or dead.
  /// Protected by weakRefMutex().
  std::deque<WeakRefSlot> weakPointers_;

  /// Whoever holds this lock is permitted to modify data structures around the
  /// GC. This includes mark bits, free lists, etc.
  Mutex gcMutex_;

  enum class Phase : uint8_t { None, Mark, WeakMapScan, Sweep };

  /// Represents the current phase the concurrent GC is in. The main difference
  /// between phases is their effect on read and write barriers. Should only be
  /// accessed if gcMutex_ is acquired.
  Phase concurrentPhase_{Phase::None};

  /// Represents whether the background thread is currently marking. Should only
  /// be accessed by the mutator thread or during a STW pause. isOldGenMarking_
  /// is true if and only if concurrentPhase_ == Mark but is kept separate in
  /// order to reduce synchronisation requirements for write barriers.
  /// Prefer using concurrentPhase_ when acquiring gcMutex_ is not a concern.
  bool isOldGenMarking_{false};

  /// Used by the write barrier to add items to the worklist.
  /// Protected by gcMutex_.
  std::unique_ptr<MarkAcceptor> oldGenMarker_;

  /// This is the background thread that does marking and sweeping concurrently
  /// with the mutator.
  /// It should only be joined via \c waitForCollectionToFinish, which ensures
  /// that the STW pause handling is done correctly.
  std::thread oldGenCollectionThread_;

  /// This condition variable and bool are used for the mutator to
  /// signal the background marking thread when it's safe to try and complete.
  /// Thus, the GC thread can wait for worldStopped_ to be true to
  /// "stop the world" -- for example, to drain the final parts of the mark
  /// stack.
  std::condition_variable stopTheWorldCondVar_;
  /// Indicates whether OG has requested an STW pause, protected by gcMutex_.
  bool stopTheWorldRequested_{false};
  /// Indicates that the world is stopped, only access from the mutator or when
  /// the world is stopped.
  bool worldStopped_{false};

  /// If true, whenever YG fills up immediately put it into the OG.
  bool promoteYGToOG_;

  /// If true, turn off promoteYGToOG_ as soon as the first OG GC occurs.
  bool revertToYGAtTTI_;

  /// Target OG occupancy ratio at the end of an OG collection.
  const double occupancyTarget_;

  /// A collection section used to track the size of OG before and after an OG
  /// collection, as well as the time an OG collection takes.
  std::unique_ptr<CollectionStats> ogCollectionStats_;

  /// Pointer to the first free weak reference slot. Free weak refs are chained
  /// together in a linked list.
  WeakRefSlot *firstFreeWeak_{nullptr};

  /// The weighted average of the YG survival ratio over time.
  ExponentialMovingAverage ygAverageSurvivalRatio_;

  /// The amount of bytes of external memory credited to objects in the YG.
  uint64_t ygExternalBytes_{0};

  /// The main entrypoint for all allocations.
  /// \param sz The size of allocation requested. This might be rounded up to
  ///   fit heap alignment requirements.
  /// \tparam fixedSize If true, the allocation is of a cell type that always
  ///   has the same size. The requirement enforced by Hades is that all
  ///   fixed-size allocations must go into YG.
  /// \tparam hasFinalizer If true, the cell about to be allocated into the
  ///   requested space will have a finalizer that the GC will need to invoke.
  template <bool fixedSize, HasFinalizer hasFinalizer>
  void *allocWork(uint32_t sz);

  /// Same as \c allocLongLived<hasFinalizer> but discards the finalizer
  /// parameter that is unused anyway.
  void *allocLongLived(uint32_t sz);

  /// Frees the weak slot, so it can be re-used by future WeakRef allocations.
  void freeWeakSlot(WeakRefSlot *slot);

  /// Perform a YG garbage collection. All live objects in YG will be evacuated
  /// to the OG.
  /// \param cause The cause of the GC, used for logging.
  /// \param forceOldGenCollection If true, always start an old gen collection
  ///   if one is not already active.
  /// \post The YG is completely empty, and all bytes are available for new
  ///   allocations.
  void youngGenCollection(std::string cause, bool forceOldGenCollection);

  /// In the "no GC before TTI" mode, move the Young Gen heap segment to the
  /// Old Gen without scanning for garbage.
  /// \return true if a promotion occurred, false if it did not.
  bool promoteYoungGenToOldGen();

  /// This function checks if the live bytes after the last OG GC is greater
  /// than the tripwire limit. If the conditions are met, the tripwire is
  /// triggered and tripwireCallback_ is called.
  /// Also resets the stats counter, so that it calls the analytics callback.
  void checkTripwireAndResetStats();

  /// Transfer any external memory charges from YG to OG. Used as part of YG
  /// collection.
  void transferExternalMemoryToOldGen();

  /// Perform an OG garbage collection. All live objects in OG will be left
  /// untouched, all unreachable objects will be placed into a free list that
  /// can be used by \c oldGenAlloc.
  void oldGenCollection(std::string cause);

  /// If there's an OG collection going on, wait for it to complete. This
  /// function is synchronous and will block the caller if the GC background
  /// thread is still running.
  /// \pre The gcMutex_ must be held before entering this function.
  /// \post The gcMutex_ will be held when the function exits, but it might
  ///   have been unlocked and then re-locked.
  void waitForCollectionToFinish();

  /// Worker function that does the bulk of the GC work concurrently with the
  /// mutator.
  void oldGenCollectionWorker();

  /// For 32-bit systems, Hades runs on a single thread, interleaving OG work
  /// with YG collections. This function completes that OG collection.
  /// \pre kConcurrentGC must be false.
  void completeNonConcurrentOldGenCollection();

  /// Finish the marking process. This requires a STW pause in order to do a
  /// final marking worklist drain, and to update weak roots.
  /// \pre Both gcMutex_ and weakRefMutex_ are held before entering.
  void completeMarkingAssumeLocks();

  /// Same as \c completeMarkingAssumeLocks, but acquires necessary locks first.
  void completeMarking();

  /// As part of finishing the marking process, iterate through all of YG to
  /// find symbols and WeakRefs only pointed to from there.
  void findYoungGenSymbolsAndWeakRefs();

  /// Put dead objects onto the free list, so their space can be re-used.
  void sweep();

  /// Find all pointers from OG into YG during a YG collection. This is done
  /// quickly through use of write barriers that detect the creation of OG-to-YG
  /// pointers.
  void scanDirtyCards(EvacAcceptor &acceptor);

  /// Common logic for doing the Snapshot At The Beginning (SATB) write barrier.
  void snapshotWriteBarrierInternal(GCCell *oldValue);

  /// Common logic for doing the Snapshot At The Beginning (SATB) write barrier.
  /// Forwards to \c snapshotWriteBarrierInternal(GCCell *) if oldValue is a
  /// pointer.
  void snapshotWriteBarrierInternal(HermesValue oldValue);

  /// Common logic for doing the generational write barrier for detecting
  /// pointers into YG.
  void generationalWriteBarrier(void *loc, void *value);

  /// Finalize all objects in YG that have finalizers.
  void finalizeYoungGenObjects();

  /// Run the finalizers for all heap objects, if the gcMutex_ is already
  /// locked.
  void finalizeAllLocked();

  /// Update all of the weak references and invalidate the ones that point to
  /// dead objects.
  void updateWeakReferencesForYoungGen();

  /// Update all of the weak references, invalidate the ones that point to
  /// dead objects, and free the ones that were not marked at all.
  void updateWeakReferencesForOldGen();

  /// The WeakMap type in JS has special semantics for handling keys kept alive
  /// by only their values. In between marking and sweeping, this function is
  /// called to handle that special case.
  void completeWeakMapMarking(MarkAcceptor &acceptor);

  /// Sets all weak references to unmarked in preparation for a collection.
  void resetWeakReferences();

  /// Return the total number of bytes that are in use by the JS heap.
  uint64_t allocatedBytes() const;

  /// \return the total number of bytes that are in use by objects on the JS
  ///   heap, but is not in the heap itself.
  uint64_t externalBytes() const;

  /// Accessor for the YG.
  HeapSegment &youngGen();
  const HeapSegment &youngGen() const;

  /// Create a new segment (to be used by either YG or OG).
  std::unique_ptr<HeapSegment> createSegment(bool isYoungGen);

  /// Searches the old gen for this pointer. This is O(number of OG segments).
  /// NOTE: In any non-debug case, \c inYoungGen should be used instead, because
  /// it is O(1).
  /// \return true if the pointer is in the old gen.
  bool inOldGen(const void *p) const;

  /// Give the background GC a chance to complete marking and finish the OG
  /// collection.
  void yieldToOldGen();

  /// \return A number of bytes that should be drained on a per-YG basis to
  ///   help ensure an incremental collection will finish before the next one is
  ///   needed.
  size_t getDrainRate();

  /// Adds the start address of the segment to the CrashManager's custom data.
  /// \param extraName append this to the name of the segment. Must be
  ///   non-empty.
  void addSegmentExtentToCrashManager(
      const HeapSegment &seg,
      std::string extraName);

  /// Unlocks any held mutexes, then aborts the program.
  /// Should only be called by the GC, OOM from outside the GC can skip the
  /// unlocks.
  LLVM_ATTRIBUTE_NORETURN void oomInternal(std::error_code reason);

#ifdef HERMES_SLOW_DEBUG
  /// Checks the heap to make sure all cells are valid.
  void checkWellFormed();

  /// Verify that the card table used to find pointers from OG into YG has the
  /// correct cards dirtied, given the contents of the OG currently.
  void verifyCardTable();
#endif
};

/// \name Inline implementations
/// \{

template <bool fixedSize, HasFinalizer hasFinalizer>
void *HadesGC::alloc(uint32_t sz) {
  return allocWork<fixedSize, hasFinalizer>(heapAlignSize(sz));
}

template <HasFinalizer hasFinalizer>
void *HadesGC::allocLongLived(uint32_t sz) {
  return allocLongLived(sz);
}

/// \}

} // namespace vm
} // namespace hermes
#endif
