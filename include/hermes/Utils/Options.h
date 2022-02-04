/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UTILS_OPTIONS_H
#define HERMES_UTILS_OPTIONS_H

namespace hermes {

enum OutputFormatKind {
  DumpAST,
  DumpTransformedAST,
  DumpJS,
  DumpTransformedJS,
  ViewCFG,
  DumpIR,
  DumpLIR,
  DumpRA,
  DumpLRA,
  DumpPostRA,
  DumpBytecode,
  EmitBundle,
  Execute,
};

/// Options controlling the type of output to generate.
struct BytecodeGenerationOptions {
  /// The format of the output.
  OutputFormatKind format = Execute;

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

  /// Strip the source map URL.
  bool stripSourceMappingURL = false;

  /* implicit */ BytecodeGenerationOptions(OutputFormatKind format)
      : format(format) {}

  static BytecodeGenerationOptions defaults() {
    return {Execute};
  }
};

} // namespace hermes

#endif
