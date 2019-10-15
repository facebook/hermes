/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/Bytecode.h"
#include "hermes/SourceMap/SourceMapGenerator.h"

#include "llvm/ADT/SmallVector.h"

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
      sourceMap, std::move(functionOffsets), cjsModuleOffset_);
}

void BytecodeModule::inlineJumpTables() {
  for (auto &f : this->getFunctionTable()) {
    if (f->getJumpTables().empty())
      continue;

    f->inlineJumpTables();
  }
}

void BytecodeFunction::inlineJumpTables() {
  // pad opcode vector  to make 32bit aligned
  while (opcodes_.size() % sizeof(uint32_t) != 0) {
    opcodes_.push_back(0);
  }

  // copy in the jump tables
  auto jmpTableStart = opcodes_.size();
  opcodes_.resize(opcodes_.size() + jumpTables_.size() * sizeof(uint32_t));
  memcpy(
      opcodes_.data() + jmpTableStart,
      jumpTables_.data(),
      jumpTables_.size() * sizeof(uint32_t));
}
