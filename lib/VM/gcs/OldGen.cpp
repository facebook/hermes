/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#define DEBUG_TYPE "gc"
#include "hermes/VM/OldGen.h"

#include "hermes/Support/OSCompat.h"
#include "hermes/VM/AllocResult.h"
#include "hermes/VM/CardTable.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/SlotAcceptorDefault.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"

namespace hermes {
namespace vm {

#ifdef HERMES_SLOW_DEBUG
/// These constants allow us to selective turn off some of the
/// slow debugging checks (by modifying this code and recompiling).
/// But they're on by default.
static bool kVerifyCardObjectTable = true;
static bool kVerifyCardTable = true;
#endif

OldGen::OldGen(GenGC *gc, char *lowLim, char *hiLim, size_t size)
    : ContigAllocGCSpace(lowLim, lowLim, lowLim + size, hiLim),
      gc_(gc),
      cardTable_(lowLim, lowLim, lowLim + size, hiLim),
      cardObjectTable_(lowLim, lowLim, lowLim + size, hiLim, &cardTable_) {
  compactMoveCallback_ = [this](char *start, char *end) {
    if (end > cardObjectTable_.nextCardBoundary()) {
      cardObjectTable_.updateEntries(start, end);
    }
  };
}

void OldGen::creditExternalMemory(uint32_t size) {
  externalMemory_ += size;
  ContigAllocGCSpace::creditExternalMemory(size);
}

void OldGen::debitExternalMemory(uint32_t size) {
  assert(externalMemory_ >= size);
  externalMemory_ -= size;
  ContigAllocGCSpace::debitExternalMemory(size);
}

void OldGen::updateEffectiveEndForExternalMemory() {
  char *newEffectiveEnd = std::max(level_, end_ - externalMemory_);
  if (newEffectiveEnd > effectiveEnd_) {
#ifndef NDEBUG
    clear(effectiveEnd_, newEffectiveEnd);
#endif
    effectiveEnd_ = newEffectiveEnd;
  }
}

void OldGen::markYoungGenPointers(GCSpace *youngGen) {
  if (used() == 0) {
    // Nothing to do if the old gen is empty.
    return;
  }

#ifdef HERMES_SLOW_DEBUG
  struct VerifyCardDirtyAcceptor final : public SlotAcceptorDefault {
    OldGen &gen;
    VerifyCardDirtyAcceptor(GC &gc, OldGen &gen)
        : SlotAcceptorDefault(gc), gen(gen) {}

    using SlotAcceptorDefault::accept;

    void accept(void *&ptr) {
      char *&loc = reinterpret_cast<char *&>(ptr);
      if (loc && loc < gen.lowLim()) {
        assert(gen.cardTable().isCardForAddressDirty(&ptr));
      }
    }
    void accept(HermesValue &hv) {
      if (hv.isPointer()) {
        char *ptr = static_cast<char *>(hv.getPointer());
        if (ptr && ptr < gen.lowLim()) {
          assert(gen.cardTable().isCardForAddressDirty(&hv));
        }
      }
    }
  };

  verifyCardObjectTable();

  // This code detects old-to-young pointers that occur on clean cards.
  // Thus, it detects pointer writes without barriers, if
  //  1) they happen to create an old-to-young pointer, and
  //  2) they don't happen within a card that is dirty for other reasons,
  //     such as a pointer write to a different field with a barrier that
  //     dirties the card.
  //
  // We could correct deficiency (2) by creating a bit array with a
  // bit per pointer-aligned heap slot, and having the write barrier
  // set the bit corresponding to a slot when it writes to the slot.
  // Peter Ammons had an interesting idea for correcting defect (1):
  // at the end of collection N, make a copy of the old-gen state.  At
  // collection N+1, compare the old state to the new.  This would
  // allow us to identify all pointer fields that were modified (to
  // different values than before), not just those that created
  // old-to-young pointers.
  if (kVerifyCardTable) {
    VerifyCardDirtyAcceptor acceptor(*gc_, *this);

    char *ptr = start_;
    while (ptr < level_) {
      GCCell *cell = (GCCell *)ptr;
      GCBase::markCell(cell, gc_, acceptor);
      ptr += cell->getAllocatedSize();
    }
  }
#endif

  struct ScanDirtyCardEvacAcceptor final : public SlotAcceptorDefault {
    using SlotAcceptorDefault::accept;
    using SlotAcceptorDefault::SlotAcceptorDefault;

    // NOTE: C++ does not allow templates on local classes, so duplicate the
    // body of \c helper for ensureReferentCopied.
    void helper(GCCell **slotAddr, void *slotContents) {
      char *slotPtr = reinterpret_cast<char *>(slotAddr);
      char *ptr = static_cast<char *>(slotContents);
      char *boundary = gc.oldGen_.lowLim();
      if (slotPtr >= boundary && ptr < boundary) {
        gc.youngGen_.ensureReferentCopied(slotAddr);
      }
    }
    void helper(HermesValue *slotAddr, void *slotContents) {
      char *slotPtr = reinterpret_cast<char *>(slotAddr);
      char *ptr = static_cast<char *>(slotContents);
      char *boundary = gc.oldGen_.lowLim();
      if (slotPtr >= boundary && ptr < boundary) {
        gc.youngGen_.ensureReferentCopied(slotAddr);
      }
    }

    void accept(void *&ptr) {
      helper(reinterpret_cast<GCCell **>(&ptr), ptr);
    }

    void accept(HermesValue &hv) {
      if (hv.isPointer()) {
        helper(&hv, hv.getPointer());
      }
    }
  };

  ScanDirtyCardEvacAcceptor acceptor(*gc_);
  SlotVisitor<ScanDirtyCardEvacAcceptor> visitor(acceptor);
  originalLevel_ = level();

  size_t fromIndex = cardTable_.addressToIndex(start());
  // endIndex should be the index of the first card guaranteed to not
  // contain any allocated objects (which will be the index of end(),
  // if the last allocated object extends onto the last card).
  // Make sure we don't
  assert(level() > start() && "Because we returned above if used() was 0.");
  size_t finalIndex = cardTable_.addressToIndex(level() - 1) + 1;

  // Find a consecutive range of dirty cards.
  while (auto optCardIndex =
             cardTable_.findNextDirtyCard(fromIndex, finalIndex)) {
    const auto beginIndex = *optCardIndex;
    auto endIndex = beginIndex + 1;
    // Advance until endIndex is the first clean card after a sequence of dirty
    // cards.
    while (endIndex < finalIndex && cardTable_.isCardForIndexDirty(endIndex)) {
      ++endIndex;
    }
    assert(
        (endIndex == finalIndex || !cardTable_.isCardForIndexDirty(endIndex)) &&
        cardTable_.isCardForIndexDirty(endIndex - 1) &&
        "endIndex should either be the end of the card table, or the first "
        "non-dirty card after a sequence of dirty cards");
    assert(beginIndex < endIndex && "Indices must be apart by at least one");
    // Now the range [beginIndex, endIndex) is a contiguous sequence of dirty
    // cards.
    auto *const begin = cardTable_.indexToAddress(beginIndex);
    auto *const end = cardTable_.indexToAddress(endIndex);
    const auto *const boundary = std::min(end, originalLevel_);
    GCCell *obj = cardObjectTable_.firstObjForCard(beginIndex);
    // The first object should be marked with respect to the dirty card
    // boundaries.
    GCBase::markCellWithinRange(visitor, obj, obj->getVT(), gc_, begin, end);
    obj = obj->nextCell();
    while (reinterpret_cast<char *>(obj) < boundary) {
      auto *nextObj = obj->nextCell();
      if (reinterpret_cast<char *>(nextObj) > boundary) {
        // obj is the last object for this card.
        GCBase::markCellWithinRange(
            visitor, obj, obj->getVT(), gc_, begin, end);
      } else {
        // Interior objects (objects completely within a dirty card) don't need
        // any special handling, mark it normally.
        GCBase::markCell(visitor, obj, obj->getVT(), gc_);
      }
      obj = nextObj;
    }
    fromIndex = endIndex;
  }

  cardTable_.clear();
}

#ifdef HERMES_SLOW_DEBUG
void OldGen::verifyCardObjectTable() const {
  if (kVerifyCardObjectTable) {
    cardObjectTable_.verify(level_);
  }
}
#endif

void OldGen::addToFinalizerList(GCCell *cell) {
  cellsWithFinalizers_.push_back(cell);
}

void OldGen::prepareForCompaction() {
  cardObjectTable_.reset();
}

bool OldGen::growHigh(size_t amount) {
  bool res = ContigAllocGCSpace::growHigh(amount);
  if (res) {
    gc_->didResize();
    didResize();
  }
  return res;
}

void OldGen::didResize() {
  cardTable_.resizeParentUsedRegion(start_, effectiveEnd_);
  cardObjectTable_.resizeParentUsedRegion(start_, effectiveEnd_);
}

void OldGen::levelChangedFrom(char *levelBefore) {
  if (level() >= levelBefore)
    return;

    // Return memory to the OS in optimized builds. In un-optimized builds, this
    // would interfere with filling the dead parts of the heap with a known bit
    // pattern.
#ifdef NDEBUG
  const size_t PS = hermes::oscompat::page_size();
  auto nextPageAfter = reinterpret_cast<char *>(
      llvm::alignTo(reinterpret_cast<uintptr_t>(level()), PS));
  auto nextPageBefore = reinterpret_cast<char *>(
      llvm::alignTo(reinterpret_cast<uintptr_t>(levelBefore), PS));

  gc_->storage_.markUnused(nextPageAfter, nextPageBefore);
  cardTable_.parentDidAdviseUnused(nextPageAfter, nextPageBefore);
  cardObjectTable_.parentDidAdviseUnused(nextPageAfter, nextPageBefore);
#endif
}

void *OldGen::fullCollectThenAlloc(
    uint32_t allocSize,
    HasFinalizer hasFinalizer) {
  gc_->collect();
  {
    AllocResult res = allocRaw(allocSize, hasFinalizer);
    if (LLVM_LIKELY(res.success)) {
      return res.ptr;
    }
  }

  if (growHigh(allocSize)) {
    AllocResult res = allocRaw(allocSize, hasFinalizer);
    assert(res.success && "preceding test should guarantee success.");
    return res.ptr;
  }

  gc_->oom(make_error_code(OOMError::MaxHeapReached));
}

void OldGen::moveHeap(GC *gc, ptrdiff_t moveHeapDelta) {
  ContigAllocGCSpace::moveHeap(gc, moveHeapDelta);
  cardTable_.moveParentRegion(moveHeapDelta);
  cardObjectTable_.moveParentRegion(moveHeapDelta);
}

#ifdef HERMES_SLOW_DEBUG
void OldGen::checkWellFormed(const GC *gc) const {
  uint64_t extSize = 0;
  ContigAllocGCSpace::checkWellFormed(gc, &extSize);
  assert(extSize == externalMemory_);
}
#endif

} // namespace vm
} // namespace hermes
