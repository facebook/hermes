/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_CARDTABLE_H
#define HERMES_VM_CARDTABLE_H

#include "hermes/Support/ErrorHandling.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/Support/OptValue.h"
#include "hermes/VM/ExpectedPageSize.h"
#include "hermes/VM/GCCell.h"

#include "llvh/Support/MathExtras.h"

#include <cassert>

namespace hermes {
namespace vm {

/// Supports the following query:  Given a card in the heap that intersects
/// with the used portion of its segment, find its "crossing object" -- the
/// object whose extent [obj-start, end) contains the start of the card.  This
/// allows us to scan dirty cards: finding the crossing object allows us to then
/// do object-to-object traversal to the end of the card.
/// Note: This only works for FixedSizeHeapSegment. JumboHeapSegment can at most
/// have one GCCell, which must be allocated at the start of the allocation
/// region. So it does not use need boundary information.
class CardBoundaryTable {
 public:
  /// Points at the start of a card.
  class Boundary {
    friend class CardBoundaryTable;

   public:
    Boundary() = default;

    /// Move boundary to the edge at the next highest address.
    inline void bump();

    /// The index for the card starting at \c address(), in the table covering
    /// that address.
    inline size_t index() const;

    /// The (inclusive) start address of the card.
    inline const char *address() const;

   private:
    inline Boundary(size_t index, const char *address);

    size_t index_{0};
    const char *address_{nullptr};
  };

  /// The size (and base-two log of the size) of cards used in the card table.
  static constexpr size_t kLogCardSize = 9; // ==> 512-byte cards.
  static constexpr size_t kCardSize = 1 << kLogCardSize; // ==> 512-byte cards.
  /// Maximum size of segment that can have inline cards array and requires a
  /// card boundary array.
  static constexpr size_t kSegmentUnitSize = 1
      << HERMESVM_LOG_HEAP_SEGMENT_SIZE;

  /// The size of the boundary array. JumboHeapSegment does not use
  /// boundary array so this is always large enough.
  static constexpr size_t kBoundaryArraySize = kSegmentUnitSize >> kLogCardSize;

  CardBoundaryTable() = default;
  /// CardBoundaryTable is not copyable or movable: It must be constructed
  /// in-place.
  CardBoundaryTable(const CardBoundaryTable &) = delete;
  CardBoundaryTable(CardBoundaryTable &&) = delete;
  CardBoundaryTable &operator=(const CardBoundaryTable &) = delete;
  CardBoundaryTable &operator=(CardBoundaryTable &&) = delete;

  /// \return The first boundary that could be crossed by a suitably large
  /// allocation starting at \p level.
  inline Boundary nextBoundary(const char *level) const;

  /// An allocation of [start, end) has crossed at least one card boundary.
  /// Update the boundaries table appropriately to describe the allocation.
  ///
  /// \pre boundary is not null.
  /// \pre [start, end) must be covered by this table.
  /// \pre boundary's index must correspond to its address in this table.
  /// \pre [start, end) crosses \p *boundary.
  ///
  /// \post for all card indices I s.t. start <= indexToAddress(I) < end,
  ///   firstObjForCard(I) == start
  ///
  /// \post *boundary == nextBoundary(end)
  void updateBoundaries(Boundary *boundary, const char *start, const char *end);

  /// Returns the start address of the first object that extends onto the card
  /// with the given index.  (If an object is allocated at a card boundary, that
  /// is the first object.)
  GCCell *firstObjForCard(const char *lowLim, const char *hiLim, unsigned index)
      const;

#ifdef HERMES_SLOW_DEBUG
  /// Asserts that for every card covering [start, level), what we believe to
  /// be its "crossing object"
  ///
  ///  1. Is valid.
  ///  2. Starts on or before the start of the card, and ends after the start of
  ///     the card.
  ///
  /// \pre start is card-aligned.
  void verifyBoundaries(char *start, char *level) const;

  /// Find the object that owns the memory at \p loc.
  GCCell *findObjectContaining(
      const char *lowLim,
      const char *hiLim,
      const void *loc) const;
#endif // HERMES_SLOW_DEBUG

#ifndef UNIT_TEST
 private:
#endif
  /// Returns the card table index corresponding to a byte at the given address.
  /// \pre \p addr must be within the bounds of the segment owning this card
  /// table or at most 1 card after it, that is to say
  ///
  ///    segment.lowLim() <= addr < segment.hiLim() + kCardSize
  ///
  /// Note that we allow the extra card after the segment in order to simplify
  /// the logic for callers that are using this function to generate an open
  /// interval of card indices. See \c dirtyCardsForAddressRange for an example
  /// of how this is used.
  size_t addressToIndex(const void *addr) const LLVM_NO_SANITIZE("null") {
    auto addrPtr = reinterpret_cast<const char *>(addr);
    auto *lowLim = storageStart(addr);
    assert(
        lowLim <= addrPtr && addrPtr < (storageEnd(addr) + kCardSize) &&
        "address is required to be within range.");
    return (addrPtr - lowLim) >> kLogCardSize;
  }

  /// Returns the address corresponding to the given card table index.
  ///
  /// \pre \p index is bounded:
  ///
  ///     0 <= index <= getEndIndex()
  const char *indexToAddress(const char *lowLim, size_t index) const {
    assert(
        index <= (kSegmentUnitSize >> kLogCardSize) &&
        "index must be within the index range");
    const char *res = lowLim + (index << kLogCardSize);
    assert(
        lowLim <= res && res <= storageEnd(lowLim) &&
        "result must be within the covered range");
    return res;
  }

 private:
  /// Computing the lowLim and hiLim of the segment owning \p addr.
  static char *storageStart(const void *addr) {
    return reinterpret_cast<char *>(
        reinterpret_cast<uintptr_t>(addr) & ~(kSegmentUnitSize - 1));
  }
  static char *storageEnd(const void *addr) {
    return storageStart(addr) + kSegmentUnitSize;
  }

  /// The encoding scheme for the logarithmic-time object boundary queries for
  /// large objects.  encodeExp encodes an exponent to a (negative) table value,
  /// and decodeExp is the inverse function.
  static inline int8_t encodeExp(int8_t exp);
  static inline int8_t decodeExp(int8_t encodedVal);

  /// Returns true iff ptr is card-aligned.
  static inline bool isCardAligned(const void *ptr);

  /// Each card has a corresponding signed byte in the boundaries_ table.  A
  /// non-negative entry, K, indicates that the crossing object starts K *
  /// HeapAlign bytes before the start of the card. A negative entry, L,
  /// indicates that we must seek backwards by 2^(-L-1) indices and consult the
  /// entry there.
  ///
  /// This scheme allows the start of a large object to be found in logarithmic
  /// time:  If we allocate a large object that crosses many cards, the first
  /// crossed cards gets a non-negative value, and each subsequent one uses the
  /// maximum exponent that stays within the card range for the object.
  int8_t boundaries_[kBoundaryArraySize];
};

/// Implementations of inlines.
inline void CardBoundaryTable::Boundary::bump() {
  index_++;
  address_ += kCardSize;
}

inline size_t CardBoundaryTable::Boundary::index() const {
  return index_;
}

inline const char *CardBoundaryTable::Boundary::address() const {
  return address_;
}

inline CardBoundaryTable::Boundary::Boundary(size_t index, const char *address)
    : index_(index), address_(address) {}

inline CardBoundaryTable::Boundary CardBoundaryTable::nextBoundary(
    const char *level) const {
  assert(level != nullptr);
  size_t ix = addressToIndex(level - 1) + 1;
  const char *addr = indexToAddress(storageStart(level), ix);

  return {ix, addr};
}

/* static */ inline int8_t CardBoundaryTable::encodeExp(int8_t exp) {
  return -(exp + 1);
}

/* static */ inline int8_t CardBoundaryTable::decodeExp(int8_t encodedVal) {
  return -encodedVal - 1;
}

/* static */ inline bool CardBoundaryTable::isCardAligned(const void *ptr) {
  return (reinterpret_cast<intptr_t>(ptr) & (kCardSize - 1)) == 0;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_CARDTABLE_H
