/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/MarkBitArray.h"

namespace hermes {
namespace vm {

size_t MarkBitArray::findNextMarkedBit(size_t ind) {
  assert(
      ind < numValidIndices_ &&
      "precondition: ind is less than number valid indices");
  size_t bitArrayIndex = ind / kBitsPerVal;

  // Start looking from the given ind.
  size_t currentValue = bitArray_[bitArrayIndex] >> (ind % kBitsPerVal);

  // Immediately traverse by word values if the next marked bit is in the same
  // word-sized block.
  if (currentValue != 0) {
    return ind + llvm::findFirstSet(currentValue);
  }
  bitArrayIndex++;

  // Loops through word-sized values in bitArray_. It is possible that
  // bitArraySize_ * kBitsPerVal > numValidIndices_, causing this loop to
  // check bits above numValidIndices_. However, those are all set to 0 and
  // will not affect the search.
  while (bitArrayIndex < bitArraySize_) {
    currentValue = bitArray_[bitArrayIndex];
    if (currentValue != 0)
      return bitArrayIndex * kBitsPerVal + llvm::findFirstSet(currentValue);
    bitArrayIndex++;
  }
  return numValidIndices_;
}

} // namespace vm
} // namespace hermes
