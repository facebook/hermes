/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/InternalBytecode/InternalBytecode.h"

namespace hermes {
namespace vm {

llvm::ArrayRef<uint8_t> getInternalBytecode() {
#ifdef HERMESVM_USE_JS_LIBRARY_IMPLEMENTATION
  static const uint8_t InternalBytecode[] = {
#ifdef HERMES_CMAKE_BUILD
#include "InternalBytecode.inc"
#else
#include "hermes/InternalBytecode/InternalBytecode.inc"
#endif
  };

  return llvm::makeArrayRef(InternalBytecode, sizeof(InternalBytecode));
#else
  return llvm::ArrayRef<uint8_t>{};
#endif
}
} // namespace vm
} // namespace hermes
