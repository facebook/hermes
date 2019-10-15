/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/OldGenSegmentRanges.h"
#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/OldGenNC.h"

namespace hermes {
namespace vm {

AlignedHeapSegment *OldGenFilledSegmentRange::next() {
  if (LLVM_LIKELY(cursor_ < gen_->filledSegments_.size())) {
    return &gen_->filledSegments_[cursor_++];
  }

  return nullptr;
}

AlignedHeapSegment *OldGenMaterializingRange::next() {
  if (LLVM_LIKELY(gen_->materializeNextSegment())) {
    return &gen_->activeSegment();
  }

  return nullptr;
}

} // namespace vm
} // namespace hermes
