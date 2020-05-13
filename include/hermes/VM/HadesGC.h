/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_HADESGC_H
#define HERMES_VM_HADESGC_H

#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/GCBase.h"

#include <cstdint>
#include <deque>
#include <memory>
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
    return AlignedHeapSegment::maxSize();
  }

  /// \name GCBase overrides
  /// \{

  void getHeapInfo(HeapInfo &info) override;
  void getHeapInfoWithMallocSize(HeapInfo &info) override;
  void getCrashManagerHeapInfo(CrashManager::HeapInformation &info) override;
  void createSnapshot(llvm::raw_ostream &os) override;
  void printStats(llvm::raw_ostream &os, bool trailingComma) override;

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
  ///   have a finalizer.
  template <HasFinalizer hasFinalizer = HasFinalizer::No>
  inline void *allocLongLived(uint32_t sz);

  /// Force a garbage collection cycle.
  /// (Part of general GC API defined in GCBase.h).
  void collect();

  /// Run the finalizers for all heap objects.
  void finalizeAll();

  /// \name Write Barriers
  /// \{

  /// NOTE: For all write barriers and read barriers:
  /// The call to writeBarrier/readBarrier must happen *before* the write/read
  /// to memory occurs.

  /// The given value is being written at the given loc (required to
  /// be in the heap). If value is a pointer, execute a write barrier.
  void writeBarrier(void *loc, HermesValue value);

  /// The given pointer value is being written at the given loc (required to
  /// be in the heap). The value is may be null. Execute a write barrier.
  void writeBarrier(void *loc, void *value);

  /// We copied HermesValues into the given region. Note that \p numHVs is
  /// the number of HermesValues in the the range, not the char length.
  /// Do any necessary barriers.
  ///
  /// \pre The range described must be wholly contained within one segment of
  ///     the heap.
  void writeBarrierRange(GCHermesValue *start, uint32_t numHVs);

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

  /// \}

  /// \return true if the pointer lives in the young generation.
  bool inYoungGen(const void *p) const;

#ifndef NDEBUG
  /// \name Debug APIs
  /// \{

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

  class HeapSegment;
  class CollectionSection;
  class EvacAcceptor;
  class MarkAcceptor;
  class WeakRootAcceptor;

 private:
  const uint64_t maxHeapSize_;

  /// Keeps the storage provider alive until after the GC is fully destructed.
  std::shared_ptr<StorageProvider> provider_;

  /// youngGen is a bump-pointer space, so it can re-use AlignedHeapSegment.
  std::unique_ptr<HeapSegment> youngGen_;
  /// List of cells in YG that have finalizers. Iterate through this to clean
  /// them out.
  std::vector<GCCell *> youngGenFinalizables_;

  /// oldGen_ is a free list space, so it needs a different segment
  /// representation.
  std::vector<std::unique_ptr<HeapSegment>> oldGen_;

  /// weakPointers_ is a list of all the weak pointers in the system. They are
  /// invalidated if they point to an object that is dead, and do not count
  /// towards whether an object is live or dead.
  std::deque<WeakRefSlot> weakPointers_;

  /// The main entrypoint for all allocations.
  /// \param sz The size of allocation requested. This might be rounded up to
  ///   fit heap alignment requirements.
  /// \param longLived If true, allocate directly into OG, instead of YG.
  /// \param fixedSize If true, the allocation is of a cell type that always has
  ///   the same size. The requirement enforced by Hades is that all fixed-size
  ///   allocations must go into YG, unless \p longLived is also true.
  /// \param hasFinalizer If true, the cell about to be allocated into the
  ///   requested space will have a finalizer that the GC will need to invoke.
  void *allocWork(
      uint32_t sz,
      bool longLived,
      bool fixedSize,
      HasFinalizer hasFinalizer);

  /// Allocate into OG. Returns a pointer to the newly allocated space. That
  /// space must be filled immediately after this call completes.
  /// \return A non-null pointer to memory in the old gen that should have a
  ///   constructor run in immediately.
  /// \post This function either successfully allocates, or reports OOM.
  GCCell *oldGenAlloc(uint32_t sz);

  /// Searches the OG for a space to allocate memory into.
  /// \return A pointer to uninitialized memory that can be written into, null
  ///   if no such space exists.
  /// NOTE: oldGenAlloc should be called instead, which will try to do
  /// collections until this function returns a non-null pointer.
  GCCell *oldGenSearch(uint32_t sz);

  /// Frees the weak slot, so it can be re-used by future WeakRef allocations.
  void freeWeakSlot(WeakRefSlot *slot);

  /// Perform a YG garbage collection. All live objects in YG will be evacuated
  /// to the OG.
  /// \post The YG is completely empty, and all bytes are available for new
  ///   allocations.
  void youngGenCollection(bool allowOGBegin);

  /// Perform an OG garbage collection. All live objects in OG will be left
  /// untouched, all unreachable objects will be placed into a free list that
  /// can be used by \c oldGenAlloc.
  void oldGenCollection();

  /// Find all pointers from OG into YG during a YG collection. This is done
  /// quickly through use of write barriers that detect the creation of OG-to-YG
  /// pointers.
  void scanDirtyCards(EvacAcceptor &acceptor);

  /// Finalize all objects in YG that have finalizers.
  void finalizeYoungGenObjects();

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
  uint64_t allocatedBytes();

  /// Return the total number of bytes that are in use by the OG section of the
  /// JS heap.
  uint64_t oldGenAllocatedBytes();

  /// Accessor for the YG.
  HeapSegment &youngGen();
  const HeapSegment &youngGen() const;

  /// Accessors for the segments of OG.
  std::vector<std::unique_ptr<HeapSegment>>::iterator oldGenBegin();
  std::vector<std::unique_ptr<HeapSegment>>::const_iterator oldGenBegin() const;

  std::vector<std::unique_ptr<HeapSegment>>::iterator oldGenEnd();
  std::vector<std::unique_ptr<HeapSegment>>::const_iterator oldGenEnd() const;

  /// Create a new OG segment and attach it to the end of the OG segment vector.
  /// \return a reference to the newly created segment.
  HeapSegment &createOldGenSegment();

  /// Searches the old gen for this pointer. This is O(number of OG segments).
  /// NOTE: In any non-debug case, \c inYoungGen should be used instead, because
  /// it is O(1).
  /// \return true if the pointer is in the old gen.
  bool inOldGen(const void *p) const;

#ifdef HERMES_SLOW_DEBUG
  /// Checks the heap to make sure all cells are valid.
  void checkWellFormed();

  /// Verify that the card table used to find pointers from OG into YG has the
  /// correct cards dirtied, given the contents of the OG currently.
  void verifyCardTable();
  void verifyCardTableBoundaries() const;
#endif
};

/// \name Inline implementations
/// \{

template <bool fixedSize, HasFinalizer hasFinalizer>
void *HadesGC::alloc(uint32_t sz) {
  return allocWork(sz, false, fixedSize, hasFinalizer);
}

template <HasFinalizer hasFinalizer>
void *HadesGC::allocLongLived(uint32_t sz) {
  return allocWork(sz, true, false, hasFinalizer);
}

/// \}

} // namespace vm
} // namespace hermes
#endif
