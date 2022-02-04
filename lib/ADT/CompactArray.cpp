/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/ADT/CompactArray.h"

namespace hermes {

void CompactArray::scaleUp() {
  assert(scale_ < UINT32 && "cannot go above 32 bits");
  CompactArray newTable(size_, (Scale)(scale_ + 1));
  for (uint32_t idx = 0; idx < size_; ++idx) {
    bool success = newTable.trySet(idx, get(idx));
    assert(success && "value no longer fits");
    (void)success;
  }
  swap(newTable);
}

} // namespace hermes
