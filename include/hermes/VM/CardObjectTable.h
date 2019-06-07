/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_CARDOBJECTTABLE_H
#define HERMES_VM_CARDOBJECTTABLE_H

#include "hermes/VM/CardTable.h"
#include "hermes/VM/DependentMemoryRegion.h"
#include "hermes/VM/GCCell.h"

namespace hermes {
namespace vm {

/// The CardObjectTable.  This allows the following query: given a card
/// in the heap (typically a card made dirty in the card table by the GC
/// write barrier), find the correspond "crossing object": the object
/// whose extent [obj-start, end) intersects the start of the card.
/// This allows us to scan dirty cards: finding the crossing object
/// allows us to then do object-to-object traversal to the end of the
/// card.
///
/// We assume that the table describes a space in which allocation is
/// done contiguously, in address order.  To facilitate this, the
/// CardObjectTable maintains the index and lower boundary of the next
/// card that allocation will cross.
///
/// Use of DependentMemoryRegion allows the used portion of the card
/// table to grow or shrink as the used portion of the heap does.

/// Implementation: we view the memory as a sequence of signed bytes,
/// in one-to-one correspondence with the cards of the parent region.
/// Non-negative values k indicate that the crossing object starts
/// k * HeapAlign bytes before the start of the card..  If k is a
/// negative value, it indicates to go back 2^((-k)-1) entries in the object
/// table, and consult the entry there.  This scheme allows the
/// start of a large object to be found in logarithmic time: if we
/// allocate a large object that crosses many cards, the first
/// crossed card gets a non-negative value, and each subsequent one
/// uses the maximum exponent that stays within the card range for
/// the object.

class CardObjectTable : public DependentMemoryRegion {
 public:
  /// This requires the cardTable in order to be able to translate
  /// addresses to card indices.
  CardObjectTable(
      char *parentLowLim,
      char *parentStart,
      char *parentEnd,
      char *parentHiLim,
      CardTable *cardTable);

  /// An allocation of [start, end) has crossed at least one
  /// card boundary.  Update the table to appropriately
  /// to describe the allocation.
  void updateEntries(char *start, char *end);

  /// Returns the start address of the first object that extends onto
  /// the card with the given index.  (If an object is allocated at a
  /// card boundary, that is the first object.)
  GCCell *firstObjForCard(unsigned index) const;

  /// The lower limit of the card whose index is nextIndex_.
  char *nextCardBoundary() const {
    return nextCardBoundary_;
  }

  /// The parent region has moved by the given delta; update accordingly.
  void moveParentRegion(ptrdiff_t delta) {
    DependentMemoryRegion::moveParentRegion(delta);
    nextCardBoundary_ += delta;
  }

  /// Reset the table, so that the first covered card is the next one
  /// that will be crossed.
  void reset();

#ifdef HERMES_SLOW_DEBUG
  /// Verify the accuracy of the card object table: for each card that
  /// starts before level, find the crossing object, and verify that
  /// it has a valid VTable.
  void verify(char *level) const;
#endif

 private:
  /// View of the allocated memory as an array of int8_t.
  int8_t *entries() {
    return reinterpret_cast<int8_t *>(lowLim());
  }

  /// Const view of the same thing.
  const int8_t *entries() const {
    return reinterpret_cast<const int8_t *>(lowLim());
  }

  /// The encoding scheme for the logarithmic-time card object table
  /// queries for large object, as explained in the comment above
  /// cardObjectTable_, below.  encodeExp encodes an exponent to a (negative)
  /// table value, and decodeExp is the inverse function.
  static inline int8_t encodeExp(int8_t exp);
  static inline int8_t decodeExp(int8_t encodedVal);

  /// The index in table of the next card that will be
  /// crossed.
  unsigned nextIndex_{0};

  /// The lower limit of the card whose index is nextIndex_.
  char *nextCardBoundary_;

  /// We require the card table in order to translate addresses to indices.
  CardTable const *cardTable_;
};

/*static*/
inline int8_t CardObjectTable::encodeExp(int8_t exp) {
  return -(exp + 1);
}

/*static*/
inline int8_t CardObjectTable::decodeExp(int8_t encodedVal) {
  return -encodedVal - 1;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_CARDOBJECTTABLE_H
