/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_INSTRUMENTATION_PAGEACCESSTRACKERWINDOWS_H
#define HERMES_VM_INSTRUMENTATION_PAGEACCESSTRACKERWINDOWS_H

#include <memory>
#include <vector>
#include "hermes/Support/JSONEmitter.h"
#include "llvh/Support/raw_ostream.h"

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

  bool printStats(llvh::raw_ostream &OS, bool json) volatile {
    return false;
  }

  void getJSONStats(JSONEmitter &json) volatile {
    json.emitNullValue();
  }

  bool printPageAccessedOrder(llvh::raw_ostream &OS, bool json) volatile {
    return false;
  }
};

} // namespace hermes

#endif // HERMES_VM_INSTRUMENTATION_PAGEACCESSTRACKERWINDOWS_H
