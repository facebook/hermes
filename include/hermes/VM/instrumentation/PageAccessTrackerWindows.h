/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_INSTRUMENTATION_PAGEACCESSTRACKERWINDOWS_H
#define HERMES_VM_INSTRUMENTATION_PAGEACCESSTRACKERWINDOWS_H

#include "llvm/Support/raw_ostream.h"

namespace hermes {

class PageAccessTracker {
 public:
  static bool initialize(void *bufStart, size_t bufSize) {
    return false;
  }

  static bool printStats(llvm::raw_ostream &OS, bool json) {
    return false;
  }

  static bool printPageAccessedOrder(llvm::raw_ostream &OS, bool json) {
    return false;
  }

  static bool shutdown() {
    return true;
  }
};

} // namespace hermes

#endif // HERMES_VM_INSTRUMENTATION_PAGEACCESSTRACKERWINDOWS_H
