/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JIT_JIT_H
#define HERMES_VM_JIT_JIT_H

#ifdef HERMESVM_JIT

#include "hermes/VM/JIT/x86-64/JIT.h"

namespace hermes {
namespace vm {

using x86_64::JITContext;

} // namespace vm
} // namespace hermes

#else

#include "hermes/VM/CodeBlock.h"

namespace hermes {
namespace vm {

/// All state related to JIT compilation.
class JITContext {
 public:
  /// Construct a JIT context. No executable memory is allocated before it is
  /// needed.
  /// \param enable whether JIT is enabled.
  /// \param blockSize the size of individual blocks of executable memory to be
  ///     allocated.
  /// \param maximum amount of executable memory that can be allocated by the
  ///     JIT.
  JITContext(bool enable, size_t blockSize, size_t maxMemory) {}
  ~JITContext() = default;

  JITContext(const JITContext &) = delete;
  void operator=(const JITContext &) = delete;

  /// Compile a function to native code and return the native pointer. If the
  /// function was previously compiled, return the existing body. If it cannot
  /// be compiled, return nullptr.
  inline JITCompiledFunctionPtr compile(
      Runtime *runtime,
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
};

} // namespace vm
} // namespace hermes

#endif // HERMESVM_JIT

#endif // HERMES_VM_JIT_FASTJIT_H
