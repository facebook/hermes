/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UTILS_OPTIONS_H
#define HERMES_UTILS_OPTIONS_H

#include "llvh/ADT/StringRef.h"

namespace hermes {

enum OutputFormatKind {
  DumpNone,
  DumpAST,
  DumpTransformedAST,
  DumpJS,
  DumpTransformedJS,
  ViewCFG,
  DumpIR,
  DumpLIR,
  DumpRA,
  DumpLRA,
  DumpBytecode,
  EmitBundle,
  Execute,
};

/// \return true if \p name is a valid unit name. That is, it is non-empty and
/// only consists of alphanumeric characters and underscores.
inline bool isValidSHUnitName(llvh::StringRef name) {
  return !name.empty() && llvh::all_of(name, [](char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '_';
  });
}

/// Options controlling the type of output to generate.
/// TODO: Split these options for bytecode and SH code generation.
struct BytecodeGenerationOptions {
  /// The format of the output.
  OutputFormatKind format = Execute;

  /// Whether optimizations are enabled.
  bool optimizationEnabled = false;

  /// The name of the unit emitted by the SH backend. This can only contain
  /// alphanumeric characters and underscores.
  llvh::StringRef unitName = "this_unit";

  /// Whether the SH backend should emit a main function that executes the
  /// generated unit.
  bool emitMain = true;

  /// Whether to strip the debug info in the bytecode binary.
  bool stripDebugInfoSection = false;

  /// Whether to enable basic block profiling or not.
  bool basicBlockProfiling = false;

  /// Whether static builtins are enabled.
  bool staticBuiltinsEnabled = false;

  /// Whether the IR should be verified.
  bool verifyIR = false;

  /// Strip all function names to reduce string table size.
  bool stripFunctionNames = false;

  /// Whether to run the HBC ReorderRegisters pass.
  bool reorderRegisters = true;

  /// Add this much garbage after each function body (relative to its size).
  unsigned padFunctionBodiesPercent = 0;

  /// Strip the source map URL.
  bool stripSourceMappingURL = false;

  // Emit source locations in the resulting output.
  bool emitSourceLocations = false;

  // Emit #line directives in the resulting output.
  bool emitLineDirectives = false;

  // Emit asserts in the bytecode.
  bool emitAsserts = false;

  /* implicit */ BytecodeGenerationOptions(OutputFormatKind format)
      : format(format) {}

  static BytecodeGenerationOptions defaults() {
    return {Execute};
  }
};

} // namespace hermes

#endif
