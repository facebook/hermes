/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_COPYGEN_H
#define HERMES_VM_COPYGEN_H

#include "hermes/VM/AllocResult.h"
#include "hermes/VM/HasFinalizer.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/Space.h"

namespace hermes {
namespace vm {

// Forward declaration.
class OldGen;

/// A generation whose preferred mode of collection is a copying evacuation, to
/// be used as the young generation in a generational heap.
class YoungGen : public ContigAllocGCSpace {
 public:
  /// Initialize the CopyGen as a generation in the given GenGC, with
  /// a later generation nextGen.  The bounds lowLim and hiLim define
  /// the address range the generation can use at its maximum size,
  /// while the size argument indicates the suggested initial heap size.
  YoungGen(GenGC *gc, OldGen *nextGen, char *lowLim, char *hiLim, size_t size);

  /// Allocate a new cell of the specified type \p vt and size \p
  /// size.  If necessary perform a GC, which may potentially move
  /// allocated objects.  The \hasFinalizer argument indicates whether
  /// the object being allocated has a finalizer, and the \p
  /// fixedSizeAlloc argument, if true, indicates that the object is a
  /// small fixed-size object, which should always be allocated in the
  /// young generation.  If the allocation is successful, return
  /// {cell, true}, where cell is the allocated GCCell.  If
  /// unsuccessful, return {nullptr, false}.
  inline AllocResult
  alloc(uint32_t sz, HasFinalizer hasFinalizer, bool fixedSizeAlloc = false);

  /// The number of bytes in the generation that are currently in use.
  inline size_t used() const;

  /// The amount of external memory credited to objects allocated in this
  /// generation.
  uint64_t externalMemory() const {
    return externalMemory_;
  }
  /// Sets the amount of external memory charged to object allocated in this
  /// generation.  (Does not change the effective size of the generation.)
  void setExternalMemory(uint64_t size) {
    externalMemory_ = size;
  }

  /// Increase the external memory charged to the generation by the given \p
  /// size.
  void creditExternalMemory(uint32_t size);

  /// Decrease the external memory charged to the generation by the given \p
  /// size.
  void debitExternalMemory(uint32_t size);

  /// The external memory charge of the generation may have changed.  Update the
  /// effective size of the generation to reflect this.
  void updateEffectiveEndForExternalMemory();

  uint64_t extSizeFromFinalizerList() const;

  /// If *hv is a pointer into the current generation, check whether the
  /// referent has already been evacuated.  If not, copy to the old gen, and
  /// install a forwarding pointer in the vtable slot of the referent.  If so,
  /// redirect *hv to point where the already-installed forwarding pointer
  /// points.
  void ensureReferentCopied(HermesValue *hv);

  /// If *ptrLoc is a pointer into the current generation, check
  /// whether the referent has already been evacuated.  If not, copy
  /// to the old gen, and install a forwarding pointer in the vtable
  /// slot of the referent.  If so, redirect *ptrLoc to point where the
  /// already-installed forwarding pointer points.
  void ensureReferentCopied(GCCell **ptrLoc);

  /// Print stats (in JSON format) specific to young-gen collection to an output
  /// stream.
  /// \p os Is the output stream to print the stats to.
  /// \p trailingComma determines whether the output includes a trailing comma.
  void printStats(llvm::raw_ostream &os, bool trailingComma) const;

  /// Finalizes all unreachable cells with finalizers. If the cell was moved to
  /// the old generation, moves a reference to the cell from a list containing
  /// references to cells with finalizers in the young gen to a list containing
  /// references to cells with finalizers in the old gen.
  void finalizeUnreachableAndTransferReachableObjects();

  /// Assumes updateReferences is complete, and that sweepResult is
  /// the result of a previous call to
  /// sweepAndInstallForwardingPointers for this space.  Moves each
  /// live object to the post-compaction address indicated by its
  /// forwarding pointer, and restores the displaced VTable for that
  /// object stored in sweepResult.  Also moves any finalizable objects that
  /// have been moved to the old gen to that generations finalizable object
  /// list.
  void compact(const MarkBitArray &markBits, const SweepResult &sweepResult);

  /// Moves any objects on the young-gen's finalizable object list that have
  /// been moved to the old generation to that generation's finalizable object
  /// list.
  void compactFinalizableObjectList();

  /// Called after the GC's heap has been copied to a new location, in order to
  /// update the references in this space (and the space's own limits) to the
  /// new location.
  ///
  /// This function assumes that \p gc is a valid pointer to this space's owning
  /// GC.
  void moveHeap(GC *gc, ptrdiff_t moveHeapDelta);

#ifdef HERMES_SLOW_DEBUG
  /// Check the well-formedness of the generation.
  void checkWellFormed(const GC *gc) const;
#endif

 private:
  /// Slow path taken when allocation first fails.  If the worst-case
  /// evacuation of this generation could fit in the current size of
  /// the next generation, collect the current generation, then retry
  /// the allocation.  Otherwise, allocate via fullCollectThenAlloc, below.
  /// If the allocation is too large to fit in the young generation,
  /// it may be allocated in the old generation -- if \p fixedSizeAlloc
  /// is false.  If true, then the allocation either succeeds in the
  /// young generation, or fails (and returns null).
  AllocResult curGenAllocFailure(
      uint32_t allocSize,
      HasFinalizer hasFinalizer,
      bool fixedSizeAlloc);

  /// Slow path taken when we can't attempt young-gen collection
  /// because there is insufficient free space in the older generation
  /// to allow worst-case survival.  Do a full collection.  If this
  /// frees enough memory in the old generation to allow worst-case
  /// young-gen evacuation, retry the allocation from scratch.  If
  /// not, try to grow the old generation to allow young-gen
  /// collection.  If this succeeds, also retry the allocation from
  /// scratch.  In both cases, the \p fixedSizeAlloc argument
  /// indicates that the allocation is of a small fixed size, and
  /// should always be satisfied from the young generation.
  /// Otherwise, pay attention for allocations larger than the
  /// young-generation, where allocations in the young generation will
  /// never succeed.  If the strategies above fail, or the allocation
  /// is too large, try allocating directly in the old generation.
  AllocResult fullCollectThenAlloc(
      uint32_t allocSize,
      HasFinalizer hasFinalizer,
      bool fixedSizeAlloc);

#ifndef NDEBUG
  /// In debug builds, we expose the collect call to tests.
 public:
#endif
  /// Do an evacuating collection of the young generation, copying
  /// reachable objects into the nextGen.
  void collect();
#ifndef NDEBUG
 private:
#endif

  /// Assumes that ptr is a pointer into the current space.  If the
  /// vtable slot of *ptr already contains a forwarding pointer,
  /// return that.  Otherwise, copy *ptr into the old gen, install a
  /// forwarding pointer in that vtable slot, and return the address
  /// of the copied GCCell.
  GCCell *forwardPointer(GCCell *ptr);

  /// \return true if the cell is a forwarding pointer to the next gen.
  bool isForwardingPointer(const GCCell *cell) const;

  /// Sets the compactMoveCallback_ field to an appropriate std::function.
  void setCompactMoveCallback();

  /// The GenGC of which this is a part.
  GenGC *gc_;

  /// The older generation.
  OldGen *nextGen_;

  uint64_t externalMemory_{0};

  /// Cumulative by-phase times within young-gen collection.
  double markOldToYoungSecs_ = 0.0;
  double markRootsSecs_ = 0.0;
  double scanTransitiveSecs_ = 0.0;
  double updateWeakRefsSecs_ = 0.0;
  double finalizersSecs_ = 0.0;

  /// The sum of the pre-collection sizes of the young gen before
  /// collection, and the number of bytes promoted.  The latter over
  /// the former will yield the survival rate.
  gcheapsize_t cumPreBytes_ = 0;
  gcheapsize_t cumPromotedBytes_ = 0;
};

size_t YoungGen::used() const {
  return ContigAllocGCSpace::used() + externalMemory();
}

inline AllocResult
YoungGen::alloc(uint32_t sz, HasFinalizer hasFinalizer, bool fixedSizeAlloc) {
  AllocResult res = allocRaw(sz, hasFinalizer);
  if (res.success) {
    return res;
  }
  return curGenAllocFailure(sz, hasFinalizer, fixedSizeAlloc);
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_COPYGEN_H
