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
  /// Segments have sizes that are multiples of
  /// 1<<HERMESVM_LOG_HEAP_SEGMENT_SIZE, so we only need to store the number of
  /// SEGMENT_SIZEs after a right shift. A two-byte integer is large enough to
  /// hold the size of a 4GB segment.
  unsigned short shiftedSegmentSize;
} SHSegmentInfo;

#endif
