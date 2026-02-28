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

} // namespace hermes
