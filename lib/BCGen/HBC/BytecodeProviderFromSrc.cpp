/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/BCGen/HBC/BytecodeProviderFromSrc.h"

#include "hermes/AST/SemValidate.h"
#include "hermes/BCGen/HBC/HBC.h"
#ifdef HERMESVM_ENABLE_OPT_COMPILE_IN_RUNTIME
#include "hermes/Optimizer/PassManager/Pipeline.h"
#endif
#include "hermes/Parser/JSParser.h"
#include "hermes/Runtime/Libhermes.h"
#include "hermes/Support/MemoryBuffer.h"
#include "hermes/Support/SimpleDiagHandler.h"

namespace hermes {
namespace hbc {

#ifndef HERMESVM_LEAN

BCProviderFromSrc::BCProviderFromSrc(
    std::unique_ptr<hbc::BytecodeModule> module)
    : module_(std::move(module)) {
  options_ = module_->getBytecodeOptions();

  functionCount_ = module_->getNumFunctions();

  globalFunctionIndex_ = module_->getGlobalFunctionIndex();

  stringStorage_ = module_->getStringStorage();
  stringCount_ = module_->getStringTable().size();
  identifierTranslations_ = module_->getIdentifierTranslations();

  regExpStorage_ = module_->getRegExpStorage();
  regExpTable_ = module_->getRegExpTable();

  arrayBuffer_ = module_->getArrayBuffer();
  objKeyBuffer_ = module_->getObjectBuffer().first;
  objValueBuffer_ = module_->getObjectBuffer().second;

  cjsModuleOffset_ = module_->getCJSModuleOffset();
  cjsModuleTable_ = module_->getCJSModuleTable();
  cjsModuleTableStatic_ = module_->getCJSModuleTableStatic();

  debugInfo_ = &module_->getDebugInfo();

  // Since we are executing from source, the serialization step that normally
  // takes care of putting the jump table into the bytecode is not run and hence
  // we do it here instead.
  module_->inlineJumpTables();
}

std::pair<std::unique_ptr<BCProviderFromSrc>, std::string>
BCProviderFromSrc::createBCProviderFromSrc(
    std::unique_ptr<Buffer> buffer,
    llvm::StringRef sourceURL,
    const CompileFlags &compileFlags) {
  using llvm::Twine;

  assert(
      buffer->data()[buffer->size()] == 0 &&
      "The input buffer must be null terminated");

  CodeGenerationSettings codeGenOpts{};
  codeGenOpts.unlimitedRegisters = false;

  OptimizationSettings optSettings;
  optSettings.staticBuiltins = compileFlags.staticBuiltins;

  auto context = std::make_shared<Context>(
      codeGenOpts, TypeCheckerSettings(), optSettings);
  SimpleDiagHandlerRAII outputManager{
      context->getSourceErrorManager().getSourceMgr()};

  // To avoid frequent source buffer rescans, avoid emitting warnings about
  // undefined variables.
  context->getSourceErrorManager().setWarningStatus(
      Warning::UndefinedVariable, false);

  context->setStrictMode(compileFlags.strict);
  context->setEnableEval(true);
  context->setLazyCompilation(compileFlags.lazy);
#ifdef HERMES_ENABLE_DEBUGGER
  context->setDebugInfoSetting(
      compileFlags.debug ? DebugInfoSetting::ALL : DebugInfoSetting::THROWING);
#else
  context->setDebugInfoSetting(DebugInfoSetting::THROWING);
#endif

  // Populate the declFileList.
  DeclarationFileListTy declFileList;
  auto libBuffer = llvm::MemoryBuffer::getMemBuffer(libhermes);
  parser::JSParser libParser(*context, std::move(libBuffer));
  auto libParsed = libParser.parse();
  assert(libParsed && "Libhermes failed to parse");
  declFileList.push_back(libParsed.getValue());

  int fileBufId = context->getSourceErrorManager().addNewSourceBuffer(
      llvm::make_unique<HermesLLVMMemoryBuffer>(std::move(buffer), sourceURL));

  auto parserMode = parser::FullParse;
  if (context->isLazyCompilation()) {
    if (!parser::JSParser::preParseBuffer(*context, fileBufId)) {
      return {nullptr, outputManager.getErrorString()};
    }
    parserMode = parser::LazyParse;
  }

  sem::SemContext semCtx{};
  parser::JSParser parser(*context, fileBufId, parserMode);
  auto parsed = parser.parse();
  if (!parsed || !hermes::sem::validateAST(*context, semCtx, *parsed)) {
    return {nullptr, outputManager.getErrorString()};
  }

  Module M(context);
  hermes::generateIRFromESTree(parsed.getValue(), &M, declFileList, {});
  if (context->getSourceErrorManager().getErrorCount() > 0) {
    return {nullptr, outputManager.getErrorString()};
  }

  assert(
      (compileFlags.optimize ? !compileFlags.lazy : true) &&
      "Can't optimize in lazy mode.");

#ifdef HERMESVM_ENABLE_OPTIMIZATION_AT_RUNTIME
  if (compileFlags.optimize) {
    runFullOptimizationPasses(M);
  } else {
    runNoOptimizationPasses(M);
  }
#endif

  BytecodeGenerationOptions opts{OutputFormatKind::None};
  opts.optimizationEnabled = compileFlags.optimize;
  opts.staticBuiltinsEnabled = compileFlags.staticBuiltins;
  opts.verifyIR = compileFlags.verifyIR;
  auto bytecode = createBCProviderFromSrc(
      hbc::generateBytecodeModule(&M, M.getTopLevelFunction(), opts));
  return {std::move(bytecode), std::string{}};
}

BCProviderLazy::BCProviderLazy(hbc::BytecodeFunction *bytecodeFunction)
    : bytecodeFunction_(bytecodeFunction) {
  // Lazy module should always contain one function to begin with.
  functionCount_ = 1;
}

#endif // HERMESVM_LEAN

} // namespace hbc
} // namespace hermes
