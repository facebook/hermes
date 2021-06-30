/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GENGC_H
#define HERMES_VM_GENGC_H

#include "hermes/Public/GCConfig.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/Support/OptValue.h"
#include "hermes/Support/PerfSection.h"
#include "hermes/Support/SlowAssert.h"
#include "hermes/VM/AlignedStorage.h"
#include "hermes/VM/AllocResult.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/CompleteMarkState.h"
#include "hermes/VM/GCBase.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/GCSegmentAddressIndex.h"
#include "hermes/VM/GenGCHeapSegment.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/OldGenNC.h"
#include "hermes/VM/SweepResultNC.h"
#include "hermes/VM/VMExperiments.h"
#include "hermes/VM/YoungGenNC.h"
#include "llvh/Support/Casting.h"
#include "llvh/Support/Compiler.h"
#include "llvh/Support/ErrorHandling.h"

#include <deque>
#include <limits>
#include <vector>

// We assume this file is only included by GC.h, which declares GCBase.

namespace hermes {
namespace vm {

// External forward declarations.
class WeakRefBase;
template <class T>
class WeakRef;

/// A simple two-generation GC.
///
/// The young gen is a single contiguous-allocation space.  A
/// young-gen collection completely evacuates live objects into the
/// older generation.
///
/// The old generation is also a single contiguous-allocation space,
/// but it is collected using mark-sweep-compact.  (Actually, a full
/// collection collects both generations.)
class GenGC final : public GCBase {
 public:
  class Size final {
   public:
    explicit Size(const GCConfig &gcConfig);
    Size(gcheapsize_t min, gcheapsize_t max);

    gcheapsize_t min() const {
      return ygs_.min() + ogs_.min();
    }

    gcheapsize_t max() const {
      return ygs_.max() + ogs_.max();
    }

    /// \return The maximum number of bytes ever needed in storage.
    gcheapsize_t storageFootprint() const;
    gcheapsize_t minStorageFootprint() const;

   private:
    // Expose private functions to GenGC, but not the rest of the world.
    friend class GenGC;

    YoungGen::Size ygs_;
    OldGen::Size ogs_;

    YoungGen::Size youngGenSize() const {
      return ygs_;
    }

    OldGen::Size oldGenSize() const {
      return ogs_;
    }

    std::pair<gcheapsize_t, gcheapsize_t> adjustSize(
        gcheapsize_t desired) const;

    /// If a > b, returns a - b, else 0.  Used to make sure we don't overflow
    /// when we subtract unsigned heap sizes in some situations.
    static constexpr gcheapsize_t clampDiffNonNeg(
        gcheapsize_t a,
        gcheapsize_t b) {
      return a > b ? a - b : 0;
    }

    /// We aim to keep the young gen 1 / kYoungGenFractionDenom of the total
    /// heap size, subject to its own bounding and alignment requirements.
    static constexpr unsigned kYoungGenFractionDenom = 8;
  };

  /// Initialize the GC with the give \p gcCallbacks and \p gcConfig.
  /// maximum size.
  /// \param gcCallbacks A callback interface enabling the garbage collector to
  ///   mark roots and free symbols.
  /// \param gcConfig A struct giving, e.g., minimum, initial, and maximum heap
  /// sizes.
  /// \param provider A provider of storage to be used by segments.
  GenGC(
      GCCallbacks *gcCallbacks,
      PointerBase *pointerBase,
      const GCConfig &gcConfig,
      std::shared_ptr<CrashManager> crashMgr,
      std::shared_ptr<StorageProvider> provider,
      experiments::VMExperimentFlags vmExperimentFlags);

  ~GenGC();

  static bool classof(const GCBase *gc) {
    return gc->getKind() == HeapKind::NCGEN;
  }

#ifndef NDEBUG
  /// Allocation path we use in debug builds, where we potentially follow
  /// different code paths to test things.
  AllocResult
  debugAlloc(uint32_t sz, HasFinalizer hasFinalizer, bool fixedSize);

  /// In debugAlloc, if shouldRandomizeAllocSpace() is true, we call this to
  /// alternate the generation we allocate into.
  AllocResult
  debugAllocRandomize(uint32_t sz, HasFinalizer hasFinalizer, bool fixedSize);
#endif

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

  /// Returns whether an external allocation of the given \p size fits
  /// within the maximum heap size.  (Note that this does not guarantee that the
  /// allocation will "succeed" -- the size plus the used() of the heap may
  /// still exceed the max heap size.  But if it fails, the allocation can never
  /// succeed.)
  bool canAllocExternalMemory(uint32_t size) override;

  /// An allocation that yielded the given \p alloc has associated external
  /// memory of the given \p size.  Add that to the appropriate external memory
  /// charge.
  void creditExternalMemory(GCCell *alloc, uint32_t size) override;

  /// The object at \p alloc is being collected, and has associated external
  /// memory of the given \p size.  Decrease the external memory charge of the
  /// generation owning \p alloc by this amount.
  void debitExternalMemory(GCCell *alloc, uint32_t size) override;

  /// Write barriers.

  /// The given value is being written at the given loc (required to
  /// be in the heap).  If value is a pointer, execute a write barrier.
  void writeBarrier(const GCHermesValue *loc, HermesValue value);
  void writeBarrier(const GCSmallHermesValue *loc, SmallHermesValue value);

  /// The given pointer value is being written at the given loc (required to
  /// be in the heap).  The value is may be null.  Execute a write barrier.
  void writeBarrier(const GCPointerBase *loc, const GCCell *value);

  /// Write barriers for symbols are no-ops in GenGC.
  void writeBarrier(SymbolID) {}

  /// The given value is being written at the given loc (required to
  /// be in the heap).  If value is a pointer, execute a write barrier.
  /// The memory pointed to by \p loc is guaranteed to not have a valid pointer.
  void constructorWriteBarrier(const GCHermesValue *loc, HermesValue value) {
    // There's no difference for GenGC between the constructor and an
    // assignment.
    writeBarrier(loc, value);
  }
  void constructorWriteBarrier(
      const GCSmallHermesValue *loc,
      SmallHermesValue value) {
    // There's no difference for GenGC between the constructor and an
    // assignment.
    writeBarrier(loc, value);
  }

  /// The given pointer value is being written at the given loc (required to
  /// be in the heap).  The value is may be null.  Execute a write barrier.
  /// The memory pointed to by \p loc is guaranteed to not have a valid pointer.
  void constructorWriteBarrier(const GCPointerBase *loc, const GCCell *value) {
    // There's no difference for GenGC between the constructor and an
    // assignment.
    writeBarrier(loc, value);
  }

#ifndef NDEBUG
  /// Count the number of barriers executed.
  /// numWriteBarriers_[false] counts void* write barriers,
  /// numWriteBarriers_[true] the HermesValue barrier.  The second
  /// index indicates the number of write barrier calls of the kind
  /// indicated by the first index that get past a "filter level"
  /// indicated by the second index.  The HermesValue case has a test
  /// not done for the void* case; this is index 1, and is unused in
  /// the void* case.  We introduce symbolic names for those indices
  /// below (they must be defined in non-debug builds).
  static constexpr unsigned kNumFilterLevels = 4;
  /// Note; the initializer of this array initializes all elements to zero.
  uint64_t numWriteBarriers_[2][kNumFilterLevels] = {{0}, { 0 }};
  uint64_t numRangeBarriers_{0};
  uint64_t numRangeFillBarriers_{0};

  /// Returns whether a write of the given value into the given location
  /// requires a write barrier, assuming \p loc and \p value are both within the
  /// heap managed by this GC.
  bool needsWriteBarrier(void *loc, GCCell *value) override;

  /// Statistics related to external memory associated with heap objects:
  ///   The number of objects with associated external memory.
  uint64_t numExtAllocs_{0};
  ///   The current amount of allocated external memory.
  uint64_t curExtAllocBytes_{0};
  ///   The maximum amount of external memory allocated at any time in the
  ///   execution.
  uint64_t maxExtAllocBytes_{0};
  ///   The total amount of external memory allocated during the execution.
  uint64_t totalExtAllocBytes_{0};

  void printExtAllocStats(llvh::raw_ostream &os);
#endif

  /// Symbolic constants for indexing numWriteBarriers_.
  static constexpr unsigned kNumWriteBarrierTotalCountIdx = 0;
  static constexpr unsigned kNumWriteBarrierOfObjectPtrIdx = 1;
  static constexpr unsigned kNumWriteBarrierWithDifferentSegsIdx = 2;
  static constexpr unsigned kNumWriteBarrierPtrInYGIdx = 3;

  /// We copied HermesValues into the given region.  Note that \p numHVs is
  /// the number of HermesValues in the the range, not the char length.
  /// Do any necessary barriers.
  ///
  /// \pre The range described must be wholly contained within one segment of
  ///     the heap.
  void writeBarrierRange(const GCHermesValue *start, uint32_t numHVs);
  void writeBarrierRange(const GCSmallHermesValue *start, uint32_t numHVs);

  /// Same as \p writeBarrierRange, except this is for previously uninitialized
  /// memory.
  void constructorWriteBarrierRange(
      const GCHermesValue *start,
      uint32_t numHVs) {
    // GenGC doesn't do anything special for uninitialized memory.
    writeBarrierRange(start, numHVs);
  }
  void constructorWriteBarrierRange(
      const GCSmallHermesValue *start,
      uint32_t numHVs) {
    // GenGC doesn't do anything special for uninitialized memory.
    writeBarrierRange(start, numHVs);
  }

  void snapshotWriteBarrier(const GCHermesValue *loc) {}
  void snapshotWriteBarrier(const GCSmallHermesValue *loc) {}
  void snapshotWriteBarrier(const GCPointerBase *loc) {}
  void snapshotWriteBarrierRange(const GCHermesValue *start, uint32_t numHVs) {}
  void snapshotWriteBarrierRange(
      const GCSmallHermesValue *start,
      uint32_t numHVs) {}
  void weakRefReadBarrier(GCCell *value) {}
  void weakRefReadBarrier(HermesValue value) {}

  /// Inform the GC that TTI has been reached.
  void ttiReached() override;

  void collect(std::string cause, bool canEffectiveOOM = false) override;

  static constexpr uint32_t minAllocationSizeImpl() {
    // NCGen doesn't enforce a minimum allocation requirement.
    return 0;
  }

  static constexpr uint32_t maxAllocationSizeImpl() {
    // The largest allocation allowable in NCGen is the max size a single
    // segment supports.
    return GenGCHeapSegment::maxSize();
  }

  /// The occupancy target guides heap sizing -- the fraction of the heap
  /// that is intended to be occupied by live data.
  double occupancyTarget() const {
    return occupancyTarget_;
  }

  /// Run the finalizers for all heap objects.
  void finalizeAll() override;

#ifndef NDEBUG

  /// See comment in GCBase.
  bool calledByGC() const override {
    return inGC();
  }

  /// Return true if \p ptr is within one of the virtual address ranges
  /// allocated for the heap. Not intended for use in normal production GC
  /// operation, debug mode only.
  bool dbgContains(const void *ptr) const override;

  /// Return true if \p ptr is currently pointing at valid accessable memory,
  /// allocated to an object.
  bool validPointer(const void *ptr) const override;

  /// Returns true if \p cell is the most-recently allocated finalizable object.
  bool isMostRecentFinalizableObj(const GCCell *cell) const override;

  /// Whether the last allocation was fixed size.  Used to check that the
  /// FixedSize parameter used in allocation matches the fixed-size attribute of
  /// the object constructed on the allocated memory.  For long-lived
  /// allocations, we do not declare whether they are fixed size.
  inline FixedSizeValue lastAllocationWasFixedSize() const override;
#endif

  /// \return true if \p is in the young generation.
  bool inYoungGen(const void *ptr) const override {
    return youngGen_.contains(ptr);
  }

  /// Amount of space currently in use by allocated objects.
  /// The first version may be used always; the usedDirect version can only
  /// be used when the allocation context has been yielded, but is faster.
  size_t used() const;
  size_t usedDirect() const;

  /// Amount of space available for use (including space for external memory).
  /// The first version may be used always; the sizeDirect version can only
  /// be used when the allocation context has been yielded, but is faster.
  size_t size() const;
  size_t sizeDirect() const;

  /// The largest the size of this heap could ever grow to.
  inline size_t maxSize() const;

  /// Cumulative stats over time so far.
  size_t getPeakLiveAfterGC() const override;

  /// Populate \p info with information about the heap.
  void getHeapInfo(HeapInfo &info) override;
  void getHeapInfoWithMallocSize(HeapInfo &info) override;

  /// Populate \p info with crash manager information about the heap
  void getCrashManagerHeapInfo(CrashManager::HeapInformation &info) override;

#ifndef NDEBUG
  void getDebugHeapInfo(DebugHeapInfo &info) override;
#endif

  /// We expose counts of the number of GCs, for debug and testing purposes.
  inline size_t numGCs() const;
  inline size_t numYoungGCs() const;
  inline size_t numFullGCs() const;

  /// Creates a snapshot of the heap, which includes information about what
  /// objects exist, their sizes, and what they point to.
  virtual void createSnapshot(llvh::raw_ostream &os) override;

#ifdef HERMESVM_SERIALIZE
  /// Serialize WeakRefs.
  virtual void serializeWeakRefs(Serializer &s) override;

  /// Deserialize WeakRefs.
  virtual void deserializeWeakRefs(Deserializer &d) override;

  /// Serialize heap objects.
  virtual void serializeHeap(Serializer &s) override;

  /// Deserialize heap objects.
  virtual void deserializeHeap(Deserializer &d) override;

  /// See GCBase documentation for this function.
  virtual void deserializeStart() override;

  /// See GCBase documentation for this function.
  virtual void deserializeEnd() override;
#endif

  /// Returns the number of bytes allocated allocated since the last GC.
  gcheapsize_t bytesAllocatedSinceLastGC() const override;

  /// Shows statistics relevant to GenGC.
  virtual void printStats(JSONEmitter &json) override;

  /// Add some GenGC-specific stats to the output.
  void dump(llvh::raw_ostream &os, bool verbose = false) override;

  std::string getKindAsStr() const override;

  // Stats maintainence.

  /// Iterate over all objects in the heap, and call \p callback on them.
  /// \param callback A function to call on each found object.
  void forAllObjs(const std::function<void(GCCell *)> &callback) override;

#ifndef NDEBUG
  /// For testing purposes, we expose the ability to explicity request a
  /// young-generation collection.
  void youngGenCollect();

  /// The number of allocated objects in the heap (sum over the generations).
  unsigned computeNumAllocatedObjects() const;

  /// Set the GCBase instance variable that records the number of
  /// allocated objects before the most recent collection (recording
  /// the value returned by computeNumAllocatedObjects()).
  void recordNumAllocatedObjects() override {
    numAllocatedObjects_ = computeNumAllocatedObjects();
  }

  /// After a full collection, we've compacted the generations, so set
  /// the number of allocated objects to be the number of reachable
  /// objects in each generation.
  void setNumAllocatedObjectsAfterFullCollectionNC(
      const CompactionResult &compactionResult);

  /// The number of reachable objects in the heap (sum over the generations).
  unsigned computeNumReachableObjects() const;
  /// Reset the number of reachable objects in each generation to zero.
  void resetNumReachableObjectsInGens();
  /// Set the GCBase instance variable that records the number of
  /// reachable objects in the most recent collection.
  void recordNumReachableObjects(unsigned val) {
    numReachableObjects_ = val;
  }

  /// The number of reachable hidden classes objects in the heap (sum
  /// over the generations).
  unsigned computeNumHiddenClasses() const;

  /// The number of reachable leaf hidden classes objects in the heap (sum over
  /// the generations).
  unsigned computeNumLeafHiddenClasses() const;

  /// Reset the number of reachable hidden classes in each generation to zero.
  void resetNumAllHiddenClassesInGens();

  /// Set the GCBase instance variable that records the number of
  /// reachable hidden class objects in the most recent collection.
  void recordNumHiddenClasses(unsigned total, unsigned leaf) {
    numHiddenClasses_ = total;
    numLeafHiddenClasses_ = leaf;
  }

  // Record the given val as number of objects collected in the most
  // recent GC.
  void recordNumCollectedObjects(unsigned val) {
    numCollectedObjects_ = val;
  }

  /// The number of objects finalized in a just-completed full
  /// collection (the sum over the generations).
  unsigned computeNumFinalizedObjects() const;
  /// Set the GCBase instance variable that records the number of
  /// final objects in the most recent collection.
  void recordNumFinalizedObjects(unsigned val) {
    numFinalizedObjects_ = val;
  }

  /// Break down allocations, and objects surviving collection, by cell kind.
  static constexpr unsigned kNKinds =
      static_cast<unsigned>(CellKind::AllCellsKind_last) -
      static_cast<unsigned>(CellKind::AllCellsKind_first) + 1;
  uint64_t allocsByCellKind_[kNKinds] = {};
  uint64_t bytesAllocatedByCellKind_[kNKinds] = {};
  uint64_t fullGCReachableByCellKind_[kNKinds] = {};
  uint64_t fullGCBytesAllocatedByCellKind_[kNKinds] = {};

  /// Record that a cell of the given \p kind and size \p sz has been allocated.
  void trackAlloc(CellKind kind, unsigned sz);

  /// Record that a cell of the given \p kind and size \p sz has been
  /// found reachable in a full GC.
  void trackReachable(CellKind kind, unsigned sz) override;
#endif

  /// Reset the number of finalized objects in each generation to zero.
  void resetNumFinalizedObjectsInGens();

  inline size_t numFailedSegmentMaterializations() const;

  /// Mark a symbol id as being used.
  void markSymbol(SymbolID symbolID);

 protected:
  /// Do any additional GC-specific logging that is useful before dying with
  /// out-of-memory.
  void oomDetail(std::error_code reason) override;

// Mangling scheme used by MSVC encode public/private into the name.
// As a result, vanilla "ifdef public" trick leads to link errors.
#if defined(UNIT_TEST) || defined(_MSC_VER)
 public:
#else
 private:
#endif

  /// Return a number that is higher when the GC heap contains more dirty pages.
  llvh::ErrorOr<size_t> getVMFootprintForTest() const;

  /// FIXME: This is for backwards compatibility, new users should use
  /// Size::kYoungGenFractionDenom.
  static constexpr unsigned kYoungGenFractionDenom =
      Size::kYoungGenFractionDenom;

  /// Young gen initial size, as a function of the overall heap's size.
  size_t youngGenSize(size_t totalHeapSize) const;

 private:
  // Allow generations to use some private methods (checkWellFormedHeap,
  // markRoots, and recordGCStats).
  friend class GCGeneration;
  friend class YoungGen;
  friend class OldGen;

  /// Allocate a new cell of the specified size \p size.
  /// If necessary perform a GC cycle, which may potentially move allocated
  /// objects.
  /// The \p fixedSize template argument indicates whether the allocation is for
  /// a fixed-size cell, which can assumed to be small if true.  The
  /// \p hasFinalizer template argument indicates whether the object
  /// being allocated will have a finalizer.
  template <bool fixedSize = true, HasFinalizer hasFinalizer = HasFinalizer::No>
  inline void *alloc(uint32_t sz);

  /// The slow path for allocation.  Same specification as alloc(),
  /// albeit with the template arguments passed as explicit dynamic
  /// arguments.
  void *allocSlow(uint32_t sz, bool fixedSize, HasFinalizer hasFinalizer);

  /// Like alloc above, but the resulting object is expected to be long-lived.
  /// Allocate directly in the old generation (doing a full collection if
  /// necessary to create room).
  template <HasFinalizer hasFinalizer = HasFinalizer::No>
  inline void *allocLongLived(uint32_t size);

  /// The given pointer value is being written at the given loc (required to
  /// be in the heap).  The value is may be null.  Execute a write
  /// barrier.  The \p hv argument indicates whether this is being
  /// called from the HV (true) or void* (false) version of the write
  /// barrier; this is used only in debug builds, for statistics.
  inline void writeBarrierImpl(const void *loc, const void *value, bool hv);

  /// Returns the desired heap size for the given number of used bytes --
  /// determined by the occupancyTarget() ratio and the max heap size
  /// representable by gcheapsize_t.  Note that \p usedBytes is given as a
  /// size_t because it can include sizes due to external memory allocations
  /// which may in general exceed quantities representable by gcheapsize_t.
  gcheapsize_t usedToDesiredSize(size_t usedBytes);

  /// Signal to the heap that it can now use around \p hint bytes of heap.
  void growTo(size_t hint);

  /// Signal to the heap that it should decrease its size to around \p
  /// hint bytes.
  void shrinkTo(size_t hint);

  /// Update totalBytesAllocated, at the start of a GC.
  void updateTotalAllocStats();

  /// Return (via swap) the current allocation context to the GCGeneration
  /// from which it was claimed.
  void yieldAllocContext();

  /// Claim (via swap) the allocation context of the GCGeneration we're
  /// allocating from by default -- currently this is the young
  /// generation.
  void claimAllocContext();

  /// Returns whether the GC has currently claimed the allocation context.
  inline bool allocContextClaimed() const;

  /// An RAII class whose constructor ensures that the current allocation
  /// context has been yielded, and whose destructor (re)claims it if
  /// the constructor actually yielded it.
  class AllocContextYieldThenClaim {
   public:
    AllocContextYieldThenClaim(GenGC *gc)
        : gc_(gc), enable_(gc->allocContextClaimed()) {
      if (enable_) {
        gc_->yieldAllocContext();
      }
    }
    ~AllocContextYieldThenClaim() {
      if (enable_) {
        gc_->claimAllocContext();
      }
    }

   private:
    GenGC *gc_;
    bool enable_;
  };

  /// Dump information about a cell to stderr.
  void dumpCellInfo(const GCCell *cell);

  /// Print stats (in JSON format) specific to full collections to an output
  /// stream.
  void printFullCollectionStats(JSONEmitter &json) const;

  /// In debug, these increment the counts of the indicated kinds of
  /// write barriers.  First is for normal barriers.  In opt, they do nothing.
  /// In countWriteBarrier, \p hv indicates whether the barrier is for
  /// the HermesValue barrier version (true) or the void* version
  /// (false).  The \p filterLevel indicates what filtering tests have
  /// been passed.
  inline void countWriteBarrier(bool hv, unsigned filterLevel);
  inline void countRangeWriteBarrier();
  inline void countRangeFillWriteBarrier();

  friend struct CollectionSection;

  /// RAII class managing the actions that need to be performed immediately
  /// before and immediately after every garbage collection.
  struct CollectionSection : public PerfSection {
    CollectionSection(
        GenGC *gc,
        const char *name,
        std::string cause,
        OptValue<GCCallbacks *> gcCallbacksOpt = llvh::None);
    ~CollectionSection();

    /// Update the cumulative GC statistics held for all GCs, and the statistics
    /// held for the region being collected at the moment (the young generation
    /// for young gen collections, and the entire heap for full collections).
    ///
    /// \param regionSize The size of the region after the last GC.
    /// \param usedBefore The number of bytes allocated just before the GC.
    /// \param sizeBefore The size of all segments in bytes just before the GC.
    /// \param externalBefore The number of bytes external to the JS heap just
    ///   before the GC.
    /// \param usedAfter The number of bytes live after the GC.
    /// \param sizeAfter The size of all segments in bytes after the GC.
    /// \param externalAfter The number of bytes external to the JS heap after
    ///   the GC.
    /// \param regionStats A pointer to the cumulative statistics struct for the
    ///     region.
    void recordGCStats(
        size_t regionSize,
        size_t usedBefore,
        size_t sizeBefore,
        size_t externalBefore,
        size_t usedAfter,
        size_t sizeAfter,
        size_t externalAfter,
        CumulativeHeapStats *regionStats);

   private:
    GenGC *gc_;
    GCCycle cycle_;
    std::string cause_;
    TimePoint wallStart_;
    std::chrono::microseconds cpuStart_;
    size_t gcUsedBefore_;
    // Initial value indicates unset.
    double wallElapsedSecs_{-1.0};
    double cpuElapsedSecs_{-1.0};
    AllocContextYieldThenClaim yielder_;
  };

#ifndef NDEBUG
  /// Traverse the generations, adding the GC cells allocated since
  /// the last GC to the alloc-tracking histogram.
  void doAllocCensus();

  /// Print data recorded about the distribution of allocations (and
  /// bytes), and reachable objects (and bytes) by cell kind.
  void printCensusByKindStats(llvh::raw_ostream &os) const;

  /// Helper function for the above.  Assumes \p allocs and \b bytes
  /// are arrays, indexed by CellKind, of number of objects and
  /// bytes for the given CellKind.  If there are any objects in the
  /// data, prints \p msg, then the table, sorted by # of allocs.
  void printCensusByKindStatsWork(
      llvh::raw_ostream &os,
      const char *msg,
      const uint64_t *allocs,
      const uint64_t *bytes) const;
#endif

  /// Mark/Sweep/Compact GC:

  /// The individual phases of Mark/Sweep/Compact GC.

  /// Do the marking: find all the reachable objects, setting the bit
  /// corresponding to their start address in the appropriate mark bit arrays.
  void markPhase();

  /// Clear the mark bits for all segments in use.
  void clearMarkBits();

  /// Call the finalizer methods on cells that are not marked in both the young
  /// and old generations.
  void finalizeUnreachableObjects();

  /// Sweep the marked heap, finding dead objects (and calling their finalizers,
  /// if necessary), determining the addresses to which live objects will be
  /// copied, and installing forwarding pointers to those addresses.  Forwarding
  /// pointers overwrite vtable pointers, which are copied to an external
  /// vector.  Updates the SweepResult that is passed in and holds the
  /// (eventual) new "levels" (i.e., allocation points) of segments that will be
  /// filled during compaction -- the first free bytes in the post-compaction
  /// segments -- and the displaced vtable pointers.
  void sweepAndInstallForwardingPointers(SweepResult *sweepResult);

  /// Iterate over the pointer fields of all live objects, updating
  /// them by following the forwarding pointers in their referents.
  void updateReferences(const SweepResult &sweepResult);

  /// Iterate over the live objects, moving them to their post-compaction
  /// addresses and restoring their displaced VTable pointers (from
  /// sweepResult).  Also, sets the "levels" of the generations to the values in
  /// the chunks used during compaction.
  void compact(const SweepResult &sweepResult);

  /// Helper routines used by marking:

  /// Complete the marking phase: after marking from roots has set mark
  /// bits of objects directly reachable from the roots, transitively
  /// close the mark bits.
  void completeMarking();

  /// In the first phase of marking, before this is called, we treat
  /// JSWeakMaps specially: when we mark a reachable JSWeakMap, we do
  /// not mark from it, but rather save a pointer to it in a vector.
  /// Then we call this method, which finds the keys that are
  /// reachable, and marks transitively from the corresponding value.
  /// This is done carefully, to reach a correct global transitive
  /// closure, in cases where keys are reachable only via values of
  /// other keys.  When this marking is done, entries with unreachable
  /// keys are cleared.  Normal WeakRef processing at the end of GC
  /// will delete the cleared entries from the map.
  void completeWeakMapMarking();

  /// Does any work necessary for GC stats at the end of collection.
  /// Returns the number of allocated objects before collection starts.
  /// (In optimized builds, does nothing, and returns zero.)
  unsigned recordStatsNC(const CompactionResult &compactionResult);

  /// Checks that some invariants hold at the end of collection.  The
  /// \p numAllocatedObjectsBefore argument should be the number of
  /// objects allocated at the start of collection, and \p usedBefore
  /// should be the number of bytes those objects occupied.  (In optimized
  /// builds, does nothing.)
  void checkInvariants(unsigned numAllocatedObjectsBefore, size_t usedBefore);

  /// The different "mark" implementations for different phases of the
  /// collector:

  /// Mark from roots: just set the bit in the bitmap.
  void markRoot(void *ptr);
  /// Complete marking: for each marked object, mark from it, using
  /// marking stack.
  void markTransitive(void *ptr);
  /// Update root pointers: for each root, update the pointer value
  /// using the forwarding pointer in the referent.
  void markUpdateRoots(GCCell **ptrLoc);
  /// Same as above, but for a HermesValue ref.
  void markUpdateRoots(void *p, HermesValue &hv);

#ifdef HERMES_SLOW_DEBUG
  /// For debugging: iterates over objects, asserting that all GCCells
  /// have vtables with valid cell kinds, and that all object pointers
  /// point to GCCells whose vtables have valid cell kinds.
  void checkWellFormedHeap() const;
#endif

  /// Notify the GC that a segment it is responsible for (through one of its
  /// generations) has moved -- in the sense of std::move, i.e. the contents of
  /// the struct representing the segment have been moved to a different struct.
  /// The memory region the segment owns did not move.
  ///
  /// \p segment A pointer to the new location of the segment.
  void segmentMoved(GenGCHeapSegment *segment);

  /// Notify the GC that it should stop tracking the given segments.
  ///
  /// \p lowLims A vector of addresses identifying segments by their lowLim
  ///     value.  I.e. a segment, s, in this GC's index is identified by
  ///     s.lowLim().
  ///
  /// \pre \p lowLims is sorted in increasing order of address.
  ///
  /// \post Any segment that is identified by an address in \p lowLims and was
  ///     in this GC's segment index before this call has been removed from that
  ///     index.
  void forgetSegments(const std::vector<const char *> &lowLims);

  /// Update all the weak references' pointers in response to the GC's heap
  /// having moved.
  void moveWeakReferences(ptrdiff_t delta);

  /// Update all weak reference slots: update the pointers of live objects and
  /// clear the pointers of freed objects. Additionally, free all weak slots
  /// that are no longer in use (weren't marked).
  void updateWeakReferences(bool fullGC);

  /// Updates a single WeakRefSlot.  The \p fullGC argument indicates
  /// what kind  of collection this is being done as part of.
  /// This version uses internal mark bits.
  void updateWeakReference(WeakRefSlot *slot, bool fullGC);

  /// Set all the marked weak references to unmarked.
  void unmarkWeakReferences();

  /// Allocate a new weak reference slot and return a pointer to it.
  WeakRefSlot *allocWeakSlot(HermesValue init) override;

  /// Free a weak reference slot.
  void freeWeakSlot(WeakRefSlot *value);

  /// Shrink the slot storage if there are free slots at the end.
  void shrinkWeakSlots();

  /// At the end of a full collection, update the exponential weighted average
  /// of the live data.
  void updateWeightedUsed();

  /// At the end of a full collection, grow or shrink the heap, if heuristics
  /// indicate.
  void updateHeapSize();

  /// The generation from which the alloc context is claimed, as a
  /// GCGeneration*.
  inline GCGeneration *targetGeneration();

  /// Change the heap extents recorded via the crash manager's custom data.
  void updateCrashManagerHeapExtents();

  template <class T>
  friend class WeakRef;

  /// The storage provider is a way to access storage for new segments.
  std::shared_ptr<StorageProvider> storageProvider_;

  /// A mapping from the lowest address in a segment's memory region, to a
  /// pointer to the segment itself.
  GCSegmentAddressIndex segmentIndex_;

  /// The sizes of each generation.
  Size generationSizes_;

#ifdef HERMES_EXTRA_DEBUG
  /// Whether we should do metadata protection -- mprotect GC data
  /// structures when not in use (e.g., between GCs), in order to
  /// detect corrupting writes.  (Note: must declare/define before
  /// youngGen_/oldGen_, because may be accessed in the ctors of the
  /// generations.)
  bool doMetadataProtection_;
#endif

  /// The generations that make up the heap.
  /// Note: these must be declared and initialized in this order; the OldGen's
  /// max size is calculated based on the YoungGen's max size.
  YoungGen youngGen_;
  OldGen oldGen_;

  /// The current allocation context: what segment we are allocating
  /// into, where to accumulate finalizable objects, the count of
  /// allocated objects.  Should be valid when allocating.  When doing
  /// GC, will be yielded back to the owning generation.
  GCGeneration::AllocContext allocContext_;

  /// True if the allocContext_ is from the young generation.  If
  /// false, it comes from the old generation.
  bool allocContextFromYG_{true};

  /// Indicates that at TTI notification, should revert to YG
  /// allocation (if had been doing OG allocation).
  const bool revertToYGAtTTI_;

#ifndef NDEBUG
  bool allocInYoung_{true};
#endif

  /// Fraction of the heap that is intended to be occupied by live data.
  double occupancyTarget_;

  /// The number of consecutive full collections we consider to be an "Effective
  /// OOM".
  unsigned oomThreshold_;

  /// Number of consecutive full GCs we have seen so far.
  unsigned consecFullGCs_{0};

  /// We maintain an exponential weighted average of the live data, which
  /// we will consult to see whether to shrink the heap size after full GCs.
  double weightedUsed_;
  /// This is the factor that defines the exponential value:
  ///    V[n+1] = kWeightedUsedAlpha * current  + (1.0 - kWeightedUsedAlpha) *
  ///    V[n]
  static constexpr double kWeightedUsedAlpha = 0.2;

  /// Full heap marking infrastructure.

  /// Contains the markStack, overflow boolean, and pointer to the
  /// parent object of the object currently being marked.
  CompleteMarkState markState_;

  /// Every bit corresponds to a symbol id. It is set to true if the symbol is
  /// in use (was marked).
  llvh::BitVector markedSymbols_{};

  /// Pointers to the slots (elements of weakSlots_, above) that may
  /// possibly have pointer referents that point into the young generation.
  std::vector<WeakRefSlot *> weakRefSlotsWithPossibleYoungReferent_{};

  /// Pointer to the first free weak reference slot.
  WeakRefSlot *firstFreeWeak_{nullptr};

  /// VTable pointers overwritten with forwarding pointers are stored here.
  std::vector<KindAndSize> displacedKinds_;

  /// Status by kind of collection.
  CumulativeHeapStats youngGenCollectionCumStats_;
  CumulativeHeapStats fullCollectionCumStats_;

  /// When the mark behavior is MarkBehavior::ScanDirtyCardEvac, these
  /// variables hold the bounds of the dirty card(s) to be scanned.
  struct CardSeqBounds {
    char *cardStart;
    char *cardEnd;
  };

  /// Cumulative by-phase times for full collection.
  double markRootsSecs_ = 0.0;
  double markTransitiveSecs_ = 0.0;
  double sweepSecs_ = 0.0;
  double updateReferencesSecs_ = 0.0;
  double compactSecs_ = 0.0;

  /// The sum of the pre-collection sizes of the heap before/after
  /// full collections.
  gcheapsize_t cumPreBytes_ = 0;
  gcheapsize_t cumPostBytes_ = 0;

#ifdef HERMES_SLOW_DEBUG
  /// We increment this on every (SLOW_DEBUG) allocation.  The purpose is to
  // double-check our more-efficient (per YG GC) allocated bytes calculation.
  uint64_t totalAllocatedBytesDebug_ = 0;
#endif
#ifndef NDEBUG
  /// Whether the last allocation was fixed size (if we know).  Used
  /// to check that the FixedSize parameter used in allocation matches
  /// the fixed-size attribute of the object constructed on the
  /// allocated memory.  (Initial value doesn't matter, since it
  /// should not be referenced before being set explicitly.)
  FixedSizeValue lastAllocWasFixedSize_;
#endif
};

template <bool fixedSize, HasFinalizer hasFinalizer>
inline void *GenGC::alloc(uint32_t sz) {
#ifndef NDEBUG
  lastAllocWasFixedSize_ = fixedSize ? FixedSizeValue::Yes : FixedSizeValue::No;
#endif
  if (shouldSanitizeHandles()) {
    // In order to get the maximum benefit of sanitization, the entire heap
    // should be moved and poisoned with ASAN to force errors to occur.
    // MallocGC already implements that well though, and it is complicated and
    // slow to implement that for NCGen. So instead do a full collection which
    // is almost as good.
    collect("handle-san");
  }

#ifndef NDEBUG
  AllocResult res = debugAlloc(sz, hasFinalizer, fixedSize);
  assert(res.success && "Should never fail to allocate at the top level");
#ifdef HERMES_SLOW_DEBUG
  totalAllocatedBytesDebug_ += sz;
#endif
  return res.ptr;
#else
  // We repeat this in opt, to ensure that the AllocResult is only
  // initialized once.
  AllocResult res = allocContext_.alloc(sz, hasFinalizer);
  if (LLVM_LIKELY(res.success)) {
    return res.ptr;
  }
  return allocSlow(sz, fixedSize, hasFinalizer);
#endif // NDEBUG
}

template <HasFinalizer hasFinalizer>
inline void *GenGC::allocLongLived(uint32_t size) {
#ifndef NDEBUG
  lastAllocWasFixedSize_ = FixedSizeValue::Unknown;
#endif
  if (shouldSanitizeHandles()) {
    // In order to get the maximum benefit of sanitization, the entire heap
    // should be moved and poisoned with ASAN to force errors to occur.
    // MallocGC already implements that well though, and it is complicated and
    // slow to implement that for NCGen. So instead do a full collection which
    // is almost as good.
    collect("handle-san");
  }
  AllocResult res;
  if (allocContextFromYG_) {
    res = oldGen_.alloc(size, hasFinalizer);
    assert(res.success && "Should never fail to allocate at the top level");
#ifdef HERMES_SLOW_DEBUG
    totalAllocatedBytesDebug_ += size;
#endif
    return res.ptr;
  } else {
    res = allocContext_.alloc(size, hasFinalizer);
    if (LLVM_LIKELY(res.success)) {
#ifdef HERMES_SLOW_DEBUG
      totalAllocatedBytesDebug_ += size;
#endif
      return res.ptr;
    } else {
      AllocContextYieldThenClaim yielder(this);
      res = oldGen_.alloc(size, hasFinalizer);
      assert(res.success && "Should never fail to allocate at the top level");
#ifdef HERMES_SLOW_DEBUG
      totalAllocatedBytesDebug_ += size;
#endif
      return res.ptr;
    }
  }
}

template <
    typename T,
    bool fixedSize,
    HasFinalizer hasFinalizer,
    LongLived longLived,
    class... Args>
inline T *GenGC::makeA(uint32_t size, Args &&...args) {
  assert(
      isSizeHeapAligned(size) &&
      "Call to makeA must use a size aligned to HeapAlign");
  // TODO: Once all callers are using makeA, remove allocLongLived.
  void *mem = longLived == LongLived::Yes
      ? allocLongLived<hasFinalizer>(size)
      : alloc<fixedSize, hasFinalizer>(size);
  return new (mem) T(std::forward<Args>(args)...);
}

inline void GenGC::countWriteBarrier(bool hv, unsigned filterLevel) {
#ifndef NDEBUG
  assert(filterLevel < kNumFilterLevels);
  numWriteBarriers_[hv][filterLevel]++;
#endif
}

inline void GenGC::countRangeWriteBarrier() {
#ifndef NDEBUG
  numRangeBarriers_++;
#endif
}

inline void GenGC::countRangeFillWriteBarrier() {
#ifndef NDEBUG
  numRangeFillBarriers_++;
#endif
}

inline size_t GenGC::maxSize() const {
  return youngGen_.maxSize() + oldGen_.maxSize();
}

inline size_t GenGC::numGCs() const {
  return cumStats_.numCollections;
}

inline size_t GenGC::numYoungGCs() const {
  return youngGenCollectionCumStats_.numCollections;
}

inline size_t GenGC::numFullGCs() const {
  return fullCollectionCumStats_.numCollections;
}

inline size_t GenGC::numFailedSegmentMaterializations() const {
  return storageProvider_->numFailedAllocs();
}

#ifndef NDEBUG
inline GenGC::FixedSizeValue GenGC::lastAllocationWasFixedSize() const {
  return lastAllocWasFixedSize_;
}
#endif

inline void
GenGC::writeBarrierImpl(const void *loc, const void *value, bool hv) {
  HERMES_SLOW_ASSERT(dbgContains(loc));

  const char *locPtr = reinterpret_cast<const char *>(loc);

  // value may be null.  But if that occurs: locPtr and value will not
  // be in the same AlignedStorage, so the first test below will fail,
  // and we will not return early.  But youngGen_.contains(value) will
  // fail, so we will (correctly) not dirty the card for loc.
  HERMES_SLOW_ASSERT(value == nullptr || dbgContains(value));
  if (AlignedStorage::containedInSame(locPtr, value)) {
    return;
  }
  countWriteBarrier(hv, kNumWriteBarrierWithDifferentSegsIdx);
  if (youngGen_.contains(value)) {
    countWriteBarrier(hv, kNumWriteBarrierPtrInYGIdx);
    GenGCHeapSegment::cardTableCovering(locPtr)->dirtyCardForAddress(locPtr);
  }
}

inline bool GenGC::allocContextClaimed() const {
  return !!allocContext_;
}

inline GCGeneration *GenGC::targetGeneration() {
  return allocContextFromYG_ ? static_cast<GCGeneration *>(&youngGen_)
                             : static_cast<GCGeneration *>(&oldGen_);
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GENGC_H
