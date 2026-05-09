/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "llvh/ADT/ArrayRef.h"

#include <cstddef>
#include <cstdint>

namespace hermes {

/// Search the range [start, end) of \p arr forward for \p target.
/// \pre start <= end <= arr.size().
/// \return the index of the first match, or -1 if not found.
int64_t
searchU8(llvh::ArrayRef<uint8_t> arr, size_t start, size_t end, uint8_t target);

/// Search the range [start, end) of \p arr backward for \p target.
/// \pre start <= end <= arr.size().
/// \return the index of the last match, or -1 if not found.
int64_t searchReverseU8(
    llvh::ArrayRef<uint8_t> arr,
    size_t start,
    size_t end,
    uint8_t target);

/// Search the range [start, end) of \p arr forward for \p target.
/// \pre start <= end <= arr.size().
/// \return the index of the first match, or -1 if not found.
int64_t searchU16(
    llvh::ArrayRef<uint16_t> arr,
    size_t start,
    size_t end,
    uint16_t target);

/// Search the range [start, end) of \p arr backward for \p target.
/// \pre start <= end <= arr.size().
/// \return the index of the last match, or -1 if not found.
int64_t searchReverseU16(
    llvh::ArrayRef<uint16_t> arr,
    size_t start,
    size_t end,
    uint16_t target);

/// Search the range [start, end) of \p arr forward for \p target.
/// \pre start <= end <= arr.size().
/// \return the index of the first match, or -1 if not found.
int64_t searchU32(
    llvh::ArrayRef<uint32_t> arr,
    size_t start,
    size_t end,
    uint32_t target);

/// Search the range [start, end) of \p arr backward for \p target.
/// \pre start <= end <= arr.size().
/// \return the index of the last match, or -1 if not found.
int64_t searchReverseU32(
    llvh::ArrayRef<uint32_t> arr,
    size_t start,
    size_t end,
    uint32_t target);

/// Search the range [start, end) of \p arr forward for \p target.
/// \pre start <= end <= arr.size().
/// \return the index of the first match, or -1 if not found.
int64_t searchU64(
    llvh::ArrayRef<uint64_t> arr,
    size_t start,
    size_t end,
    uint64_t target);

/// Search the range [start, end) of \p arr backward for \p target.
/// \pre start <= end <= arr.size().
/// \return the index of the last match, or -1 if not found.
int64_t searchReverseU64(
    llvh::ArrayRef<uint64_t> arr,
    size_t start,
    size_t end,
    uint64_t target);

/// Scan [start, end) for the first byte that requires escaping in a JSON
/// string per ECMA-404: '"' (0x22), '\\' (0x5C), or any control character
/// (<= 0x1F).
/// \return pointer to the first such character, or \p end if none found.
const char *scanJsonEscapeU8(const char *start, const char *end);

/// Scan [start, end) for the first char16_t that requires escaping in a JSON
/// string per ECMA-404: '"' (0x22), '\\' (0x5C), or any control character
/// (<= 0x1F).
/// \return pointer to the first such character, or \p end if none found.
const char16_t *scanJsonEscapeU16(const char16_t *start, const char16_t *end);

} // namespace hermes
