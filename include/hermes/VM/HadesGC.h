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
#include "hermes/Support/Algorithms.h"
#include "hermes/Support/SlowAssert.h"
#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/GCBase.h"
#include "hermes/VM/VMExperiments.h"

#include "llvh/ADT/SparseBitVector.h"
#include "llvh/Support/ErrorOr.h"
#include "llvh/Support/PointerLikeTypeTraits.h"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <future>
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
      std::shared_ptr<StorageProvider> provider,
      experiments::VMExperimentFlags vmExperimentFlags);

  ~HadesGC();

  static bool classof(const GCBase *gc) {
    return gc->getKind() == HeapKind::HADES;
  }

  static constexpr uint32_t maxAllocationSize() {
    // The largest allocation allowable in Hades is the max size a single
    // segment supports.
    return HeapSegment::maxSize();
  }

  static constexpr uint32_t minAllocationSize() {
    return heapAlignSize(
        max(sizeof(OldGen::FreelistCell), sizeof(CopyListCell)));
  }

  /// \name GCBase overrides
  /// \{

  void getHeapInfo(HeapInfo &info) override;
  void getHeapInfoWithMallocSize(HeapInfo &info) override;
  void getCrashManagerHeapInfo(CrashManager::HeapInformation &info) override;
  void createSnapshot(llvh::raw_ostream &os) override;
  void snapshotAddGCNativeNodes(HeapSnapshot &snap) override;
  void snapshotAddGCNativeEdges(HeapSnapshot &snap) override;
  void enableHeapProfiler(
      std::function<void(
          uint64_t,
          std::chrono::microseconds,
          std::vector<GCBase::AllocationLocationTracker::HeapStatsUpdate>)>
          fragmentCallback) override;
  void disableHeapProfiler() override;
  void enableSamplingHeapProfiler(size_t samplingInterval, int64_t seed)
      override;
  void disableSamplingHeapProfiler(llvh::raw_ostream &os) override;
  void printStats(JSONEmitter &json) override;
  std::string getKindAsStr() const override;

  /// \}

  /// \name GC non-virtual API
  /// \{

  /// Allocate a new cell of the specified size \p size by calling alloc.
  /// Instantiate an object of type T with constructor arguments \p args in the
  /// newly allocated cell.
  /// \return a pointer to the newly created object in the GC heap.
  template <
      typename T,
      bool fixedSize = true,
      HasFinalizer hasFinalizer = HasFinalizer::No,
      LongLived longLived = LongLived::No,
      class... Args>
  inline T *makeA(uint32_t size, Args &&...args);

  /// Force a garbage collection cycle.
  /// (Part of general GC API defined in GCBase.h).
  void collect(std::string cause, bool canEffectiveOOM = false) override;

  /// Run the finalizers for all heap objects.
  void finalizeAll() override;

  /// Add some external memory cost to a cell.
  /// (Part of general GC API defined in GCBase.h).
  /// \pre canAllocExternalMemory(size) is true.
  void creditExternalMemory(GCCell *alloc, uint32_t size) override;

  /// Remove some external memory cost from a cell.
  /// (Part of general GC API defined in GCBase.h).
  void debitExternalMemory(GCCell *alloc, uint32_t size) override;

  /// \name Write Barriers
  /// \{

  /// NOTE: For all write barriers and read barriers:
  /// The call to writeBarrier/readBarrier must happen *before* the write/read
  /// to memory occurs.

  /// The given value is being written at the given loc (required to
  /// be in the heap). If value is a pointer, execute a write barrier.
  /// NOTE: The write barrier call must be placed *before* the write to the
  /// pointer, so that the current value can be fetched.
  void writeBarrier(const GCHermesValue *loc, HermesValue value);

  /// The given pointer value is being written at the given loc (required to
  /// be in the heap). The value may be null. Execute a write barrier.
  /// NOTE: The write barrier call must be placed *before* the write to the
  /// pointer, so that the current value can be fetched.
  void writeBarrier(const GCPointerBase *loc, const GCCell *value);

  /// The given symbol is being written at the given loc (required to be in the
  /// heap).
  void writeBarrier(SymbolID symbol);

  /// Special versions of \p writeBarrier for when there was no previous value
  /// initialized into the space.
  void constructorWriteBarrier(const GCHermesValue *loc, HermesValue value);
  void constructorWriteBarrier(const GCPointerBase *loc, const GCCell *value);

  void snapshotWriteBarrier(const GCHermesValue *loc);
  void snapshotWriteBarrier(const GCPointerBase *loc);
  void snapshotWriteBarrierRange(const GCHermesValue *start, uint32_t numHVs);

  void weakRefReadBarrier(GCCell *value);
  void weakRefReadBarrier(HermesValue value);

  /// \}

  /// Returns whether an external allocation of the given \p size fits
  /// within the maximum heap size. (Note that this does not guarantee that the
  /// allocation will "succeed" -- the size plus the used() of the heap may
  /// still exceed the max heap size. But if it fails, the allocation can never
  /// succeed.)
  bool canAllocExternalMemory(uint32_t size) override;

  WeakRefSlot *allocWeakSlot(HermesValue init) override;

  /// Iterate over all objects in the heap, and call \p callback on them.
  /// \param callback A function to call on each found object.
  void forAllObjs(const std::function<void(GCCell *)> &callback) override;

  /// Inform the GC that TTI has been reached. This will transition the GC mode,
  /// if the GC was currently allocating directly into OG.
  void ttiReached() override;

  /// \}

  /// \return true if the pointer lives in the young generation.
  bool inYoungGen(const void *p) const override;

  /// Approximate the dirty memory footprint of the GC's heap. Note that this
  /// does not return the number of dirty pages in the heap, but instead returns
  /// a number that goes up if pages are dirtied, and goes down if pages are
  /// cleaned.
  llvh::ErrorOr<size_t> getVMFootprintForTest() const;

#ifndef NDEBUG
  /// \name Debug APIs
  /// \{

  bool calledByBackgroundThread() const override;

  /// See comment in GCBase.
  bool calledByGC() const override {
    // Check if this is called by the background thread or the inGC flag is
    // set.
    return calledByBackgroundThread() || inGC();
  }

  /// Return true if \p ptr is currently pointing at valid accessable memory,
  /// allocated to an object.
  bool validPointer(const void *ptr) const override;

  /// Return true if \p ptr is within one of the virtual address ranges
  /// allocated for the heap. Not intended for use in normal production GC
  /// operation, debug mode only.
  bool dbgContains(const void *ptr) const override;

  /// Record that a cell of the given \p kind and size \p sz has been
  /// found reachable in a full GC.
  void trackReachable(CellKind kind, unsigned sz) override;

  /// Returns true if \p cell is the most-recently allocated finalizable object.
  bool isMostRecentFinalizableObj(const GCCell *cell) const override;

  /// \}
#endif

  class CollectionStats;
  class HeapMarkingAcceptor;
  template <bool CompactionEnabled>
  class EvacAcceptor;
  class MarkAcceptor;
  class MarkWeakRootsAcceptor;
  class OldGen;
  class Executor;

  struct CopyListCell final : public GCCell {
    // Linked list of cells pointing to the next cell that was copied.
    CopyListCell *next_;
    // If the cell was trimmed, this field will have the original size of the
    // object stored. If the cell wasn't trimmed it'll have the same size as the
    // forwarded pointer.
    uint32_t originalSize_;
  };

  /// Similar to AlignedHeapSegment except it uses a free list.
  class HeapSegment final : public AlignedHeapSegment {
   public:
    explicit HeapSegment(AlignedStorage storage);
    HeapSegment() = default;

    /// Allocate space by bumping a level.
    AllocResult bumpAlloc(uint32_t sz);

    /// Record the head of this cell so it can be found by the card scanner.
    static void setCellHead(const GCCell *start, const size_t sz);

    /// Find the head of the first cell that extends into the card at index
    /// \p cardIdx.
    /// \return A cell such that
    /// cell <= indexToAddress(cardIdx) < cell->nextCell().
    GCCell *getFirstCellHead(const size_t cardIdx);

    /// Call \p callback on every non-freelist cell allocated in this segment.
    template <typename CallbackFunction>
    void forAllObjs(CallbackFunction callback);
    /// Only call the callback on cells without forwarding pointers.
    template <typename CallbackFunction>
    void forCompactedObjs(CallbackFunction callback);
  };

  class OldGen final {
   public:
    explicit OldGen(HadesGC *gc);

    std::deque<HeapSegment>::iterator begin();
    std::deque<HeapSegment>::iterator end();
    std::deque<HeapSegment>::const_iterator begin() const;
    std::deque<HeapSegment>::const_iterator end() const;

    size_t numSegments() const;
    size_t maxNumSegments() const;

    HeapSegment &operator[](size_t i);

    /// Take ownership of the given segment.
    void addSegment(HeapSegment seg);

    /// Remove the segment at \p segmentIdx.
    /// WARN: This is an expensive operation and should be used sparingly. It
    /// calls eraseSegmentFreelists, which has a worst case time complexity of
    /// O(#Freelist buckets * #Segments).
    /// \return the segment previously at segmentIdx
    HeapSegment removeSegment(size_t segmentIdx);

    /// Indicate that OG should target having \p targetSegments segments.
    void setTargetSegments(size_t targetSegments);

    /// Allocate into OG. Returns a pointer to the newly allocated space. That
    /// space must be filled before releasing the gcMutex_.
    /// \return A non-null pointer to memory in the old gen that should have a
    ///   constructor run in immediately.
    /// \pre gcMutex_ must be held before calling this function.
    /// \post This function either successfully allocates, or reports OOM.
    GCCell *alloc(uint32_t sz);

    /// Adds the given region of memory to the free list for this segment.
    void addCellToFreelist(void *addr, uint32_t sz, size_t segmentIdx);

    /// \return the total number of bytes that are in use by the OG section of
    /// the JS heap, excluding free list entries.
    uint64_t allocatedBytes() const;

    /// \return the total number of bytes that are in use by the segment with
    /// index \p segmentIdx in the OG section of the JS heap, excluding free
    /// list entries.
    uint64_t allocatedBytes(uint16_t segmentIdx) const;

    /// Increase the allocated bytes tracker for the segment at index \p
    /// segmentIdx;
    void incrementAllocatedBytes(int32_t incr, uint16_t segmentIdx);

    /// Get the peak allocated bytes for the segment at \p segmentIdx.
    uint64_t peakAllocatedBytes(uint16_t segmentIdx) const;

    /// Trigger an update of the peak allocated bytes. This should be done right
    /// before sweeping a particular segment so we have the true peak.
    void updatePeakAllocatedBytes(uint16_t segmentIdx);

    /// \return the total number of bytes that are held in external memory, kept
    /// alive by objects in the OG.
    uint64_t externalBytes() const;

    /// \return the total number of bytes that are in use by the OG section of
    /// the JS heap, including free list entries.
    uint64_t size() const;

    /// \return the total number of bytes that we aim to use in the OG
    /// section of the JS heap, including free list entries. This may be smaller
    /// or greater than size().
    uint64_t targetSizeBytes() const;

    /// Add some external memory cost to the OG.
    void creditExternalMemory(uint32_t size) {
      assert(
          gc_->gcMutex_ &&
          "OG external bytes must be accessed under gcMutex_.");
      externalBytes_ += size;
    }

    /// Remove some external memory cost from the OG.
    void debitExternalMemory(uint32_t size) {
      assert(
          externalBytes_ >= size && "Debiting more memory than was credited");
      assert(
          gc_->gcMutex_ &&
          "OG external bytes must be accessed under gcMutex_.");
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
    void addCellToFreelist(FreelistCell *cell, size_t segmentIdx);

    /// Remove the cell pointed to by the pointer at \p prevLoc from
    /// the given \p segmentIdx and \p bucket in the freelist.
    /// \return a pointer to the removed cell.
    FreelistCell *removeCellFromFreelist(
        FreelistCell **prevLoc,
        size_t bucket,
        size_t segmentIdx);

    /// Remove the first cell from the given \p segmentIdx and \p bucket in the
    /// freelist.
    /// \return a pointer to the removed cell.
    FreelistCell *removeCellFromFreelist(size_t bucket, size_t segmentIdx);

    /// Unset all the bits for a given segment's freelist, so that no new
    /// allocations take place in it.
    void clearFreelistForSegment(size_t segmentIdx);

    /// Remove a segment entirely from every freelist. This will shift all bits
    /// after segmentIdx down by one.
    void eraseSegmentFreelists(size_t segmentIdx);

    /// Sweep the next segment and advance the internal sweep iterator. If there
    /// are no more segments left to sweep, update OG collection stats with
    /// numbers from the sweep.
    bool sweepNext();

    /// Initialize the internal sweep iterator. This will reset the internal
    /// sweep stats to 0, and set the sweep iterator to the last segment in the
    /// OG. The sweep iterator then works backwards from there, to avoid
    /// sweeping newly added segments.
    void initializeSweep();

    /// Number of segments left to sweep. Useful for determining how many YG
    /// collections it will take to complete an incremental OG collection.
    size_t sweepSegmentsRemaining() const;

    /// \return The number of bytes of native memory in use by this OldGen.
    size_t getMemorySize() const;

   private:
    /// \return the index of the bucket in freelistBuckets_ corresponding to
    /// \p size.
    /// \post The returned index is less than kNumFreelistBuckets.
    static uint32_t getFreelistBucket(uint32_t size);

    /// Adds the given region of memory to the free list for the segment that is
    /// currently being swept. This does not update the Freelist bits, those
    /// should all be updated in a single pass at the end of sweeping.
    void addCellToFreelistFromSweep(
        char *freeRangeStart,
        char *freeRangeEnd,
        bool setHead);

    HadesGC *gc_;

    /// Use a std::deque instead of a std::vector so that references into it
    /// remain valid across a push_back.
    std::deque<HeapSegment> segments_;

    /// This is the target number of segments in the OG JS heap. It does not
    /// include external memory and may be larger or smaller than the actual
    /// number of segments allocated.
    size_t targetSegments_{0};

    /// This is the sum of all bytes currently allocated in the heap, excluding
    /// bump-allocated segments. Use \c allocatedBytes() to include
    /// bump-allocated segments.
    uint64_t allocatedBytes_{0};

    /// Each element in the vector corresponds to the segment at the same index
    /// in segments_. The first element in the pair represents the currently
    /// allocated bytes in this segment, the second element represents the peak
    /// allocated bytes in this segment, since it was created or compacted.
    std::vector<std::pair<uint32_t, uint32_t>> segmentAllocatedBytes_;

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

    /// The below data structures should be used as follows.
    /// 1. When looking for a given size, first find the smallest appropriate
    ///    bucket that has any elements at all, by scanning
    ///    freelistBucketBitArray_.
    /// 2. Once a bucket is identified, find a segment that contains cells for
    ///    that bucket using freelistBucketSegmentBitArray_.
    /// 3. Finally, with the given pair of bucket and segment index, obtain the
    ///    head of the freelist from freelistSegmentsBuckets_.

    /// Maintain a freelist as described above for every segment. Each element
    /// is the head of a freelist for the given segment + bucket pair.
    std::vector<std::array<FreelistCell *, kNumFreelistBuckets>>
        freelistSegmentsBuckets_;

    /// Keep track of which freelist buckets have valid elements to make search
    /// fast. This includes all segments.
    BitArray<kNumFreelistBuckets> freelistBucketBitArray_;

    /// Keep track of which segments have a valid element in a particular
    /// bucket. Combined with the above bit array, this allows us to quickly
    /// index into a freelist bucket.
    std::array<llvh::SparseBitVector<>, kNumFreelistBuckets>
        freelistBucketSegmentBitArray_;

    /// Tracks the current progress of sweeping.
    struct SweepIterator {
      /// The current segment being swept, this should start at the end and move
      /// to the front of the segment list, to avoid sweeping newly added
      /// segments.
      size_t segNumber{0};

      /// The total number of GC-managed and external bytes swept in the current
      /// sweep.
      uint64_t sweptBytes{0};
      uint64_t sweptExternalBytes{0};
    } sweepIterator_;

    /// Searches the OG for a space to allocate memory into.
    /// \return A pointer to uninitialized memory that can be written into, null
    ///   if no such space exists.
    GCCell *search(uint32_t sz);

    /// Common path for when an allocation has succeeded.
    /// \param cell The free memory that will soon have an object allocated into
    ///   it.
    /// \param sz The number of bytes associated with the free memory.
    /// \param segmentIdx An index into segments_ representing which segment the
    /// allocation is being made in.
    GCCell *finishAlloc(GCCell *cell, uint32_t sz, uint16_t segmentIdx);
  };

 private:
  /// The maximum number of bytes that the heap can hold. Once this amount has
  /// been filled up, OOM will occur.
  const uint64_t maxHeapSize_;

  /// This needs to be placed before youngGen_ and oldGen_, because those
  /// members use numSegments_ as part of being constructed.
  uint64_t numSegments_{0};

  /// Stores previously allocated segment indices that have since
  /// been freed. We can reuse them when another segment is allocated.
  std::vector<size_t> segmentIndices_;

  /// Keeps the storage provider alive until after the GC is fully destructed.
  std::shared_ptr<StorageProvider> provider_;

  /// youngGen is a bump-pointer space, so it can re-use AlignedHeapSegment.
  /// Protected by gcMutex_.
  HeapSegment youngGen_;

  /// List of cells in YG that have finalizers. Iterate through this to clean
  /// them out.
  /// Protected by gcMutex_.
  std::vector<GCCell *> youngGenFinalizables_;

  /// oldGen_ is a free list space, so it needs a different segment
  /// representation.
  /// Protected by gcMutex_.
  OldGen oldGen_;

  /// Whoever holds this lock is permitted to modify data structures around the
  /// GC. This includes mark bits, free lists, etc.
  Mutex gcMutex_;

  enum class Phase : uint8_t {
    None,
    Mark,
    CompleteMarking,
    Sweep,
  };

  /// Represents the current phase the concurrent GC is in. The main difference
  /// between phases is their effect on read and write barriers. Should only be
  /// accessed if gcMutex_ is acquired.
  Phase concurrentPhase_{Phase::None};

  /// Represents whether the background thread is currently marking. Should only
  /// be accessed by the mutator thread or during a STW pause.
  /// ogMarkingBarriers_ is true from the start of marking the OG heap until the
  /// start of WeakMap marking but is kept separate from concurrentPhase_ in
  /// order to reduce synchronisation requirements for write barriers.
  bool ogMarkingBarriers_{false};

  /// Used by the write barrier to add items to the worklist.
  /// Protected by gcMutex_.
  std::unique_ptr<MarkAcceptor> oldGenMarker_;

  /// This provides the background thread for doing marking and sweeping
  /// concurrently with the mutator.
  std::unique_ptr<Executor> backgroundExecutor_;

  /// This tracks the current status of execution in the background thread. The
  /// future should be set every time work is enqueued onto the executor. After
  /// that, whenever we need to wait for execution in the background thread to
  /// end, we can call get() on this future.
  std::future<void> ogThreadStatus_;

#ifndef NDEBUG
  /// True from the time the background task is created, to the time it exits
  /// the collection loop. False otherwise.
  bool backgroundTaskActive_{false};
#endif

  /// If true, whenever YG fills up immediately put it into the OG.
  bool promoteYGToOG_;

  /// If true, turn off promoteYGToOG_ as soon as the first OG GC occurs.
  bool revertToYGAtTTI_;

  /// Target OG occupancy ratio at the end of an OG collection.
  const double occupancyTarget_;

  /// A collection section used to track the size of YG before and after a YG
  /// collection, as well as the time a YG collection takes.
  std::unique_ptr<CollectionStats> ygCollectionStats_;

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

  struct CompacteeState {
    /// \return true if the pointer lives in the segment that is being marked or
    /// evacuated for compaction.
    bool contains(const void *p) const {
      return start == AlignedStorage::start(p);
    }

    /// \return true if the pointer lives in the segment that is currently being
    /// evacuated for compaction.
    bool evacContains(const void *p) const {
      return evacStart == AlignedStorage::start(p);
    }

    /// \return true if the compactee is ready to be evacuated.
    bool evacActive() const {
      return evacStart != reinterpret_cast<void *>(kInvalidCompacteeStart);
    }

#ifndef NDEBUG
    /// \return true if the compactee has not been assigned.
    bool empty() const {
      void *const invalid = reinterpret_cast<void *>(kInvalidCompacteeStart);
      return evacStart == invalid && start == invalid && !segment;
    }
#endif

    /// The following variables track the state of compactions.
    /// 1. To trigger a compaction, segment and start should be set at the
    /// beginning of marking. This ensures that all cards containing pointers to
    /// the compactee will be dirtied.
    /// 2. Once marking is done, completeMarking should then set evacStart, so
    /// that the next YG collection will evacuate the segment.
    /// 3. On completion, the YG collection will reset all these variables.

    /// In order to keep the "contains" check cheap, this can be any non-null
    /// value that cannot correspond to the start of a segment.
    static constexpr uintptr_t kInvalidCompacteeStart = 0x1;

    /// The start address of the segment that will be compacted next. This is
    /// used during marking and by write barriers to determine whether a pointer
    /// is in the compactee segment.
    void *start{reinterpret_cast<void *>(kInvalidCompacteeStart)};

    /// The start address of the segment that is currently being compacted. When
    /// this is set, the next YG will evacuate objects in this segment. This is
    /// always going to be equal to "start" or nullptr.
    void *evacStart{reinterpret_cast<void *>(kInvalidCompacteeStart)};

    /// The segment being compacted. This should be removed from the OG right
    /// after it is identified, and freed entirely once the compaction is
    /// complete.
    std::shared_ptr<HeapSegment> segment;

    /// The number of bytes in the compactee, should be set before the compactee
    /// is removed from the OG.
    uint32_t allocatedBytes{0};
  } compactee_;

  /// If compaction completes before sweeping, there is a possibility that
  /// dangling pointers into the now freed compactee may remain in the OG heap
  /// until sweeping finishes. In certain cases, like when scanning dirty cards,
  /// this could cause a segfault if you attempt to say, compress a pointer. To
  /// handle this case, if compaction completes while sweeping is still in
  /// progress, this shared_ptr will keep the compactee segment alive until the
  /// end of sweeping.
  std::shared_ptr<HeapSegment> compacteeHandleForSweep_;

  struct NativeIDs {
    HeapSnapshot::NodeID ygFinalizables{IDTracker::kInvalidNode};
    HeapSnapshot::NodeID og{IDTracker::kInvalidNode};
  } nativeIDs_;

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

  /// Like alloc, but the resulting object is expected to be long-lived.
  /// Allocate directly in the old generation (doing a full collection if
  /// necessary to create room).
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

  template <typename Acceptor>
  void youngGenEvacuateImpl(Acceptor &acceptor, bool doCompaction);

  /// In the "no GC before TTI" mode, move the Young Gen heap segment to the
  /// Old Gen without scanning for garbage.
  /// \return true if a promotion occurred, false if it did not.
  bool promoteYoungGenToOldGen();

  /// This function checks if the live bytes after the last OG GC is greater
  /// than the tripwire limit. If the conditions are met, the tripwire is
  /// triggered and tripwireCallback_ is called.
  /// Also resets the stats counter, so that it calls the analytics callback.
  /// WARNING: Do not call this while there is an ongoing collection. It can
  /// cause a race condition and a deadlock.
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
  void waitForCollectionToFinish(std::string cause);

  /// Worker function that schedules the bulk of the GC work on the background
  /// thread, to perform it concurrently with the mutator.
  void collectOGInBackground();

  /// Perform a single step of an OG collection. \p backgroundThread indicates
  /// whether this call was made from the background thread.
  void incrementalCollect(bool backgroundThread);

  /// Finish the marking process. This requires a STW pause in order to do a
  /// final marking worklist drain, and to update weak roots. It must be invoked
  /// from the mutator.
  void completeMarking();

  /// Select a segment to compact and initialise any state needed for
  /// compaction.
  void prepareCompactee();

  /// Search a single segment for pointers that may need to be updated as the
  /// YG/compactee are evacuated.
  template <bool CompactionEnabled>
  void scanDirtyCardsForSegment(
      SlotVisitor<EvacAcceptor<CompactionEnabled>> &visitor,
      HeapSegment &segment);

  /// Find all pointers from OG into the YG/compactee during a YG collection.
  /// This is done quickly through use of write barriers that detect the
  /// creation of such pointers.
  template <bool CompactionEnabled>
  void scanDirtyCards(EvacAcceptor<CompactionEnabled> &acceptor);

  /// Common logic for doing the Snapshot At The Beginning (SATB) write barrier.
  void snapshotWriteBarrierInternal(GCCell *oldValue);

  /// Common logic for doing the Snapshot At The Beginning (SATB) write barrier.
  /// Forwards to \c snapshotWriteBarrierInternal(GCCell*) if oldValue is a
  /// pointer. Forwards to \c snapshotWriteBarrierInternal(SymbolID) is oldValue
  /// is a symbol.
  void snapshotWriteBarrierInternal(HermesValue oldValue);

  /// Performs a Snapshot At The Beginning (SATB) write barrier for a symbol,
  /// which assumes the old symbol was reachable at the start of the collection.
  void snapshotWriteBarrierInternal(SymbolID symbol);

  /// Common logic for doing the relocation write barrier for detecting
  /// pointers into YG and for tracking newly created pointers into the
  /// compactee.
  void relocationWriteBarrier(const void *loc, const void *value);

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

  /// \return the total number of bytes used by the heap, including segment
  /// metadata and external memory.
  uint64_t heapFootprint() const;

  /// \return the remaining available space we are allowed to use. That is, the
  /// difference between maxHeapSize_ and heapFootprint.
  uint64_t remainingBytes() const;

  /// Accessor for the YG.
  HeapSegment &youngGen();
  const HeapSegment &youngGen() const;

  /// Create a new segment (to be used by either YG or OG).
  llvh::ErrorOr<HeapSegment> createSegment();

  /// Set a given segment as the YG segment.
  /// \return the previous YG segment.
  HeapSegment setYoungGen(HeapSegment seg);

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
      const std::string &extraName);

  /// Deletes a segment from the CrashManager's custom data.
  /// \param extraName that was used to initially add this segment to the crash
  /// manager.
  void removeSegmentExtentFromCrashManager(const std::string &extraName);

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

template <
    typename T,
    bool fixedSize,
    HasFinalizer hasFinalizer,
    LongLived longLived,
    class... Args>
inline T *HadesGC::makeA(uint32_t size, Args &&...args) {
  assert(
      isSizeHeapAligned(size) &&
      "Call to makeA must use a size aligned to HeapAlign");
  if (longLived == LongLived::Yes) {
    std::lock_guard<Mutex> lk{gcMutex_};
    return new (allocLongLived(size)) T(std::forward<Args>(args)...);
  }

  return new (allocWork<fixedSize, hasFinalizer>(size))
      T(std::forward<Args>(args)...);
}

/// \}

} // namespace vm
} // namespace hermes
#endif
