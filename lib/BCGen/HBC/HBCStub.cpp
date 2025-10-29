/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// Dummy implementation of HBC APIs to be used by a lean build of the VM.
/// Reports errors and returns no-op results.
//===----------------------------------------------------------------------===//

#include "hermes/BCGen/HBC/HBC.h"

namespace hermes {
namespace hbc {

void fullOptimizationPipeline(Module &M) {
  // No optimizer.
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
  return {nullptr, "Lean VM does not support bytecode generation"};
}

std::unique_ptr<BytecodeModule> generateBytecodeModule(
    Module *M,
    Function *entryPoint,
    const BytecodeGenerationOptions &options,
    hermes::OptValue<uint32_t> segment,
    std::unique_ptr<BCProviderBase> baseBCProvider) {
  return nullptr;
}

bool generateBytecodeFunctionLazy(
    BytecodeModule &bm,
    Module *M,
    Function *lazyFunc,
    uint32_t lazyFuncID,
    FileAndSourceMapIdCache &debugIdCache,
    const BytecodeGenerationOptions &options) {
  return false;
}

std::unique_ptr<BytecodeModule> generateBytecodeModuleForEval(
    Module *M,
    Function *entryPoint,
    const BytecodeGenerationOptions &options) {
  return nullptr;
}

std::pair<bool, llvh::StringRef> compileLazyFunction(
    hbc::BCProvider *baseProvider,
    uint32_t funcID) {
  return {false, "Lean VM does not support bytecode generation"};
}

SMLoc findSMLocFromCoords(
    hbc::BCProvider *baseProvider,
    uint32_t line,
    uint32_t col) {
  return SMLoc{};
}

bool coordsInLazyFunction(
    hbc::BCProvider *baseProvider,
    uint32_t funcID,
    SMLoc loc) {
  return false;
}

std::pair<std::unique_ptr<BCProvider>, std::string> compileEvalModule(
    std::unique_ptr<Buffer> src,
    hbc::BCProvider *provider,
    uint32_t enclosingFuncID,
    const CompileFlags &compileFlags,
    uint32_t lexicalScopeIdxInParentFunction) {
  return {nullptr, "Lean VM does not support bytecode generation"};
}

std::vector<uint32_t> getVariableCounts(
    hbc::BCProvider *provider,
    uint32_t funcID,
    uint32_t lexicalScopeIdxInParentFunction) {
  return {0};
}

VariableInfoAtDepth getVariableInfoAtDepth(
    hbc::BCProvider *provider,
    uint32_t funcID,
    uint32_t depth,
    uint32_t variableIndex,
    uint32_t lexicalScopeIdxInParentFunction) {
  hermes_fatal("Lean VM does not support debugging");
}

void serializeBytecodeModule(
    BytecodeModule &BM,
    const SHA1 &sourceHash,
    llvh::raw_ostream &os,
    BytecodeGenerationOptions options) {
  hermes_fatal("Lean VM does not support bytecode serialization");
}

} // namespace hbc
} // namespace hermes

#undef DEBUG_TYPE
