/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_BYTECODEGENERATOR_H
#define HERMES_BCGEN_HBC_BYTECODEGENERATOR_H

#include "hermes/BCGen/Exceptions.h"
#include "hermes/BCGen/HBC/BCProvider.h"
#include "hermes/BCGen/HBC/Bytecode.h"
#include "hermes/BCGen/HBC/BytecodeInstructionGenerator.h"
#include "hermes/BCGen/HBC/ConsecutiveStringStorage.h"
#include "hermes/BCGen/HBC/DebugInfo.h"
#include "hermes/BCGen/HBC/UniquingFilenameTable.h"
#include "hermes/BCGen/HBC/UniquingStringLiteralTable.h"
#include "hermes/BCGen/LiteralBufferBuilder.h"
#include "hermes/BCGen/SerializedLiteralGenerator.h"
#include "hermes/IR/IR.h"
#include "hermes/Regex/RegexSerialization.h"
#include "hermes/Support/BigIntSupport.h"
#include "hermes/Support/Conversions.h"
#include "hermes/Support/OptValue.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/MapVector.h"
#include "llvh/ADT/StringRef.h"

namespace hermes {
namespace hbc {

const char *const kStrippedFunctionName = "function-name-stripped";

class BytecodeModuleGenerator;

/// This class is used by the hermes backend.
/// It wraps all data required to generate the bytecode for a function.
class BytecodeFunctionGenerator : public BytecodeInstructionGenerator {
  // The bytecode module generator.
  BytecodeModuleGenerator &BMGen_;

  /// Exception handler table.
  std::vector<HBCExceptionHandlerInfo> exceptionHandlers_{};

  /// Size of the frame on stack (i.e. number of virtual registers used).
  uint32_t frameSize_{0};

  DebugSourceLocation sourceLocation_;
  std::vector<DebugSourceLocation> debugLocations_{};

  /// Table mapping variable names to frame locations.
  std::vector<Identifier> debugVariableNames_;

  /// Lexical parent function ID, i.e. the lexically containing function.
  OptValue<uint32_t> lexicalParentID_{};

  /// The size (in bytes) of the bytecode array in this function.
  uint32_t bytecodeSize_{0};

  /// Whether we are done generating this bytecode. Should be set to true by
  /// bytecodeGenerationComplete.
  bool complete_{false};

  /// Highest accessed property cache indices in this function.
  uint8_t highestReadCacheIndex_{0};
  uint8_t highestWriteCacheIndex_{0};

  /// The jump table for this function (if any)
  /// this vector consists of jump table for each SwitchImm instruction,
  /// laid out sequentially. Each entry is a relative jump.
  std::vector<uint32_t> jumpTable_{};

  explicit BytecodeFunctionGenerator(
      BytecodeModuleGenerator &BMGen,
      uint32_t frameSize)
      : BytecodeInstructionGenerator(), BMGen_(BMGen), frameSize_(frameSize) {}

 public:
  static std::unique_ptr<BytecodeFunctionGenerator> create(
      BytecodeModuleGenerator &BMGen,
      uint32_t frameSize) {
    return std::unique_ptr<BytecodeFunctionGenerator>(
        new BytecodeFunctionGenerator(BMGen, frameSize));
  }

  /// \return the generator for the current BytecodeModule.
  BytecodeModuleGenerator &getBytecodeModuleGenerator() {
    return BMGen_;
  }

  /// \return the generator for the current BytecodeModule.
  const BytecodeModuleGenerator &getBytecodeModuleGenerator() const {
    return BMGen_;
  }

  /// Create a bytecode function.
  /// \p nameID is an index to the string table representing the name
  /// of this function. If unspecified, the creator basically don't
  /// care about the name and we default it to the first entry in the
  /// string table. We guarantee that the string table at runtime will
  /// have at least one entry for this purpose.
  std::unique_ptr<BytecodeFunction> generateBytecodeFunction(
      Function::ProhibitInvoke prohibitInvoke,
      bool strictMode,
      uint32_t paramCount,
      uint32_t nameID = 0);

  unsigned getFunctionID(Function *F);

  /// \return the ID in the bytecode's bigint table for a given literal string
  /// \p value.
  unsigned getBigIntID(LiteralBigInt *value) const;

  /// \return the ID in the bytecode's string table for a given literal string
  /// \p value.
  unsigned getStringID(LiteralString *value) const;

  /// \return the ID in the bytecode's string table for a given literal string
  /// \p value, assuming it has been registered for us as an identifier.
  unsigned getIdentifierID(LiteralString *value) const;

  /// Adds a parsed bigint to the module table.
  /// \return the index of the bigint in the table.
  uint32_t addBigInt(bigint::ParsedBigInt bigint);

  /// Adds a compiled regexp to the module table.
  /// \return the index of the regexp in the table.
  uint32_t addRegExp(CompiledRegExp *regexp);

  /// Add filename to the filename table.
  /// \return the index of the string.
  uint32_t addFilename(llvh::StringRef filename);

  void addExceptionHandler(HBCExceptionHandlerInfo info);

  /// Set the source location of the function definition.
  void setSourceLocation(const DebugSourceLocation &location) {
    assert(
        !complete_ &&
        "Cannot modify BytecodeFunction after call to bytecodeGenerationComplete.");
    sourceLocation_ = location;
  }

  const DebugSourceLocation &getSourceLocation() const {
    return sourceLocation_;
  }

  /// Add the location of an opcode.
  void addDebugSourceLocation(const DebugSourceLocation &info);
  const std::vector<DebugSourceLocation> &getDebugLocations() const {
    return debugLocations_;
  }

  bool hasDebugInfo() const {
    return !debugLocations_.empty() || lexicalParentID_ ||
        !debugVariableNames_.empty();
  }

  /// Add a debug variable named \name.
  void setDebugVariableNames(std::vector<Identifier> names) {
    assert(
        !complete_ &&
        "Cannot modify BytecodeFunction after call to bytecodeGenerationComplete.");
    debugVariableNames_ = std::move(names);
  }

  /// \return the list of debug variable names.
  llvh::ArrayRef<Identifier> getDebugVariableNames() const {
    return debugVariableNames_;
  }

  /// Set the lexical parent ID to \p parentId.
  void setLexicalParentID(OptValue<uint32_t> parentID) {
    assert(
        !complete_ &&
        "Cannot modify BytecodeFunction after call to bytecodeGenerationComplete.");
    lexicalParentID_ = parentID;
  }

  /// \return the lexical parent ID (that is, the function lexically enclosing
  /// this function) or None if there is no lexical parent ID (i.e. the function
  /// is global).
  OptValue<uint32_t> getLexicalParentID() const {
    return lexicalParentID_;
  }

  /// Shift the bytecode stream starting from \p loc left by 3 bytes.
  /// This is called when a long jump offset is found to fit into 1 byte.
  void shrinkJump(offset_t loc);

  /// Update \p bytes number of bytes in opcode stream location \loc
  /// with \p newVal.
  void updateJumpTarget(offset_t loc, int newVal, int bytes);

  /// Update the jump table offset of a SwitchImm instruction during
  /// jump relocation.
  /// \param loc location of the instruction
  /// \param jumpTableOffset the offset into the jump table;
  /// \param ip offset will be computed relative to this position in bytecode
  ///   vector.
  void
  updateJumpTableOffset(offset_t loc, uint32_t jumpTableOffset, uint32_t cs);

  /// Change the opcode of a long jump instruction into a short jump.
  inline void longToShortJump(offset_t loc) {
    switch (opcodes_[loc]) {
#define DEFINE_JUMP_LONG_VARIANT(shortName, longName) \
  case longName##Op:                                  \
    opcodes_[loc] = shortName##Op;                    \
    break;
#include "hermes/BCGen/HBC/BytecodeList.def"
      default:
        llvm_unreachable("Unknown jump opcode");
    }
  }

  /// \return the size of the frame.
  uint32_t getFrameSize() const {
    return frameSize_;
  }

  void setHighestReadCacheIndex(uint8_t sz) {
    assert(
        !complete_ &&
        "Cannot modify BytecodeFunction after call to bytecodeGenerationComplete.");
    this->highestReadCacheIndex_ = sz;
  }
  void setHighestWriteCacheIndex(uint8_t sz) {
    assert(
        !complete_ &&
        "Cannot modify BytecodeFunction after call to bytecodeGenerationComplete.");
    this->highestWriteCacheIndex_ = sz;
  }

  /// Set the jump table for this function, if any.
  void setJumpTable(std::vector<uint32_t> &&jumpTable);

  /// Signal that bytecode generation is finalized.
  void bytecodeGenerationComplete();

  friend class BytecodeModuleGenerator;
};

/// This class is used by the hermes backend.
/// It wraps all data required to generate the module.
class BytecodeModuleGenerator {
  /// The bytecode module being generated.
  /// Never nullptr while this BytecodeModuleGenerator is valid_.
  std::unique_ptr<BytecodeModule> bm_;

  /// Mapping from Function * to a sequential ID.
  llvh::MapVector<Function *, unsigned> functionIDMap_{};

  /// Generates debug information.
  /// Stores the filename table.
  DebugInfoGenerator debugInfoGenerator_;

  /// A map from instruction to literal offset in the corresponding buffers.
  /// \c arrayBuffer_, \c objKeyBuffer_, \c objValBuffer_.
  /// This map is populated before instruction selection.
  LiteralBufferBuilder::LiteralOffsetMapTy literalOffsetMap_{};

  /// Options controlling bytecode generation.
  BytecodeGenerationOptions options_;

  /// The source map generator to use (nullptr if none).
  SourceMapGenerator *sourceMapGen_;

  /// Indicate whether this generator is still valid.
  /// We need this because one can only call the generate() function
  /// once, and after that, this generator is no longer valid because
  /// the content has been modified during generation.
  bool valid_{true};

 public:
  /// Constructor which enables optimizations if \p options.optimizationEnabled
  /// is set.
  BytecodeModuleGenerator(
      BytecodeGenerationOptions options = BytecodeGenerationOptions::defaults(),
      SourceMapGenerator *sourceMapGen = nullptr)
      : bm_(new BytecodeModule()),
        options_(options),
        sourceMapGen_(sourceMapGen) {
    bm_->getBytecodeOptionsMut().staticBuiltins =
        options_.staticBuiltinsEnabled;
  }

  /// Add a function to request generating bytecode for it if it doesn't
  /// already exist.
  /// The associated BytecodeFunction will be nullptr until it's generated.
  /// \return the function ID.
  unsigned addFunction(Function *F);

  /// Add a function to the list of functions.
  void setFunctionGenerator(
      Function *F,
      std::unique_ptr<BytecodeFunctionGenerator> BFG);

  /// Gets the index of the entry point function (global function).
  int getEntryPointIndex() const {
    return bm_->getGlobalFunctionIndex();
  }

  /// Sets the index of the entry point function (global function).
  void setEntryPointIndex(int index) {
    bm_->setGlobalFunctionIndex(index);
  }

  /// \returns the index of the bigint in this module's bigint table if it
  /// exists.  If the bigint does not exist will trigger an assertion failure
  /// if assertions are enabled.
  unsigned getBigIntID(llvh::StringRef str) const;

  /// \returns the index of the string in this module's string table if it
  /// exists.  If the string does not exist will trigger an assertion failure
  /// if assertions are enabled.
  unsigned getStringID(llvh::StringRef str) const;

  /// \returns the index of the string in this module's string table, assuming
  /// it exists and is an identifier.  If the string does not exist in the
  /// table, or it is not marked as an identifier, an assertion failure will be
  /// triggered, if assertions are enabled.
  unsigned getIdentifierID(llvh::StringRef str) const;

  /// Set the string table this generator uses to find the IDs for strings.
  /// Once it is set, this table will not be further modified -- all strings
  /// must be added beforehand.  This can only be called once on a given
  /// generator.
  void initializeStringTable(StringLiteralTable stringTable);

  /// Adds a parsed bigint to the module table.
  /// \return the index of the bigint in the table.
  uint32_t addBigInt(bigint::ParsedBigInt bigint);

  /// Set the serialized literal tables that this generator will use. Once set,
  /// no further modifications are possible.
  /// \param bufs containing the serialized literals.
  void initializeSerializedLiterals(LiteralBufferBuilder::Result &&bufs);

  /// Adds a compiled regexp to the module table.
  /// \return the index of the regexp in the table.
  uint32_t addRegExp(CompiledRegExp *regexp);

  /// Add filename to the filename table.
  /// \return the index of the string.
  uint32_t addFilename(llvh::StringRef str);

  /// Set the segment ID for this module.
  void setSegmentID(uint32_t id) {
    bm_->setSegmentID(id);
  }

  /// Adds a CJS module entry to the table.
  void addCJSModule(uint32_t functionID, uint32_t nameID);

  /// Adds a statically-resolved CJS module entry to the table.
  /// \param moduleID the index of the CJS module (incremented each call).
  void addCJSModuleStatic(uint32_t moduleID, uint32_t functionID);

  /// Adds a function source entry to the table.
  /// \param functionID the index of the function.
  /// \param stringID the index of the corresponding source in the string table.
  void addFunctionSource(uint32_t functionID, uint32_t stringID);

  /// Serializes the array of literals given into a compact char buffer.
  /// The serialization format can be found in:
  /// include/hermes/BCGen/SerializedLiteralGenerator.h
  /// This function serializes the literals, and checks to see if the exact
  /// byte pattern is already present in \buff. If it is, it simply returns
  /// its offset in \buff. If it isn't, the function appends it and returns
  /// its offset.
  /// NOTE: Since it simply does a byte by byte search, it can return indices
  /// that don't correspond to any previously inserted literals.
  ///   e.g. When serialized, [int 24833]'s last two bytes are equivalent to
  ///   [String 1], and if they are added separately, serializeBuffer would
  ///   return the offset of the last two bytes instead of appending
  ///   [String 1] to the buffer.
  uint32_t serializeBuffer(
      ArrayRef<Literal *> literals,
      std::vector<unsigned char> &buff,
      bool isKeyBuffer);

  /// For a given instruction \p inst that has an associated serialized literal,
  /// obtain the offset of the literal in the associated buffer. In case of
  /// an object literal, it is a pair of offsets (key and value). In case of
  /// array literal, only the first offset is used.
  LiteralBufferBuilder::LiteralOffset serializedLiteralOffsetFor(
      const Instruction *inst) const {
    assert(
        literalOffsetMap_.count(inst) &&
        "instruction has no serialized literal");
    return literalOffsetMap_.find(inst)->second;
  }

  /// \return a BytecodeModule.
  std::unique_ptr<BytecodeModule> generate() &&;
};
} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_BYTECODEGENERATOR_H
