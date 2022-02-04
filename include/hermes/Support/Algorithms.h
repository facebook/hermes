/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// <algorithm> style algorithms.
//===----------------------------------------------------------------------===//

#ifndef HERMES_SUPPORT_ALGORITHMS_H
#define HERMES_SUPPORT_ALGORITHMS_H

#include <algorithm>
#include <iterator>
#include <memory>
#include <type_traits>

namespace hermes {

/// Variants of std::uninitialized_copy and std::uninitialized_copy_n that
/// properly fall to std::copy (and thereby memmove) for trivial types. This
/// works around naive implementations of std::uninitialized_copy as seen in
/// some STL versions.
template <class InputIt, class ForwardIt>
ForwardIt uninitializedCopy(InputIt start, InputIt end, ForwardIt dst) {
  using SrcType = typename std::iterator_traits<InputIt>::value_type;
  using DstType = typename std::iterator_traits<ForwardIt>::value_type;
  if (std::is_trivial<SrcType>::value && std::is_trivial<DstType>::value)
    return std::copy(start, end, dst);
  return std::uninitialized_copy(start, end, dst);
}

template <class InputIt, class Size, class ForwardIt>
ForwardIt uninitializedCopyN(InputIt src, Size count, ForwardIt dst) {
  using SrcType = typename std::iterator_traits<InputIt>::value_type;
  using DstType = typename std::iterator_traits<ForwardIt>::value_type;
  if (std::is_trivial<SrcType>::value && std::is_trivial<DstType>::value)
    return std::copy_n(src, count, dst);
  return std::uninitialized_copy_n(src, count, dst);
}

} // namespace hermes

#endif // HERMES_SUPPORT_ALGORITHMS_H
