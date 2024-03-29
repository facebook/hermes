/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/HBC.h"

#include "hermes/BCGen/HBC/BytecodeGenerator.h"
#include "hermes/Support/PerfSection.h"

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

  return BytecodeModuleGenerator{
      M, options, sourceMapGen, std::move(baseBCProvider)}
      .generate(entryPoint, segment);
}

} // namespace hbc
} // namespace hermes

#undef DEBUG_TYPE
