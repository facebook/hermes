/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JIT_X86_64_JIT_H
#define HERMES_VM_JIT_X86_64_JIT_H

#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/JIT/ExecHeap.h"
#include "hermes/VM/JIT/NativeDisassembler.h"

namespace hermes {
namespace vm {
namespace x86_64 {

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
  JITContext(bool enable, size_t blockSize, size_t maxMemory);
  ~JITContext();

  JITContext(const JITContext &) = delete;
  void operator=(const JITContext &) = delete;

  /// Compile a function to native code and return the native pointer. If the
  /// function was previously compiled, return the existing body. If it cannot
  /// be compiled, return nullptr.
  inline JITCompiledFunctionPtr compile(Runtime *runtime, CodeBlock *codeBlock);

  /// \return true if JIT compilation is enabled.
  bool isEnabled() const {
    return enabled_;
  }

  /// Enable or disable JIT compilation.
  void setEnabled(bool enabled) {
    enabled_ = enabled;
  }

  /// Enable or disable dumping JIT'ed Code.
  void setDumpJITCode(bool dump) {
    dumpJITCode_ = dump;
  }

  /// \return true if dumping JIT'ed Code is enabled.
  bool getDumpJITCode() {
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

  /// \return the executable memory heap.
  ExecHeap &getHeap() {
    return heap_;
  }

  /// \return the native disassembler for our target.
  NativeDisassembler &getDisassembler() {
    return *dis_;
  }

 private:
  /// Slow path that actually performs the compilation of the specified
  /// CodeBlock.
  JITCompiledFunctionPtr compileImpl(Runtime *runtime, CodeBlock *codeBlock);

 private:
  /// Whether JIT compilation is enabled.
  bool enabled_{false};
  /// Executable heap where all executable code is allocated.
  ExecHeap heap_;
  /// whether to dump JIT'ed code
  bool dumpJITCode_{false};
  /// whether to fatally crash on JIT compilation errors
  bool crashOnError_{false};

  /// The disassembler for our target.
  std::unique_ptr<NativeDisassembler> dis_ =
      NativeDisassembler::create(NativeDisassembler::x86_64_unknown_linux_gnu);

  /// The JIT compile threshold for function execution count
  static constexpr uint32_t COMPILE_THRESHOLD = 0;
};

LLVM_ATTRIBUTE_ALWAYS_INLINE
inline JITCompiledFunctionPtr JITContext::compile(
    Runtime *runtime,
    CodeBlock *codeBlock) {
  auto ptr = codeBlock->getJITCompiled();
  if (LLVM_LIKELY(ptr))
    return ptr;
  if (LLVM_LIKELY(!enabled_))
    return nullptr;
  if (LLVM_LIKELY(codeBlock->getDontJIT()))
    return nullptr;
  if (LLVM_LIKELY(codeBlock->getExecutionCount() < COMPILE_THRESHOLD))
    return nullptr;
  return compileImpl(runtime, codeBlock);
}

} // namespace x86_64
} // namespace vm
} // namespace hermes
#endif // HERMES_VM_JIT_X86_64_JIT_H
