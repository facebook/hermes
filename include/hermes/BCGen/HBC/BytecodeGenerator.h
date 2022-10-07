/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_BYTECODEGENERATOR_H
#define HERMES_BCGEN_HBC_BYTECODEGENERATOR_H

#include "llvh/ADT/DenseMap.h"

#include "hermes/BCGen/Exceptions.h"
#include "hermes/BCGen/HBC/Bytecode.h"
#include "hermes/BCGen/HBC/BytecodeDataProvider.h"
#include "hermes/BCGen/HBC/BytecodeInstructionGenerator.h"
#include "hermes/BCGen/HBC/ConsecutiveStringStorage.h"
#include "hermes/BCGen/HBC/DebugInfo.h"
#include "hermes/BCGen/HBC/SerializedLiteralGenerator.h"
#include "hermes/BCGen/HBC/UniquingFilenameTable.h"
#include "hermes/BCGen/HBC/UniquingStringLiteralTable.h"
#include "hermes/IR/IR.h"
#include "hermes/Support/BigIntSupport.h"
#include "hermes/Support/Conversions.h"
#include "hermes/Support/OptValue.h"
#include "hermes/Support/RegExpSerialization.h"
#include "llvh/ADT/StringRef.h"

namespace hermes {
namespace hbc {
using llvh::DenseMap;
using llvh::SmallVector;
using std::move;
using std::unique_ptr;

const char *const kStrippedFunctionName = "function-name-stripped";

/// An allocation table that assigns a sequential integer ID
/// to each newly added element. To support both fast lookup
/// and sequential iteration, we use both DenseMap and SmallVector
/// to store the data in different format.
template <typename T>
class AllocationTable {
  DenseMap<T, unsigned> indexMap_{};
  SmallVector<T, 8> elements_{};

 public:
  unsigned allocate(T val) {
    auto it = indexMap_.find(val);
    if (it != indexMap_.end()) {
      return it->second;
    }
    auto nextId = indexMap_.size();
    indexMap_[val] = nextId;
    elements_.push_back(val);
    return nextId;
  }

  const ArrayRef<T> getElements() const {
    return elements_;
  }
};

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

  /// Whether there are any lazy functions present.
  bool lazyFunctions_{false};

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

  /// Create a bytecode function.
  /// \p nameID is an index to the string table representing the name
  /// of this function. If unspecified, the creator basically don't
  /// care about the name and we default it to the first entry in the
  /// string table. We guarantee that the string table at runtime will
  /// have at least one entry for this purpose.
  std::unique_ptr<BytecodeFunction> generateBytecodeFunction(
      Function::DefinitionKind definitionKind,
      ValueKind valueKind,
      bool strictMode,
      uint32_t paramCount,
      uint32_t environmentSize,
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

  friend class HBCISel;
  friend class BytecodeModuleGenerator;
};

/// This class is used by the hermes backend.
/// It wraps all data required to generate the module.
class BytecodeModuleGenerator {
  /// Mapping from Function * to a sequential ID.
  AllocationTable<Function *> functionIDMap_{};

  /// Mapping from Function * to it's BytecodeFunctionGenerator *.
  DenseMap<Function *, std::unique_ptr<BytecodeFunctionGenerator>>
      functionGenerators_{};

  /// Generate literals buffer for object/array.
  SerializedLiteralGenerator literalGenerator_;

  /// The mapping from strings to ID for strings in the resulting bytecode
  /// module.
  StringLiteralTable stringTable_{};

  /// A module-wide parsed bigint table.
  bigint::UniquingBigIntTable bigIntTable_{};

  /// A module-wide compiled regexp table.
  UniquingRegExpTable regExpTable_;

  /// A module-wide filename table, kept separate from the main string table.
  /// This allows us to serialize the filenames as part of the debug info.
  UniquingFilenameTable filenameTable_{};

  /// The ID of this segment.
  uint32_t segmentID_{0};

  /// A record of all the CJS modules registered in this run of generation.
  /// List of pairs: (filename ID, function index).
  std::vector<std::pair<uint32_t, uint32_t>> cjsModules_;

  /// A record of all the CJS modules resolved in this run of generation.
  /// List of pairs: (module ID, function index).
  std::vector<std::pair<uint32_t, uint32_t>> cjsModulesStatic_;

  /// A record of all the function with non-default source representation,
  /// List of pairs: (function ID, string ID).
  std::vector<std::pair<uint32_t, uint32_t>> functionSourceTable_;

  /// Table of constants used to initialize constant arrays.
  /// They are stored as chars in order to shorten bytecode size.
  std::vector<unsigned char> arrayBuffer_{};

  /// Table of constants used to initialize object keys.
  /// They are stored as chars in order to shorten bytecode size
  std::vector<unsigned char> objKeyBuffer_{};

  /// Table of constants used to initialize object values.
  /// They are stored as chars in order to shorten bytecode size
  std::vector<unsigned char> objValBuffer_{};

  /// Options controlling bytecode generation.
  BytecodeGenerationOptions options_;

  /// Whether there are any lazy functions present.
  bool lazyFunctions_{false};

  /// Whether there are any async functions present.
  bool asyncFunctions_{false};

  /// Indicate whether this generator is still valid.
  /// We need this because one can only call the generate() function
  /// once, and after that, this generator is no longer valid because
  /// the content has been modified during generation.
  bool valid_{true};

  /// The entry point of the function (usually the global function).
  int entryPointIndex_{-1};

 public:
  /// Constructor which enables optimizations if \p optimizationEnabled is set.
  BytecodeModuleGenerator(
      BytecodeGenerationOptions options = BytecodeGenerationOptions::defaults())
      : literalGenerator_(*this, options.optimizationEnabled),
        options_(options) {}

  /// Add a function to functionIDMap_ if not already exist. Returns the ID.
  unsigned addFunction(Function *F);

  /// Add a function to the list of functions.
  void setFunctionGenerator(
      Function *F,
      unique_ptr<BytecodeFunctionGenerator> BFG);

  /// Gets the index of the entry point function (global function).
  int getEntryPointIndex() const {
    return entryPointIndex_;
  }

  /// Sets the index of the entry point function (global function).
  void setEntryPointIndex(int index) {
    entryPointIndex_ = index;
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

  /// Adds a compiled regexp to the module table.
  /// \return the index of the regexp in the table.
  uint32_t addRegExp(CompiledRegExp *regexp);

  /// Add filename to the filename table.
  /// \return the index of the string.
  uint32_t addFilename(llvh::StringRef str);

  /// Set the segment ID for this module.
  void setSegmentID(uint32_t id) {
    segmentID_ = id;
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

  /// Returns the starting offset of the elements.
  uint32_t addArrayBuffer(ArrayRef<Literal *> elements);

  /// Add to the the object buffer using \keys as the array of keys, and
  /// \vals as the array of values.
  /// Returns a pair where the first value is the object's offset into the
  /// key buffer, and the second value is its offset into the value buffer.
  std::pair<uint32_t, uint32_t> addObjectBuffer(
      ArrayRef<Literal *> keys,
      ArrayRef<Literal *> vals);

  /// Serializes the array of literals given into a compact char buffer.
  /// The serialization format can be found in:
  /// include/hermes/VM/SerializedLiteralParser.h
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

  /// \return a BytecodeModule.
  std::unique_ptr<BytecodeModule> generate();
};
} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_BYTECODEGENERATOR_H
