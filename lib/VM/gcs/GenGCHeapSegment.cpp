/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/GenGCHeapSegment.h"

#include "hermes/VM/CheckHeapWellFormedAcceptor.h"
#include "hermes/VM/CompactionResult-inline.h"
#include "hermes/VM/CompleteMarkState-inline.h"
#include "hermes/VM/DeadRegion.h"
#include "hermes/VM/GCBase-inline.h"
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

void GenGCHeapSegment::creditExternalMemory(uint32_t size) {
  // Decrease effectiveEnd_ by size, but not below level_.  Be careful of
  // overflow. The cast to size_t below is justified by this invariant:
  assert(effectiveEnd_ >= level_);
  if (static_cast<size_t>(effectiveEnd_ - level_) >= size) {
    effectiveEnd_ -= size;
  } else {
    effectiveEnd_ = level_;
  }
}

void GenGCHeapSegment::debitExternalMemory(uint32_t size) {
  // Increase effectiveEnd_ by size, but not beyond end_.  Be careful of
  // overflow. The cast to size_t below is justified by this invariant:
  assert(effectiveEnd_ <= end_);
  if (static_cast<size_t>(end_ - effectiveEnd_) >= size) {
    effectiveEnd_ += size;
  } else {
    effectiveEnd_ = end_;
  }
}

void GenGCHeapSegment::completeMarking(GC *gc, CompleteMarkState *markState) {
  assert(!markState->markStackOverflow_);

  // Return early if nothing was allocated.
  if (used() == 0) {
    return;
  }

  CompleteMarkState::FullMSCMarkTransitiveAcceptor acceptor(*gc, markState);

  MarkBitArrayNC &markBits = markBitArray();

  char *ptr = start();
  size_t ind = markBits.addressToIndex(ptr);
  assert(level_ > start() && "We return above if level_ == start()");
  size_t indexLimit = markBits.addressToIndex(level_ - 1) + 1;
  for (ind = markBits.findNextMarkedBitFrom(ind); ind < indexLimit;
       ind = markBits.findNextMarkedBitFrom(ind + 1)) {
    ptr = markBits.indexToAddress(ind);
    GCCell *cell = reinterpret_cast<GCCell *>(ptr);

    markState->currentParPointer = cell;
    if (cell->isVariableSize()) {
      markState->varSizeMarkStack_.push_back(cell);
    } else {
      markState->markStack_.push_back(cell);
    }
    markState->drainMarkStack(gc, acceptor);

    if (LLVM_UNLIKELY(markState->markStackOverflow_)) {
      markState->markStack_.clear();
      markState->varSizeMarkStack_.clear();
      break;
    }
  }

  assert(markState->markStack_.empty());
  assert(markState->varSizeMarkStack_.empty());
}

void GenGCHeapSegment::deleteDeadObjectIDs(GC *gc) {
  GCBase::IDTracker &idTracker = gc->getIDTracker();
  GCBase::AllocationLocationTracker &allocationLocationTracker =
      gc->getAllocationLocationTracker();
  if (gc->isTrackingIDs()) {
    MarkBitArrayNC &markBits = markBitArray();
    // Separate out the delete tracking into a different loop in order to keep
    // the normal case fast.
    forAllObjs([&markBits, &idTracker, &allocationLocationTracker](
                   const GCCell *cell) {
      if (!markBits.at(markBits.addressToIndex(cell))) {
        // The allocation tracker needs to use the ID, so this needs to come
        // before untrackObject.
        allocationLocationTracker.freeAlloc(cell, cell->getAllocatedSize());
        idTracker.untrackObject(cell);
      }
    });
  }
}

void GenGCHeapSegment::updateObjectIDs(
    GC *gc,
    SweepResult::VTablesRemaining &vTables) {
  if (!gc->isTrackingIDs()) {
    // If ID tracking isn't on, there's nothing to do here.
    return;
  }

  GCBase::IDTracker &idTracker = gc->getIDTracker();
  SweepResult::VTablesRemaining vTablesCopy{vTables};
  MarkBitArrayNC &markBits = markBitArray();
  char *ptr = start();
  size_t ind = markBits.addressToIndex(ptr);
  while (ptr < level()) {
    if (markBits.at(ind)) {
      auto *cell = reinterpret_cast<GCCell *>(ptr);
      idTracker.moveObject(cell, cell->getForwardingPointer());
      const VTable *vtp = vTablesCopy.next();
      auto cellSize = cell->getAllocatedSize(vtp);
      ptr += cellSize;
      ind += (cellSize >> LogHeapAlign);
    } else {
      auto *deadRegion = reinterpret_cast<DeadRegion *>(ptr);
      ptr += deadRegion->size();
      ind += (deadRegion->size() >> LogHeapAlign);
    }
  }
}

void GenGCHeapSegment::updateReferences(
    GC *gc,
    FullMSCUpdateAcceptor *acceptor,
    SweepResult::VTablesRemaining &vTables) {
  updateObjectIDs(gc, vTables);

  MarkBitArrayNC &markBits = markBitArray();
  char *ptr = start();
  size_t ind = markBits.addressToIndex(ptr);
  while (ptr < level()) {
    if (markBits.at(ind)) {
      GCCell *cell = reinterpret_cast<GCCell *>(ptr);
      // Get the VTable.
      assert(vTables.hasNext() && "Need a displaced vtable pointer");
      const VTable *vtp = vTables.next();
      // Scan the pointer fields, updating via forwarding pointers.
      GCBase::markCell(cell, vtp, gc, *acceptor);
      uint32_t cellSize = cell->getAllocatedSize(vtp);
      ptr += cellSize;
      ind += (cellSize >> LogHeapAlign);
    } else {
      auto *deadRegion = reinterpret_cast<DeadRegion *>(ptr);
      ptr += deadRegion->size();
      ind += (deadRegion->size() >> LogHeapAlign);
    }
  }
}

void GenGCHeapSegment::compact(SweepResult::VTablesRemaining &vTables) {
  // If we're using ASAN, we've poisoned the unallocated portion of the space;
  // unpoison that now, since we may copy into it.
  __asan_unpoison_memory_region(level(), end() - level());
  MarkBitArrayNC &markBits = markBitArray();
  char *ptr = start();
  size_t ind = markBits.addressToIndex(ptr);
  while (ptr < level()) {
    if (markBits.at(ind)) {
      GCCell *cell = reinterpret_cast<GCCell *>(ptr);
      // Read the new address from the forwarding pointer.
      char *newAddr = reinterpret_cast<char *>(cell->getForwardingPointer());
      // Put back the vtable.
      assert(vTables.hasNext() && "Need a displaced vtable pointer");
      cell->setForwardingPointer(
          reinterpret_cast<const GCCell *>(vTables.next()));
      assert(
          cell->isValid() &&
          "Cell was invalid after placing the vtable back in");
      // Must read this now, since the memmove below might overwrite it.
      auto cellSize = cell->getAllocatedSize();
      const bool canBeCompacted = cell->getVT()->canBeTrimmed();
      const auto trimmedSize = cell->getVT()->getTrimmedSize(cell, cellSize);
      if (newAddr != ptr) {
        std::memmove(newAddr, ptr, trimmedSize);
      }
      if (canBeCompacted) {
        // Set the new cell size.
        auto *newCell = reinterpret_cast<VariableSizeRuntimeCell *>(newAddr);
        newCell->setSizeFromGC(trimmedSize);
        newCell->getVT()->trim(newCell);
      }

      ptr += cellSize;
      ind += (cellSize >> LogHeapAlign);
    } else {
      auto *deadRegion = reinterpret_cast<DeadRegion *>(ptr);
      ptr += deadRegion->size();
      ind += (deadRegion->size() >> LogHeapAlign);
    }
  }
}

void GenGCHeapSegment::forObjsInRange(
    const std::function<void(GCCell *)> &callback,
    char *low,
    const char *high) {
  assert(low >= start());
  assert(high <= effectiveEnd());
  char *ptr = low;
  while (ptr < high) {
    GCCell *cell = reinterpret_cast<GCCell *>(ptr);
    callback(cell);
    ptr += cell->getAllocatedSize();
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

void GenGCHeapSegment::forObjsInRange(
    const std::function<void(const GCCell *)> &callback,
    const char *low,
    const char *high) const {
  assert(low >= start());
  assert(high <= effectiveEnd());
  const char *ptr = low;
  while (ptr < high) {
    const GCCell *cell = reinterpret_cast<const GCCell *>(ptr);
    callback(cell);
    ptr += cell->getAllocatedSize();
  }
}

void GenGCHeapSegment::forAllObjs(
    const std::function<void(GCCell *)> &callback) {
  forObjsInRange(callback, start(), level());
}

void GenGCHeapSegment::forAllObjs(
    const std::function<void(const GCCell *)> &callback) const {
  forObjsInRange(callback, start(), level());
}

void AlignedHeapSegment::addExtentToString(char **buf, int *sz) {
  int n = snprintf(*buf, *sz, "{lo: \"%p\", hi: \"%p\"}", lowLim(), hiLim());
  *buf += n;
  *sz -= n;
}

#ifdef HERMES_EXTRA_DEBUG
size_t GenGCHeapSegment::summarizeVTablesWork(const char *level) const {
  std::hash<const void *> hash;
  size_t res = 0;
  forObjsInRange(
      [&res, hash](const GCCell *cell) { res += hash(cell->getVT()); },
      start(),
      level);
  return res;
}

void GenGCHeapSegment::summarizeVTables() {
  lastVTableSummary_ = summarizeVTablesWork(level());
  lastVTableSummaryLevel_ = level();
}

bool GenGCHeapSegment::checkSummarizedVTables() const {
  return lastVTableSummary_ == summarizeVTablesWork(lastVTableSummaryLevel_);
}
#endif

#ifdef HERMES_SLOW_DEBUG
void GenGCHeapSegment::checkWellFormed(const GC *gc, uint64_t *externalMemory)
    const {
  CheckHeapWellFormedAcceptor acceptor(*const_cast<GC *>(gc));
  char *ptr = start();
  uint64_t extSize = 0;
  while (ptr < level()) {
    GCCell *cell = (GCCell *)ptr;
    assert(cell->isValid() && "cell is invalid");
    // We assume that CheckHeapWellFormedAcceptor does not mutate the GC.  Thus
    // it's OK to cast away the const on \p gc.
    GCBase::markCell(cell, const_cast<GC *>(gc), acceptor);
    ptr += cell->getAllocatedSize();
    extSize += cell->externalMemorySize();
  }

  checkUnwritten(ptr, end());

  if (externalMemory) {
    *externalMemory = extSize;
  }
}
#endif // HERMES_SLOW_DEBUG

} // namespace vm
} // namespace hermes
