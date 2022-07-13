/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GCBASE_H
#define HERMES_VM_GCBASE_H

#include "hermes/Inst/Inst.h"
#include "hermes/Platform/Logging.h"
#include "hermes/Public/CrashManager.h"
#include "hermes/Public/GCConfig.h"
#include "hermes/Public/GCTripwireContext.h"
#include "hermes/Support/CheckedMalloc.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/Support/StatsAccumulator.h"
#include "hermes/VM/AllocOptions.h"
#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/CompressedPointer.h"
#include "hermes/VM/GCDecl.h"
#include "hermes/VM/GCExecTrace.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/HeapAlign.h"
#include "hermes/VM/HeapSnapshot.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/SlotAcceptor.h"
#include "hermes/VM/SlotVisitor.h"
#include "hermes/VM/SmallHermesValue.h"
#include "hermes/VM/StackTracesTree-NoRuntime.h"
#include "hermes/VM/StorageProvider.h"
#include "hermes/VM/StringRefUtils.h"
#include "hermes/VM/VTable.h"
#include "hermes/VM/WeakRefSlot.h"

#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/BitVector.h"
#include "llvh/ADT/DenseMap.h"
#include "llvh/Support/ErrorHandling.h"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <list>
#include <random>
#include <system_error>
#include <vector>

namespace hermes {
namespace vm {

/// Forward declarations;
namespace detail {
struct WeakRefKey;
}
template <CellKind C>
class JSWeakMapImpl;
using JSWeakMap = JSWeakMapImpl<CellKind::JSWeakMapKind>;

class GCCell;

#ifdef HERMESVM_GC_RUNTIME
#define RUNTIME_GC_KINDS GC_KIND(HadesGC)
#endif

/// Used by XorPtr to separate encryption keys between uses.
enum XorPtrKeyID { ArrayBufferData, JSFunctionCodeBlock, _NumKeys };

// A specific GC class extend GCBase, and override its virtual functions.
// In addition, it must implement the following methods:

/// Allocate a new cell of type \p T and size \p size. Instantiate an object of
/// type \p T in the newly allocated cell, using \p args as the arguments to its
/// constructor. If necessary perform a GC cycle, which may potentially move
/// allocated objects. \p fixedSize should indicate whether the allocation is
/// for a fixed-size, small object; some GCs may allow optimizations on this
/// basis. \p hasFinalizer must be \p HasFinalizer::Yes if cells of the given
/// type require a finalizer to be called.
/// \pre size must be heap-aligned.
///
///   template <
///     typename T,
///     bool fixedSize = true,
///     HasFinalizer hasFinalizer = HasFinalizer::No,
///     LongLived longLived = LongLived::No,
///     class... Args>
/// inline T *makeA(uint32_t size, Args &&... args);
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
/// Force a garbage collection cycle. The provided cause will be used in
/// logging.
///   void collect(std::string cause);
///
/// The maximum size of any one allocation allowable by the GC in any state.
///   static constexpr uint32_t maxAllocationSizeImpl();
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
/// Iterate over all objects in the heap.
///   void forAllObjs(const std::function<void(GCCell *)> &callback);
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
/// Return true if the GC is active and is calling into the VM.
/// If true, some objects in the heap may have an invalid VTable pointer.
///   bool calledByGC() const;
///
/// Various forms of write barriers: these can have empty implementations
/// for GCs that don't require them:
///
///   The given value is being written at the given loc (required to
///   be in the heap).  If value is a pointer, execute a write barrier.
///     void writeBarrier(GCHermesValue *loc, HermesValue value);
///     void writeBarrier(
///         const GCSmallHermesValue *loc,
///         SmallHermesValue value);
///
///   The given pointer value is being written at the given loc (required to
///   be in the heap).  The value is may be null.  Execute a write barrier.
///     void writeBarrier(const GCPointerBase *loc, const GCCell *value);
///
///   The given value/pointer is being written at a previously uninitialized loc
///   (required to be in the heap).
///     void constructorWriteBarrier(
///         const GCHermesValue *loc,
///         HermesValue value);
///     void constructorWriteBarrier(
///         const GCSmallHermesValue *loc,
///         SmallHermesValue value);
///     void constructorWriteBarrier(
///         const GCPointerBase *loc,
///         const GCCell *value);
///
///   A weak ref is about to be read. Executes a read barrier so the GC can
///   take action such as extending the lifetime of the reference. The
///   HermesValue version does nothing if the value isn't a pointer.
///     void weakRefReadBarrier(GCCell *value);
///
///   We copied HermesValues into the given region.  Note that \p numHVs is
///   the number of HermesValues in the the range, not the char length.
///   Do any necessary barriers.
///     void writeBarrierRange(GCHermesValue* start, uint32_t numHVs);
///     void writeBarrierRange(
///         const GCSmallHermesValue *start, uint32_t
///         numHVs);
///     void constructorWriteBarrierRange(
///         const GCHermesValue *start,
///         uint32_t numHVs);
///     void constructorWriteBarrierRange(
///         const GCSmallHermesValue *start,
///         uint32_t numHVs);
///
///   The given loc or region is about to be overwritten, but the new value is
///   not important. Perform any necessary barriers.
///     void snapshotWriteBarrier(const GCHermesValue *loc);
///     void snapshotWriteBarrier(const GCSmallHermesValue *loc);
///     void snapshotWriteBarrier(const GCPointerBase *loc);
///     void snapshotWriteBarrier(const GCSymboldID *symbol);
///     void snapshotWriteBarrierRange(
///         const GCHermesValue *start,
///         uint32_t numHVs);
///     void snapshotWriteBarrierRange(
///         const GCSmallHermesValue *start,
///         uint32_t numHVs);
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
  static const char kNaturalCauseForAnalytics[];
  static const char kHandleSanCauseForAnalytics[];

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
        RootAndSlotAcceptorWithNames &acceptor,
        bool markLongLived = true) = 0;

    /// Callback that will be invoked by the GC to mark all weak roots in the
    /// beginning of every GC.
    virtual void markWeakRoots(
        WeakRootAcceptor &weakAcceptor,
        bool markLongLived = true) = 0;

    /// Callback that might be invoked by the GC before it completes marking.
    /// Not all GCs will call this. It should be used to mark any roots that
    /// might change without executing proper read and write barriers on the GC.
    /// An example would be something that skipped weak ref read barriers in a
    /// signal handler. If they aren't marked, they would be collected as
    /// garbage.
    /// While it is possible to implement this just as forwarding to
    /// \c markRoots, to be faster it should try to mark only things that would
    /// not have been properly doing barriers.
    virtual void markRootsForCompleteMarking(
        RootAndSlotAcceptorWithNames &acceptor) = 0;

    /// \return one higher than the largest symbol in the identifier table. This
    /// enables the GC to size its internal structures for symbol marking.
    /// Optionally invoked at the beginning of a garbage collection.
    virtual unsigned getSymbolsEnd() const = 0;

    /// If any symbols are marked by the IdentifierTable, clear that marking.
    /// Optionally invoked at the beginning of some collections.
    virtual void unmarkSymbols() = 0;

    /// Free all symbols which are not marked as \c true in \p markedSymbols.
    /// Optionally invoked at the end of a garbage collection.
    virtual void freeSymbols(const llvh::BitVector &markedSymbols) = 0;

    /// Prints any statistics maintained in the Runtime about GC to \p
    /// os.  At present, this means the breakdown of markRoots time by
    /// "phase" within markRoots.
    virtual void printRuntimeGCStats(JSONEmitter &json) const = 0;

    /// \returns the approximate usage of memory external to the GC such as
    /// malloc by the roots of the object graph.
    virtual size_t mallocSize() const = 0;

    /// Visits every entry in the identifier table and calls acceptor with
    /// the entry as argument. This is intended to be used only for Snapshots,
    /// as it is slow. The function passed as acceptor shouldn't perform any
    /// heap operations.
    virtual void visitIdentifiers(
        const std::function<void(SymbolID, const StringPrimitive *)>
            &acceptor) = 0;

    /// Convert the given symbol into its UTF-8 string representation.
    /// \post The implementation of this function must not perform any GC
    ///   operations, such as allocations, mutating values in the heap, or
    ///   making handles.
    virtual std::string convertSymbolToUTF8(SymbolID id) = 0;

    /// Returns the current stack as a string. This function will not cause
    /// any allocs in the GC.
    virtual std::string getCallStackNoAlloc() = 0;

    /// This is called with CollectionStart at the start of each GC, and with
    /// CollectionEnd at the end.
    /// \param extraInfo contains more detailed extra info for specific GC.
    virtual void onGCEvent(GCEventKind kind, const std::string &extraInfo) = 0;

    /// Return the current VM instruction pointer which can be used to derive
    /// the current VM stack-trace. It's "slow" because it's virtual.
    virtual const inst::Inst *getCurrentIPSlow() const = 0;

#ifdef HERMES_MEMORY_INSTRUMENTATION
    /// Return a \c StackTracesTreeNode representing the current VM stack-trace
    /// at this point.
    virtual StackTracesTreeNode *getCurrentStackTracesTreeNode(
        const inst::Inst *ip) = 0;

    /// Get a StackTraceTree which can be used to recover stack-traces from \c
    /// StackTraceTreeNode() as returned by \c getCurrentStackTracesTreeNode() .
    virtual StackTracesTree *getStackTracesTree() = 0;
#endif

#ifdef HERMES_SLOW_DEBUG
    /// \return true if the given symbol is a live entry in the identifier
    /// table.
    /// NOTE: Used by CheckHeapWellFormedAcceptor to make sure a symbol
    /// that is discovered live is marked as live.
    virtual bool isSymbolLive(SymbolID id) = 0;

    /// \return An associated heap cell for the symbol if one exists, null
    /// otherwise.
    virtual const void *getStringForSymbol(SymbolID id) = 0;
#endif
  };

  /// Struct that keeps a reference to a GC.  Useful, for example, as a base
  /// class of Acceptors that need access to the GC.
  struct GCRef {
    GC &gc;
    GCRef(GC &gc) : gc(gc) {}
  };

  /// Stats for collections. Time unit, where applicable, is seconds.
  struct CumulativeHeapStats {
    unsigned numCollections{0};

    /// Summary statistics for GC wall times. For Hades, this should only track
    /// time spent on the mutator.
    StatsAccumulator<double> gcWallTime;

    /// Summary statistics for GC CPU times.
    StatsAccumulator<double> gcCPUTime;

    gcheapsize_t finalHeapSize{0};

    /// Bytes allocated just before a collection.
    StatsAccumulator<gcheapsize_t, uint64_t> usedBefore;

    /// Bytes alive after a collection.
    StatsAccumulator<gcheapsize_t, uint64_t> usedAfter;
  };

  struct HeapInfo {
    /// Number of garbage collections (of any kind) since creation.
    unsigned numCollections{0};
    /// Total (cumulative) bytes allocated within the JS heap since creation.
    uint64_t totalAllocatedBytes{0};
    /// Number of currently allocated bytes within the JS heap. Some may be
    /// in unreachable objects (unless a full collection just occurred).
    uint64_t allocatedBytes{0};
    /// Current capacity of the JS heap, in bytes.
    uint64_t heapSize{0};
    /// Estimate of amount of current malloc space used by the runtime and any
    /// auxiliary allocations owned by heap objects. (Calculated by querying
    /// each finalizable object to report its malloc usage.)
    unsigned mallocSizeEstimate{0};
    /// The total amount of Virtual Address space (VA) that the GC is using.
    uint64_t va{0};
    /// Number of bytes retained by objects as external memory on the C++ heap.
    /// This is typically associated with allocations that are modified
    /// infrequently, and can therefore be stored in a counter in the GC, making
    /// them cheaper to query. This is a subset of mallocSizeEstimate.
    uint64_t externalBytes{0};
    /// Cumulative number of mark stack overflows in full collections
    /// (zero if non-generational GC).
    unsigned numMarkStackOverflows{0};
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

#ifdef HERMES_MEMORY_INSTRUMENTATION
  /// When enabled, every allocation gets an attached stack-trace and an
  /// object ID. When disabled old allocations continue to be tracked but
  /// no new allocations get a stack-trace.
  struct AllocationLocationTracker final {
    explicit AllocationLocationTracker(GCBase *gc);

    /// Returns true if tracking is enabled for new allocations.
    bool isEnabled() const;
    /// Must be called by GC implementations whenever a new allocation is made.
    void newAlloc(const GCCell *ptr, uint32_t sz);

    /// If an object's size changes, update the entry here.
    void updateSize(const GCCell *ptr, uint32_t oldSize, uint32_t newSize);

    /// Must be called by GC implementations whenever an allocation is freed.
    void freeAlloc(const GCCell *ptr, uint32_t sz);
    /// Returns data needed to reconstruct the JS stack used to create the
    /// specified allocation.
    StackTracesTreeNode *getStackTracesTreeNodeForAlloc(
        HeapSnapshot::NodeID id) const;

    /// A Fragment is a time bound for when objects are allocated. Any
    /// allocations that occur before the lastSeenObjectID_ are in this
    /// fragment. Allocations increment the numObjects_ and numBytes_. Free'd
    /// cells from this fragment decrement numObjects_ and numBytes_.
    struct Fragment {
      HeapSnapshot::NodeID lastSeenObjectID_;
      std::chrono::microseconds timestamp_;
      /// Number of objects still alive in this fragment. Incremented when
      /// objects are created, decremented when objects are destroyed.
      uint64_t numObjects_;
      /// Total size of objects still alive in this fragment.
      uint64_t numBytes_;
      /// If true, one of numObjects or numBytes changed since the last flush.
      bool touchedSinceLastFlush_;
    };

    /// This must match the definition in jsi::Instrumentation to avoid
    /// unnecessary copying.
    using HeapStatsUpdate = std::tuple<uint64_t, uint64_t, uint64_t>;

    /// Enable location tracking.
    void enable(std::function<void(
                    uint64_t,
                    std::chrono::microseconds,
                    std::vector<HeapStatsUpdate>)> callback);

    /// Disable location tracking - turns \c newAlloc() into a no-op. Existing
    /// allocations continue to be tracked.
    void disable();

    /// Flush the current fragment and write all flushed fragments to \p snap.
    void addSamplesToSnapshot(HeapSnapshot &snap);

   private:
    /// Flush out heap profiler data to the callback after a new kFlushThreshold
    /// bytes are allocated.
    static constexpr uint64_t kFlushThreshold = 128 * (1 << 10);
    /// This mutex protects stackMap_ and fragments_. Specifically does not
    /// protect enabled_, because enabled_ should only be changed while the GC
    /// isn't running anyway.
    Mutex mtx_;
    /// Associates allocations at their current location with their stack trace
    /// data.
    llvh::DenseMap<HeapSnapshot::NodeID, StackTracesTreeNode *> stackMap_;
    /// We need access to the GCBase to collect the current stack when nodes are
    /// allocated.
    GCBase *gc_;
    /// Indicates if tracking of new allocations is enabled.
    bool enabled_{false};
    /// Time when the profiler was started.
    std::chrono::steady_clock::time_point startTime_;
    /// This should be called periodically whenever the last seen object ID is
    /// updated.
    std::function<
        void(uint64_t, std::chrono::microseconds, std::vector<HeapStatsUpdate>)>
        fragmentCallback_;
    /// All samples that have been flushed. Only needs the last object ID to be
    /// written to the file.
    std::vector<Fragment> fragments_;

    /// Updates the last fragment to have the current last ID and timestamp,
    /// then calls fragmentCallback_ with both the new fragment and any changed
    /// fragments from freeAlloc.
    void flushCallback();

    /// Find the fragment corresponding to the given id.
    /// \return fragments_.back() if none exists (it's the currently active
    ///   fragment).
    Fragment &findFragmentForID(HeapSnapshot::NodeID id);
  };

  class SamplingAllocationLocationTracker final {
   public:
    explicit inline SamplingAllocationLocationTracker(GCBase *gc) : gc_(gc) {}

    /// Returns true if tracking is enabled for new allocations.
    bool isEnabled() const {
      return !!dist_;
    }

    /// Must be called by GC implementations whenever a new allocation is made.
    void newAlloc(const GCCell *ptr, uint32_t sz);

    /// Must be called by GC implementations whenever an allocation is freed.
    void freeAlloc(const GCCell *ptr, uint32_t sz);

    /// If an object's size changes, update the entry here.
    void updateSize(const GCCell *ptr, uint32_t oldSize, uint32_t newSize);

    /// Turn the sampling memory profiler on. About once every
    /// \p samplingInterval bytes are allocated, sample the allocation by
    /// recording its stack.
    /// \param seed If non-negative, use as the seed for the random sampling
    ///   mechanism, giving deterministic output.
    void enable(size_t samplingInterval, int64_t seed);

    void disable(llvh::raw_ostream &os);

   private:
    struct Sample final {
      size_t size;
      StackTracesTreeNode *node;
      /// This is the auto-incremented sample ID, not the ID of the object
      /// associated with the sample.
      uint64_t id;
    };

    /// This mutex protects stackMap_ and samples_. Not needed for enabling and
    /// disabling because those only happen while the world is stopped.
    Mutex mtx_;

    GCBase *gc_;

    /// Subtract from this each allocation. If it would underflow below zero,
    /// take a sample.
    size_t limit_{0};

    /// Track all samples that have been taken.
    llvh::DenseMap<HeapSnapshot::NodeID, Sample> samples_;

    /// Use a poisson distribution to decide when to take the next sample.
    std::minstd_rand randomEngine_;
    std::unique_ptr<std::poisson_distribution<>> dist_;

    /// An auto-incrementing integer representing a unique ID for a sample.
    /// Used for ordering samples.
    uint64_t nextSampleID_{1};

    /// \return How many bytes should be waited until the next sample.
    size_t nextSample();
  };
#endif

  class IDTracker final {
   public:
    /// These are IDs that are reserved for special objects.
    enum class ReservedObjectID : HeapSnapshot::NodeID {
      // The ID for the super root object.
      SuperRoot,
      // The ID for the (GC roots) object.
      GCRoots,
#define ROOT_SECTION(name) name,
#include "hermes/VM/RootSections.def"
      IdentifierTableLookupVector,
      IdentifierTableHashTable,
      IdentifierTableMarkedSymbols,
      JSIHermesValueList,
      JSIWeakHermesValueList,
      WeakRefSlotStorage,
      Undefined,
      Null,
      True,
      False,
      FirstNonReservedID,
    };

    // 0 is guaranteed to never be a valid node ID.
    static constexpr HeapSnapshot::NodeID kInvalidNode = 0;

    static constexpr HeapSnapshot::NodeID reserved(ReservedObjectID id) {
      // All reserved IDs should be odd numbers to signify that they're JS
      // objects and not native objects. This follows v8's output.
      return static_cast<std::underlying_type<ReservedObjectID>::type>(id) * 2 +
          1;
    }

    /// A comparator for doubles that allows NaN.
    struct DoubleComparator {
      static double getEmptyKey() {
        // Use a non-canonical NaN value as an empty value, which should never
        // occur naturally.
        // NOTE: HermesValue uses NaN tagging internally so we can use that to
        // get the encoding.
        return llvh::BitsToDouble(HermesValue::encodeUndefinedValue().getRaw());
      }
      static double getTombstoneKey() {
        // Use a non-canonical NaN value as the tombstone, which should never
        // occur naturally.
        // NOTE: HermesValue uses NaN tagging internally so we can use that to
        // get the encoding.
        return llvh::BitsToDouble(HermesValue::encodeNullValue().getRaw());
      }
      static unsigned getHashValue(double val) {
        return std::hash<uint64_t>{}(llvh::DoubleToBits(val));
      }
      static bool isEqual(double LHS, double RHS) {
        return llvh::DoubleToBits(LHS) == llvh::DoubleToBits(RHS);
      }
    };

    explicit IDTracker();

    /// Return true if IDs are being tracked.
    bool isTrackingIDs();

    /// Get the unique object id of the given object.
    /// If one does not yet exist, start tracking it.
    HeapSnapshot::NodeID getObjectID(CompressedPointer cell);

    /// \return true if the cell has an object ID associated with it, false if
    ///   there is none.
    bool hasObjectID(CompressedPointer cell);

    /// Same as \c getObjectID, except it asserts if the cell doesn't have an
    /// ID.
    HeapSnapshot::NodeID getObjectIDMustExist(CompressedPointer cell);

    /// Get the unique object id of the symbol with the given symbol \p sym. If
    /// one does not yet exist, start tracking it.
    HeapSnapshot::NodeID getObjectID(SymbolID sym);

    /// Get the unique object id of the given native memory (non-JS-heap).
    /// If one does not yet exist, start tracking it.
    HeapSnapshot::NodeID getNativeID(const void *mem);

    /// Get a list of IDs for native memory attached to the given node.
    /// List will be empty if nothing is attached yet. Then push onto the end
    /// with nextNativeID().
    /// When the NodeID that these are attached to is untracked, so are the
    /// attached native NodeIDs.
    llvh::SmallVector<HeapSnapshot::NodeID, 1> &getExtraNativeIDs(
        HeapSnapshot::NodeID node);

    /// Assign a unique ID to a literal number value that occurs in the heap.
    /// Can be used to make fake nodes that will display their numeric value.
    HeapSnapshot::NodeID getNumberID(double num);

    /// Get the object pointer for the given ID. This is the inverse of \c
    /// getObjectID.
    /// Returns none if there is no object for that ID.
    llvh::Optional<CompressedPointer> getObjectForID(HeapSnapshot::NodeID id);

    /// Tell the tracker that an object has moved locations.
    /// This must be called in a safe order, if A moves to B, and C moves to A,
    /// the first move must be recorded before the second.
    void moveObject(
        CompressedPointer oldLocation,
        CompressedPointer newLocation);

    /// Remove the object from being tracked. This should be done to keep the
    /// tracking working set small.
    void untrackObject(CompressedPointer cell);

    /// Remove the symbol from being tracked. This needs to be done to allow
    /// symbols to be re-used.
    void untrackSymbol(uint32_t symIdx);

    /// Remove the native memory from being tracked. This should be done to keep
    /// the tracking working set small. It is also required to be done when
    /// malloc'ed memory is freed, since addresses can be re-used by future
    /// allocations.
    void untrackNative(const void *mem);

    /// \return True if this is tracking any native IDs, false if there are no
    ///   native IDs being tracked.
    bool hasNativeIDs();

    /// Get the current last ID. All other existing IDs are less than or equal
    /// to this one.
    HeapSnapshot::NodeID lastID() const;

    /// Get the next unique native ID for a chunk of native memory.
    /// NOTE: public to get assigned native ids without needing to reserve in
    /// advance.
    HeapSnapshot::NodeID nextNativeID();

   private:
    /// Get the next unique object ID for a newly created object.
    HeapSnapshot::NodeID nextObjectID();
    /// Get the next unique number ID for a number.
    HeapSnapshot::NodeID nextNumberID();

    /// JS heap nodes are represented by odd-numbered IDs, while native nodes
    /// are represented with even-numbered IDs. This requirement is enforced by
    /// the Chrome snapshot viewer.
    static constexpr HeapSnapshot::NodeID kIDStep = 2;

    /// This mutex protects objectIDMap_, symbolIDMap_, and numberIDMap_.
    /// Specifically does not protect lastID_, since there's only one allocator
    /// at a time, and only new allocations affect lastID_.
    Mutex mtx_;

    /// The last ID assigned to a non-native object. Object IDs are not
    /// recycled so that snapshots don't confuse two objects with each other.
    /// NOTE: Reserved guarantees that this is an odd number.
    HeapSnapshot::NodeID lastID_{
        reserved(ReservedObjectID::FirstNonReservedID)};

    /// Map of object pointers to IDs. Only populated once the first heap
    /// snapshot is requested, or the first time the memory profiler is turned
    /// on, or if JSI tracing is in effect.
    /// This map's size is O(number of cells in the heap), which can grow quite
    /// large. Using compressed pointers keeps the size small.
    llvh::DenseMap<CompressedPointer::RawType, HeapSnapshot::NodeID>
        objectIDMap_;

    /// The inverse of \c objectIDMap_. Only used for debugging views on heap
    /// snapshots. To avoid wasting memory in the case where the debugger hasn't
    /// requested any, it is populated lazily as each entry is requested. We
    /// expect the vast majority of objects aren't inspected in the snapshot.
    llvh::DenseMap<HeapSnapshot::NodeID, CompressedPointer::RawType>
        idObjectMap_;

    /// Map of native pointers to IDs. Populated according to
    /// the same rules as the objectIDMap_.
    llvh::DenseMap<const void *, HeapSnapshot::NodeID> nativeIDMap_;

    /// Map from a JS heap object ID to additional lazily created IDs for
    /// objects. Most useful for native IDs that are attached to a heap object
    /// but don't have a stable pointer to use (such as std::vector and
    /// llvh::DenseMap).
    llvh::DenseMap<
        HeapSnapshot::NodeID,
        llvh::SmallVector<HeapSnapshot::NodeID, 1>>
        extraNativeIDs_;

    /// Map from symbol indices to unique IDs.  Populated according to
    /// the same rules as the objectIDMap_.
    llvh::DenseMap<uint32_t, HeapSnapshot::NodeID> symbolIDMap_;

    /// Map of numeric values to IDs. Used to give numbers in the heap a unique
    /// node.
    llvh::DenseMap<double, HeapSnapshot::NodeID, DoubleComparator> numberIDMap_;
  };

#ifndef NDEBUG
  /// Whether the last allocation was fixed size.  For long-lived
  /// allocations, we do not declare whether they are fixed size;
  /// Unknown is used in that case.
  enum class FixedSizeValue { Yes, No, Unknown };
#endif

  enum class HeapKind { HadesGC, MallocGC };

  GCBase(
      GCCallbacks &gcCallbacks,
      PointerBase &pointerBase,
      const GCConfig &gcConfig,
      std::shared_ptr<CrashManager> crashMgr,
      HeapKind kind);

  virtual ~GCBase() {}

  /// Create a fixed size object of type T.
  /// \return a pointer to the newly created object in the GC heap.
  template <
      typename T,
      HasFinalizer hasFinalizer = HasFinalizer::No,
      LongLived longLived = LongLived::No,
      class... Args>
  T *makeAFixed(Args &&...args);

  /// Create a variable size object of type T and size \p size.
  /// \return a pointer to the newly created object in the GC heap.
  template <
      typename T,
      HasFinalizer hasFinalizer = HasFinalizer::No,
      LongLived longLived = LongLived::No,
      class... Args>
  T *makeAVariable(uint32_t size, Args &&...args);

  template <
      typename T,
      bool fixedSize = true,
      HasFinalizer hasFinalizer = HasFinalizer::No,
      LongLived longLived = LongLived::No,
      class... Args>
  T *makeA(uint32_t size, Args &&...args);

  /// \return true if the "target space" for allocations should be randomized
  /// (for GCs where that concept makes sense).
  bool shouldRandomizeAllocSpace() const {
    return randomizeAllocSpace_;
  }

  /// Name to identify this heap in logs.
  const std::string &getName() const {
    return name_;
  }

  /// \return the base of pointers in the heap.
  /// NOTE: This normally should not be needed, Runtime provides it.
  /// However in some scenarios there is only a GC available, not a
  /// Runtime. In those cases use this function.
  PointerBase &getPointerBase() const {
    return pointerBase_;
  }

  GCCallbacks &getCallbacks() const {
    return gcCallbacks_;
  }

  /// Forwards to the GC callback \p convertSymbolToUTF8, see documentation
  /// for that function.
  std::string convertSymbolToUTF8(SymbolID id) {
    return gcCallbacks_.convertSymbolToUTF8(id);
  }

  /// Called by the Runtime to inform the GC that it is about to execute JS for
  /// the first time.
  void runtimeWillExecute();

  /// Inform the GC that TTI has been reached.  (In case, for example,
  /// behavior should change at that point.  Default behavior is to do
  /// nothing.)
  virtual void ttiReached() {}

  /// Do anything necessary to record the current number of allocated
  /// objects in numAllocatedObjects_.  Default is to do nothing.
  virtual void recordNumAllocatedObjects() {}

  /// Print any and all collected statistics to the give output stream, \p os.
  void printAllCollectedStats(llvh::raw_ostream &os);

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

  GCCallbacks &getGCCallbacks() const {
    return gcCallbacks_;
  }

  /// Cumulative stats over time so far.
  virtual size_t getPeakAllocatedBytes() const {
    return cumStats_.usedBefore.max();
  }
  virtual size_t getPeakLiveAfterGC() const {
    return cumStats_.usedAfter.max();
  }

  /// Populate \p info with information about the heap.
  virtual void getHeapInfo(HeapInfo &info);
  /// Same as \c getHeapInfo, and it adds the amount of malloc memory in use.
  virtual void getHeapInfoWithMallocSize(HeapInfo &info);

  /// Return a reference to the GCExecTrace object, which is used if
  /// we're keeping track of information about GCs, for tracing, for example.
  const GCExecTrace &getGCExecTrace() const;

  /// Populate \p info with crash manager information about the heap
  virtual void getCrashManagerHeapInfo(CrashManager::HeapInformation &info) = 0;

#ifndef NDEBUG
  /// Populate \p info with more detailed information about the heap that is
  /// too expensive to know during production builds.
  virtual void getDebugHeapInfo(DebugHeapInfo &info);

  /// \return Number of weak ref slots currently in use.
  /// Inefficient. For testing/debugging.
  size_t countUsedWeakRefs() const;

  /// Return true if \p ptr is currently pointing at valid accessable memory,
  /// allocated to an object.
  virtual bool validPointer(const void *ptr) const = 0;
#endif

#ifdef HERMESVM_GC_RUNTIME
  inline static constexpr uint32_t minAllocationSizeImpl();

  inline static constexpr uint32_t maxAllocationSizeImpl();
#endif

  inline static constexpr uint32_t minAllocationSize();

  inline static constexpr uint32_t maxAllocationSize();

  /// Dump detailed heap contents to the given output stream, \p os.
  virtual void dump(llvh::raw_ostream &os, bool verbose = false);

  /// Run the finalizers for all heap objects.
  virtual void finalizeAll() = 0;

  /// Force a garbage collection cycle. The provided cause will be used in
  /// logging.
  virtual void collect(std::string cause, bool canEffectiveOOM = false) = 0;

  /// Iterate over all objects in the heap, and call \p callback on them.
  /// \param callback A function to call on each found object.
  virtual void forAllObjs(const std::function<void(GCCell *)> &callback) = 0;

  /// \return true if the pointer lives in the young generation.
  virtual bool inYoungGen(const void *p) const {
    return false;
  }

  /// Returns whether an external allocation of the given \p size fits
  /// within the maximum heap size. (Note that this does not guarantee that the
  /// allocation will "succeed" -- the size plus the used() of the heap may
  /// still exceed the max heap size. But if it fails, the allocation can never
  /// succeed.)
  virtual bool canAllocExternalMemory(uint32_t size) {
    return true;
  }

  virtual WeakRefSlot *allocWeakSlot(CompressedPointer ptr) = 0;

#ifndef NDEBUG
  /// \name Debug APIs
  /// \{
  virtual bool calledByBackgroundThread() const {
    return false;
  }
  virtual bool calledByGC() const {
    return true;
  }
  virtual bool dbgContains(const void *ptr) const = 0;
  virtual void trackReachable(CellKind kind, unsigned sz) {}
  /// \}
#endif

  /// Do any logging of info about the heap that is useful, then dies with a
  /// fatal out-of-memory error.
  LLVM_ATTRIBUTE_NORETURN void oom(std::error_code reason);

#ifdef HERMES_MEMORY_INSTRUMENTATION
  /// Creates a snapshot of the heap and writes it to the given \p fileName.
  /// \return An error code on failure, else an empty error code.
  std::error_code createSnapshotToFile(const std::string &fileName);

  /// Creates a snapshot of the heap, which includes information about what
  /// objects exist, their sizes, and what they point to.
  virtual void createSnapshot(llvh::raw_ostream &os) = 0;
  void createSnapshot(GC &gc, llvh::raw_ostream &os);

  /// Subclasses can override and add more specific native memory usage.
  virtual void snapshotAddGCNativeNodes(HeapSnapshot &snap);

  /// Subclasses can override and add more specific edges.
  virtual void snapshotAddGCNativeEdges(HeapSnapshot &snap);

  /// Turn on the heap profiler, which will track when allocations are made and
  /// the stack trace of when they were created.
  virtual void enableHeapProfiler(
      std::function<void(
          uint64_t,
          std::chrono::microseconds,
          std::vector<GCBase::AllocationLocationTracker::HeapStatsUpdate>)>
          fragmentCallback);

  /// Turn off the heap profiler, which will stop tracking new allocations and
  /// not record any stack traces.
  /// Disabling will not forget any objects that are still alive. This way
  /// re-enabling later will still remember earlier objects.
  virtual void disableHeapProfiler();

  /// Turn the sampling memory profiler on. About once every
  /// \p samplingInterval bytes are allocated, sample the allocation by
  /// recording its stack.
  /// \param seed If non-negative, use as the seed for the random sampling
  ///   mechanism, giving deterministic output.
  virtual void enableSamplingHeapProfiler(
      size_t samplingInterval,
      int64_t seed);

  /// Turn off the sampling heap profiler, which will stop tracking new
  /// allocations and not record any stack traces. Write out the results of the
  /// trace to \p os. After this call, any remembered data about sampled objects
  /// will be gone.
  virtual void disableSamplingHeapProfiler(llvh::raw_ostream &os);
#endif // HERMES_MEMORY_INSTRUMENTATION

  /// Inform the GC about external memory retained by objects.
  virtual void creditExternalMemory(GCCell *alloc, uint32_t size) = 0;
  virtual void debitExternalMemory(GCCell *alloc, uint32_t size) = 0;

#ifdef HERMESVM_GC_RUNTIME
  /// Default implementations for read and write barriers: do nothing.
  void writeBarrier(const GCHermesValue *loc, HermesValue value);
  void writeBarrier(const GCSmallHermesValue *loc, SmallHermesValue value);
  void writeBarrier(const GCPointerBase *loc, const GCCell *value);
  void constructorWriteBarrier(const GCHermesValue *loc, HermesValue value);
  void constructorWriteBarrier(
      const GCSmallHermesValue *loc,
      SmallHermesValue value);
  void constructorWriteBarrier(const GCPointerBase *loc, const GCCell *value);
  void writeBarrierRange(const GCHermesValue *start, uint32_t numHVs);
  void writeBarrierRange(const GCSmallHermesValue *start, uint32_t numHVs);
  void constructorWriteBarrierRange(
      const GCHermesValue *start,
      uint32_t numHVs);
  void constructorWriteBarrierRange(
      const GCSmallHermesValue *start,
      uint32_t numHVs);
  void snapshotWriteBarrier(const GCHermesValue *loc);
  void snapshotWriteBarrier(const GCSmallHermesValue *loc);
  void snapshotWriteBarrier(const GCPointerBase *loc);
  void snapshotWriteBarrier(const GCSymbolID *symbol);
  void snapshotWriteBarrierRange(const GCHermesValue *start, uint32_t numHVs);
  void snapshotWriteBarrierRange(
      const GCSmallHermesValue *start,
      uint32_t numHVs);
  void weakRefReadBarrier(GCCell *value);
#endif

#ifndef NDEBUG
  virtual bool needsWriteBarrier(void *loc, GCCell *value) {
    return false;
  }
#endif

  /// @name Marking APIs
  /// @{

  /// Marks a cell by its metadata.
  /// \p cell The heap object to mark.
  /// \p acceptor The action to perform on each slot in the cell.
  template <typename Acceptor>
  inline void markCell(GCCell *cell, Acceptor &acceptor);

  /// Same as the normal \c markCell, but for cells that don't have a valid
  /// CellKind.
  template <typename Acceptor>
  inline void markCell(GCCell *cell, CellKind kind, Acceptor &acceptor);

  /// Same as the normal \c markCell, but takes a visitor instead.
  template <typename Acceptor>
  inline void
  markCell(SlotVisitor<Acceptor> &visitor, GCCell *cell, CellKind kind);

  /// Marks a cell by its metadata, but only for the slots that point between
  /// [begin, end).
  template <typename Acceptor>
  inline void markCellWithinRange(
      SlotVisitor<Acceptor> &visitor,
      GCCell *cell,
      CellKind kind,
      const char *begin,
      const char *end);

  /// Marks a cell by its metadata, and outputs the names of the slots.
  /// Meant to be used by heap snapshots.
  template <typename Acceptor>
  inline void markCellWithNames(
      SlotVisitorWithNames<Acceptor> &visitor,
      GCCell *cell);

  /// Utilities for WeakMap marking.

  /// \return a list of pointers to all the WeakRefKeys in \p weakMap.
  /// The \p gc argument is passed to methods that verify they're only
  /// called during GC.
  static std::vector<detail::WeakRefKey *> buildKeyList(
      GC &gc,
      JSWeakMap *weakMap);

  /// For all non-null keys in \p weakMap that are unreachable, clear
  /// the key (clear the pointer in the WeakRefSlot) and value (set it
  /// to undefined).
  template <typename KeyReachableFunc>
  static void clearEntriesWithUnreachableKeys(
      GC &gc,
      JSWeakMap *weakMap,
      KeyReachableFunc keyReachable);

  /// For all reachable keys in \p weakMap, mark from the
  /// corresponding value using the given \p acceptor, reaching a
  /// transitive closure.  The acceptor is required to have a property
  /// we don't normally require: it must be idempotent.  I.e., it must
  /// function properly when applied multiple times to the same
  /// pointer slot during a collection.  "Mark-in-place" acceptors
  /// will generally have this property.  Uses \p objIsMarked to
  /// determine whether an object is marked, and, for entries whose
  /// keys are marked, invokes \p markFromVal on the corresponding value.
  /// These have the following specs:
  ///
  ///  * objIsMarked: (GCCell*) ==> bool
  ///    Returns whether a GCCell is marked.
  ///
  ///  * markFromVal: GCCell *cell, GCHermesValue &cellRef) ==> bool
  ///    If the argument is unmarked, mark it, schedule for scanning.
  ///    Returns whether the object was newly marked.
  ///
  /// If the \p unreachableKeys map has an entry for \p weakMap, assumes the
  /// list of WeakRefKeys contains all possibly-unreachable keys; any
  /// other keys are assumed to have already been found reachable.
  /// Ensures that \p unreachableKeys has an accurate value for \p
  /// weakMap before return.  \return whether any previously-unmarked
  /// values were marked.
  template <
      typename Acceptor,
      typename ObjIsMarkedFunc,
      typename MarkFromValFunc>
  static bool markFromReachableWeakMapKeys(
      GC &gc,
      JSWeakMap *weakMap,
      Acceptor &acceptor,
      llvh::DenseMap<JSWeakMap *, std::vector<detail::WeakRefKey *>>
          *unreachableKeys,
      ObjIsMarkedFunc objIsMarked,
      MarkFromValFunc markFromVal);

  /// \return A reference to the mutex that controls accessing any WeakRef.
  ///   This mutex must be held if a WeakRef is created or modified.
  WeakRefMutex &weakRefMutex() {
    return weakRefMutex_;
  }

  /// Assumes that all known reachable WeakMaps have been collected in
  /// \p reachableWeakMaps.  For all these WeakMaps, find all
  /// reachable keys and mark from the corresponding value using the given \p
  /// acceptor, reaching a transitive closure.
  /// Do this until no newly reachable objects are found in a
  /// traversal of the WeakMaps.  We assume that WeakMaps found newly
  /// reachable are added to \p reachableWeakMaps, and do not assume
  /// we've reached transitive closure until all maps are scanned.
  /// Uses \p objIsMarked to determine whether an object is marked,
  /// and, for entries whose keys are marked, invokes \p
  /// checkValIsMarked on the corresponding value.  Used \p
  /// drainMarkStack to ensure that the transitive closure of what's
  /// currently on the mark stack is marked.  Requires \p acceptor
  /// to be idempotent: it must be legal to apply the acceptor
  /// multiple times to the same slot.  The function arguments have
  /// the following specs:
  ///
  ///  * objIsMarked: (GCCell *) ==> bool
  ///    Returns whether a GCCell is marked.
  ///
  ///  * markFromVal: (GCCell *cell, HermesValue &cellRef) ==> bool
  ///    Requires that \p cell is non-null, and the value of \p
  ///    cellRef.  If the argument is unmarked, ensure that its
  ///    transitive closure is marked.  Returns whether the object was
  ///    newly marked.
  ///
  ///  * drainMarkStack: (Acceptor &acceptor) ==> void
  ///    Ensures that the mark stack used by the collector is empty;
  ///    the transitive closure of the original contents is marked.
  ///
  ///  * checkMarkStackOverflow: () ==> bool
  ///    Returns whether mark stack overflow has occurred.
  ///
  /// Some collectors compute the allocated bytes during GC.  If this
  /// is done in \p drainMarkStack, that will cover all objects except
  /// WeakWaps, which are never pushed on the mark stack.  Thus:
  /// \return the total size of reachable WeakMaps.
  template <
      typename Acceptor,
      typename ObjIsMarkedFunc,
      typename MarkFromValFunc,
      typename DrainMarkStackFunc,
      typename CheckMarkStackOverflowFunc>
  static gcheapsize_t completeWeakMapMarking(
      GC &gc,
      Acceptor &acceptor,
      std::vector<JSWeakMap *> &reachableWeakMaps,
      ObjIsMarkedFunc objIsMarked,
      MarkFromValFunc markFromVal,
      DrainMarkStackFunc drainMarkStack,
      CheckMarkStackOverflowFunc checkMarkStackOverflow);

  /// @}

  /// If false, all reachable objects in the heap will have a dereference-able
  /// VTable pointer which describes its type and size. If true, both reachable
  /// objects and unreachable objects may not have dereference-able VTable
  /// pointers, and any reads from JS heap memory may give strange results.
  bool inGC() const {
    return inGC_;
  }

  HeapKind getKind() const {
    return heapKind_;
  }

  /// \return A string representation of the kind of GC.
  virtual std::string getKindAsStr() const = 0;

  bool isTrackingIDs() {
#ifdef HERMES_MEMORY_INSTRUMENTATION
    return getIDTracker().isTrackingIDs() ||
        getAllocationLocationTracker().isEnabled() ||
        getSamplingAllocationTracker().isEnabled();
#else
    return getIDTracker().isTrackingIDs();
#endif
  }

  IDTracker &getIDTracker() {
    return idTracker_;
  }

#ifdef HERMES_MEMORY_INSTRUMENTATION
  AllocationLocationTracker &getAllocationLocationTracker() {
    return allocationLocationTracker_;
  }

  SamplingAllocationLocationTracker &getSamplingAllocationTracker() {
    return samplingAllocationTracker_;
  }
#endif

  /// \name Snapshot ID methods
  /// \{
  // This set of methods are all mirrors of IDTracker, except with pointer
  // compression done automatically.
  HeapSnapshot::NodeID getObjectID(const GCCell *cell);
  HeapSnapshot::NodeID getObjectIDMustExist(const GCCell *cell);
  HeapSnapshot::NodeID getObjectID(CompressedPointer cell);
  HeapSnapshot::NodeID getObjectID(SymbolID sym);
  HeapSnapshot::NodeID getNativeID(const void *mem);
  /// \return The ID for the given value. If the value cannot be represented
  ///   with an ID, returns None.
  llvh::Optional<HeapSnapshot::NodeID> getSnapshotID(HermesValue val);

  /// \return The object pointer for the given \p id. This is quite slow.
  ///   If there is no such object this returns nullptr.
  void *getObjectForID(HeapSnapshot::NodeID id);

  /// \return True if the given cell has an ID associated with it.
  bool hasObjectID(const GCCell *cell);
  /// Records that a new allocation has occurred.
  void newAlloc(const GCCell *ptr, uint32_t sz);
  /// Moves an object to a new address and a new size for all trackers.
  void moveObject(
      const GCCell *oldPtr,
      uint32_t oldSize,
      const GCCell *newPtr,
      uint32_t newSize);
  /// Untracks a freed object from all trackers.
  void untrackObject(const GCCell *cell, uint32_t sz);
  /// \}

#ifndef NDEBUG
  /// \return The next debug allocation ID for embedding directly into a GCCell.
  /// NOTE: This is not the same ID as is used for stable lifetime tracking, use
  /// \p getObjectID for that.
  uint64_t nextObjectID();
#endif

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
  void checkTripwire(size_t dataSize);

  // Visibility here is public for unit_tests and protected otherwise

 protected:
  /// An RAII-style object used to denote regions when a GC cycle is considered
  /// active.
  class GCCycle final {
   public:
    explicit GCCycle(GCBase &gc, std::string extraInfo = "");
    ~GCCycle();

    const std::string &extraInfo() {
      return extraInfo_;
    }

   private:
    GCBase &gc_;
    std::string extraInfo_;
    bool previousInGC_;
  };

  /// Returns the number of bytes allocated allocated since the last GC.
  /// TODO: Implement this for heaps other than GenGC
  /// (at which point this can become an abstract function).
  virtual gcheapsize_t bytesAllocatedSinceLastGC() const {
    return 0;
  }

  /// Convenience method to invoke the mark roots function provided at
  /// initialization, using the context provided then (on this heap).
  /// The \p markLongLived argument indicates whether root data structures
  /// containing only pointers to objects allocated via allocLongLived
  /// are required to be marked.  In this collector, such objects will
  /// be allocated in the old gen, and references to them need not be
  /// marked during young-gen collection.
  void markRoots(RootAndSlotAcceptorWithNames &acceptor, bool markLongLived) {
    gcCallbacks_.markRoots(acceptor, markLongLived);
  }

  /// Convenience method to invoke the mark weak roots function provided at
  /// initialization, using the context provided then (on this heap).
  void markWeakRoots(WeakRootAcceptor &acceptor, bool markLongLived) {
    gcCallbacks_.markWeakRoots(acceptor, markLongLived);
    acceptor.beginRootSection(RootAcceptor::Section::WeakRefSlots);
    for (auto &slot : weakSlots_) {
      slot.markWeakRoots(acceptor);
    }
    acceptor.endRootSection();
  }

  /// Print the cumulative statistics.
  virtual void printStats(JSONEmitter &json);

  /// Record statistics from a single GC, which are specified in the given
  /// \p event, in the overall cumulative stats struct.
  void recordGCStats(const GCAnalyticsEvent &event, bool onMutator);

  /// Record statistics from a single GC, which are specified in the given
  /// \p event, in the given cumulative stats struct.
  void recordGCStats(
      const GCAnalyticsEvent &event,
      CumulativeHeapStats *stats,
      bool onMutator);

  /// Print detailed stats of the breakdown of the roots and heap in terms of
  /// the number of pointers, symbols, HermesValues, etc.
  void sizeDiagnosticCensus(size_t allocatedBytes);

  /// Do any additional GC-specific logging that is useful before dying with
  /// out-of-memory. Takes a char buffer to avoid dynamic allocations.
  virtual void oomDetail(
      llvh::MutableArrayRef<char> detailBuffer,
      std::error_code reason);

  /// If a cell has any weak references to mark, and the acceptor supports
  /// marking them, mark those weak references.
  template <typename Acceptor>
  void markWeakRefsIfNecessary(GCCell *cell, CellKind kind, Acceptor &acceptor);

  /// Overload of \p markWeakRefsIfNecessary for acceptors that support marking
  /// weak references.
  /// Don't call this directly, use the three-argument variant instead.
  template <typename Acceptor>
  void markWeakRefsIfNecessary(
      GCCell *cell,
      CellKind kind,
      Acceptor &acceptor,
      std::true_type) {
    // In C++17, we could implement this via "constexpr if" rather than
    // overloads with std::true_type.
    // Once C++17 is available, switch to using that.
    if (auto *cb = VTable::getVTable(kind)->getMarkWeakCallback()) {
      std::lock_guard<Mutex> wrLk{weakRefMutex()};
      cb(cell, acceptor);
    }
  }

  /// Overload of \p markWeakRefsIfNecessary for acceptors that do not support
  /// marking weak references.
  /// Don't call this directly, use the three-argument variant instead.
  template <typename Acceptor>
  static void markWeakRefsIfNecessary(
      GCCell *,
      CellKind kind,
      Acceptor &,
      std::false_type) {}

  template <typename T, class... Args>
  static T *constructCell(void *ptr, uint32_t size, Args &&...args) {
    auto *cell = new (ptr) T(std::forward<Args>(args)...);
    constexpr auto kind = T::getCellKind();
    cell->setKindAndSize({kind, size});
    return cell;
  }

  /// Number of finalized objects in the last collection.
  unsigned numFinalizedObjects_{0};

  /// The total number of bytes allocated in the execution.
  uint64_t totalAllocatedBytes_{0};

  /// A trace of GC execution.
  GCExecTrace execTrace_;

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

  /// Associate a semi-unique (until it overflows) id with every allocation
  /// for easier identification when debugging.
  uint64_t debugAllocationCounter_{0};
#endif

  /// User-supplied callbacks invoked by the GC to query information or perform
  /// tasks.
  GCCallbacks &gcCallbacks_;

  /// Base of all pointers in compressed pointers implementation.
  PointerBase &pointerBase_;

  /// A place to log crash data if a crash is about to occur.
  std::shared_ptr<CrashManager> crashMgr_;

  HeapKind heapKind_;

  /// Callback called once for each GC event that wants to be logged. Can be
  /// null if no analytics are requested.
  std::function<void(const GCAnalyticsEvent &)> analyticsCallback_;

  /// Capture all analytics events to print stats at the end.
  std::vector<GCAnalyticsEvent> analyticsEvents_;

  /// Whether to output GC statistics at the end of execution.
  bool recordGcStats_{false};

  /// Whether or not a GC cycle is currently occurring.
  bool inGC_{false};

  /// The block of fields below records values of various metrics at
  /// the start of execution, so that we can get the values at the end
  /// and subtract.  The "runtimeWillExecute" method is called at
  /// first bytecode execution, but also when executing the bodies of
  /// other bundles.  We want to record these at the first such call.
  /// This field tells whether these values have been recorded yet.
  bool execStartTimeRecorded_{false};

  /// Time at which execution of the Hermes VM began.
  std::chrono::time_point<std::chrono::steady_clock> execStartTime_;
  std::chrono::microseconds execStartCPUTime_;
  /// Number of context switches before execution of the Hermes VM began.
  long startNumVoluntaryContextSwitches_{0};
  long startNumInvoluntaryContextSwitches_{0};

  // The cumulative GC stats.
  CumulativeHeapStats cumStats_;

  /// Name to identify this heap in logs.
  std::string name_;

  /// weakSlots_ is a list of all the weak pointers in the system. They are
  /// invalidated if they point to an object that is dead, and do not count
  /// towards whether an object is live or dead.
  /// Protected by weakRefMutex_.
  std::deque<WeakRefSlot> weakSlots_;

  /// Any thread that modifies a WeakRefSlot or a data structure containing
  /// WeakRefs that the GC will mark must hold this mutex. The GC will hold this
  /// mutex while scanning any weak references.
  WeakRefMutex weakRefMutex_;

  /// Tracks what objects need a stable identity for features such as heap
  /// snapshots and the memory profiler.
  IDTracker idTracker_;

#ifdef HERMES_MEMORY_INSTRUMENTATION
  /// Attaches stack-traces to objects when enabled.
  AllocationLocationTracker allocationLocationTracker_;

  /// Attaches stack-traces to objects when enabled.
  SamplingAllocationLocationTracker samplingAllocationTracker_;
#endif

#ifndef NDEBUG
  /// The number of reasons why no allocation is allowed in this heap right
  /// now.
  uint32_t noAllocLevel_{0};

  friend class NoAllocScope;
#endif

#ifdef HERMESVM_SANITIZE_HANDLES
  /// \return true if we should run handle sanitization and the coin flip with
  /// probability sanitizeRate_ has passed.
  bool shouldSanitizeHandles();

  /// Whether to keep moving the heap around to detect unsanitary GC handles.
  double sanitizeRate_{1.0};

  /// PRNG for sanitizing at a less than 1.0 rate.
  std::minstd_rand randomEngine_;
#else
  /// Sanitize handles is completely disabled (and ignored at runtime) without
  /// a special build mode.
  static constexpr double sanitizeRate_{0.0};

  static constexpr bool shouldSanitizeHandles() {
    return false;
  }
#endif

 private:
#ifdef HERMESVM_GC_RUNTIME
  /// Use the kind tag of the GC to statically call a function with one of the
  /// available runtime GCs.
  template <typename Func>
  auto runtimeGCDispatch(Func f) {
    switch (getKind()) {
#define GC_KIND(kind)          \
  case GCBase::HeapKind::kind: \
    return f(llvh::cast<kind>(this));
      RUNTIME_GC_KINDS
#undef GC_KIND
      default:
        llvm_unreachable("No other valid GC for RuntimeGC");
    }
  }
#endif

  template <typename T, XorPtrKeyID K>
  friend class XorPtr;

  /// Randomly generated key used to obfuscate pointers in XorPtr.
  uintptr_t pointerEncryptionKey_[XorPtrKeyID::_NumKeys];

  /// Callback called if it's not null when the Live Data Tripwire is
  /// triggered.
  std::function<void(GCTripwireContext &)> tripwireCallback_;

  /// Maximum size limit before the heap size tripwire will trigger.
  gcheapsize_t tripwireLimit_;

  /// True if the tripwire has already been called on this heap.
  bool tripwireCalled_{false};

/// Whether to randomize the "target space" for allocations, for GC's in which
/// this concept makes sense. Only available in debug builds.
#ifndef NDEBUG
  bool randomizeAllocSpace_{false};
#else
  static const bool randomizeAllocSpace_{false};
#endif
};

#ifdef HERMESVM_EXCEPTION_ON_OOM
/// A std::runtime_error class for out-of-memory.
class JSOutOfMemoryError : public std::runtime_error {
 public:
  JSOutOfMemoryError(const std::string &what_arg)
      : std::runtime_error(what_arg) {}
  JSOutOfMemoryError(const char *what_arg) : std::runtime_error(what_arg) {}
};
#endif

// Utilities for formatting time durations and memory sizes.

/// An object that, written to an ostream, formats the given # of
/// secs in appropriate units (down to microseconds).
struct DurationFormatObj {
  double secs;
};
llvh::raw_ostream &operator<<(
    llvh::raw_ostream &os,
    const DurationFormatObj &dfo);
inline DurationFormatObj formatSecs(double secs) {
  return {secs};
}

/// An object that, written to an ostream, formats the given # of
/// bytes in appropriate units (bytes to GiB).
struct SizeFormatObj {
  uint64_t bytes;
};
llvh::raw_ostream &operator<<(llvh::raw_ostream &os, const SizeFormatObj &sfo);
inline SizeFormatObj formatSize(uint64_t size) {
  return {size};
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCBASE_H
