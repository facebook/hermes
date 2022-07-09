/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/BytecodeStream.h"

using namespace hermes;
using namespace hbc;

// ============================ File ============================
void BytecodeSerializer::serialize(BytecodeModule &BM, const SHA1 &sourceHash) {
  bytecodeModule_ = &BM;
  uint32_t cjsModuleCount = BM.getBytecodeOptions().cjsModulesStaticallyResolved
      ? BM.getCJSModuleTableStatic().size()
      : BM.getCJSModuleTable().size();
  BytecodeFileHeader header{
      MAGIC,
      BYTECODE_VERSION,
      sourceHash,
      fileLength_,
      BM.getGlobalFunctionIndex(),
      BM.getNumFunctions(),
      static_cast<uint32_t>(BM.getStringKinds().size()),
      BM.getIdentifierCount(),
      BM.getStringTableSize(),
      overflowStringEntryCount_,
      BM.getStringStorageSize(),
      static_cast<uint32_t>(BM.getBigIntTable().size()),
      static_cast<uint32_t>(BM.getBigIntStorage().size()),
      static_cast<uint32_t>(BM.getRegExpTable().size()),
      static_cast<uint32_t>(BM.getRegExpStorage().size()),
      BM.getArrayBufferSize(),
      BM.getObjectKeyBufferSize(),
      BM.getObjectValueBufferSize(),
      BM.getSegmentID(),
      cjsModuleCount,
      static_cast<uint32_t>(BM.getFunctionSourceTable().size()),
      debugInfoOffset_,
      BM.getBytecodeOptions()};
  writeBinary(header);
  // Sizes of file and function headers are tuned for good cache line packing.
  // If you reorder the format, try to avoid headers crossing cache lines.
  visitBytecodeSegmentsInOrder(*this);
  serializeFunctionsBytecode(BM);

  for (auto &entry : BM.getFunctionTable()) {
    serializeFunctionInfo(*entry);
  }

  serializeDebugInfo(BM);

  SHA1 fileHash{};
  if (!isLayout_) {
    auto hash = outputHasher_.result();
    assert(hash.size() == sizeof(fileHash) && "Incorrect length of SHA1 hash");
    std::copy(hash.begin(), hash.end(), fileHash.begin());
  }
  // Even in layout mode, we "write" a footer (with an ignored zero hash),
  // so that fileLength_ is set correctly.
  writeBinary(BytecodeFileFooter{fileHash});

  if (isLayout_) {
    finishLayout(BM);
    serialize(BM, sourceHash);
  }
}

void BytecodeSerializer::finishLayout(BytecodeModule &BM) {
  fileLength_ = loc_;
  assert(fileLength_ > 0 && "Empty file after layout");
  isLayout_ = false;
  loc_ = 0;
}

// ========================== Function Table ==========================
void BytecodeSerializer::serializeFunctionTable(BytecodeModule &BM) {
  for (auto &entry : BM.getFunctionTable()) {
    if (options_.stripDebugInfoSection) {
      // Change flag on the actual BF, so it's seen by serializeFunctionInfo.
      entry->mutableFlags().hasDebugInfo = false;
    }
    FunctionHeader header = entry->getHeader();
    writeBinary(SmallFuncHeader(header));
  }
}

// ========================== DebugInfo ==========================
void BytecodeSerializer::serializeDebugInfo(BytecodeModule &BM) {
  pad(BYTECODE_ALIGNMENT);
  const DebugInfo &info = BM.getDebugInfo();
  debugInfoOffset_ = loc_;

  if (options_.stripDebugInfoSection) {
    const DebugInfoHeader empty = {0, 0, 0, 0, 0};
    writeBinary(empty);
    return;
  }

  const llvh::ArrayRef<StringTableEntry> filenameTable =
      info.getFilenameTable();
  const auto filenameStorage = info.getFilenameStorage();
  const DebugInfo::DebugFileRegionList &files = info.viewFiles();
  const StreamVector<uint8_t> &data = info.viewData();
  uint32_t lexOffset = info.lexicalDataOffset();

  DebugInfoHeader header{
      (uint32_t)filenameTable.size(),
      (uint32_t)filenameStorage.size(),
      (uint32_t)files.size(),
      lexOffset,
      (uint32_t)data.size()};
  writeBinary(header);
  writeBinaryArray(filenameTable);
  writeBinaryArray(filenameStorage);
  for (auto &file : files) {
    writeBinary(file);
  }
  writeBinaryArray(data.getData());
}

// ===================== CommonJS Module Table ======================
void BytecodeSerializer::serializeCJSModuleTable(BytecodeModule &BM) {
  pad(BYTECODE_ALIGNMENT);

  for (const auto &it : BM.getCJSModuleTable()) {
    writeBinary(it.first);
    writeBinary(it.second);
  }

  writeBinaryArray(BM.getCJSModuleTableStatic());
}

// ===================== Function Source Table ======================
void BytecodeSerializer::serializeFunctionSourceTable(BytecodeModule &BM) {
  pad(BYTECODE_ALIGNMENT);

  writeBinaryArray(BM.getFunctionSourceTable());
}

// ==================== Exception Handler Table =====================
void BytecodeSerializer::serializeExceptionHandlerTable(BytecodeFunction &BF) {
  if (!BF.hasExceptionHandlers())
    return;

  pad(INFO_ALIGNMENT);
  ExceptionHandlerTableHeader header{BF.getExceptionHandlerCount()};
  writeBinary(header);

  writeBinaryArray(BF.getExceptionHandlers());
}

// ========================= Array Buffer ==========================
void BytecodeSerializer::serializeArrayBuffer(BytecodeModule &BM) {
  writeBinaryArray(BM.getArrayBuffer());
}

void BytecodeSerializer::serializeObjectBuffer(BytecodeModule &BM) {
  auto objectKeyValBufferPair = BM.getObjectBuffer();

  writeBinaryArray(objectKeyValBufferPair.first);
  writeBinaryArray(objectKeyValBufferPair.second);
}

void BytecodeSerializer::serializeDebugOffsets(BytecodeFunction &BF) {
  if (options_.stripDebugInfoSection || !BF.hasDebugInfo()) {
    return;
  }

  pad(INFO_ALIGNMENT);
  auto *offsets = BF.getDebugOffsets();
  writeBinary(*offsets);
}

// ============================ Function ============================
void BytecodeSerializer::serializeFunctionsBytecode(BytecodeModule &BM) {
  // Map from opcodes and jumptables to offsets, used to deduplicate bytecode.
  using DedupKey = llvh::ArrayRef<opcode_atom_t>;
  llvh::DenseMap<DedupKey, uint32_t> bcMap;
  for (auto &entry : BM.getFunctionTable()) {
    if (options_.optimizationEnabled) {
      // If identical bytecode exists, we'll reuse it.
      bool reuse = false;
      if (isLayout_) {
        // Deduplicate the bytecode during layout phase.
        DedupKey key = entry->getOpcodeArray();
        auto pair = bcMap.insert(std::make_pair(key, loc_));
        if (!pair.second) {
          reuse = true;
          entry->setOffset(pair.first->second);
        }
      } else {
        // Cheaply determine whether bytecode was deduplicated.
        assert(entry->getOffset() && "Function lacks offset after layout");
        assert(entry->getOffset() <= loc_ && "Function has too large offset");
        reuse = entry->getOffset() < loc_;
      }
      if (reuse) {
        continue;
      }
    }

    // Set the offset of this function's bytecode.
    if (isLayout_) {
      entry->setOffset(loc_);
    }

    // Serialize opcodes.
    writeBinaryArray(entry->getOpcodesOnly());

    // Serialize any jump table after the opcode block.
    if (!entry->getJumpTablesOnly().empty()) {
      pad(sizeof(uint32_t));
      writeBinaryArray(entry->getJumpTablesOnly());
    }
    if (options_.padFunctionBodiesPercent) {
      size_t size = entry->getOpcodesOnly().size();
      size = (size * options_.padFunctionBodiesPercent) / 100;
      while (size--)
        writeBinary('\0');
      pad(sizeof(uint32_t));
    }
  }
}

void BytecodeSerializer::serializeFunctionInfo(BytecodeFunction &BF) {
  // Set the offset of this function's info. Any subsection that is present is
  // aligned to INFO_ALIGNMENT, so we also align the recorded offset to that.
  if (isLayout_) {
    BF.setInfoOffset(llvh::alignTo(loc_, INFO_ALIGNMENT));
  }

  // Write large header if it doesn't fit in a small.
  FunctionHeader header = BF.getHeader();
  if (SmallFuncHeader(header).flags.overflowed) {
    pad(INFO_ALIGNMENT);
    writeBinary(header);
  }

  // Serialize exception handlers.
  serializeExceptionHandlerTable(BF);

  // Add offset in debug info (if function has debug info).
  serializeDebugOffsets(BF);
}

void BytecodeSerializer::visitFunctionHeaders() {
  pad(BYTECODE_ALIGNMENT);
  serializeFunctionTable(*bytecodeModule_);
}

void BytecodeSerializer::visitStringKinds() {
  pad(BYTECODE_ALIGNMENT);
  writeBinaryArray(bytecodeModule_->getStringKinds());
}

void BytecodeSerializer::visitIdentifierHashes() {
  pad(BYTECODE_ALIGNMENT);
  writeBinaryArray(bytecodeModule_->getIdentifierHashes());
}

void BytecodeSerializer::visitSmallStringTable() {
  pad(BYTECODE_ALIGNMENT);
  uint32_t overflowCount = 0;
  for (auto &entry : bytecodeModule_->getStringTable()) {
    SmallStringTableEntry small(entry, overflowCount);
    writeBinary(small);
    overflowCount += small.isOverflowed();
  }
  overflowStringEntryCount_ = overflowCount;
}

void BytecodeSerializer::visitOverflowStringTable() {
  pad(BYTECODE_ALIGNMENT);
  llvh::SmallVector<OverflowStringTableEntry, 64> overflow;
  for (auto &entry : bytecodeModule_->getStringTable()) {
    SmallStringTableEntry small(entry, overflow.size());
    if (small.isOverflowed()) {
      overflow.emplace_back(entry.getOffset(), entry.getLength());
    }
  }
  writeBinaryArray(llvh::makeArrayRef(overflow));
}

void BytecodeSerializer::visitStringStorage() {
  pad(BYTECODE_ALIGNMENT);
  writeBinaryArray(bytecodeModule_->getStringStorage());
}

void BytecodeSerializer::visitArrayBuffer() {
  pad(BYTECODE_ALIGNMENT);
  serializeArrayBuffer(*bytecodeModule_);
}

void BytecodeSerializer::visitObjectKeyBuffer() {
  pad(BYTECODE_ALIGNMENT);
  auto objectKeyValBufferPair = bytecodeModule_->getObjectBuffer();
  writeBinaryArray(objectKeyValBufferPair.first);
}

void BytecodeSerializer::visitObjectValueBuffer() {
  pad(BYTECODE_ALIGNMENT);
  auto objectKeyValBufferPair = bytecodeModule_->getObjectBuffer();
  writeBinaryArray(objectKeyValBufferPair.second);
}

void BytecodeSerializer::visitBigIntTable() {
  pad(BYTECODE_ALIGNMENT);
  writeBinaryArray(bytecodeModule_->getBigIntTable());
}

void BytecodeSerializer::visitBigIntStorage() {
  pad(BYTECODE_ALIGNMENT);
  writeBinaryArray(bytecodeModule_->getBigIntStorage());
}

void BytecodeSerializer::visitRegExpTable() {
  pad(BYTECODE_ALIGNMENT);
  writeBinaryArray(bytecodeModule_->getRegExpTable());
}

void BytecodeSerializer::visitRegExpStorage() {
  pad(BYTECODE_ALIGNMENT);
  writeBinaryArray(bytecodeModule_->getRegExpStorage());
}

void BytecodeSerializer::visitCJSModuleTable() {
  pad(BYTECODE_ALIGNMENT);
  serializeCJSModuleTable(*bytecodeModule_);
}

void BytecodeSerializer::visitFunctionSourceTable() {
  pad(BYTECODE_ALIGNMENT);
  serializeFunctionSourceTable(*bytecodeModule_);
}
