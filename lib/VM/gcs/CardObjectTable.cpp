/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#define DEBUG_TYPE "gc"

#include "hermes/VM/CardObjectTable.h"

namespace hermes {
namespace vm {

CardObjectTable::CardObjectTable(
    char *parentLowLim,
    char *parentStart,
    char *parentEnd,
    char *parentHiLim,
    CardTable *cardTable)
    : DependentMemoryRegion(
          "hermes-cardobjecttable",
          parentLowLim,
          parentStart,
          parentEnd,
          parentHiLim,
          CardTable::kLogCardSize),
      nextCardBoundary_(parentLowLim),
      cardTable_(cardTable) {}

void CardObjectTable::updateEntries(char *start, char *end) {
  // We must always have just crossed the boundary of the next card:
  assert(
      start <= nextCardBoundary_ && nextCardBoundary_ < end &&
      "Precondition: must have crossed nextCardBoundary_.");
  // The object may be large, and may cross multiple cards, but first
  // handle the first card.
  entries()[nextIndex_] = (nextCardBoundary_ - start) >> LogHeapAlign;
  nextIndex_++;
  nextCardBoundary_ += CardTable::kCardSize;
  // Now we must fill in the remainder of the card boundaries crossed by the
  // allocation.  We use a logarithmic scheme, so we fill in one card
  // with -1, 2 with -2, etc., where each negative value k indicates
  // that we should go backwards by 2^(-k - 1) cards, and consult the
  // table there.
  int8_t currentExp = 0;
  int64_t currentIndexDelta = 1;
  int8_t numWithCurrentExp = 0;
  while (end > nextCardBoundary_) {
    entries()[nextIndex_] = encodeExp(currentExp);
    numWithCurrentExp++;
    if (numWithCurrentExp == currentIndexDelta) {
      numWithCurrentExp = 0;
      currentExp++;
      currentIndexDelta *= 2;
      // Note that 7 bits handles object sizes up to 2^128, so we
      // don't have to worry about overflow of the int8_t.
    }
    nextIndex_++;
    nextCardBoundary_ += CardTable::kCardSize;
  }
}

GCCell *CardObjectTable::firstObjForCard(unsigned index) const {
  assert(index < used());
  char *cardBoundary = cardTable_->indexToAddress(index);
  int8_t val = entries()[index];
  // If val is negative, it means skip backwards some number of cards.
  // In general, for an object crossing 2^N cards, a query for one of
  // those cards will examine at most N entries in the table.
  while (val < 0) {
    index = index - (1 << decodeExp(val));
    cardBoundary = cardTable_->indexToAddress(index);
    val = entries()[index];
  }
  char *resPtr = cardBoundary - (val << LogHeapAlign);
  return reinterpret_cast<GCCell *>(resPtr);
}

void CardObjectTable::reset() {
  nextCardBoundary_ = const_cast<char *>(parentLowLim_);
  nextIndex_ = 0;
}

#ifdef HERMES_SLOW_DEBUG
void CardObjectTable::verify(char *level) const {
  size_t tableSize = used();
  for (unsigned index = 0; index < tableSize; index++) {
    const char *cardBoundary = cardTable_->indexToAddress(index);
    if (cardBoundary >= level) {
      break;
    }
    GCCell *cell = firstObjForCard(index);
    // Should be a valid cell.
    assert(
        cell->isValid() &&
        "Card object table is broken: firstObjForCard yields invalid cell.");
    char *cellPtr = reinterpret_cast<char *>(cell);
    // And it should extend across the card boundary.
    assert(
        cellPtr <= cardBoundary &&
        (cellPtr + cell->getAllocatedSize()) > cardBoundary &&
        "Card object table is broken: first obj doesn't extend into card");
  }
}
#endif

} // namespace vm
} // namespace hermes
