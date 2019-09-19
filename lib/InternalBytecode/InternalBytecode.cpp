/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/InternalBytecode/InternalBytecode.h"

namespace hermes {
namespace vm {

const uint8_t InternalBytecode[] = {
#ifdef HERMES_CMAKE_BUILD
#include "InternalBytecode.inc"
#else
#include "hermes/lib/InternalBytecode/InternalBytecode.inc"
#endif
};

llvm::ArrayRef<uint8_t> getInternalBytecode() {
  return llvm::makeArrayRef(InternalBytecode, sizeof(InternalBytecode));
}
} // namespace vm
} // namespace hermes
