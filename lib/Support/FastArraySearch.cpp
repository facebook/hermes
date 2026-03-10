/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/// \file FastArraySearch.cpp
/// SIMD-accelerated linear search for typed arrays.
///
/// Each search function processes the array in 128-bit (16-byte) chunks using
/// SIMD, then falls back to scalar for the remaining tail elements.  The number
/// of elements per chunk depends on the element width:
///   - uint8_t:  16 elements per chunk
///   - uint16_t:  8 elements per chunk
///   - uint32_t:  4 elements per chunk
///   - uint64_t:  2 elements per chunk
///
/// NEON (aarch64) strategy:
///   1. Broadcast the target into every lane of a 128-bit vector (vdupq_n_*).
///   2. Load 128 bits of array data (vld1q_*).
///   3. Compare element-wise: result lanes are all-ones on match, zero
///      otherwise (vceqq_*).
///   4. Quick any-match check:
///      - For u8/u16/u32: vmaxvq_* reduces the comparison vector to its
///        maximum lane value.  Non-zero means at least one lane matched.
///      - For u64: extract individual lanes (vgetq_lane_u64) since there are
///        only 2 lanes.
///   5. On a hit, collapse to a scalar bitmask by ANDing the comparison lanes
///      with per-lane power-of-2 bit values {1, 2, 4, ...} and summing
///      (vaddvq_*), then use countTrailingZeros (forward) or countLeadingZeros
///      (reverse) to find the matching element position.
///
/// SSE2 (x86-64) strategy:
///   1. Broadcast the target into every lane (_mm_set1_epi*).
///   2. Load 128 bits of array data (_mm_loadu_si128, unaligned).
///   3. Compare element-wise (_mm_cmpeq_epi*): result lanes are all-ones on
///      match, zero otherwise.
///   4. Collapse the comparison into a 16-bit bitmask where each bit
///      corresponds to one *byte* of the 128-bit result (_mm_movemask_epi8).
///      Non-zero means at least one lane matched.
///   5. On a hit, find the matching element position from the bitmask:
///      - For u8: each element produces exactly 1 mask bit, so
///        countTrailingZeros(mask) gives the byte (= element) index directly.
///      - For u16: each element produces 2 mask bits, so divide by 2.
///      - For u32: each element produces 4 mask bits, so divide by 4.
///      For reverse searches, countLeadingZeros is used to find the highest
///      set bit instead.
///
/// For u64 on SSE2, there is no _mm_cmpeq_epi64 intrinsic, so the search
/// falls through to the scalar path.

#include "hermes/Support/FastArraySearch.h"
#include "hermes/Support/SIMD.h"

#include "llvh/Support/MathExtras.h"

#include <cassert>

namespace hermes {
namespace {

/// Scalar linear search over [start, end) of \p arr for \p target.
/// \tparam T the element type.
/// \tparam Reverse when true, search backward returning the last match;
///   when false, search forward returning the first match.
/// \return the index of the match, or -1 if not found.
template <typename T, bool Reverse>
int64_t
scalarSearch(llvh::ArrayRef<T> arr, size_t start, size_t end, T target) {
  if constexpr (Reverse) {
    for (size_t i = end; i > start;) {
      --i;
      if (arr[i] == target)
        return static_cast<int64_t>(i);
    }
  } else {
    for (size_t i = start; i < end; ++i) {
      if (arr[i] == target)
        return static_cast<int64_t>(i);
    }
  }
  return -1;
}

//===----------------------------------------------------------------------===//
// u8 search implementation
//===----------------------------------------------------------------------===//

/// SIMD-accelerated linear search over [start, end) of a uint8_t array.
/// Uses NEON (aarch64) or SSE2 (x86-64) when available, with a scalar tail
/// for remaining elements.
/// \tparam Reverse when true, search backward returning the last match;
///   when false, search forward returning the first match.
/// \return the index of the match, or -1 if not found.
template <bool Reverse>
int64_t searchU8Impl(
    llvh::ArrayRef<uint8_t> arr,
    size_t start,
    size_t end,
    uint8_t target) {
  assert(start <= end && "start must be <= end");
  assert(end <= arr.size() && "end exceeds array bounds");
  size_t len = end - start;

#ifdef HERMES_SIMD_NEON
  // Broadcast target into all sixteen 8-bit lanes.
  uint8x16_t needle = vdupq_n_u8(target);
  // Per-lane bit values for collapsing comparison lanes into a scalar bitmask
  // on a hit.  Since each lane is only 8 bits, the maximum power-of-2 is
  // 128 = 2^7, so we process the 16 lanes as two groups of 8.
  // MSVC doesn't support constexpr initializer for NEON vector types so we
  // use vld1 here.
  static constexpr uint8_t kLaneBitsArr[] = {1, 2, 4, 8, 16, 32, 64, 128};
  static const uint8x8_t kLaneBits = vld1_u8(kLaneBitsArr);
  size_t numChunks = len / 16;
  for (size_t c = 0; c < numChunks; ++c) {
    size_t base = Reverse ? end - (c + 1) * 16 : start + c * 16;
    // Load 16 consecutive bytes.
    uint8x16_t data = vld1q_u8(arr.data() + base);
    // Per-lane equality: matching lanes become 0xFF, others 0.
    uint8x16_t cmp = vceqq_u8(data, needle);
    // Quick rejection: horizontal max is zero when no lane matched.
    if (vmaxvq_u8(cmp)) {
      // Collapse to a 16-bit scalar mask: split into two 8-lane halves,
      // AND each with {1,2,4,8,16,32,64,128}, sum each half to get an
      // 8-bit mask, then combine into a 16-bit mask.
      uint8_t lo = vaddv_u8(vand_u8(vget_low_u8(cmp), kLaneBits));
      uint8_t hi = vaddv_u8(vand_u8(vget_high_u8(cmp), kLaneBits));
      uint16_t mask = lo | (static_cast<uint16_t>(hi) << 8);
      if constexpr (Reverse)
        return static_cast<int64_t>(
            base + (15 - llvh::countLeadingZeros(mask)));
      else
        return static_cast<int64_t>(base + llvh::countTrailingZeros(mask));
    }
  }
  size_t tailLen = len % 16;
#elif defined(HERMES_SIMD_SSE2)
  // Broadcast target into all sixteen 8-bit lanes.
  __m128i needle = _mm_set1_epi8(static_cast<char>(target));
  size_t numChunks = len / 16;
  for (size_t c = 0; c < numChunks; ++c) {
    size_t base = Reverse ? end - (c + 1) * 16 : start + c * 16;
    __m128i data =
        _mm_loadu_si128(reinterpret_cast<const __m128i *>(arr.data() + base));
    // Per-lane equality: matching lanes become 0xFF, others 0.
    __m128i cmp = _mm_cmpeq_epi8(data, needle);
    // Each byte produces exactly 1 mask bit, so the mask bit position
    // equals the element index directly.
    unsigned mask = static_cast<unsigned>(_mm_movemask_epi8(cmp));
    if (mask) {
      if constexpr (Reverse) {
        // Highest set bit = last matching byte index.
        unsigned pos = 31 - llvh::countLeadingZeros(mask);
        return static_cast<int64_t>(base + pos);
      } else {
        unsigned pos = llvh::countTrailingZeros(mask);
        return static_cast<int64_t>(base + pos);
      }
    }
  }
  size_t tailLen = len % 16;
#else
  size_t tailLen = len;
#endif

  // Scalar tail.
  if constexpr (Reverse)
    return scalarSearch<uint8_t, true>(arr, start, start + tailLen, target);
  else
    return scalarSearch<uint8_t, false>(arr, end - tailLen, end, target);
}

//===----------------------------------------------------------------------===//
// u16 search implementation
//===----------------------------------------------------------------------===//

/// SIMD-accelerated linear search over [start, end) of a uint16_t array.
/// Uses NEON (aarch64) or SSE2 (x86-64) when available, with a scalar tail
/// for remaining elements.
/// \tparam Reverse when true, search backward returning the last match;
///   when false, search forward returning the first match.
/// \return the index of the match, or -1 if not found.
template <bool Reverse>
int64_t searchU16Impl(
    llvh::ArrayRef<uint16_t> arr,
    size_t start,
    size_t end,
    uint16_t target) {
  assert(start <= end && "start must be <= end");
  assert(end <= arr.size() && "end exceeds array bounds");
  size_t len = end - start;

#ifdef HERMES_SIMD_NEON
  // Broadcast target into all eight 16-bit lanes.
  uint16x8_t needle = vdupq_n_u16(target);
  // Per-lane bit values for collapsing comparison lanes into a scalar bitmask
  // on a hit.
  // MSVC doesn't support constexpr initializer for NEON vector types so we
  // use vld1q here.
  static constexpr uint16_t kLaneBitsArr[] = {1, 2, 4, 8, 16, 32, 64, 128};
  static const uint16x8_t kLaneBits = vld1q_u16(kLaneBitsArr);
  size_t numChunks = len / 8;
  for (size_t c = 0; c < numChunks; ++c) {
    size_t base = Reverse ? end - (c + 1) * 8 : start + c * 8;
    // Load 8 consecutive uint16_t elements.
    uint16x8_t data = vld1q_u16(arr.data() + base);
    // Per-lane equality: matching lanes become 0xFFFF, others 0.
    uint16x8_t cmp = vceqq_u16(data, needle);
    // Quick rejection: horizontal max is zero when no lane matched.
    if (vmaxvq_u16(cmp)) {
      // Collapse to an 8-bit scalar mask: AND with {1,2,4,...,128} and sum,
      // then use ctz/clz to find the first/last match.
      uint16_t mask = vaddvq_u16(vandq_u16(cmp, kLaneBits));
      if constexpr (Reverse)
        return static_cast<int64_t>(
            base + (15 - llvh::countLeadingZeros(mask)));
      else
        return static_cast<int64_t>(base + llvh::countTrailingZeros(mask));
    }
  }
  size_t tailLen = len % 8;
#elif defined(HERMES_SIMD_SSE2)
  // Broadcast target into all eight 16-bit lanes.
  __m128i needle = _mm_set1_epi16(static_cast<short>(target));
  size_t numChunks = len / 8;
  for (size_t c = 0; c < numChunks; ++c) {
    size_t base = Reverse ? end - (c + 1) * 8 : start + c * 8;
    __m128i data =
        _mm_loadu_si128(reinterpret_cast<const __m128i *>(arr.data() + base));
    // Per-lane equality: matching lanes become 0xFFFF, others 0.
    __m128i cmp = _mm_cmpeq_epi16(data, needle);
    // Each 16-bit lane produces 2 mask bits, so divide the bit
    // position by 2 to get the element index.
    unsigned mask = static_cast<unsigned>(_mm_movemask_epi8(cmp));
    if (mask) {
      if constexpr (Reverse) {
        // Highest set bit / 2 = last matching element index.
        unsigned pos = (31 - llvh::countLeadingZeros(mask)) / 2;
        return static_cast<int64_t>(base + pos);
      } else {
        unsigned pos = llvh::countTrailingZeros(mask) / 2;
        return static_cast<int64_t>(base + pos);
      }
    }
  }
  size_t tailLen = len % 8;
#else
  size_t tailLen = len;
#endif

  // Scalar tail.
  if constexpr (Reverse)
    return scalarSearch<uint16_t, true>(arr, start, start + tailLen, target);
  else
    return scalarSearch<uint16_t, false>(arr, end - tailLen, end, target);
}

//===----------------------------------------------------------------------===//
// u32 search implementation
//===----------------------------------------------------------------------===//

/// SIMD-accelerated linear search over [start, end) of a uint32_t array.
/// Uses NEON (aarch64) or SSE2 (x86-64) when available, with a scalar tail
/// for remaining elements.
/// \tparam Reverse when true, search backward returning the last match;
///   when false, search forward returning the first match.
/// \return the index of the match, or -1 if not found.
template <bool Reverse>
int64_t searchU32Impl(
    llvh::ArrayRef<uint32_t> arr,
    size_t start,
    size_t end,
    uint32_t target) {
  assert(start <= end && "start must be <= end");
  assert(end <= arr.size() && "end exceeds array bounds");
  size_t len = end - start;

#ifdef HERMES_SIMD_NEON
  // Broadcast target into all four 32-bit lanes.
  uint32x4_t needle = vdupq_n_u32(target);
  // Per-lane bit values used to collapse comparison lanes into a scalar
  // bitmask on a hit: AND the all-ones/zero lanes with {1,2,4,8} then sum.
  // MSVC doesn't support constexpr initializer for NEON vector types so we
  // use vld1q here.
  static constexpr uint32_t kLaneBitsArr[] = {1, 2, 4, 8};
  static const uint32x4_t kLaneBits = vld1q_u32(kLaneBitsArr);
  size_t numChunks = len / 4;
  for (size_t c = 0; c < numChunks; ++c) {
    size_t base = Reverse ? end - (c + 1) * 4 : start + c * 4;
    // Load 4 consecutive uint32_t elements.
    uint32x4_t data = vld1q_u32(arr.data() + base);
    // Per-lane equality: matching lanes become 0xFFFFFFFF, others 0.
    uint32x4_t cmp = vceqq_u32(data, needle);
    // Quick rejection: horizontal max is zero when no lane matched.
    if (vmaxvq_u32(cmp)) {
      // Collapse to a 4-bit scalar mask where bit N is set iff lane N
      // matched, then use ctz/clz to find the first/last match.
      uint32_t mask = vaddvq_u32(vandq_u32(cmp, kLaneBits));
      if constexpr (Reverse)
        return static_cast<int64_t>(
            base + (31 - llvh::countLeadingZeros(mask)));
      else
        return static_cast<int64_t>(base + llvh::countTrailingZeros(mask));
    }
  }
  size_t tailLen = len % 4;
#elif defined(HERMES_SIMD_SSE2)
  // Broadcast target into all four 32-bit lanes.
  __m128i needle = _mm_set1_epi32(static_cast<int>(target));
  size_t numChunks = len / 4;
  for (size_t c = 0; c < numChunks; ++c) {
    size_t base = Reverse ? end - (c + 1) * 4 : start + c * 4;
    __m128i data =
        _mm_loadu_si128(reinterpret_cast<const __m128i *>(arr.data() + base));
    // Per-lane equality: matching lanes become 0xFFFFFFFF, others 0.
    __m128i cmp = _mm_cmpeq_epi32(data, needle);
    // Extract the MSB of each byte into a 16-bit mask.
    // Each 32-bit lane spans 4 bytes, so a matching lane sets 4
    // consecutive mask bits (a nibble like 0xF).
    unsigned mask = static_cast<unsigned>(_mm_movemask_epi8(cmp));
    if (mask) {
      // Each 32-bit lane produces 4 mask bits, so divide the bit
      // position by 4 to get the element index.
      if constexpr (Reverse)
        return static_cast<int64_t>(
            base + (31 - llvh::countLeadingZeros(mask)) / 4);
      else
        return static_cast<int64_t>(base + llvh::countTrailingZeros(mask) / 4);
    }
  }
  size_t tailLen = len % 4;
#else
  size_t tailLen = len;
#endif

  // Scalar tail.
  if constexpr (Reverse)
    return scalarSearch<uint32_t, true>(arr, start, start + tailLen, target);
  else
    return scalarSearch<uint32_t, false>(arr, end - tailLen, end, target);
}

//===----------------------------------------------------------------------===//
// u64 search implementation
//===----------------------------------------------------------------------===//

/// SIMD-accelerated linear search over [start, end) of a uint64_t array.
/// Uses NEON (aarch64) when available, with a scalar fallback otherwise
/// (SSE2 lacks a 64-bit integer compare).
/// \tparam Reverse when true, search backward returning the last match;
///   when false, search forward returning the first match.
/// \return the index of the match, or -1 if not found.
template <bool Reverse>
int64_t searchU64Impl(
    llvh::ArrayRef<uint64_t> arr,
    size_t start,
    size_t end,
    uint64_t target) {
  assert(start <= end && "start must be <= end");
  assert(end <= arr.size() && "end exceeds array bounds");
  size_t len = end - start;

#ifdef HERMES_SIMD_NEON
  // Broadcast target into both 64-bit lanes.
  uint64x2_t needle = vdupq_n_u64(target);
  size_t numChunks = len / 2;
  for (size_t c = 0; c < numChunks; ++c) {
    size_t base = Reverse ? end - (c + 1) * 2 : start + c * 2;
    uint64x2_t data = vld1q_u64(arr.data() + base);
    // Per-lane equality: matching lanes become all-ones.
    uint64x2_t cmp = vceqq_u64(data, needle);
    // No vmaxvq_u64, so extract each lane individually.
    uint64_t lane0 = vgetq_lane_u64(cmp, 0);
    uint64_t lane1 = vgetq_lane_u64(cmp, 1);
    if constexpr (Reverse) {
      if (lane1)
        return static_cast<int64_t>(base + 1);
      if (lane0)
        return static_cast<int64_t>(base);
    } else {
      if (lane0)
        return static_cast<int64_t>(base);
      if (lane1)
        return static_cast<int64_t>(base + 1);
    }
  }
  size_t tailLen = len % 2;
#else
  // SSE2 lacks a 64-bit integer compare (_mm_cmpeq_epi64 is SSE4.1),
  // so fall through to the scalar path.
  size_t tailLen = len;
#endif

  // Scalar tail.
  if constexpr (Reverse)
    return scalarSearch<uint64_t, true>(arr, start, start + tailLen, target);
  else
    return scalarSearch<uint64_t, false>(arr, end - tailLen, end, target);
}

} // namespace

//===----------------------------------------------------------------------===//
// Public API
//===----------------------------------------------------------------------===//

int64_t searchU8(
    llvh::ArrayRef<uint8_t> arr,
    size_t start,
    size_t end,
    uint8_t target) {
  return searchU8Impl<false>(arr, start, end, target);
}

int64_t searchReverseU8(
    llvh::ArrayRef<uint8_t> arr,
    size_t start,
    size_t end,
    uint8_t target) {
  return searchU8Impl<true>(arr, start, end, target);
}

int64_t searchU16(
    llvh::ArrayRef<uint16_t> arr,
    size_t start,
    size_t end,
    uint16_t target) {
  return searchU16Impl<false>(arr, start, end, target);
}

int64_t searchReverseU16(
    llvh::ArrayRef<uint16_t> arr,
    size_t start,
    size_t end,
    uint16_t target) {
  return searchU16Impl<true>(arr, start, end, target);
}

int64_t searchU32(
    llvh::ArrayRef<uint32_t> arr,
    size_t start,
    size_t end,
    uint32_t target) {
  return searchU32Impl<false>(arr, start, end, target);
}

int64_t searchReverseU32(
    llvh::ArrayRef<uint32_t> arr,
    size_t start,
    size_t end,
    uint32_t target) {
  return searchU32Impl<true>(arr, start, end, target);
}

int64_t searchU64(
    llvh::ArrayRef<uint64_t> arr,
    size_t start,
    size_t end,
    uint64_t target) {
  return searchU64Impl<false>(arr, start, end, target);
}

int64_t searchReverseU64(
    llvh::ArrayRef<uint64_t> arr,
    size_t start,
    size_t end,
    uint64_t target) {
  return searchU64Impl<true>(arr, start, end, target);
}

} // namespace hermes
