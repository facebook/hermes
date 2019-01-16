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

  /// Pass by value makes little sense for this class.
  ConsumableRange(const ConsumableRange &other) = delete;
  ConsumableRange &operator=(const ConsumableRange &other) = delete;

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
