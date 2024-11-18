/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/BytecodeStream.h"

#include "hermes/BCGen/Exceptions.h"
#include "hermes/BCGen/HBC/Bytecode.h"
#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "hermes/BCGen/HBC/DebugInfo.h"
#include "hermes/BCGen/HBC/StreamVector.h"
#include "hermes/Support/Buffer.h"

namespace hermes {
namespace hbc {

namespace {

class BytecodeSerializer {
  friend void hermes::hbc::visitBytecodeSegmentsInOrder<BytecodeSerializer>(
      BytecodeSerializer &);
  /// Output Stream
  llvh::raw_ostream &os_;
  // Module being serialized.
  BytecodeModule *bytecodeModule_;
  /// Options controlling bytecode generation.
  BytecodeGenerationOptions options_;
  /// Current output offset.
  size_t loc_{0};
  /// Wheather we are doing a layout run.
  bool isLayout_{true};
  /// Total file length in bytes.
  uint32_t fileLength_{0};
  /// Offset of the debug info tables.
  uint32_t debugInfoOffset_{0};
  /// Count of overflow string entries, computed during layout phase.
  uint32_t overflowStringEntryCount_{0};
  /// Hash of everything written in non-layout mode so far.
  llvh::SHA1 outputHasher_;

  /// Each subsection of a function's `info' section is aligned thusly.
  static constexpr uint32_t INFO_ALIGNMENT = 4;

  template <typename T>
  void writeBinaryArray(const llvh::ArrayRef<T> array) {
    size_t size = sizeof(T) * array.size();
    if (!isLayout_) {
      outputHasher_.update(llvh::ArrayRef<uint8_t>(
          reinterpret_cast<const uint8_t *>(array.data()), size));
      os_.write(reinterpret_cast<const char *>(array.data()), size);
    }
    loc_ += size;
  }

  template <typename T>
  void writeBinary(const T &structure) {
    return writeBinaryArray(llvh::ArrayRef<T>{&structure, 1});
  }

  /// Padding the binary according to the \p alignment.
  void pad(unsigned alignment) {
    // Support alignment as many as 8 bytes.
    assert(
        alignment > 0 && alignment <= 8 &&
        ((alignment & (alignment - 1)) == 0));
    if (loc_ % alignment == 0)
      return;
    unsigned bytes = alignment - loc_ % alignment;
    for (unsigned i = 0; i < bytes; ++i) {
      writeBinary('\0');
    }
  }

  void serializeFunctionTable(BytecodeModule &BM);

  void serializeCJSModuleTable(BytecodeModule &BM);

  void serializeFunctionSourceTable(BytecodeModule &BM);

  void serializeDebugInfo(BytecodeModule &BM);

  void serializeExceptionHandlerTable(BytecodeFunction &BF);

  void serializeDebugOffsets(BytecodeFunction &BF);

  void serializeFunctionsBytecode(BytecodeModule &BM);
  void serializeFunctionInfo(BytecodeFunction &BF);

  void finishLayout(BytecodeModule &BM);

  void visitFunctionHeaders();
  void visitStringKinds();
  void visitIdentifierHashes();
  void visitSmallStringTable();
  void visitOverflowStringTable();
  void visitStringStorage();
  void visitLiteralValueBuffer();
  void visitObjectKeyBuffer();
  void visitObjectShapeTable();
  void visitBigIntTable();
  void visitBigIntStorage();
  void visitRegExpTable();
  void visitRegExpStorage();
  void visitCJSModuleTable();
  void visitFunctionSourceTable();

 public:
  explicit BytecodeSerializer(
      llvh::raw_ostream &OS,
      BytecodeGenerationOptions options = BytecodeGenerationOptions::defaults())
      : os_(OS), options_(options) {}

  void serialize(BytecodeModule &BM, const SHA1 &sourceHash);
};

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
      BM.getLiteralValueBufferSize(),
      BM.getObjectKeyBufferSize(),
      static_cast<uint32_t>(BM.getObjectShapeTable().size()),
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

    // If an infoOffset has been set, create a header with the offset. Note that
    // since the infoOffset is set by serializeFunctionInfo, this may differ
    // between the layout and final runs, but this is fine because it is the
    // same size.
    if (isLayout_ || entry->getInfoOffset())
      writeBinary(SmallFuncHeader(entry->getInfoOffset()));
    else
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
  uint32_t lexOffset = data.size();

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
  if (!BF.getHeader().flags.hasExceptionHandler)
    return;

  pad(INFO_ALIGNMENT);
  ExceptionHandlerTableHeader header{BF.getExceptionHandlerCount()};
  writeBinary(header);

  writeBinaryArray(BF.getExceptionHandlers());
}

// ========================= Buffers ==========================
void BytecodeSerializer::serializeDebugOffsets(BytecodeFunction &BF) {
  if (!BF.getHeader().flags.hasDebugInfo) {
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
        auto pair =
            bcMap.insert(std::make_pair(key, static_cast<uint32_t>(loc_)));
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
  FunctionHeader header = BF.getHeader();
  // We only need function info if the function has exceptions, debug info, or
  // its header does not fit in a small header. Otherwise, we can skip it.
  if (!header.flags.hasExceptionHandler && !header.flags.hasDebugInfo &&
      SmallFuncHeader::canFitInSmallHeader(header))
    return;

  // Set the offset of this function's info. Any subsection that is present is
  // aligned to INFO_ALIGNMENT, so we also align the recorded offset to that.
  auto infoOffset = llvh::alignTo(loc_, INFO_ALIGNMENT);
  assert((isLayout_ || infoOffset == BF.getInfoOffset()) && "Offset mismatch");
  BF.setInfoOffset(infoOffset);

  // Write large header if it doesn't fit in a small.

  pad(INFO_ALIGNMENT);
  writeBinary(header);

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

void BytecodeSerializer::visitLiteralValueBuffer() {
  pad(BYTECODE_ALIGNMENT);
  writeBinaryArray(bytecodeModule_->getLiteralValueBuffer());
}

void BytecodeSerializer::visitObjectKeyBuffer() {
  pad(BYTECODE_ALIGNMENT);
  writeBinaryArray(bytecodeModule_->getObjectKeyBuffer());
}

void BytecodeSerializer::visitObjectShapeTable() {
  pad(BYTECODE_ALIGNMENT);
  writeBinaryArray(bytecodeModule_->getObjectShapeTable());
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
} // namespace

void serializeBytecodeModule(
    BytecodeModule &BM,
    const SHA1 &sourceHash,
    llvh::raw_ostream &os,
    BytecodeGenerationOptions options) {
  BytecodeSerializer{os, options}.serialize(BM, sourceHash);
}

} // namespace hbc
} // namespace hermes
