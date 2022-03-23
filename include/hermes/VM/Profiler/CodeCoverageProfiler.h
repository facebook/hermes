/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_CODECOVERAGEPROFILER_H
#define HERMES_VM_CODECOVERAGEPROFILER_H

#include "hermes/VM/Domain.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/RuntimeModule-inline.h"

#include "llvh/ADT/DenseMap.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hermes {
namespace vm {

/// Function code coverage profiler.
class CodeCoverageProfiler {
 public:
  /// Executed function info.
  struct FuncInfo {
    FuncInfo(
        uint32_t moduleId,
        uint32_t funcVirtualOffset,
        std::string debugInfo)
        : moduleId(moduleId),
          funcVirtualOffset(funcVirtualOffset),
          debugInfo(debugInfo) {}

    /// Runtime module unique id.
    uint32_t moduleId;
    /// Executed function bytecode virtual offset.
    uint32_t funcVirtualOffset;

    std::string debugInfo;
  };

  explicit CodeCoverageProfiler(Runtime &runtime) : runtime_(runtime) {
    std::lock_guard<std::mutex> lk(globalMutex());
    allProfilers().insert(this);
  }

  ~CodeCoverageProfiler() {
    std::lock_guard<std::mutex> lk(globalMutex());
    allProfilers().erase(this);
  }

  /// \return executed function information indexed per runtime.
  static std::
      unordered_map<std::string, std::vector<CodeCoverageProfiler::FuncInfo>>
      getExecutedFunctions();

  /// globally enable the code coverage profiler.
  static void enableGlobal() {
    globalEnabledFlag().store(true, std::memory_order_relaxed);
  }

  /// globally disable the code coverage profiler.
  static void disableGlobal() {
    globalEnabledFlag().store(false, std::memory_order_relaxed);
  }

  /// \return global enabled state for profiler.
  static bool globallyEnabled() {
    return globalEnabledFlag().load(std::memory_order_relaxed);
  }

  /// \return enabled state for profiler.
  bool isEnabled() const {
    return !localDisabled_ && globallyEnabled();
  }

  /// Restore this runtime's profiler state to the global state.
  void restore() {
    localDisabled_ = false;
  }

  /// Override global profiler state to disable code coverage profiling for this
  /// runtime.
  void disable() {
    localDisabled_ = true;
  }

  /// Mark roots that are kept alive by the CodeCoverageProfiler for \p runtime.
  void markRoots(RootAcceptor &acceptor);

  /// Record the executed JS function associated with \p codeBlock.
  inline void markExecuted(CodeBlock *codeBlock) {
    if (LLVM_LIKELY(!isEnabled())) {
      return;
    }
    markExecutedSlowPath(codeBlock);
  }

  /// \return executed function information for this profiler.
  std::vector<CodeCoverageProfiler::FuncInfo> getExecutedFunctionsLocal();

 private:
  static std::unordered_set<CodeCoverageProfiler *> &allProfilers();
  static std::mutex &globalMutex();
  static std::atomic<bool> &globalEnabledFlag() {
    static std::atomic<bool> globalEnabledFlag;
    return globalEnabledFlag;
  }

  void markExecutedSlowPath(CodeBlock *codeBlock);

  /// \pre localMutex_ must be held.
  /// \return reference to function bits array map for \p module.
  std::vector<bool> &getModuleFuncMapRef(RuntimeModule *module);

  Runtime &runtime_;

  /// Protect any local state of this code coverage profiler that can be
  /// accessed by the static members. For now, this is only used to protect
  /// executedFuncBitsArrayMap_.
  std::mutex localMutex_;

  /// RuntimeModule => executed function bits array map.
  /// Function bits array is a function id indexed bits array
  /// representing the executed state of all JS functions in single
  /// RuntimeModule.
  llvh::DenseMap<RuntimeModule *, std::vector<bool>> executedFuncBitsArrayMap_;

  /// Domains to keep its RuntimeModules alive. Will be marked by markRoots().
  /// Does not require localMutex_ to be held since it is only modified and read
  /// by the runtime.
  llvh::DenseSet<Domain *> domains_;
  /// Override the global enable for certain sections (such as when running
  /// internal bytecode).
  /// Does not require localMutex_ to be held since it is only modified and read
  /// by the runtime.
  bool localDisabled_{false};
};

bool operator==(
    const CodeCoverageProfiler::FuncInfo &left,
    const CodeCoverageProfiler::FuncInfo &right);

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_CODECOVERAGEPROFILER_H
