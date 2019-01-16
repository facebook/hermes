#ifndef HERMES_SUPPORT_ERRORHANDLING_H
#define HERMES_SUPPORT_ERRORHANDLING_H

#include "llvm/Support/ErrorHandling.h"

#include <cstdlib>

namespace hermes {

/// Reports a serious error and aborts execution. Calls any installed error
/// handlers (or defaults to print the message to stderr), then calls exit(1).
LLVM_ATTRIBUTE_NORETURN inline void hermes_fatal(const char *msg) {
  llvm::report_fatal_error(msg);
}

} // namespace hermes

#endif // HERMES_SUPPORT_ERRORHANDLING_H
