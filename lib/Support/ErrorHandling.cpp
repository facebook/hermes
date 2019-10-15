/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/ErrorHandling.h"

#include "llvm/ADT/Twine.h"

namespace hermes {

const std::error_category &oom_category() {
  class OOMErrorCategory final : public std::error_category {
    const char *name() const noexcept override {
      return "vm_allocate_category";
    }

    std::string message(int condition) const override {
      switch (static_cast<OOMError>(condition)) {
        case OOMError::None:
          return "No error";
        case OOMError::MaxHeapReached:
          return "Max heap size was exceeded";
        case OOMError::MaxStorageReached:
          return "Number of storages requested exceeded the limit";
        case OOMError::Effective:
          return "Effective OOM";
        case OOMError::SuperSegmentAlloc:
          return "Allocation occurred that was larger than a heap segment";
        case OOMError::CopyableVectorCapacityIntegerOverflow:
          return "CopyableVector capacity integer overflow";
        case OOMError::TestVMLimitReached:
          return "A test set a limit for virtual memory that was exceeded";
      }
      return "Unknown";
    }
  };

  static OOMErrorCategory category;
  return category;
}

std::error_code make_error_code(OOMError err) {
  return std::error_code(static_cast<int>(err), oom_category());
}

std::string convert_error_to_message(std::error_code code) {
  return (llvm::Twine("error_code(value = ") + llvm::Twine(code.value()) +
          ", category = " + code.category().name() +
          ", message = " + code.message() + ")")
      .str();
}

LLVM_ATTRIBUTE_NORETURN void hermes_fatal(const char *msg) {
  llvm::report_fatal_error(msg);
}

LLVM_ATTRIBUTE_NORETURN void hermes_fatal(const std::string &msg) {
  llvm::report_fatal_error(msg.c_str());
}

} // namespace hermes
