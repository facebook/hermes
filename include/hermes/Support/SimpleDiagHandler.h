/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_SIMPLEDIAGHANDLER_H
#define HERMES_SUPPORT_SIMPLEDIAGHANDLER_H

#include "hermes/Support/SourceErrorManager.h"

namespace hermes {

/// A diagnostic handler for \c SourceErrorManager that remembers the first
///     error and ignores the rest.
class SimpleDiagHandler {
 public:
  /// Install this handler into the specified SourceErrorManager.
  void installInto(SourceErrorManager &sm);

  /// \return true if an error message has been tracked.
  bool haveErrors() const {
    return !firstMessage_.getMessage().empty();
  }

  /// \return the first error message received.
  const llvh::SMDiagnostic &getFirstMessage() const {
    assert(haveErrors() && "getFirstMessage() called without errors");
    return firstMessage_;
  }

  /// \return an error string, containing the line and column.
  std::string getErrorString() const;

 private:
  /// First error message given to handler(), if it exists.
  llvh::SMDiagnostic firstMessage_;

  /// The actual handler callback.
  static void handler(const llvh::SMDiagnostic &msg, void *ctx);
};

/// A RAII wrapper around \c SimpleDiagHandler that automatically installs
/// and uninstalls itself. It also sets an error limit of 1.
class SimpleDiagHandlerRAII : public SimpleDiagHandler {
 public:
  SimpleDiagHandlerRAII(SourceErrorManager &sourceErrorManager);
  ~SimpleDiagHandlerRAII();

 private:
  /// The SourceErrorManager where this handler is installed.
  SourceErrorManager &sourceErrorManager_;

  /// The previous handler, to be restored on destruction.
  void (*const oldHandler_)(const llvh::SMDiagnostic &, void *);
  /// The previous context, to be restoed on destruction.
  void *const oldContext_;
  /// The previous error limit.
  unsigned const oldErrorLimit_;
};

} // namespace hermes

#endif // HERMES_SUPPORT_SIMPLEDIAGHANDLER_H
