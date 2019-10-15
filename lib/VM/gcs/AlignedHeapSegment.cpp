/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/AlignedHeapSegment.h"

#include "hermes/Support/OSCompat.h"
#include "hermes/VM/Casting.h"
#include "hermes/VM/CheckHeapWellFormedAcceptor.h"
#include "hermes/VM/CompactionResult-inline.h"
#include "hermes/VM/CompleteMarkState-inline.h"
#include "hermes/VM/CompleteMarkState.h"
#include "hermes/VM/DeadRegion.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/GCBase.h"
#include "hermes/VM/GCCell-inline.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/GCGeneration.h"
#include "hermes/VM/HiddenClass.h"

#include "llvm/Support/MathExtras.h"

#include <algorithm>
#include <cassert>
#include <cstring>

using namespace hermes;

namespace hermes {
namespace vm {

AlignedHeapSegment::AlignedHeapSegment(
    AlignedStorage &&storage,
    GCGeneration *owner)
    : storage_(std::move(storage)), generation_(owner) {
  // Storage end must be page-aligned so that markUnused below stays in segment.
  assert(
      reinterpret_cast<uintptr_t>(hiLim()) % oscompat::page_size() == 0 &&
      "storage end must be page-aligned");
  if (*this) {
    new (contents()) Contents();
  }
}

AlignedHeapSegment::~AlignedHeapSegment() {
  if (lowLim() == nullptr) {
    return;
  }

  contents()->~Contents();
  __asan_unpoison_memory_region(start(), end() - start());
}

template <AdviseUnused MU>
void AlignedHeapSegment::setLevel(char *lvl) {
  assert(dbgContainsLevel(lvl));
  if (lvl < level_) {
#ifndef NDEBUG
    clear(lvl, level_);
#else
    if (MU == AdviseUnused::Yes) {
      const size_t PS = oscompat::page_size();
      auto nextPageAfter = reinterpret_cast<char *>(
          llvm::alignTo(reinterpret_cast<uintptr_t>(lvl), PS));
      auto nextPageBefore = reinterpret_cast<char *>(
          llvm::alignTo(reinterpret_cast<uintptr_t>(level_), PS));

      storage_.markUnused(nextPageAfter, nextPageBefore);
    }
#endif
  }
  level_ = lvl;
}

/// Explicit template instantiations for setLevel
template void AlignedHeapSegment::setLevel<AdviseUnused::Yes>(char *lvl);
template void AlignedHeapSegment::setLevel<AdviseUnused::No>(char *lvl);

template <AdviseUnused MU>
void AlignedHeapSegment::resetLevel() {
  setLevel<MU>(start());
}

/// Explicit template instantiations for resetLevel
template void AlignedHeapSegment::resetLevel<AdviseUnused::Yes>();
template void AlignedHeapSegment::resetLevel<AdviseUnused::No>();

void AlignedHeapSegment::setEffectiveEnd(char *effectiveEnd) {
  assert(
      start() <= effectiveEnd && effectiveEnd <= end() &&
      "Must be valid end for segment.");
  assert(level() <= effectiveEnd && "Must not set effective end below level");
  effectiveEnd_ = effectiveEnd;
}

void AlignedHeapSegment::clearExternalMemoryCharge() {
  setEffectiveEnd(end());
}

void AlignedHeapSegment::growTo(size_t desired) {
  assert(desired <= maxSize() && "Cannot request more than the max size");

  if (size() >= desired) {
    return;
  }

  char *newEnd = start() + desired;
#ifndef NDEBUG
  clear(end_, newEnd);
#endif

  end_ = newEnd;
}

void AlignedHeapSegment::shrinkTo(size_t desired) {
  assert(desired > 0 && "precondition");
  assert(desired <= maxSize() && "Cannot request more than the max size");
  assert(desired >= used() && "Cannot shrink to less than used data.");

  if (size() <= desired) {
    return;
  }

  end_ = start() + desired;
}

bool AlignedHeapSegment::growToFit(size_t amount) {
  // Insufficient space
  if (static_cast<size_t>(hiLim() - level_) < amount) {
    return false;
  }

  growTo(llvm::alignTo(used() + amount, oscompat::page_size()));
  return true;
}

void AlignedHeapSegment::creditExternalMemory(uint32_t size) {
  // Decrease effectiveEnd_ by size, but not below level_.  Be careful of
  // overflow. The cast to size_t below is justified by this invariant:
  assert(effectiveEnd_ >= level_);
  if (static_cast<size_t>(effectiveEnd_ - level_) >= size) {
    effectiveEnd_ -= size;
  } else {
    effectiveEnd_ = level_;
  }
}

void AlignedHeapSegment::debitExternalMemory(uint32_t size) {
  // Increase effectiveEnd_ by size, but not beyond end_.  Be careful of
  // overflow. The cast to size_t below is justified by this invariant:
  assert(effectiveEnd_ <= end_);
  if (static_cast<size_t>(end_ - effectiveEnd_) >= size) {
    effectiveEnd_ += size;
  } else {
    effectiveEnd_ = end_;
  }
}

void AlignedHeapSegment::completeMarking(GC *gc, CompleteMarkState *markState) {
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

void AlignedHeapSegment::sweepAndInstallForwardingPointers(
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
      // TODO(T43077289): if we rehabilitate ArrayStorage trimming, reenable
      // this code.
#if 0
      auto trimmedSize = cell->getVT()->getTrimmedSize(cell, cellSize);

      auto res = allocator.alloc(trimmedSize);
#else
      auto res = allocator.alloc(cellSize);
#endif
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

void AlignedHeapSegment::deleteDeadObjectIDs(GC *gc) {
  GCBase::IDTracker &tracker = gc->getIDTracker();
  if (tracker.isTrackingIDs()) {
    MarkBitArrayNC &markBits = markBitArray();
    // Separate out the delete tracking into a different loop in order to keep
    // the normal case fast.
    forAllObjs([&markBits, &tracker](const GCCell *cell) {
      if (!markBits.at(markBits.addressToIndex(cell))) {
        tracker.untrackObject(cell);
      }
    });
  }
}

void AlignedHeapSegment::updateObjectIDs(
    GC *gc,
    SweepResult::VTablesRemaining &vTables) {
  GCBase::IDTracker &tracker = gc->getIDTracker();
  if (!tracker.isTrackingIDs()) {
    // If ID tracking isn't on, there's nothing to do here.
    return;
  }

  SweepResult::VTablesRemaining vTablesCopy{vTables};
  MarkBitArrayNC &markBits = markBitArray();
  char *ptr = start();
  size_t ind = markBits.addressToIndex(ptr);
  while (ptr < level()) {
    if (markBits.at(ind)) {
      auto *cell = reinterpret_cast<GCCell *>(ptr);
      tracker.moveObject(cell, cell->getForwardingPointer());
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

void AlignedHeapSegment::updateReferences(
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

void AlignedHeapSegment::compact(SweepResult::VTablesRemaining &vTables) {
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
      // TODO(T43077289): if we rehabilitate ArrayStorage trimming, reenable
      // this code.
#if 0
      const bool canBeCompacted = cell->getVT()->canBeTrimmed();
      const auto trimmedSize = cell->getVT()->getTrimmedSize(cell, cellSize);
      if (newAddr != ptr) {
        std::memmove(newAddr, ptr, trimmedSize);
      }
      if (canBeCompacted) {
        // Set the new cell size.
        auto *newCell = reinterpret_cast<VariableSizeRuntimeCell *>(newAddr);
        newCell->setSizeDuringGCCompaction(trimmedSize);
        newCell->getVT()->trim(newCell);
      }
#else
      if (newAddr != ptr) {
        std::memmove(newAddr, ptr, cellSize);
      }
#endif

      ptr += cellSize;
      ind += (cellSize >> LogHeapAlign);
    } else {
      auto *deadRegion = reinterpret_cast<DeadRegion *>(ptr);
      ptr += deadRegion->size();
      ind += (deadRegion->size() >> LogHeapAlign);
    }
  }
}

void AlignedHeapSegment::forObjsInRange(
    const std::function<void(GCCell *)> &callback,
    char *low,
    char *high) {
  assert(low >= start());
  assert(high <= effectiveEnd());
  char *ptr = low;
  while (ptr < high) {
    GCCell *cell = reinterpret_cast<GCCell *>(ptr);
    callback(cell);
    ptr += cell->getAllocatedSize();
  }
}

void AlignedHeapSegment::forAllObjs(
    const std::function<void(GCCell *)> &callback) {
  forObjsInRange(callback, start(), level());
}

void AlignedHeapSegment::addExtentToString(char **buf, int *sz) {
  int n = snprintf(*buf, *sz, "{lo: \"%p\", hi: \"%p\"}", lowLim(), hiLim());
  *buf += n;
  *sz -= n;
}

void AlignedHeapSegment::recreateCardTableBoundaries() {
  const char *ptr = start();
  const char *const lim = level();
  CardTable::Boundary boundary = cardTable().nextBoundary(ptr);

  assert(
      cardTable().indexToAddress(cardTable().addressToIndex(ptr)) == ptr &&
      "ptr must be card aligned.");

  while (ptr < lim) {
    const GCCell *cell = reinterpret_cast<const GCCell *>(ptr);
    const char *nextPtr = ptr + cell->getAllocatedSize();
    if (boundary.address() < nextPtr) {
      cardTable().updateBoundaries(&boundary, ptr, nextPtr);
    }
    ptr = nextPtr;
  }
}

#ifndef NDEBUG
bool AlignedHeapSegment::dbgContainsLevel(const void *lvl) const {
  return contains(lvl) || lvl == hiLim();
}

bool AlignedHeapSegment::validPointer(const void *p) const {
  return start() <= p && p < level() &&
      static_cast<const GCCell *>(p)->isValid();
}

void AlignedHeapSegment::clear() {
  clear(start(), end());
}

/* static */ void AlignedHeapSegment::clear(char *start, char *end) {
#if LLVM_ADDRESS_SANITIZER_BUILD
  __asan_poison_memory_region(start, end - start);
#else
  std::memset(start, kInvalidHeapValue, end - start);
#endif
}

/* static */ void AlignedHeapSegment::checkUnwritten(char *start, char *end) {
#if !LLVM_ADDRESS_SANITIZER_BUILD && defined(HERMES_SLOW_DEBUG)
  // Check that the space was not written into.
  std::for_each(
      start, end, [](char value) { assert(value == kInvalidHeapValue); });
#endif
}
#endif // !NDEBUG

#ifdef HERMES_SLOW_DEBUG
void AlignedHeapSegment::checkWellFormed(const GC *gc, uint64_t *externalMemory)
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
