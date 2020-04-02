/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/GCSegmentAddressIndex.h"

#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/PointerBase-inline.h"

#include <algorithm>
#include <iterator>

namespace hermes {
namespace vm {

GCSegmentAddressIndex::GCSegmentAddressIndex(size_t cap)
    : GCSegmentAddressIndex() {
  keys_.reserve(cap);
  segments_.reserve(cap);
}

void GCSegmentAddressIndex::update(AlignedHeapSegment *segment) {
  assert(segment != nullptr);
  char *const key = segment->lowLim();
  // If the low lim is null, ignore.
  if (key == nullptr) {
    return;
  }

  auto itk = std::lower_bound(keys_.begin(), keys_.end(), key);
  auto its = segmentFor(itk);

  if (itk != keys_.end() && key == *itk) {
    *its = segment;
    return;
  }

  keys_.insert(itk, key);
  segments_.insert(its, segment);
}

AlignedHeapSegment *GCSegmentAddressIndex::segmentCovering(
    const void *ptr) const {
  auto key = AlignedStorage::start(ptr);
  auto itk = std::lower_bound(keys_.begin(), keys_.end(), key);

  if (itk == keys_.end() || key != *itk) {
    return nullptr;
  }

  return *segmentFor(itk);
}

GCSegmentAddressIndex::iterator GCSegmentAddressIndex::begin() {
#ifdef HERMES_SLOW_DEBUG
  checkConsistency();
#endif
  return segments_.begin();
}

GCSegmentAddressIndex::const_iterator GCSegmentAddressIndex::begin() const {
#ifdef HERMES_SLOW_DEBUG
  checkConsistency();
#endif
  return segments_.begin();
}

GCSegmentAddressIndex::iterator GCSegmentAddressIndex::end() {
  return segments_.end();
}

GCSegmentAddressIndex::const_iterator GCSegmentAddressIndex::end() const {
  return segments_.end();
}

GCSegmentAddressIndex::Values::iterator GCSegmentAddressIndex::segmentFor(
    Keys::iterator itk) {
  return segments_.begin() + std::distance(keys_.begin(), itk);
}

GCSegmentAddressIndex::Values::const_iterator GCSegmentAddressIndex::segmentFor(
    Keys::const_iterator itk) const {
  return segments_.begin() + std::distance(keys_.begin(), itk);
}

#ifdef HERMES_SLOW_DEBUG
void GCSegmentAddressIndex::checkConsistency() const {
  assert(
      keys_.size() == segments_.size() &&
      "There must be the same number of keys as values");

  auto itk = keys_.begin();
  auto its = segments_.begin();
  for (; itk != keys_.end(); ++itk, ++its) {
    assert(
        *itk == (*its)->lowLim() && "Index out of sync with segment addresses");
  }
}
#endif // HERMES_SLOW_DEBUG

} // namespace vm
} // namespace hermes
