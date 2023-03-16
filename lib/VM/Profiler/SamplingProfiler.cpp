/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/Profiler/SamplingProfiler.h"

#if HERMESVM_SAMPLING_PROFILER_AVAILABLE

#include "hermes/VM/Callable.h"
#include "hermes/VM/HostModel.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/RuntimeModule-inline.h"
#include "hermes/VM/StackFrame-inline.h"

#include "llvh/Support/Compiler.h"

#include "ChromeTraceSerializer.h"
#include "SamplingProfilerSampler.h"

#include <fcntl.h>
#include <cassert>
#include <chrono>
#include <cmath>
#include <csignal>
#include <random>
#include <thread>

namespace hermes {
namespace vm {

void SamplingProfiler::registerDomain(Domain *domain) {
  // If domain is not already registered, add it to the list.
  auto it = std::find(domains_.begin(), domains_.end(), domain);
  if (it == domains_.end())
    domains_.push_back(domain);
}

SamplingProfiler::NativeFunctionFrameInfo
SamplingProfiler::registerNativeFunction(NativeFunction *nativeFunction) {
  // If nativeFunction is not already registered, add it to the list.
  auto it = std::find(
      nativeFunctions_.begin(), nativeFunctions_.end(), nativeFunction);
  if (it != nativeFunctions_.end()) {
    return it - nativeFunctions_.begin();
  }

  nativeFunctions_.push_back(nativeFunction);
  return nativeFunctions_.size() - 1;
}

void SamplingProfiler::markRootsForCompleteMarking(RootAcceptor &acceptor) {
  std::lock_guard<std::mutex> lockGuard(runtimeDataLock_);
  for (Domain *&domain : domains_) {
    acceptor.acceptPtr(domain);
  }
}

void SamplingProfiler::markRoots(RootAcceptor &acceptor) {
  std::lock_guard<std::mutex> lockGuard(runtimeDataLock_);
  for (Domain *&domain : domains_) {
    acceptor.acceptPtr(domain);
  }

  for (NativeFunction *&fn : nativeFunctions_) {
    acceptor.acceptPtr(fn);
  }
}

uint32_t SamplingProfiler::walkRuntimeStack(
    StackTrace &sampleStorage,
    InLoom inLoom,
    uint32_t startIndex) {
  unsigned count = startIndex;

  // TODO: capture leaf frame IP.
  const Inst *ip = nullptr;
  for (ConstStackFramePtr frame : runtime_.getStackFrames()) {
    // Whether we successfully captured a stack frame or not.
    bool capturedFrame = true;
    auto &frameStorage = sampleStorage.stack[count];
    // Check if it is pure JS frame.
    auto *calleeCodeBlock = frame.getCalleeCodeBlock(runtime_);
    if (calleeCodeBlock != nullptr) {
      frameStorage.kind = StackFrame::FrameKind::JSFunction;
      frameStorage.jsFrame.functionId = calleeCodeBlock->getFunctionID();
      frameStorage.jsFrame.offset =
          (ip == nullptr ? 0 : calleeCodeBlock->getOffsetOf(ip));
      auto *module = calleeCodeBlock->getRuntimeModule();
      assert(module != nullptr && "Cannot fetch runtimeModule for code block");
      frameStorage.jsFrame.module = module;
      // Don't execute a read or write barrier here because this is a signal
      // handler.
      if (inLoom != InLoom::Yes)
        registerDomain(module->getDomainForSamplingProfiler(runtime_));
    } else if (
        auto *nativeFunction =
            dyn_vmcast<NativeFunction>(frame.getCalleeClosureUnsafe())) {
      frameStorage.kind = vmisa<FinalizableNativeFunction>(nativeFunction)
          ? StackFrame::FrameKind::FinalizableNativeFunction
          : StackFrame::FrameKind::NativeFunction;
      if (inLoom != InLoom::Yes) {
        frameStorage.nativeFrame = registerNativeFunction(nativeFunction);
      } else {
        frameStorage.nativeFunctionPtrForLoom =
            nativeFunction->getFunctionPtr();
      }
    } else {
      // TODO: handle BoundFunction.
      capturedFrame = false;
    }

    // Update ip to caller for next iteration.
    ip = frame.getSavedIP();
    if (capturedFrame) {
      ++count;
      if (count >= sampleStorage.stack.size()) {
        break;
      }
    }
  }
  sampleStorage.tid = oscompat::thread_id();
  sampleStorage.timeStamp = std::chrono::steady_clock::now();
  return count;
}

SamplingProfiler::SamplingProfiler(Runtime &runtime) : runtime_{runtime} {
  threadNames_[oscompat::thread_id()] = oscompat::thread_name();
  sampling_profiler::Sampler::get()->registerRuntime(this);
}

void SamplingProfiler::dumpSampledStackGlobal(llvh::raw_ostream &OS) {
  auto globalProfiler = sampling_profiler::Sampler::get();
  std::lock_guard<std::mutex> lk(globalProfiler->profilerLock_);
  if (!globalProfiler->profilers_.empty()) {
    auto *localProfiler = *globalProfiler->profilers_.begin();
    localProfiler->dumpSampledStack(OS);
  }
}

void SamplingProfiler::dumpSampledStack(llvh::raw_ostream &OS) {
  std::lock_guard<std::mutex> lk(runtimeDataLock_);
  OS << "dumpSamples called from runtime\n";
  OS << "Total " << sampledStacks_.size() << " samples\n";
  for (unsigned i = 0; i < sampledStacks_.size(); ++i) {
    auto &sample = sampledStacks_[i];
    uint64_t timeStamp = sample.timeStamp.time_since_epoch().count();
    OS << "[" << i << "]: tid[" << sample.tid << "], ts[" << timeStamp << "] ";

    // Leaf frame is in sample[0] so dump it backward to
    // get root => leaf represenation.
    for (auto iter = sample.stack.rbegin(); iter != sample.stack.rend();
         ++iter) {
      const StackFrame &frame = *iter;
      switch (frame.kind) {
        case StackFrame::FrameKind::JSFunction:
          OS << "[JS] " << frame.jsFrame.functionId << ":"
             << frame.jsFrame.offset;
          break;

        case StackFrame::FrameKind::NativeFunction: {
          NativeFunctionPtr nativeFrame = getNativeFunctionPtr(frame);
          OS << "[Native] " << reinterpret_cast<uintptr_t>(nativeFrame);
          break;
        }

        case StackFrame::FrameKind::FinalizableNativeFunction:
          OS << "[HostFunction] " << getNativeFunctionName(frame);
          break;

        default:
          llvm_unreachable("Unknown frame kind");
      }
      OS << " => ";
    }
    OS << "\n";
  }
}

void SamplingProfiler::dumpChromeTraceGlobal(llvh::raw_ostream &OS) {
  auto globalProfiler = sampling_profiler::Sampler::get();
  std::lock_guard<std::mutex> lk(globalProfiler->profilerLock_);
  if (!globalProfiler->profilers_.empty()) {
    auto *localProfiler = *globalProfiler->profilers_.begin();
    localProfiler->dumpChromeTrace(OS);
  }
}

void SamplingProfiler::dumpChromeTrace(llvh::raw_ostream &OS) {
  std::lock_guard<std::mutex> lk(runtimeDataLock_);
  auto pid = oscompat::process_id();
  ChromeTraceSerializer serializer(
      *this, ChromeTraceFormat::create(pid, threadNames_, sampledStacks_));
  serializer.serialize(OS);
  clear();
}

void SamplingProfiler::serializeInDevToolsFormat(llvh::raw_ostream &OS) {
  std::lock_guard<std::mutex> lk(runtimeDataLock_);
  hermes::vm::serializeAsProfilerProfile(
      *this,
      OS,
      ChromeTraceFormat::create(
          oscompat::process_id(), threadNames_, sampledStacks_));
  clear();
}

bool SamplingProfiler::enable() {
  return sampling_profiler::Sampler::get()->enable();
}

bool SamplingProfiler::disable() {
  return sampling_profiler::Sampler::get()->disable();
}

void SamplingProfiler::clear() {
  sampledStacks_.clear();
  // Release all strong roots.
  domains_.clear();
  nativeFunctions_.clear();
  // TODO: keep thread names that are still in use.
  threadNames_.clear();
}

void SamplingProfiler::suspend(std::string_view extraInfo) {
  // Need to check whether the profiler is enabled without holding the
  // runtimeDataLock_. Otherwise, we'd have a lock inversion.
  bool enabled = sampling_profiler::Sampler::get()->enabled();

  std::lock_guard<std::mutex> lk(runtimeDataLock_);
  if (++suspendCount_ > 1 || extraInfo.empty()) {
    // If there are multiple nested suspend calls use a default "suspended"
    // label for the suspend entry in the call stack. Also use the default
    // when no extra info is provided.
    extraInfo = "suspended";
  }

  // Only record the stack trace for the first suspend() call.
  if (LLVM_UNLIKELY(enabled && suspendCount_ == 1)) {
    recordPreSuspendStack(extraInfo);
  }
}

void SamplingProfiler::resume() {
  std::lock_guard<std::mutex> lk(runtimeDataLock_);
  assert(suspendCount_ > 0 && "resume() without suspend()");
  if (--suspendCount_ == 0) {
    preSuspendStackDepth_ = 0;
  }
}

void SamplingProfiler::recordPreSuspendStack(std::string_view extraInfo) {
  std::pair<std::unordered_set<std::string>::iterator, bool> retPair =
      suspendEventExtraInfoSet_.emplace(extraInfo);
  SuspendFrameInfo suspendExtraInfo = &(*(retPair.first));

  auto &leafFrame = preSuspendStackStorage_.stack[0];
  leafFrame.kind = StackFrame::FrameKind::SuspendFrame;
  leafFrame.suspendFrame = suspendExtraInfo;

  // Leaf frame slot has been used, filling from index 1.
  preSuspendStackDepth_ =
      walkRuntimeStack(preSuspendStackStorage_, InLoom::No, 1);
}

bool operator==(
    const SamplingProfiler::StackFrame &left,
    const SamplingProfiler::StackFrame &right) {
  if (left.kind != right.kind) {
    return false;
  }
  switch (left.kind) {
    case SamplingProfiler::StackFrame::FrameKind::JSFunction:
      return left.jsFrame.functionId == right.jsFrame.functionId &&
          left.jsFrame.offset == right.jsFrame.offset;

    case SamplingProfiler::StackFrame::FrameKind::NativeFunction:
    case SamplingProfiler::StackFrame::FrameKind::FinalizableNativeFunction:
      return left.nativeFrame == right.nativeFrame;

    case SamplingProfiler::StackFrame::FrameKind::SuspendFrame:
      return left.suspendFrame == right.suspendFrame;

    default:
      llvm_unreachable("Unknown frame kind");
  }
}

} // namespace vm
} // namespace hermes

#endif // HERMESVM_SAMPLING_PROFILER_AVAILABLE
