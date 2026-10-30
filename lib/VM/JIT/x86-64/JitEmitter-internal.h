/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT

#include "JitEmitter.h"

#include "../RuntimeOffsets.h"

namespace hermes::vm::x86_64 {

// Ensure that HermesValue tags are handled correctly by updating this every
// time the HERMESVALUE_VERSION changes, and going through the JIT and updating
// any relevant code.
static_assert(
    HERMESVALUE_VERSION == 2,
    "HermesValue version mismatch, JIT may need to be updated");

/// This macro is used to catch and handle low probability instructing encoding
/// errors - i.e. when an immediate operand doesn't fit in the instruction
/// encoding. It causes Asmjit to just return an error code instead of
/// terminating the entire compilation.
///
/// \param expValue the error value that we want to handle.
/// \param code  C++ code to invoke asmjit and store the result in a variable.
#define EXPECT_ERROR(expValue, code)          \
  do {                                        \
    assert(                                   \
        expectedError_ == asmjit::kErrorOk && \
        "expectedError_ is not cleared");     \
    expectedError_ = (expValue);              \
    code;                                     \
    expectedError_ = asmjit::kErrorOk;        \
  } while (0)

/// Save the current IP and emit a call to a runtime function. This should be
/// used in most cases when invoking slow paths and handlers for complex
/// functionality.
#define EMIT_RUNTIME_CALL(em, type, func)             \
  do {                                                \
    using _FnT = type;                                \
    _FnT _fn = func;                                  \
    (void)_fn;                                        \
    (em).callRuntimeWithSavedIP((void *)func, #func); \
  } while (0)

/// Call a runtime function without saving the IP. This is intended for special
/// cases where we want to preserve the currently saved IP or if the IP is not
/// needed.
#define EMIT_RUNTIME_CALL_WITHOUT_SAVED_IP(em, type, func) \
  do {                                                     \
    using _FnT = type;                                     \
    _FnT _fn = func;                                       \
    (void)_fn;                                             \
    (em).callRuntime((void *)func, #func);                 \
  } while (0)

class OurErrorHandler : public asmjit::ErrorHandler {
  asmjit::Error &expectedError_;
  std::function<void(std::string &&message)> const longjmpError_;

 public:
  /// \param expectedError if we get an error matching this value, we ignore it.
  explicit OurErrorHandler(
      asmjit::Error &expectedError,
      const std::function<void(std::string &&message)> &longjmpError)
      : expectedError_(expectedError), longjmpError_(longjmpError) {}

  void handleError(
      asmjit::Error err,
      const char *message,
      asmjit::BaseEmitter *origin) override;
};

#ifndef ASMJIT_NO_LOGGING
class OurLogger : public asmjit::Logger {
 private:
  x86::Assembler &a_;
  PerfJitDump *perfJitDump_{nullptr};
  bool dumpJitCode_{false};

 public:
  OurLogger(x86::Assembler &a, PerfJitDump *perfJitDump, bool dumpJitCode)
      : a_(a), perfJitDump_(perfJitDump), dumpJitCode_(dumpJitCode) {}

  ASMJIT_API asmjit::Error _log(const char *data, size_t size) noexcept
      override;
};
#endif

} // namespace hermes::vm::x86_64

#endif // HERMESVM_JIT
