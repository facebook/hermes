/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SH_SEGMENT_INFO_H
#define HERMES_SH_SEGMENT_INFO_H

/// The very beginning of a segment must contain this small structure, which can
/// contain segment-specific information.
typedef struct SHSegmentInfo {
  unsigned index;
  /// The storage size for this segment. We practically don't support segment
  /// with size larger than UINT32_MAX.
  unsigned segmentSize;
  /// Pointer that points to the CardStatus array for this segment.
  /// Erase the actual type AtomicIfConcurrent<CardStatus> here to avoid using
  /// C++ type and forward declaring nested type.
  void *cards;
  /// Pointer that points to the boundary array for this segment.
  int8_t *boundaries;
} SHSegmentInfo;

#endif
