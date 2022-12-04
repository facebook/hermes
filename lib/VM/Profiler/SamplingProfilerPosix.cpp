/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "GlobalProfiler.h"

#if defined(HERMESVM_SAMPLING_PROFILER_POSIX)

#include "hermes/Support/ThreadLocal.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/HostModel.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/RuntimeModule-inline.h"
#include "hermes/VM/StackFrame-inline.h"

#include "llvh/Support/Compiler.h"

#include "ChromeTraceSerializer.h"

#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <cassert>
#include <chrono>
#include <cmath>
#include <csignal>
#include <random>
#include <thread>

namespace hermes {
namespace vm {

std::atomic<GlobalProfiler *> GlobalProfiler::instance_{nullptr};

std::atomic<SamplingProfiler *> GlobalProfiler::profilerForSig_{nullptr};

int GlobalProfiler::invokeSignalAction(void (*handler)(int)) {
  struct sigaction actions;
  memset(&actions, 0, sizeof(actions));
  sigemptyset(&actions.sa_mask);
  // Allows interrupted IO primitives to restart.
  actions.sa_flags = SA_RESTART;
  actions.sa_handler = handler;
  return sigaction(SIGPROF, &actions, nullptr);
}

bool GlobalProfiler::registerSignalHandlers() {
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

bool GlobalProfiler::unregisterSignalHandler() {
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

void GlobalProfiler::profilingSignalHandler(int signo) {
  // Ensure that writes made on the timer thread before setting the current
  // profiler are correctly acquired.
  SamplingProfiler *localProfiler;
  while (!(localProfiler = profilerForSig_.load(std::memory_order_acquire))) {
  }

  assert(
      localProfiler->suspendCount_ == 0 &&
      "Shouldn't interrupt the VM thread when the sampling profiler is "
      "suspended.");

  // Avoid spoiling errno in a signal handler by storing the old version and
  // re-assigning it.
  auto oldErrno = errno;

  auto profilerInstance = instance_.load();
  assert(
      profilerInstance != nullptr &&
      "Why is GlobalProfiler::instance_ not initialized yet?");

  // Sampling stack will touch GC objects(like closure) so only do so if heap
  // is valid.
  auto &curThreadRuntime = localProfiler->runtime_;
  assert(
      !curThreadRuntime.getHeap().inGC() &&
      "sampling profiler should be suspended before GC");
  (void)curThreadRuntime;
  profilerInstance->sampledStackDepth_ = localProfiler->walkRuntimeStack(
      profilerInstance->sampleStorage_, SamplingProfiler::InLoom::No);
  // Ensure that writes made in the handler are visible to the timer thread.
  profilerForSig_.store(nullptr);

  if (!instance_.load()->samplingDoneSem_.notifyOne()) {
    errno = oldErrno;
    abort(); // Something is wrong.
  }
  errno = oldErrno;
}

/*static*/ GlobalProfiler *GlobalProfiler::get() {
  // We intentionally leak this memory to avoid a case where instance is
  // accessed after it is destroyed during shutdown.
  static GlobalProfiler *instance = new GlobalProfiler{};
  return instance;
}

#if (defined(__ANDROID__) || defined(__APPLE__)) && \
    defined(HERMES_FACEBOOK_BUILD)
void SamplingProfiler::collectStackForLoomCommon(
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
#if defined(__ANDROID__)
      NativeFunctionPtr nativeFrame = frame.nativeFunctionPtrForLoom;
#else
      NativeFunctionPtr nativeFrame = getNativeFunctionPtr(frame);
#endif
      frames[(index)] = ((uint64_t)nativeFrame | kNativeFrameMask);
      break;
    }

#if defined(__APPLE__) && defined(HERMES_FACEBOOK_BUILD)
    case StackFrame::FrameKind::SuspendFrame:
      break;
#endif

    default:
      llvm_unreachable("Loom: unknown frame kind");
  }
}
#endif

#if defined(__ANDROID__) && defined(HERMES_FACEBOOK_BUILD)
/*static*/ StackCollectionRetcode SamplingProfiler::collectStackForLoom(
    ucontext_t *ucontext,
    int64_t *frames,
    uint16_t *depth,
    uint16_t max_depth) {
  auto profilerInstance = GlobalProfiler::instance_.load();
  SamplingProfiler *localProfiler =
      profilerInstance->threadLocalProfilerForLoom_.get();
  if (localProfiler == nullptr) {
    // No runtime in this thread.
    return StackCollectionRetcode::NO_STACK_FOR_THREAD;
  }

  uint32_t sampledStackDepth = 0;
  // Sampling stack will touch GC objects(like closure) so
  // only do so if heap is valid.
  if (LLVM_LIKELY(localProfiler->suspendCount_ == 0)) {
    Runtime &curThreadRuntime = localProfiler->runtime_;
    assert(
        !curThreadRuntime.getHeap().inGC() &&
        "sampling profiler should be suspended before GC");
    (void)curThreadRuntime;
    assert(
        profilerInstance != nullptr &&
        "Why is GlobalProfiler::instance_ not initialized yet?");
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

#if defined(__APPLE__) && defined(HERMES_FACEBOOK_BUILD)
bool SamplingProfiler::shouldPushDataToLoom() const {
  auto now = std::chrono::system_clock::now();
  constexpr auto kLoomDelay = std::chrono::milliseconds(50);
  // The default sample stack interval in timerLoop() is between 5-15ms which
  // is too often for loom.
  return loomDataPushEnabled_ && (now - previousPushTs > kLoomDelay);
}

void SamplingProfiler::pushLastSampledStackToLoom() {
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
  // TODO(xidachen): do a refactor to use the enum in ExternalTracer.h
  const int32_t tracerTypeJavascript = 1;
  fbloom_profilo_api()->fbloom_write_stack_to_loom(
      tracerTypeJavascript, frames, depth);
  previousPushTs = std::chrono::system_clock::now();
  clear();
}
#endif

} // namespace vm
} // namespace hermes

#endif // defined(HERMESVM_SAMPLING_PROFILER_POSIX)
