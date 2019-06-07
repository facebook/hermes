/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#define DEBUG_TYPE "gc"

#include "hermes/VM/CardTable.h"

#include <string.h>
#include <algorithm>
#include <cassert>
#include <cstdint>

namespace hermes {
namespace vm {

CardTable::CardTable(
    char *parentLowLim,
    char *parentStart,
    char *parentEnd,
    char *parentHiLim)
    : DependentMemoryRegion(
          "hermes-cardtable",
          parentLowLim,
          parentStart,
          parentEnd,
          parentHiLim,
          kLogCardSize) {}

void CardTable::dirtyCardsForAddressRange(void *low, void *high) {
  dirtyRange(addressToIndex(low), addressToIndex(high));
}

OptValue<size_t> CardTable::findNextDirtyCard(size_t fromIndex, size_t endIndex)
    const {
  char *fromIndexPtr = lowLim() + fromIndex;
  void *nextDirty = memchr(
      fromIndexPtr, static_cast<char>(CardStatus::Dirty), endIndex - fromIndex);
  return (nextDirty == nullptr)
      ? OptValue<size_t>()
      : OptValue<size_t>(reinterpret_cast<char *>(nextDirty) - lowLim());
}

void CardTable::clear() {
  size_t firstIndex = addressToIndex(parentLowLim_);
  size_t lastIndex = addressToIndex(parentEnd_ - 1);
  cleanRange(firstIndex, lastIndex);
}

void CardTable::updateAfterCompaction(bool youngIsEmpty, void *newLevel) {
  size_t firstIndex = addressToIndex(parentStart_);
  // Note that care must be taken to subtract one, to get the last card in
  // the oldGen, not the first card beyond.
  size_t endCardIndex = addressToIndex(parentEnd_ - 1);
  if (youngIsEmpty) {
    cleanRange(firstIndex, endCardIndex);
    return;
  }
  // Otherwise, must dirty the occupied cards.
  char *newLevelPtr = static_cast<char *>(newLevel);
  size_t firstCleanCardIndex = 0;
  if (newLevelPtr > parentStart_) {
    size_t lastDirtyCardIndex = addressToIndex(newLevelPtr - 1);
    dirtyRange(firstIndex, lastDirtyCardIndex);
    firstCleanCardIndex = lastDirtyCardIndex + 1;
  }
  if (parentEnd_ > parentStart_) {
    // It is OK if firstCleanCardIndex may be greater than endCardIndex; this
    // call does nothing in that case.
    cleanRange(firstCleanCardIndex, endCardIndex);
  }
}

void CardTable::cleanRange(size_t from, size_t to) {
  cleanOrDirtyRange(from, to, CardStatus::Clean);
}

void CardTable::dirtyRange(size_t from, size_t to) {
  cleanOrDirtyRange(from, to, CardStatus::Dirty);
}

void CardTable::cleanOrDirtyRange(
    size_t from,
    size_t to,
    CardStatus cleanOrDirty) {
  for (size_t index = from; index <= to; index++) {
    cards()[index] = cleanOrDirty;
  }
}

} // namespace vm
} // namespace hermes
