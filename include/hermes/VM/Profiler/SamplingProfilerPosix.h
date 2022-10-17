/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_PROFILER_SAMPLINGPROFILERPOSIX_H
#define HERMES_VM_PROFILER_SAMPLINGPROFILERPOSIX_H

#include "hermes/Support/Semaphore.h"
#include "hermes/Support/ThreadLocal.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/JSNativeFunctions.h"
#include "hermes/VM/Runtime.h"

#include "llvh/ADT/DenseMap.h"

#ifndef __APPLE__
// Prevent "The deprecated ucontext routines require _XOPEN_SOURCE to be
// defined" error on mac.
#include <ucontext.h>
#endif

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <vector>

#if defined(__ANDROID__) && defined(HERMES_FACEBOOK_BUILD)
#include <profilo/ExternalApi.h>
#endif

#if defined(__APPLE__) && defined(HERMES_FACEBOOK_BUILD)
#include <FBLoom/ExternalApi/ExternalApi.h>
#endif

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
      llvh::DenseMap<SamplingProfiler::ThreadId, std::string>;

  /// Captured JSFunction stack frame information for symbolication.
  /// TODO: consolidate the stack frame struct with other function/extern
  /// profilers.
  struct JSFunctionFrameInfo {
    // RuntimeModule this function is associated with.
    RuntimeModule *module;
    // Function id associated with current frame.
    uint32_t functionId;
    // IP offset within the function.
    uint32_t offset;
  };
  /// Captured NativeFunction frame information for symbolication.
  using LoomNativeFrameInfo = NativeFunctionPtr;
  /// Captured NativeFunction frame information for symbolication that hasn't
  /// been registered with the sampling profiler yet, and therefore can be moved
  /// by the GC.
  using NativeFunctionFrameInfo = size_t;
  /// GC frame info. Pointing to string in suspendEventExtraInfoSet_.
  using SuspendFrameInfo = const std::string *;

  // This will break with more than one RuntimeModule(like FB4a, eval() call or
  // lazy compilation etc...). It is simply a temporary thing to get started.
  // Will revisit after figuring out symbolication.
  struct StackFrame {
    /// Kind of frame.
    enum class FrameKind {
      JSFunction,
      NativeFunction,
      FinalizableNativeFunction,
      SuspendFrame,
    };

    // TODO: figure out how to store BoundFunction.
    // TODO: Should we do something special for NativeConstructor?
    union {
      /// Pure JS function frame info.
      JSFunctionFrameInfo jsFrame;
      /// Native function frame info storage used for loom profiling.
      LoomNativeFrameInfo nativeFunctionPtrForLoom;
      /// Native function frame info storage used for "regular" profiling.
      NativeFunctionFrameInfo nativeFrame;
      /// Suspend frame info. Pointing to string
      /// in suspendExtraInfoSet_; it is optional and
      /// can be null to indicate no extra info.
      /// We can't directly use std::string here because it is
      /// inside a union.
      SuspendFrameInfo suspendFrame;
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

    explicit StackTrace(uint32_t preallocatedSize) : stack(preallocatedSize) {}
    explicit StackTrace(
        ThreadId tid,
        TimeStampType ts,
        const std::vector<StackFrame>::iterator stackStart,
        const std::vector<StackFrame>::iterator stackEnd)
        : tid(tid), timeStamp(ts), stack(stackStart, stackEnd) {}
  };

#ifdef UNIT_TEST
  pthread_t getCurrentThread() const {
    return currentThread_;
  }
#endif // UNIT_TEST

  /// \returns the NativeFunctionPtr for \p stackFrame. Caller must hold
  /// runtimeDataLock_.
  NativeFunctionPtr getNativeFunctionPtr(const StackFrame &stackFrame) const {
    assert(
        (stackFrame.kind == StackFrame::FrameKind::NativeFunction ||
         stackFrame.kind == StackFrame::FrameKind::FinalizableNativeFunction) &&
        "unexpected stack frame kind");
    return nativeFunctions_[stackFrame.nativeFrame]->getFunctionPtr();
  }

  /// \returns the name (if one exists) for \p stackFrame. Caller must hold
  /// runtimeDataLock_.
  std::string getNativeFunctionName(const StackFrame &stackFrame) const {
    if (stackFrame.kind == StackFrame::FrameKind::NativeFunction) {
      // FrameKing::NativeFunction frames may be JS functions that are internal
      // to hermes -- e.g., Array.prototype.sort. For those functions, return
      // the native function name as defined in NativeFunctions.def.
      const char *name =
          hermes::vm::getFunctionName(getNativeFunctionPtr(stackFrame));
      if (strcmp(name, "")) {
        return name;
      }
    }

    // For all other FrameKind::NativeFunction frames, as well as
    // FrameKing::FinalizableNativeFunction ones, return the native function
    // name attribute, if one is available.
    return nativeFunctions_[stackFrame.nativeFrame]->getNameIfExists(runtime_);
  }

 private:
  /// Max size of sampleStorage_.
  static const int kMaxStackDepth = 500;

  struct GlobalProfiler {
    /// Pointing to the singleton SamplingProfiler instance.
    /// We need this field because accessing local static variable from
    /// signal handler is unsafe.
    static std::atomic<GlobalProfiler *> instance_;

    /// Used to synchronise data writes between the timer thread and the signal
    /// handler in the runtime thread. Also used to send the target
    /// SamplingProfiler to be used during the stack walk.
    static std::atomic<SamplingProfiler *> profilerForSig_;

    /// Lock for profiler operations and access to member fields.
    std::mutex profilerLock_;

    /// Profiler instances for all the individual runtimes that are currently
    /// registered.
    std::unordered_set<SamplingProfiler *> profilers_;

#if defined(__ANDROID__) && defined(HERMES_FACEBOOK_BUILD)
    /// Per-thread profiler instance for loom profiling.
    /// Limitations: No recursive runtimes in one thread.
    ThreadLocal<SamplingProfiler> threadLocalProfilerForLoom_;
#endif

    /// Whether profiler is enabled or not. Protected by profilerLock_.
    bool enabled_{false};
    /// Whether signal handler is registered or not. Protected by profilerLock_.
    bool isSigHandlerRegistered_{false};

#if defined(__APPLE__) && defined(HERMES_FACEBOOK_BUILD)
    /// Indicating whether or not we are in the middle of collecting stack for
    /// loom.
    bool collectingStack_{false};
#endif

    /// Semaphore to indicate all signal handlers have finished the sampling.
    Semaphore samplingDoneSem_;

    /// Threading: load/store of sampledStackDepth_ and sampleStorage_
    /// are protected by samplingDoneSem_.
    /// Actual sampled stack depth in sampleStorage_.
    uint32_t sampledStackDepth_{0};
    /// Preallocated stack frames storage for signal handler(because
    /// allocating memory in signal handler is not allowed)
    /// This storage does not need to be protected by lock because accessing to
    /// it is serialized by samplingDoneSem_.
    StackTrace sampleStorage_{kMaxStackDepth};

    /// This thread starts in timerLoop_, and samples the stacks of registered
    /// runtimes periodically. It is created in \p enable() and joined in
    /// \p disable().
    std::thread timerThread_;

    /// This condition variable can be used to wait for a change in the enabled
    /// member variable.
    std::condition_variable enabledCondVar_;

#if defined(__APPLE__) && defined(HERMES_FACEBOOK_BUILD)
    /// This condition variable is used when we disable profiler for loom
    /// collection, in the disableForLoomCollection() function.
    std::condition_variable disableForLoomCondVar_;
#endif

    /// invoke sigaction() posix API to register \p handler.
    /// \return what sigaction() returns: 0 to indicate success.
    static int invokeSignalAction(void (*handler)(int));

    /// Register sampling signal handler if not done yet.
    /// \return true to indicate success.
    bool registerSignalHandlers();

    /// Unregister sampling signal handler.
    bool unregisterSignalHandler();

    /// Signal handler to walk the stack frames.
    static void profilingSignalHandler(int signo);

    /// Main routine to take a sample of runtime stack.
    /// \return false for failure which timer loop thread should stop.
    bool sampleStacks();
    /// Sample stack for a profiler.
    bool sampleStack(SamplingProfiler *localProfiler);

    /// Timer loop thread main routine.
    void timerLoop();

    /// Implementation of SamplingProfiler::enable/disable.
    bool enable();
    bool disable();

#if defined(__APPLE__) && defined(HERMES_FACEBOOK_BUILD)
    /// Modified version of enable/disable, designed to be called by
    /// SamplingProfiler::collectStackForLoom.
    bool enableForLoomCollection();
    bool disableForLoomCollection();
#endif

    /// \return true if the sampling profiler is enabled, false otherwise.
    bool enabled();

    /// Register the \p profiler associated with an active runtime.
    /// Should only be called from the thread running hermes runtime.
    void registerRuntime(SamplingProfiler *profiler);

    /// Unregister the active runtime and current thread associated with
    /// \p profiler.
    void unregisterRuntime(SamplingProfiler *profiler);

    /// \return the singleton profiler instance.
    static GlobalProfiler *get();

   private:
    GlobalProfiler();
  };

  /// Protect data specific to a runtime, such as the sampled stacks and
  /// domains.
  std::mutex runtimeDataLock_;

  /// Sampled stack traces overtime. Protected by runtimeDataLock_.
  std::vector<StackTrace> sampledStacks_;

  // Threading: the suspendCount/preSuspendStack are accessed by both the VM
  // thread as well as the sampling profiler timer thread, hence they are all
  // protected by runtimeDataLock_.
  /// The counter of how many suspend calls are pending -- i.e., need to be
  /// resume()d.
  volatile uint32_t suspendCount_{0};
  /// The actual sampled stack depth in preSuspendStackStorage_.
  volatile uint32_t preSuspendStackDepth_{0};
  /// JS stack captured at time of GC.
  StackTrace preSuspendStackStorage_{kMaxStackDepth};

  /// Prellocated map that contains thread names mapping.
  ThreadNamesMap threadNames_;

  /// Thread that this profiler instance represents. This can currently only be
  /// set from the constructor of SamplingProfiler, so we need to construct a
  /// new SamplingProfiler every time the runtime is moved to a different
  /// thread.
  pthread_t currentThread_;

  /// Unique GC event extra info strings container.
  std::unordered_set<std::string> suspendEventExtraInfoSet_;

  /// Domains to be kept alive for sampled RuntimeModules. Protected by
  /// runtimeDataLock_.
  std::vector<Domain *> domains_;

  /// NativeFunctions to be kept alive for sampled NativeFunctionFrameInfo.
  /// Protected by runtimeDataLock_.
  std::vector<NativeFunction *> nativeFunctions_;

  Runtime &runtime_;

 private:
  /// Hold \p domain so that the RuntimeModule(s) used by profiler are not
  /// released during symbolication.
  /// Refer to Domain.h for relationship between Domain and RuntimeModule.
  void registerDomain(Domain *domain);

  /// Hold \p nativeFunction so native function names can be added to the stack
  /// traces.
  NativeFunctionFrameInfo registerNativeFunction(
      NativeFunction *nativeFunction);

  enum class InLoom { No, Yes };

  /// Walk runtime stack frames and store in \p sampleStorage.
  /// This function is called from signal handler so should obey all
  /// rules of signal handler(no lock, no memory allocation etc...)
  /// \param startIndex specifies the start index in \p sampleStorage to fill.
  /// \param inLoom specifies this function is being invoked in a Loom callback.
  /// \return total number of stack frames captured in \p sampleStorage
  /// including existing frames before \p startIndex.
  uint32_t walkRuntimeStack(
      StackTrace &sampleStorage,
      InLoom inLoom,
      uint32_t startIndex = 0);

  /// Record JS stack at time of suspension, caller must hold
  /// runtimeDataLock_.
  void recordPreSuspendStack(std::string_view extraInfo);

#if defined(__ANDROID__) && defined(HERMES_FACEBOOK_BUILD)
  /// Registered loom callback for collecting stack frames.
  static StackCollectionRetcode collectStackForLoom(
      ucontext_t *ucontext,
      int64_t *frames,
      uint16_t *depth,
      uint16_t max_depth);
#endif

#if defined(__APPLE__) && defined(HERMES_FACEBOOK_BUILD)
  /// Registered loom callback for collecting stack frames.
  static FBLoomStackCollectionRetcode collectStackForLoom(
      int64_t *frames,
      uint16_t *depth,
      uint16_t max_depth,
      void *profiler);
  /// Modified version of enable/disable, designed to be called by
  /// SamplingProfiler::collectStackForLoom.
  static bool enableForLoom();
  static bool disableForLoom();
#endif

  /// Clear previous stored samples.
  /// Note: caller should take the lock before calling.
  void clear();

 public:
  explicit SamplingProfiler(Runtime &runtime);
  ~SamplingProfiler();

  /// See documentation on \c GCBase::GCCallbacks.
  void markRootsForCompleteMarking(RootAcceptor &acceptor);

  /// Mark roots that are kept alive by the SamplingProfiler.
  void markRoots(RootAcceptor &acceptor);

  /// Dump sampled stack to \p OS.
  /// NOTE: this is for manual testing purpose.
  void dumpSampledStack(llvh::raw_ostream &OS);

  /// Dump sampled stack to \p OS in chrome trace format.
  void dumpChromeTrace(llvh::raw_ostream &OS);

  /// Dump the sampled stack to \p OS in the format expected by the
  /// Profiler.stop return type. See
  ///
  /// https://chromedevtools.github.io/devtools-protocol/tot/Profiler/#type-Profile
  ///
  /// for a description.
  void serializeInDevToolsFormat(llvh::raw_ostream &OS);

  /// Static wrapper for dumpSampledStack.
  static void dumpSampledStackGlobal(llvh::raw_ostream &OS);

  /// Static wrapper for dumpChromeTrace.
  static void dumpChromeTraceGlobal(llvh::raw_ostream &OS);

  /// Enable and start profiling.
  static bool enable();

  /// Disable and stop profiling.
  static bool disable();

  /// Suspends the sample profiling. Every call to suspend must be matched by a
  /// call to resume.
  void suspend(std::string_view reason);

  /// Resumes the sample profiling. There must have been a previous call to
  /// suspend() that hansn't been resume()d yet.
  void resume();

#if (defined(__ANDROID__) || defined(__APPLE__)) && \
    defined(HERMES_FACEBOOK_BUILD)
  // Common code that is shared by the collectStackForLoom(), for both the
  // Android and Apple versions.
  void collectStackForLoomCommon(
      const StackFrame &frame,
      int64_t *frames,
      uint32_t index);
#endif
};

/// An RAII class for temporarily suspending (and auto-resuming) the sampling
/// profiler.
class SuspendSamplingProfilerRAII {
 public:
  explicit SuspendSamplingProfilerRAII(
      Runtime &runtime,
      std::string_view reason = "")
      : profiler_(runtime.samplingProfiler.get()) {
    if (profiler_) {
      profiler_->suspend(reason);
    }
  }

  ~SuspendSamplingProfilerRAII() {
    if (profiler_) {
      profiler_->resume();
    }
  }

 private:
  SamplingProfiler *profiler_;
};

bool operator==(
    const SamplingProfiler::StackFrame &left,
    const SamplingProfiler::StackFrame &right);

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_PROFILER_SAMPLINGPROFILERPOSIX_H
