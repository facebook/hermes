/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_COMPILEJS_H
#define HERMES_COMPILEJS_H

#include <string>
#include <vector>

namespace hermes {

/// Interface for receiving errors, warnings and notes produced by compileJS.
class DiagnosticHandler {
 public:
  enum Kind {
    Error,
    Warning,
    Note,
  };

  struct Diagnostic {
    Kind kind;
    int line; /// 1-based index
    int column; /// 1-based index
    std::string message;
    /// 0-based char indices in half-open intervals
    std::vector<std::pair<unsigned, unsigned>> ranges;
  };

  /// Called once for each diagnostic message produced during compilation.
  virtual void handle(const Diagnostic &diagnostic) = 0;
  virtual ~DiagnosticHandler() = default;
};

/// Compiles JS source \p str and if compilation is successful, returns true
/// and outputs to \p bytecode otherwise returns false.
/// \param sourceURL this will be used as the "file name" of the buffer for
///   errors, stack traces, etc.
/// \param optimize this will enable optimizations, but only if the
/// HERMESVM_ENABLE_OPTIMIZATION_AT_RUNTIME preprocessor directive is set.
/// \param diagHandler if not null, receives any and all errors, warnings and
///   notes produced during compilation.
bool compileJS(
    const std::string &str,
    const std::string &sourceURL,
    std::string &bytecode,
    bool optimize,
    DiagnosticHandler *diagHandler);

bool compileJS(
    const std::string &str,
    std::string &bytecode,
    bool optimize = true);

bool compileJS(
    const std::string &str,
    const std::string &sourceURL,
    std::string &bytecode,
    bool optimize = true);

} // namespace hermes

#endif
