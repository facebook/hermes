/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_SUPPORT_ERRORHANDLING_H
#define HERMES_SUPPORT_ERRORHANDLING_H

#include "llvm/Support/ErrorHandling.h"

#include <cstdlib>

/// This macro is always enabled.  It exists to mark code that intended to
/// do some extra checking in the service of debugging some production crash;
/// if we figure out the problem, we might wish to delete them.
#define HERMES_EXTRA_DEBUG(x) x

namespace hermes {

/// Reports a serious error and aborts execution. Calls any installed error
/// handlers (or defaults to print the message to stderr), then calls exit(1).
LLVM_ATTRIBUTE_NORETURN inline void hermes_fatal(const char *msg) {
  llvm::report_fatal_error(msg);
}

} // namespace hermes

#endif // HERMES_SUPPORT_ERRORHANDLING_H
