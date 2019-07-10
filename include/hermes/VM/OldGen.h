/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_OLDGEN_H
#define HERMES_VM_OLDGEN_H

#include "hermes/VM/AllocResult.h"
#include "hermes/VM/CardObjectTable.h"
#include "hermes/VM/CardTable.h"
#include "hermes/VM/HasFinalizer.h"
#include "hermes/VM/Space.h"

#include <cstddef>
#include <vector>

namespace hermes {
namespace vm {

/// A generation that can function as the old generation in a
/// two-generation system.  Supports finding old-to-young pointers.
class OldGen : public ContigAllocGCSpace {
 public:
  /// Initialize the OldGen as a generation in the given GenGC.  The
  /// bounds lowLim and hiLim define the address range the generation
  /// can use at its maximum size, while the size argument indicates
  /// the suggested initial heap size.
  OldGen(GenGC *gc, char *lowLim, char *hiLim, size_t size);

  /// Find all pointers in the current generation that point into
  /// youngGen, and apply the current mark function to them.
  void markYoungGenPointers(GCSpace *youngGen);

  /// Static override of ContigAllocGCSpace::moveHeap.
  void moveHeap(GC *gc, ptrdiff_t moveHeapDelta);

  /// Allocate a new cell of the specified type \p vt and size \p size.
  /// If necessary perform a GC, which may potentially move allocated
  /// objects.  Returns {cell, true}, where cell is the allocated
  /// GCCell, if successful, otherwise {nullptr, false}.
  inline AllocResult alloc(uint32_t size, HasFinalizer hasFinalizer);

  /// Attempt the given allocation.  If it succeeds, update the card
  /// object table, if necessary.  Returns {cell, true}, where cell is
  /// the allocated GCCell, if successful, otherwise {nullptr, false}.
  inline AllocResult allocRaw(uint32_t size, HasFinalizer hasFinalizer);

  /// The numbe of bytes in the generation that are currently in use.
  inline size_t used() const;

  /// The amount of external memory credited to objects allocated in this
  /// generation.
  uint64_t externalMemory() const {
    return externalMemory_;
  }

  /// Increase the external memory credited to the generation by the given \p
  /// size.
  void creditExternalMemory(uint32_t size);

  /// Decrease the external memory charged to the generation by the given \p
  /// size.
  void debitExternalMemory(uint32_t size);

  /// The external memory charge of the generation may have changed.  Update the
  /// effective size of the generation to reflect this.
  void updateEffectiveEndForExternalMemory();

  /// Do any operations necessary to prepare for compaction.  This
  /// includes setting the variables that are consulted during
  /// compaction for recomputing the card object table.
  void prepareForCompaction();

  CardTable &cardTable() {
    return cardTable_;
  }

  const CardTable &cardTable() const {
    return cardTable_;
  }

  CardObjectTable *cardObjectTable() {
    return &cardObjectTable_;
  }

#ifdef HERMES_SLOW_DEBUG
  /// Verify the accuracy of the card object table: for each card in
  /// range, find the crossing object, and verify that it has a valid
  /// VTable.
  void verifyCardObjectTable() const;
#endif

  /// Used by young gen to transfer references to cells with finalizers over to
  /// the old gen's list of references to cells with finalizers.
  void addToFinalizerList(GCCell *cell);

  /// Calls growHigh() from ContigAllocGCSpace, resizing the MarkBitArray
  /// of the corresponding GC if needed.
  bool growHigh(size_t amount);

  /// Callback to notify OldGen that its size bounds have changed, and any
  /// dependent memory regions should be updated.
  void didResize();

  /// Callback to notify OldGen that its level has changed, most likely due to
  /// a collection. This gives it the opportunity to clean up any resources
  /// being used to serve the range between the level before and its current
  /// position, when the level became smaller.
  ///
  /// \p levelBefore The previous position of the level.
  void levelChangedFrom(char *levelBefore);

#ifdef HERMES_SLOW_DEBUG
  /// Check the well-formedness of the generation.
  void checkWellFormed(const GC *gc) const;
#endif

 private:
  void *fullCollectThenAlloc(uint32_t allocSize, HasFinalizer hasFinalizer);

  /// The GenGC of which this is a part.
  GenGC *gc_;

  uint64_t externalMemory_{0};

  /// The card table.
  CardTable cardTable_;

  /// The card object table: map from card index (in the old
  /// generation) to a value indicating how far back to go to find the
  /// start of the first object that extends onto the card
  /// corresponding to the index.
  CardObjectTable cardObjectTable_;

  /// The original allocation point of the old generation during dirty card
  /// scanning.  Do not scan dirty card slots at or above this point.
  char *originalLevel_;
};

size_t OldGen::used() const {
  return ContigAllocGCSpace::used() + externalMemory();
}

inline AllocResult OldGen::alloc(uint32_t size, HasFinalizer hasFinalizer) {
  // Define the symbol above to use it locally.
  AllocResult result = allocRaw(size, hasFinalizer);
  if (LLVM_LIKELY(result.success)) {
    return result;
  }
  return {fullCollectThenAlloc(size, hasFinalizer), true};
}

inline AllocResult OldGen::allocRaw(uint32_t size, HasFinalizer hasFinalizer) {
  AllocResult res = ContigAllocGCSpace::allocRaw(size, hasFinalizer);
  if (LLVM_UNLIKELY(!res.success)) {
    return {res.ptr, false};
  }
  char *resPtr = reinterpret_cast<char *>(res.ptr);
  char *nextAllocPtr = resPtr + size;
  if (nextAllocPtr > cardObjectTable_.nextCardBoundary()) {
    cardObjectTable_.updateEntries(resPtr, nextAllocPtr);
  }
  return {res.ptr, true};
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_OLDGEN_H
