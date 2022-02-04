/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// Split opcode names into a separate file to avoid linking if not used.
//===----------------------------------------------------------------------===//

#include "hermes/Inst/InstDecode.h"

namespace hermes {
namespace inst {

llvh::StringRef getOpCodeString(OpCode opCode) {
  assert(opCode < OpCode::_last && "invalid OpCode");

  static const char *opCodeStrings[] = {
#define DEFINE_OPCODE(name) #name,
#include "hermes/BCGen/HBC/BytecodeList.def"
  };

  return opCodeStrings[(int)opCode];
};

} // namespace inst
} // namespace hermes
