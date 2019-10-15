/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UTILS_OPTIONS_H
#define HERMES_UTILS_OPTIONS_H

namespace hermes {

enum OutputFormatKind {
  None = 0,
  DumpAST,
  DumpTransformedAST,
  ViewCFG,
  DumpIR,
  DumpLIR,
  DumpRA,
  DumpLRA,
  DumpPostRA,
  DumpBytecode,
  EmitBundle
};

/// Options controlling the type of output to generate.
struct BytecodeGenerationOptions {
  /// The format of the output.
  OutputFormatKind format = None;

  /// Whether optimizations are enabled.
  bool optimizationEnabled = false;

  /// Whether to strip the debug info in the bytecode binary.
  bool stripDebugInfoSection = false;

  /// Whether to print the disassembled bytecode in a more human readable
  /// format.
  bool prettyDisassemble = true;

  /// Whether to enable basic block profiling or not.
  bool basicBlockProfiling = false;

  /// Whether static builtins are enabled.
  bool staticBuiltinsEnabled = false;

  /// Whether the IR should be verified.
  bool verifyIR = false;

  /// Strip all function names to reduce string table size.
  bool stripFunctionNames = false;

  /// Add this much garbage after each function body (relative to its size).
  unsigned padFunctionBodiesPercent = 0;

  /* implicit */ BytecodeGenerationOptions(OutputFormatKind format)
      : format(format) {}

  static BytecodeGenerationOptions defaults() {
    return {None};
  }
};

} // namespace hermes

#endif
