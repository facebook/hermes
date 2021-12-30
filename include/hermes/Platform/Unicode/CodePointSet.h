/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMUNICODE_CODEPOINTSET_H
#define HERMES_PLATFORMUNICODE_CODEPOINTSET_H

#include <algorithm>
#include <cassert>

#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/SmallVector.h"

namespace hermes {

/// A simple struct which represents a contiguous range of code points.
struct CodePointRange {
  /// The first element of the range.
  uint32_t first;

  /// The length of the range.
  uint32_t length;

  /// \return one past the last element of the range.
  uint32_t end() const {
    assert(first + length >= first && "Range overflowed");
    return first + length;
  }

  /// \return whether this range overlaps another range.
  bool overlaps(const CodePointRange &rhs) {
    return this->first < rhs.end() && rhs.first < this->end();
  }

  /// \return whether this range abuts (is exactly next to) another range.
  bool abuts(const CodePointRange &rhs) {
    return this->first == rhs.end() || rhs.first == this->end();
  }
};

/// A class which manages a set of Unicode codepoints as a list of sorted,
/// disjoint ranges. Deletion is not supported.

// N.B.: This may seem a natural use case for llvh::IntervalMap. However it is
// not for a few reasons:
// 1. IntervalMap trips an assertion if you add a range that overlaps an
// existing range. So when inserting, you must mask out the ranges already
// stored.
// 2. IntervalMap requires an allocator whose lifetime is awkward to manage.
// 3. IntervalMap is quite complex and is significant code size regression
// relative to this.
class CodePointSet {
 public:
  /// Add a range of code points to this set.
  void add(CodePointRange r) {
    if (r.length == 0)
      return;

    // Use equal_range to find the subarray which we overlap, treating
    // overlapping or abutting as equality.
    auto cmp = [](CodePointRange lhs, CodePointRange rhs) {
      if (lhs.overlaps(rhs) || lhs.abuts(rhs))
        return false;
      return lhs.first < rhs.first;
    };
    auto pair = std::equal_range(ranges_.begin(), ranges_.end(), r, cmp);

    if (pair.first == pair.second) {
      // There was no overlap, just insert.
      ranges_.insert(pair.first, r);
    } else {
      // We overlapped with at least one existing range. Extend the first such
      // range with the total overlap, and erase any subsequent overlapping
      // ranges.
      // The beginning of the merged range is the smaller of the first overlap's
      // left, and our range's left. The end of the merged range is the larger
      // of the last overlaps's end, and our range's end.
      uint32_t start = std::min(r.first, pair.first->first);
      uint32_t end = std::max(r.end(), (pair.second - 1)->end());
      *pair.first = CodePointRange{start, end - start};
      ranges_.erase(pair.first + 1, pair.second);
    }
  }

  /// Add a single code point to this set.
  void add(uint32_t cp) {
    add(CodePointRange{cp, 1});
  }

  /// \return whether the set is empty.
  bool empty() const {
    return ranges_.empty();
  }

  /// \return the value of the first code point.
  uint32_t first() const {
    return ranges_.front().first;
  }

  /// \return one past the last code point.
  uint32_t end() const {
    return ranges_.back().end();
  }

  /// \return the list of ranges.
  llvh::ArrayRef<CodePointRange> ranges() const {
    return ranges_;
  }

  /// \return whether a code point \p cp is contained within this set.
  bool contains(uint32_t cp) const {
    auto cmp = [](CodePointRange left, uint32_t cp) {
      return left.first + left.length <= cp;
    };
    auto where = std::lower_bound(ranges_.begin(), ranges_.end(), cp, cmp);
    assert(
        (where == ranges_.end() || where->end() > cp) &&
        "Code point should be inside or before found range");
    return where != ranges_.end() && where->first <= cp;
  }

 private:
  llvh::SmallVector<CodePointRange, 4> ranges_;
};

} // namespace hermes

#endif // HERMES_PLATFORMUNICODE_CODEPOINTSET_H
