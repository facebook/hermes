/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_BYTECODE_H
#define HERMES_BCGEN_HBC_BYTECODE_H

#include "llvh/ADT/ArrayRef.h"
#include "llvh/Support/SMLoc.h"

#include "hermes/AST/Context.h"
#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "hermes/BCGen/HBC/BytecodeInstructionGenerator.h"
#include "hermes/BCGen/HBC/BytecodeStream.h"
#include "hermes/BCGen/HBC/DebugInfo.h"
#include "hermes/IR/IR.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Support/RegExpSerialization.h"
#include "hermes/Support/StringKind.h"
#include "hermes/Support/StringTableEntry.h"
#include "hermes/Utils/Options.h"

#include <memory>

namespace hermes {
class SourceMapGenerator;

namespace hbc {

using llvh::ArrayRef;

// This class represents the in-memory representation of the bytecode function.
class BytecodeFunction {
  /// The bytecode executable.
  std::vector<opcode_atom_t> opcodes_;

  /// Header that stores all the metadata of this function.
  FunctionHeader header_;

  /// Offsets of this function in debug info.
  DebugOffsets debugOffsets_{};

  /// List of exception handlers.
  std::vector<HBCExceptionHandlerInfo> exceptions_;

  /// Data to lazily compile this BytecodeFunction, if applicable.
  std::unique_ptr<LazyCompilationData> lazyCompilationData_{};

  /// Jump table section. This vector consists of a jump table for each
  /// SwitchImm instruction in the function, laid out sequentially.
  /// Entries are jumps relative to the corresponding
  /// SwitchImm instruction that the jump table segment belongs to.
  std::vector<uint32_t> jumpTables_;

 public:
  /// Used during serialization. \p opcodes will be swapped after this call.
  explicit BytecodeFunction(
      std::vector<opcode_atom_t> &&opcodes,
      Function::DefinitionKind definitionKind,
      ValueKind valueKind,
      bool strictMode,
      FunctionHeader &&header,
      std::vector<HBCExceptionHandlerInfo> &&exceptionHandlers,
      std::vector<uint32_t> &&jumpTables)
      : opcodes_(std::move(opcodes)),
        header_(std::move(header)),
        exceptions_(std::move(exceptionHandlers)),
        jumpTables_(std::move(jumpTables)) {
    switch (definitionKind) {
      case Function::DefinitionKind::ES6Arrow:
      case Function::DefinitionKind::ES6Method:
        header_.flags.prohibitInvoke = FunctionHeaderFlag::ProhibitConstruct;
        break;
      case Function::DefinitionKind::ES6Constructor:
        header_.flags.prohibitInvoke = FunctionHeaderFlag::ProhibitCall;
        break;
      default:
        // ES9.0 9.2.3 step 4 states that generator functions cannot be
        // constructed.
        // We place this check outside the `DefinitionKind` because generator
        // functions may also be ES6 methods, for example, and are not included
        // in the DefinitionKind enum.
        // Note that we only have to check for GeneratorFunctionKind in this
        // case, because ES6 methods are already checked above, and ES6
        // constructors are prohibited from being generator functions.
        // As such, this is the only case in which we must change the
        // prohibitInvoke flag based on valueKind.
        header_.flags.prohibitInvoke =
            valueKind == ValueKind::GeneratorFunctionKind
            ? FunctionHeaderFlag::ProhibitConstruct
            : FunctionHeaderFlag::ProhibitNone;
        break;
    }
    header_.flags.strictMode = strictMode;
    header_.flags.hasExceptionHandler = exceptions_.size();
  }

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

  ArrayRef<opcode_atom_t> getOpcodeArray() const {
    return opcodes_;
  }

  ArrayRef<uint32_t> getJumpTables() const {
    return jumpTables_;
  }

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

  /// Inlines the jump tables (if any) into the bytecode array.
  void inlineJumpTables();
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

  /// The ID of the first CJS module in this BytecodeModule.
  /// This is the first entry in the Domain's CJS module table that will be
  /// populated by the corresponding RuntimeModule.
  uint32_t cjsModuleOffset_;

  /// Table which indicates where to find the different CommonJS modules.
  /// Mapping from {filename ID => function index}.
  std::vector<std::pair<uint32_t, uint32_t>> cjsModuleTable_{};

  /// Table which indicates where to find the different CommonJS modules.
  /// Used if the modules have been statically resolved.
  /// Element i contains the function index for module i + cjsModuleOffset.
  std::vector<uint32_t> cjsModuleTableStatic_{};

  /// Storing information about the bytecode, needed when it is loaded by the
  /// runtime.
  BytecodeOptions options_{};

#ifndef HERMESVM_LEAN
  /// Stores references to source code strings for functions. These are only
  /// available when compiling from source at run-time, and when lazily compiled
  /// functions or Function.toString() returning source are enabled.
  llvh::DenseMap<uint32_t, llvh::SMRange> functionSourceRangeMap_;

  /// If there are any entries in functionSourceRangeMap_ we need to keep
  /// Context alive so the source buffers don't dissapear from under us.
  std::shared_ptr<Context> context_;
#endif

 public:
  /// Used during serialization.
  explicit BytecodeModule(
      uint32_t functionCount,
      std::vector<StringKind::Entry> &&stringKinds,
      std::vector<uint32_t> &&identifierHashes,
      std::vector<StringTableEntry> &&stringTable,
      std::vector<unsigned char> &&stringStorage,
      std::vector<RegExpTableEntry> &&regExpTable,
      std::vector<unsigned char> &&regExpStorage,
      uint32_t globalFunctionIndex,
      std::vector<unsigned char> &&arrayBuffer,
      std::vector<unsigned char> &&objKeyBuffer,
      std::vector<unsigned char> &&objValBuffer,
      uint32_t cjsModuleOffset,
      std::vector<std::pair<uint32_t, uint32_t>> &&cjsModuleTable,
      std::vector<uint32_t> &&cjsModuleTableStatic,
      BytecodeOptions options)
      : globalFunctionIndex_(globalFunctionIndex),
        stringKinds_(std::move(stringKinds)),
        identifierHashes_(std::move(identifierHashes)),
        stringTable_(std::move(stringTable)),
        stringStorage_(std::move(stringStorage)),
        regExpStorage_(std::move(regExpStorage)),
        regExpTable_(std::move(regExpTable)),
        arrayBuffer_(std::move(arrayBuffer)),
        objKeyBuffer_(std::move(objKeyBuffer)),
        objValBuffer_(std::move(objValBuffer)),
        cjsModuleOffset_(cjsModuleOffset),
        cjsModuleTable_(std::move(cjsModuleTable)),
        cjsModuleTableStatic_(std::move(cjsModuleTableStatic)),
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

  llvh::ArrayRef<RegExpTableEntry> getRegExpTable() const {
    return regExpTable_;
  }

  llvh::ArrayRef<unsigned char> getRegExpStorage() const {
    return regExpStorage_;
  }

  uint32_t getCJSModuleOffset() const {
    return cjsModuleOffset_;
  }

  llvh::ArrayRef<std::pair<uint32_t, uint32_t>> getCJSModuleTable() const {
    return cjsModuleTable_;
  }

  llvh::ArrayRef<uint32_t> getCJSModuleTableStatic() const {
    return cjsModuleTableStatic_;
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
    return {ArrayRef<unsigned char>(objKeyBuffer_),
            ArrayRef<unsigned char>(objValBuffer_)};
  }

  /// Returns a pair, where the first element is the key buffer starting at
  /// keyIdx and the second element is the value buffer starting at valIdx
  std::pair<ArrayRef<unsigned char>, ArrayRef<unsigned char>>
  getObjectBufferAtOffset(unsigned keyIdx, unsigned valIdx) const {
    return {ArrayRef<unsigned char>(objKeyBuffer_).slice(keyIdx),
            ArrayRef<unsigned char>(objValBuffer_).slice(valIdx)};
  }

  /// Populate the source map \p sourceMap with the debug information.
  void populateSourceMap(SourceMapGenerator *sourceMap) const;

  /// Jump tables are inlined into bytecode segment, however this does not
  /// happen until serialization. If we are executing straight from source
  /// however this step never happens, this function will inline the jump table
  /// into the opcode vector instead.
  void inlineJumpTables();

  BytecodeOptions getBytecodeOptions() const {
    return options_;
  }

#ifndef HERMESVM_LEAN
  /// Called during BytecodeModule generation.
  void setFunctionSourceRange(uint32_t functionID, llvh::SMRange range) {
    functionSourceRangeMap_.try_emplace(functionID, range);
  }

  /// If we retain source code for any functions we also need to preserve
  /// \c Context. Called during BytecodeModule generation.
  void setContext(std::shared_ptr<Context> context) {
    context_ = context;
  }

  /// Returns source code for a given function if available. Use \c isValid() on
  /// the result to confirm source is actually available.
  llvh::SMRange getFunctionSourceRange(uint32_t functionID) {
    auto it = functionSourceRangeMap_.find(functionID);
    return it == functionSourceRangeMap_.end() ? llvh::SMRange() : it->second;
  }

  /// This will only be available if we're retaining source code for at least
  /// one function in this module.
  std::shared_ptr<Context> getContext() const {
    return context_;
  }
#endif
};

} // namespace hbc
} // namespace hermes

#endif
