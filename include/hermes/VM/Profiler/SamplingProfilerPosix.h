/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_PROFILER_SAMPLINGPROFILERPOSIX_H
#define HERMES_VM_PROFILER_SAMPLINGPROFILERPOSIX_H

#include "hermes/Support/Semaphore.h"
#include "hermes/Support/ThreadLocal.h"
#include "hermes/VM/Profiler/ModuleIdManager.h"
#include "hermes/VM/Runtime.h"

#include "llvm/ADT/DenseMap.h"

#ifndef __APPLE__
// Prevent "The deprecated ucontext routines require _XOPEN_SOURCE to be
// defined" error on mac.
#include <ucontext.h>
#endif

#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

namespace hermes {
namespace vm {

/// Singleton wall-time based JS sampling profiler that walks VM stack frames
/// in a configurable interval. The profiler can be enabled and disabled
/// on demand.
class SamplingProfiler {
 public:
  using ThreadId = uint64_t;
  using TimeStampType = std::chrono::steady_clock::time_point;
  using ThreadNamesMap =
      llvm::DenseMap<SamplingProfiler::ThreadId, std::string>;

  /// Captured JSFunction stack frame information for symbolication.
  // TODO: consolidate the stack frame struct with other function/extern
  // profilers.
  struct JSFunctionFrameInfo {
    // Id of the module this function is associated with.
    ModuleIdManager::ModuleId runtimeModuleId;
    // Function id associated with current frame.
    uint32_t functionId;
    // IP offset within the function.
    uint32_t offset;
  };
  /// Captured NativeFunction frame information for symbolication.
  using NativeFunctionFrameInfo = uintptr_t;

  // This will break with more than one RuntimeModule(like FB4a, eval() call or
  // lazy compilation etc...). It is simply a temporary thing to get started.
  // Will revisit after figuring out symbolication.
  struct StackFrame {
    /// Kind of frame.
    enum class FrameKind {
      JSFunction,
      NativeFunction,
    };

    // TODO: figure out how to store BoundFunction.
    // TODO: Should we do something special for NativeConstructor?
    union {
      // Pure JS function frame info.
      JSFunctionFrameInfo jsFrame;
      // Native function frame info.
      NativeFunctionFrameInfo nativeFrame;
    };
    FrameKind kind;
  };

  /// Represent stack trace captured by one sampling.
  struct StackTrace {
    /// Id of the thread that this stack trace is taken from.
    ThreadId tid{0};
    /// Timestamp when the stack trace is taken.
    TimeStampType timeStamp;
    /// Captured stack frames.
    std::vector<StackFrame> stack;

    StackTrace(uint32_t preallocatedSize) : stack(preallocatedSize) {}
    StackTrace(
        ThreadId tid,
        TimeStampType ts,
        const std::vector<StackFrame>::iterator stackStart,
        const std::vector<StackFrame>::iterator stackEnd)
        : tid(tid), timeStamp(ts), stack(stackStart, stackEnd) {}
  };

 private:
  /// Max size of sampleStorage_.
  static const int kMaxStackDepth = 500;

  /// Pointing to the singleton SamplingProfiler instance.
  /// We need this field because accessing local static variable from
  /// signal handler is unsafe.
  static volatile std::atomic<SamplingProfiler *> sProfilerInstance_;

  /// Lock for profiler operations.
  std::mutex profilerLock_;
  /// Stores a list of active <thread, runtime> pair.
  /// Protected by profilerLock_.
  llvm::DenseMap<Runtime *, pthread_t> activeRuntimeThreads_;

  /// Last registered runtime.
  /// NOTE: currently we only support sampling one thread/runtime so
  /// use a single static variable to store the last registered runtime.
  /// It has the following limitations:
  /// 1. No multiple threads sampling.
  /// 2. No recursive runtime in one thread sampling.
  /// Will revise these limitations in later diffs.
  static std::atomic<Runtime *> targetRuntime_;

  /// Per-thread runtime instance for loom profiling.
  /// Note: it is different from targetRuntime_ which is used by local
  /// sample profiling.
  ThreadLocal<Runtime> threadLocalRuntime_;

  /// Whether profiler is enabled or not. Protected by profilerLock_.
  bool enabled_{false};
  /// Whether signal handler is registered or not. Protected by profilerLock_.
  bool isSigHandlerRegistered_{false};

  /// Semaphore to indicate all signal handlers have finished the sampling.
  Semaphore samplingDoneSem_;

  /// Sampled stack traces overtime. Protected by profilerLock_.
  std::vector<StackTrace> sampledStacks_;

  /// Threading: load/store of sampledStackDepth_ and sampleStorage_
  /// are protected by samplingDoneSem_.
  /// Actual sampled stack depth in sampleStorage_.
  uint32_t sampledStackDepth_{0};
  /// Preallocated stack frames storage for signal handler(because
  /// allocating memory in signal handler is not allowed)
  /// This storage does not need to be protected by lock because accessing to
  /// it is serialized by samplingDoneSem_.
  StackTrace sampleStorage_{kMaxStackDepth};

  /// Prellocated map that contains thread names mapping.
  ThreadNamesMap threadNames_;

  /// Manages the module id for symbolication.
  ModuleIdManager moduleIdManager_;

 private:
  SamplingProfiler();

  /// invoke sigaction() posix API to register \p handler.
  /// \return what sigaction() returns: 0 to indicate success.
  int invokeSignalAction(void (*handler)(int));

  /// Register sampling signal handler if not done yet.
  /// \return true to indicate success.
  bool registerSignalHandlers();

  /// Unregister sampling signal handler.
  bool unregisterSignalHandler();

  /// Signal handler to walk the stack frames.
  static void profilingSignalHandler(int signo);

  /// Main routine to take a sample of runtime stack.
  /// \return false for failure which timer loop thread should stop.
  bool sampleStack();

  /// Timer loop thread main routine.
  void timerLoop();

  /// Walk runtime stack frames and store in \p sampleStorage.
  /// This function is called from signal handler so should obey all
  /// rules of signal handler(no lock, no memory allocation etc...)
  uint32_t walkRuntimeStack(const Runtime *runtime, StackTrace &sampleStorage);

  /// Registered loom callback for collecting stack frames.
  static bool collectStackForLoom(
      ucontext_t *ucontext,
      int64_t *frames,
      uint8_t *depth,
      uint8_t max_depth);

  /// Clear previous stored samples.
  /// Note: caller should take the lock before calling.
  void clear() {
    sampledStacks_.clear();
    moduleIdManager_.clear();
    // TODO: keep thread names that are still in use.
    threadNames_.clear();
  }

 public:
  /// Return the singleton profiler instance.
  static const std::shared_ptr<SamplingProfiler> &getInstance();

  /// Register an active \p runtime and current thread with profiler.
  /// Should only be called from the thread running hermes runtime.
  void registerRuntime(Runtime *runtime);

  /// Unregister an active \p runtime and current thread with profiler.
  void unregisterRuntime(Runtime *runtime);

  /// Dump sampled stack to \p OS.
  /// NOTE: this is for manual testing purpose.
  void dumpSampledStack(llvm::raw_ostream &OS);

  /// Dump sampled stack to \p OS in chrome trace format.
  void dumpChromeTrace(llvm::raw_ostream &OS);

  /// Enable and start profiling.
  bool enable();

  /// Disable and stop profiling.
  bool disable();
};

bool operator==(
    const SamplingProfiler::StackFrame &left,
    const SamplingProfiler::StackFrame &right);

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_PROFILER_SAMPLINGPROFILERPOSIX_H
