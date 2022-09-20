/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef SHERMES_COMPILE_H
#define SHERMES_COMPILE_H

#include "hermes/AST/Context.h"
#include "hermes/IR/IR.h"
#include "hermes/Utils/Options.h"

enum class OutputLevelKind {
  None,
  AST,
  TransformedAST,
  CFG,
  IR,
  LIR,
  RA,
  LRA,
  PostRA,
  C,
  Asm,
  Obj,
  Executable,
  Run,
};

enum class OptLevel {
  O0,
  Og,
  Os,
  OMax,
};

struct ShermesCompileParams {
  const hermes::BytecodeGenerationOptions &genOptions;
  OptLevel nativeOptimize;
  enum class Lean { off, on };
  Lean lean = Lean::off;
  int verbosity;

  ShermesCompileParams(
      hermes::BytecodeGenerationOptions const &genOptions,
      OptLevel nativeOptimize,
      Lean lean,
      int verbosity)
      : genOptions(genOptions),
        nativeOptimize(nativeOptimize),
        lean(lean),
        verbosity(verbosity) {}
};

/// Compile the input to the specified output level.
/// Errors are printed to STDERR.
bool shermesCompile(
    hermes::Context *context,
    hermes::Module &M,
    const ShermesCompileParams &params,
    OutputLevelKind outputLevel,
    llvh::StringRef inputFilename,
    llvh::StringRef outputFilename,
    llvh::ArrayRef<std::string> execArgs);

#endif // SHERMES_COMPILE_H
