/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_BYTECODE_H
#define HERMES_BCGEN_HBC_BYTECODE_H

#include "llvh/ADT/ArrayRef.h"

#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "hermes/BCGen/HBC/BytecodeInstructionGenerator.h"
#include "hermes/BCGen/HBC/BytecodeStream.h"
#include "hermes/BCGen/HBC/DebugInfo.h"
#include "hermes/BCGen/HBC/StringKind.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Support/BigIntSupport.h"
#include "hermes/Support/RegExpSerialization.h"
#include "hermes/Support/StringTableEntry.h"
#include "hermes/Utils/Options.h"

#include <memory>
#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
namespace hermes {
class SourceMapGenerator;

namespace hbc {

using llvh::ArrayRef;

// This class represents the in-memory representation of the bytecode function.
class BytecodeFunction {
  /// The bytecode executable. If this function contains switch statements, then
  /// the jump tables are also inlined in a 4-byte aligned block at the end of
  /// this vector.
  /// During execution, this vector can be executed directly because the vector
  /// storage will be 4-byte aligned.
  /// During serialisation, the jump tables and opcodes have to be separated so
  /// that the jump tables can be realigned based on the overall bytecode
  /// layout.
  const std::vector<opcode_atom_t> opcodesAndJumpTables_;

  /// Header that stores all the metadata of this function.
  FunctionHeader header_;

  /// Offsets of this function in debug info.
  DebugOffsets debugOffsets_{};

  /// List of exception handlers.
  std::vector<HBCExceptionHandlerInfo> exceptions_;

  /// Data to lazily compile this BytecodeFunction, if applicable.
  std::unique_ptr<LazyCompilationData> lazyCompilationData_{};

 public:
  /// Used during serialization. \p opcodes will be swapped after this call.
  explicit BytecodeFunction(
      std::vector<opcode_atom_t> &&opcodesAndJumpTables,
      FunctionHeader &&header,
      std::vector<HBCExceptionHandlerInfo> &&exceptionHandlers)
      : opcodesAndJumpTables_(std::move(opcodesAndJumpTables)),
        header_(std::move(header)),
        exceptions_(std::move(exceptionHandlers)) {}

  const FunctionHeader &getHeader() const {
    return header_;
  }

  FunctionHeaderFlag &mutableFlags() {
    return header_.flags;
  }

  void setOffset(uint32_t offset) {
    header_.offset = offset;
  }
  uint32_t getOffset() const {
    return header_.offset;
  }

  void setInfoOffset(uint32_t offset) {
    header_.infoOffset = offset;
  }

  bool isStrictMode() const {
    return header_.flags.strictMode;
  }

  /// Return the entire opcode array for execution, including the inlined jump
  /// tables.
  ArrayRef<opcode_atom_t> getOpcodeArray() const {
    return opcodesAndJumpTables_;
  }

  /// Return only the opcodes for serialisation. The jump tables need to be
  /// accessed separately so they can be correctly inlined.
  ArrayRef<opcode_atom_t> getOpcodesOnly() const {
    return {opcodesAndJumpTables_.data(), header_.bytecodeSizeInBytes};
  }

  /// Return the jump table portion of the opcode array. This is useful for the
  /// bytecode serialisation code when it is aligning the jump tables.
  ArrayRef<uint32_t> getJumpTablesOnly() const;

  bool hasExceptionHandlers() const {
    return exceptions_.size() > 0;
  }

  uint32_t getExceptionHandlerCount() const {
    return exceptions_.size();
  }

  ArrayRef<HBCExceptionHandlerInfo> getExceptionHandlers() const {
    return exceptions_;
  }

  bool hasDebugInfo() const {
    return debugOffsets_.sourceLocations != DebugOffsets::NO_OFFSET ||
        debugOffsets_.lexicalData != DebugOffsets::NO_OFFSET;
  }

  const DebugOffsets *getDebugOffsets() const {
    return &debugOffsets_;
  }

  void setDebugOffsets(DebugOffsets offsets) {
    debugOffsets_ = offsets;
    header_.flags.hasDebugInfo = hasDebugInfo();
  }

  LazyCompilationData *getLazyCompilationData() const {
    return lazyCompilationData_.get();
  }

  void setLazyCompilationData(std::unique_ptr<LazyCompilationData> data) {
    lazyCompilationData_ = std::move(data);
  }

  /// \return true if the function should be compiled lazily.
  bool isLazy() const {
    return (bool)lazyCompilationData_;
  }
};

// This class represents the in-memory representation of the bytecode module.
class BytecodeModule {
  using FunctionList = std::vector<std::unique_ptr<BytecodeFunction>>;
  using SerializableBufferTy = std::vector<unsigned char>;

  /// A list of bytecode functions.
  FunctionList functions_{};

  /// Index of the top level function, code that gets executed in the global
  /// scope.
  uint32_t globalFunctionIndex_{};

  /// Run-length encoding representing the kinds of strings in the table.
  std::vector<StringKind::Entry> stringKinds_;

  /// The list of identifier hashes, corresponding to the string entries
  /// marked as identifiers, in order.
  std::vector<uint32_t> identifierHashes_;

  /// The global string table, a list of <offset, length> pair to represent
  /// each string in the string storage.
  std::vector<StringTableEntry> stringTable_;

  /// The global string storage. A sequence of bytes.
  std::vector<unsigned char> stringStorage_;

  /// The bigint digit table. This is a list of pairs of (offset, lengths)
  /// into bigIntStorage_.
  std::vector<bigint::BigIntTableEntry> bigIntTable_;

  /// The buffer with bigint literal bytes.
  std::vector<uint8_t> bigIntStorage_;

  /// The regexp bytecode buffer.
  std::vector<unsigned char> regExpStorage_;

  /// The regexp bytecode table. This is a list of pairs of (offset, lengths)
  /// into regExpStorage_.
  std::vector<RegExpTableEntry> regExpTable_;

  /// A table containing debug info (if compiled with -g).
  DebugInfo debugInfo_;

  /// Array Buffer table.
  SerializableBufferTy arrayBuffer_;

  /// Object Key Buffer table.
  SerializableBufferTy objKeyBuffer_;

  /// Object Value Buffer table.
  SerializableBufferTy objValBuffer_;

  /// The segment ID corresponding to this BytecodeModule.
  /// This uniquely identifies this BytecodeModule within a set of modules
  /// which were compiled at the same time (and will correspond to a set of
  /// RuntimeModules in a Domain).
  uint32_t segmentID_;

  /// Table which indicates where to find the different CommonJS modules.
  /// Mapping from {filename ID => function index}.
  std::vector<std::pair<uint32_t, uint32_t>> cjsModuleTable_{};

  /// Table which indicates where to find the different CommonJS modules.
  /// Used if the modules have been statically resolved.
  /// Mapping from {global module ID => function index}.
  std::vector<std::pair<uint32_t, uint32_t>> cjsModuleTableStatic_{};

  /// Mapping function ids to the string table offsets that store their
  /// non-default source code representation that would be used by `toString`.
  /// These are only available when functions are declared with source
  /// visibility directives such as 'show source', 'hide source', etc.
  std::vector<std::pair<uint32_t, uint32_t>> functionSourceTable_{};

  /// Storing information about the bytecode, needed when it is loaded by the
  /// runtime.
  BytecodeOptions options_{};

 public:
  /// Used during serialization.
  explicit BytecodeModule(
      uint32_t functionCount,
      std::vector<StringKind::Entry> &&stringKinds,
      std::vector<uint32_t> &&identifierHashes,
      std::vector<StringTableEntry> &&stringTable,
      std::vector<unsigned char> &&stringStorage,
      std::vector<bigint::BigIntTableEntry> &&bigIntTable,
      std::vector<uint8_t> &&bigIntStorage,
      std::vector<RegExpTableEntry> &&regExpTable,
      std::vector<unsigned char> &&regExpStorage,
      uint32_t globalFunctionIndex,
      std::vector<unsigned char> &&arrayBuffer,
      std::vector<unsigned char> &&objKeyBuffer,
      std::vector<unsigned char> &&objValBuffer,
      uint32_t segmentID,
      std::vector<std::pair<uint32_t, uint32_t>> &&cjsModuleTable,
      std::vector<std::pair<uint32_t, uint32_t>> &&cjsModuleTableStatic,
      std::vector<std::pair<uint32_t, uint32_t>> &&functionSourceTable,
      BytecodeOptions options)
      : globalFunctionIndex_(globalFunctionIndex),
        stringKinds_(std::move(stringKinds)),
        identifierHashes_(std::move(identifierHashes)),
        stringTable_(std::move(stringTable)),
        stringStorage_(std::move(stringStorage)),
        bigIntTable_(std::move(bigIntTable)),
        bigIntStorage_(std::move(bigIntStorage)),
        regExpStorage_(std::move(regExpStorage)),
        regExpTable_(std::move(regExpTable)),
        arrayBuffer_(std::move(arrayBuffer)),
        objKeyBuffer_(std::move(objKeyBuffer)),
        objValBuffer_(std::move(objValBuffer)),
        segmentID_(segmentID),
        cjsModuleTable_(std::move(cjsModuleTable)),
        cjsModuleTableStatic_(std::move(cjsModuleTableStatic)),
        functionSourceTable_(std::move(functionSourceTable)),
        options_(options) {
    functions_.resize(functionCount);
  }

  /// Create a simple BytecodeModule with only functionCount set.
  explicit BytecodeModule(uint32_t functionCount) {
    functions_.resize(functionCount);
  }

  const FunctionList &getFunctionTable() const {
    return functions_;
  }
  uint32_t getNumFunctions() const {
    return functions_.size();
  }

  uint32_t getGlobalFunctionIndex() const {
    return globalFunctionIndex_;
  }

  /// Add a new bytecode function to the module at \p index.
  void setFunction(uint32_t index, std::unique_ptr<BytecodeFunction> F);

  BytecodeFunction &getFunction(unsigned index);

  BytecodeFunction &getGlobalCode() {
    return getFunction(globalFunctionIndex_);
  }

  llvh::ArrayRef<StringKind::Entry> getStringKinds() const {
    return stringKinds_;
  }

  /// \return the number of identifiers.
  uint32_t getIdentifierCount() const {
    return identifierHashes_.size();
  }

  llvh::ArrayRef<uint32_t> getIdentifierHashes() const {
    return identifierHashes_;
  }

  uint32_t getStringTableSize() const {
    return stringTable_.size();
  }

  StringTableEntry::StringTableRefTy getStringTable() const {
    return stringTable_;
  }

  uint32_t getStringStorageSize() const {
    return stringStorage_.size();
  }

  StringTableEntry::StringStorageRefTy getStringStorage() const {
    return stringStorage_;
  }

  llvh::ArrayRef<bigint::BigIntTableEntry> getBigIntTable() const {
    return bigIntTable_;
  }

  llvh::ArrayRef<uint8_t> getBigIntStorage() const {
    return bigIntStorage_;
  }

  llvh::ArrayRef<RegExpTableEntry> getRegExpTable() const {
    return regExpTable_;
  }

  llvh::ArrayRef<unsigned char> getRegExpStorage() const {
    return regExpStorage_;
  }

  uint32_t getSegmentID() const {
    return segmentID_;
  }

  llvh::ArrayRef<std::pair<uint32_t, uint32_t>> getCJSModuleTable() const {
    return cjsModuleTable_;
  }

  llvh::ArrayRef<std::pair<uint32_t, uint32_t>> getCJSModuleTableStatic()
      const {
    return cjsModuleTableStatic_;
  }

  llvh::ArrayRef<std::pair<uint32_t, uint32_t>> getFunctionSourceTable() const {
    return functionSourceTable_;
  }

  DebugInfo &getDebugInfo() {
    return debugInfo_;
  }

  void setDebugInfo(DebugInfo info) {
    debugInfo_ = std::move(info);
  }

  uint32_t getArrayBufferSize() const {
    return arrayBuffer_.size();
  }

  ArrayRef<unsigned char> getArrayBuffer() const {
    return arrayBuffer_;
  }

  /// Returns the amount of bytes in the object key buffer
  uint32_t getObjectKeyBufferSize() const {
    return objKeyBuffer_.size();
  }

  /// Returns the amount of bytes in the object value buffer
  uint32_t getObjectValueBufferSize() const {
    return objValBuffer_.size();
  }

  /// Returns a pair of arrays, where the first array represents the
  /// object keys and the second array represents the object values.
  std::pair<ArrayRef<unsigned char>, ArrayRef<unsigned char>> getObjectBuffer()
      const {
    return {
        ArrayRef<unsigned char>(objKeyBuffer_),
        ArrayRef<unsigned char>(objValBuffer_)};
  }

  /// Returns a pair, where the first element is the key buffer starting at
  /// keyIdx and the second element is the value buffer starting at valIdx
  std::pair<ArrayRef<unsigned char>, ArrayRef<unsigned char>>
  getObjectBufferAtOffset(unsigned keyIdx, unsigned valIdx) const {
    return {
        ArrayRef<unsigned char>(objKeyBuffer_).slice(keyIdx),
        ArrayRef<unsigned char>(objValBuffer_).slice(valIdx)};
  }

  /// Populate the source map \p sourceMap with the debug information.
  void populateSourceMap(SourceMapGenerator *sourceMap) const;

  BytecodeOptions getBytecodeOptions() const {
    return options_;
  }
};

} // namespace hbc
} // namespace hermes
#pragma GCC diagnostic pop

#endif
