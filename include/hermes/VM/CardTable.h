/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_CARDTABLE_H
#define HERMES_VM_CARDTABLE_H

#include "hermes/Support/OptValue.h"
#include "hermes/VM/DependentMemoryRegion.h"

#include <cassert>

namespace hermes {
namespace vm {

/// The card table.  Use of DependentMemoryRegion allows the used
/// portion of the card table to grow or shrink as the used portion of
/// the heap does.

class CardTable : public DependentMemoryRegion {
 public:
  /// The size (and base-two log of the size) of cards used in the card table.
  static constexpr size_t kLogCardSize = 9; // ==> 512-byte cards.
  static constexpr size_t kCardSize = 1 << kLogCardSize; // ==> 512-byte cards.

  CardTable(
      char *parentLowLim,
      char *parentStart,
      char *parentEnd,
      char *parentHiLim);

  /// Returns the card table index corresponding to the given address.
  /// \pre \p addr must be within the bounds of the heap.
  inline size_t addressToIndex(void *addr) const;

  /// Returns the address corresponding to the given card table
  /// index.
  /// \pre \p index is required to be a valid index for the card table.
  inline char *indexToAddress(size_t index) const;

  /// Make the card table entry for the given address dirty.
  /// \pre \p addr is required to be an address covered by the card table.
  inline void dirtyCardForAddress(void *addr);

  /// Make the card table entries for cards that intersect the given address
  /// range dirty.  The range is a closed interval [low, high].
  /// \pre \p low and \p high are required to be addresses covered by the card
  /// table.
  void dirtyCardsForAddressRange(void *low, void *high);

  /// Returns whether the card table entry for the given address is dirty.
  /// \pre \p addr is required to be an address covered by the card table.
  inline bool isCardForAddressDirty(void *addr) const;

  /// Returns whether the card table entry for the given index is dirty.
  /// \pre \p index is required to be a valid card table index.
  inline bool isCardForIndexDirty(size_t index) const;

  /// If there is a dirty card at or after \p fromIndex, at an index less than
  /// \p endIndex, returns the index of the dirty card, else returns none.
  OptValue<size_t> findNextDirtyCard(size_t fromIndex, size_t endIndex) const;

  /// Clears the card table.
  void clear();

  /// If youngIsEmpty is true, assume that full GC has completely
  /// emptied the young generation, and clear the card table.
  /// Otherwise, the young generation is not empty, so we modify the
  /// card table to be (conservatively) valid after old generation
  /// compaction: dirty all cards containing objects after compaction
  /// (up to \p newLevel), clean all now-unoccupied cards.  This keeps
  /// us from having to track what cards contain old-to-young pointers
  /// after compaction -- we assume any card might.  TODO: figure out
  /// if this is a performance problem, and do better if necessary.
  void updateAfterCompaction(bool youngIsEmpty, void *newLevel);

 private:
  enum class CardStatus : char { Clean = 0, Dirty = 1 };

  /// View of the allocated memory as an array of CardStatus.
  CardStatus *cards() {
    return reinterpret_cast<CardStatus *>(lowLim());
  }

  /// View of the allocated memory as an array of const CardStatus.
  const CardStatus *cards() const {
    return reinterpret_cast<const CardStatus *>(lowLim());
  }

  /// Clean, or dirty, the indicated index ranges in the card table.
  /// Ranges are inclusive: [from, to].
  void cleanRange(size_t from, size_t to);
  void dirtyRange(size_t from, size_t to);

  void cleanOrDirtyRange(size_t from, size_t to, CardStatus cleanOrDirty);
};

/// Implementations of inlines.
inline size_t CardTable::addressToIndex(void *addr) const {
  char *addrPtr = reinterpret_cast<char *>(addr);
  assert(
      parentLowLim_ <= addrPtr && addrPtr <= parentHiLim_ &&
      "address is required to be within range.");
  return (addrPtr - parentLowLim_) >> kLogCardSize;
}

inline char *CardTable::indexToAddress(size_t index) const {
  char *res = parentLowLim_ + (index << kLogCardSize);
  assert(parentLowLim_ <= res && res <= parentHiLim_);
  return res;
}

inline void CardTable::dirtyCardForAddress(void *addr) {
  cards()[addressToIndex(addr)] = CardStatus::Dirty;
}

inline bool CardTable::isCardForAddressDirty(void *addr) const {
  return isCardForIndexDirty(addressToIndex(addr));
}

inline bool CardTable::isCardForIndexDirty(size_t index) const {
  assert(
      static_cast<ptrdiff_t>(index) < (hiLim() - lowLim()) &&
      "index is required to be in range.");
  return cards()[index] == CardStatus::Dirty;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_CARDTABLE_H
