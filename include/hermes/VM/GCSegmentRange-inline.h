/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GCSEGMENTRANGE_INLINE_H
#define HERMES_VM_GCSEGMENTRANGE_INLINE_H

#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/GCSegmentRange.h"

namespace hermes {
namespace vm {

template <typename I>
inline GCSegmentRange::Ptr GCSegmentRange::fromConsumable(I begin, I end) {
  return std::unique_ptr<Consumable<I>>(new Consumable<I>{{begin, end}});
}

inline GCSegmentRange::Ptr GCSegmentRange::fuse(
    GCSegmentRange::Ptr underlying) {
  return std::unique_ptr<Fused>(new Fused{std::move(underlying)});
}

inline GCSegmentRange::Ptr GCSegmentRange::singleton(AlignedHeapSegment *seg) {
  return fromConsumable(seg, seg + 1);
}

template <typename... Ranges>
inline GCSegmentRange::Ptr GCSegmentRange::concat(
    std::unique_ptr<Ranges>... ranges) {
  Concat::Spine spine;
  spine.reserve(sizeof...(Ranges));

  // Hack to emit a sequence of emplace_back calls.
  // The leading zero is necessary because MSVC does not allow stack-allocated
  // empty arrays, which `sink` would otherwise be when this function is
  // invoked with an empty parameter list.
  int sink[] = {0, (spine.emplace_back(std::move(ranges)), 0)...};
  (void)sink;

  return std::unique_ptr<Concat>(new Concat{std::move(spine)});
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCSEGMENTRANGE_INLINE_H
