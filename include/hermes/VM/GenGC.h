/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_GENGC_H
#define HERMES_VM_GENGC_H

#include "hermes/Public/GCConfig.h"
#include "hermes/Support/OptValue.h"
#include "hermes/Support/SlowAssert.h"
#include "hermes/VM/AllocResult.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/CompleteMarkState.h"
#include "hermes/VM/DependentMemoryRegion.h"
#include "hermes/VM/GCBase.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/MarkBitArray.h"
#include "hermes/VM/OldGen.h"
#include "hermes/VM/SweepResult.h"
#include "hermes/VM/YoungGen.h"
#include "hermes/VM/detail/BackingStorage.h"

#include "llvm/Support/Casting.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/ErrorHandling.h"

#include <deque>
#include <limits>
#include <vector>

// We assume this file is only included by GC.h, which declares GCBase.

namespace hermes {
namespace vm {

using namespace detail;

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
  /// See comment in GCBase.
  class Size final {
   public:
    explicit Size(const GCConfig &gcConfig);
    Size(gcheapsize_t min, gcheapsize_t max);

    gcheapsize_t min() const {
      return min_;
    }
    gcheapsize_t max() const {
      return max_;
    }
    gcheapsize_t storageFootprint() const;
    gcheapsize_t minStorageFootprint() const;

   private:
    gcheapsize_t min_;
    gcheapsize_t max_;
  };

  /// Initialize the GC with a suggested initial heap size, and given
  /// maximum size.
  /// \param gcCallbacks A callback interface enabling the garbage collector to
  ///   mark roots and free symbols.
  GenGC(
      MetadataTable metaTable,
      GCCallbacks *gcCallbacks,
      PointerBase *pointerBase,
      const GCConfig &gcConfig,
      std::shared_ptr<CrashManager> crashMgr,
      StorageProvider *provider);

  /// Allocate a new cell of the specified size \p size.
  /// If necessary perform a GC cycle, which may potentially move allocated
  /// objects.
  /// The \p fixedSize template argument indicates whether the allocation is for
  /// a fixed-size cell, which can assumed to be small if true.  The
  /// \p hasFinalizer template argument indicates whether the object
  /// being allocated will have a finalizer.
  template <bool fixedSize = true, HasFinalizer hasFinalizer = HasFinalizer::No>
  inline void *alloc(uint32_t sz);

#ifndef NDEBUG
  /// Allocation path we use in debug builds, where we potentially follow
  /// different code paths to test things.
  AllocResult
  debugAlloc(uint32_t sz, HasFinalizer hasFinalizer, bool fixedSize);
#endif

  /// Returns whether an external allocation of the given \p size fits
  /// within the maximum heap size.  (Note that this does not guarantee that the
  /// allocation will "succeed" -- the size plus the used() of the heap may
  /// still exceed the max heap size.  But if it fails, the allocation can never
  /// succeed.)
  bool canAllocExternalMemory(uint32_t size);

  /// Like alloc above, but the resulting object is expected to be long-lived.
  /// Allocate directly in the old generation (doing a full collection if
  /// necessary to create room).
  template <HasFinalizer hasFinalizer = HasFinalizer::No>
  inline void *allocLongLived(uint32_t size);

  /// An allocation that yielded the given \p alloc has associated external
  /// memory of the given \p size.  Add that to the appropriate external memory
  /// charge.
  void creditExternalMemory(GCCell *alloc, uint32_t size);

  /// The object at \p alloc is being collected, and has associated external
  /// memory of the given \p size.  Decrease the external memory charge of the
  /// generation owning \p alloc by this amount.
  void debitExternalMemory(GCCell *alloc, uint32_t size);

  /// Write barriers.

  /// The given value is being written at the given loc (required to
  /// be in the heap).  If value is a pointer, execute a write barrier.
  inline void writeBarrier(void *loc, HermesValue value);

  /// The given pointer value is being written at the given loc (required to
  /// be in the heap).  The value is may be null.  Execute a write barrier.
  inline void writeBarrier(void *loc, void *value);

#ifndef NDEBUG
  /// Count the number of barriers executed.
  uint64_t numWriteBarriers_{0};
  uint64_t numRangeBarriers_{0};
  uint64_t numRangeFillBarriers_{0};

  /// Returns whether a write of the given value into the given location
  /// requires a write barrier.
  bool needsWriteBarrier(void *loc, void *value);

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

  void printExtAllocStats(llvm::raw_ostream &os);
#endif

  /// We copied HermesValues into the given region.  Note that \p numHVs is
  /// the number of HermesValues in the the range, not the char length.
  /// Do any necessary barriers.
  void writeBarrierRange(HermesValue *start, uint32_t numHVs);

  /// We filled numHVs slots starting at start with the given value.
  /// Do any necessary barriers.
  void
  writeBarrierRangeFill(HermesValue *start, uint32_t numHVs, HermesValue value);

  /// Force a garbage collection cycle.
  /// (Part of general GC API defined in GC.h).
  /// Does a mark/sweep/compact collection of both generations.  (The
  /// compaction is within each generation separately).
  void collect();

  static constexpr uint32_t maxAllocationSize() {
    // GenGC imposes no limit on individual allocations.
    return std::numeric_limits<uint32_t>::max();
  }

  /// The occupancy target guides heap sizing -- the fraction of the heap
  /// that is intended to be occupied by live data.
  double occupancyTarget() const {
    // TODO: make this settable on the command line.
    return 0.5;
  }

  /// Run the finalizers for all heap objects.
  void finalizeAll();

  /// Return true if \p ptr is within the (maximum) virtual address
  /// range allocated for the heap.
  bool contains(void *ptr) const;

  /// Return the lowerbound of the heap's virtual address range (inclusive).
  char *lowLim() const;

  /// Return the upperbound of the heap's virtual address range (exclusive).
  char *hiLim() const;

#ifndef NDEBUG
  /// Return true if \p ptr is currently pointing at valid accessable memory,
  /// allocated to an object.
  bool validPointer(const void *ptr) const;

  /// Returns true if \p cell is the most-recently allocated finalizable object.
  bool isMostRecentFinalizableObj(const GCCell *cell) const;
#endif

  /// \return true if \p is in the young generation.
  bool inYoungGen(const void *ptr) const {
    return youngGen_.contains(ptr);
  }

  /// Amount of space currently in use by allocated objects.
  size_t used() const;

  /// Amount of space available for use (including space for external memory).
  size_t size() const;

  /// Maximum size that the heap can grow to.
  size_t maxSize() const {
    return maxSize_;
  }

  /// \return true if cell is marked.
  bool isMarked(GCCell *cell) const {
    return markBits_.at(markBits_.addressToIndex(cell));
  }

  /// Mark a weak reference as being used.
  void markWeakRef(const WeakRefBase &wr);
#ifndef NDEBUG
  /// \return Number of weak ref slots currently in use.
  /// Inefficient. For testing/debugging.
  size_t countUsedWeakRefs() const;
#endif

  /// Populate \p info with information about the heap.
  /// Includes more specific GenGC information.
  void getHeapInfo(HeapInfo &info) override;
  void getHeapInfoWithMallocSize(HeapInfo &info) override;

  /// We expose counts of the number of GCs, for debug and testing purposes.
  inline size_t numGCs() const;
  inline size_t numYoungGCs() const;
  inline size_t numFullGCs() const;

  /// Creates a snapshot of the heap, which includes information about what
  /// objects exist, their sizes, and what they point to.
  virtual void createSnapshot(llvm::raw_ostream &os, bool compact) override;

  /// Returns the number of bytes allocated allocated since the last GC.
  gcheapsize_t bytesAllocatedSinceLastGC() const override;

  /// Shows statistics relevant to GenGC.
  virtual void printStats(llvm::raw_ostream &os, bool trailingComma) override;

  /// Add some GenGC-specific stats to the output.
  void dump(llvm::raw_ostream &os, bool verbose = false) override;

  // Stats maintainence.

  /// Access to the number of collections, for use by generations.
  unsigned numCollections() {
    return cumStats_.numCollections;
  }

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
  void setNumAllocatedObjectsAfterFullCollection(
      const std::vector<SweepResult> &sweepResults);

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
  /// over the generations).  If
  unsigned computeNumHiddenClasses(bool leafOnly = false) const;
  /// Reset the number of reachable hidden classes in each generation to zero.
  void resetNumHiddenClassesInGens();
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
  void trackReachable(CellKind kind, unsigned sz);
#endif

  /// Reset the number of finalized objects in each generation to zero.
  void resetNumFinalizedObjectsInGens();

  /// Mark a symbol id as being used.
  void markSymbol(SymbolID symbolID);

// Mangling scheme used by MSVC encode public/private into the name.
// As a result, vanilla "ifdef public" trick leads to link errors.
#if defined(UNIT_TEST) || defined(_MSC_VER)
 public:
#else
 private:
#endif

  /// These constants control the sizing of the young generation wrt the
  /// total heap size.  The young gen will be 1 / kYoungGenFractionDenom
  /// of the total heap size, up to a maximum of kMaxYounGenSize.
  /// Many heuristics are possible here; this requires experimentation.
  static constexpr gcheapsize_t kMaxYoungGenSize = 8 * 1024 * 1024;
  static constexpr unsigned kYoungGenFractionDenom = 8;

  /// Young gen initial size, as a function of the overall heap's size.
  static size_t youngGenSize(size_t totalHeapSize);

 private:
  // Allow YoungGen to use some private methods (checkWellFormedHeap,
  // markRoots, and recordGCStats).
  friend class YoungGen;
  friend class OldGen;

  /// Clamp and page-align \p desiredOldGenSize and derive the total heap size
  /// from it.
  size_t totalSizeFromDesiredOldGenSize(uint64_t desiredOldGenSize);

  /// Update totalBytesAllocated, at the start of a GC.
  void updateTotalAllocStats();

  /// Dump information about a cell to stderr.
  void dumpCellInfo(const GCCell *cell);

  /// Print stats (in JSON format) specific to full collections to an output
  /// stream.
  /// \p os Is the output stream to print the stats to.
  /// \p trailingComma determines whether the output includes a trailing comma.
  void printFullCollectionStats(llvm::raw_ostream &os, bool trailingComma)
      const;

  /// In debug, these increment the counts of the indicated kinds of
  /// write barriers.  First is for normal barriers.  In opt, they do nothing.
  void countWriteBarrier();
  void countRangeWriteBarrier();
  void countRangeFillWriteBarrier();

  /// Record the levels of the generations at the end of the a GC (to
  /// enable later iteration over objects allocated since then).
  void recordGenLevelsAtEndOfLastGC();

#ifndef NDEBUG
  /// Traverse the generations, adding the GC cells allocated since
  /// the last GC to the alloc-tracking histogram.
  void doAllocCensus();

  /// Print data recorded about the distribution of allocations (and
  /// bytes), and reachable objects (and bytes) by cell kind.
  void printCensusByKindStats(llvm::raw_ostream &os) const;

  /// Helper function for the above.  Assumes \p allocs and \b bytes
  /// are arrays, indexed by CellKind, of number of objects and
  /// bytes for the given CellKind.  If there are any objects in the
  /// data, prints \p msg, then the table, sorted by # of allocs.
  void printCensusByKindStatsWork(
      llvm::raw_ostream &os,
      const char *msg,
      const uint64_t *allocs,
      const uint64_t *bytes) const;
#endif

  /// Convenience method to invoke the mark roots function provided at
  /// initialization, using the context provided then (on this heap).
  /// The \p markLongLived argument indicates whether root data structures
  /// containing only pointers to objects allocated via allocLongLived
  /// are required to be marked.  In this collector, such objects will
  /// be allocated in the old gen, and references to them need not be
  /// marked during young-gen collection.
  void markRoots(SlotAcceptorWithNames &acceptor, bool markLongLived) {
    gcCallbacks_->markRoots(this, acceptor, markLongLived);
  }

  /// Mark/Sweep/Compact GC:

  /// The individual phases of Mark/Sweep/Compact GC.

  /// Do the marking.
  void markPhase();

  /// Sweep the marked heap, finding dead objects (and calling their
  /// finalizers, if necessary), determining the addresses to which
  /// live objects will be copied, and installing forwarding pointers
  /// to those addresses.  Forwarding pointers overwrite vtable
  /// pointers, which are copied to an external vector.  Returns a
  /// SweepResult for every generation.  This has the (eventual) new
  /// "levels" (i.e., allocation points) of the generations -- the
  /// first free bytes in the post-compaction generations -- and the
  /// displaced vtable pointers.  There will be two elements in the
  /// returned vector.
  std::vector<SweepResult> sweepAndInstallForwardingPointers();

  /// Iterate over the pointer fields of all live objects, updating
  /// them by following the forwarding pointers in their referents.
  void updateReferences(const std::vector<SweepResult> &sweepResults);

  /// Iterate over the live objects, moving them "downwards" to their
  /// post-compaction addresses.  Also, restore displaced VTable
  /// pointers (from sweepResults).  Sets the "levels" of the
  /// generations to the values in sweepResults.  (Requires there to
  /// be two elements in "newLevels".)
  void compact(const std::vector<SweepResult> &sweepResults);

  /// Helper routines used by marking:

  /// Complete the marking phase: after marking from roots has set mark
  /// bits of objects directly reachable from the roots, transitively
  /// close the mark bits.
  void completeMarking();

  /// Does any work necessary for GC stats at the end of collection.
  /// Returns the number of allocated objects before collection starts.
  /// (In optimized builds, does nothing, and returns zero.)
  unsigned recordStats(const std::vector<SweepResult> &sweepResults);

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

  /// Create a new backing storage, for this heap.  Expects
  /// maxSize_ and initialSize_ to be set to the expected and minimum
  /// sizes, respectively, of the backing storage.  May update
  /// maxSize_ to the size actually obtained (which is guaranteed to
  /// be page-aligned and at least as large as initialSize_).
  BackingStorage allocBackingStorage();

  /// Allocate new backing storage, and move the contents of the heap there.
  void swapToFreshHeap();

  /// Update all the weak references' pointers in response to the GC's heap
  /// having moved.
  void moveWeakReferences(ptrdiff_t delta);

  /// Max size of the young generation for the given \p totalSize of
  /// the heap.  (Function also of the young-gen sizing policy.)
  static size_t maxYoungGenSize(size_t totalSize);

  /// Young gen high limit (and old gen low limit), as a function of the overall
  /// heap's lower limit, size, and max size.
  char *youngGenHiLim() const;

  /// The sizes of the young and old generations, respectively.
  struct SizeConfig {
    size_t ygSize;
    size_t ogSize;
    size_t total() const {
      return ygSize + ogSize;
    }
  };
  /// The GC may apply some heuristic to get a desired new size for
  /// the heap as a whole.  This method applies various constraints
  /// (including the constraint that the individual generation sizes
  /// do not decrease) and yield new sizes for the young and old
  /// generations that approximate the desired total size, subject to
  /// those constraints.  Returns an OptValue with a value only if the
  /// heap size increases.
  OptValue<SizeConfig> newSizesForDesiredSize(uint64_t desiredSize) const;

  /// This enum describes the states of a weak reference slot.
  struct WeakSlotState {
    enum {
      /// Valid but still not marked as reachable.
      Unmarked,
      /// Valid and marked as reachable.
      Marked,
      /// Available for reuse.
      Free,
    };
  };

  /// Update all weak reference slots: update the pointers of live objects and
  /// clear the pointers of freed objects. Additionally, free all weak slots
  /// that are no longer in use (weren't marked).
  void updateWeakReferences(bool fullGC);

  /// Updates a single WeakRefSlot.  The \p fullGC argument indicates
  /// what kind  of collection this is being done as part of.
  void updateWeakReference(WeakRefSlot *slot, bool fullGC);

  /// Set all the marked weak references to unmarked.
  void unmarkWeakReferences();

  /// Allocate a new weak reference slot and return a pointer to it.
  WeakRefSlot *allocWeakSlot(HermesValue init);

  /// Free a weak reference slot.
  void freeWeakSlot(WeakRefSlot *value);

  /// Shrink the slot storage if there are free slots at the end.
  void shrinkWeakSlots();

  /// Callback to notify GenGC that its size bounds have changed, and any
  /// dependent memory regions should be updated.
  void didResize();

  template <class T>
  friend class WeakRef;

  /// The initial and maximum sizes of the heap.  The maxSize_ is
  /// const after initialization, but may be modified after init in
  /// the GenGC constructor.
  const gcheapsize_t initialSize_;
  gcheapsize_t maxSize_;

  /// The storage that the heap owns.  The generations will be allocated within
  /// this memory.
  BackingStorage storage_;

  /// The generations that make up the heap.
  /// Note: these must be declared and initialized in this order; the YoungGen
  /// constructor takes a pointer to the OldGen as an argument, and assumes it
  /// has been initialized.
  OldGen oldGen_;
  YoungGen youngGen_;

#ifndef NDEBUG
  bool allocInYoung_{true};
#endif

  /// Full heap marking infrastructure.

  /// The mark bits.  Every bit corresponds to a HeapAlign-sized region
  /// in the heap.
  /// TODO: vm_allocate this, and manage its memory properly.
  MarkBitArray markBits_;

  /// Contains the markStack, overflow boolean, and pointer to the
  /// parent object of the object currently being marked.
  CompleteMarkState markState_;

  /// Every bit corresponds to a symbol id. It is set to true if the symbol is
  /// in use (was marked).
  std::vector<bool> markedSymbols_{};

  /// The weak reference slots.
  std::deque<WeakRefSlot> weakSlots_{};

  /// Pointers to the slots (elements of weakSlots_, above) that may
  /// possibly have pointer referents that point into the young generation.
  std::vector<WeakRefSlot *> weakRefSlotsWithPossibleYoungReferent_{};

  /// Pointer to the first free weak reference slot.
  WeakRefSlot *firstFreeWeak_{nullptr};

  /// VTable pointers overwritten with forwarding pointers are stored here.
  std::vector<const VTable *> displacedVtablePtrs_;

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
};

// A special vmcast implementation used during GC.  At some points
// during a mark-sweep-compact GC, the heap becomes invalid: GCCells
// no longer have valid vtables.  When the heap is valid, we do the normal
// checked cast that vmcast does, but when the heap is invalid, we
// just do an unchecked cast.
template <class ToType>
ToType *vmcast_during_gc(GCCell *cell, GC *gc) {
  if (!gc->inGC()) {
    return llvm::cast<ToType>(cell);
  } else {
    return static_cast<ToType *>(cell);
  }
}

template <bool fixedSize, HasFinalizer hasFinalizer>
inline void *GenGC::alloc(uint32_t sz) {
  if (shouldSanitizeHandles())
    swapToFreshHeap();

#ifdef HERMESVM_GC_GENERATIONAL_MARKSWEEPCOMPACT
  AllocResult res = oldGen_.alloc(sz, hasFinalizer);
#else
#ifndef NDEBUG
  AllocResult res = debugAlloc(sz, hasFinalizer, fixedSize);
#else
  // We repeat this in opt, to ensure that the AllocResult is only
  // initialized once.
  AllocResult res = youngGen_.alloc(sz, hasFinalizer, fixedSize);
#endif // NDEBUG
#endif // HERMESVM_GC_GENERATIONAL_MARKSWEEPCOMPACT
  assert(res.success && "Should never fail to allocate at the top level");
  return res.ptr;
}

template <HasFinalizer hasFinalizer>
inline void *GenGC::allocLongLived(uint32_t size) {
  if (shouldSanitizeHandles())
    swapToFreshHeap();

  auto res = oldGen_.alloc(size, hasFinalizer);
  assert(res.success && "Should never fail to allocate at the top level");
  return res.ptr;
}

inline bool GenGC::canAllocExternalMemory(uint32_t size) {
  return size <= maxSize();
}

inline void GenGC::countWriteBarrier() {
#ifndef NDEBUG
  numWriteBarriers_++;
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

inline void GenGC::writeBarrier(void *loc, HermesValue value) {
  // TODO: this version of the write barrier can call the pointer version (after
  // the test that determines that the Hermes value is a pointer).  T24661639.
  countWriteBarrier();
  HERMES_SLOW_ASSERT(contains(loc));
  if (value.isPointer()) {
    char *ptr = reinterpret_cast<char *>(value.getPointer());
    char *locPtr = reinterpret_cast<char *>(loc);
    char *boundary = oldGen_.lowLim();
    if (locPtr >= boundary && ptr < boundary) {
      oldGen_.cardTable().dirtyCardForAddress(locPtr);
    }
  }
}

inline void GenGC::writeBarrier(void *loc, void *value) {
  countWriteBarrier();
  assert(contains(loc));
  char *locPtr = reinterpret_cast<char *>(loc);
  char *valPtr = reinterpret_cast<char *>(value);
  char *boundary = oldGen_.lowLim();
  if (value) {
    assert(contains(value));
    if (locPtr >= boundary && valPtr < boundary) {
      oldGen_.cardTable().dirtyCardForAddress(locPtr);
    }
  }
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

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GENGC_H
