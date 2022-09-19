/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/SH/SH.h"

#include "hermes/BCGen/HBC/Passes.h"
#include "hermes/BCGen/Lowering.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Support/UTF8.h"

using namespace hermes;

namespace {
/// Generates the correct label for BasicBlock \p B based on \p bbMap and
/// outputs it through \p OS.
void generateBasicBlockLabel(
    BasicBlock *B,
    llvh::raw_ostream &OS,
    const llvh::DenseMap<BasicBlock *, unsigned> &bbMap) {
  OS << "L" << bbMap.find(B)->second;
}

/// Constructs a map from basic blocks to their catch instruction
void constructCatchMap(
    llvh::DenseMap<BasicBlock *, CatchInst *> &catchMap,
    Function &F) {
  // Create catchInfoMap entry for every CatchInst
  CatchInfoMap catchInfoMap{};
  for (auto &B : F)
    for (auto &I : B)
      if (auto CI = llvh::dyn_cast<CatchInst>(&I))
        catchInfoMap[CI] = CatchCoverageInfo();

  // Populate catchInfoMap
  llvh::SmallVector<CatchInst *, 4> aliveCatches{};
  llvh::SmallPtrSet<BasicBlock *, 32> visited{};
  hermes::constructCatchMap(
      &F, catchInfoMap, aliveCatches, visited, &F.front(), 1024);

  // Invert catchInfoMap into a catchMap from BB to CatchInst
  // Make sure to always invert the CatchInst of highest depth
  llvh::DenseMap<BasicBlock *, uint32_t> maxDepths{};
  for (auto it : catchInfoMap) {
    auto catchInst = it.first;
    auto depth = it.second.depth;
    for (auto BB : it.second.coveredBlockList) {
      auto depthIt = maxDepths.find(BB);
      if (depthIt == maxDepths.end() || depth > depthIt->second) {
        catchMap[BB] = catchInst;
        maxDepths[BB] = depth;
      }
    }
  }
}

struct ModuleGen {
  /// A map from functions to unique numbers for identificatio
  llvh::DenseMap<Function *, unsigned> funcMap;

  /// Generates the correct label for BasicBlock \p B based on \p bbMap and
  /// outputs it through \p OS. If the JS function name is valid C then it will
  /// be appended to the name for searchability, otherwise just the unique
  /// number alone will be used.
  void generateFunctionLabel(Function *F, llvh::raw_ostream &OS) {
    OS << "_" << funcMap.find(F)->second;

    auto name = F->getInternalNameStr();
    for (auto c : name) {
      if (!(isalnum(c) || c == '_')) {
        return;
      }
    }

    OS << "_" << name;
  }
};

class InstrGen {
 public:
  /// \p os is the output stream
  /// \p ra is the pre-ran register allocator for the current function
  /// \p envSize is the environment size of the current function
  InstrGen(
      llvh::raw_ostream &os,
      hbc::HVMRegisterAllocator &ra,
      const llvh::DenseMap<BasicBlock *, unsigned> &bbMap,
      Function &F,
      ModuleGen &moduleGen,
      FunctionScopeAnalysis &scopeAnalysis,
      llvh::DenseMap<BasicBlock *, CatchInst *> &catchMap,
      unsigned envSize,
      uint32_t &nextCacheIdx)
      : os_(os),
        ra_(ra),
        bbMap_(bbMap),
        moduleGen_(moduleGen),
        scopeAnalysis_(scopeAnalysis),
        catchMap_(catchMap),
        envSize_(envSize),
        nextCacheIdx_(nextCacheIdx) {}

  /// Converts Instruction \p I into valid C code and outputs it through the
  /// ostream.
  void generate(Instruction &I) {
    switch (I.getKind()) {
#define INCLUDE_HBC_INSTRS
#define DEF_VALUE(CLASS, PARENT) \
  case ValueKind::CLASS##Kind:   \
    return generate##CLASS(*cast<CLASS>(&I));
#include "hermes/IR/Instrs.def"
      default:
        llvm_unreachable("Invalid kind");
    }
  }

 private:
  /// The ostream used to output C code
  llvh::raw_ostream &os_;

  /// The register allocator that was created for the current function
  hbc::HVMRegisterAllocator &ra_;

  /// A map from basic blocks to unique numbers for identification
  const llvh::DenseMap<BasicBlock *, unsigned> &bbMap_;

  /// The state for the module currently being emitted.
  ModuleGen &moduleGen_;

  /// Function scope analysis of the current module
  FunctionScopeAnalysis &scopeAnalysis_;

  /// Map from basic blocks to their catch instruction
  llvh::DenseMap<BasicBlock *, CatchInst *> catchMap_;

  /// The size of this functions environment
  unsigned envSize_;

  /// Starts out at 0 and increments every time a cache index is used
  uint32_t &nextCacheIdx_;

  void generateInstruction(Instruction &inst) {
    hermes_fatal("Unimplemented instruction Instruction");
  }
  void generateSingleOperandInst(SingleOperandInst &inst) {
    hermes_fatal("Unimplemented instruction SingleOperandInst");
  }
  void generateAddEmptyStringInst(AddEmptyStringInst &inst) {
    hermes_fatal("Unimplemented instruction AddEmptyStringInst");
  }
  void generateAsNumberInst(AsNumberInst &inst) {
    hermes_fatal("Unimplemented instruction AsNumberInst");
  }
  void generateAsNumericInst(AsNumericInst &inst) {
    hermes_fatal("Unimplemented instruction AsNumericInst");
  }
  void generateAsInt32Inst(AsInt32Inst &inst) {
    hermes_fatal("Unimplemented instruction AsInt32Inst");
  }
  void generateLoadStackInst(LoadStackInst &inst) {
    hermes_fatal("Unimplemented instruction LoadStackInst");
  }
  void generateMovInst(MovInst &inst) {
    hermes_fatal("Unimplemented instruction MovInst");
  }
  void generateImplicitMovInst(ImplicitMovInst &inst) {
    hermes_fatal("Unimplemented instruction ImplicitMovInst");
  }
  void generateCoerceThisNSInst(CoerceThisNSInst &inst) {
    hermes_fatal("Unimplemented instruction CoerceThisNSInst");
  }
  void generateUnaryOperatorInst(UnaryOperatorInst &inst) {
    hermes_fatal("Unimplemented instruction UnaryOperatorInst");
  }
  void generateDirectEvalInst(DirectEvalInst &inst) {
    hermes_fatal("Unimplemented instruction DirectEvalInst");
  }
  void generateLoadFrameInst(LoadFrameInst &inst) {
    hermes_fatal("Unimplemented instruction LoadFrameInst");
  }
  void generateHBCLoadConstInst(HBCLoadConstInst &inst) {
    hermes_fatal("Unimplemented instruction HBCLoadConstInst");
  }
  void generateHBCLoadParamInst(HBCLoadParamInst &inst) {
    hermes_fatal("Unimplemented instruction HBCLoadParamInst");
  }
  void generateHBCResolveEnvironment(HBCResolveEnvironment &inst) {
    hermes_fatal("Unimplemented instruction HBCResolveEnvironment");
  }
  void generateHBCGetArgumentsLengthInst(HBCGetArgumentsLengthInst &inst) {
    hermes_fatal("Unimplemented instruction HBCGetArgumentsLengthInst");
  }
  void generateHBCReifyArgumentsInst(HBCReifyArgumentsInst &inst) {
    hermes_fatal("Unimplemented instruction HBCReifyArgumentsInst");
  }
  void generateHBCSpillMovInst(HBCSpillMovInst &inst) {
    hermes_fatal("Unimplemented instruction HBCSpillMovInst");
  }
  void generatePhiInst(PhiInst &inst) {
    hermes_fatal("Unimplemented instruction PhiInst");
  }
  void generateBinaryOperatorInst(BinaryOperatorInst &inst) {
    hermes_fatal("Unimplemented instruction BinaryOperatorInst");
  }
  void generateStorePropertyInst(StorePropertyInst &inst) {
    hermes_fatal("Unimplemented instruction StorePropertyInst");
  }
  void generateTryStoreGlobalPropertyInst(TryStoreGlobalPropertyInst &inst) {
    hermes_fatal("Unimplemented instruction TryStoreGlobalPropertyInst");
  }
  void generateStoreOwnPropertyInst(StoreOwnPropertyInst &inst) {
    hermes_fatal("Unimplemented instruction StoreOwnPropertyInst");
  }
  void generateStoreNewOwnPropertyInst(StoreNewOwnPropertyInst &inst) {
    hermes_fatal("Unimplemented instruction StoreNewOwnPropertyInst");
  }
  void generateStoreGetterSetterInst(StoreGetterSetterInst &inst) {
    hermes_fatal("Unimplemented instruction StoreGetterSetterInst");
  }
  void generateDeletePropertyInst(DeletePropertyInst &inst) {
    hermes_fatal("Unimplemented instruction DeletePropertyInst");
  }
  void generateLoadPropertyInst(LoadPropertyInst &inst) {
    hermes_fatal("Unimplemented instruction LoadPropertyInst");
  }
  void generateTryLoadGlobalPropertyInst(TryLoadGlobalPropertyInst &inst) {
    hermes_fatal("Unimplemented instruction TryLoadGlobalPropertyInst");
  }
  void generateStoreStackInst(StoreStackInst &inst) {
    hermes_fatal("Unimplemented instruction StoreStackInst");
  }
  void generateStoreFrameInst(StoreFrameInst &inst) {
    hermes_fatal("Unimplemented instruction StoreFrameInst");
  }
  void generateAllocStackInst(AllocStackInst &inst) {
    hermes_fatal("Unimplemented instruction AllocStackInst");
  }
  void generateAllocObjectInst(AllocObjectInst &inst) {
    hermes_fatal("Unimplemented instruction AllocObjectInst");
  }
  void generateAllocArrayInst(AllocArrayInst &inst) {
    hermes_fatal("Unimplemented instruction AllocArrayInst");
  }
  void generateAllocObjectLiteralInst(AllocObjectLiteralInst &inst) {
    hermes_fatal("Unimplemented instruction AllocObjectLiteralInst");
  }
  void generateCreateArgumentsInst(CreateArgumentsInst &inst) {
    hermes_fatal("Unimplemented instruction CreateArgumentsInst");
  }
  void generateCatchInst(CatchInst &inst) {
    hermes_fatal("Unimplemented instruction CatchInst");
  }
  void generateDebuggerInst(DebuggerInst &inst) {
    hermes_fatal("Unimplemented instruction DebuggerInst");
  }
  void generateCreateRegExpInst(CreateRegExpInst &inst) {
    hermes_fatal("Unimplemented instruction CreateRegExpInst");
  }
  void generateTryEndInst(TryEndInst &inst) {
    hermes_fatal("Unimplemented instruction TryEndInst");
  }
  void generateGetNewTargetInst(GetNewTargetInst &inst) {
    hermes_fatal("Unimplemented instruction GetNewTargetInst");
  }
  void generateThrowIfEmptyInst(ThrowIfEmptyInst &inst) {
    hermes_fatal("Unimplemented instruction ThrowIfEmptyInst");
  }
  void generateIteratorBeginInst(IteratorBeginInst &inst) {
    hermes_fatal("Unimplemented instruction IteratorBeginInst");
  }
  void generateIteratorNextInst(IteratorNextInst &inst) {
    hermes_fatal("Unimplemented instruction IteratorNextInst");
  }
  void generateIteratorCloseInst(IteratorCloseInst &inst) {
    hermes_fatal("Unimplemented instruction IteratorCloseInst");
  }
  void generateHBCStoreToEnvironmentInst(HBCStoreToEnvironmentInst &inst) {
    hermes_fatal("Unimplemented instruction HBCStoreToEnvironmentInst");
  }
  void generateHBCLoadFromEnvironmentInst(HBCLoadFromEnvironmentInst &inst) {
    hermes_fatal("Unimplemented instruction HBCLoadFromEnvironmentInst");
  }
  void generateUnreachableInst(UnreachableInst &inst) {
    hermes_fatal("Unimplemented instruction UnreachableInst");
  }
  void generateCreateFunctionInst(CreateFunctionInst &inst) {
    hermes_fatal("Unimplemented instruction CreateFunctionInst");
  }
  void generateCreateGeneratorInst(CreateGeneratorInst &inst) {
    hermes_fatal("Unimplemented instruction CreateGeneratorInst");
  }
  void generateHBCCreateFunctionInst(HBCCreateFunctionInst &inst) {
    hermes_fatal("Unimplemented instruction HBCCreateFunctionInst");
  }
  void generateHBCCreateGeneratorInst(HBCCreateGeneratorInst &inst) {
    hermes_fatal("Unimplemented instruction HBCCreateGeneratorInst");
  }
  void generateTerminatorInst(TerminatorInst &inst) {
    hermes_fatal("Unimplemented instruction TerminatorInst");
  }
  void generateBranchInst(BranchInst &inst) {
    hermes_fatal("Unimplemented instruction BranchInst");
  }
  void generateReturnInst(ReturnInst &inst) {
    hermes_fatal("Unimplemented instruction ReturnInst");
  }
  void generateThrowInst(ThrowInst &inst) {
    hermes_fatal("Unimplemented instruction ThrowInst");
  }
  void generateSwitchInst(SwitchInst &inst) {
    hermes_fatal("Unimplemented instruction SwitchInst");
  }
  void generateCondBranchInst(CondBranchInst &inst) {
    hermes_fatal("Unimplemented instruction CondBranchInst");
  }
  void generateGetPNamesInst(GetPNamesInst &inst) {
    hermes_fatal("Unimplemented instruction GetPNamesInst");
  }
  void generateGetNextPNameInst(GetNextPNameInst &inst) {
    hermes_fatal("Unimplemented instruction GetNextPNameInst");
  }
  void generateCheckHasInstanceInst(CheckHasInstanceInst &inst) {
    hermes_fatal("Unimplemented instruction CheckHasInstanceInst");
  }
  void generateTryStartInst(TryStartInst &inst) {
    hermes_fatal("Unimplemented instruction TryStartInst");
  }
  void generateCompareBranchInst(CompareBranchInst &inst) {
    hermes_fatal("Unimplemented instruction CompareBranchInst");
  }
  void generateSwitchImmInst(SwitchImmInst &inst) {
    hermes_fatal("Unimplemented instruction SwitchImmInst");
  }
  void generateSaveAndYieldInst(SaveAndYieldInst &inst) {
    hermes_fatal("Unimplemented instruction SaveAndYieldInst");
  }
  void generateCallInst(CallInst &inst) {
    hermes_fatal("Unimplemented instruction CallInst");
  }
  void generateConstructInst(ConstructInst &inst) {
    hermes_fatal("Unimplemented instruction ConstructInst");
  }
  void generateCallBuiltinInst(CallBuiltinInst &inst) {
    hermes_fatal("Unimplemented instruction CallBuiltinInst");
  }
  void generateHBCConstructInst(HBCConstructInst &inst) {
    hermes_fatal("Unimplemented instruction HBCConstructInst");
  }
  void generateHBCCallDirectInst(HBCCallDirectInst &inst) {
    hermes_fatal("Unimplemented instruction HBCCallDirectInst");
  }
  void generateHBCCallNInst(HBCCallNInst &inst) {
    hermes_fatal("Unimplemented instruction HBCCallNInst");
  }
  void generateGetBuiltinClosureInst(GetBuiltinClosureInst &inst) {
    hermes_fatal("Unimplemented instruction GetBuiltinClosureInst");
  }
  void generateStartGeneratorInst(StartGeneratorInst &inst) {
    hermes_fatal("Unimplemented instruction StartGeneratorInst");
  }
  void generateResumeGeneratorInst(ResumeGeneratorInst &inst) {
    hermes_fatal("Unimplemented instruction ResumeGeneratorInst");
  }
  void generateHBCGetGlobalObjectInst(HBCGetGlobalObjectInst &inst) {
    hermes_fatal("Unimplemented instruction HBCGetGlobalObjectInst");
  }
  void generateHBCCreateEnvironmentInst(HBCCreateEnvironmentInst &inst) {
    hermes_fatal("Unimplemented instruction HBCCreateEnvironmentInst");
  }
  void generateHBCGetThisNSInst(HBCGetThisNSInst &inst) {
    hermes_fatal("Unimplemented instruction HBCGetThisNSInst");
  }
  void generateHBCCreateThisInst(HBCCreateThisInst &inst) {
    hermes_fatal("Unimplemented instruction HBCCreateThisInst");
  }
  void generateHBCGetArgumentsPropByValInst(
      HBCGetArgumentsPropByValInst &inst) {
    hermes_fatal("Unimplemented instruction HBCGetArgumentsPropByValInst");
  }
  void generateHBCGetConstructedObjectInst(HBCGetConstructedObjectInst &inst) {
    hermes_fatal("Unimplemented instruction HBCGetConstructedObjectInst");
  }
  void generateHBCAllocObjectFromBufferInst(
      HBCAllocObjectFromBufferInst &inst) {
    hermes_fatal("Unimplemented instruction HBCAllocObjectFromBufferInst");
  }
  void generateHBCProfilePointInst(HBCProfilePointInst &inst) {
    hermes_fatal("Unimplemented instruction HBCProfilePointInst");
  }
  void generateLiteral(Literal &inst) {
    hermes_fatal("Unimplemented instruction Literal");
  }
  void generateLiteralEmpty(LiteralEmpty &inst) {
    hermes_fatal("Unimplemented instruction LiteralEmpty");
  }
  void generateLiteralUndefined(LiteralUndefined &inst) {
    hermes_fatal("Unimplemented instruction LiteralUndefined");
  }
  void generateLiteralNull(LiteralNull &inst) {
    hermes_fatal("Unimplemented instruction LiteralNull");
  }
  void generateLiteralNumber(LiteralNumber &inst) {
    hermes_fatal("Unimplemented instruction LiteralNumber");
  }
  void generateLiteralBigInt(LiteralBigInt &inst) {
    hermes_fatal("Unimplemented instruction LiteralBigInt");
  }
  void generateLiteralString(LiteralString &inst) {
    hermes_fatal("Unimplemented instruction LiteralString");
  }
  void generateLiteralBool(LiteralBool &inst) {
    hermes_fatal("Unimplemented instruction LiteralBool");
  }
  void generateGlobalObject(GlobalObject &inst) {
    hermes_fatal("Unimplemented instruction GlobalObject");
  }
  void generateEmptySentinel(EmptySentinel &inst) {
    hermes_fatal("Unimplemented instruction EmptySentinel");
  }
  void generateLabel(Label &inst) {
    hermes_fatal("Unimplemented instruction Label");
  }
  void generateGlobalObjectProperty(GlobalObjectProperty &inst) {
    hermes_fatal("Unimplemented instruction GlobalObjectProperty");
  }
  void generateVariable(Variable &inst) {
    hermes_fatal("Unimplemented instruction Variable");
  }
  void generateParameter(Parameter &inst) {
    hermes_fatal("Unimplemented instruction Parameter");
  }
  void generateBasicBlock(BasicBlock &inst) {
    hermes_fatal("Unimplemented instruction BasicBlock");
  }
  void generateVariableScope(VariableScope &inst) {
    hermes_fatal("Unimplemented instruction VariableScope");
  }
  void generateExternalScope(ExternalScope &inst) {
    hermes_fatal("Unimplemented instruction ExternalScope");
  }
  void generateFunction(Function &inst) {
    hermes_fatal("Unimplemented instruction Function");
  }
  void generateGeneratorFunction(GeneratorFunction &inst) {
    hermes_fatal("Unimplemented instruction GeneratorFunction");
  }
  void generateGeneratorInnerFunction(GeneratorInnerFunction &inst) {
    hermes_fatal("Unimplemented instruction GeneratorInnerFunction");
  }
  void generateAsyncFunction(AsyncFunction &inst) {
    hermes_fatal("Unimplemented instruction AsyncFunction");
  }
  void generateModule(Module &inst) {
    hermes_fatal("Unimplemented instruction Module");
  }
};

/// Converts Function \p F into valid C code and outputs it through \p OS.
void generateFunction(
    Function &F,
    llvh::raw_ostream &OS,
    ModuleGen &moduleGen,
    FunctionScopeAnalysis &scopeAnalysis,
    uint32_t &nextCacheIdx,
    BytecodeGenerationOptions options) {
  PostOrderAnalysis PO(&F);

  llvh::SmallVector<BasicBlock *, 16> order(PO.rbegin(), PO.rend());

  hbc::HVMRegisterAllocator RA(&F);

  RA.allocate(order);

  if (options.format == DumpRA) {
    RA.dump();
    return;
  }

  PassManager PM;
  PM.addPass(new LowerStoreInstrs(RA));
  PM.addPass(new hbc::LowerCalls(RA));
  if (options.optimizationEnabled) {
    PM.addPass(new MovElimination(RA));
    PM.addPass(new hbc::RecreateCheapValues(RA));
    PM.addPass(new hbc::LoadConstantValueNumbering(RA));
  }
  PM.run(&F);

  if (options.format == DumpLRA) {
    RA.dump();
    return;
  }

  if (options.format == DumpPostRA) {
    F.dump();
    return;
  }

  assert(
      (options.format == DumpBytecode || options.format == EmitBundle) &&
      "All other dump formats must be handled before this.");

  // Set up the entry to the function to look like:
  // static SHLegacyValue _1_myFunc(SHRuntime *shr) {
  //   struct {
  //     SHLocals head;
  //     SHLegacyValue t0;
  //     SHLegacyValue t1;
  //     SHLegacyValue t2;
  //   } locals;
  //   SHLegacyValue *frame = _sh_enter(shr, &locals.head, 3);
  //   locals.head.count = 3;
  //   locals.t0 = _sh_ljs_undefined();
  //   locals.t1 = _sh_ljs_undefined();
  //   locals.t2 = _sh_ljs_undefined();

  OS << "static SHLegacyValue ";
  moduleGen.generateFunctionLabel(&F, OS);
  OS << "(SHRuntime *shr) {\n"
     << "  struct {\n    SHLocals head;\n";

  for (size_t i = 0; i < RA.getMaxRegisterUsage(); ++i)
    OS << "    SHLegacyValue t" << i << ";\n";

  OS << "  } locals;\n"
     << "  SHLegacyValue *frame = _sh_enter(shr, &locals.head,"
     << RA.getMaxRegisterUsage() << ");\n"
     << "  locals.head.count =" << RA.getMaxRegisterUsage() << ";\n";

  for (size_t i = 0; i < RA.getMaxRegisterUsage(); ++i)
    OS << "  locals.t" << i << " = _sh_ljs_undefined();\n";

  unsigned bbCounter = 0;
  llvh::DenseMap<BasicBlock *, unsigned> bbMap;
  for (auto &B : order) {
    bbMap[B] = bbCounter;
    bbCounter++;
  }

  unsigned envSize = F.getFunctionScope()->getVariables().size();

  llvh::DenseMap<BasicBlock *, CatchInst *> catchMap{};
  constructCatchMap(catchMap, F);

  InstrGen instrGen(
      OS,
      RA,
      bbMap,
      F,
      moduleGen,
      scopeAnalysis,
      catchMap,
      envSize,
      nextCacheIdx);

  for (auto &B : order) {
    OS << "\n";
    generateBasicBlockLabel(B, OS, bbMap);
    OS << ":\n";

    for (auto &I : *B) {
      instrGen.generate(I);
    }
  }

  OS << "}\n";
}

/// Converts Module \p M into valid C code and outputs it through \p OS.
/// Returns the cache size necessary to store all the cache indexes used.
void generateModule(
    Module *M,
    llvh::raw_ostream &OS,
    const BytecodeGenerationOptions &options) {
  PassManager PM;
  PM.addPass(new LowerNumericProperties());
  PM.addPass(new hbc::LowerConstruction());
  PM.addPass(new hbc::LowerArgumentsArray());
  PM.addPass(new LimitAllocArray(UINT16_MAX));
  PM.addPass(new hbc::DedupReifyArguments());
  // TODO Consider supporting LowerSwitchIntoJumpTables for optimization
  PM.addPass(new SwitchLowering());
  PM.addPass(new hbc::LoadConstants(true));
  PM.addPass(new hbc::LoadParameters());
  PM.addPass(new hbc::LowerLoadStoreFrameInst());
  if (options.optimizationEnabled) {
    // Lowers AllocObjects and its sequential literal properties into a single
    // Reduce comparison and conditional jump to single comparison jump
    PM.addPass(new LowerCondBranch());
    // Move loads to child blocks if possible.
    PM.addCodeMotion();
    // Eliminate common HBCLoadConstInsts.
    PM.addCSE();
    // Drop unused HBCLoadParamInsts.
    PM.addDCE();
  }
  PM.run(M);

  if (options.format == DumpLIR) {
    M->dump();
    return;
  }

  uint32_t nextCacheIdx = 0;
  ModuleGen moduleGen;

  auto topLevelFunc = M->getTopLevelFunction();
  moduleGen.funcMap[topLevelFunc] = 0;
  unsigned funcCounter = 1;
  for (auto &F : *M) {
    if (&F != topLevelFunc) {
      moduleGen.funcMap[&F] = funcCounter;
      funcCounter++;
    }
  }

  FunctionScopeAnalysis scopeAnalysis{topLevelFunc};

  if (options.format == DumpBytecode || options.format == EmitBundle) {
    OS << R"(
#include "hermes/VM/static_h.h"
)";

    // Forward declare every JS function.
    for (auto &F : *M) {
      OS << "static SHLegacyValue ";
      moduleGen.generateFunctionLabel(&F, OS);
      OS << "(SHRuntime *shr);\n";
    }
  }

  for (auto &F : *M)
    generateFunction(F, OS, moduleGen, scopeAnalysis, nextCacheIdx, options);

  OS << R"(
int main(int argc, char **argv) {
  SHRuntime *shr = _sh_init();
  _sh_initialize_units(shr, 1, &s_this_unit);
  _sh_done(shr);
  return 0;
}
)";
}
} // namespace

/// Converts Module \p M into valid C code and outputs it through \p OS
void sh::generateSH(
    Module *M,
    llvh::raw_ostream &OS,
    const BytecodeGenerationOptions &options) {
  generateModule(M, OS, options);
}
