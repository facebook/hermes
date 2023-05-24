/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SamplingProfilerSampler.h"

#if defined(HERMESVM_SAMPLING_PROFILER_POSIX)

#include "hermes/Support/Semaphore.h"
#include "hermes/Support/ThreadLocal.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/HostModel.h"
#include "hermes/VM/Profiler/SamplingProfiler.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/RuntimeModule-inline.h"
#include "hermes/VM/StackFrame-inline.h"

#include "llvh/Support/Compiler.h"

#include "ChromeTraceSerializer.h"

// Determine if LOOM allowed as well as if it is supported by the target
// platform. From this point on, the code should use
//
// HERMESVM_ENABLE_LOOM (for generic loom code)
//
// and platform specific checks where needed.

#if defined(HERMES_FACEBOOK_BUILD) && defined(HERMESVM_ALLOW_LOOM) && \
    (defined(__APPLE__) || defined(__ANDROID__))
#define HERMESVM_ENABLE_LOOM
#endif

#if defined(HERMESVM_ENABLE_LOOM) && defined(__ANDROID__)
// Prevent "The deprecated ucontext routines require _XOPEN_SOURCE to be
// defined" error on mac.
#include <ucontext.h>
#endif

#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <cassert>
#include <chrono>
#include <cmath>
#include <csignal>
#include <random>
#include <thread>

#if defined(HERMESVM_ENABLE_LOOM) && defined(__ANDROID__)
#include <profilo/ExternalApi.h>

#elif defined(HERMESVM_ENABLE_LOOM) && defined(__APPLE__)
#include <FBLoom/ExternalApi/ExternalApi.h>

#endif

namespace hermes {
namespace vm {
namespace sampling_profiler {
namespace {

/// Name of the semaphore.
constexpr char kSamplingDoneSemaphoreName[] = "/samplingDoneSem";

struct SamplingProfilerPosix : SamplingProfiler {
  SamplingProfilerPosix(Runtime &rt);
  ~SamplingProfilerPosix() override;

  /// Thread that this profiler instance represents. This can currently only be
  /// set from the constructor of SamplingProfiler, so we need to construct a
  /// new SamplingProfiler every time the runtime is moved to a different
  /// thread.
  pthread_t currentThread_;

#if defined(HERMESVM_ENABLE_LOOM) && defined(__ANDROID__)
  /// Registered loom callback for collecting stack frames.
  static StackCollectionRetcode collectStackForLoom(
      ucontext_t *ucontext,
      int64_t *frames,
      uint16_t *depth,
      uint16_t max_depth);

#elif defined(HERMESVM_ENABLE_LOOM) && defined(__APPLE__)
  /// Loom in Apple platforms is a "push" interface -- meaning the client code
  /// (in this case, the sampling profiler) will invoke an API when a new stack
  /// trace is available. This member controls whether Loom is enabled for the
  /// current this SamplingProfilerPosix.
  bool loomDataPushEnabled_{false};

  /// Previous timestamp when a push to loom occurred. The Loom API does not
  /// rate limit its callers, and thus care must be taken to not overload it.
  std::chrono::time_point<std::chrono::system_clock> previousPushTs;

  /// \return true if a new stack trace should be pushed to Loom.
  bool shouldPushDataToLoom() const;

  /// Converts the last sampled stack trace and push it to loom.
  void pushLastSampledStackToLoom();

#endif

#if defined(HERMESVM_ENABLE_LOOM)
  // Common code that is shared by the collectStackForLoom(), for both the
  // Android and Apple versions.
  void collectStackForLoomCommon(
      const StackFrame &frame,
      int64_t *frames,
      uint32_t index);
#endif // defined(HERMESVM_ENABLE_LOOM)
};

struct SamplerPosix : Sampler {
  SamplerPosix();
  ~SamplerPosix() override;
  /// Pointing to the singleton SamplingProfiler instance.
  /// We need this field because accessing local static variable from
  /// signal handler is unsafe.
  static std::atomic<SamplerPosix *> instance_;

  /// Used to synchronise data writes between the timer thread and the signal
  /// handler in the runtime thread. Also used to send the target
  /// SamplingProfiler to be used during the stack walk.
  static std::atomic<SamplingProfiler *> profilerForSig_;

#if defined(HERMESVM_ENABLE_LOOM) && defined(__ANDROID__)
  /// Per-thread profiler instance for loom profiling.
  /// Limitations: No recursive runtimes in one thread.
  ThreadLocal<SamplingProfilerPosix> threadLocalProfilerForLoom_;
#endif

  /// Whether signal handler is registered or not. Protected by profilerLock_.
  bool isSigHandlerRegistered_{false};

  /// Semaphore to indicate all signal handlers have finished the sampling.
  Semaphore samplingDoneSem_;

  /// Register sampling signal handler if not done yet.
  /// \return true to indicate success.
  bool registerSignalHandlers();

  /// Unregister sampling signal handler.
  bool unregisterSignalHandler();

  /// Signal handler to walk the stack frames.
  static void profilingSignalHandler(int signo);
};
} // namespace

SamplingProfilerPosix::SamplingProfilerPosix(Runtime &rt)
    : SamplingProfiler(rt), currentThread_{pthread_self()} {
#if defined(HERMESVM_ENABLE_LOOM) && defined(__APPLE__)
  fbloom_profilo_api()->fbloom_register_enable_for_loom_callback(
      FBLoomTracerType::JAVASCRIPT, enable);
  fbloom_profilo_api()->fbloom_register_disable_for_loom_callback(
      FBLoomTracerType::JAVASCRIPT, disable);
  loomDataPushEnabled_ = true;
#endif
}

SamplingProfilerPosix::~SamplingProfilerPosix() {
  // TODO(T125910634): re-introduce the requirement for destroying the sampling
  // profiler on the same thread in which it was created.
  Sampler::get()->unregisterRuntime(this);
}

std::atomic<SamplerPosix *> SamplerPosix::instance_{nullptr};

std::atomic<SamplingProfiler *> SamplerPosix::profilerForSig_{nullptr};

namespace {
/// invoke sigaction() posix API to register \p handler.
/// \return what sigaction() returns: 0 to indicate success.
static int invokeSignalAction(void (*handler)(int)) {
  struct sigaction actions;
  memset(&actions, 0, sizeof(actions));
  sigemptyset(&actions.sa_mask);
  // Allows interrupted IO primitives to restart.
  actions.sa_flags = SA_RESTART;
  actions.sa_handler = handler;
  return sigaction(SIGPROF, &actions, nullptr);
}
} // namespace

bool SamplerPosix::registerSignalHandlers() {
  if (isSigHandlerRegistered_) {
    return true;
  }
  if (invokeSignalAction(profilingSignalHandler) != 0) {
    perror("signal handler registration failed");
    return false;
  }
  isSigHandlerRegistered_ = true;
  return true;
}

bool SamplerPosix::unregisterSignalHandler() {
  if (!isSigHandlerRegistered_) {
    return true;
  }
  // Restore to default.
  if (invokeSignalAction(SIG_DFL) != 0) {
    perror("signal handler unregistration failed");
    return false;
  }
  isSigHandlerRegistered_ = false;
  return true;
}

void SamplerPosix::profilingSignalHandler(int signo) {
  // Ensure that writes made on the timer thread before setting the current
  // profiler are correctly acquired.
  SamplingProfiler *localProfiler;
  while (!(localProfiler = profilerForSig_.load(std::memory_order_acquire))) {
  }

  // Avoid spoiling errno in a signal handler by storing the old version and
  // re-assigning it.
  auto oldErrno = errno;

  auto *profilerInstance = static_cast<SamplerPosix *>(instance_.load());
  assert(
      profilerInstance != nullptr &&
      "Why is SamplerPosix::instance_ not initialized yet?");

  profilerInstance->walkRuntimeStack(localProfiler);

  // Ensure that writes made in the handler are visible to the timer thread.
  profilerForSig_.store(nullptr);

  if (!profilerInstance->samplingDoneSem_.notifyOne()) {
    errno = oldErrno;
    abort(); // Something is wrong.
  }
  errno = oldErrno;
}

/*static*/ Sampler *Sampler::get() {
  // We intentionally leak this memory to avoid a case where instance is
  // accessed after it is destroyed during shutdown.
  static SamplerPosix *instance = new SamplerPosix{};
  return instance;
}

SamplerPosix::~SamplerPosix() = default;

SamplerPosix::SamplerPosix() {
  instance_.store(this);
#if defined(HERMESVM_ENABLE_LOOM) && defined(__ANDROID__)
  profilo_api()->register_external_tracer_callback(
      TRACER_TYPE_JAVASCRIPT, SamplingProfilerPosix::collectStackForLoom);
#endif
}

bool Sampler::platformEnable() {
  auto *self = static_cast<SamplerPosix *>(this);
  if (!self->samplingDoneSem_.open(kSamplingDoneSemaphoreName)) {
    return false;
  }
  if (!self->registerSignalHandlers()) {
    return false;
  }

  return true;
}

bool Sampler::platformDisable() {
  auto *self = static_cast<SamplerPosix *>(this);
  if (!self->samplingDoneSem_.close()) {
    return false;
  }
  // Unregister handlers before shutdown.
  if (!self->unregisterSignalHandler()) {
    return false;
  }

  return true;
}

void Sampler::platformRegisterRuntime(SamplingProfiler *profiler) {
#if defined(HERMESVM_ENABLE_LOOM) && defined(__ANDROID__)
  auto *self = static_cast<SamplerPosix *>(this);
  assert(
      self->threadLocalProfilerForLoom_.get() == nullptr &&
      "multiple hermes runtime in the same thread");
  self->threadLocalProfilerForLoom_.set(
      static_cast<SamplingProfilerPosix *>(profiler));
#endif
}

void Sampler::platformUnregisterRuntime(SamplingProfiler *profiler) {
#if defined(HERMESVM_ENABLE_LOOM) && defined(__ANDROID__)
  auto *self = static_cast<SamplerPosix *>(this);
  // TODO(T125910634): re-introduce the requirement for unregistering the
  // runtime in the same thread it was registered.
  self->threadLocalProfilerForLoom_.set(nullptr);
#endif
}

void Sampler::platformPostSampleStack(SamplingProfiler *localProfiler) {
#if defined(HERMESVM_ENABLE_LOOM) && defined(__APPLE__)
  auto *posixProfiler = static_cast<SamplingProfilerPosix *>(localProfiler);
  if (posixProfiler->shouldPushDataToLoom()) {
    posixProfiler->pushLastSampledStackToLoom();
  }
#endif // defined(HERMESVM_ENABLE_LOOM_APPLE)
}

bool Sampler::platformSuspendVMAndWalkStack(SamplingProfiler *profiler) {
  auto *self = static_cast<SamplerPosix *>(this);
  auto *posixProfiler = static_cast<SamplingProfilerPosix *>(profiler);
  // Guarantee that the runtime thread will not proceed until it has
  // acquired the updates to domains_.
  self->profilerForSig_.store(profiler, std::memory_order_release);

  // Signal target runtime thread to sample stack.
  pthread_kill(posixProfiler->currentThread_, SIGPROF);

  // Threading: samplingDoneSem_ will synchronise this thread with the
  // signal handler, so that we only have one active signal at a time.
  if (!self->samplingDoneSem_.wait()) {
    return false;
  }

  // Guarantee that this thread will observe all changes made to data
  // structures in the signal handler.
  while (self->profilerForSig_.load(std::memory_order_acquire) != nullptr) {
  }

  return true;
}

#if defined(HERMESVM_ENABLE_LOOM)
void SamplingProfilerPosix::collectStackForLoomCommon(
    const StackFrame &frame,
    int64_t *frames,
    uint32_t index) {
  constexpr uint64_t kNativeFrameMask = ((uint64_t)1 << 63);
  switch (frame.kind) {
    case StackFrame::FrameKind::JSFunction: {
      auto *bcProvider = frame.jsFrame.module->getBytecode();
      uint32_t virtualOffset =
          bcProvider->getVirtualOffsetForFunction(frame.jsFrame.functionId) +
          frame.jsFrame.offset;
      uint32_t segmentID = bcProvider->getSegmentID();
      uint64_t frameAddress = ((uint64_t)segmentID << 32) + virtualOffset;
      assert(
          (frameAddress & kNativeFrameMask) == 0 &&
          "Module id should take less than 32 bits");
      frames[(index)] = static_cast<int64_t>(frameAddress);
      break;
    }

    case StackFrame::FrameKind::NativeFunction:
    case StackFrame::FrameKind::FinalizableNativeFunction: {
#if defined(HERMESVM_ENABLE_LOOM) && defined(__ANDROID__)
      NativeFunctionPtr nativeFrame = frame.nativeFunctionPtrForLoom;
#elif defined(HERMESVM_ENABLE_LOOM) && defined(__APPLE__)
      NativeFunctionPtr nativeFrame = getNativeFunctionPtr(frame);
#else
      // Intentionally left empty to catch cases where
      // HERMESVM_ENABLE_LOOM is defined but the underlying platform is
      // not handled.
#endif
      frames[(index)] = ((uint64_t)nativeFrame | kNativeFrameMask);
      break;
    }

#if defined(HERMESVM_ENABLE_LOOM) && defined(__APPLE__)
    case StackFrame::FrameKind::SuspendFrame:
      break;
#endif

    default:
      llvm_unreachable("Loom: unknown frame kind");
  }
}
#endif

#if defined(HERMESVM_ENABLE_LOOM) && defined(__ANDROID__)
/*static*/ StackCollectionRetcode SamplingProfilerPosix::collectStackForLoom(
    ucontext_t *ucontext,
    int64_t *frames,
    uint16_t *depth,
    uint16_t max_depth) {
  auto profilerInstance = SamplerPosix::instance_.load();
  SamplingProfilerPosix *localProfiler =
      profilerInstance->threadLocalProfilerForLoom_.get();
  if (localProfiler == nullptr) {
    // No runtime in this thread.
    return StackCollectionRetcode::NO_STACK_FOR_THREAD;
  }

  uint32_t sampledStackDepth = 0;
  // Sampling stack will touch GC objects(like closure) so
  // only do so if heap is valid.
  if (LLVM_LIKELY(!localProfiler->isSuspended())) {
    Runtime &curThreadRuntime = localProfiler->runtime_;
    assert(
        !curThreadRuntime.getHeap().inGC() &&
        "sampling profiler should be suspended before GC");
    (void)curThreadRuntime;
    assert(
        profilerInstance != nullptr &&
        "Why is Sampler::instance_ not initialized yet?");
    // Do not register domains for Loom profiling, since we don't use them for
    // symbolication.
    sampledStackDepth = localProfiler->walkRuntimeStack(
        profilerInstance->sampleStorage_, InLoom::Yes);
  } else {
    // TODO: log "GC in process" meta event.
    sampledStackDepth = 0;
  }

  // Loom stack frames are encoded like this:
  //   1. Most significant bit of 64bits address is set for native frame.
  //   2. JS frame: module id in high 32 bits, address virtual offset in lower
  //   32 bits, with MSB unset.
  //   3. Native/Finalizable frame: address returned with MSB set.
  // TODO: enhance this when supporting more frame types.
  sampledStackDepth = std::min(sampledStackDepth, (uint32_t)max_depth);
  for (uint32_t i = 0; i < sampledStackDepth; ++i) {
    const StackFrame &stackFrame = profilerInstance->sampleStorage_.stack[i];
    localProfiler->collectStackForLoomCommon(stackFrame, frames, i);
  }
  *depth = sampledStackDepth;
  if (*depth == 0) {
    return StackCollectionRetcode::EMPTY_STACK;
  }
  return StackCollectionRetcode::SUCCESS;
}
#endif

#if defined(HERMESVM_ENABLE_LOOM) && defined(__APPLE__)
bool SamplingProfilerPosix::shouldPushDataToLoom() const {
  auto now = std::chrono::system_clock::now();
  constexpr auto kLoomDelay = std::chrono::milliseconds(50);
  // The default sample stack interval in timerLoop() is between 5-15ms which
  // is too often for loom.
  return loomDataPushEnabled_ && (now - previousPushTs > kLoomDelay);
}

void SamplingProfilerPosix::pushLastSampledStackToLoom() {
  constexpr uint16_t maxDepth = 512;
  int64_t frames[maxDepth];
  uint16_t depth = 0;
  // Each element in sampledStacks_ is one call stack, access the last one
  // to get the latest stack trace.
  auto sample = sampledStacks_.back();
  for (auto iter = sample.stack.rbegin(); iter != sample.stack.rend(); ++iter) {
    const StackFrame &frame = *iter;
    collectStackForLoomCommon(frame, frames, depth);
    depth++;
    if (depth > maxDepth) {
      return;
    }
  }
  fbloom_profilo_api()->fbloom_write_stack_to_loom(
      FBLoomTracerType::JAVASCRIPT, frames, depth);
  previousPushTs = std::chrono::system_clock::now();
  clear();
}
#endif
} // namespace sampling_profiler

std::unique_ptr<SamplingProfiler> SamplingProfiler::create(Runtime &rt) {
  return std::make_unique<sampling_profiler::SamplingProfilerPosix>(rt);
}

bool SamplingProfiler::belongsToCurrentThread() const {
  return static_cast<const sampling_profiler::SamplingProfilerPosix *>(this)
             ->currentThread_ == pthread_self();
}

} // namespace vm
} // namespace hermes

#endif // defined(HERMESVM_SAMPLING_PROFILER_POSIX)
