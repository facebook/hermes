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
  // Every segment is aligned to 1<<HERMESVM_LOG_HEAP_SEGMENT_SIZE, so we only
  // need to store the bits after a right shift. A two bytes integer is large
  // enough to hold a 4GB segment.
  unsigned short shiftedSegmentSize;
} SHSegmentInfo;

#endif
