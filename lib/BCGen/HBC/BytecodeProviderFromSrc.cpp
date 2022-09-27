/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/BytecodeProviderFromSrc.h"

#include "hermes/AST/SemValidate.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Runtime/Libhermes.h"
#include "hermes/SourceMap/SourceMapTranslator.h"
#include "hermes/Support/MemoryBuffer.h"
#include "hermes/Support/SimpleDiagHandler.h"

namespace hermes {
namespace hbc {

namespace {
bool isSingleFunctionExpression(ESTree::NodePtr ast) {
  auto *prog = llvh::dyn_cast<ESTree::ProgramNode>(ast);
  if (!prog) {
    return false;
  }
  ESTree::NodeList &body = prog->_body;
  if (body.size() != 1) {
    return false;
  }
  auto *exprStatement =
      llvh::dyn_cast<ESTree::ExpressionStatementNode>(&body.front());
  if (!exprStatement) {
    return false;
  }
  return llvh::isa<ESTree::FunctionExpressionNode>(
             exprStatement->_expression) ||
      llvh::isa<ESTree::ArrowFunctionExpressionNode>(
             exprStatement->_expression);
}
} // namespace

BCProviderFromSrc::BCProviderFromSrc(
    std::unique_ptr<hbc::BytecodeModule> module)
    : module_(std::move(module)) {
  options_ = module_->getBytecodeOptions();

  functionCount_ = module_->getNumFunctions();

  globalFunctionIndex_ = module_->getGlobalFunctionIndex();

  stringKinds_ = module_->getStringKinds();
  identifierHashes_ = module_->getIdentifierHashes();
  stringCount_ = module_->getStringTable().size();
  stringStorage_ = module_->getStringStorage();

  bigIntStorage_ = module_->getBigIntStorage();
  bigIntTable_ = module_->getBigIntTable();

  regExpStorage_ = module_->getRegExpStorage();
  regExpTable_ = module_->getRegExpTable();

  arrayBuffer_ = module_->getArrayBuffer();
  objKeyBuffer_ = module_->getObjectBuffer().first;
  objValueBuffer_ = module_->getObjectBuffer().second;

  segmentID_ = module_->getSegmentID();
  cjsModuleTable_ = module_->getCJSModuleTable();
  cjsModuleTableStatic_ = module_->getCJSModuleTableStatic();

  functionSourceTable_ = module_->getFunctionSourceTable();

  debugInfo_ = &module_->getDebugInfo();
}

std::pair<std::unique_ptr<BCProviderFromSrc>, std::string>
BCProviderFromSrc::createBCProviderFromSrc(
    std::unique_ptr<Buffer> buffer,
    llvh::StringRef sourceURL,
    const CompileFlags &compileFlags) {
  return createBCProviderFromSrc(
      std::move(buffer), sourceURL, /*sourceMap*/ nullptr, compileFlags);
}

std::pair<std::unique_ptr<BCProviderFromSrc>, std::string>
BCProviderFromSrc::createBCProviderFromSrc(
    std::unique_ptr<Buffer> buffer,
    llvh::StringRef sourceURL,
    std::unique_ptr<SourceMap> sourceMap,
    const CompileFlags &compileFlags) {
  return createBCProviderFromSrc(
      std::move(buffer), sourceURL, std::move(sourceMap), compileFlags, {}, {});
}

std::pair<std::unique_ptr<BCProviderFromSrc>, std::string>
BCProviderFromSrc::createBCProviderFromSrc(
    std::unique_ptr<Buffer> buffer,
    llvh::StringRef sourceURL,
    std::unique_ptr<SourceMap> sourceMap,
    const CompileFlags &compileFlags,
    const ScopeChain &scopeChain,
    SourceErrorManager::DiagHandlerTy diagHandler,
    void *diagContext,
    const std::function<void(Module &)> &runOptimizationPasses) {
  return createBCProviderFromSrcImpl(
      std::move(buffer),
      sourceURL,
      std::move(sourceMap),
      compileFlags,
      scopeChain,
      diagHandler,
      diagContext,
      runOptimizationPasses);
}

std::pair<std::unique_ptr<BCProviderFromSrc>, std::string>
BCProviderFromSrc::createBCProviderFromSrcImpl(
    std::unique_ptr<Buffer> buffer,
    llvh::StringRef sourceURL,
    std::unique_ptr<SourceMap> sourceMap,
    const CompileFlags &compileFlags,
    const ScopeChain &scopeChain,
    SourceErrorManager::DiagHandlerTy diagHandler,
    void *diagContext,
    const std::function<void(Module &)> &runOptimizationPasses) {
  assert(
      buffer->data()[buffer->size()] == 0 &&
      "The input buffer must be null terminated");

  CodeGenerationSettings codeGenOpts{};
  codeGenOpts.unlimitedRegisters = false;
  codeGenOpts.instrumentIR = compileFlags.instrumentIR;

  OptimizationSettings optSettings;
  // If the optional value is not set, the parser will automatically detect
  // the 'use static builtin' directive and we will set it correctly.
  optSettings.staticBuiltins = compileFlags.staticBuiltins.hasValue()
      ? compileFlags.staticBuiltins.getValue()
      : false;

  auto context = std::make_shared<Context>(codeGenOpts, optSettings);
  std::unique_ptr<SimpleDiagHandlerRAII> outputManager;
  if (diagHandler) {
    context->getSourceErrorManager().setDiagHandler(diagHandler, diagContext);
  } else {
    outputManager.reset(
        new SimpleDiagHandlerRAII(context->getSourceErrorManager()));
  }
  // If a custom diagHandler was provided, it will receive the details and we
  // just return the string "error" on failure.
  auto getErrorString = [&outputManager]() {
    return outputManager ? outputManager->getErrorString()
                         : std::string("error");
  };

  // To avoid frequent source buffer rescans, avoid emitting warnings about
  // undefined variables.
  context->getSourceErrorManager().setWarningStatus(
      Warning::UndefinedVariable, false);

  context->setStrictMode(compileFlags.strict);
  context->setEnableEval(true);
  context->setPreemptiveFunctionCompilationThreshold(
      compileFlags.preemptiveFunctionCompilationThreshold);
  context->setPreemptiveFileCompilationThreshold(
      compileFlags.preemptiveFileCompilationThreshold);

  if (compileFlags.lazy && !runOptimizationPasses) {
    context->setLazyCompilation(true);
  }

  context->setGeneratorEnabled(compileFlags.enableGenerator);
  context->setDebugInfoSetting(
      compileFlags.debug ? DebugInfoSetting::ALL : DebugInfoSetting::THROWING);
  context->setEmitAsyncBreakCheck(compileFlags.emitAsyncBreakCheck);

  // Populate the declFileList.
  DeclarationFileListTy declFileList;
  if (compileFlags.includeLibHermes) {
    auto libBuffer = llvh::MemoryBuffer::getMemBuffer(libhermes);
    parser::JSParser libParser(*context, std::move(libBuffer));
    auto libParsed = libParser.parse();
    assert(libParsed && "Libhermes failed to parse");
    declFileList.push_back(libParsed.getValue());
  }

  bool isLargeFile =
      buffer->size() >= context->getPreemptiveFileCompilationThreshold();
  int fileBufId = context->getSourceErrorManager().addNewSourceBuffer(
      std::make_unique<HermesLLVMMemoryBuffer>(std::move(buffer), sourceURL));
  if (sourceMap != nullptr) {
    auto sourceMapTranslator =
        std::make_shared<SourceMapTranslator>(context->getSourceErrorManager());
    context->getSourceErrorManager().setTranslator(sourceMapTranslator);
    sourceMapTranslator->addSourceMap(fileBufId, std::move(sourceMap));
  }

  auto parserMode = parser::FullParse;
  bool useStaticBuiltinDetected = false;
  if (context->isLazyCompilation() && isLargeFile) {
    if (!parser::JSParser::preParseBuffer(
            *context, fileBufId, useStaticBuiltinDetected)) {
      return {nullptr, getErrorString()};
    }
    parserMode = parser::LazyParse;
  }

  sem::SemContext semCtx{};
  parser::JSParser parser(*context, fileBufId, parserMode);
  auto parsed = parser.parse();
  if (!parsed || !hermes::sem::validateAST(*context, semCtx, *parsed)) {
    return {nullptr, getErrorString()};
  }
  // If we are using lazy parse mode, we should have already detected the 'use
  // static builtin' directive in the pre-parsing stage.
  if (parserMode != parser::LazyParse) {
    useStaticBuiltinDetected = parser.getUseStaticBuiltin();
  }
  // The compiler flag is not set, automatically detect 'use static builtin'
  // from the source.
  if (!compileFlags.staticBuiltins) {
    context->setStaticBuiltinOptimization(useStaticBuiltinDetected);
  }

  Module M(context);
  hermes::generateIRFromESTree(parsed.getValue(), &M, declFileList, scopeChain);
  if (context->getSourceErrorManager().getErrorCount() > 0) {
    return {nullptr, getErrorString()};
  }

  if (runOptimizationPasses)
    runOptimizationPasses(M);

  BytecodeGenerationOptions opts{compileFlags.format};
  opts.optimizationEnabled = !!runOptimizationPasses;
  opts.staticBuiltinsEnabled =
      context->getOptimizationSettings().staticBuiltins;
  opts.verifyIR = compileFlags.verifyIR;
  auto BM = hbc::generateBytecodeModule(&M, M.getTopLevelFunction(), opts);
  if (context->getSourceErrorManager().getErrorCount() > 0) {
    return {nullptr, getErrorString()};
  }
  auto bytecode = createBCProviderFromSrc(std::move(BM));
  bytecode->singleFunction_ = isSingleFunctionExpression(parsed.getValue());
  return {std::move(bytecode), std::string{}};
}

BCProviderLazy::BCProviderLazy(hbc::BytecodeFunction *bytecodeFunction)
    : bytecodeFunction_(bytecodeFunction) {
  // Lazy module should always contain one function to begin with.
  functionCount_ = 1;
}

} // namespace hbc
} // namespace hermes
