/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/VM/GC.h"
#include "hermes/VM/GCCell.h"

namespace hermes::vm {
size_t GCCell::getAllocatedSizeSlow() const {
  uint32_t sz = kindAndSize_.getSize();
  // If KindAndSize is configured to use 32bits for size, we can always go
  // the fast path.
  if constexpr (
      KindAndSize::maxSize() >= std::numeric_limits<gcheapsize_t>::max())
    return sz;

  // For GCCell with size smaller than maxNormalSize(), we simply return
  // the size as saved in kindAndSize_.
  if (LLVM_LIKELY(sz))
    return sz;
  // If sz == 0, query the size from the GC.
  return GC::getLargeCellSize(this);
}
} // namespace hermes::vm
