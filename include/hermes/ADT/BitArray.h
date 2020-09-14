/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ADT_BITARRAY_H
#define HERMES_ADT_BITARRAY_H

#include "llvh/Support/MathExtras.h"

#include <array>
#include <bitset>

namespace hermes {

/// This serves as a replacement for std::bitset that provides fast search and
/// customisable alignment. The storage is fixed size and inline, unlike
/// std::vector<bool> or llvm::BitVector.
/// \tparam N the number of bits to be stored.
/// \tparam A the desired alignment of this structure in bytes.
template <size_t N, size_t A = sizeof(uintptr_t)>
class BitArray {
 private:
  static_assert(
      A && A % sizeof(uintptr_t) == 0,
      "Bit set alignment must be a non-zero multiple of word size!");
  static constexpr size_t kBitsPerWord = 8 * sizeof(uintptr_t);
  static constexpr size_t kNumWords =
      llvh::alignTo<kBitsPerWord>(N) / kBitsPerWord;
  static constexpr size_t kPaddedWords =
      llvh::alignTo<A / sizeof(uintptr_t)>(kNumWords);
  std::array<uintptr_t, kPaddedWords> allBits_;

  /// Find the first bit with value \p V at or after \p idx.
  template <bool V>
  size_t findNextBitImpl(size_t idx) const {
    assert(
        idx <= N &&
        "precondition: idx is less than or equal to number of bits");
    // If there are bits after N but in the same word, we do not need to
    // generate this special case, since we will do a check on idx at the end.
    if (N % kBitsPerWord == 0 && idx == N) {
      return N;
    }
    size_t wordIdx = idx / kBitsPerWord;
    size_t offset = idx % kBitsPerWord;
    // Start looking from the given idx. Invert the word if we are looking for a
    // 0 so it's the same as looking for a 1.
    uintptr_t currentWord = V ? allBits_[wordIdx] : ~allBits_[wordIdx];
    // Set all bits before idx in the word to zero so we skip past them.
    currentWord &= (std::numeric_limits<uintptr_t>::max() << offset);

    // In the case where we have padding words, they are filled with 1's, so we
    // do not need the bounds check on wordIdx when searching for a 1.
    constexpr bool noBoundsCheck = kNumWords < kPaddedWords && V;
    // Loops through word-sized values in the bit array. Note that at the end of
    // this loop, wordIdx points to the element right after currentWord. Writing
    // it in this way helps with handling the first and last element edge cases.
    ++wordIdx;
    while (!currentWord && (noBoundsCheck || wordIdx < kNumWords)) {
      currentWord = V ? allBits_[wordIdx] : ~allBits_[wordIdx];
      ++wordIdx;
    }
    idx = (wordIdx - 1) * kBitsPerWord + llvh::countTrailingZeros(currentWord);
    // In the case where N is not a multiple of the word size, this ensures
    // that the returned value is equal to N. Otherwise, this check is
    // optimised away at compile time.
    if (N % kBitsPerWord) {
      idx = std::min(idx, N);
    }
    return idx;
  }

 public:
  BitArray() {
    std::fill_n(allBits_.begin(), kNumWords, 0);
    // Fill the padding bits with 1's so we can efficiently search for 1's in
    // the data.
    std::fill_n(
        allBits_.begin() + kNumWords,
        kPaddedWords - kNumWords,
        std::numeric_limits<uintptr_t>::max());
  }

  /// Set all bits to 1.
  inline void set() {
    std::fill_n(
        allBits_.begin(), kNumWords, std::numeric_limits<uintptr_t>::max());
  }

  inline void set(size_t idx, bool val) {
    const uintptr_t mask = 1ULL << (idx % kBitsPerWord);
    const size_t wordIdx = idx / kBitsPerWord;
    if (val)
      allBits_[wordIdx] |= mask;
    else
      allBits_[wordIdx] &= ~mask;
  }

  /// Set all bits to 0.
  inline void reset() {
    std::fill_n(allBits_.begin(), kNumWords, 0);
  }

  /// Return a bool representing the value at \p idx.
  inline bool at(size_t idx) const {
    assert(idx < N && "Index must be within the bitset");
    const uintptr_t mask = 1ULL << (idx % kBitsPerWord);
    const auto wordIdx = idx / kBitsPerWord;
    return allBits_[wordIdx] & mask;
  }

  /// Find the index of the first set bit at or after \p idx.
  size_t findNextSetBitFrom(size_t idx) const {
    return findNextBitImpl<true>(idx);
  }
  /// Find the index of the first unset bit at or after \p idx.
  size_t findNextZeroBitFrom(size_t idx) const {
    return findNextBitImpl<false>(idx);
  }
};
} // namespace hermes

#endif // HERMES_ADT_BITARRAY_H
