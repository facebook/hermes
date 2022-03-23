/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "NodeBytecode.h"
#include "hermes/BCGen/HBC/BytecodeFileFormat.h"

/// Returns the bytecode for the builtin JS files
llvh::ArrayRef<uint8_t> getNodeBytecode() {
  // Bytecode is required to be aligned, so ensure we don't fail to load it
  // at runtime.
  alignas(
      hermes::hbc::BYTECODE_ALIGNMENT) static const uint8_t NodeBytecode[] = {
#ifdef HERMES_CMAKE_BUILD
#include "NodeBytecode.inc"
#else
#include "hermes/NodeBytecode/NodeBytecode.inc"
#endif
  };
  return llvh::makeArrayRef(NodeBytecode, sizeof(NodeBytecode));
}
