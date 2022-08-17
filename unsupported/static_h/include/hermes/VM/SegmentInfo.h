/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SEGMENTINFO_H
#define HERMES_VM_SEGMENTINFO_H

#include "hermes/VM/AlignedStorage.h"

/// Define this macro here because SegmentInfo is necessarily a dependency of
/// anything using compressed pointers.
#if defined(HERMESVM_ALLOW_COMPRESSED_POINTERS) && LLVM_PTR_SIZE == 8 && \
    (defined(HERMESVM_GC_HADES) || defined(HERMESVM_GC_RUNTIME))
/// \macro HERMESVM_COMPRESSED_POINTERS
/// \brief If defined, store pointers as 32 bits in GC-managed Hermes objects.
#define HERMESVM_COMPRESSED_POINTERS
#ifdef HERMESVM_ALLOW_CONTIGUOUS_HEAP
#define HERMESVM_CONTIGUOUS_HEAP
#endif
#endif

namespace hermes {
namespace vm {

/// The very beginning of a segment must contain this small structure, which can
/// contain segment-specific information. See AlignedHeapSegment for details on
/// how it is stored.
struct SegmentInfo {
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

  unsigned index;
};

} // namespace vm
} // namespace hermes

#endif
