/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ExtensionsBytecode.h"

#include "hermes/BCGen/HBC/BytecodeFileFormat.h"

namespace facebook {
namespace hermes {

llvh::ArrayRef<uint8_t> getExtensionsBytecode() {
  // Bytecode is required to be aligned, so ensure we don't fail to load it
  // at runtime.
  alignas(::hermes::hbc::BYTECODE_ALIGNMENT) static const uint8_t
      ExtensionsBytecode[] = {
#include "ExtensionsBytecode.inc"
      };

  return llvh::makeArrayRef(ExtensionsBytecode, sizeof(ExtensionsBytecode));
}

} // namespace hermes
} // namespace facebook
