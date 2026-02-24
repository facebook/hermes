/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/HBC.h"

#include "BytecodeGenerator.h"
#include "hermes/AST/TransformAST.h"
#include "hermes/BCGen/HBC/BCProviderFromSrc.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Optimizer/PassManager/Pipeline.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Sema/SemContext.h"
#include "hermes/Sema/SemResolve.h"
#include "hermes/Support/MemoryBuffer.h"
#include "hermes/Support/PerfSection.h"
#include "hermes/Support/SimpleDiagHandler.h"

#include "llvh/Support/raw_ostream.h"

#define DEBUG_TYPE "hbc-backend"

namespace hermes {
namespace hbc {

void fullOptimizationPipeline(Module &M) {
  hermes::runFullOptimizationPasses(M);
}

std::pair<std::unique_ptr<BCProvider>, std::string> createBCProviderFromSrc(
    std::unique_ptr<Buffer> buffer,
    llvh::StringRef sourceURL,
    llvh::StringRef sourceMap,
    const CompileFlags &compileFlags,
    llvh::StringRef topLevelFunctionName,
    SourceErrorManager::DiagHandlerTy diagHandler,
    void *diagContext,
    const std::function<void(Module &)> &runOptimizationPasses,
    const BytecodeGenerationOptions &defaultBytecodeGenerationOptions) {
  return BCProviderFromSrc::create(
      std::move(buffer),
      sourceURL,
      sourceMap,
      compileFlags,
      topLevelFunctionName,
      diagHandler,
      diagContext,
      runOptimizationPasses,
      defaultBytecodeGenerationOptions);
}

std::unique_ptr<BytecodeModule> generateBytecodeModule(
    Module *M,
    Function *entryPoint,
    const BytecodeGenerationOptions &options,
    hermes::OptValue<uint32_t> segment,
    std::unique_ptr<BCProviderBase> baseBCProvider) {
  PerfSection perf("Bytecode Generation");
  auto bm = std::make_unique<BytecodeModule>();
  FileAndSourceMapIdCache debugIdCache{};

  bool success =
      BytecodeModuleGenerator{
          *bm, M, debugIdCache, options, std::move(baseBCProvider)}
          .generate(entryPoint, segment);

  return success ? std::move(bm) : nullptr;
}

bool generateBytecodeFunctionLazy(
    BytecodeModule &bm,
    Module *M,
    Function *lazyFunc,
    uint32_t lazyFuncID,
    FileAndSourceMapIdCache &debugIdCache,
    const BytecodeGenerationOptions &options) {
  return BytecodeModuleGenerator{bm, M, debugIdCache, options, nullptr}
      .generateLazyFunctions(lazyFunc, lazyFuncID);
}

std::unique_ptr<BytecodeModule> generateBytecodeModuleForEval(
    Module *M,
    Function *entryPoint,
    const BytecodeGenerationOptions &options) {
  PerfSection perf("Bytecode Generation");
  auto bm = std::make_unique<BytecodeModule>();
  FileAndSourceMapIdCache debugIdCache{};

  bool success = BytecodeModuleGenerator{*bm, M, debugIdCache, options, nullptr}
                     .generateForEval(entryPoint);

  return success ? std::move(bm) : nullptr;
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
  Function *F = lazyFunc.getFunctionIR();
  assert(F && "no lazy IR for lazy function");

  SourceErrorManager &manager =
      F->getParent()->getContext().getSourceErrorManager();
  SimpleDiagHandlerRAII outputManager{manager};
  Context &context = F->getParent()->getContext();

  const LazyCompilationDataInst *lazyDataInst = F->getLazyCompilationDataInst();
  assert(lazyDataInst && "function must be lazy");
  const LazyCompilationData &lazyData = lazyDataInst->getData();

  LLVM_DEBUG(
      llvh::dbgs() << "Compiling lazy " << F->getDescriptiveDefinitionKindStr()
                   << ": " << F->getOriginalOrInferredName() << " @ ";
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

  if (!optParsed) {
    data->success = false;
    data->error = outputManager.getErrorString();
    return;
  }

  optParsed = hermes::transformASTForCompilation(context, *optParsed);

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
          provider->getFileAndSourceMapIdCache(),
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
  /// Input: the lexical scope to perform the eval in.
  uint32_t lexicalScopeIdxInParentFunction;
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
      const CompileFlags &compileFlags,
      uint32_t lexicalScopeIdxInParentFunction)
      : src(std::move(src)),
        provider(provider),
        enclosingFuncID(enclosingFuncID),
        compileFlags(compileFlags),
        lexicalScopeIdxInParentFunction(lexicalScopeIdxInParentFunction) {}
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
  context.setEnableES6BlockScoping(data->compileFlags.enableES6BlockScoping);
  context.setEnableAsyncGenerators(data->compileFlags.enableAsyncGenerators);
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
  auto *funcInfo = evalData.semInfo;
  sema::LexicalScope *lexScope =
      funcInfo->getScopes()[data->lexicalScopeIdxInParentFunction];

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
  if (!optParsed) {
    data->success = false;
    data->error = outputManager.getErrorString();
    return;
  }

  if (optParsed && parserMode != parser::LazyParse) {
    parser.registerMagicURLs();
  }

  // Make a new SemContext which is a child of the SemContext we're referring
  // to, allowing it to be freed when the eval is complete and the
  // BCProviderFromSrc is destroyed.
  std::shared_ptr<sema::SemContext> semCtx = std::make_shared<sema::SemContext>(
      context, provider->shareSemCtx(), lexScope);

  optParsed = llvh::cast<ESTree::ProgramNode>(
      hermes::transformASTForCompilation(context, *optParsed));

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

  // Use this callback-style API to reduce conflicts with stable for now.
  LazyCompilationThreadData data{provider, funcID};
  compileLazyFunctionWorker(&data);

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

SMRange findSMRangeForLine(hbc::BCProvider *baseProvider, uint32_t line) {
  if (!llvh::isa<BCProviderFromSrc>(baseProvider))
    return SMRange{};

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
  return manager.findSMRangeForLine(
      manager.findBufferIdForLoc(F->getSourceRange().Start), line);
}

bool coordsInLazyFunction(
    hbc::BCProvider *baseProvider,
    uint32_t funcID,
    SMLoc loc,
    OptValue<SMLoc> end) {
  auto *provider = llvh::cast<BCProviderFromSrc>(baseProvider);
  hbc::BytecodeModule *bcModule = provider->getBytecodeModule();
  hbc::BytecodeFunction &lazyFunc = bcModule->getFunction(funcID);
  assert(lazyFunc.isLazy() && "function is not lazy");
  Function *F = lazyFunc.getFunctionIR();
  assert(F && "no lazy IR for lazy function");

  if (!loc.isValid())
    return false;

  bool startsIn = F->getSourceRange().Start.getPointer() <= loc.getPointer() &&
      loc.getPointer() < F->getSourceRange().End.getPointer();
  // If the loc is in the function, we're done.
  if (startsIn)
    return true;

  // If the end is specified, check if the range is larger than the function.
  if (end.hasValue()) {
    return loc.getPointer() <= F->getSourceRange().Start.getPointer() &&
        F->getSourceRange().End.getPointer() <= end.getValue().getPointer();
  }

  // Otherwise, failed.
  return false;
}

std::pair<std::unique_ptr<BCProvider>, std::string> compileEvalModule(
    std::unique_ptr<Buffer> src,
    hbc::BCProvider *provider,
    uint32_t enclosingFuncID,
    const CompileFlags &compileFlags,
    uint32_t lexicalScopeIdxInParentFunction) {
  hbc::BCProviderFromSrc *providerFromSrc =
      llvh::dyn_cast<hbc::BCProviderFromSrc>(provider);
  if (!providerFromSrc) {
    return std::make_pair(
        std::unique_ptr<BCProviderFromSrc>{},
        "Code compiled without support for eval");
  }
  // Use this callback-style API to reduce conflicts with stable for now.
  EvalThreadData data{
      std::move(src),
      providerFromSrc,
      enclosingFuncID,
      compileFlags,
      lexicalScopeIdxInParentFunction};
  compileEvalWorker(&data);

  return data.success
      ? std::make_pair(std::move(data.result), "")
      : std::make_pair(std::unique_ptr<BCProviderFromSrc>{}, data.error);
}

/// \return true if \p decl should be shown when collecting variables.
static bool shouldListDecl(sema::Decl *decl) {
  if (decl->special == sema::Decl::Special::Arguments)
    return false;
  switch (decl->kind) {
    case sema::Decl::Kind::Let:
    case sema::Decl::Kind::Const:
    case sema::Decl::Kind::Catch:
    case sema::Decl::Kind::ES5Catch:
    case sema::Decl::Kind::Var:
    case sema::Decl::Kind::Parameter:
      return true;
    default:
      return false;
  }
}

/// \return the sema::FunctionInfo for function \p funcID, living in \p
/// BCProvider, or nullptr if none can be found.
static const sema::FunctionInfo *getFunctionInfo(
    hbc::BCProvider *provider,
    uint32_t funcID) {
  hbc::BCProviderFromSrc *providerFromSrc =
      llvh::dyn_cast<hbc::BCProviderFromSrc>(provider);
  if (!providerFromSrc) {
    return nullptr;
  }
  hbc::BytecodeModule *bcModule = providerFromSrc->getBytecodeModule();
  hbc::BytecodeFunction &bcFunc = bcModule->getFunction(funcID);
  Function *F = bcFunc.getFunctionIR();
  if (!F) {
    return nullptr;
  }
  auto &evalData = F->getEvalCompilationDataInst()->getData();
  return evalData.semInfo;
}

std::vector<uint32_t> getVariableCounts(
    hbc::BCProvider *provider,
    uint32_t funcID,
    uint32_t lexicalScopeIdxInParentFunction) {
  auto *funcInfo = getFunctionInfo(provider, funcID);
  if (!funcInfo)
    return std::vector<uint32_t>({0});
  sema::LexicalScope *lexScope =
      funcInfo->getScopes()[lexicalScopeIdxInParentFunction];
  assert(lexScope && "lexical scope cannot be null.");
  std::vector<uint32_t> counts;
  for (auto *cur = lexScope; cur; cur = cur->parentScope) {
    counts.push_back(llvh::count_if(cur->decls, shouldListDecl));
  }
  return counts;
}

VariableInfoAtDepth getVariableInfoAtDepth(
    hbc::BCProvider *provider,
    uint32_t funcID,
    uint32_t depth,
    uint32_t variableIndex,
    uint32_t lexicalScopeIdxInParentFunction) {
  auto *funcInfo = getFunctionInfo(provider, funcID);
  if (!funcInfo)
    return {};
  sema::LexicalScope *lexScope =
      funcInfo->getScopes()[lexicalScopeIdxInParentFunction];
  assert(lexScope && "lexical scope cannot be null.");
  auto *beginLexScope = lexScope;
  for (size_t i = 0; i < depth; i++) {
    lexScope = lexScope->parentScope;
    assert(lexScope && "depth out of bounds");
  }

  /// In order to avoid having to search through the entire `decls` array each
  /// time, the result of the last call is cached, and used to try to restart
  /// the search from that point. This struct is the cache information.
  struct CachedValues {
    // The lexical scope queried.
    sema::LexicalScope *lexScope = nullptr;
    // The variable index queried.
    uint32_t variableIndex = UINT32_MAX;
    // The resulting index into the lexical scope's decl that satisfies the
    // variableIndex search.
    uint32_t declIdx = UINT32_MAX;
  };
  hbc::BCProviderFromSrc *providerFromSrc =
      llvh::cast<hbc::BCProviderFromSrc>(provider);
  sema::SemContext *semCtx = providerFromSrc->getSemCtx();
  if (!semCtx->customData2) {
    // The lifetime of this cache has to be tied to the lifetime of the
    // SemContext. A SemContext could be destoryed and allocated again. there's
    // then a potential for a new LexicalScope in the same place but with
    // different data. Use shared_ptr to make sure it isn't accessible after the
    // SemContext is freed.
    semCtx->customData2 = std::make_shared<CachedValues>();
  }
  auto *cache = static_cast<CachedValues *>(semCtx->customData2.get());

  const auto &decls = lexScope->decls;
  // This is how many listable decls we need to find in order to satisfy the
  // search for \p variableIdx.
  uint32_t targetListableDeclsSeen = variableIndex + 1;
  // The index we found the matching decl. Initialized to out of bounds.
  uint32_t declIdx = 0;
  // When iterating through the decls, how many user-presentable decls have been
  // seen.
  uint32_t listableDeclsSeen;
  uint32_t beginDeclIdx;
  // Only reuse the cached values when searching for a subsequent variableIdx on
  // the same lexical scope as last time.
  if (lexScope == cache->lexScope && variableIndex > cache->variableIndex) {
    beginDeclIdx = cache->declIdx;
    listableDeclsSeen = cache->variableIndex;
  } else {
    beginDeclIdx = 0;
    listableDeclsSeen = 0;
  }
  for (size_t i = beginDeclIdx, e = decls.size(); i < e; ++i) {
    if (shouldListDecl(decls[i])) {
      if (++listableDeclsSeen == targetListableDeclsSeen) {
        declIdx = i;
        break;
      }
    }
  }
  assert(
      listableDeclsSeen == targetListableDeclsSeen &&
      "variable at variableIdx not found");
  sema::Decl *decl = decls[declIdx];
  auto *varForDecl = static_cast<Variable *>(getDeclCustomData(decl));

  // Calculate how many steps is required to get to the target VariableScope.
  VariableScope *curVS = getLexicalScopeCustomData(beginLexScope);
  VariableScope *targetVS = getLexicalScopeCustomData(lexScope);
  assert(curVS && targetVS && "VariableScope must be set for lexical scopes");
  uint32_t varScopeDepth = 0;
  for (; curVS; curVS = curVS->getParentScope()) {
    if (curVS == targetVS) {
      break;
    }
    ++varScopeDepth;
  }
  assert(curVS && "target VariableScope could not be reached");

  cache->lexScope = lexScope;
  cache->variableIndex = variableIndex;
  cache->declIdx = declIdx;
  return {
      decl->name.str(), varScopeDepth, varForDecl->getIndexInVariableList()};
}

} // namespace hbc
} // namespace hermes

#undef DEBUG_TYPE
