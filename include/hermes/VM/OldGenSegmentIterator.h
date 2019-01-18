/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_OLDGENSEGMENTITERATOR_H
#define HERMES_VM_OLDGENSEGMENTITERATOR_H

#include <iterator>

namespace hermes {
namespace vm {

class OldGen;
class AlignedHeapSegment;

/// The type of iterators over segments in the old generation.
class OldGenSegmentIterator
    : std::iterator<std::forward_iterator_tag, AlignedHeapSegment, ssize_t> {
  inline friend difference_type distance(
      const OldGenSegmentIterator &first,
      const OldGenSegmentIterator &last);

 public:
  inline OldGenSegmentIterator &operator++();
  inline OldGenSegmentIterator operator++(int);
  inline OldGenSegmentIterator operator+(size_t diff) const;
  inline bool operator==(const OldGenSegmentIterator &that) const;
  inline bool operator!=(const OldGenSegmentIterator &that) const;
  inline pointer operator->() const;
  reference operator*() const;

 private:
  friend class OldGen;
  inline OldGenSegmentIterator(OldGen *oldGen, size_t ix);

  size_t ix_;
  OldGen *oldGen_;
};

OldGenSegmentIterator::difference_type distance(
    const OldGenSegmentIterator &first,
    const OldGenSegmentIterator &last) {
  return last.ix_ - first.ix_;
}

OldGenSegmentIterator &OldGenSegmentIterator::operator++() {
  ix_++;
  return *this;
}

OldGenSegmentIterator OldGenSegmentIterator::operator++(int) {
  auto cpy = *this;
  ++(*this);
  return cpy;
}

OldGenSegmentIterator OldGenSegmentIterator::operator+(size_t diff) const {
  return {oldGen_, ix_ + diff};
}

bool OldGenSegmentIterator::operator==(
    const OldGenSegmentIterator &that) const {
  return oldGen_ == that.oldGen_ && ix_ == that.ix_;
}

bool OldGenSegmentIterator::operator!=(
    const OldGenSegmentIterator &that) const {
  return !(*this == that);
}

OldGenSegmentIterator::pointer OldGenSegmentIterator::operator->() const {
  return &**this;
}

OldGenSegmentIterator::OldGenSegmentIterator(OldGen *oldGen, size_t ix)
    : ix_(ix), oldGen_(oldGen) {}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_OLDGENSEGMENTITERATOR_H
