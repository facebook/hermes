/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_HBC_H
#define HERMES_BCGEN_HBC_HBC_H

#include "hermes/BCGen/HBC/BCProvider.h"
#include "hermes/BCGen/HBC/BCProviderFromSrc.h"
#include "hermes/BCGen/HBC/Bytecode.h"
#include "hermes/IR/IR.h"
#include "hermes/Support/OptValue.h"
#include "hermes/Support/SHA1.h"
#include "hermes/Utils/Dumper.h"
#include "hermes/Utils/Options.h"

namespace llvh {
class raw_ostream;
} // namespace llvh

namespace hermes {
class SourceMapGenerator;
namespace hbc {

/// Generates a BytecodeModule from a module \p M, and will return a unique_ptr
/// to the new module. The \p options parameter controls bytecode generation. If
/// \p sourceMap is not null, populate it with debug information from the
/// generated module.
/// \p baseBCProvider is the base bytecode used in delta optimizing mode;
/// when it is not null and optimization is turned on, we optimize for the delta
/// size between base and current bytecode.
/// \returns a pointer to the BytecodeModule.
std::unique_ptr<BytecodeModule> generateBytecodeModule(
    Module *M,
    Function *entryPoint,
    const BytecodeGenerationOptions &options,
    hermes::OptValue<uint32_t> segment = llvh::None,
    SourceMapGenerator *sourceMap = nullptr,
    std::unique_ptr<BCProviderBase> baseBCProvider = nullptr);

/// Generate bytecode for a lazy function and mutate the BytecodeModule
/// accordingly.
/// Called after parser/resolver/IRGen have run by lazy compilation.
/// \param bm the bytecode module to be modified.
/// \param M the IR module containing the lazy function.
/// \param lazyFunc the lazy function to be compiled.
/// \param lazyFuncID the ID of the lazy function.
/// \param options the bytecode generation options.
bool generateBytecodeFunctionLazy(
    BytecodeModule &bm,
    Module *M,
    Function *lazyFunc,
    uint32_t lazyFuncID,
    const BytecodeGenerationOptions &options);

/// Generates a BytecodeModule from a module \p M, and will return a unique_ptr
/// to the new module. The \p options parameter controls bytecode generation.
/// \returns a pointer to the BytecodeModule.
std::unique_ptr<BytecodeModule> generateBytecodeModuleForEval(
    Module *M,
    Function *entryPoint,
    const BytecodeGenerationOptions &options);

/// Generate the bytecode for the lazy function by running the full compiler
/// pipeline without optimizations.
/// Mutates the underlying BytecodeModule of the BCProviderFromSrc and updates
/// all references stored in the BCProviderFromSrc.
/// Runs the compiler on a separate thread to avoid stack overflows, blocking
/// the current thread while doing so.
///
/// \param provider the BCProviderFromSrc owning the BytecodeModule.
///   Passed in as BCProvider to avoid BCProviderFromSrc dependencies in
///   CodeBlock (for simplicity).
/// \param funcID the ID of the lazy function to generate.
/// \return [success, errMsg] where errMsg is only populated when success is
///   false. The error message is stored in the lazy BytecodeFunction which
///   failed to compile.
///
/// NOTE: This function is exposed here for convenience, because there's no
/// better place to put it to access all the parts of the compiler.
/// It calls \c generateBytecodeFunctionLazy, but that function is also exposed
/// on principle because it's an actual entry point into the backend - this
/// function is really a driver for the entire compiler.
std::pair<bool, llvh::StringRef> compileLazyFunction(
    hbc::BCProvider *baseProvider,
    uint32_t funcID);

/// \pre the BytecodeFunction at funcID is lazy.
/// \param provider the BCProviderFromSrc owning the BytecodeModule.
///   Passed in as BCProvider to avoid BCProviderFromSrc dependencies in
///   CodeBlock (for simplicity).
/// \param funcID the ID of the lazy function.
/// \param line 1-based line.
/// \param col 1-based column.
/// \return whether the line/col location is contained in the lazy function.
bool coordsInLazyFunction(
    hbc::BCProvider *provider,
    uint32_t funcID,
    uint32_t line,
    uint32_t col);

/// Generate the bytecode for eval function by running the full compiler
/// pipeline without optimizations.
/// Creates a new BytecodeModule.
/// Mutates the IR Module (creates new Functions).
/// Creates a new SemContext as a child of the input provider's SemContext.
/// Runs the compiler on a separate thread to avoid stack overflows, blocking
/// the current thread while doing so.
///
/// \param src the JS source to be compiled.
/// \param provider the BCProviderFromSrc owning the BytecodeModule.
///   Passed in as BCProvider to avoid BCProviderFromSrc dependencies in
///   CodeBlock (for simplicity).
/// \param enclosingFuncID the ID of the function enclosing the new eval.
/// \return [success, errMsg] where errMsg is only populated when success is
///   false.
std::pair<std::unique_ptr<BCProviderFromSrc>, std::string> compileEvalModule(
    std::unique_ptr<Buffer> src,
    hbc::BCProviderFromSrc *provider,
    uint32_t enclosingFuncID,
    const CompileFlags &compileFlags);

} // namespace hbc
} // namespace hermes

#endif
