/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/InternalBytecode/InternalBytecode.h"

namespace hermes {
namespace vm {

llvh::ArrayRef<uint8_t> getInternalBytecode() {
#ifdef HERMESVM_USE_JS_LIBRARY_IMPLEMENTATION
  static const uint8_t InternalBytecode[] = {
#ifdef HERMES_CMAKE_BUILD
#include "InternalBytecode.inc"
#else
#include "hermes/InternalBytecode/InternalBytecode.inc"
#endif
  };

  return llvh::makeArrayRef(InternalBytecode, sizeof(InternalBytecode));
#else
  return llvh::ArrayRef<uint8_t>{};
#endif
}
} // namespace vm
} // namespace hermes
