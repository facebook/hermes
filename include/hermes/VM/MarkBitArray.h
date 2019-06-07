/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_MARKBITARRAY_H
#define HERMES_VM_MARKBITARRAY_H

#include "hermes/VM/DependentMemoryRegion.h"
#include "hermes/VM/HeapAlign.h"
#include "llvm/Support/MathExtras.h"

namespace hermes {
namespace vm {

/// Encapsulates the array of bits used for marking.
class MarkBitArray : public DependentMemoryRegion {
 public:
  /// Initialize a bit array that can cover the range [baseAddr, endAddr),
  /// assuming the given (byte) heap alignment.  (One bit for each
  /// aligned address within the range.)
  MarkBitArray(
      char *parentLowLim,
      char *parentStart,
      char *parentEnd,
      char *parentHiLim)
      : DependentMemoryRegion(
            "hermes-markbitarray",
            parentLowLim,
            parentStart,
            parentEnd,
            parentHiLim,
            LogHeapAlign + kLogBitsPerByte),
        numValidIndices_((parentEnd_ - parentStart_) >> LogHeapAlign),
        bitArraySize_(
            llvm::alignTo(numValidIndices_, kBitsPerVal) >> kLogBitsPerVal),
        bitArray_(reinterpret_cast<size_t *>(lowLim())) {}

  /// Translate the given address, which is required to be within the
  /// covered range of the MarkBitArray, to a 0-based index in the
  /// array.  (Rounds down to the nearest aligned address.)
  size_t addressToIndex(void *p) const {
    assert(
        parentStart_ <= p && p < parentEnd_ &&
        "precondition: argument must be within the covered range");
    const char *cp = reinterpret_cast<const char *>(p);
    return (cp - parentStart_) >> LogHeapAlign;
  }

  /// Translate the given index, which is required to be within the
  /// range of the array, to an external address.
  char *indexToAddress(size_t ind) const {
    assert(
        ind < numValidIndices_ &&
        "precondition: ind must be within the index range");
    char *res = parentStart_ + (ind << LogHeapAlign);
    // This assertion should follow from the precondition on "ind".
    assert(res < parentEnd_ && "result must be within the covered range.");
    return res;
  }

  /// Returns the bit for the given index, which is required to be within the
  /// range of the array.
  bool at(size_t ind) const {
    assert(
        ind < numValidIndices_ &&
        "precondition: ind must be within the index range");
    return bitArray_[ind / kBitsPerVal] & (size_t)1 << (ind % kBitsPerVal);
  }

  /// Sets the bit for the given index, which is required to be within the
  /// range of the array, to the given value.
  void set(size_t ind, bool value) {
    assert(
        ind < numValidIndices_ &&
        "precondition: ind must be within the index range");
    if (value) {
      bitArray_[ind / kBitsPerVal] |= (size_t)1 << (ind % kBitsPerVal);
    } else {
      bitArray_[ind / kBitsPerVal] &= ~((size_t)1 << (ind % kBitsPerVal));
    }
  }

  /// Clears the bit array.
  void clear() {
    ::memset(bitArray_, 0, sizeof(bitArray_[0]) * bitArraySize_);
  }

  /// Returns the size of the array.
  size_t size() const {
    return numValidIndices_;
  }

  /// Resizes the start and end of the MarkBitArray, updating size values.
  void resizeParentUsedRegion(char *start, char *end) override {
    DependentMemoryRegion::resizeParentUsedRegion(start, end);
    numValidIndices_ = (parentEnd_ - parentStart_) >> LogHeapAlign;
    bitArraySize_ =
        llvm::alignTo(numValidIndices_, kBitsPerVal) >> kLogBitsPerVal;
  }

  // Finds the next bit in the MarkBitArray that is set to 1, starting at and
  // including \p ind, the index from which to begin searching. If there is not
  // another marked bit, the number of valid indices in the array is returned.
  size_t findNextMarkedBit(size_t ind);

 private:
  // TODO: vm_allocate the storage, and treat as array of size_t, to
  // allow skipping of entirely zero words, and to enable declaring
  // unused memory to the OS.

  /// The number of bits representing the total number of heap-aligned addresses
  /// on the heap. This is different from bitArraySize_, since each value of
  /// bitArray_ can fit an amount of valid indices equal to the number of bits
  /// in a word.
  size_t numValidIndices_;

  /// Size of bitArray_
  size_t bitArraySize_;

  /// The underlying bits.
  size_t *const bitArray_;

  /// Log of bits in each value of bitArray_.
  static constexpr unsigned kLogBitsPerVal = sizeof(size_t) == 8 ? 6 : 5;
  /// Number of bits in each value of bitArray_, which is the number of bits a
  /// word can hold.
  static constexpr unsigned kBitsPerVal = 1 << kLogBitsPerVal;

  // Log of number of bits in a byte. Needed to calculate the log ratio of bytes
  // in the parent region to bytes in the mark bit array.
  static constexpr unsigned kLogBitsPerByte = 3;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_MARKBITARRAY_H
