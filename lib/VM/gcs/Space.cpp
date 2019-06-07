/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#define DEBUG_TYPE "gc"
#include "hermes/VM/Space.h"

#include "hermes/Support/OSCompat.h"
#include "hermes/VM/Casting.h"
#include "hermes/VM/CheckHeapWellFormedAcceptor.h"
#include "hermes/VM/CompleteMarkState-inline.h"
#include "hermes/VM/DeadRegion.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/GCCell-inline.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HeapAlign.h"
#include "hermes/VM/HermesValue-inline.h"
#include "hermes/VM/HiddenClass.h"
#include "hermes/VM/MarkBitArray.h"
#include "hermes/VM/SlotAcceptorDefault.h"
#include "hermes/VM/Space-inline.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/SweepResult.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/MathExtras.h"

#include <cassert>

using namespace hermes;

namespace hermes {
namespace vm {

namespace {

struct MoveHeapAcceptor final : public SlotAcceptorDefault {
  const std::ptrdiff_t delta;
  MoveHeapAcceptor(GC &gc, std::ptrdiff_t delta)
      : SlotAcceptorDefault(gc), delta(delta) {}

  using SlotAcceptorDefault::accept;

  void accept(void *&ptr) override {
    if (gc.contains(ptr)) {
      char *&ptrVal = reinterpret_cast<char *&>(ptr);
      ptrVal += delta;
    }
  }

  void accept(HermesValue &hv) override {
    if (hv.isPointer()) {
      auto ptr = reinterpret_cast<char *>(hv.getPointer());
      if (gc.contains(ptr)) {
        hv.setInGC(hv.updatePointer(ptr + delta), &gc);
      }
    }
  }
};

} // namespace

GCSpace::GCSpace(char *lowLim, char *start, char *end, char *hiLim)
    : lowLim_(lowLim),
      start_(start),
      effectiveEnd_(end),
      end_(end),
      hiLim_(hiLim) {
#ifndef NDEBUG
  size_t pg_sz = oscompat::page_size();
#endif
  assert(
      (hiLim - lowLim) % pg_sz == 0 &&
      "Precondition: hiLim - lowLim must be page-aligned");
  assert(
      (end - start) % pg_sz == 0 &&
      "Precondition: end - start must be page-aligned");
  assert(
      lowLim <= start && start <= end && end <= hiLim &&
      "Precondition: lowLim <= start <= end <= hiLim");
#ifndef NDEBUG
  // Fill the memory with dead values.
  clear();
#endif
}

bool GCSpace::growHigh(size_t amount) {
  assert(
      reinterpret_cast<uintptr_t>(end_) % oscompat::page_size() == 0 &&
      "The end of the space must be page aligned");
  amount = llvm::alignTo(amount, oscompat::page_size());
  if (static_cast<size_t>(hiLim_ - end_) >= amount) {
    end_ += amount;
    effectiveEnd_ += amount;
#ifndef NDEBUG
    // After growing, fill with dead values.
    clear(end_ - amount, end_);
#endif
    return true;
  } else {
    return false;
  }
}

void GCSpace::dumpCellInfo(llvm::raw_ostream &os, const GCCell *cell) const {
  os << llvm::format_hex((uintptr_t)((const char *)cell - start_), 10);
  os << ": type";
  if (cell->getKind() != CellKind::UninitializedKind)
    os << " '" << cellKindStr(cell->getKind()) << "'";
  os << " @" << llvm::format_hex((uintptr_t)cell->getVT(), 0);

  os << ", size " << cell->getAllocatedSize() << " bytes";
}

#ifndef NDEBUG
void GCSpace::clear() {
  clear(start_, end_);
}

void GCSpace::clear(char *start, char *end) {
#if LLVM_ADDRESS_SANITIZER_BUILD
  __asan_poison_memory_region(start, end - start);
#else
  std::memset(start, kInvalidHeapValue, end - start);
#endif
}

void GCSpace::checkUnwritten(char *start, char *end) const {
#if !LLVM_ADDRESS_SANITIZER_BUILD && defined(HERMES_SLOW_DEBUG)
  // Check that the space was not written into.
  std::for_each(
      start, end, [](char value) { assert(value == kInvalidHeapValue); });
#endif
}
#endif

std::vector<const VTable *>
ContigAllocGCSpace::sweepAndInstallForwardingPointers(
    GC *gc,
    MarkBitArray *markBits,
    std::vector<CompactionRegion *> *compactionRegions) {
  std::vector<const VTable *> res;
  // Return early if nothing was allocated.
  if (used() == 0) {
    assert(cellsWithFinalizers_.empty());
    return res;
  }

  // Finalize unreachable cells that have finalizers.
  finalizeUnreachableObjects(gc, markBits);

  char *ptr = start_;
  size_t ind = markBits->addressToIndex(start_);

  assert(level_ > start_ && "We return above if level_ == start_");
  size_t indexLimit = markBits->addressToIndex(level_ - 1) + 1;
  // We will set adjacentPtr to point just after each marked object.  Thus,
  // if there is a gap in the sequence of marked objects, it will indicate the
  // beginning of a dead region.
  char *adjacentPtr = start_;
  ind = markBits->findNextMarkedBit(ind);
  for (CompactionRegion *&compactionRegion : *compactionRegions) {
    // Each iteration of this loop fills the current compactionRegion
    // as much as possible.
    if (compactionRegion == nullptr) {
      // The region was exhausted in a previous call.
      continue;
    }
    char *nextCompactedObj = compactionRegion->start;
    char *compactionRegionEnd = compactionRegion->end;

    while (ind < indexLimit) {
      ptr = markBits->indexToAddress(ind);
      GCCell *cell = reinterpret_cast<GCCell *>(ptr);
      uint32_t cellSize = cell->getAllocatedSize();

      if (nextCompactedObj + cellSize > compactionRegionEnd) {
        // The current CompactionRegion is exhausted; must move on to the next.
        break;
      }

#ifndef NDEBUG
      numReachableObjects_++;
      if (auto *hiddenClass = dyn_vmcast<HiddenClass>(cell)) {
        ++numHiddenClasses_;
        numLeafHiddenClasses_ += hiddenClass->isKnownLeaf();
      }
      gc->trackReachable(cell->getKind(), cellSize);
#endif

      if (ptr != adjacentPtr) {
        new (adjacentPtr) DeadRegion(ptr - adjacentPtr);
      }
      res.push_back(cell->getVT());
      adjacentPtr = ptr + cellSize;
      ind = markBits->findNextMarkedBit(ind + 1);
#ifndef NDEBUG
      compactionRegion->numAllocated++;
#endif
      cell->setForwardingPointer(reinterpret_cast<GCCell *>(nextCompactedObj));
      nextCompactedObj += cellSize;
    }
    compactionRegion->start = nextCompactedObj;
    // If we didn't complete the iteration over the indices, then we must
    // have exhausted the current CompactionRegion.  Set the current vector
    // entry to null to indicate this exhaustion, so we won't attempt to
    // allocate in the CompactionRegion in future calls.
    if (ind < indexLimit) {
      compactionRegion = nullptr;
    }
  }
  assert(ind >= indexLimit && "We didn't have enough space to compact into");

  if (adjacentPtr < level_) {
    new (adjacentPtr) DeadRegion(level_ - adjacentPtr);
  }

  return res;
}

void ContigAllocGCSpace::compact(
    const MarkBitArray &markBits,
    const SweepResult &sweepResult) {
  // If we're using ASAN, we've poisoned the unallocated portion of the space;
  // unpoison that now, since we may copy into it.
  __asan_unpoison_memory_region(level(), end() - level());
  char *ptr = start_;
  size_t ind = markBits.addressToIndex(start_);
  size_t objInd = 0;
  while (ptr < level_) {
    if (markBits.at(ind)) {
      GCCell *cell = (GCCell *)ptr;
      // Read the new address from the forwarding pointer.
      char *newAddr = reinterpret_cast<char *>(cell->getForwardingPointer());
      // Put back the vtable.
      cell->setForwardingPointer(reinterpret_cast<const GCCell *>(
          sweepResult.displacedVtablePtrs.at(objInd)));
      assert(
          cell->isValid() &&
          "Cell was invalid after placing the vtable back in");
      objInd++;
      // Must read this now, since the memmove below might overwrite it.
      size_t cellSize = cell->getAllocatedSize();
      if (newAddr != ptr) {
        std::memmove(newAddr, ptr, cellSize);
      }
      // TODO: if we decide that making per-moved-object callbacks is
      // too expensive, we could acknowledge the specific purpose of
      // the callback here, and keep track of a next card boundary
      // here, and only do the callback when the boundary is crossed.
      if (compactMoveCallback_) {
        compactMoveCallback_(newAddr, newAddr + cellSize);
      }
      ptr += cellSize;
      ind += (cellSize >> LogHeapAlign);
    } else {
      auto *deadRegion = reinterpret_cast<DeadRegion *>(ptr);
      ptr += deadRegion->size();
      if (ptr < level_) {
        ind = markBits.addressToIndex(ptr);
      }
    }
  }
  // Note that his level is the *final* level for this space; compaction of
  // other spaces may still still write into this one.
  level_ = sweepResult.level;
#ifndef NDEBUG
  // Fill with dead values after compaction.
  clear(level_, end_);
#endif
}

void ContigAllocGCSpace::finalizeUnreachableObjects(
    GC *gc,
    MarkBitArray *markBits) {
  for (gcheapsize_t i = 0; i < cellsWithFinalizers_.size(); i++) {
    GCCell *cell = cellsWithFinalizers_.at(i);
    if (!markBits->at(markBits->addressToIndex(cell))) {
      cell->getVT()->finalize(cell, gc);
      numFinalizedObjects_++;
      continue;
    }
    cellsWithFinalizers_[i - numFinalizedObjects_] = cell;
  }
  cellsWithFinalizers_.resize(
      cellsWithFinalizers_.size() - numFinalizedObjects_);
}

void ContigAllocGCSpace::updateFinalizableCellListReferences() {
  for (gcheapsize_t i = 0; i < cellsWithFinalizers_.size(); i++) {
    cellsWithFinalizers_[i] = cellsWithFinalizers_[i]->getForwardingPointer();
  }
}

void ContigAllocGCSpace::dump(llvm::raw_ostream &os) const {
  for (char *ptr = start_; ptr < level_;) {
    GCCell *cell = (GCCell *)ptr;
    ptr += cell->getAllocatedSize();
    dumpCellInfo(os, cell);
    os << "\n";
  }
}

size_t ContigAllocGCSpace::countMallocSize() {
  size_t sum = 0;
  for (char *ptr = start_; ptr < level_;) {
    GCCell *cell = reinterpret_cast<GCCell *>(ptr);
    sum += cell->getVT()->getMallocSize(cell);
    ptr += cell->getAllocatedSize();
  }
  return sum;
}

#ifdef HERMES_SLOW_DEBUG
void ContigAllocGCSpace::checkWellFormed(const GC *gc, uint64_t *externalMemory)
    const {
  CheckHeapWellFormedAcceptor acceptor(*const_cast<GC *>(gc));
  char *ptr = start_;
  uint64_t extSize = 0;
  while (ptr < level_) {
    GCCell *cell = (GCCell *)ptr;
    assert(cell->isValid() && "cell is invalid");
    // We assume that the GC's mark behavior is Verify.  Thus, it's OK to
    // cast away the const on gc.
    GCBase::markCell(cell, const_cast<GC *>(gc), acceptor);
    ptr += cell->getAllocatedSize();
    extSize += cell->externalMemorySize();
  }

  checkUnwritten(ptr, end_);

  if (externalMemory) {
    *externalMemory = extSize;
  }
}
#endif

void ContigAllocGCSpace::moveHeap(GC *gc, ptrdiff_t moveHeapDelta) {
  MoveHeapAcceptor acceptor(*gc, moveHeapDelta);
  char *ptr = start_;
  while (ptr < level_) {
    auto cell = reinterpret_cast<GCCell *>(ptr);
    GCBase::markCell(cell, gc, acceptor);
    ptr += cell->getAllocatedSize();
  }

  for (GCCell *&cell : cellsWithFinalizers_) {
    auto &ptr = reinterpret_cast<char *&>(cell);
    ptr += moveHeapDelta;
  }

  lowLim_ += moveHeapDelta;
  start_ += moveHeapDelta;
  level_ += moveHeapDelta;
  levelAtEndOfLastGC_ += moveHeapDelta;
  effectiveEnd_ += moveHeapDelta;
  end_ += moveHeapDelta;
  hiLim_ += moveHeapDelta;
}

void ContigAllocGCSpace::creditExternalMemory(uint32_t size) {
  // Decrease effectiveEnd_ by size, but not below level_.  Be careful of
  // overflow. The cast to size_t below is justified by this invariant:
  assert(effectiveEnd_ >= level_);
  if (static_cast<size_t>(effectiveEnd_ - level_) >= size) {
    effectiveEnd_ -= size;
  } else {
    effectiveEnd_ = level_;
  }
}

void ContigAllocGCSpace::debitExternalMemory(uint32_t size) {
  // Increase effectiveEnd_ by size, but not beyond end_.  Be careful of
  // overflow. The cast to size_t below is justified by this invariant:
  assert(effectiveEnd_ <= end_);
  if (static_cast<size_t>(end_ - effectiveEnd_) >= size) {
    effectiveEnd_ += size;
  } else {
    effectiveEnd_ = end_;
  }
}

bool ContigAllocGCSpace::completeMarking(
    GC *gc,
    MarkBitArray *markBits,
    CompleteMarkState *markState) {
  // Return early if nothing was allocated.
  if (used() == 0) {
    return false;
  }

  CompleteMarkState::FullMSCMarkTransitiveAcceptor acceptor(
      *gc, markBits, markState);
  // We have a loop here: in case we have mark stack overflow, we re-scan the
  // mark bits.
  bool markStackOverflowOccurred = false;
  do {
    markState->markStackOverflow_ = false;
    char *ptr = start_;
    size_t ind = markBits->addressToIndex(start_);
    assert(level_ > start_ && "We return above if level_ == start_");
    size_t indexLimit = markBits->addressToIndex(level_ - 1) + 1;
    ind = markBits->findNextMarkedBit(ind);
    while (ind < indexLimit) {
      ptr = markBits->indexToAddress(ind);
      GCCell *cell = reinterpret_cast<GCCell *>(ptr);

      markState->currentParPointer = cell;
      if (cell->isVariableSize()) {
        markState->varSizeMarkStack_.push_back(cell);
      } else {
        markState->markStack_.push_back(cell);
      }
      markState->drainMarkStack(gc, markBits, acceptor);

      if (markState->markStackOverflow_) {
        markState->markStack_.clear();
        markState->varSizeMarkStack_.clear();
        break;
      }
      ind = markBits->findNextMarkedBit(ind + 1);
    }
    assert(markState->markStack_.empty());
    assert(markState->varSizeMarkStack_.empty());
    if (markState->markStackOverflow_) {
      markStackOverflowOccurred = true;
    }
  } while (markState->markStackOverflow_);
  return markStackOverflowOccurred;
}

void ContigAllocGCSpace::updateReferences(
    GC *gc,
    const MarkBitArray &markBits,
    const std::vector<const VTable *> &displacedVtablePtrs) {
  FullMSCUpdateAcceptor acceptor(*gc);
  // Update reachable cells with finalizers to their after-compaction location.
  updateFinalizableCellListReferences();

  char *ptr = start_;
  size_t ind = markBits.addressToIndex(start_);
  size_t objInd = 0;
  while (ptr < level_) {
    if (markBits.at(ind)) {
      // Get the VTable.
      const VTable *vtp = displacedVtablePtrs.at(objInd);
      objInd++;
      // Scan the pointer fields, updating via forwarding pointers.
      GCCell *cell = (GCCell *)ptr;
      GCBase::markCell(cell, vtp, gc, acceptor);
      uint32_t cellSize = cell->getAllocatedSize(vtp);
      ptr += cellSize;
      ind += (cellSize >> LogHeapAlign);
    } else {
      auto *deadRegion = reinterpret_cast<DeadRegion *>(ptr);
      ptr += deadRegion->size();
      if (ptr < level_) {
        ind = markBits.addressToIndex(ptr);
      }
    }
  }
}

#ifndef NDEBUG
bool ContigAllocGCSpace::validPointer(const void *p) const {
  return p >= start_ && p < level_ && static_cast<const GCCell *>(p)->isValid();
}

#endif

std::unique_ptr<SlotAcceptor> getMoveHeapAcceptor(
    GC &gc,
    std::ptrdiff_t delta) {
  return std::unique_ptr<SlotAcceptor>(new MoveHeapAcceptor(gc, delta));
}

std::unique_ptr<FullMSCUpdateAcceptor> getFullMSCUpdateAcceptor(GC &gc) {
  return std::unique_ptr<FullMSCUpdateAcceptor>(new FullMSCUpdateAcceptor(gc));
}

void ContigAllocGCSpace::recordLevelAtEndOfLastGC() {
  levelAtEndOfLastGC_ = level_;
}

#ifndef NDEBUG
void ContigAllocGCSpace::forObjsAllocatedSinceGC(
    std::function<void(GCCell *)> callback) {
  forObjsInRange(callback, levelAtEndOfLastGC_, level_);
}

void ContigAllocGCSpace::forObjsInRange(
    std::function<void(GCCell *)> callback,
    char *low,
    char *high) {
  assert(low >= start_);
  assert(high <= effectiveEnd_);
  char *ptr = low;
  while (ptr < high) {
    GCCell *cell = reinterpret_cast<GCCell *>(ptr);
    callback(cell);
    ptr += cell->getAllocatedSize();
  }
}
#endif

} // namespace vm
} // namespace hermes
