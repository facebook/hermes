/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/HBC.h"

#include "hermes/BCGen/BCOpt.h"
#include "hermes/BCGen/HBC/BytecodeGenerator.h"
#include "hermes/BCGen/HBC/BytecodeStream.h"
#include "hermes/BCGen/HBC/ISel.h"
#include "hermes/BCGen/HBC/Passes.h"
#include "hermes/BCGen/HBC/Passes/FuncCallNOpts.h"
#include "hermes/BCGen/HBC/Passes/InsertProfilePoint.h"
#include "hermes/BCGen/HBC/Passes/LowerBuiltinCalls.h"
#include "hermes/BCGen/HBC/Passes/OptEnvironmentInit.h"
#include "hermes/BCGen/HBC/TraverseLiteralStrings.h"
#include "hermes/BCGen/Lowering.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/IRVerifier.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Optimizer/PassManager/PassManager.h"
#include "hermes/Support/PerfSection.h"
#include "hermes/Support/UTF8.h"

#define DEBUG_TYPE "hbc-backend"

using namespace hermes;
using namespace hbc;

namespace {

// If we have less than this number of instructions in a Function, and we're
// not compiling in optimized mode, take shortcuts during register allocation.
// 250 was chosen so that registers will fit in a single byte even after some
// have been reserved for function parameters.
const unsigned kFastRegisterAllocationThreshold = 250;

// If register allocation is expected to take more than this number of bytes of
// RAM, and we're not trying to optimize, use a simpler pass to reduce compile
// time memory usage.
const uint64_t kRegisterAllocationMemoryLimit = 10L * 1024 * 1024;

void lowerIR(Module *M, const BytecodeGenerationOptions &options) {
  if (M->isLowered())
    return;

  PassManager PM{M->getContext().getCodeGenerationSettings()};
  PM.addPass<LowerLoadStoreFrameInst>();
  if (options.optimizationEnabled) {
    // OptEnvironmentInit needs to run before LowerConstants.
    PM.addPass<OptEnvironmentInit>();
  }
  // LowerExponentiationOperator needs to run before LowerBuiltinCalls because
  // it introduces calls to HermesInternal.
  PM.addPass<LowerExponentiationOperator>();
  // LowerBuiltinCalls needs to run before the rest of the lowering.
  PM.addPass<LowerBuiltinCalls>();
  // It is important to run LowerNumericProperties before LoadConstants
  // as LowerNumericProperties could generate new constants.
  PM.addPass<LowerNumericProperties>();
  // Lower AllocObjectLiteral into a mixture of HBCAllocObjectFromBufferInst,
  // AllocObjectInst, StoreNewOwnPropertyInst and StorePropertyInst.
  PM.addPass<LowerAllocObjectLiteral>();
  PM.addPass<LowerConstruction>();
  PM.addPass<LowerArgumentsArray>();
  PM.addPass<LimitAllocArray>(UINT16_MAX);
  PM.addPass<DedupReifyArguments>();
  PM.addPass<LowerSwitchIntoJumpTables>();
  PM.addPass<SwitchLowering>();
  PM.addPass<LoadConstants>(options.optimizationEnabled);
  PM.addPass<LoadParameters>();
  if (options.optimizationEnabled) {
    // Lowers AllocObjects and its sequential literal properties into a single
    // HBCAllocObjectFromBufferInst
    PM.addPass<LowerAllocObject>();
    // Reduce comparison and conditional jump to single comparison jump
    PM.addPass<LowerCondBranch>();
    // Turn Calls into CallNs.
    PM.addPass<FuncCallNOpts>();
    // Move loads to child blocks if possible.
    PM.addCodeMotion();
    // Eliminate common HBCLoadConstInsts.
    PM.addCSE();
    // Drop unused HBCLoadParamInsts.
    PM.addDCE();
  }

  // Move StartGenerator instructions to the start of functions.
  PM.addHoistStartGenerator();

  PM.run(M);
  M->setLowered(true);

  if (options.verifyIR &&
      verifyModule(*M, &llvh::errs(), VerificationMode::IR_VALID)) {
    M->dump();
    llvm_unreachable("IR verification failed");
  }
}

/// Used in delta optimizing mode.
/// \return a UniquingStringLiteralAccumulator seeded with strings  from a
/// bytecode provider \p bcProvider.
UniquingStringLiteralAccumulator stringAccumulatorFromBCProvider(
    const BCProviderBase &bcProvider) {
  uint32_t count = bcProvider.getStringCount();

  std::vector<StringTableEntry> entries;
  std::vector<bool> isIdentifier;

  entries.reserve(count);
  isIdentifier.reserve(count);

  {
    unsigned i = 0;
    for (auto kindEntry : bcProvider.getStringKinds()) {
      bool isIdentRun = kindEntry.kind() != StringKind::String;
      for (unsigned j = 0; j < kindEntry.count(); ++j, ++i) {
        entries.push_back(bcProvider.getStringTableEntry(i));
        isIdentifier.push_back(isIdentRun);
      }
    }

    assert(i == count && "Did not initialise every string");
  }

  auto strStorage = bcProvider.getStringStorage();
  ConsecutiveStringStorage css{std::move(entries), strStorage.vec()};

  return UniquingStringLiteralAccumulator{
      std::move(css), std::move(isIdentifier)};
}

/// A container that deduplicates byte sequences, while keeping the original
/// insertion order with the duplicates. Note: strings are used as a
/// representation for code reuse and simplicity, but the contents are meant to
/// be interpreted as unsigned bytes.
/// It maintains:
/// - a StringSetVector containing the uniqued strings.
/// - a vector mapping each originally inserted string in order to an index in
///   the StringSetVector.
/// This is a specialized class used only by \c LiteralBufferBuilder.
/// NOTE: We use std::string instead of std::vector<uint8_t> for code reuse.
class UniquedStringVector {
 public:
  /// Append a string.
  void push_back(llvh::StringRef str) {
    indexInSet_.push_back(set_.insert(str));
  }

  /// \return how many strings the vector contains, in other words, how many
  ///     times \c push_back() was called.
  size_t size() const {
    return indexInSet_.size();
  }

  /// \return the begin iterator over the uniqued set of strings in insertion
  ///  order.
  StringSetVector::const_iterator beginSet() const {
    return set_.begin();
  }
  /// \return the end iterator over the uniqued set of strings in insertion
  ///  order.
  StringSetVector::const_iterator endSet() const {
    return set_.end();
  }

  /// \return the index in the uniqued set corresponding to the insertion index
  ///     \p insertionIndex.
  uint32_t indexInSet(size_t insertionIndex) const {
    return indexInSet_[insertionIndex];
  }

 private:
  /// The uniqued string set in insertion order.
  StringSetVector set_{};
  /// Index into the set of each original non-deduplicated string in insertion
  /// order.
  std::vector<uint32_t> indexInSet_{};
};

/// A utility class which collects all serialized literals from a module,
/// optionally deduplicates them, and installs them in the
/// BytecodeModuleGenerator.
class LiteralBufferBuilder {
 public:
  /// Constructor.
  /// \param m the IR module to process.
  /// \param shouldVisitFunction a predicate indicating whether a function
  ///     should be processed or not. (In some cases like segment splitting we
  ///     want to exclude part of the module.)
  /// \param bmGen the BytecodeModuleGenerator to use.
  /// \param optimize whether to deduplicate the serialized literals.
  LiteralBufferBuilder(
      Module *m,
      const std::function<bool(Function const *)> &shouldVisitFunction,
      BytecodeModuleGenerator &bmGen,
      bool optimize)
      : M_(m),
        shouldVisitFunction_(shouldVisitFunction),
        bmGen_(bmGen),
        optimize_(optimize),
        literalGenerator_(bmGen) {}

  /// Do everything: collect the literals, optionally deduplicate them, install
  /// them in the BytecodeModuleGenerator.
  void generate();

 private:
  /// Traverse the module, skipping functions that should not be visited,
  /// and collect all serialized array and object literals and the corresponding
  /// instruction.
  void traverse();

  // Serialization handlers for different instructions.

  void serializeLiteralFor(AllocArrayInst *AAI);
  void serializeLiteralFor(HBCAllocObjectFromBufferInst *AOFB);

  /// Serialize the the input literals \p elements into the UniquedStringVector
  /// \p dest.
  /// \p isKeyBuffer: whether this is generating object literal key buffer or
  /// not.
  void serializeInto(
      UniquedStringVector &dest,
      llvh::ArrayRef<Literal *> elements,
      bool isKeyBuffer);

 private:
  /// The IR module to process.
  Module *const M_;
  /// A predicate indicating whether a function should be processed or not. (In
  /// some cases like segment splitting we want to exclude part of the module.)
  const std::function<bool(const Function *)> &shouldVisitFunction_;
  /// The BytecodeModuleGenerator to use.
  BytecodeModuleGenerator &bmGen_;
  /// Whether to deduplicate the serialized literals.
  bool const optimize_;

  /// The stateless generator object.
  SerializedLiteralGenerator literalGenerator_;

  /// Temporary buffer to serialize literals into. We keep it around instead
  /// of allocating a new one every time.
  std::vector<unsigned char> tempBuffer_{};

  /// Each element is a serialized array literal.
  UniquedStringVector arrays_{};

  /// Each element records the instruction whose literal was serialized at the
  /// corresponding index in \c arrays_.
  std::vector<const Instruction *> arraysInst_{};

  /// Each element is the keys portion of a serialized object literal.
  UniquedStringVector objKeys_{};

  /// Each element is the values portion of a serialized object literal.
  UniquedStringVector objVals_{};

  /// Each element records the instruction whose literal was serialized at the
  /// corresponding index in \c objKeys_/objVals_.
  std::vector<const Instruction *> objInst_{};
};

void LiteralBufferBuilder::generate() {
  traverse();

  // Construct the serialized storage, optionally optimizing it.
  ConsecutiveStringStorage arrayStorage{
      arrays_.beginSet(), arrays_.endSet(), std::true_type{}, optimize_};
  ConsecutiveStringStorage keyStorage{
      objKeys_.beginSet(), objKeys_.endSet(), std::true_type{}, optimize_};
  ConsecutiveStringStorage valStorage{
      objVals_.beginSet(), objVals_.endSet(), std::true_type{}, optimize_};

  // Populate the offset map.
  BytecodeModuleGenerator::LiteralOffsetMapTy literalOffsetMap{};

  // Visit all array literals.
  auto arrayView = const_cast<const ConsecutiveStringStorage &>(arrayStorage)
                       .getStringTableView();
  for (size_t i = 0, e = arraysInst_.size(); i != e; ++i) {
    assert(
        literalOffsetMap.count(arraysInst_[i]) == 0 &&
        "instruction literal can't be serialized twice");
    uint32_t arrayIndexInSet = arrays_.indexInSet(i);
    literalOffsetMap[arraysInst_[i]] = BytecodeModuleGenerator::LiteralOffset{
        arrayView[arrayIndexInSet].getOffset(), UINT32_MAX};
  }

  // Visit all object literals - they are split in two buffers.
  auto keyView = const_cast<const ConsecutiveStringStorage &>(keyStorage)
                     .getStringTableView();
  auto valView = const_cast<const ConsecutiveStringStorage &>(valStorage)
                     .getStringTableView();
  for (size_t i = 0, e = objInst_.size(); i != e; ++i) {
    assert(
        literalOffsetMap.count(objInst_[i]) == 0 &&
        "instruction literal can't be serialized twice");
    uint32_t keyIndexInSet = objKeys_.indexInSet(i);
    uint32_t valIndexInSet = objVals_.indexInSet(i);
    literalOffsetMap[objInst_[i]] = BytecodeModuleGenerator::LiteralOffset{
        keyView[keyIndexInSet].getOffset(), valView[valIndexInSet].getOffset()};
  }

  bmGen_.initializeSerializedLiterals(
      arrayStorage.acquireStringStorage(),
      keyStorage.acquireStringStorage(),
      valStorage.acquireStringStorage(),
      std::move(literalOffsetMap));
}

void LiteralBufferBuilder::traverse() {
  for (auto &F : *M_) {
    if (!shouldVisitFunction_(&F))
      continue;

    for (auto &BB : F) {
      for (auto &I : BB) {
        if (auto *AAI = dyn_cast<AllocArrayInst>(&I)) {
          serializeLiteralFor(AAI);
        } else if (auto *AOFB = dyn_cast<HBCAllocObjectFromBufferInst>(&I)) {
          serializeLiteralFor(AOFB);
        }
      }
    }
  }
}

void LiteralBufferBuilder::serializeInto(
    UniquedStringVector &dest,
    llvh::ArrayRef<Literal *> elements,
    bool isKeyBuffer) {
  tempBuffer_.clear();
  literalGenerator_.serializeBuffer(elements, tempBuffer_, isKeyBuffer);
  dest.push_back(
      llvh::StringRef((const char *)tempBuffer_.data(), tempBuffer_.size()));
}

void LiteralBufferBuilder::serializeLiteralFor(AllocArrayInst *AAI) {
  SmallVector<Literal *, 8> elements;
  for (unsigned i = 0, e = AAI->getElementCount(); i < e; ++i) {
    elements.push_back(cast<Literal>(AAI->getArrayElement(i)));
  }

  serializeInto(arrays_, elements, false);
  arraysInst_.push_back(AAI);
}

void LiteralBufferBuilder::serializeLiteralFor(
    HBCAllocObjectFromBufferInst *AOFB) {
  unsigned e = AOFB->getKeyValuePairCount();
  if (!e)
    return;

  SmallVector<Literal *, 8> objKeys;
  SmallVector<Literal *, 8> objVals;
  for (unsigned ind = 0; ind != e; ++ind) {
    auto keyValuePair = AOFB->getKeyValuePair(ind);
    objKeys.push_back(cast<Literal>(keyValuePair.first));
    objVals.push_back(cast<Literal>(keyValuePair.second));
  }

  serializeInto(objKeys_, objKeys, true);
  serializeInto(objVals_, objVals, false);
  assert(
      objKeys_.size() == objVals_.size() &&
      "objKeys_ and objVals_ must be the same size");
  objInst_.push_back(AOFB);
}
}; // namespace

std::unique_ptr<BytecodeModule> hbc::generateBytecodeModule(
    Module *M,
    Function *entryPoint,
    const BytecodeGenerationOptions &options,
    hermes::OptValue<uint32_t> segment,
    SourceMapGenerator *sourceMapGen,
    std::unique_ptr<BCProviderBase> baseBCProvider) {
  return generateBytecodeModule(
      M,
      entryPoint,
      entryPoint,
      options,
      segment,
      sourceMapGen,
      std::move(baseBCProvider));
}

std::unique_ptr<BytecodeModule> hbc::generateBytecodeModule(
    Module *M,
    Function *lexicalTopLevel,
    Function *entryPoint,
    const BytecodeGenerationOptions &options,
    hermes::OptValue<uint32_t> segment,
    SourceMapGenerator *sourceMapGen,
    std::unique_ptr<BCProviderBase> baseBCProvider) {
  PerfSection perf("Bytecode Generation");
  lowerIR(M, options);

  if (options.format == DumpLIR)
    M->dump();

  BytecodeModuleGenerator BMGen(options);

  if (segment) {
    BMGen.setSegmentID(*segment);
  }
  // Empty if all functions should be generated (i.e. bundle splitting was not
  // requested).
  llvh::DenseSet<Function *> functionsToGenerate = segment
      ? M->getFunctionsInSegment(*segment)
      : llvh::DenseSet<Function *>{};

  /// \return true if we should generate function \p f.
  std::function<bool(const Function *)> shouldGenerate;
  if (segment) {
    shouldGenerate = [entryPoint, &functionsToGenerate](const Function *f) {
      return f == entryPoint || functionsToGenerate.count(f) > 0;
    };
  } else {
    shouldGenerate = [](const Function *) { return true; };
  }

  { // Collect all the strings in the bytecode module into a storage.
    // If we are in delta optimizing mode, start with the string storage from
    // our base bytecode provider.
    auto strings = baseBCProvider
        ? stringAccumulatorFromBCProvider(*baseBCProvider)
        : UniquingStringLiteralAccumulator{};

    auto addStringOrIdent = [&strings](llvh::StringRef str, bool isIdentifier) {
      strings.addString(str, isIdentifier);
    };

    auto addString = [&strings](llvh::StringRef str) {
      strings.addString(str, /* isIdentifier */ false);
    };

    traverseLiteralStrings(M, shouldGenerate, addStringOrIdent);

    if (options.stripFunctionNames) {
      addString(kStrippedFunctionName);
    }
    traverseFunctions(M, shouldGenerate, addString, options.stripFunctionNames);

    if (!M->getCJSModulesResolved()) {
      traverseCJSModuleNames(M, shouldGenerate, addString);
    }

    BMGen.initializeStringTable(UniquingStringLiteralAccumulator::toTable(
        std::move(strings), options.optimizationEnabled));
  }

  // Generate the serialized literal buffers.
  {
    LiteralBufferBuilder litBuilder{
        M, shouldGenerate, BMGen, options.optimizationEnabled};
    litBuilder.generate();
  }

  // Add each function to BMGen so that each function has a unique ID.
  for (auto &F : *M) {
    if (!shouldGenerate(&F)) {
      continue;
    }

    unsigned index = BMGen.addFunction(&F);
    if (&F == entryPoint) {
      BMGen.setEntryPointIndex(index);
    }

    auto *cjsModule = M->findCJSModule(&F);
    if (cjsModule) {
      if (M->getCJSModulesResolved()) {
        BMGen.addCJSModuleStatic(cjsModule->id, index);
      } else {
        BMGen.addCJSModule(index, BMGen.getStringID(cjsModule->filename.str()));
      }
    }

    // Add entries to function source table for non-default source.
    if (!F.isGlobalScope()) {
      if (auto source = F.getSourceRepresentationStr()) {
        BMGen.addFunctionSource(index, BMGen.getStringID(*source));
      }
    }
  }
  assert(BMGen.getEntryPointIndex() != -1 && "Entry point not added");

  // Construct the relative function scope depth map.
  FunctionScopeAnalysis scopeAnalysis{lexicalTopLevel};

  // Allow reusing the debug cache between functions
  HBCISelDebugCache debugCache;

  // Bytecode generation for each function.
  for (auto &F : *M) {
    if (!shouldGenerate(&F)) {
      continue;
    }

    std::unique_ptr<BytecodeFunctionGenerator> funcGen;

    if (F.isLazy()) {
      funcGen = BytecodeFunctionGenerator::create(BMGen, 0);
    } else {
      HVMRegisterAllocator RA(&F);
      if (!options.optimizationEnabled) {
        RA.setFastPassThreshold(kFastRegisterAllocationThreshold);
        RA.setMemoryLimit(kRegisterAllocationMemoryLimit);
      }
      PostOrderAnalysis PO(&F);
      /// The order of the blocks is reverse-post-order, which is a simply
      /// topological sort.
      llvh::SmallVector<BasicBlock *, 16> order(PO.rbegin(), PO.rend());
      RA.allocate(order);

      if (options.format == DumpRA) {
        RA.dump();
      }

      PassManager PM{M->getContext().getCodeGenerationSettings()};
      PM.addPass<LowerStoreInstrs>(RA);
      PM.addPass<LowerCalls>(RA);
      if (options.optimizationEnabled) {
        PM.addPass<MovElimination>(RA);
        PM.addPass<RecreateCheapValues>(RA);
        PM.addPass<LoadConstantValueNumbering>(RA);
      }
      PM.addPass<SpillRegisters>(RA);
      if (options.basicBlockProfiling) {
        // Insert after all other passes so that it sees final basic block
        // list.
        PM.addPass<InsertProfilePoint>();
      }
      PM.run(&F);

      if (options.format == DumpLRA)
        RA.dump();

      if (options.format == DumpPostRA)
        F.dump();

      funcGen =
          BytecodeFunctionGenerator::create(BMGen, RA.getMaxRegisterUsage());
      HBCISel hbciSel(&F, funcGen.get(), RA, scopeAnalysis, options);
      hbciSel.populateDebugCache(debugCache);
      hbciSel.generate(sourceMapGen);
      debugCache = hbciSel.getDebugCache();
    }

    if (funcGen->hasEncodingError()) {
      M->getContext().getSourceErrorManager().error(
          F.getSourceRange().Start, "Error encoding bytecode");
      return nullptr;
    }
    BMGen.setFunctionGenerator(&F, std::move(funcGen));
  }

  return BMGen.generate();
}

std::unique_ptr<BytecodeModule> hbc::generateBytecode(
    Module *M,
    raw_ostream &OS,
    const BytecodeGenerationOptions &options,
    const SHA1 &sourceHash,
    hermes::OptValue<uint32_t> segment,
    SourceMapGenerator *sourceMapGen,
    std::unique_ptr<BCProviderBase> baseBCProvider) {
  auto BM = generateBytecodeModule(
      M,
      M->getTopLevelFunction(),
      options,
      segment,
      sourceMapGen,
      std::move(baseBCProvider));

  if (!BM) {
    return {};
  }

  if (options.format == OutputFormatKind::EmitBundle) {
    assert(BM != nullptr);
    BytecodeSerializer BS{OS, options};
    BS.serialize(*BM, sourceHash);
  }
  // Now that the BytecodeFunctions know their offsets into the stream, we can
  // populate the source map.
  if (sourceMapGen)
    BM->populateSourceMap(sourceMapGen);
  return BM;
}

#undef DEBUG_TYPE
