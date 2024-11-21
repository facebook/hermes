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

/// The card table optimizes young gen collections by restricting the amount of
/// heap belonging to the old gen that must be scanned. The card table expects
/// to be constructed at the beginning of a segment's storage, and covers the
/// extent of that storage's memory. There are two cases:
/// 1. For FixedSizeHeapSegment, the inline CardStatus array and Boundary array
/// in the card table is large enough.
/// 2. For JumboHeapSegment, the two arrays are allocated separately.
/// In either case, the pointers to the CardStatus array and Boundary array are
/// stored in \c cards and \c boundaries field of SHSegmentInfo.
///
/// Also supports the following query:  Given a card in the heap that intersects
/// with the used portion of its segment, find its "crossing object" -- the
/// object whose extent [obj-start, end) contains the start of the card.  This
/// allows us to scan dirty cards: finding the crossing object allows us to then
/// do object-to-object traversal to the end of the card.
class CardTable {
 public:
  /// Points at the start of a card.
  class Boundary {
    friend class CardTable;

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

  enum class CardStatus : char { Clean = 0, Dirty = 1 };

  /// The size (and base-two log of the size) of cards used in the card table.
  static constexpr size_t kLogCardSize = 9; // ==> 512-byte cards.
  static constexpr size_t kCardSize = 1 << kLogCardSize; // ==> 512-byte cards.
  /// Maximum size of segment that can have inline cards and boundaries array.
  static constexpr size_t kSegmentUnitSize = 1
      << HERMESVM_LOG_HEAP_SEGMENT_SIZE;

  /// The size of the maximum inline card table. CardStatus array and boundary
  /// array for larger segment has larger size and is stored separately.
  static constexpr size_t kInlineCardTableSize =
      kSegmentUnitSize >> kLogCardSize;

  /// For convenience, this is a conversion factor to determine how many bytes
  /// in the heap correspond to a single byte in the card table. This is
  /// distinct from kCardSize, which tells us how many bytes in the heap
  /// correspond to a single byte in the card table. However, since each index
  /// corresponds to a single byte for now, they are the same value. This is
  /// guaranteed by a static_assert below.
  static constexpr size_t kHeapBytesPerCardByte = kCardSize;

  /// A prefix of every segment is occupied by auxiliary data structures. The
  /// card table is the first such data structure. The card table maps to the
  /// segment. Only the suffix of the card table that maps to the suffix of
  /// entire segment that is used for allocation is ever used; the prefix that
  /// maps to the card table itself is not used, nor is the portion of the card
  /// table that maps to the other auxiliary data structure: the mark bit array
  /// and guard pages. This small space can be used for other purpose, such as
  /// storing the SHSegmentInfo (we assert in AlignedHeapSegment that its
  /// size won't exceed this unused space). The actual first used index should
  /// take into account all these structures. Here we only calculate for
  /// CardTable and size of SHSegmentInfo. It's only used as starting index for
  /// clearing/dirtying range of bits.
  /// Note that the total size of the card table is 2 times kCardTableSize,
  /// since the CardTable contains two byte arrays of that size (cards_ and
  /// boundaries_). And this index must be larger than the size of SHSegmentInfo
  /// to avoid corrupting it when clearing/dirtying bits.
  static constexpr size_t kFirstUsedIndex = std::max(
      sizeof(SHSegmentInfo),
      (2 * kInlineCardTableSize) >> kLogCardSize);

  CardTable(size_t segmentSize) {
    assert(
        segmentSize && segmentSize % kSegmentUnitSize == 0 &&
        "segmentSize must be a multiple of kSegmentUnitSize");

    segmentInfo_.shiftedSegmentSize =
        segmentSize >> HERMESVM_LOG_HEAP_SEGMENT_SIZE;
    if (segmentSize == kSegmentUnitSize) {
      // Just use the inline storage.
      setCards(inlineCardStatusArray_);
      setBoundaries(inlineBoundaryArray_);
    } else {
      size_t cardTableSize = segmentSize >> kLogCardSize;
      // CardStatus is clean by default, so must zero-initialize it.
      setCards(new AtomicIfConcurrentGC<CardStatus>[cardTableSize] {});
      setBoundaries(new int8_t[cardTableSize]);
    }
  }
  /// CardTable is not copyable or movable: It must be constructed in-place.
  CardTable(const CardTable &) = delete;
  CardTable(CardTable &&) = delete;
  CardTable &operator=(const CardTable &) = delete;
  CardTable &operator=(CardTable &&) = delete;

  ~CardTable() {
    // If CardStatus/Boundary array is allocated separately, free them.
    if (cards() != inlineCardStatusArray_) {
      delete[] cards();
      delete[] boundaries();
    }
  }

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
  inline size_t addressToIndex(const void *addr) const LLVM_NO_SANITIZE("null");

  /// Returns the address corresponding to the given card table index.
  ///
  /// \pre \p index is bounded:
  ///
  ///     0 <= index <= getEndIndex()
  inline const char *indexToAddress(size_t index) const;

  /// Make the card table entry for the given address dirty.
  /// \pre \p addr is required to be an address covered by the card table.
  /// This only works for memory in normal objects (i.e., do not support large
  /// allocation) and is more efficient than the version for large objects.
  inline void dirtyCardForAddress(const void *addr);

  /// Make the card table entry for the given address dirty.
  /// \pre \p addr is required to be an address covered by the card table.
  /// This reads the cards array pointer from SHSegmentInfo, so works for normal
  /// and large objects.
  inline void dirtyCardForAddressInLargeObj(const void *addr);

  /// Make the card table entries for cards that intersect the given address
  /// range dirty.  The range is a closed interval [low, high].
  /// \pre \p low and \p high are required to be addresses covered by the card
  /// table.
  void dirtyCardsForAddressRange(const void *low, const void *high);

  /// Returns whether the card table entry for the given address is dirty.
  /// \pre \p addr is required to be an address covered by the card table.
  /// This only works for memory in normal objects (i.e., do not support large
  /// allocation) and is more efficient than the version for large objects.
  inline bool isCardForAddressDirty(const void *addr) const;

  /// Returns whether the card table entry for the given index is dirty.
  /// \pre \p index is required to be a valid card table index.
  /// This only works for memory in normal objects (i.e., do not support large
  /// allocation) and is more efficient than the version for large objects.
  inline bool isCardForIndexDirty(const size_t index) const;

  /// Version of isCardForAddressDirty()/isCardForIndexDirty that works for
  /// normal and large objects.
  inline bool isCardForAddressDirtyInLargeObj(const void *addr) const;
  inline bool isCardForIndexDirtyInLargeObj(const size_t index) const;

  /// If there is a dirty card at or after \p fromIndex, at an index less than
  /// \p endIndex, returns the index of the dirty card, else returns none.
  inline OptValue<size_t> findNextDirtyCard(size_t fromIndex, size_t endIndex)
      const;

  /// If there is a card at or after \p fromIndex, at an index less than
  /// \p endIndex, returns the index of the clean card, else returns none.
  inline OptValue<size_t> findNextCleanCard(size_t fromIndex, size_t endIndex)
      const;

  /// \return The first boundary that could be crossed by a suitably large
  /// allocation starting at \p level.
  inline Boundary nextBoundary(const char *level) const;

  /// Clears the card table.
  void clear();

  /// Modify the card table to be (conservatively) valid after old generation
  /// compaction: dirty all cards containing objects after compaction (up to \p
  /// newLevel), clean all now-unoccupied cards.  This keeps us from having to
  /// track what cards contain old-to-young pointers after compaction -- we
  /// assume any card might.
  ///
  /// TODO (T26751833) figure out if this is a performance problem, and do
  /// better if necessary.
  void updateAfterCompaction(const void *newLevel);

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
  GCCell *firstObjForCard(unsigned index) const;

  /// Get the segment size from SHSegmentInfo. This is only used in debug code
  /// or when clearing the entire card table.
  size_t getSegmentSize() const {
    return (size_t)segmentInfo_.shiftedSegmentSize
        << HERMESVM_LOG_HEAP_SEGMENT_SIZE;
  }

  /// The end index of the card table (all valid indices should be smaller).
  size_t getEndIndex() const {
    return getSegmentSize() >> kLogCardSize;
  }

#ifdef HERMES_EXTRA_DEBUG
  /// Temporary debugging hack: yield the numeric value of the boundaries_ array
  /// for the given \p index.
  /// TODO(T48709128): remove this when the problem is diagnosed.
  int8_t cardObjectTableValue(unsigned index) const {
    return boundaries()[index];
  }

  /// These methods protect and unprotect, respectively, the memory
  /// that comprises the card boundary table.  They require that the
  /// start of the boundary table is page-aligned, and its size is a
  /// multiple of the page size.
  /// TODO(T48709128): remove this when the problem is diagnosed.
  void protectBoundaryTable();
  void unprotectBoundaryTable();
#endif // HERMES_EXTRA_DEBUG

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
#endif // HERMES_SLOW_DEBUG

 private:
#ifndef NDEBUG
  /// Returns the pointer to the end of the storage starting at \p lowLim.
  void *storageEnd(const void *lowLim) const {
    return reinterpret_cast<char *>(
        reinterpret_cast<uintptr_t>(lowLim) + getSegmentSize());
  }
#endif

  void setCards(AtomicIfConcurrentGC<CardStatus> *cards) {
    segmentInfo_.cards = cards;
  }

  AtomicIfConcurrentGC<CardStatus> *cards() const {
    return static_cast<AtomicIfConcurrentGC<CardStatus> *>(segmentInfo_.cards);
  }

  void setBoundaries(int8_t *boundaries) {
    segmentInfo_.boundaries = boundaries;
  }

  int8_t *boundaries() const {
    return segmentInfo_.boundaries;
  }

  /// \return The lowest address whose card can be dirtied in this array. i.e.
  ///     The smallest address such that
  ///
  ///     addressToIndex(base()) == 0
  ///
  /// Note that the base address will be strictly less than the address
  /// corresponding to the start of the allocation region (by at least the width
  /// of the card table).
  inline const char *base() const LLVM_NO_SANITIZE("null");

  /// The encoding scheme for the logarithmic-time object boundary queries for
  /// large objects.  encodeExp encodes an exponent to a (negative) table value,
  /// and decodeExp is the inverse function.
  static inline int8_t encodeExp(int8_t exp);
  static inline int8_t decodeExp(int8_t encodedVal);

  /// Returns true iff ptr is card-aligned.
  static inline bool isCardAligned(const void *ptr);

  /// \return the index of the first card in the range [fromIndex, endIndex)
  /// with value \p status, or none if one does not exist.
  OptValue<size_t> findNextCardWithStatus(
      CardStatus status,
      size_t fromIndex,
      size_t endIndex) const;

  /// Clean, or dirty, the indicated index ranges in the card table.
  /// Ranges are half-open: [from, to).
  void cleanRange(size_t from, size_t to);
  void dirtyRange(size_t from, size_t to);

  void cleanOrDirtyRange(size_t from, size_t to, CardStatus cleanOrDirty);

  union {
    /// The bytes occupied by segmentInfo_ are guaranteed to be not override by
    /// writes to cards_ array. See static assertions in AlignedHeapSegment.
    /// Pointers to the underlying CardStatus array and boundary array are
    /// stored in it. Note that we could also store the boundary array in a
    /// union along with inlineBoundaryArray_, since that array has unused
    /// prefix bytes as well. It will save 8 bytes here. But it makes the size
    /// check more complex as we need to ensure that the segment size is large
    /// enough so that inlineBoundaryArray_ has enough unused prefix bytes to
    /// store the pointer.
    SHSegmentInfo segmentInfo_;
    /// This needs to be atomic so that the background thread in Hades can
    /// safely dirty cards when compacting.
    AtomicIfConcurrentGC<CardStatus>
        inlineCardStatusArray_[kInlineCardTableSize]{};
  };

  /// See the comment at kHeapBytesPerCardByte above to see why this is
  /// necessary.
  static_assert(
      sizeof(inlineCardStatusArray_[0]) == 1,
      "Validate assumption that card table entries are one byte");

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
  int8_t inlineBoundaryArray_[kInlineCardTableSize];
};

/// Implementations of inlines.
inline void CardTable::Boundary::bump() {
  index_++;
  address_ += kCardSize;
}

inline size_t CardTable::Boundary::index() const {
  return index_;
}

inline const char *CardTable::Boundary::address() const {
  return address_;
}

inline CardTable::Boundary::Boundary(size_t index, const char *address)
    : index_(index), address_(address) {}

inline size_t CardTable::addressToIndex(const void *addr) const {
  auto addrPtr = reinterpret_cast<const char *>(addr);
  assert(
      base() <= addrPtr &&
      addrPtr < (static_cast<const char *>(storageEnd(base())) + kCardSize) &&
      "address is required to be within range.");
  return (addrPtr - base()) >> kLogCardSize;
}

inline const char *CardTable::indexToAddress(size_t index) const {
  assert(index <= getEndIndex() && "index must be within the index range");
  const char *res = base() + (index << kLogCardSize);
  assert(
      base() <= res && res <= storageEnd(base()) &&
      "result must be within the covered range");
  return res;
}

inline void CardTable::dirtyCardForAddress(const void *addr) {
  // Make sure that this is not called on a large segment.
  assert(
      inlineCardStatusArray_ == cards() &&
      "CardStatus array for this CardTable is allocated separately");
  inlineCardStatusArray_[addressToIndex(addr)].store(
      CardStatus::Dirty, std::memory_order_relaxed);
}

inline void CardTable::dirtyCardForAddressInLargeObj(const void *addr) {
  cards()[addressToIndex(addr)].store(
      CardStatus::Dirty, std::memory_order_relaxed);
}

inline bool CardTable::isCardForAddressDirty(const void *addr) const {
  return isCardForIndexDirty(addressToIndex(addr));
}

inline bool CardTable::isCardForIndexDirty(size_t index) const {
  assert(index < getEndIndex() && "index is required to be in range.");
  // Make sure that this is not called on a large segment.
  assert(
      inlineCardStatusArray_ == cards() &&
      "CardStatus array for this CardTable is allocated separately");
  return inlineCardStatusArray_[index].load(std::memory_order_relaxed) ==
      CardStatus::Dirty;
}

inline bool CardTable::isCardForAddressDirtyInLargeObj(const void *addr) const {
  return isCardForIndexDirtyInLargeObj(addressToIndex(addr));
}

inline bool CardTable::isCardForIndexDirtyInLargeObj(size_t index) const {
  assert(index < getEndIndex() && "index is required to be in range.");
  return cards()[index].load(std::memory_order_relaxed) == CardStatus::Dirty;
}

inline OptValue<size_t> CardTable::findNextDirtyCard(
    size_t fromIndex,
    size_t endIndex) const {
  return findNextCardWithStatus(CardStatus::Dirty, fromIndex, endIndex);
}

inline OptValue<size_t> CardTable::findNextCleanCard(
    size_t fromIndex,
    size_t endIndex) const {
  return findNextCardWithStatus(CardStatus::Clean, fromIndex, endIndex);
}

inline CardTable::Boundary CardTable::nextBoundary(const char *level) const {
  assert(level != nullptr);
  size_t ix = addressToIndex(level - 1) + 1;
  const char *addr = indexToAddress(ix);

  return {ix, addr};
}

inline const char *CardTable::base() const {
  // As we know the card table is laid out inline at the beginning of the
  // segment storage, which is before the allocation region, we can use its own
  // this pointer as the base address.
  return reinterpret_cast<const char *>(this);
}

/* static */ inline int8_t CardTable::encodeExp(int8_t exp) {
  return -(exp + 1);
}

/* static */ inline int8_t CardTable::decodeExp(int8_t encodedVal) {
  return -encodedVal - 1;
}

/* static */ inline bool CardTable::isCardAligned(const void *ptr) {
  return (reinterpret_cast<intptr_t>(ptr) & (kCardSize - 1)) == 0;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_CARDTABLE_H
