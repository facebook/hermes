/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JIT_JIT_H
#define HERMES_VM_JIT_JIT_H

#include "hermes/VM/JIT/Config.h"

#if HERMESVM_JIT

#if defined(__aarch64__) || defined(_M_ARM64)
#include "hermes/VM/JIT/arm64/JIT.h"
#elif defined(__x86_64__) || defined(_M_X64)
#include "hermes/VM/JIT/x86-64/JIT.h"
#endif

namespace hermes {
namespace vm {

#if defined(__aarch64__) || defined(_M_ARM64)
using arm64::JITContext;
namespace arm64 {
class Emitter;
}
#define FRIEND_JIT                \
  friend class arm64::JITContext; \
  friend class arm64::Emitter;
#elif defined(__x86_64__) || defined(_M_X64)
using x86_64::JITContext;
#define FRIEND_JIT friend class x86_64::JITContext;
#endif

} // namespace vm
} // namespace hermes

#else

#include "hermes/VM/CodeBlock.h"

#define FRIEND_JIT

namespace hermes {
namespace vm {

/// All state related to JIT compilation.
class JITContext {
 public:
  /// Construct a JIT context. No executable memory is allocated before it is
  /// needed.
  /// \param enable whether JIT is enabled.
  JITContext(bool enable) {}
  ~JITContext() = default;

  JITContext(const JITContext &) = delete;
  void operator=(const JITContext &) = delete;

  /// Compile a function to native code and return the native pointer. If the
  /// function was previously compiled, return the existing body. If it cannot
  /// be compiled, return nullptr.
  inline JITCompiledFunctionPtr compile(
      Runtime &runtime,
      CodeBlock *codeBlock) {
    return codeBlock->getJITCompiled();
  }

  /// \return true if JIT compilation is enabled.
  bool isEnabled() const {
    return false;
  }

  /// Enable or disable JIT compilation.
  void setEnabled(bool enabled) {}

  /// Enable or disable dumping JIT'ed Code.
  void setDumpJITCode(unsigned dump) {}

  /// \return true if dumping JIT'ed Code is enabled.
  unsigned getDumpJITCode() {
    return 0;
  }

  /// Construct data structure used for perf profiling support. This should be
  /// called only when PerfProf is enabled and perf JITContext.
  /// \param jitdumpFd The file descriptor of the opened jitdump file.
  /// \param commentFd The file descriptor of the opended \p commentFile.
  /// \param commentFile The path of the file to store the comments.
  void initPerfProfData(
      int jitdumpFd,
      int commentFd,
      const std::string &commentFile) {}

  /// Set the flag to fatally crash on JIT compilation errors.
  void setCrashOnError(bool crash) {}

  /// \return true if we should fatally crash on JIT compilation errors.
  bool getCrashOnError() {
    return false;
  }

  /// Set the flag to force jitting of all functions.
  void setForceJIT(bool force) {}

  /// Set the memory limit for JIT'ed code in bytes.
  void setMemoryLimit(uint32_t memoryLimit) {}

  /// Set the default threshold for function execution count before a function
  /// is compiled. On a per-function basis, the count may be altered based on
  /// internal heuristics.
  /// Can be overridden by setForceJIT(true).
  void setDefaultExecThreshold(uint32_t threshold) {}

  /// Set the flag to emit asserts in the JIT'ed code.
  void setEmitAsserts(bool emitAsserts) {}

  /// \return true if we should emit asserts in the JIT'ed code.
  bool getEmitAsserts() {
    return false;
  }
};

} // namespace vm
} // namespace hermes

#endif // HERMESVM_JIT

#endif // HERMES_VM_JIT_JIT_H
