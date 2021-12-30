/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/StringTable.h"
#include "llvh/Support/raw_ostream.h"

namespace hermes {

llvh::raw_ostream &operator<<(llvh::raw_ostream &os, Identifier id) {
  return os << id.str();
}

} // namespace hermes
