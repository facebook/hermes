/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JIT_ARM64_JIT_H
#define HERMES_VM_JIT_ARM64_JIT_H

#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/JIT/PerfJitDump.h"

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

  /// Set the default threshold for function execution count before a function
  /// is compiled. On a per-function basis, the count may be altered based on
  /// internal heuristics.
  /// Can be overridden by setForceJIT(true).
  void setDefaultExecThreshold(uint32_t threshold) {
    defaultExecThreshold_ = threshold;
  }

  /// Enable or disable dumping JIT'ed Code.
  void setDumpJITCode(unsigned dump) {
    dumpJITCode_ = dump;
  }

  /// \return true if dumping JIT'ed Code is enabled.
  unsigned getDumpJITCode() {
    return dumpJITCode_;
  }

  /// Construct data structure used for perf profiling support. This should be
  /// called only when PerfProf is enabled and perf JITContext.
  /// \param jitdumpFd The file descriptor of the opened jitdump file.
  /// \param commentFd The file descriptor of the opended \p commentFile.
  /// \param commentFile The path of the file to store the comments.
  void initPerfProfData(
      int jitdumpFd,
      int commentFd,
      const std::string &commentFile) {
    assert(
        !perfJitDump_ &&
        "perfJitDump_ should be constructed once per JITContext");
    perfJitDump_ =
        std::make_unique<PerfJitDump>(jitdumpFd, commentFd, commentFile);
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
    forceJIT_ = force;
  }

  /// Set the memory limit for JIT'ed code in bytes.
  void setMemoryLimit(uint32_t memoryLimit) {
    memoryLimit_ = memoryLimit;
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
  /// The memory limit for JIT'ed code in bytes.
  /// Once the limit is reached, no more code will be JIT'ed.
  uint32_t memoryLimit_{32u << 20};
  /// whether to dump JIT'ed code
  unsigned dumpJITCode_{0};
  /// whether to fatally crash on JIT compilation errors
  bool crashOnError_{false};
  /// Whether to emit asserts in the JIT'ed code.
  bool emitAsserts_{false};
  /// Whether to force jitting of all functions.
  /// If true, ignores the default exec threshold completely.
  bool forceJIT_{false};

  /// Generate jitdump for all jitted functions.
  std::unique_ptr<PerfJitDump> perfJitDump_{};

  /// The JIT threshold for function execution count.
  /// Lowered based on the loop depth before deciding whether to JIT.
  uint32_t defaultExecThreshold_ = 1 << 5;
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

  uint32_t loopDepth = codeBlock->getFunctionHeader().getLoopDepth();
  // It's possible that if the loop depth is too high, we will set the
  // execThreshold to 0 for this function, but that's OK because we want to JIT
  // it immediately.
  assert(loopDepth <= 3 && "loopDepth is larger than expected");
  uint32_t execThreshold =
      forceJIT_ ? 0 : (defaultExecThreshold_ >> (loopDepth * 2));

  if (LLVM_LIKELY(codeBlock->getExecutionCount() < execThreshold))
    return nullptr;

  return compileImpl(runtime, codeBlock);
}

} // namespace arm64
} // namespace vm
} // namespace hermes
#endif // HERMES_VM_JIT_ARM64_JIT_H
