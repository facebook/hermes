/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/HBC.h"

#include "BytecodeGenerator.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Sema/SemResolve.h"
#include "hermes/Support/MemoryBuffer.h"
#include "hermes/Support/PerfSection.h"
#include "hermes/Support/SimpleDiagHandler.h"

#include "llvh/Support/raw_ostream.h"

#include <future>

#define DEBUG_TYPE "hbc-backend"

namespace hermes {
namespace hbc {

std::unique_ptr<BytecodeModule> generateBytecodeModule(
    Module *M,
    Function *entryPoint,
    const BytecodeGenerationOptions &options,
    hermes::OptValue<uint32_t> segment,
    SourceMapGenerator *sourceMapGen,
    std::unique_ptr<BCProviderBase> baseBCProvider) {
  PerfSection perf("Bytecode Generation");
  auto bm = std::make_unique<BytecodeModule>();

  bool success =
      BytecodeModuleGenerator{
          *bm, M, options, sourceMapGen, std::move(baseBCProvider)}
          .generate(entryPoint, segment);

  return success ? std::move(bm) : nullptr;
}

bool generateBytecodeFunctionLazy(
    BytecodeModule &bm,
    Module *M,
    Function *lazyFunc,
    uint32_t lazyFuncID,
    const BytecodeGenerationOptions &options) {
  return BytecodeModuleGenerator{bm, M, options, nullptr, nullptr}
      .generateLazyFunctions(lazyFunc, lazyFuncID);
}

std::unique_ptr<BytecodeModule> generateBytecodeModuleForEval(
    Module *M,
    Function *entryPoint,
    const BytecodeGenerationOptions &options) {
  PerfSection perf("Bytecode Generation");
  auto bm = std::make_unique<BytecodeModule>();

  bool success = BytecodeModuleGenerator{*bm, M, options, nullptr, nullptr}
                     .generateForEval(entryPoint);

  return success ? std::move(bm) : nullptr;
}

namespace {

/// Execute a function in a SerialExecutor, blocking until the function
/// completes.
/// \param executor the executor to use.
/// \param f the function to execute.
/// \param data the data to pass to the function.
static void executeBlockingInSerialExecutor(
    SerialExecutor &executor,
    void (*f)(void *),
    void *data) {
  std::packaged_task<void()> task([f, data]() { f(data); });
  auto future = task.get_future();
  executor.add([&task]() { task(); });
  future.wait();
}

/// Data for the compileLazyFunctionWorker.
class LazyCompilationThreadData {
 public:
  /// Input: the bytecode module to compile.
  hbc::BCProviderFromSrc *const provider;
  /// Input: The function ID to compile.
  uint32_t const funcID;
  /// Output: whether the compilation succeeded.
  bool success = false;
  /// Output: the error message, if success=false.
  std::string error{};

  explicit LazyCompilationThreadData(
      hbc::BCProviderFromSrc *provider,
      uint32_t funcID)
      : provider(provider), funcID(funcID) {}
};

/// Worker function for the compileLazyFunction, intended to be run in a
/// thread with a fresh stack to prevent stack overflows.
/// \param argPtr[in/out] pointer to the the LazyCompilationThreadData to use as
///   input/output.
static void compileLazyFunctionWorker(void *argPtr) {
  LazyCompilationThreadData *data =
      reinterpret_cast<LazyCompilationThreadData *>(argPtr);
  hbc::BCProviderFromSrc *provider = data->provider;
  uint32_t funcID = data->funcID;

  hbc::BytecodeModule *bcModule = provider->getBytecodeModule();
  hbc::BytecodeFunction &lazyFunc = bcModule->getFunction(funcID);
  Function *F = lazyFunc.getFunctionIR();
  assert(F && "no lazy IR for lazy function");

  SourceErrorManager &manager =
      F->getParent()->getContext().getSourceErrorManager();
  SimpleDiagHandlerRAII outputManager{manager};
  Context &context = F->getParent()->getContext();

  const LazyCompilationDataInst *lazyDataInst = F->getLazyCompilationDataInst();
  assert(lazyDataInst && "function must be lazy");
  const LazyCompilationData &lazyData = lazyDataInst->getData();

  LLVM_DEBUG(llvh::dbgs() << "Compiling lazy "
                          << F->getDescriptiveDefinitionKindStr() << ": "
                          << F->getOriginalOrInferredName() << " @ ";
             manager.dumpCoords(llvh::dbgs(), F->getSourceRange().Start);
             llvh::dbgs() << "\n");

  // Free the AST once we're done compiling this function.
  AllocationScope alloc(context.getAllocator());

  parser::JSParser parser(context, lazyData.bufferId, parser::LazyParse);

  // Note: we don't know the parent's strictness, which we need to pass, but
  // we can just use the child's strictness, which is always stricter or equal
  // to the parent's.
  parser.setStrictMode(lazyData.strictMode);

  auto optParsed = parser.parseLazyFunction(
      lazyData.nodeKind,
      lazyData.paramYield,
      lazyData.paramAwait,
      F->getSourceRange().Start);

  sema::SemContext *semCtx = provider->getSemCtx();
  assert(semCtx && "missing semantic data to compile");

  // A non-null home object means the parent function context could reference
  // super.
  bool parentHadSuperBinding = lazyDataInst->getHomeObject();
  // If parsing or resolution fails, report the error and return.
  if (!optParsed ||
      !sema::resolveASTLazy(
          context,
          *semCtx,
          llvh::cast<ESTree::FunctionLikeNode>(*optParsed),
          lazyData.semInfo,
          parentHadSuperBinding)) {
    data->success = false;
    data->error = outputManager.getErrorString();
    return;
  }

  assert(F && "no IR to compileLazyFunction");

  Function *func = hermes::generateLazyFunctionIR(
      F, llvh::cast<ESTree::FunctionLikeNode>(*optParsed), *semCtx);
  if (outputManager.haveErrors()) {
    data->success = false;
    data->error = outputManager.getErrorString();
    return;
  }

  Module *M = provider->getModule();
  assert(M && "missing IR data to compile");
  if (!hbc::generateBytecodeFunctionLazy(
          *provider->getBytecodeModule(),
          M,
          func,
          funcID,
          provider->getBytecodeGenerationOptions())) {
    data->success = false;
    data->error = outputManager.getErrorString();
    return;
  }

  data->success = true;
}

/// Data for the compileEvalWorker.
class EvalThreadData {
 public:
  /// Input: The source to compile.
  std::unique_ptr<Buffer> src;
  /// Input: the enclosing bytecode module.
  hbc::BCProviderFromSrc *const provider;
  /// Input: The function ID to compile.
  uint32_t const enclosingFuncID;
  /// Input: The CompileFlags to use.
  const CompileFlags &compileFlags;
  /// Output: whether the compilation succeeded.
  bool success = false;
  /// Output: Result if success=true.
  std::unique_ptr<BCProviderFromSrc> result{};
  /// Output: the error message, if success=false.
  std::string error{};

  EvalThreadData(
      std::unique_ptr<Buffer> src,
      hbc::BCProviderFromSrc *provider,
      uint32_t enclosingFuncID,
      const CompileFlags &compileFlags)
      : src(std::move(src)),
        provider(provider),
        enclosingFuncID(enclosingFuncID),
        compileFlags(compileFlags) {}
};

static void compileEvalWorker(void *argPtr) {
  EvalThreadData *data = reinterpret_cast<EvalThreadData *>(argPtr);
  hbc::BCProviderFromSrc *provider = data->provider;
  uint32_t enclosingFuncID = data->enclosingFuncID;

  hbc::BytecodeModule *bcModule = provider->getBytecodeModule();
  hbc::BytecodeFunction &enclosingFunc = bcModule->getFunction(enclosingFuncID);
  Function *F = enclosingFunc.getFunctionIR();

  if (!F) {
    data->success = false;
    data->error =
        "Unable to find scope data for local eval (generators are unsupported)";
    return;
  }

  SourceErrorManager &manager =
      F->getParent()->getContext().getSourceErrorManager();
  SimpleDiagHandlerRAII outputManager{manager};
  Context &context = F->getParent()->getContext();

  context.setEmitAsyncBreakCheck(data->compileFlags.emitAsyncBreakCheck);
  context.setConvertES6Classes(data->compileFlags.enableES6Classes);
  context.setEnableES6BlockScoping(data->compileFlags.enableES6BlockScoping);
  context.setDebugInfoSetting(
      data->compileFlags.debug ? DebugInfoSetting::ALL
                               : DebugInfoSetting::THROWING);

  EvalCompilationDataInst *evalDataInst = F->getEvalCompilationDataInst();
  if (!evalDataInst) {
    data->success = false;
    data->error = "Unable to find scope data for function in eval";
    return;
  }
  const EvalCompilationData &evalData = evalDataInst->getData();

  // Free the AST once we're done compiling this function.
  AllocationScope alloc(context.getAllocator());

  int fileBufId = context.getSourceErrorManager().addNewSourceBuffer(
      std::make_unique<HermesLLVMMemoryBuffer>(std::move(data->src), "eval"));

  auto parserMode = parser::FullParse;

  // NOTE: We don't use lazy compilation if eval requests it but the AST Context
  // doesn't allow it.
  // Eager compilation is really just a version of lazy compilation with very
  // small functions, so it won't violate invariants on what the caller is
  // expecting. e.g. when 'eval' calls this function with lazy compilation
  // enabled, it copies the source into the MemoryBuffer.
  if (context.isLazyCompilation() && data->compileFlags.lazy) {
    auto preParser = parser::JSParser::preParseBuffer(
        context, fileBufId, data->compileFlags.strict);
    if (!preParser) {
      data->success = false;
      data->error = outputManager.getErrorString();
      return;
    }
    preParser->registerMagicURLs();
    parserMode = parser::LazyParse;
  }

  parser::JSParser parser(context, fileBufId, parserMode);
  parser.setStrictMode(data->compileFlags.strict);

  auto optParsed = parser.parse();

  if (optParsed && parserMode != parser::LazyParse) {
    parser.registerMagicURLs();
  }

  // Make a new SemContext which is a child of the SemContext we're referring
  // to, allowing it to be freed when the eval is complete and the
  // BCProviderFromSrc is destroyed.
  std::shared_ptr<sema::SemContext> semCtx =
      std::make_shared<sema::SemContext>(context, provider->shareSemCtx());

  // A non-null home object means the parent function context could reference
  // super.
  bool parentHadSuperBinding = evalDataInst->getHomeObject() != nullptr;
  // If parsing or resolution fails, report the error and return.
  if (!optParsed ||
      !sema::resolveASTInScope(
          context,
          *semCtx,
          llvh::cast<ESTree::ProgramNode>(*optParsed),
          evalData.semInfo,
          parentHadSuperBinding)) {
    data->success = false;
    data->error = outputManager.getErrorString();
    return;
  }

  Function *newFunc = hermes::generateEvalIR(
      F->getParent(),
      evalDataInst,
      llvh::cast<ESTree::FunctionLikeNode>(*optParsed),
      *semCtx);
  if (outputManager.haveErrors()) {
    data->success = false;
    data->error = outputManager.getErrorString();
    return;
  }

  BytecodeGenerationOptions bcGenOpts =
      provider->getBytecodeGenerationOptions();
  bcGenOpts.verifyIR = data->compileFlags.verifyIR;

  Module *M = provider->getModule();
  assert(M && "missing IR data to compile");
  auto bm = hbc::generateBytecodeModuleForEval(M, newFunc, bcGenOpts);
  if (!bm) {
    data->success = false;
    data->error = outputManager.getErrorString();
    return;
  }

  data->success = true;
  data->result = BCProviderFromSrc::createFromBytecodeModule(
      std::move(bm),
      BCProviderFromSrc::CompilationData{
          provider->getBytecodeGenerationOptions(),
          provider->shareModule(),
          semCtx});
}

} // namespace

std::pair<bool, llvh::StringRef> compileLazyFunction(
    hbc::BCProvider *baseProvider,
    uint32_t funcID) {
  auto *provider = llvh::cast<BCProviderFromSrc>(baseProvider);

  if (auto errMsgOpt = provider->getBytecodeModule()
                           ->getFunction(funcID)
                           .getLazyCompileError()) {
    return {false, *errMsgOpt};
  }

  // Run on a thread to prevent stack overflow if this is run from deep inside
  // JS execution.
  LazyCompilationThreadData data{provider, funcID};
  executeBlockingInSerialExecutor(
      provider->getSerialExecutor(), compileLazyFunctionWorker, &data);

  if (data.success) {
    return std::make_pair(true, llvh::StringRef{});
  } else {
    BytecodeFunction &bcFunc =
        provider->getBytecodeModule()->getFunction(funcID);
    bcFunc.setLazyCompileError(std::move(data.error));
    return std::make_pair(false, *bcFunc.getLazyCompileError());
  }
}

SMLoc findSMLocFromCoords(
    hbc::BCProvider *baseProvider,
    uint32_t line,
    uint32_t col) {
  if (!llvh::isa<BCProviderFromSrc>(baseProvider))
    return SMLoc{};

  auto *provider = llvh::cast<BCProviderFromSrc>(baseProvider);
  hbc::BytecodeModule *bcModule = provider->getBytecodeModule();
  assert(bcModule && "no bytecode module while debugging");
  hbc::BytecodeFunction &globalFunc =
      bcModule->getFunction(provider->getGlobalFunctionIndex());
  Function *F = globalFunc.getFunctionIR();
  assert(F && "no IR for global function while debugging");

  SourceErrorManager &manager =
      provider->getModule()->getContext().getSourceErrorManager();

  // Convert the coords to SMLoc to check for membership, because that's simpler
  // than converting the exclusive end SMLoc of the function to coords,
  // plus it only requires one conversion.
  SourceErrorManager::SourceCoords coords{
      manager.findBufferIdForLoc(F->getSourceRange().Start), line, col};
  return manager.findSMLocFromCoords(coords);
}

bool coordsInLazyFunction(
    hbc::BCProvider *baseProvider,
    uint32_t funcID,
    SMLoc loc) {
  auto *provider = llvh::cast<BCProviderFromSrc>(baseProvider);
  hbc::BytecodeModule *bcModule = provider->getBytecodeModule();
  hbc::BytecodeFunction &lazyFunc = bcModule->getFunction(funcID);
  assert(lazyFunc.isLazy() && "function is not lazy");
  Function *F = lazyFunc.getFunctionIR();
  assert(F && "no lazy IR for lazy function");

  if (!loc.isValid())
    return false;

  return F->getSourceRange().Start.getPointer() <= loc.getPointer() &&
      loc.getPointer() < F->getSourceRange().End.getPointer();
}

std::pair<std::unique_ptr<BCProviderFromSrc>, std::string> compileEvalModule(
    std::unique_ptr<Buffer> src,
    hbc::BCProviderFromSrc *provider,
    uint32_t enclosingFuncID,
    const CompileFlags &compileFlags) {
  // Run on a thread to prevent stack overflow if this is run from deep inside
  // JS execution.
  EvalThreadData data{std::move(src), provider, enclosingFuncID, compileFlags};
  executeBlockingInSerialExecutor(
      provider->getSerialExecutor(), compileEvalWorker, &data);

  return data.success
      ? std::make_pair(std::move(data.result), "")
      : std::make_pair(std::unique_ptr<BCProviderFromSrc>{}, data.error);
}

std::vector<uint32_t> getVariableCounts(
    hbc::BCProviderFromSrc *provider,
    uint32_t funcID) {
  hbc::BytecodeModule *bcModule = provider->getBytecodeModule();
  hbc::BytecodeFunction &bcFunc = bcModule->getFunction(funcID);
  Function *F = bcFunc.getFunctionIR();
  if (!F) {
    // Couldn't find an IR function, possibly unsupported function kind or
    // compiled without -g.
    return std::vector<uint32_t>({0});
  }

  const EvalCompilationDataInst *evalDataInst = F->getEvalCompilationDataInst();
  if (!evalDataInst) {
    // No known way to do this, defensive programming.
    // functionIR is either lazy or has EvalCompilationDataInst, and we can't be
    // here if the function was lazy.
    return std::vector<uint32_t>({0});
  }

  std::vector<uint32_t> counts;

  for (auto *cur = evalDataInst->getFuncVarScope(); cur;
       cur = cur->getParentScope()) {
    counts.push_back(cur->getNumVisibleVariables());
  }

  return counts;
}

llvh::StringRef getVariableNameAtDepth(
    hbc::BCProviderFromSrc *provider,
    uint32_t funcID,
    uint32_t depth,
    uint32_t variableIndex) {
  hbc::BytecodeModule *bcModule = provider->getBytecodeModule();
  hbc::BytecodeFunction &lazyFunc = bcModule->getFunction(funcID);
  Function *F = lazyFunc.getFunctionIR();
  assert(F && "no lazy IR for lazy function");

  const EvalCompilationDataInst *evalDataInst = F->getEvalCompilationDataInst();
  assert(evalDataInst && "function must be eval");

  auto *varScope = llvh::cast<VariableScope>(evalDataInst->getOperand(
      EvalCompilationDataInst::FuncVarScopeIdx + depth));

  return varScope->getVariables()[variableIndex]->getName().str();
}

} // namespace hbc
} // namespace hermes

#undef DEBUG_TYPE
