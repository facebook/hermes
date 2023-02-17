/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SEGMENTINFO_H
#define HERMES_VM_SEGMENTINFO_H

#include "hermes/VM/AlignedStorage.h"
#include "hermes/VM/sh_runtime.h"

namespace hermes {
namespace vm {

/// The very beginning of a segment must contain this small structure, which can
/// contain segment-specific information. See AlignedHeapSegment for details on
/// how it is stored.
struct SegmentInfo : private SHSegmentInfo {
  /// Returns the index of the segment containing \p lowLim, which is required
  /// to be the start of its containing segment.  (This can allow extra
  /// efficiency, in cases where the segment start has already been computed.)
  static unsigned segmentIndexFromStart(const void *lowLim) {
    assert(lowLim == AlignedStorage::start(lowLim) && "Precondition.");
    return get(lowLim)->index;
  }

  /// Returns the index of the segment containing \p ptr.
  static unsigned segmentIndex(const void *ptr) {
    return segmentIndexFromStart(AlignedStorage::start(ptr));
  }

  /// Requires that \p lowLim is the start address of a segment, and sets
  /// that segment's index to \p index.
  static void setSegmentIndexFromStart(void *lowLim, unsigned index) {
    assert(lowLim == AlignedStorage::start(lowLim) && "Precondition.");
    get(lowLim)->index = index;
  }

 private:
  /// Given the \p lowLim of some valid AlignedStorage's memory region, returns
  /// a pointer to the SegmentInfo laid out in that storage,
  /// assuming it exists.
  static SegmentInfo *get(void *lowLim) {
    assert(lowLim == AlignedStorage::start(lowLim) && "Precondition.");
    return reinterpret_cast<SegmentInfo *>(lowLim);
  }

  static const SegmentInfo *get(const void *lowLim) {
    assert(lowLim == AlignedStorage::start(lowLim) && "Precondition.");
    return reinterpret_cast<const SegmentInfo *>(lowLim);
  }
};

} // namespace vm
} // namespace hermes

#endif
