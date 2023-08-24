/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_ISEL_H
#define HERMES_BCGEN_HBC_ISEL_H

#include "hermes/BCGen/Exceptions.h"
#include "hermes/BCGen/HBC/BytecodeGenerator.h"
#include "hermes/BCGen/HBC/HVMRegisterAllocator.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/IR.h"
#include "hermes/Utils/Dumper.h"
#include "hermes/Utils/Options.h"

#include "llvh/ADT/DenseMap.h"

namespace hermes {
namespace hbc {

/// The filename ID and source map URL ID of a buffer.
struct FileAndSourceMapId {
  /// ID of the filename when added to BytecodeFunctionGenerator.
  uint32_t filenameId;
  /// ID of the source map URL when added as a file to
  /// BytecodeFunctionGenerator.
  uint32_t sourceMappingUrlId;
};

/// A map from buffer ID to filelname+source map.
/// Looking up filename/sourcemap id for each instruction is pretty slow,
/// so we cache it.
using FileAndSourceMapIdCache =
    llvh::SmallDenseMap<unsigned, FileAndSourceMapId>;

class HBCISel {
  struct Relocation {
    enum RelocationType {
      // A short jump instruction
      JumpType = 0,
      // A long jump instruction
      LongJumpType,
      // A basic block
      BasicBlockType,
      // A catch instruction
      CatchType,
      // Debug info
      DebugInfo,
      // Textified callee
      TextifiedCallee,
      // Jump table dispatch
      JumpTableDispatch,
    };

    /// The current location of this relocation.
    offset_t loc;
    /// Type of the relocation.
    RelocationType type;
    /// We multiplex pointer for different things under different types:
    /// If the type is jump or long jump, pointer is the target basic block;
    /// if the type is basic block, pointer is the pointer to it.
    /// if the type is catch instruction, pointer is the pointer to it.
    Value *pointer;
  };

  /// Info about a jump table instruction used during jump relocation.
  struct SwitchImmInfo {
    /// Offset of the instruction
    uint32_t offset;

    /// Block to jump to when no matching case is found.
    BasicBlock *defaultTarget;

    /// The actual jump table table.
    /// The i'th index indicates which basic block should be jumped to for value
    /// i
    std::vector<BasicBlock *> table;
  };

  /// The function that we are compiling.
  Function *F_;

  /// The bytecode function that we are constructing.
  BytecodeFunctionGenerator *BCFGen_;

  /// The register allocator.
  HVMRegisterAllocator &RA_;

  /// The function scope depth analysis, used to determine the lexical parents
  /// and scope depth of each function.
  FunctionScopeAnalysis &scopeAnalysis_;

  /// The scope register analysis, used to determine which register contains the
  /// Environment that's equivalent to the source-level scope where this
  /// instruction was emitted.
  ScopeRegisterAnalysis &SRA_;

  /// For each Basic Block, we map to its beginning instruction location
  /// and the next basic block. We need this information to resolve jump
  /// targets and exception handler table.
  DenseMap<BasicBlock *, std::pair<offset_t, BasicBlock *>> basicBlockMap_{};

  /// The set of BasicBlocks that require an async break check prefix.
  DenseSet<const BasicBlock *> asyncBreakChecks_{};

  /// The list of all jump instructions and jump targets that require
  /// relocation and address resolution.
  SmallVector<Relocation, 8> relocations_{};

  /// A map of instructions to bytecode locations for debug info.
  DenseMap<Instruction *, offset_t> debugInstructionOffset_{};

  /// Mapping from CatchInst to the catch coverage information.
  CatchInfoMap catchInfoMap_{};

  /// Bytecode generation options.
  const BytecodeGenerationOptions &bytecodeGenerationOptions_;

  /// Map from SwitchImm -> (inst offset, default block, jump table).
  llvh::DenseMap<SwitchImmInst *, SwitchImmInfo> switchImmInfo_{};
  using switchInfoEntry =
      llvh::DenseMap<SwitchImmInst *, SwitchImmInfo>::iterator::value_type;

  /// Saved identifier of "__proto__" for fast comparisons.
  Identifier protoIdent_{};

  /// Encode a value into a param_t type.
  unsigned encodeValue(Value *);

  /// Resolve the offset of every relocation.
  void resolveRelocations();

  /// Add long jump instruction to the relocation list.
  void registerLongJump(offset_t loc, BasicBlock *target);

  /// Add a jump table switch to relocation list.
  void registerSwitchImm(offset_t loc, SwitchImmInst *target);

  /// Resolve all exception handlers.
  void resolveExceptionHandlers();

  /// Generate the jump table into the final representation.
  void generateJumpTable();

  /// Conveniently extract file/line/column from a SMLoc.
  /// Associate the source map script ID with the filename ID in the Module.
  bool getDebugSourceLocation(
      SourceErrorManager &manager,
      SMLoc loc,
      DebugSourceLocation *out);

  /// Given a bufferID, find or add the corresponding filename and source map
  /// IDs in BytecodeFunctionGenerator and return them.
  FileAndSourceMapId obtainFileAndSourceMapId(
      SourceErrorManager &sm,
      unsigned bufId);

  /// Add applicable debug info.
  void addDebugSourceLocationInfo(SourceMapGenerator *outSourceMap);
  void addDebugTextifiedCalleeInfo();

  /// Populate Property caching metadata to the function.
  void populatePropertyCachingInfo();

  /// Emit instructions at the entry block to handle several special cases.
  void initialize();

  /// Emit a mov, or none if it would be a no-op.
  void emitMovIfNeeded(param_t dest, param_t src);

  /// Emit an Unreachable opcode in debug builds, otherwise do nothing.
  void emitUnreachableIfDebug();

  /// In debug mode, assert that parameters have been correctly allocated.
  void verifyCall(CallInst *Inst);

  /// The last emitted property cache index.
  uint8_t lastPropertyReadCacheIndex_{0};
  uint8_t lastPropertyWriteCacheIndex_{0};

  /// Map from property name to the read/write cache index for that name.
  llvh::DenseMap<unsigned /* name */, uint8_t> propertyReadCacheIndexForId_;
  llvh::DenseMap<unsigned /* name */, uint8_t> propertyWriteCacheIndexForId_;

  /// Compute and return the index to use for caching the read/write of a
  /// property with the given identifier name.
  uint8_t acquirePropertyReadCacheIndex(unsigned id);
  uint8_t acquirePropertyWriteCacheIndex(unsigned id);

  /// A cache mapping from buffer ID to filelname+source map.
  FileAndSourceMapIdCache &fileAndSourceMapIdCache_;
  /// To avoid performing a hash lookup in most cases, cache the last found
  /// buffer ID and file and source map IDs.
  FileAndSourceMapIdCache::value_type *lastFoundFileSourceMapId_ = nullptr;

 public:
  /// C'tor.
  /// \p F is the function that we are constructing.
  /// \p OS is the output stream.
  HBCISel(
      Function *F,
      BytecodeFunctionGenerator *BCFGen,
      HVMRegisterAllocator &RA,
      FunctionScopeAnalysis &scopeAnalysis,
      ScopeRegisterAnalysis &SRA,
      const BytecodeGenerationOptions &options,
      FileAndSourceMapIdCache &debugIdCache)
      : F_(F),
        BCFGen_(BCFGen),
        RA_(RA),
        scopeAnalysis_(scopeAnalysis),
        SRA_(SRA),
        bytecodeGenerationOptions_(options),
        fileAndSourceMapIdCache_(debugIdCache) {
    protoIdent_ = F->getContext().getIdentifier("__proto__");
  }

/// This is the header declaration for all of the methods that emit opcodes
/// for specific high-level IR instructions.
#define INCLUDE_HBC_INSTRS
#define DEF_VALUE(CLASS, PARENT) \
  void generate##CLASS(CLASS *Inst, BasicBlock *next);
#include "hermes/IR/ValueKinds.def"
#undef DEF_VALUE
#undef MARK_VALUE
#undef INCLUDE_HBC_INSTRS

  /// Generate bytecode for the basic block \p BB with the knowledge that the
  /// next basic block that we'll generate after this block is \p next. If \p BB
  /// is the last basic block then \p next is null.
  void generate(BasicBlock *BB, BasicBlock *next);

  /// Generate bytecode for the instruction \p II.
  void generate(Instruction *ii, BasicBlock *next);

  /// Generate the bytecode stream for the function.
  void generate(SourceMapGenerator *outSourceMap);
};

} // namespace hbc
} // namespace hermes

#endif
