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
   public:
    /// If true, then this cell is in the "young generation". Note that MallocGC
    /// is not actually a generational collector, this just tracks some objects
    /// that could plausibly be in the young gen to exercise codepaths that
    /// depend on inYoungGen being true.
    bool inYoungGen;

   private:
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
      CanBeLarge canBeLarge = CanBeLarge::No,
      MayFail mayFail = MayFail::No,
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

  static constexpr uint32_t maxNormalAllocationSizeImpl() {
    // MallocGC imposes no limit on individual allocations.
    return std::numeric_limits<uint32_t>::max();
  }

  /// Run the finalizers for all heap objects.
  void finalizeAll() override;

  bool inYoungGen(const GCCell *p) const override {
    return CellHeader::from(p)->inYoungGen;
  }

#ifndef NDEBUG
  /// See comment in GCBase.
  bool calledByGC() const override {
    return inGC();
  }

  /// \return true iff the pointer \p p is controlled by this GC.
  bool validPointer(const void *p) const override;
  bool dbgContains(const void *p) const override;

  bool needsWriteBarrier(const GCHermesValueBase *loc, HermesValue value)
      const override;
  bool needsWriteBarrier(
      const GCSmallHermesValueBase *loc,
      SmallHermesValue value) const override;
  bool needsWriteBarrier(const GCPointerBase *loc, GCCell *value)
      const override;
  bool needsWriteBarrierInCtor(const GCHermesValueBase *loc, HermesValue value)
      const override;
  bool needsWriteBarrierInCtor(
      const GCSmallHermesValueBase *loc,
      SmallHermesValue value) const override;
#endif

#ifdef HERMES_MEMORY_INSTRUMENTATION
  /// Same as in superclass GCBase.
  virtual void createSnapshot(llvh::raw_ostream &os, bool captureNumericValue)
      override;
#endif

  virtual void creditExternalMemory(GCCell *alloc, uint32_t size) override;
  virtual void debitExternalMemory(GCCell *alloc, uint32_t size) override;

  void writeBarrier(const GCHermesValue *, HermesValue) {}
  void writeBarrier(const GCSmallHermesValue *, SmallHermesValue) {}
  void writeBarrierForLargeObj(
      const GCCell *,
      const GCHermesValueInLargeObj *,
      HermesValue) {}
  void writeBarrierForLargeObj(
      const GCCell *,
      const GCSmallHermesValueInLargeObj *,
      SmallHermesValue) {}
  void writeBarrier(const GCPointerBase *, const GCCell *) {}
  void writeBarrierForLargeObj(
      const GCCell *,
      const GCPointerBase *,
      const GCCell *) {}
  void constructorWriteBarrier(const GCHermesValue *, HermesValue) {}
  void constructorWriteBarrier(const GCSmallHermesValue *, SmallHermesValue) {}
  void constructorWriteBarrierForLargeObj(
      const GCCell *,
      const GCSmallHermesValueInLargeObj *,
      SmallHermesValue) {}
  void constructorWriteBarrierForLargeObj(
      const GCCell *,
      const GCHermesValueInLargeObj *,
      HermesValue) {}
  void constructorWriteBarrier(const GCPointerBase *, const GCCell *) {}
  void constructorWriteBarrierForLargeObj(
      const GCCell *,
      const GCPointerBase *,
      const GCCell *) {}
  void writeBarrierRange(const GCHermesValue *, uint32_t) {}
  void writeBarrierRange(const GCSmallHermesValue *, uint32_t) {}
  void constructorWriteBarrierRange(
      const GCCell *,
      const GCHermesValueBase *,
      uint32_t) {}
  void constructorWriteBarrierRange(
      const GCCell *,
      const GCSmallHermesValueBase *,
      uint32_t) {}
  void snapshotWriteBarrier(const GCHermesValueBase *) {}
  void snapshotWriteBarrier(const GCSmallHermesValueBase *) {}
  void snapshotWriteBarrier(const GCPointerBase *) {}
  void snapshotWriteBarrier(const GCSymbolID *) {}
  void snapshotWriteBarrierRange(const GCHermesValueBase *, uint32_t) {}
  void snapshotWriteBarrierRange(const GCSmallHermesValueBase *, uint32_t) {}
  void weakRefReadBarrier(HermesValue) {}
  void weakRefReadBarrier(GCCell *) {}
  void weakRefReadBarrier(SymbolID) {}

  void getHeapInfo(HeapInfo &info) override;
  void getHeapInfoWithMallocSize(HeapInfo &info) override;
  void getCrashManagerHeapInfo(CrashManager::HeapInformation &info) override;
  std::string getKindAsStr() const override;

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

 private:
#ifdef HERMES_SLOW_DEBUG
  void checkWellFormed();
  void clearUnmarkedPropertyMaps();
#endif

  /// Allocate an object in the GC controlled heap with the size to allocate
  /// given by \p size.
  GCCell *alloc(uint32_t size);

  /// Initialize a cell with the required basic data for any cell.
  inline void initCell(GCCell *cell, uint32_t size);

  /// See \c GCBase::printStats.
  void printStats(JSONEmitter &json) override;

  /// Reset the statistics used for reporting GC information.
  void resetStats();

  struct MarkingAcceptor;
  class SkipWeakRefsMarkingAcceptor;
  struct FullMSCUpdateWeakRootsAcceptor;

  /// Continually pops elements from the mark stack of \p acceptor and
  /// scans their pointer fields.  If such a field points to an
  /// unmarked object, mark it and push it on the mark stack.
  void drainMarkStack(MarkingAcceptor &acceptor);

  /// Iterate the list of `weakMapEntrySlots_`, for each non-free slot, if
  /// both the key and the owner are marked, mark the mapped value with
  /// \p acceptor. This may further cause other values to be marked, so we need
  /// to keep iterating until no update. After the iteration, set each
  /// unreachable mapped value to Empty.
  void markWeakMapEntrySlots(MarkingAcceptor &acceptor);

  /// Update all of the weak references and invalidate the ones that point to
  /// dead objects.
  void updateWeakReferences();
};

/// @name Inline implementations
/// @{

inline bool MallocGC::canAllocExternalMemory(uint32_t size) {
  return size <= maxSize_;
}

template <
    typename T,
    bool fixedSize,
    HasFinalizer hasFinalizer,
    LongLived longLived,
    CanBeLarge canBeLarge,
    MayFail mayFail,
    class... Args>
inline T *MallocGC::makeA(uint32_t size, Args &&...args) {
  assert(
      isSizeHeapAligned(size) &&
      "Call to makeA must use a size aligned to HeapAlign");
  // Since there is no old generation in this collector, always forward to the
  // normal allocation.
  GCCell *mem = alloc(size);
  CellHeader::from(mem)->inYoungGen = longLived == LongLived::No;
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
