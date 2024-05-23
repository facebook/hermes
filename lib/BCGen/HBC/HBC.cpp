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
#include "hermes/Support/PerfSection.h"
#include "hermes/Support/SimpleDiagHandler.h"

#include "llvh/Support/Threading.h"
#include "llvh/Support/raw_ostream.h"

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

namespace {

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
  Function *F = lazyFunc.getLazyFunction();
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
             manager.dumpCoords(llvh::dbgs(), lazyData.span.Start);
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
      lazyData.span.Start);

  sema::SemContext *semCtx = provider->getSemCtx();
  assert(semCtx && "missing semantic data to compile");

  // If parsing or resolution fails, report the error and return.
  if (!optParsed ||
      !sema::resolveASTLazy(
          context,
          *semCtx,
          llvh::cast<ESTree::FunctionLikeNode>(*optParsed),
          lazyData.semInfo)) {
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

  // Use an 8MB stack, which is the default size on mac and linux.
  constexpr unsigned kStackSize = 1 << 23;

  LazyCompilationThreadData data{provider, funcID};
  llvh::llvm_execute_on_thread(compileLazyFunctionWorker, &data, kStackSize);

  if (data.success) {
    return std::make_pair(true, llvh::StringRef{});
  } else {
    BytecodeFunction &bcFunc =
        provider->getBytecodeModule()->getFunction(funcID);
    bcFunc.setLazyCompileError(std::move(data.error));
    return std::make_pair(false, *bcFunc.getLazyCompileError());
  }
}

} // namespace hbc
} // namespace hermes

#undef DEBUG_TYPE
