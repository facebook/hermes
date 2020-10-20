/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/GenGCHeapSegment.h"

#include "hermes/VM/CompactionResult-inline.h"
#include "hermes/VM/DeadRegion.h"
#include "hermes/VM/GCGeneration.h"
#include "hermes/VM/HiddenClass.h"

namespace hermes {
namespace vm {

GenGCHeapSegment::GenGCHeapSegment(
    AlignedStorage &&storage,
    GCGeneration *owner)
    : AlignedHeapSegment(std::move(storage)), generation_(owner) {
  // Storage end must be page-aligned so that markUnused below stays in segment.
  assert(
      reinterpret_cast<uintptr_t>(hiLim()) % oscompat::page_size() == 0 &&
      "storage end must be page-aligned");
  if (*this) {
    new (contents()) Contents();
    contents()->protectGuardPage(oscompat::ProtectMode::None);
  }
}

void GenGCHeapSegment::sweepAndInstallForwardingPointers(
    GC *gc,
    SweepResult *sweepResult) {
  deleteDeadObjectIDs(gc);
  MarkBitArrayNC &markBits = markBitArray();
  char *ptr = start();
  size_t ind = markBits.addressToIndex(ptr);
  ind = markBits.findNextMarkedBitFrom(ind);

  size_t indexLimit = markBits.addressToIndex(level() - 1) + 1;
  // We will set adjacentPtr to point just after each marked object.  Thus,
  // if there is a gap in the sequence of marked objects, it will indicate the
  // beginning of a dead region.
  char *adjacentPtr = ptr;

  auto &compactionResult = sweepResult->compactionResult;
  auto *chunk = compactionResult.activeChunk();
  do {
    auto allocator = chunk->allocator();

    // Each iteration of this loop fills the current compactionResult
    // as much as possible.
    for (; ind < indexLimit; ind = markBits.findNextMarkedBitFrom(ind + 1)) {
      ptr = markBits.indexToAddress(ind);
      GCCell *cell = reinterpret_cast<GCCell *>(ptr);
      auto cellSize = cell->getAllocatedSize();
      auto trimmedSize = cell->getVT()->getTrimmedSize(cell, cellSize);
      auto res = allocator.alloc(trimmedSize);
      if (!res.success) {
        // The current chunk is exhausted; must move on to the next.
        break;
      }

#ifndef NDEBUG
      assert(generation_ && "Must have an owning generation");
      generation_->incNumReachableObjects();
      if (auto *hiddenClass = dyn_vmcast<HiddenClass>(cell)) {
        generation_->incNumHiddenClasses();
        generation_->incNumLeafHiddenClasses(hiddenClass->isKnownLeaf());
      }
      gc->trackReachable(cell->getKind(), cellSize);
#endif

      if (ptr != adjacentPtr) {
        new (adjacentPtr) DeadRegion(ptr - adjacentPtr);
      }

      sweepResult->displacedVtablePtrs.push_back(cell->getVT());
      cell->setForwardingPointer(reinterpret_cast<GCCell *>(res.ptr));
      adjacentPtr = ptr += cellSize;
    }

    allocator.recordLevel();

    // If we completed the iteration over the indices, then exit the loop.
    if (ind >= indexLimit) {
      break;
    }
  } while ((chunk = compactionResult.nextChunk()));
  assert(ind >= indexLimit && "We didn't have enough space to compact into");

  if (adjacentPtr < level_) {
    new (adjacentPtr) DeadRegion(level_ - adjacentPtr);
  }
}

} // namespace vm
} // namespace hermes
