/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <vector>

namespace hermes {
namespace vm {

class Callable;

/// Singleton function code coverage profiler.
class CodeCoverageProfiler {
 public:
  /// Executed function info of a single runtime.
  struct FuncInfo {
    FuncInfo(uint32_t moduleId, uint32_t funcVirtualOffset)
        : moduleId(moduleId), funcVirtualOffset(funcVirtualOffset) {}

    /// Runtime module unique id.
    uint32_t moduleId;
    /// Executed function bytecode virtual offset.
    uint32_t funcVirtualOffset;
  };

  /// Return the singleton profiler instance.
  static std::shared_ptr<CodeCoverageProfiler> getInstance();

  /// \return enabled state for profiler.
  bool isEnabled() const {
    return enabled_;
  }

  /// Enable code coverage profiling.
  void enable() {
    enabled_ = true;
  }

  /// Disable code coverage profiling.
  void disable() {
    enabled_ = false;
  }

  /// Clear code coverage data for \p runtime.
  void clear(Runtime *runtime) {
    coverageInfo_.erase(runtime);
  }

  /// Mark roots that are kept alive by the CodeCoverageProfiler for \p runtime.
  void markRoots(Runtime *runtime, SlotAcceptorWithNames &acceptor);

  /// Record the executed JS function associated with \p codeBlock.
  inline void markExecuted(Runtime *runtime, CodeBlock *codeBlock) {
    if (LLVM_LIKELY(!enabled_)) {
      return;
    }
    markExecutedSlowPath(runtime, codeBlock);
  }

  void markExecutedSlowPath(Runtime *runtime, CodeBlock *codeBlock);

  /// \return executed function information.
  std::vector<CodeCoverageProfiler::FuncInfo> getExecutedFunctions();

 private:
  explicit CodeCoverageProfiler() = default;

  /// \return reference to function bits array map for \p module.
  std::vector<bool> &getModuleFuncMapRef(
      Runtime *runtime,
      RuntimeModule *module);

 private:
  struct RuntimeCodeCoverageInfo {
    /// RuntimeModule => executed function bits array map.
    /// Function bits array is a function id indexed bits array
    /// representing the executed state of all JS functions in single
    /// RuntimeModule.
    llvh::DenseMap<RuntimeModule *, std::vector<bool>> executedFuncBitsArrayMap;
    /// Domains to keep its RuntimeModules alive. Will be marked by markRoots().
    llvh::DenseSet<Domain *> domains;
  };

  /// Contain the code coverage info for each profiled runtime.
  llvh::DenseMap<Runtime *, RuntimeCodeCoverageInfo> coverageInfo_;

  /// Whether the profiler is enabled or not.
  bool enabled_{false};
};

bool operator==(
    const CodeCoverageProfiler::FuncInfo &left,
    const CodeCoverageProfiler::FuncInfo &right);

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_CODECOVERAGEPROFILER_H
