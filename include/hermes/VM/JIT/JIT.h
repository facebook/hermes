/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JIT_JIT_H
#define HERMES_VM_JIT_JIT_H

#include "hermes/VM/JIT/Config.h"

#include "hermes/VM/CodeBlock.h"

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
  void setDumpJITCode(bool dump) {}

  /// \return true if dumping JIT'ed Code is enabled.
  bool getDumpJITCode() {
    return false;
  }

  /// Set the flag to fatally crash on JIT compilation errors.
  void setCrashOnError(bool crash) {}

  /// \return true if we should fatally crash on JIT compilation errors.
  bool getCrashOnError() {
    return false;
  }

  /// Whether to force jitting of all functions right away.
  void setForceJIT(bool force) {}
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JIT_JIT_H
