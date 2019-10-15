/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GCSEGMENTADDRESSINDEX_H
#define HERMES_VM_GCSEGMENTADDRESSINDEX_H

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <vector>

namespace hermes {
namespace vm {

class AlignedHeapSegment;

/// Data structure for associating aligned heap segments with their starting
/// addresses.  Supports updating/inserting a segment into the index, and
/// iteration over the inserted segments, in order of their starting address.
class GCSegmentAddressIndex {
  using Keys = std::vector<char *>;
  using Values = std::vector<AlignedHeapSegment *>;

 public:
  using iterator = Values::iterator;
  using const_iterator = Values::const_iterator;

  /// Create an index with the default capacity.
  GCSegmentAddressIndex() = default;

  /// Create an index with \p cap slots reserved ahead of time.
  GCSegmentAddressIndex(size_t cap);

  /// \return the number of segments held in the index.
  inline size_t size() const;

  /// If \c segment->lowLim() is non-null, adds a mapping from \c
  /// segment->lowLim() to \c segment into the index, overwriting the
  /// entry for the low lim address if it already exists.
  ///
  /// \pre \p segment cannot be \c nullptr.
  void update(AlignedHeapSegment *segment);

  /// Removes entries from the index by key.
  ///
  /// \p first The start iterator of the sequence of lowLim addresses (keys) to
  ///     remove (inclusive).
  ///
  /// \p last The end iterator of the sequence of lowLim addresses (keys) to
  ///     remove (exclusive).
  ///
  /// \pre The seqeuence of addresses to remove must be sorted in ascending
  ///     order.
  ///
  /// \post Any index entries with a starting address in the sequence will be
  ///     removed, and will not appear in future traversals, unless it is
  ///     inserted once again.
  template <typename ForwardIt>
  void remove(ForwardIt first, ForwardIt last);

  /// Finds the segment managed by this index that owns the storage pointed into
  /// by \p ptr, if such a segment exists.  Returns nullptr if such a segment
  /// does not exist.
  AlignedHeapSegment *segmentCovering(const void *ptr) const;

  /// Iterators over the values (aligned heap segments), in order of their
  /// starting address.
  ///
  /// \pre For every entry (lowLim -> segment) in the index,
  ///        segment->lowLim() == lowLim
  iterator begin();
  const_iterator begin() const;

  iterator end();
  const_iterator end() const;

#ifdef HERMES_SLOW_DEBUG
  void checkConsistency() const;
#endif

 private:
  Values::iterator segmentFor(Keys::iterator itk);
  Values::const_iterator segmentFor(Keys::const_iterator itk) const;

  /// The addresses of segments are stored in this vector, in sorted order, as
  /// the keys of the index.
  Keys keys_{};

  /// Pointers to segment structs are stored in this vector, as the values of
  /// the index, meaning that
  ///
  ///     keys_.size() == segments_.size()
  ///
  /// And for every index, i, into keys_ and segments_, this index contains the
  /// mapping
  ///
  ///     keys_[i] --> segments_[i]
  Values segments_{};

  /// Although an implementation using \c std::map would have better asymptotic
  /// complexity, we found that at the sizes of index we expect, the worst-case
  /// performance of the above implementation is noticeably faster.
  ///
  /// Code for a benchmark verifying this observation can be found at P59111115
  /// and results of a run at P59111110.
};

size_t GCSegmentAddressIndex::size() const {
  return keys_.size();
}

template <typename ForwardIt>
void GCSegmentAddressIndex::remove(ForwardIt first, ForwardIt last) {
  assert(
      std::is_sorted(first, last) &&
      "Keys to be removed must be sorted in ascending order");

  // Nothing to remove.
  if (first == last)
    return;

  auto fromk = std::lower_bound(keys_.begin(), keys_.end(), *first);
  auto froms = segmentFor(fromk);

  auto tok = fromk;
  auto tos = froms;

  auto delk = first;

  for (; fromk != keys_.end(); ++fromk, ++froms) {
    while (delk != last && *delk < *fromk)
      ++delk;
    if (delk != last && *delk == *fromk) {
      continue;
    }

    *tok++ = *fromk;
    *tos++ = *froms;
  }

  size_t newSize = std::distance(keys_.begin(), tok);
  keys_.resize(newSize);
  segments_.resize(newSize);
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCSEGMENTADDRESSINDEX_H
