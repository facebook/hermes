/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/FlowLib/FlowLib.h"

namespace hermes {

static const unsigned char flowLibSource[] = {
#include "FlowLib.inc"
    // Add null terminator for parser.
    ,
    0x00};

llvh::StringRef getFlowLibSource() {
  // Return size without the null terminator.
  return llvh::StringRef(
      reinterpret_cast<const char *>(flowLibSource), sizeof(flowLibSource) - 1);
}

} // namespace hermes
