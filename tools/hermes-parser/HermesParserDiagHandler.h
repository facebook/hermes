/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

 private:
  /// The SourceErrorManager where this handler is installed.
  SourceErrorManager &sm_;

  /// First error given to handler(), if one exists.
  llvh::SMDiagnostic firstError_;

  /// The actual handler callback.
  static void handler(const llvh::SMDiagnostic &msg, void *ctx);
};

} // namespace hermes

#endif // HERMES_TOOLS_HERMESPARSER_HERMESPARSERDIAGHANDLER_H
