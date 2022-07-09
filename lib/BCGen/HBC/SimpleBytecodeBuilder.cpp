/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/SimpleBytecodeBuilder.h"

#include "llvh/Support/MathExtras.h"
#include "llvh/Support/SHA1.h"

using namespace hermes;
using namespace hbc;

/// An implementation of Buffer through a std::vector of bytes.
class VectorBuffer final : public Buffer {
  std::vector<uint8_t> bytecode_;

 public:
  VectorBuffer(std::vector<uint8_t> &&bytecode)
      : bytecode_(std::move(bytecode)) {
    data_ = bytecode_.data();
    size_ = bytecode_.size();
  }
};

template <typename T>
void appendStructToBytecode(std::vector<uint8_t> &bytecode, const T &data) {
  assert((bytecode.size() % alignof(T) == 0) && "Bytecode is not aligned");
  bytecode.insert(
      bytecode.end(),
      reinterpret_cast<const uint8_t *>(&data),
      reinterpret_cast<const uint8_t *>((&data) + 1));
}

std::unique_ptr<Buffer> SimpleBytecodeBuilder::generateBytecodeBuffer() {
  // TODO: There is a fair amount of logic duplication between this function
  // and the BytecodeSerializer. We should consider merging some of them.
  std::vector<uint8_t> bytecode;
  // First do a dry-run to compute the bytecode offset of each function.
  uint32_t functionCount = functions_.size();
  uint32_t currentSize =
      sizeof(BytecodeFileHeader) + sizeof(SmallFuncHeader) * functionCount;
  for (uint32_t i = 0; i < functionCount; ++i) {
    // Populate the offset of the function's bytecode.
    functions_[i].offset = currentSize;
    currentSize += functions_[i].opcodes.size();
  }
  // DebugInfo comes after the bytescodes, padded by 4 bytes.
  uint32_t debugOffset = llvh::alignTo(currentSize, 4);
  uint32_t totalSize =
      debugOffset + sizeof(DebugInfoHeader) + sizeof(BytecodeFileFooter);
  BytecodeOptions options;
  BytecodeFileHeader header{
      MAGIC,
      BYTECODE_VERSION,
      // There is no source for a bytecode builder, so
      // leave it as zeros.
      SHA1{},
      totalSize,
      0,
      functionCount,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      debugOffset,
      options};
  // Write BytecodeFileHeader to the buffer.
  appendStructToBytecode(bytecode, header);
  // Write all function headers to the buffer.
  for (uint32_t i = 0; i < functionCount; ++i) {
    uint32_t opcodeSize = functions_[i].opcodes.size();
    FunctionHeader funcHeader{
        opcodeSize,
        functions_[i].paramCount,
        functions_[i].frameSize,
        0,
        0,
        0,
        0};
    funcHeader.offset = functions_[i].offset;
    funcHeader.flags.strictMode = true;
    SmallFuncHeader small(funcHeader);
    assert(!small.flags.overflowed);
    appendStructToBytecode(bytecode, small);
  }
  // Write all opcodes to the buffer.
  for (uint32_t i = 0; i < functionCount; ++i) {
    bytecode.insert(
        bytecode.end(),
        functions_[i].opcodes.begin(),
        functions_[i].opcodes.end());
  }
  // Pad by 4 bytes.
  bytecode.resize(llvh::alignTo(bytecode.size(), 4));
  // Write an empty debug info header.
  DebugInfoHeader debugInfoHeader{0, 0, 0, 0, 0};
  appendStructToBytecode(bytecode, debugInfoHeader);
  // Add the bytecode hash.
  appendStructToBytecode(
      bytecode, BytecodeFileFooter{llvh::SHA1::hash(bytecode)});
  // Generate the buffer.
  return std::unique_ptr<Buffer>(new VectorBuffer(std::move(bytecode)));
}
