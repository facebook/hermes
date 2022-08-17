/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_COUNTINGITERATOR_H
#define HERMES_SUPPORT_COUNTINGITERATOR_H

#include <iterator>

namespace hermes {

// Iterator that counts number of operator++ invocations
template <typename T>
class CountingIterator {
 public:
  using iterator_category = std::input_iterator_tag;
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type *;
  using reference = value_type &;

  CountingIterator() = default;
  CountingIterator(const CountingIterator &cit) : count_(cit.count_) {}

  CountingIterator &operator++() {
    ++count_;
    return *this;
  }

  CountingIterator operator++(int) {
    CountingIterator tmp(*this);
    operator++();
    return tmp;
  }

  T &operator*() {
    return dummy_;
  }
  int count() const {
    return count_;
  }

  bool operator==(const CountingIterator &rhs) const {
    return false;
  }
  bool operator!=(const CountingIterator &rhs) const {
    return true;
  }

 private:
  int count_{0};
  T dummy_{};
};

} // namespace hermes

#endif // HERMES_SUPPORT_COUNTINGITERATOR_H
