/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_MARKBITARRAYNC_H
#define HERMES_VM_MARKBITARRAYNC_H

#include "hermes/Support/OSCompat.h"
#include "hermes/VM/AlignedStorage.h"
#include "hermes/VM/ExpectedPageSize.h"
#include "hermes/VM/HeapAlign.h"

#include "llvm/Support/MathExtras.h"

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

  /// Finds the next bit in the MarkBitArray that is set to 1, starting at and
  /// including \p ind, the index from which to begin searching. Returns one
  /// past the last array index if there is not another marked bit.
  size_t findNextMarkedBitFrom(size_t ind);

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

  /// Log of bits in each value of bitArray().
  static constexpr unsigned kLogBitsPerVal = sizeof(size_t) == 8 ? 6 : 5;
  /// Number of bits in each value of bitArray(), which is the number of bits a
  /// word can hold.
  static constexpr unsigned kBitsPerVal = 1 << kLogBitsPerVal;

  // Log of number of bits in a byte. Needed to calculate the log ratio of bytes
  // in the parent region to bytes in the mark bit array.
  static constexpr unsigned kLogBitsPerByte = 3;

  /// The number of bits representing the total number of heap-aligned addresses
  /// in the AlignedStorage. This is different from kBitArraySize, since each
  /// value in the bit array can fit an amount of valid indices equal to the
  /// number of bits in a word.
  static constexpr size_t kValidIndices =
      AlignedStorage::size() >> LogHeapAlign;

  /// Number of elements in the bit array (values, not bits).  Round up to the
  /// platform-specific maximum page size.
  static constexpr size_t kValsPerMaxPage =
      pagesize::kExpectedPageSize / sizeof(size_t);
  static constexpr size_t kBitArraySize = llvm::alignTo<kValsPerMaxPage>(
      ((kValidIndices - 1) >> kLogBitsPerVal) + 1);

  /// The inline array holding the contents of the mark bit array.
  size_t bitArray_[kBitArraySize]{};
};

/* static */ constexpr size_t MarkBitArrayNC::size() {
  return kValidIndices;
}

size_t MarkBitArrayNC::addressToIndex(const void *p) const {
  assert(
      base() <= p && p < AlignedStorage::end(base()) &&
      "precondition: argument must be within the covered range");
  auto cp = reinterpret_cast<const char *>(p);
  return (cp - base()) >> LogHeapAlign;
}

char *MarkBitArrayNC::indexToAddress(size_t ind) const {
  assert(ind < kValidIndices && "ind must be within the index range");
  char *res = base() + (ind << LogHeapAlign);

  assert(
      base() <= res && res < AlignedStorage::end(base()) &&
      "result must be within the covered range.");
  return res;
}

bool MarkBitArrayNC::at(size_t ind) const {
  assert(
      ind < kValidIndices &&
      "precondition: ind must be within the index range");
  return bitArray_[ind / kBitsPerVal] & (size_t)1 << (ind % kBitsPerVal);
}

void MarkBitArrayNC::mark(size_t ind) {
  assert(
      ind < kValidIndices &&
      "precondition: ind must be within the index range");

  bitArray_[ind / kBitsPerVal] |= (size_t)1 << (ind % kBitsPerVal);
}

void MarkBitArrayNC::clear() {
  ::memset(bitArray_, 0, sizeof(bitArray_));
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
