/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_SPACE_H
#define HERMES_VM_SPACE_H

#include "hermes/Support/OSCompat.h"
#include "hermes/VM/AllocResult.h"
#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/CompleteMarkState.h"
#include "hermes/VM/GCBase.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/HasFinalizer.h"
#include "hermes/VM/HeapAlign.h"
#include "hermes/VM/MarkBitArray.h"
#include "hermes/VM/SweepResult.h"

#include "llvm/Support/Compiler.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <functional>
#include <vector>

namespace hermes {
namespace vm {

/// A GCSpace is part of a GC heap.  It is a contiguous region of virtual
/// addresses, within which some contiguous subrange is currently in
/// use.  (The remainder is declared unused to the OS; if we use such
/// pages later, they will be zero-filled.)
class GCSpace {
 public:
  /// Creates a GCSpace whose maximum virtual address range is
  /// [lowLim, hiLim), within which [start, end) is initially in use.
  GCSpace(char *lowLim, char *start, char *end, char *hiLim);
  ~GCSpace() {
    __asan_unpoison_memory_region(start_, end_ - start_);
  }

  /// Expose the bounds.
  char *lowLim() const {
    return lowLim_;
  }
  char *start() const {
    return start_;
  }
  char *effectiveEnd() const {
    return effectiveEnd_;
  }
  char *end() const {
    return end_;
  }
  char *hiLim() const {
    return hiLim_;
  }

  /// Returns true if \p p is within the current bounds of the space.
  bool contains(const void *p) const {
    return start_ <= (const char *)p && (const char *)p < end_;
  }

  /// The current "effective" size of the space -- the size less the
  /// amount of external memory credited to the space.
  size_t effectiveSize() const {
    return effectiveEnd_ - start_;
  }

  /// The "real" size of the space -- the difference between "end()" and
  /// "start()".
  size_t size() const {
    return end_ - start_;
  }

  size_t maxSize() const {
    return hiLim_ - lowLim_;
  }

  /// The external memory charge of the generation owning this space may have
  /// changed.  Update the effective end of the space to reflect this.
  void updateEffectiveEndForExternalMemory();

  /// If the space can be grown by the given amount in the high direction,
  /// increase the size by that amount (amount is page-aligned upwards if
  /// necessary) and return true. Otherwise, return false, and leave the
  /// space boundaries unchanged.
  bool growHigh(size_t amount);

#ifndef NDEBUG
  /// The number of allocated objects.
  unsigned numAllocatedObjects() const {
    return numAllocatedObjects_;
  }

  void setNumAllocatedObjects(unsigned allocs) {
    numAllocatedObjects_ = allocs;
  }

  /// Set the contents of the space to a dead value.
  void clear();
  /// Set the given range [start, end) to a dead value.
  void clear(char *start, char *end);
  /// Checks that dead values are present in the [start, end) range.
  void checkUnwritten(char *start, char *end) const;
#endif

 protected:
  /// Dump information about a cell to the given output stream, \p os.
  void dumpCellInfo(llvm::raw_ostream &os, const GCCell *cell) const;

  /// The lower bound of the virtual address region.
  char *lowLim_;
  /// The lower bound of the region currently in use.
  char *start_;
  /// The upper limit of the space that we can currently allocated into;
  /// this may be decreased when externally allocated memory is credited to
  /// the generation owning this space.
  char *effectiveEnd_;
  /// The upper bound of the region currently in use.
  char *end_;
  /// The lower bound of the virtual address region.
  char *hiLim_;

  /// Invariants:
  ///   All the members above are page-size-aligned, and obey:
  ///   lowLim_ <= start_ < effectiveEnd_ <= end_ <= hiLim_

#ifndef NDEBUG
  /// Statistics:
  /// The number of allocated objects.
  unsigned numAllocatedObjects_ = 0;
#endif
};

/// A GC space that provides contiguous allocation, from start_ to end_.
class ContigAllocGCSpace : public GCSpace {
 public:
  /// Creates a ContigGCSpace whose maximum virtual address range is
  /// [lowLim, hiLim), within which [start, end) is initially in use.
  /// Alllocation will start at start_.
  ContigAllocGCSpace(char *lowLim, char *start, char *end, char *hiLim)
      : GCSpace(lowLim, start, end, hiLim),
        level_(start),
        levelAtEndOfLastGC_(start) {}

  /// The number of bytes in the space that are currently allocated.
  size_t used() const {
    return level_ - start_;
  }

  /// The number of bytes in the space that are avialable for allocation.
  size_t available() const {
    return effectiveEnd_ - level_;
  }

  /// Returns the address at which the next allocation, if any, will occur.
  char *level() const {
    return level_;
  };

  /// Attempt an allocation of the given size in the space.  If there
  /// is sufficent space, initialized the space as a GCCell with the
  /// given VTable and size, and returns a pointer to that cell (with
  /// success = true).  If there is not sufficient space, returns
  /// {nullptr, false}.
  inline AllocResult allocRaw(uint32_t size, HasFinalizer hasFinalizer);

  /// The given amount of external memory is credited to objects allocated in
  /// this space.  Adjust the effective size of the space accordingly.
  void creditExternalMemory(uint32_t size);

  /// The given amount of external memory is debited from objects allocated in
  /// this space (perhaps the external memory was explicitly freed.)
  /// Adjust the effective size of the space accordingly.
  void debitExternalMemory(uint32_t size);

  // MarkSweepCompact support.

  /// Assume that marking from the roots has set some bits in
  /// markBits, and the markStack found in markState is empty.  Traverse
  /// the objects in the space, finding objects whose mark bits are set.
  /// For each such object, push its address on the markStack, then enter
  /// a loop in which we pop objects from markStack while it is
  /// non-empty, and scan their pointers, marking unmarked referents,
  /// and pushing their addresses on markStack.  If we overflow the
  /// max size of markStack sets a flag; we return whether mark stack
  /// overflow occurred, since this may require this traversal to be
  /// redone in other spaces.
  /// (Note: markState is passed in from the caller because we use the
  /// gc->mark functions, so we must use a markState that's a member
  /// of gc and that contains a markStack.)
  bool
  completeMarking(GC *gc, MarkBitArray *markBits, CompleteMarkState *markState);

  /// A CompactionRegion is a contiguous region of memory, into which compaction
  /// is performed.  The start and end fields define the region.
  /// When we compact a Space into a CompactionRegion, via
  /// sweepAndInstallForwardingPointers, we update start to point to
  /// the first byte after the last object compacted into the space.
  /// The numAllocated field, in debug builds, is the number of objects
  /// compacted into this CompactionRegion.
  struct CompactionRegion {
    char *start;
    char *const end;
#ifndef NDEBUG
    unsigned numAllocated = 0;
#endif
    CompactionRegion(const ContigAllocGCSpace &space)
        : start(space.start()), end(space.effectiveEnd()) {}
  };

  /// Assumes marking is complete.  Scans the heap, determining, for
  /// each live object, the address to which it will later be
  /// compacted.  Objects are compacted into the address ranges
  /// specified by compactionRegions, in the order they appear.  The
  /// first object that doesn't fit in a region causes this region to
  /// be complete, and further allocation to occur in the next.  We
  /// require that compactionRegions have enough space to contain all
  /// the live objects in the current space; this can usually be
  /// guaranteed trivially when we're compacting "in place."  For each
  /// live object, installs a forwarding pointer to the
  /// post-compaction address in the VTable slot, after saving the
  /// VTable value in a vector, which becomes the return result of the
  /// call.  Dead objects with finalizers have their finalizers
  /// executed.  Sequences of dead objects have a DeadRegion
  /// containing a pointer to the next live object inserted, allowing
  /// them to be skipped efficiently n subsequent heap traversals.
  std::vector<const VTable *> sweepAndInstallForwardingPointers(
      GC *gc,
      MarkBitArray *markBits,
      std::vector<CompactionRegion *> *compactionRegions);

  /// Assumes sweeping is complete.  Traverses the live objects,
  /// scanning their pointers.  For each pointer to another heap
  /// object, update the pointer by following the referent's
  /// forwarding pointer.  Uses the given markBits to indicate
  /// liveness, and assumes that displaceVtablePtrs contains displaced
  /// VTables in 1-1 correspondence with the live objects in the space.
  void updateReferences(
      GC *gc,
      const MarkBitArray &markBits,
      const std::vector<const VTable *> &displacedVtablePtrs);

  /// Assumes updateReferences is complete, and that sweepResult is
  /// the result of a previous call to
  /// sweepAndInstallForwardingPointers for this space.  Moves each
  /// live object to the post-compaction address indicated by its
  /// forwarding pointer, and restores the displaced VTable for that
  /// object stored in sweepResult.
  void compact(const MarkBitArray &markBits, const SweepResult &sweepResult);

  /// Uses the MarkBitArray to determine which cells with finalizers are
  /// unreachable, if so calling their finalize method.
  void finalizeUnreachableObjects(GC *gc, MarkBitArray *markBits);

  /// Iterates through a list of all cells with finalizers, updating the
  /// GCCell references to their post-compaction locations.
  void updateFinalizableCellListReferences();

  /// Called after the GC's heap has been copied to a new location, in order to
  /// update the references in this space (and the space's own limits) to the
  /// new location.
  ///
  /// This function assumes that \p gc is a valid pointer to this space's owning
  /// GC.
  void moveHeap(GC *gc, ptrdiff_t moveHeapDelta);

  gcheapsize_t bytesAllocatedSinceLastGC() const {
    return level() - levelAtEndOfLastGC_;
  }

  /// Record the level at the end of a GC.  At the start of next
  /// young-gen GC, objects at addresses at or greater than this were
  /// allocated directly in the OldGen.
  void recordLevelAtEndOfLastGC();

#ifndef NDEBUG
  /// Invokes the given callback on all the GCCells allocated directly
  /// in the Space since the end of the last GC.
  void forObjsAllocatedSinceGC(std::function<void(GCCell *)> callback);

  /// Statistics

  /// The number of reachable objects in the space.
  unsigned numReachableObjects() const {
    return numReachableObjects_;
  }

  /// Reset the number of reachable objects in the space to zero.
  void resetNumReachableObjects() {
    numReachableObjects_ = 0;
  }

  /// Inc the number of hidden classes (leaf-only if \p leafOnly) by
  /// the given \p val.
  void incNumHiddenClasses(bool leafOnly, unsigned val) {
    if (leafOnly) {
      numLeafHiddenClasses_ += val;
    } else {
      numHiddenClasses_ += val;
    }
  }

  /// The number of reachable objects in the space.
  unsigned numHiddenClasses(bool leafOnly = false) const {
    return leafOnly ? numLeafHiddenClasses_ : numHiddenClasses_;
  }

  /// Reset the number of hidden classes in the space to zero.
  void resetNumHiddenClasses() {
    numLeafHiddenClasses_ = 0;
    numHiddenClasses_ = 0;
  }

#endif

  /// \return the sum of all the malloc sizes of objects in this space.
  size_t countMallocSize();

  /// The number of finalized objects in the space.
  unsigned numFinalizedObjects() const {
    return numFinalizedObjects_;
  }

  /// Reset the number of finalized objects in the space to zero.
  void resetNumFinalizedObjects() {
    numFinalizedObjects_ = 0;
  }

#ifdef HERMES_SLOW_DEBUG
  /// For debugging: iterates over objects, asserting that all GCCells have
  /// vtables with valid cell kinds, and that all object pointers point to
  /// GCCells whose vtables have valid cell kinds.  Sums the external memory
  /// rooted by objects in the space, and, if \p externalMemory is non-null,
  /// sets \p *externalMemory to that sum.
  void checkWellFormed(const GC *gc, uint64_t *externalMemory = nullptr) const;
#endif

  /// Dump detailed heap contents to the given output stream, \p os.
  void dump(llvm::raw_ostream &os) const;

#ifndef NDEBUG
  /// Returns true iff \p p is located within a valid section of the space, and
  /// not at dead memory.
  bool validPointer(const void *p) const;

  /// Returns the space's finalizer list, for debugging purposes.
  const std::vector<GCCell *> &cellsWithFinalizers() const {
    return cellsWithFinalizers_;
  }
#endif

 protected:
  /// The next address to be allocated.  Aligned according to HeapAlign.
  char *level_;

  /// This is originally start_, and records the level at the end of
  /// the last GC.  At the start of a young-gen GC, objects
  /// at addresses at or greater than this were allocated directly in
  /// the Space since the last GC.
  char *levelAtEndOfLastGC_;

/// Statistics:
#ifndef NDEBUG
  /// Number of objects in the space found reachable during collection.
  unsigned numReachableObjects_ = 0;
  /// The number of hidden classes reachable in this space at the last
  /// collection, and  the number of those that were leaf classes.
  unsigned numHiddenClasses_ = 0;
  unsigned numLeafHiddenClasses_ = 0;
#endif
  /// Number of objects in this space finalized in the last collection.
  unsigned numFinalizedObjects_ = 0;

  /// If set, a callback that will be invoked during compaction,
  /// whenever the an object is compacted to [start, end).  These
  /// calls will be made in increasing order of start address, and the
  /// regions will not overlap.
  /// TODO: measure whether there's a penalty for using std::function,
  /// vs. a normal function pointer with an extra context argument.
  std::function<void(char *start, char *end)> compactMoveCallback_;

  /// Keeps track of all cells on the heap with finalizers.
  std::vector<GCCell *> cellsWithFinalizers_;

#ifndef NDEBUG
  /// Assumes that \p low is the address of an allocated GC cell.
  /// Invokes \p callback on all GCCells whose start address is in [low, high).
  void
  forObjsInRange(std::function<void(GCCell *)> callback, char *low, char *high);

  /// Invokes callback on all GCCells in the space.
  void forAllObjs(std::function<void(GCCell *)> callback) {
    forObjsInRange(callback, start_, level_);
  }
#endif
};

// Inline function implementations

inline AllocResult ContigAllocGCSpace::allocRaw(
    uint32_t size,
    HasFinalizer hasFinalizer) {
  assert(size >= sizeof(GCCell) && "cell must be larger than GCCell");
  size = heapAlignSize(size);

  char *cellPtr; // Initialized in the if below.
  // On 64-bit systems, we know that we can't allocate a size large enough to
  // cause a pointer value to overflow. This is not true on 32-bit systems. This
  // test should be decided statically.
  // TODO: Is there a portable way of expressing this in the preprocessor?
  if (sizeof(void *) == 8) {
    // Calculate the new level_ once.
    char *newLevel = level_ + size;
    if (LLVM_UNLIKELY(newLevel > effectiveEnd_)) {
      return {nullptr, false};
    }
    cellPtr = level_;
    level_ = newLevel;
  } else {
    if (LLVM_UNLIKELY(static_cast<size_t>(effectiveEnd_ - level_) < size)) {
      return {nullptr, false};
    }
    cellPtr = level_;
    level_ += size;
  }
  auto *cell = reinterpret_cast<GCCell *>(cellPtr);
  __asan_unpoison_memory_region(cellPtr, size);
#ifndef NDEBUG
  checkUnwritten(cellPtr, cellPtr + size);
  numAllocatedObjects_++;
#endif
  if (hasFinalizer == HasFinalizer::Yes)
    cellsWithFinalizers_.push_back(cell);
  return {cell, true};
}

std::unique_ptr<SlotAcceptor> getMoveHeapAcceptor(GC &gc, std::ptrdiff_t delta);
std::unique_ptr<FullMSCUpdateAcceptor> getFullMSCUpdateAcceptor(GC &gc);

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SPACE_H
