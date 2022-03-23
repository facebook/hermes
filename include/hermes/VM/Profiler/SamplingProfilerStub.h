/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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
 public:
  explicit SamplingProfiler(Runtime &){};

  /// Mark roots that are kept alive by the SamplingProfiler.
  void markRoots(RootAcceptor &acceptor) {}

  /// Dump sampled stack to \p OS.
  /// NOTE: this is for manual testing purpose.
  static void dumpSampledStackGlobal(llvh::raw_ostream &OS) {}

  /// Dump sampled stack to \p OS in chrome trace format.
  static void dumpChromeTraceGlobal(llvh::raw_ostream &OS) {}

  /// Dump the sampled stack to \p OS in the format consumed by the DevTools
  /// JavaScript profiler.
  void serializeInDevToolsFormat(llvh::raw_ostream &OS) {}

  /// Enable and start profiling.
  static bool enable() {
    return false;
  }

  /// Disable and stop profiling.
  static bool disable() {
    return true;
  }

  /// Called for various GC events.
  void onGCEvent(GCEventKind kind, const std::string &extraInfo) {}
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_PROFILER_SAMPLINGPROFILERWINDOWS_H
