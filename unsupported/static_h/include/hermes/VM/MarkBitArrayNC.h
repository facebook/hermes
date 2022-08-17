/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_MARKBITARRAYNC_H
#define HERMES_VM_MARKBITARRAYNC_H

#include "hermes/ADT/BitArray.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/AlignedStorage.h"
#include "hermes/VM/ExpectedPageSize.h"
#include "hermes/VM/HeapAlign.h"

namespace hermes {
namespace vm {

/// Encapsulates the array of bits used for marking the allocation region of an
/// AlignedHeapSegment.  The array expects to be constructed inside an
/// AlignedHeapSegment's storage, at some position before the allocation region.
class MarkBitArrayNC {
 public:
  MarkBitArrayNC() = default;

  /// MarkBitArrayNC is not copyable or movable: It must be constructed
  /// in-place.
  MarkBitArrayNC(const MarkBitArrayNC &) = delete;
  MarkBitArrayNC(MarkBitArrayNC &&) = delete;
  MarkBitArrayNC &operator=(const MarkBitArrayNC &) = delete;
  MarkBitArrayNC &operator=(MarkBitArrayNC &&) = delete;

  /// Returns the size of the bit array, counted as a number of bits.
  ///
  /// For the sake of simplicity, there are enough bits to cover an allocation
  /// region that takes up all the space in an AlignedHeapSegment -- even though
  /// some prefix of the storage is reserved for auxiliary data structures.
  static constexpr size_t size();

  /// Translate the given address, which is required to be within the
  /// covered range of the MarkBitArray, to a 0-based index in the
  /// array.  (Rounds down to the nearest aligned address.)
  inline size_t addressToIndex(const void *p) const;

  /// Translate the given index, which is required to be within the
  /// range of the array, to an external address.
  inline char *indexToAddress(size_t ind) const;

  /// Returns the bit for the given index, which is required to be within the
  /// range of the array.
  inline bool at(size_t ind) const;

  /// Marks the bit for the given index, which is required to be within the
  /// range of the array.
  inline void mark(size_t ind);

  /// Clears the bit array.
  inline void clear();

  /// Sets all bits to true.
  inline void markAll();

  /// Finds the next bit in the MarkBitArray that is set to 1, starting at and
  /// including \p ind, the index from which to begin searching. Returns one
  /// past the last array index if there is not another marked bit.
  inline size_t findNextMarkedBitFrom(size_t ind);

  /// Finds the next bit in the MarkBitArray that is set to 0, starting at and
  /// including \p ind, the index from which to begin searching. Returns one
  /// past the last array index if there is not another marked bit.
  inline size_t findNextUnmarkedBitFrom(size_t ind);

  /// Finds the previous bit in the MarkBitArray that is set to 1, starting at
  /// and
  /// including \p ind, the index from which to begin searching. Returns one
  /// past the last array index if there is not another marked bit.
  inline size_t findPrevMarkedBitFrom(size_t ind);

  /// Finds the previous bit in the MarkBitArray that is set to 0, starting at
  /// and including \p ind, the index from which to begin searching. Returns one
  /// past the last array index if there is not another marked bit.
  inline size_t findPrevUnmarkedBitFrom(size_t ind);

// Mangling scheme used by MSVC encode public/private into the name.
// As a result, vanilla "ifdef public" trick leads to link errors.
#if defined(UNIT_TEST) || defined(_MSC_VER)
 public:
#else
 private:
#endif

  /// \return The lowest address that can be marked in this array. i.e. The
  ///     lowest address such that
  ///
  ///     addressToIndex(base()) == 0
  ///
  /// Note that the base address will be strictly less than the address
  /// corresponding to the start of the allocation region (by at least the width
  /// of the mark bit array).
  inline char *base() const;

  /// The number of bits representing the total number of heap-aligned addresses
  /// in the AlignedStorage.
  static constexpr size_t kNumBits = AlignedStorage::size() >> LogHeapAlign;
  /// Bitset holding the contents of the mark bit array. Align it to the page
  /// size.
  BitArray<kNumBits> bitArray_;
};

/* static */ constexpr size_t MarkBitArrayNC::size() {
  return kNumBits;
}

size_t MarkBitArrayNC::addressToIndex(const void *p) const {
  assert(
      base() <= p && p < AlignedStorage::end(base()) &&
      "precondition: argument must be within the covered range");
  auto cp = reinterpret_cast<const char *>(p);
  return (cp - base()) >> LogHeapAlign;
}

char *MarkBitArrayNC::indexToAddress(size_t ind) const {
  assert(ind < kNumBits && "ind must be within the index range");
  char *res = base() + (ind << LogHeapAlign);

  assert(
      base() <= res && res < AlignedStorage::end(base()) &&
      "result must be within the covered range.");
  return res;
}

bool MarkBitArrayNC::at(size_t ind) const {
  assert(ind < kNumBits && "precondition: ind must be within the index range");
  return bitArray_.at(ind);
}

void MarkBitArrayNC::mark(size_t ind) {
  assert(ind < kNumBits && "precondition: ind must be within the index range");
  bitArray_.set(ind, true);
}

void MarkBitArrayNC::clear() {
  bitArray_.reset();
}

void MarkBitArrayNC::markAll() {
  bitArray_.set();
}

size_t MarkBitArrayNC::findNextMarkedBitFrom(size_t ind) {
  return bitArray_.findNextSetBitFrom(ind);
}

size_t MarkBitArrayNC::findNextUnmarkedBitFrom(size_t ind) {
  return bitArray_.findNextZeroBitFrom(ind);
}

size_t MarkBitArrayNC::findPrevMarkedBitFrom(size_t ind) {
  return bitArray_.findPrevSetBitFrom(ind);
}

size_t MarkBitArrayNC::findPrevUnmarkedBitFrom(size_t ind) {
  return bitArray_.findPrevZeroBitFrom(ind);
}

char *MarkBitArrayNC::base() const {
  // As we know the mark bit array is laid out inline before the allocation
  // region of its aligned heap segment, we can use its own this pointer as the
  // base address. It is safe to cast away the const because we never use the
  // resulting pointer to index back into the array, but rather to index past it
  // into the allocation region.
  return const_cast<char *>(reinterpret_cast<const char *>(this));
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_MARKBITARRAYNC_H
