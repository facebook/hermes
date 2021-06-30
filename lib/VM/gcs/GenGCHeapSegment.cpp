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
    : AlignedHeapSegment(std::move(storage)), generation_(owner) {}

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

void GenGCHeapSegment::completeMarking(
    GenGC *gc,
    CompleteMarkState *markState) {
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

void GenGCHeapSegment::deleteDeadObjectIDs(GenGC *gc) {
  if (!gc->isTrackingIDs()) {
    return;
  }
  MarkBitArrayNC &markBits = markBitArray();
  // Separate out the delete tracking into a different loop in order to keep
  // the normal case fast.
  forAllObjs([&markBits, gc](const GCCell *cell) {
    if (!markBits.at(markBits.addressToIndex(cell))) {
      gc->untrackObject(cell, cell->getAllocatedSize());
    }
  });
}

void GenGCHeapSegment::updateReferences(
    GenGC *gc,
    FullMSCUpdateAcceptor *acceptor,
    SweepResult::KindAndSizesRemaining &kindAndSizes) {
  MarkBitArrayNC &markBits = markBitArray();
  char *ptr = start();
  size_t ind = markBits.addressToIndex(ptr);
  while (ptr < level()) {
    if (markBits.at(ind)) {
      GCCell *cell = reinterpret_cast<GCCell *>(ptr);
      // Get the KindAndSize.
      assert(kindAndSizes.hasNext() && "Need a displaced KindAndSize");
      const auto kindAndSize = kindAndSizes.next();
      // Scan the pointer fields, updating via forwarding pointers.
      gc->markCell(cell, kindAndSize.getKind(), *acceptor);
      uint32_t cellSize = kindAndSize.getSize();
      ptr += cellSize;
      ind += (cellSize >> LogHeapAlign);
    } else {
      auto *deadRegion = reinterpret_cast<DeadRegion *>(ptr);
      ptr += deadRegion->size();
      ind += (deadRegion->size() >> LogHeapAlign);
    }
  }
}

void GenGCHeapSegment::compact(
    SweepResult::KindAndSizesRemaining &kindAndSizes,
    PointerBase *base) {
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
      char *newAddr = reinterpret_cast<char *>(
          cell->getForwardingPointer().getNonNull(base));
      // Put back the vtable.
      assert(kindAndSizes.hasNext() && "Need a displaced vtable pointer");
      cell->setKindAndSize(kindAndSizes.next());
      assert(
          cell->isValid() &&
          "Cell was invalid after placing the vtable back in");
      // Must read this now, since the memmove below might overwrite it.
      auto cellSize = cell->getAllocatedSize();
      const auto trimmedSize = cell->getVT()->getTrimmedSize(cell, cellSize);
      if (newAddr != ptr) {
        std::memmove(newAddr, ptr, trimmedSize);
      }
      if (trimmedSize != cellSize) {
        // Set the new cell size.
        auto *newCell = reinterpret_cast<VariableSizeRuntimeCell *>(newAddr);
        newCell->setSizeFromGC(trimmedSize);
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
    GenGC *gc,
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

      sweepResult->displacedKinds.push_back(cell->getKindAndSize());
      cell->setForwardingPointer(CompressedPointer(
          gc->getPointerBase(), static_cast<GCCell *>(res.ptr)));
      if (gc->isTrackingIDs()) {
        gc->moveObject(
            cell,
            cellSize,
            cell->getForwardingPointer().getNonNull(gc->getPointerBase()),
            trimmedSize);
      }
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

void GenGCHeapSegment::addExtentToString(char **buf, int *sz) {
  // Emit backslashes so that if the string is embedded inside JSON as a string,
  // it is still parseable.
  int n = snprintf(
      *buf, *sz, R"({\"lo\": \"%p\", \"hi\": \"%p\"})", lowLim(), hiLim());
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
void GenGCHeapSegment::checkWellFormed(
    const GenGC *gc,
    uint64_t *externalMemory) const {
  CheckHeapWellFormedAcceptor acceptor(*const_cast<GenGC *>(gc));
  char *ptr = start();
  uint64_t extSize = 0;
  while (ptr < level()) {
    GCCell *cell = (GCCell *)ptr;
    assert(cell->isValid() && "cell is invalid");
    // We assume that CheckHeapWellFormedAcceptor does not mutate the GC.  Thus
    // it's OK to cast away the const on \p gc.
    const_cast<GenGC *>(gc)->markCell(cell, acceptor);
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
