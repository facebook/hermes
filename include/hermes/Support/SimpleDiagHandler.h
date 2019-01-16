#ifndef HERMES_SUPPORT_SIMPLEDIAGHANDLER_H
#define HERMES_SUPPORT_SIMPLEDIAGHANDLER_H

#include "llvm/Support/SourceMgr.h"

namespace hermes {

/// A diagnostic handler for \c SourceErrorManager and llvm::SourceMgr that
/// remembers the first error and ignores the rest.
class SimpleDiagHandler {
 public:
  /// Install this handler into the specified SourceMgr.
  void installInto(llvm::SourceMgr &sourceMgr);

  /// \return true if an error message has been tracked.
  bool hasFirstMessage() const {
    return !firstMessage_.getMessage().empty();
  }

  /// \return the first error message received.
  const llvm::SMDiagnostic &getFirstMessage() const {
    return firstMessage_;
  }

  /// \return an error string, containing the line and column.
  std::string getErrorString() const;

 private:
  /// First error message given to handler(), if it exists.
  llvm::SMDiagnostic firstMessage_;

  /// The actual handler callback.
  static void handler(const llvm::SMDiagnostic &msg, void *ctx);
};

/// A RAII wrapper around \c SimpleDiagHandler that automatically installs
/// and uninstalls itself.
class SimpleDiagHandlerRAII : public SimpleDiagHandler {
 public:
  SimpleDiagHandlerRAII(llvm::SourceMgr &sourceMgr);
  ~SimpleDiagHandlerRAII();

 private:
  /// The SourceMgr where this handler is installed.
  llvm::SourceMgr &sourceMgr_;

  /// The previous handler, to be restored on destruction.
  void (*const oldHandler_)(const llvm::SMDiagnostic &, void *);
  /// The previous context, to be restoed on destruction.
  void *const oldContext_;
};

} // namespace hermes

#endif // HERMES_SUPPORT_SIMPLEDIAGHANDLER_H
