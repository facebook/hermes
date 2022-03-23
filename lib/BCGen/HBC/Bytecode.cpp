/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/Bytecode.h"
#include "hermes/SourceMap/SourceMapGenerator.h"

#include "llvh/ADT/SmallVector.h"

using namespace hermes;
using namespace hbc;

void BytecodeModule::setFunction(
    uint32_t index,
    std::unique_ptr<BytecodeFunction> F) {
  assert(index < getNumFunctions() && "Function ID out of bound");
  functions_[index] = std::move(F);
}

BytecodeFunction &BytecodeModule::getFunction(unsigned index) {
  assert(index < getNumFunctions() && "Function ID out of bound");
  assert(functions_[index] && "Invalid function");
  return *functions_[index];
}

void BytecodeModule::populateSourceMap(SourceMapGenerator *sourceMap) const {
  /// Construct a list of virtual function offsets, and pass it to DebugInfo to
  /// help it populate the source map.
  std::vector<uint32_t> functionOffsets;
  functionOffsets.reserve(functions_.size());
  uint32_t offset = 0;
  for (const auto &func : functions_) {
    functionOffsets.push_back(offset);
    offset += func->getHeader().bytecodeSizeInBytes;
  }
  debugInfo_.populateSourceMap(
      sourceMap, std::move(functionOffsets), segmentID_);
}

ArrayRef<uint32_t> BytecodeFunction::getJumpTablesOnly() const {
  // The jump tables (if there are any) start at the nearest 4-byte boundary
  // from the end of the opcodes.
  uint32_t jumpTableStartIdx =
      llvh::alignTo<sizeof(uint32_t)>(header_.bytecodeSizeInBytes);

  if (jumpTableStartIdx > opcodesAndJumpTables_.size()) {
    return {};
  }

  uint32_t jumpTableBytes = opcodesAndJumpTables_.size() - jumpTableStartIdx;
  assert(jumpTableBytes % sizeof(uint32_t) == 0);
  uint32_t jumpTableSize = jumpTableBytes / sizeof(uint32_t);
  const uint32_t *jumpTableStart = reinterpret_cast<const uint32_t *>(
      opcodesAndJumpTables_.data() + jumpTableStartIdx);

  return {jumpTableStart, jumpTableSize};
}
