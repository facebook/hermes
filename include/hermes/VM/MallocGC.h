/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_MALLOCGC_H
#define HERMES_VM_MALLOCGC_H

#include "hermes/Public/GCConfig.h"
#include "hermes/VM/GCBase.h"
#include "hermes/VM/GCCell.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/Support/raw_ostream.h"

#include <deque>
#include <limits>
#include <unordered_set>
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
  llvm::DenseSet<CellHeader *> pointers_;
  /// newPointers_ is the set of live objects at the end of a collection.
  /// Pointers are moved from pointers_ to this as they are discovered to be
  /// alive.
  /// This should be empty between collections.
  llvm::DenseSet<CellHeader *> newPointers_;
  /// weakPointers_ is a list of all the weak pointers in the system. They are
  /// invalidated if they point to an object that is dead, and do not count
  /// towards whether an object is live or dead.
  std::deque<WeakRefSlot> weakPointers_;
  /// maxSize_ is the absolute highest amount of memory that MallocGC is allowed
  /// to allocate.
  const gcheapsize_t maxSize_;
  /// sizeLimit_ is the point at which a collection should be run.
  gcheapsize_t sizeLimit_;
  /// allocatedBytes_ is the current amount of memory stored in the heap.
  gcheapsize_t allocatedBytes_{0};

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
      MetadataTable metaTable,
      GCCallbacks *gcCallbacks,
      PointerBase *pointerBase,
      const GCConfig &gcConfig,
      std::shared_ptr<CrashManager> crashMgr,
      StorageProvider *provider);

  /// Allocate an object in the GC controlled heap with the size to allocate
  /// given by \p size.
  template <
      bool fixedSizeIgnored = true,
      HasFinalizer hasFinalizer = HasFinalizer::No>
  inline void *alloc(uint32_t size);

  /// Checks if a requested \p size can fit in the heap. If it can't, a
  /// collection occurs. If it still can't after the collection, OOM is
  /// declared.
  void collectBeforeAlloc(uint32_t size);

  /// Same as above, but tries to allocate in a long lived area of the heap.
  /// Use this when the object is known to last for a long period of time.
  /// NOTE: this does nothing different for MallocGC, but does for GenGC.
  template <HasFinalizer hasFinalizer = HasFinalizer::No>
  inline void *allocLongLived(uint32_t size);

  /// Returns whether an external allocation of the given \p size fits
  /// within the maximum heap size.  (Note that this does not guarantee that the
  /// allocation will "succeed" -- the size plus the used() of the heap may
  /// still exceed the max heap size.  But if it fails, the allocation can never
  /// succeed.)
  bool canAllocExternalMemory(uint32_t size);

  /// Collect all of the dead objects and symbols in the heap. Also invalidate
  /// weak pointers that point to dead objects.
  void collect();

  static constexpr uint32_t maxAllocationSize() {
    // MallocGC imposes no limit on individual allocations.
    return std::numeric_limits<uint32_t>::max();
  }

  /// Run the finalizers for all objects.
  void finalizeAll();

  /// \return true iff this is collecting the entire heap, or false if it is
  /// only a portion of the heap.
  /// \pre Assumes inGC() is true, or else this has no meaning.
  bool inFullCollection() const {
    return true;
  }

#ifndef NDEBUG
  /// \return true iff the pointer \p p is controlled by this GC.
  bool validPointer(const void *p) const;

  /// Returns true if \p cell is the most-recently allocated finalizable object.
  bool isMostRecentFinalizableObj(const GCCell *cell) const;
#endif

  /// Same as in superclass GCBase.
  virtual void createSnapshot(llvm::raw_ostream &os, bool compact) override;

  /// Same as in superclass GCBase.
  virtual void serializeWeakRefs(Serializer &s) override;

  /// Same as in superclass GCBase.
  virtual void deserializeWeakRefs(Deserializer &d) override;

  /// Serialze all heap objects to a stream.
  virtual void serializeHeap(Serializer &s) override;

  /// Deserialize heap objects.
  virtual void deserializeHeap(Deserializer &d) override;

  void getHeapInfo(HeapInfo &info) override;
  void getHeapInfoWithMallocSize(HeapInfo &info) override;

  /// @name Weak references
  /// @{

  /// Allocate a weak pointer slot for the value given.
  /// \pre \p init should not be empty or a native value.
  WeakRefSlot *allocWeakSlot(HermesValue init);

  /// Marks a weak reference as being in use.
  void markWeakRef(WeakRefBase &wr);
#ifndef NDEBUG
  /// \return Number of weak ref slots currently in use.
  /// Inefficient. For testing/debugging.
  size_t countUsedWeakRefs() const;
#endif

  /// The largest the size of this heap could ever grow to.
  size_t maxSize() const {
    return maxSize_;
  }

  /// @}

 private:
  /// WeakSlotState is the current state of a weak pointer. It is either:
  ///   Unmarked: It is unknown if it is in use by the program.
  ///   Marked: It is proven to be in use by the program.
  ///   Free: It is proven to NOT be in use by the program.
  enum class WeakSlotState {
    Unmarked,
    Marked,
    Free,
  };

#ifdef HERMES_SLOW_DEBUG
  void checkWellFormed();
#endif

  /// Initialize a cell with the required basic data for any cell.
  inline void initCell(GCCell *cell, uint32_t size);

  /// Free a weak pointer slot, which invalidates it.
  void freeWeakSlot(WeakRefSlot *slot);

  /// See \c GCBase::printStats.
  void printStats(llvm::raw_ostream &os, bool trailingComma) override;

  /// Reset the statistics used for reporting GC information.
  void resetStats();

  /// Sets all weak references to unmarked in preparation for a collection.
  void resetWeakReferences();

  struct MarkingAcceptor;
  struct FullMSCUpdateWeakRootsAcceptor;

  /// Update all of the weak references and invalidate the ones that point to
  /// dead objects.
  void updateWeakReferences();
};

/// @name Free standing functions
/// @{

template <class ToType>
ToType *vmcast_during_gc(GCCell *cell, GC *gc) {
  return static_cast<ToType *>(cell);
}

/// @}

/// @name Inline implementations
/// @{

template <bool fixedSizeIgnored, HasFinalizer hasFinalizer>
inline void *MallocGC::alloc(uint32_t size) {
  size = heapAlignSize(size);
  // Use subtraction to prevent overflow.
  if (LLVM_UNLIKELY(
          shouldSanitizeHandles() || size > sizeLimit_ - allocatedBytes_)) {
    collectBeforeAlloc(size);
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

template <HasFinalizer hasFinalizer>
inline void *MallocGC::allocLongLived(uint32_t size) {
  // Since there is no old generation in this collector, forward to the normal
  // allocation.
  return alloc<true, hasFinalizer>(size);
}

inline bool MallocGC::canAllocExternalMemory(uint32_t size) {
  return size <= maxSize_;
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
