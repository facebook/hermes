/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "gc"

#include "hermes/VM/CardBoundaryTable.h"
#include "hermes/VM/AlignedHeapSegment.h"

#include "hermes/Support/OSCompat.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>

namespace hermes {
namespace vm {

void CardBoundaryTable::updateBoundaries(
    CardBoundaryTable::Boundary *boundary,
    const char *start,
    const char *end) {
  assert(boundary != nullptr && "Need a boundary cursor");
  assert(
      FixedSizeHeapSegment::containedInSame(start, end - 1) &&
      "The boundary range must be covered by the same segment");
  assert(
      storageStart(start) <= start && end <= storageEnd(start) &&
      "Precondition: [start, end) must be covered by this table.");
  assert(
      boundary->index() == addressToIndex(boundary->address()) &&
      "Precondition: boundary's index must correspond to its address in this table.");
  // We must always have just crossed the boundary of the next card:
  assert(
      start <= boundary->address() && boundary->address() < end &&
      "Precondition: must have crossed boundary.");
  // The object may be large, and may cross multiple cards, but first
  // handle the first card.
  boundaries_[boundary->index()] =
      (boundary->address() - start) >> LogHeapAlign;
  boundary->bump();

  // Now we must fill in the remainder of the card boundaries crossed by the
  // allocation.  We use a logarithmic scheme, so we fill in one card
  // with -1, 2 with -2, etc., where each negative value k indicates
  // that we should go backwards by 2^(-k - 1) cards, and consult the
  // table there.
  int8_t currentExp = 0;
  unsigned currentIndexDelta = 1;
  unsigned numWithCurrentExp = 0;
  while (boundary->address() < end) {
    boundaries_[boundary->index()] = encodeExp(currentExp);
    numWithCurrentExp++;
    if (numWithCurrentExp == currentIndexDelta) {
      numWithCurrentExp = 0;
      currentExp++;
      currentIndexDelta *= 2;
      // Note that 7 bits handles object sizes up to 2^128, so we
      // don't have to worry about overflow of the int8_t.
    }
    boundary->bump();
  }
}

GCCell *CardBoundaryTable::firstObjForCard(
    const char *lowLim,
    const char *hiLim,
    unsigned index) const {
  int8_t val = boundaries_[index];

  // If val is negative, it means skip backwards some number of cards.
  // In general, for an object crossing 2^N cards, a query for one of
  // those cards will examine at most N entries in the table.
  while (val < 0) {
    index -= 1 << decodeExp(val);
    val = boundaries_[index];
  }

  char *boundary = const_cast<char *>(indexToAddress(lowLim, index));
  char *resPtr = boundary - (val << LogHeapAlign);
  return reinterpret_cast<GCCell *>(resPtr);
}

#ifdef HERMES_SLOW_DEBUG
void CardBoundaryTable::verifyBoundaries(char *start, char *level) const {
  // Start should be card-aligned.
  assert(isCardAligned(start));
  auto *lowLim = storageStart(start);
  auto *hiLim = storageEnd(start);
  size_t endIndex = (hiLim - lowLim) >> kLogCardSize;
  for (unsigned index = addressToIndex(start); index < endIndex; index++) {
    const char *boundary = indexToAddress(lowLim, index);
    if (level <= boundary) {
      break;
    }
    GCCell *cell = firstObjForCard(lowLim, hiLim, index);
    // Should be a valid cell.
    assert(
        cell->isValid() &&
        "Card object boundary is broken: firstObjForCard yields invalid cell.");
    char *cellPtr = reinterpret_cast<char *>(cell);
    // And it should extend across the card boundary.
    assert(
        cellPtr <= boundary &&
        boundary < (cellPtr + cell->getAllocatedSize()) &&
        "Card object boundary is broken: first obj doesn't extend into card");
  }
}
#endif // HERMES_SLOW_DEBUG

} // namespace vm
} // namespace hermes
#undef DEBUG_TYPE
