/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT_X86_64
#include "JitEmitter-internal.h"

#define DEBUG_TYPE "jit"

namespace hermes::vm::x86_64 {

void OurErrorHandler::handleError(
    asmjit::Error err,
    const char *message,
    asmjit::BaseEmitter *origin) {
  if (err == expectedError_) {
    LLVM_DEBUG(
        llvh::outs() << "Expected AsmJit error: " << err << ": "
                     << asmjit::DebugUtils::errorAsString(err) << ": "
                     << message << "\n");
    return;
  }

  std::string formattedMsg{};
  {
    // Ensure we run any destructors for the ostream before longjmp.
    llvh::raw_string_ostream OS{formattedMsg};
    OS << "AsmJit error: " << err << ": "
       << asmjit::DebugUtils::errorAsString(err) << ": " << message;
    OS.flush();
  }

  // IMPORTANT: From here on, we MUST ensure that no destructors need to run.
  // One exception: formattedMsg will have its destructor skipped, but we're
  // moving out of it so in practice the std::string won't have anything to
  // free, avoiding leaks.
  LLVM_DEBUG(llvh::dbgs() << formattedMsg << "\n");
  longjmpError_(std::move(formattedMsg));
}

#ifndef ASMJIT_NO_LOGGING
asmjit::Error OurLogger::_log(const char *data, size_t size) noexcept {
  auto str =
      (size == SIZE_MAX ? llvh::StringRef(data) : llvh::StringRef(data, size));
  if (str.empty())
    return asmjit::kErrorOk;
  if (dumpJitCode_)
    llvh::outs() << str;
  if (!perfJitDump_)
    return asmjit::kErrorOk;

  // Comments by default do not have indentation, except some pseudocode
  // comments, which start with ';' after indentation.
  auto trimmed = str.ltrim();
  if (str.front() != ' ' || (!trimmed.empty() && trimmed.front() == ';')) {
    perfJitDump_->addCodeComment(str, a_.offset());
  }
  return asmjit::kErrorOk;
}
#endif

} // namespace hermes::vm::x86_64
#endif // HERMESVM_JIT_X86_64
