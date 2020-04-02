/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_OLDGENSEGMENTRANGES_H
#define HERMES_VM_OLDGENSEGMENTRANGES_H

#include "hermes/VM/GCSegmentRange.h"

#include <cstddef>

namespace hermes {
namespace vm {

class AlignedHeapSegment;
class OldGen;

/// A class conforming to the interface described in the docs for
/// GCSegmentRange which ranges over the OldGen's filled, inactive segments, in
/// the order they were filled in.
class OldGenFilledSegmentRange : public GCSegmentRange {
 public:
  /// Factory function creating an instance of this implementation, owned by a
  /// unique_ptr.
  static inline GCSegmentRange::Ptr create(OldGen *gen, size_t start = 0);

  inline OldGenFilledSegmentRange(OldGen *gen, size_t start);

  AlignedHeapSegment *next() override;

 private:
  OldGen *gen_;
  size_t cursor_;
};

/// A class conforming to the interface described in the docs for
/// GCSegmentRange which ranges over the segments backing the OldGen's current
/// size that are as-yet unmaterialized.  This range materializes them
/// on-demand, failing if materialisation fails.
class OldGenMaterializingRange : public GCSegmentRange {
 public:
  /// Factory function creating an instance of this implementation, owned by a
  /// unique_ptr.
  static inline GCSegmentRange::Ptr create(OldGen *gen);

  inline OldGenMaterializingRange(OldGen *gen);

  AlignedHeapSegment *next() override;

 private:
  OldGen *gen_;
};

/* static */
inline GCSegmentRange::Ptr OldGenFilledSegmentRange::create(
    OldGen *gen,
    size_t start) {
  return std::unique_ptr<OldGenFilledSegmentRange>(
      new OldGenFilledSegmentRange{gen, start});
}

inline OldGenFilledSegmentRange::OldGenFilledSegmentRange(
    OldGen *gen,
    size_t start)
    : gen_{gen}, cursor_{start} {}

inline GCSegmentRange::Ptr OldGenMaterializingRange::create(OldGen *gen) {
  return std::unique_ptr<OldGenMaterializingRange>(
      new OldGenMaterializingRange{gen});
}

inline OldGenMaterializingRange::OldGenMaterializingRange(OldGen *gen)
    : gen_{gen} {}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_OLDGENSEGMENTRANGES_H
