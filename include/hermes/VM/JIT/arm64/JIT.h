/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JIT_ARM64_JIT_H
#define HERMES_VM_JIT_ARM64_JIT_H

#include "hermes/VM/CodeBlock.h"

namespace hermes {
namespace vm {
namespace arm64 {

namespace DumpJitCode {
enum : unsigned {
  Code = 0x01,
  CompileStatus = 0x02,
  InstErr = 0x04,
  BRK = 0x40,
  EntryExit = 0x80,
};
}

/// All state related to JIT compilation.
class JITContext {
  class Compiler;

 public:
  /// Construct a JIT context. No executable memory is allocated before it is
  /// needed.
  /// \param enable whether JIT is enabled.
  JITContext(bool enable);
  ~JITContext();

  JITContext(const JITContext &) = delete;
  void operator=(const JITContext &) = delete;

  /// Compile a function to native code and return the native pointer. If the
  /// function was previously compiled, return the existing body. If it cannot
  /// be compiled, return nullptr.
  inline JITCompiledFunctionPtr compile(Runtime &runtime, CodeBlock *codeBlock);

  /// \return true if JIT compilation is enabled.
  bool isEnabled() const {
    return enabled_;
  }

  /// Enable or disable JIT compilation.
  void setEnabled(bool enabled) {
    enabled_ = enabled;
  }

  /// Enable or disable dumping JIT'ed Code.
  void setDumpJITCode(unsigned dump) {
    dumpJITCode_ = dump;
  }

  /// \return true if dumping JIT'ed Code is enabled.
  unsigned getDumpJITCode() {
    return dumpJITCode_;
  }

  /// Set the flag to fatally crash on JIT compilation errors.
  void setCrashOnError(bool crash) {
    crashOnError_ = crash;
  }

  /// \return true if we should fatally crash on JIT compilation errors.
  bool getCrashOnError() {
    return crashOnError_;
  }

  /// Set the flag to force jitting of all functions.
  void setForceJIT(bool force) {
    execThreshold_ = force ? 0 : DEFAULT_EXEC_THRESHOLD;
  }

  /// Set the flag to emit asserts in the JIT'ed code.
  void setEmitAsserts(bool emitAsserts) {
    emitAsserts_ = emitAsserts;
  }

  /// \return true if we should emit asserts in the JIT'ed code.
  bool getEmitAsserts() {
    return emitAsserts_;
  }

 private:
  /// Slow path that actually performs the compilation of the specified
  /// CodeBlock.
  JITCompiledFunctionPtr compileImpl(Runtime &runtime, CodeBlock *codeBlock);

 private:
  class Impl;
  std::unique_ptr<Impl> impl_{};

  /// Whether JIT compilation is enabled.
  bool enabled_{false};
  /// whether to dump JIT'ed code
  unsigned dumpJITCode_{0};
  /// whether to fatally crash on JIT compilation errors
  bool crashOnError_{false};
  /// Whether to emit asserts in the JIT'ed code.
  bool emitAsserts_{false};

  /// Execution threshold before a function is compiled.
  unsigned execThreshold_ = 0;

  /// The JIT default threshold for function execution count
  static constexpr uint32_t DEFAULT_EXEC_THRESHOLD = 0;
};

LLVM_ATTRIBUTE_ALWAYS_INLINE
inline JITCompiledFunctionPtr JITContext::compile(
    Runtime &runtime,
    CodeBlock *codeBlock) {
  auto ptr = codeBlock->getJITCompiled();
  if (LLVM_LIKELY(ptr))
    return ptr;
  if (LLVM_LIKELY(!enabled_))
    return nullptr;
  if (LLVM_LIKELY(codeBlock->getDontJIT()))
    return nullptr;
  if (LLVM_LIKELY(codeBlock->getExecutionCount() < DEFAULT_EXEC_THRESHOLD))
    return nullptr;
  return compileImpl(runtime, codeBlock);
}

} // namespace arm64
} // namespace vm
} // namespace hermes
#endif // HERMES_VM_JIT_ARM64_JIT_H
