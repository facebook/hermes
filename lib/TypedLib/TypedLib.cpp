/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/TypedLib/TypedLib.h"

namespace hermes {

static const unsigned char typedLibSource[] = {
#include "TypedLib.inc"
    // Add null terminator for parser.
    ,
    0x00};

llvh::StringRef getTypedLibSource() {
  // Return size without the null terminator.
  return llvh::StringRef(
      reinterpret_cast<const char *>(typedLibSource),
      sizeof(typedLibSource) - 1);
}

} // namespace hermes
