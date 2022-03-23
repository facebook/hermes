/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_CONSUMABLERANGE_H
#define HERMES_SUPPORT_CONSUMABLERANGE_H

#include <cassert>
#include <iterator>

namespace hermes {

/// A mutable range of elements that is consumed from the front.
template <typename T>
class ConsumableRange {
  T cur_;
  const T end_;

 public:
  ConsumableRange(T begin, T end) : cur_(begin), end_(end) {}

  ConsumableRange(const ConsumableRange &other) = default;
  ConsumableRange &operator=(const ConsumableRange &other) = delete;

  ConsumableRange(ConsumableRange &&other) = default;
  ConsumableRange &operator=(ConsumableRange &&other) = default;

  /// True if there is a next element, i.e., range is non-empty.
  bool hasNext() const {
    return cur_ != end_;
  }

  /// Consume and return the next element.
  decltype(*cur_) next() {
    assert(hasNext());
    return *cur_++;
  }

  /// Return next without consuming it.
  decltype(*cur_) peek() const {
    assert(hasNext());
    return *cur_;
  }

  /// Number of elements remaining in this range.
  size_t size() const {
    using std::distance;
    return distance(cur_, end_);
  }
};

} // namespace hermes

#endif // HERMES_SUPPORT_CONSUMABLERANGE_H
