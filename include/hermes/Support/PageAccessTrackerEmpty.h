/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_INSTRUMENTATION_PAGEACCESSTRACKERWINDOWS_H
#define HERMES_VM_INSTRUMENTATION_PAGEACCESSTRACKERWINDOWS_H

#include <memory>
#include <vector>
#include "llvm/Support/raw_ostream.h"

namespace hermes {

class PageAccessTracker {
 public:
  static std::unique_ptr<volatile PageAccessTracker> create(
      void *bufStart,
      size_t bufSize) {
    std::unique_ptr<volatile PageAccessTracker> tracker;
    return tracker;
  }

  std::vector<uint32_t> getPagesAccessed() volatile {
    return std::vector<uint32_t>();
  }

  std::vector<uint32_t> getMicros() volatile {
    return std::vector<uint32_t>();
  }

  bool printStats(llvm::raw_ostream &OS, bool json) volatile {
    return false;
  }

  bool printPageAccessedOrder(llvm::raw_ostream &OS, bool json) volatile {
    return false;
  }
};

} // namespace hermes

#endif // HERMES_VM_INSTRUMENTATION_PAGEACCESSTRACKERWINDOWS_H
