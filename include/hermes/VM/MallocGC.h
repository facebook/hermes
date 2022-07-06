/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_MALLOCGC_H
#define HERMES_VM_MALLOCGC_H

#include "hermes/Public/GCConfig.h"
#include "hermes/VM/CompressedPointer.h"
#include "hermes/VM/GCBase.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/VMExperiments.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/DenseSet.h"
#include "llvh/Support/raw_ostream.h"

#include <deque>
#include <limits>
#include <vector>

namespace hermes {
namespace vm {

class MallocGC final : public GCBase {
  class CellHeader {
    /// If true, then this cell is live. If this is false at the end of a
    /// collection, then this cell can be freed. Defaults to false when not in
    /// the middle of a collection.
    bool marked_ = false;

#ifdef HERMESVM_SANITIZE_HANDLES
    /// If non-null, contains a pointer to the new location where this cell will
    /// be after the collection is over.
    /// Only used in handle sanitization when pointers are moved.
    CellHeader *forwardingPtr_ = nullptr;
#endif

    /// Storage of the GCCell data is past this point. Needs to be aligned to
    /// the alignment of the heap.
    static_assert(
        alignof(GCCell) <= HeapAlign,
        "GCCell's alignment exceeds the alignment requirement of the heap");
    alignas(HeapAlign) uint8_t data_[0];

   public:
    bool isMarked() const {
      return marked_;
    }

    /// Mark a cell as live. Set its new location to \p newLocation.
    void mark() {
      assert(!isMarked() && "Marking an already marked pointer");
      marked_ = true;
    }

    void markWithForwardingPointer(CellHeader *newLocation) {
      mark();
#ifdef HERMESVM_SANITIZE_HANDLES
      forwardingPtr_ = newLocation;
#else
      llvm_unreachable(
          "Trying to add a forwarding pointer outside of handle-san mode");
#endif
    }

    void unmark() {
      assert(isMarked() && "Unmarking an already unmarked pointer");
      marked_ = false;
#ifdef HERMESVM_SANITIZE_HANDLES
      forwardingPtr_ = nullptr;
#endif
    }

    GCCell *data() {
      return reinterpret_cast<GCCell *>(data_);
    }

    const GCCell *data() const {
      return reinterpret_cast<const GCCell *>(data_);
    }

    CellHeader *getForwardingPointer() {
      assert(isMarked() && "Getting forwarding pointer from unmarked pointer");
#ifdef HERMESVM_SANITIZE_HANDLES
      assert(forwardingPtr_ && "Accessing a null forwarding pointer");
      return forwardingPtr_;
#else
      llvm_unreachable(
          "Can't get a forwarding pointer outside of handle-san mode");
#endif
    }

    static CellHeader *from(GCCell *cell) {
      return reinterpret_cast<CellHeader *>(
          reinterpret_cast<char *>(cell) - offsetof(CellHeader, data_));
    }

    static const CellHeader *from(const GCCell *cell) {
      return reinterpret_cast<const CellHeader *>(
          reinterpret_cast<const char *>(cell) - offsetof(CellHeader, data_));
    }
  };

  /// pointers_ stores all of the objects managed by the GC. If a pointer is not
  /// in this set, then it is not a GC pointer, and thus invalid to be collected
  /// or marked.
  llvh::DenseSet<CellHeader *> pointers_;
  /// newPointers_ is the set of live objects at the end of a collection.
  /// Pointers are moved from pointers_ to this as they are discovered to be
  /// alive.
  /// This should be empty between collections.
  llvh::DenseSet<CellHeader *> newPointers_;
  /// maxSize_ is the absolute highest amount of memory that MallocGC is allowed
  /// to allocate.
  const gcheapsize_t maxSize_;
  /// sizeLimit_ is the point at which a collection should be run.
  gcheapsize_t sizeLimit_;
  /// allocatedBytes_ is the current amount of memory stored in the heap.
  gcheapsize_t allocatedBytes_{0};
  /// externalBytes_ is the external memory retained by cells on the heap.
  uint64_t externalBytes_{0};

 public:
  /// See comment in GCBase.
  class Size final {
   public:
    explicit Size(const GCConfig &gcConfig)
        : Size(gcConfig.getMinHeapSize(), gcConfig.getMaxHeapSize()) {}
    Size(gcheapsize_t min, gcheapsize_t max) : min_(min), max_(max) {}

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

  MallocGC(
      GCCallbacks &gcCallbacks,
      PointerBase &pointerBase,
      const GCConfig &gcConfig,
      std::shared_ptr<CrashManager> crashMgr,
      std::shared_ptr<StorageProvider> provider,
      experiments::VMExperimentFlags vmExperimentFlags);

  ~MallocGC() override;

  /// Checks if a requested \p size can fit in the heap. If it can't, a
  /// collection occurs. If it still can't after the collection, OOM is
  /// declared.
  void collectBeforeAlloc(std::string cause, uint32_t size);

  /// Allocate a new cell of the specified size \p size by calling alloc.
  /// Instantiate an object of type T with constructor arguments \p args in the
  /// newly allocated cell.
  /// \return a pointer to the newly created object in the GC heap.
  template <
      typename T,
      bool fixedSize = true,
      HasFinalizer hasFinalizer = HasFinalizer::No,
      LongLived longLived = LongLived::Yes,
      class... Args>
  inline T *makeA(uint32_t size, Args &&...args);

  /// Returns whether an external allocation of the given \p size fits
  /// within the maximum heap size.  (Note that this does not guarantee that the
  /// allocation will "succeed" -- the size plus the used() of the heap may
  /// still exceed the max heap size.  But if it fails, the allocation can never
  /// succeed.)
  bool canAllocExternalMemory(uint32_t size) override;

  /// Collect all of the dead objects and symbols in the heap. Also invalidate
  /// weak pointers that point to dead objects.
  void collect(std::string cause, bool canEffectiveOOM = false) override;

  static constexpr uint32_t minAllocationSizeImpl() {
    // MallocGC imposes no limit on individual allocations.
    return 0;
  }

  static constexpr uint32_t maxAllocationSizeImpl() {
    // MallocGC imposes no limit on individual allocations.
    return std::numeric_limits<uint32_t>::max();
  }

  /// Run the finalizers for all heap objects.
  void finalizeAll() override;

  /// \return true iff this is collecting the entire heap, or false if it is
  /// only a portion of the heap.
  /// \pre Assumes inGC() is true, or else this has no meaning.
  bool inFullCollection() const {
    return true;
  }

#ifndef NDEBUG
  /// See comment in GCBase.
  bool calledByGC() const override {
    return inGC();
  }

  /// \return true iff the pointer \p p is controlled by this GC.
  bool validPointer(const void *p) const override;
  bool dbgContains(const void *p) const override;
#endif

#ifdef HERMES_MEMORY_INSTRUMENTATION
  /// Same as in superclass GCBase.
  virtual void createSnapshot(llvh::raw_ostream &os) override;
#endif

  virtual void creditExternalMemory(GCCell *alloc, uint32_t size) override;
  virtual void debitExternalMemory(GCCell *alloc, uint32_t size) override;

  void writeBarrier(const GCHermesValue *, HermesValue) {}
  void writeBarrier(const GCSmallHermesValue *, SmallHermesValue) {}
  void writeBarrier(const GCPointerBase *, const GCCell *) {}
  void constructorWriteBarrier(const GCHermesValue *, HermesValue) {}
  void constructorWriteBarrier(const GCSmallHermesValue *, SmallHermesValue) {}
  void constructorWriteBarrier(const GCPointerBase *, const GCCell *) {}
  void writeBarrierRange(const GCHermesValue *, uint32_t) {}
  void writeBarrierRange(const GCSmallHermesValue *, uint32_t) {}
  void constructorWriteBarrierRange(const GCHermesValue *, uint32_t) {}
  void constructorWriteBarrierRange(const GCSmallHermesValue *, uint32_t) {}
  void snapshotWriteBarrier(const GCHermesValue *) {}
  void snapshotWriteBarrier(const GCSmallHermesValue *) {}
  void snapshotWriteBarrier(const GCPointerBase *) {}
  void snapshotWriteBarrier(const GCSymbolID *) {}
  void snapshotWriteBarrierRange(const GCHermesValue *, uint32_t) {}
  void snapshotWriteBarrierRange(const GCSmallHermesValue *, uint32_t) {}
  void weakRefReadBarrier(GCCell *) {}

  void getHeapInfo(HeapInfo &info) override;
  void getHeapInfoWithMallocSize(HeapInfo &info) override;
  void getCrashManagerHeapInfo(CrashManager::HeapInformation &info) override;
  std::string getKindAsStr() const override;

  /// @name Weak references
  /// @{

  /// Allocate a weak pointer slot for the value given.
  /// \pre \p init should not be empty or a native value.
  WeakRefSlot *allocWeakSlot(CompressedPointer ptr) override;

  /// The largest the size of this heap could ever grow to.
  size_t maxSize() const {
    return maxSize_;
  }

  /// Iterate over all objects in the heap, and call \p callback on them.
  /// \param callback A function to call on each found object.
  void forAllObjs(const std::function<void(GCCell *)> &callback) override;

  static bool classof(const GCBase *gc) {
    return gc->getKind() == HeapKind::MallocGC;
  }

  /// @}

 private:
#ifdef HERMES_SLOW_DEBUG
  void checkWellFormed();
  void clearUnmarkedPropertyMaps();
#endif

  /// Allocate an object in the GC controlled heap with the size to allocate
  /// given by \p size.
  template <
      bool fixedSizeIgnored = true,
      HasFinalizer hasFinalizer = HasFinalizer::No>
  inline void *alloc(uint32_t size);

  /// Initialize a cell with the required basic data for any cell.
  inline void initCell(GCCell *cell, uint32_t size);

  /// Free a weak pointer slot, which invalidates it.
  void freeWeakSlot(WeakRefSlot *slot);

  /// See \c GCBase::printStats.
  void printStats(JSONEmitter &json) override;

  /// Reset the statistics used for reporting GC information.
  void resetStats();

  /// Sets all weak references to unmarked in preparation for a collection.
  void resetWeakReferences();

  struct MarkingAcceptor;
  class SkipWeakRefsMarkingAcceptor;
  struct FullMSCUpdateWeakRootsAcceptor;

  /// Continually pops elements from the mark stack of \p acceptor and
  /// scans their pointer fields.  If such a field points to an
  /// unmarked object, mark it and push it on the mark stack.
  void drainMarkStack(MarkingAcceptor &acceptor);

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
  void completeWeakMapMarking(MarkingAcceptor &acceptor);

  /// Update all of the weak references and invalidate the ones that point to
  /// dead objects.
  void updateWeakReferences();
};

/// @name Inline implementations
/// @{

template <bool fixedSizeIgnored, HasFinalizer hasFinalizer>
inline void *MallocGC::alloc(uint32_t size) {
  assert(noAllocLevel_ == 0 && "no alloc allowed right now");
  assert(
      isSizeHeapAligned(size) &&
      "Call to alloc must use a size aligned to HeapAlign");
  if (shouldSanitizeHandles()) {
    collectBeforeAlloc(kHandleSanCauseForAnalytics, size);
  }
  // Use subtraction to prevent overflow.
  if (LLVM_UNLIKELY(size > sizeLimit_ - allocatedBytes_)) {
    collectBeforeAlloc(kNaturalCauseForAnalytics, size);
  }
  // Add space for the header.
  auto *header = new (checkedMalloc(size + sizeof(CellHeader))) CellHeader();
  GCCell *mem = header->data();
  initCell(mem, size);
  // Add to the set of pointers owned by the GC.
  pointers_.insert(header);
  allocatedBytes_ += size;
  totalAllocatedBytes_ += size;
#ifndef NDEBUG
  ++numAllocatedObjects_;
#endif
  return mem;
}

inline bool MallocGC::canAllocExternalMemory(uint32_t size) {
  return size <= maxSize_;
}

template <
    typename T,
    bool fixedSize,
    HasFinalizer hasFinalizer,
    LongLived longLived,
    class... Args>
inline T *MallocGC::makeA(uint32_t size, Args &&...args) {
  assert(
      isSizeHeapAligned(size) &&
      "Call to makeA must use a size aligned to HeapAlign");
  // Since there is no old generation in this collector, always forward to the
  // normal allocation.
  void *mem = alloc<fixedSize, hasFinalizer>(size);
  return constructCell<T>(mem, size, std::forward<Args>(args)...);
}

inline void MallocGC::initCell(GCCell *cell, uint32_t size) {
#ifndef NDEBUG
  // For debugging, fill with a dead value.
  std::fill_n(reinterpret_cast<char *>(cell), size, kInvalidHeapValue);
#endif
}

/// @}

} // namespace vm
} // namespace hermes

#endif
