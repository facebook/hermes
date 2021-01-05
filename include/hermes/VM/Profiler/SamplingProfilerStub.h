/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_PROFILER_SAMPLINGPROFILERWINDOWS_H
#define HERMES_VM_PROFILER_SAMPLINGPROFILERWINDOWS_H

#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

/// No-op implementation of wall-time based JS sampling profiler.
class SamplingProfiler {
 private:
  SamplingProfiler() = default;

 public:
  /// Return the singleton profiler instance.
  static const std::shared_ptr<SamplingProfiler> &getInstance();

  /// Register an active \p runtime and current thread with profiler.
  /// Should only be called from the thread running hermes runtime.
  void registerRuntime(Runtime *runtime) {}

  /// Unregister an active \p runtime and current thread with profiler.
  void unregisterRuntime(Runtime *runtime) {}

  /// Mark roots that are kept alive by the SamplingProfiler.
  void markRoots(RootAcceptor &acceptor) {}

  /// Dump sampled stack to \p OS.
  /// NOTE: this is for manual testing purpose.
  void dumpSampledStack(llvh::raw_ostream &OS) {}

  /// Dump sampled stack to \p OS in chrome trace format.
  void dumpChromeTrace(llvh::raw_ostream &OS) {}

  /// Enable and start profiling.
  bool enable() {
    return false;
  }

  /// Disable and stop profiling.
  bool disable() {
    return true;
  }

  /// Called for various GC events.
  void
  onGCEvent(Runtime *runtime, GCEventKind kind, const std::string &extraInfo) {}
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_PROFILER_SAMPLINGPROFILERWINDOWS_H
