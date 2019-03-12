/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_GCBASE_H
#define HERMES_VM_GCBASE_H

#include "hermes/Platform/Logging.h"
#include "hermes/Public/CrashManager.h"
#include "hermes/Public/GCConfig.h"
#include "hermes/Public/GCTripwireContext.h"
#include "hermes/Support/CheckedMalloc.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/GCDecl.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/HasFinalizer.h"
#include "hermes/VM/HeapAlign.h"
#include "hermes/VM/HeapSnapshot.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/SlotAcceptor.h"
#include "hermes/VM/SlotVisitor.h"
#include "hermes/VM/StorageProvider.h"
#include "hermes/VM/StringRefUtils.h"
#include "hermes/VM/VTable.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/ErrorHandling.h"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <random>
#include <vector>

namespace hermes {
namespace vm {

class GCCell;

// A specific GC class extend GCBase, and override its virtual functions.
// In addition, it must implement the following methods:

/// Allocate a new cell of the specified type \p vt and size \p size. If
/// necessary perform a GC cycle, which may potentially move allocated
/// objects. \p fixedSize should indicate whether the allocation is
/// for a fixed-size, small object; some GCs may allow optimizations
/// on this bases.  \p hasFinalizer must be \p HasFinalizer::Yes if
/// cells of the given type require a finalizer to be called.
///
///   template <bool fixedSize = true,
///             HasFinalizer hasFinalizer = HasFinalizer::No>
///   CallResult<GCCell *> alloc(const VTable *vt, uint32_t size);
///
/// Like the above, but if the GC makes a distinction between short- and
/// long-lived objects, allocates an object that is expected to be
/// long-lived.  Does not allow specification of fixed-sizeness.
///
///   template <HasFinalizer hasFinalizer = HasFinalizer::No>
///   CallResult<GCCell *> allocLongLived(const VTable *vt, uint32_t size);
///
/// In some GCs, objects can have associated memory allocated outside the heap,
/// and this memory can influence GC initiation and heap sizing heuristics.
/// This method tests whether an external memory allocation is too large (e.g.,
/// larger than the max size of the heap):
///
///   bool canAllocExternalMemory(uint32_t size);
///
/// These APIs inform the GC of this external memory.
///
///   void creditExternalMemory(GCCell *alloc, uint32_t size);
///   void debitExternalMemory(GCCell *alloc, uint32_t size);
///
/// Force a garbage collection cycle.
///   void collect();
///
/// The maximum size of any one allocation allowable by the GC in any state.
///   static constexpr uint32_t maxAllocationSize();
///
/// Mark a pointer to a GCCell.
///   template <class T> void mark(T *&ptr);
///
/// Returns true if \p p points into the heap.
///   bool contains(const void *p) const;
///
/// Return the lower bound of the heap's virtual address range (inclusive).
///   char *lowLim() const;
///
/// Return the upper bound of the heap's virtual address range (exclusive).
///   char *hiLim() const;
///
/// In the "mark" functions below, Name is one of const char*, int,
/// unsigned, or const StringPrimitive*:
///
/// Mark a HermesValue which may or may not be a pointer.
///   void mark(HermesValue &hv, Name name);
///   void mark(HermesValue &hv);
///
/// Mark a T* location. This location must be outside the heap.
///   void mark(T *&ptr, Name name);
///   void mark(T *&ptr);
///
/// Mark a GCPointer<T>, which must be within the heap.
///   void mark(GCPointer<T> &ptr, Name name);
///
/// Check if the heap is not valid or not.
/// bool heapIsValid() const;
///
/// Various forms of write barriers: these can have empty implementations
/// for GCs that don't require them:
///
///   The given value is being written at the given loc (required to
///   be in the heap).  If value is a pointer, execute a write barrier.
///      void writeBarrier(void* loc, HermesValue value);
///
///   The given pointer value is being written at the given loc (required to
///   be in the heap).  The value is may be null.  Execute a write barrier.
///      void writeBarrier(void* loc, void* value);
///
///   We copied HermesValues into the given region.  Note that \p numHVs is
///   the number of HermesValues in the the range, not the char length.
///   Do any necessary barriers.
///      void writeBarrierRange(HermesValue* start, uint32_t numHVs);
///
///   We filled numHVs slots starting at start with the given value.
///   Do any necessary barriers.
///      void writeBarrierRangeFill(HermesValue* start, uint32_t numHVs,
///                                 HermesValue value);
///
///   In debug builds: is a write barrier necessary for a write of the given
///   GC pointer \p value to the given \p loc?
///      bool needsWriteBarrier(void *loc, void *value);
///
/// This is intended to be called only from within object or root mark
/// functions and indicates whether the \c mark() operation, called within
/// the current GC phase, is the first such call that guarantees that the
/// location passed to mark will contain the final, correct, pointer value
/// after the mark call.
///   bool isUpdatingPointers() const;
///
/// It must also have the inner type:
///   class Size;
/// Which provides at least these functions publicly:
///   Constructor from either a GCConfig or the min and max heap size.
///     explicit Size(const GCConfig &conf);
///     Size(gcheapsize_t min, gcheapsize_t max);
///   Return the minimum amount of bytes holdable by this heap.
///     gcheapsize_t min() const;
///   Return the maximum amount of bytes holdable by this heap.
///     gcheapsize_t max() const;
///   Return the total amount of bytes of storage this GC will require.
///   This will be a multiple of AlignedStorage::size().
///     gcheapsize_t storageFootprint() const;
///
class GCBase {
 public:
  /// An interface enabling the garbage collector to mark roots and free
  /// symbols.
  struct GCCallbacks {
    /// Virtual destructor to avoid warnings.
    virtual ~GCCallbacks() = 0;

    /// Callback that will be invoked by the GC to mark all roots in the
    /// beginning of every GC by calling "gc->mark()".
    /// The \p markLongLived argument indicates whether root data structures
    /// that contain only references to long-lived objects (allocated
    /// via allocLongLived) are required to be scanned.  A generational
    /// collector, for example, might take advantage of this.
    virtual void markRoots(
        GC *gc,
        SlotAcceptorWithNames &acceptor,
        bool markLongLived = true) = 0;

    /// Callback that will be invoked by the GC to mark all weak roots in the
    /// beginning of every GC.
    virtual void markWeakRoots(GCBase *gc, SlotAcceptorWithNames &acceptor) = 0;

    /// \return one higher than the largest symbol in the identifier table. This
    /// enables the GC to size its internal structures for symbol marking.
    /// Optionally invoked at the beginning of a garbage collection.
    virtual unsigned getSymbolsEnd() const = 0;

    /// Free all symbols which are not marked as \c true in \p markedSymbols.
    /// Optionally invoked at the end of a garbage collection.
    virtual void freeSymbols(const std::vector<bool> &markedSymbols) = 0;

    /// Prints any statistics maintained in the Runtime about GC to \p
    /// os.  At present, this means the breakdown of markRoots time by
    /// "phase" within markRoots.
    virtual void printRuntimeGCStats(llvm::raw_ostream &os) const = 0;

    /// \returns the approximate usage of memory external to the GC such as
    /// malloc by the roots of the object graph.
    virtual size_t mallocSize() const = 0;

    /// Visits every entry in the identifier table and calls acceptor with
    /// the entry as argument. This is intended to be used only for Snapshots,
    /// as it is slow. The function passed as acceptor shouldn't perform any
    /// heap operations.
    virtual void visitIdentifiers(
        const std::function<void(UTF16Ref, uint32_t id)> &acceptor) = 0;
  };

  /// Struct that keeps a reference to a GC.  Useful, for example, as a base
  /// class of Acceptors that need access to the GC.
  struct GCRef {
    GC &gc;
    GCRef(GC &gc) : gc(gc) {}
  };

  /// Gathers summary statistics for a given statistic.
  struct Accumulation {
    /// Update the summary stats with the addition of a new \param value.
    inline void record(double value) {
      sum_ += value;
      sumOfSquares_ += value * value;
      max_ = std::max(max_, value);
    }

    /// Accessors
    inline double sum() const {
      return sum_;
    }

    inline double sumOfSquares() const {
      return sumOfSquares_;
    }

    inline double max() const {
      return max_;
    }

    /// Returns the average of all added values, given \param n, the number of
    /// added values.
    inline double average(unsigned n) const {
      return n == 0 ? 0.0 : sum_ / n;
    }

   private:
    double sum_{0.0};
    double sumOfSquares_{0.0};
    double max_{0.0};
  };

  /// Stats for collections. Time unit, where applicable, is seconds.
  struct CumulativeHeapStats {
    unsigned numCollections{0};

    /// Summary statistics for GC wall times.
    Accumulation gcWallTime;

    /// Summary statistics for GC CPU times.
    Accumulation gcCPUTime;

    gcheapsize_t finalHeapSize{0};

    double avgGCWallTime() {
      return gcWallTime.average(numCollections);
    }

    double avgGCCPUTime() {
      return gcCPUTime.average(numCollections);
    }
  };

  struct HeapInfo {
    /// Number of collections occurred.
    unsigned numCollections{0};
    /// Total number of bytes allocated since the beginning of runtime
    /// execution.
    unsigned totalAllocatedBytes{0};
    /// Number of currently allocated bytes in the heap. Some may be
    /// unreachable.
    gcheapsize_t allocatedBytes{0};
    /// Size of the active heap (doesn't include the "to-space").
    gcheapsize_t heapSize{0};
    /// Estimate of amount of malloc space used by objects in the heap.
    /// Calculated by querying each object to report its malloc usage.
    unsigned mallocSizeEstimate{0};
    /// The total amount of Virtual Address space (VA) that the GC is using.
    uint64_t va{0};
    /// Stats for full collections (zeroes if non-generational GC).
    CumulativeHeapStats fullStats;
    /// Stats for collections in the young generation (zeroes if
    /// non-generational GC).
    CumulativeHeapStats youngGenStats;
  };

#ifndef NDEBUG
  struct DebugHeapInfo {
    /// Number of currently allocated objects present in the heap. Some may be
    /// unreachable.
    unsigned numAllocatedObjects{0};
    /// Number of reachable objects in the last collection.
    unsigned numReachableObjects{0};
    /// Number of collected objects in the last collection.
    unsigned numCollectedObjects{0};
    /// Number of finalized objects in the last collection.
    unsigned numFinalizedObjects{0};
    /// Number of marked symbols.
    unsigned numMarkedSymbols{0};
    /// Number of hidden classes alive after the last collection.
    unsigned numHiddenClasses{0};
    /// Number of "leaf" hidden classes alive after the last collection.
    unsigned numLeafHiddenClasses{0};

    // Assert any invariants that should hold among the fields of the
    // DebugHeapInfo.
    void assertInvariants() const;
  };
#endif

  GCBase(
      MetadataTable metaTable,
      GCCallbacks *gcCallbacks,
      const GCConfig &gcConfig,
      std::shared_ptr<CrashManager> crashMgr,
      // Do nothing with this in the default case, only NCGen needs this.
      StorageProvider *)
      : metaTable_(metaTable),
        gcCallbacks_(gcCallbacks),
        crashMgr_(crashMgr),
        recordGcStats_(gcConfig.getShouldRecordStats()),
        name_(gcConfig.getName()),
        tripwireCallback_(gcConfig.getTripwireConfig().getCallback()),
        tripwireLimit_(gcConfig.getTripwireConfig().getLimit()),
        tripwireCooldown_(gcConfig.getTripwireConfig().getCooldown())
#ifdef HERMESVM_SANITIZE_HANDLES
        ,
        sanitizeRate_(gcConfig.getSanitizeRate())
#endif
#ifndef NDEBUG
        ,
        randomizeAllocSpace_(gcConfig.getShouldRandomizeAllocSpace())
#endif
  {
#ifdef HERMESVM_PLATFORM_LOGGING
    hermesLog(
        "HermesGC",
        "Initialisation (Init: %dMB, Max: %dMB, Tripwire: %dMB/%" PRId64 "h)",
        gcConfig.getInitHeapSize() >> 20,
        gcConfig.getMaxHeapSize() >> 20,
        gcConfig.getTripwireConfig().getLimit() >> 20,
        static_cast<int64_t>(
            gcConfig.getTripwireConfig().getCooldown().count()));
#endif // HERMESVM_PLATFORM_LOGGING
#ifdef HERMESVM_SANITIZE_HANDLES
    std::minstd_rand::result_type seed = std::random_device()();
    if (sanitizeRate_ > 0.0 && sanitizeRate_ < 1.0) {
      llvm::errs() << "Sanitize Rate Seed: " << seed << '\n';
    }
    randomEngine_.seed(seed);
#endif
  }

  virtual ~GCBase() {}

  /// \return true if we should run handle sanitization and the coin flip with
  /// probability sanitizeRate_ has passed.
#ifdef HERMESVM_SANITIZE_HANDLES
  bool shouldSanitizeHandles();
#else
  static constexpr bool shouldSanitizeHandles() {
    return false;
  }
#endif

  /// \return true if the "target space" for allocations should be randomized
  /// (for GCs where that concept makes sense).
  bool shouldRandomizeAllocSpace() const {
    return randomizeAllocSpace_;
  }

  /// Name to indentify this heap in logs.
  const std::string &getName() const {
    return name_;
  }

  /// Called by the Runtime to inform the GC that it is about to execute JS for
  /// the first time.
  void runtimeWillExecute();

  /// Inform the GC that TTI has been reached.  (In case, for example,
  /// behavior should change at that point.  Default behavior is to do
  /// nothing.)
  void ttiReached() {}

  /// Do anything necessary to record the current number of allocated
  /// objects in numAllocatedObjects_.  Default is to do nothing.
  virtual void recordNumAllocatedObjects() {}

  /// Print any and all collected statistics to the give output stream, \p os.
  void printAllCollectedStats(llvm::raw_ostream &os);

  /// Total number of collections of any kind.
  unsigned getNumGCs() const {
    return cumStats_.numCollections;
  }

  /// Total wall time in seconds of all pauses due to collections so far.
  double getGCTime() const {
    return cumStats_.gcWallTime.sum();
  }

  /// Total CPU time in seconds of all pauses due to collections so far.
  double getGCCPUTime() const {
    return cumStats_.gcCPUTime.sum();
  }

  /// Populate \p info with information about the heap.
  virtual void getHeapInfo(HeapInfo &info);
  /// Same as \c getHeapInfo, and it adds the amount of malloc memory in use.
  virtual void getHeapInfoWithMallocSize(HeapInfo &info) = 0;

#ifndef NDEBUG
  /// Populate \p info with more detailed information about the heap that is
  /// too expensive to know during production builds.
  virtual void getDebugHeapInfo(DebugHeapInfo &info);
#endif

  /// Dump detailed heap contents to the given output stream, \p os.
  virtual void dump(llvm::raw_ostream &os, bool verbose = false);

  /// Do any logging of info about the heap that is useful, then dies with a
  /// fatal out-of-memory error.
  LLVM_ATTRIBUTE_NORETURN void oom();

  /// Creates a snapshot of the heap and writes it to the given \p fileName.
  /// \p compact whether to write a compact version or a pretty human-readable
  ///   version.
  /// \return true on success, false on failure.
  bool createSnapshotToFile(const std::string &fileName, bool compact = true);
  /// Creates a snapshot of the heap, which includes information about what
  /// objects exist, their sizes, and what they point to.
  virtual void createSnapshot(llvm::raw_ostream &os, bool compact = true) = 0;

  /// Default implementations for the external memory credit/debit APIs: do
  /// nothing.
  void creditExternalMemory(GCCell *alloc, uint32_t size) {}
  void debitExternalMemory(GCCell *alloc, uint32_t size) {}

  /// Default implementations for write barriers: do nothing.
  inline void writeBarrier(void *loc, HermesValue value) {}
  inline void writeBarrier(void *loc, void *value) {}
  inline void writeBarrierRange(HermesValue *start, uint32_t numHVs) {}
  inline void writeBarrierRangeFill(
      HermesValue *start,
      uint32_t numHVs,
      HermesValue value) {}
#ifndef NDEBUG
  bool needsWriteBarrier(void *loc, void *value) {
    return false;
  }
#endif

  /// @name Marking APIs
  /// @{

  /// Marks a cell by its metadata.
  /// \p cell The heap object to mark.
  /// \p gc The GC that owns the cell.
  /// \p acceptor The action to perform on each slot in the cell.
  template <typename Acceptor>
  static inline void markCell(GCCell *cell, GC *gc, Acceptor &acceptor);

  /// Same as the normal \c markCell, but for cells that don't have a valid
  /// vtable pointer.
  template <typename Acceptor>
  static inline void
  markCell(GCCell *cell, const VTable *vt, GC *gc, Acceptor &acceptor);

  /// Same as the normal \c markCell, but takes a visitor instead.
  template <typename Acceptor>
  static inline void markCell(
      SlotVisitor<Acceptor> &visitor,
      GCCell *cell,
      const VTable *vt,
      GC *gc);

  /// Marks a cell by its metadata, but only for the slots that point between
  /// [begin, end).
  template <typename Acceptor>
  static inline void markCellWithinRange(
      SlotVisitor<Acceptor> &visitor,
      GCCell *cell,
      const VTable *vt,
      GC *gc,
      const char *begin,
      const char *end);

  /// Marks a cell by its metadata, and outputs the names of the slots.
  /// Meant to be used by heap snapshots.
  template <typename Acceptor>
  static inline void markCellWithNames(
      SlotVisitorWithNames<Acceptor> &visitor,
      GCCell *cell,
      const VTable *vt,
      GC *gc);

  /// @}

  /// Get the next unique object ID for a newly created object.
  uint64_t nextObjectID();

  using TimePoint = std::chrono::steady_clock::time_point;
  /// Return the difference between the two time points (end - start)
  /// as a double representing the number of seconds in the duration.
  static double clockDiffSeconds(TimePoint start, TimePoint end);

  /// Return the difference between the two durations (end - start) given in
  /// microseconds as a double representing the number of seconds in the
  /// difference.
  static double clockDiffSeconds(
      std::chrono::microseconds start,
      std::chrono::microseconds end);

// Mangling scheme used by MSVC encode public/private into the name.
// As a result, vanilla "ifdef public" trick leads to link errors.
#if defined(UNIT_TEST) || defined(_MSC_VER)
 public:
#else
 protected:
#endif

  /// dataSize is the live data in bytes, now is the current time point. The
  /// function checks these parameters against the limits set at initialisation.
  /// If the conditions are met, the tripwire is triggered and tripwireCallback_
  /// is called.
  void checkTripwire(
      size_t dataSize,
      std::chrono::time_point<std::chrono::steady_clock> now);

  // Visibility here is public for unit_tests and protected otherwise

 protected:
  /// Returns the number of bytes allocated allocated since the last GC.
  /// TODO: Implement this for heaps other than GenGC
  /// (at which point this can become an abstract function).
  virtual gcheapsize_t bytesAllocatedSinceLastGC() const {
    return 0;
  }

  /// Convenience method to invoke the mark weak roots function provided at
  /// initialization, using the context provided then (on this heap).
  void markWeakRoots(SlotAcceptorWithNames &acceptor) {
    gcCallbacks_->markWeakRoots(this, acceptor);
  }

  /// Print the cumulative statistics.
  /// \p os The output stream to print the stats to.
  /// \p trailingComma true if the end of the JSON string should have a trailing
  /// comma (anticipating more objects added after it).
  virtual void printStats(llvm::raw_ostream &os, bool trailingComma);

  /// Record statistics from a single GC, which took \p wallTime seconds wall
  /// time and \p cpuTime seconds CPU time to run the gc and left the heap size
  /// at the given \p finalHeapSize, in the given cumulative stats struct.
  void recordGCStats(
      double wallTime,
      double cpuTime,
      gcheapsize_t finalHeapSize,
      CumulativeHeapStats *stats);

  /// Record statistics from a single GC, which took \p wallTime seconds wall
  /// time and \p cpuTime seconds CPU time to run the gc and left the heap size
  /// at the given \p finalHeapSize, in the overall cumulative stats struct.
  void
  recordGCStats(double wallTime, double cpuTime, gcheapsize_t finalHeapSize);

  /// Do any additional GC-specific logging that is useful before dying with
  /// out-of-memory.
  virtual void oomDetail();

  /// Number of finalized objects in the last collection.
  unsigned numFinalizedObjects_{0};

  /// The total number of bytes allocated in the execution.
  uint64_t totalAllocatedBytes_{0};

/// These fields are not available in optimized builds.
#ifndef NDEBUG
  /// Number of currently allocated objects present in the heap before the start
  /// of the last collection.  Some may be unreachable.
  unsigned numAllocatedObjects_{0};
  /// Number of reachable objects in the last collection.  (More properly, this
  /// is the number not known to be unreachable: if a GC does not consider
  /// determine the reachability of some subset of objects, for example, an old
  /// generation in a generational collection, those objects should be included
  /// in this count.)
  unsigned numReachableObjects_{0};
  /// Number of collected objects in the last collection.  Equal to
  /// numAllocatedObjects_ (at the start of the last collection),
  /// minus numReachableObjects_ found in that collection.
  unsigned numCollectedObjects_{0};
  /// Number of marked symbols.
  unsigned numMarkedSymbols_{0};
  /// Number of hidden classes alive after the last collection.
  unsigned numHiddenClasses_{0};
  /// Number of "leaf" hidden classes alive after the last collection.
  unsigned numLeafHiddenClasses_{0};
#endif

#ifdef HERMESVM_GCCELL_ID
  /// Associate a semi-unique (until it overflows) id with every allocation
  /// for easier identification when debugging.
  uint64_t debugAllocationCounter_{0};
#endif

  /// The table to retrieve metadata about each cell kind.
  const MetadataTable metaTable_;

  /// User-supplied callbacks invoked by the GC to query information or perform
  /// tasks.
  GCCallbacks *const gcCallbacks_;

  /// A place to log crash data if a crash is about to occur.
  std::shared_ptr<CrashManager> crashMgr_;

  /// Whether to output GC statistics at the end of execution.
  bool recordGcStats_{false};
  /// Time at which execution of the Hermes VM began.
  std::chrono::time_point<std::chrono::steady_clock> execStartTime_;
  std::chrono::microseconds execStartCPUTime_;
  // The cumulative GC stats.
  CumulativeHeapStats cumStats_;

  /// Name to indentify this heap in logs.
  std::string name_;

 private:
  /// Callback called if it's not null when the Live Data Tripwire is triggered
  std::function<void(GCTripwireContext &)> tripwireCallback_;

  /// Maximum size limit before the heap size tripwire will trigger
  gcheapsize_t tripwireLimit_;

  /// Time in hours before the tripwire can trigger again after it is
  /// triggered
  std::chrono::hours tripwireCooldown_;

  /// Time when the tripwire can be activated again
  std::chrono::time_point<std::chrono::steady_clock> nextTripwireMinTime_{
      std::chrono::steady_clock::now()};

  /// Variable that saves whether the callback for the live data tripwire is
  /// already running
  bool liveDataTripwireCallbackRunning_{false};

#ifdef HERMESVM_SANITIZE_HANDLES
  /// Whether to keep moving the heap around to detect unsanitary GC handles.
  double sanitizeRate_{1.0};

  /// PRNG for sanitizing at a less than 1.0 rate.
  std::minstd_rand randomEngine_;
#else
  /// Sanitize handles is completely disabled (and ignored at runtime) without
  /// a special build mode.
  static constexpr double sanitizeRate_{0.0};
#endif

/// Whether to randomize the "target space" for allocations, for GC's in which
/// this concept makes sense. Only available in debug builds.
#ifndef NDEBUG
  bool randomizeAllocSpace_{false};
#else
  static const bool randomizeAllocSpace_{false};
#endif
};

// Utilities for formatting time durations and memory sizes.

/// An object that, written to an ostream, formats the given # of
/// secs in appropriate units (down to microseconds).
struct DurationFormatObj {
  double secs;
};
llvm::raw_ostream &operator<<(
    llvm::raw_ostream &os,
    const DurationFormatObj &dfo);
inline DurationFormatObj formatSecs(double secs) {
  return {secs};
}

/// An object that, written to an ostream, formats the given # of
/// bytes in appropriate units (bytes to GiB).
struct SizeFormatObj {
  gcheapsize_t bytes;
};
llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const SizeFormatObj &sfo);
inline SizeFormatObj formatSize(gcheapsize_t size) {
  return {size};
}

/// This is a single slot in the weak reference table. It contains a
/// \c HermesValue which may refer to a GC managed object. If it does, the GC
/// will make sure it is updated when the object is moved; it the object is
/// freed, the value will be set to empty.
struct WeakRefSlot {
  PinnedHermesValue value;
  /// Extra data for use by the GC.
  unsigned extra;
};

/// This is a concrete base of \c WeakRef<T> that can be passed to concrete
/// functions in GC.
class WeakRefBase {
 protected:
  friend GC;

  WeakRefSlot *slot_;
  WeakRefBase(WeakRefSlot *slot) : slot_(slot) {}

#ifdef UNIT_TEST
 public:
  const WeakRefSlot *getSlot() const {
    return slot_;
  };
#endif
};

/// gcc 4.8 doesn't support std::aligned_alloc() so we have to do with regular
/// plain old malloc(). It is guaranteed to align things suitably for double
/// at least, which which satisfies our alignment requirements for now. Should
/// our HeapAlign constant increase, we will need to upgrade to a newer version
/// of gcc, or resort to other tricks.
inline void *alignedMalloc(size_t size) {
  static_assert(
      hermes::vm::HeapAlign == alignof(double),
      "System malloc() cannot guarantee required alignment");
  return checkedMalloc(size);
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCBASE_H
