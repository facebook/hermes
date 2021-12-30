/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_TOOLS_HERMESPARSER_HERMESPARSERDIAGHANDLER_H
#define HERMES_TOOLS_HERMESPARSER_HERMESPARSERDIAGHANDLER_H

#include "hermes/Support/SourceErrorManager.h"

namespace hermes {

/// A diagnostic handler for \c SourceErrorManager to be used by the WASM Hermes
/// parser. This diagnostic handler remembers the first error and ignores the
/// rest, as well as provides custom error formatting for WASM Hermes parser
/// errors.
class HermesParserDiagHandler {
 public:
  HermesParserDiagHandler(SourceErrorManager &sm);

  /// \return true if an error has been tracked.
  bool hasError() const {
    return !firstError_.getMessage().empty();
  }

  /// \return the error string formatted for display.
  std::string getErrorString() const;

  /// \return the line number of the error, if one exists.
  uint32_t getErrorLine() const {
    return firstError_.getLineNo();
  }

  /// \return the column number of the error, if one exists.
  uint32_t getErrorColumn() const {
    return firstError_.getColumnNo();
  }

 private:
  /// The SourceErrorManager where this handler is installed.
  SourceErrorManager &sm_;

  /// First error given to handler(), if one exists.
  llvh::SMDiagnostic firstError_;

  /// Notes that follow the first error, if one exists.
  std::vector<llvh::SMDiagnostic> firstErrorNotes_;

  /// Whether notes for the first error can still be collected.
  bool collectNotes_ = true;

  /// The actual handler callback.
  static void handler(const llvh::SMDiagnostic &msg, void *ctx);

  std::string formatDiagnostic(const llvh::SMDiagnostic &diag) const;
};

} // namespace hermes

#endif // HERMES_TOOLS_HERMESPARSER_HERMESPARSERDIAGHANDLER_H
