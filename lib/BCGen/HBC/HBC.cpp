/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
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
  PassManager PM;
  PM.addPass(new LowerLoadStoreFrameInst());
  if (options.optimizationEnabled) {
    // OptEnvironmentInit needs to run before LowerConstants.
    PM.addPass(new OptEnvironmentInit());
  }
  /// LowerBuiltinCalls needs to run before the rest of the lowering.
  PM.addPass(new LowerBuiltinCalls());
  // It is important to run LowerNumericProperties before LoadConstants
  // as LowerNumericProperties could generate new constants.
  PM.addPass(new LowerNumericProperties());
  PM.addPass(new LowerConstruction());
  PM.addPass(new LowerArgumentsArray());
  PM.addPass(new LimitAllocArray(UINT16_MAX));
  PM.addPass(new DedupReifyArguments());
  PM.addPass(new LowerSwitchIntoJumpTables());
  PM.addPass(new SwitchLowering());
  PM.addPass(new LoadConstants(options.optimizationEnabled));
  PM.addPass(new LoadParameters());
  if (options.optimizationEnabled) {
    // Lowers AllocObjects and its sequential literal properties into a single
    // HBCAllocObjectFromBufferInst
    PM.addPass(new LowerAllocObject());
    // Reduce comparison and conditional jump to single comparison jump
    PM.addPass(new LowerCondBranch());
    // Turn Calls into CallNs.
    PM.addPass(new FuncCallNOpts());
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

  if (options.verifyIR &&
      verifyModule(*M, &llvm::errs(), VerificationMode::IR_VALID)) {
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

  return UniquingStringLiteralAccumulator{std::move(css),
                                          std::move(isIdentifier)};
}

} // namespace

std::unique_ptr<BytecodeModule> hbc::generateBytecodeModule(
    Module *M,
    Function *entryPoint,
    const BytecodeGenerationOptions &options,
    OptValue<Context::SegmentRange> range,
    SourceMapGenerator *sourceMapGen,
    std::unique_ptr<BCProviderBase> baseBCProvider) {
  PerfSection perf("Bytecode Generation");
  lowerIR(M, options);

  if (options.format == DumpLIR)
    M->dump();

  BytecodeModuleGenerator BMGen(options);

  if (range) {
    BMGen.setCJSModuleOffset(range->first);
  }

  // Empty if all functions should be generated (i.e. bundle splitting was not
  // requested).
  llvm::DenseSet<Function *> functionsToGenerate =
      range ? M->getFunctionsInSegment(*range) : llvm::DenseSet<Function *>{};

  /// \return true if we should generate function \p f.
  std::function<bool(const Function *)> shouldGenerate;
  if (range) {
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

    auto addStringOrIdent = [&strings](llvm::StringRef str, bool isIdentifier) {
      strings.addString(str, isIdentifier);
    };

    auto addString = [&strings](llvm::StringRef str) {
      strings.addString(str, /* isIdentifier */ false);
    };

    traverseLiteralStrings(M, shouldGenerate, addStringOrIdent);

    if (options.stripFunctionNames) {
      addString(kStrippedFunctionName);
    } else {
      traverseFunctionNames(M, shouldGenerate, addString);
    }

    if (!M->getCJSModulesResolved()) {
      traverseCJSModuleNames(M, shouldGenerate, addString);
    }

    BMGen.initializeStringTable(UniquingStringLiteralAccumulator::toTable(
        std::move(strings), options.optimizationEnabled));
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
  }
  assert(BMGen.getEntryPointIndex() != -1 && "Entry point not added");

  // Construct the relative function scope depth map.
  FunctionScopeAnalysis scopeAnalysis{entryPoint};
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
      llvm::SmallVector<BasicBlock *, 16> order(PO.rbegin(), PO.rend());
      RA.allocate(order);

      if (options.format == DumpRA) {
        RA.dump();
      }

      PassManager PM;
      PM.addPass(new LowerStoreInstrs(RA));
      PM.addPass(new LowerCalls(RA));
      if (options.optimizationEnabled) {
        PM.addPass(new MovElimination(RA));
        PM.addPass(new RecreateCheapValues(RA));
        PM.addPass(new LoadConstantValueNumbering(RA));
      }
      PM.addPass(new SpillRegisters(RA));
      if (options.basicBlockProfiling) {
        // Insert after all other passes so that it sees final basic block
        // list.
        PM.addPass(new InsertProfilePoint());
      }
      PM.run(&F);

      if (options.format == DumpLRA)
        RA.dump();

      if (options.format == DumpPostRA)
        F.dump();

      funcGen =
          BytecodeFunctionGenerator::create(BMGen, RA.getMaxRegisterUsage());
      HBCISel hbciSel(&F, funcGen.get(), RA, scopeAnalysis);
      hbciSel.generate(sourceMapGen);
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
    OptValue<Context::SegmentRange> range,
    SourceMapGenerator *sourceMapGen,
    std::unique_ptr<BCProviderBase> baseBCProvider) {
  auto BM = generateBytecodeModule(
      M,
      M->getTopLevelFunction(),
      options,
      range,
      sourceMapGen,
      std::move(baseBCProvider));
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
