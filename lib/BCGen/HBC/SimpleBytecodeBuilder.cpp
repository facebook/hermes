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

/// Append an array of T to the bytecode buffer.
template <typename T>
static void appendArrayToBytecode(
    std::vector<uint8_t> &bytecode,
    llvh::ArrayRef<T> data) {
  bytecode.insert(
      bytecode.end(),
      reinterpret_cast<const uint8_t *>(data.begin()),
      reinterpret_cast<const uint8_t *>(data.end()));
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
  if (debugInfo_) {
    // Account for the full FunctionHeaders when there's DebugInfo.
    for (uint32_t i = 0; i < functionCount; ++i) {
      currentSize = llvh::alignTo(currentSize, 4);
      functions_[i].infoOffset = currentSize;
      currentSize += sizeof(FunctionHeader);
      currentSize = llvh::alignTo(currentSize, 4);
      currentSize += sizeof(DebugOffsets);
    }
  }
  // DebugInfo comes after the bytescodes, padded by 4 bytes.
  uint32_t debugOffset = llvh::alignTo(currentSize, 4);
  uint32_t totalSize =
      debugOffset + sizeof(DebugInfoHeader) + sizeof(BytecodeFileFooter);
  if (debugInfo_) {
    // Account for the DebugInfo.
    totalSize +=
        debugInfo_->getFilenameTable().size() * sizeof(StringTableEntry);
    totalSize += debugInfo_->getFilenameStorage().size();
    totalSize += debugInfo_->viewFiles().size() * sizeof(DebugFileRegion);
    totalSize += debugInfo_->viewData().getData().size();
  }
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
  if (debugInfo_) {
    // When there's DebugInfo, use large FunctionHeaders which are appended
    // after the opcodes.
    // Write the offset to the FunctionHeaders using SmallFuncHeader here.
    for (uint32_t i = 0; i < functionCount; ++i) {
      SmallFuncHeader small(functions_[i].infoOffset);
      assert(small.flags.getOverflowed());
      appendStructToBytecode(bytecode, small);
    }
  } else {
    for (uint32_t i = 0; i < functionCount; ++i) {
      uint32_t opcodeSize = functions_[i].opcodes.size();
      FunctionHeader funcHeader{
          opcodeSize,
          functions_[i].paramCount,
          /* loopDepth */ 0,
          functions_[i].frameSize,
          /* numberRegCount */ 0,
          /* nonPtrRegCount */ 0,
          0,
          functions_[i].highestReadCacheIndex,
          functions_[i].highestWriteCacheIndex,
          /* numCacheNewObject */ 0,
          functions_[i].highestPrivateNameCacheIndex};
      funcHeader.setOffset(functions_[i].offset);
      funcHeader.flags.setStrictMode(true);
      SmallFuncHeader small(funcHeader);
      assert(!small.flags.getOverflowed());
      appendStructToBytecode(bytecode, small);
    }
  }

  // Write all opcodes to the buffer.
  for (uint32_t i = 0; i < functionCount; ++i) {
    bytecode.insert(
        bytecode.end(),
        functions_[i].opcodes.begin(),
        functions_[i].opcodes.end());
  }

  if (debugInfo_) {
    // Write all full FunctionHeaders to the buffer when necessary.
    // Pad by 4 bytes.
    bytecode.resize(llvh::alignTo(bytecode.size(), 4));
    for (uint32_t i = 0; i < functionCount; ++i) {
      assert(
          functions_[i].infoOffset == bytecode.size() &&
          "Function offset mismatch");
      uint32_t opcodeSize = functions_[i].opcodes.size();
      FunctionHeader funcHeader{
          opcodeSize,
          functions_[i].paramCount,
          /* loopDepth */ 0,
          functions_[i].frameSize,
          /* numberRegCount */ 0,
          /* nonPtrRegCount */ 0,
          0,
          functions_[i].highestReadCacheIndex,
          functions_[i].highestWriteCacheIndex,
          /* numCacheNewObject */ 0,
          functions_[i].highestPrivateNameCacheIndex};
      funcHeader.setOffset(functions_[i].offset);
      funcHeader.flags.setStrictMode(true);
      funcHeader.flags.setHasDebugInfo(true);
      DebugOffsets offsets{0, 0};
      bytecode.resize(llvh::alignTo(bytecode.size(), 4));
      appendStructToBytecode(bytecode, funcHeader);
      bytecode.resize(llvh::alignTo(bytecode.size(), 4));
      appendStructToBytecode(bytecode, offsets);
    }
  }

  // Write the debug info to the buffer.
  // Pad by 4 bytes.
  bytecode.resize(llvh::alignTo(bytecode.size(), 4));
  if (debugInfo_) {
    const llvh::ArrayRef<StringTableEntry> filenameTable =
        debugInfo_->getFilenameTable();
    const auto filenameStorage = debugInfo_->getFilenameStorage();
    const DebugInfo::DebugFileRegionList &files = debugInfo_->viewFiles();
    const StreamVector<uint8_t> &data = debugInfo_->viewData();
    assert(debugOffset == bytecode.size() && "Debug offset mismatch");
    uint32_t lexOffset = data.size();
    DebugInfoHeader header{
        (uint32_t)filenameTable.size(),
        (uint32_t)filenameStorage.size(),
        (uint32_t)files.size(),
        lexOffset,
        (uint32_t)data.size()};
    appendStructToBytecode(bytecode, header);
    appendArrayToBytecode(bytecode, filenameTable);
    appendArrayToBytecode(bytecode, filenameStorage);
    for (auto &file : files) {
      appendStructToBytecode(bytecode, file);
    }
    appendArrayToBytecode(bytecode, data.getData());
  } else {
    // Write an empty debug info header.
    DebugInfoHeader debugInfoHeader{0, 0, 0, 0, 0};
    appendStructToBytecode(bytecode, debugInfoHeader);
  }
  assert(
      totalSize == (bytecode.size() + sizeof(BytecodeFileFooter)) &&
      "Bytecode size mismatch");
  appendStructToBytecode(
      bytecode, BytecodeFileFooter{llvh::SHA1::hash(bytecode)});
  // Generate the buffer.
  return std::unique_ptr<Buffer>(new VectorBuffer(std::move(bytecode)));
}
