/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "AsyncDebuggerAPI.h"

#include <llvh/ADT/ScopeExit.h>

namespace facebook {
namespace hermes {
namespace debugger {

AsyncDebuggerAPI::AsyncDebuggerAPI(HermesRuntime &runtime)
    : runtime_(runtime),
      isWaitingForCommand_(false),
      nextCommand_(debugger::Command::continueExecution()),
      nextEventCallbackID_(kInvalidDebuggerEventCallbackID + 1),
      eventCallbackIterator_(eventCallbacks_.end()) {
  runtime_.getDebugger().setEventObserver(this);
}

AsyncDebuggerAPI::~AsyncDebuggerAPI() {
  runtime_.getDebugger().setEventObserver(nullptr);

  // Before destructing, give anything that requested an interrupt a chance to
  // run on the runtime thread.
  runInterrupts();
}

DebuggerEventCallbackID AsyncDebuggerAPI::addDebuggerEventCallback_TS(
    DebuggerEventCallback callback) {
  std::lock_guard<std::mutex> lock(mutex_);
  uint32_t id = nextEventCallbackID_++;
  eventCallbacks_.push_back({id, callback});
  // Runtime thread might be on hold by didPause. Need to signal to unblock
  // that.
  signal_.notify_one();
  return id;
}

void AsyncDebuggerAPI::removeDebuggerEventCallback_TS(
    DebuggerEventCallbackID id) {
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto it = eventCallbacks_.begin(); it != eventCallbacks_.end(); it++) {
    if (it->id == id) {
      // eventCallbacks_ is a std::list. Its iterators remain valid even after
      // erase() as long as the iterator isn't the element being erased. If
      // we're trying to erase the same element as the one pointed to by
      // eventCallbackIterator_, then increment the iterator so that the next
      // time takeNextEventCallback() runs it'll have a valid iterator.
      if (it == eventCallbackIterator_) {
        eventCallbackIterator_++;
      }

      // Note that the callback at iterator `it` might be the currently
      // executing DebuggerEventCallback. This is ok because runEventCallbacks()
      // and takeNextEventCallback() will make a copy of the
      // DebuggerEventCallback, so removing the copy from eventCallbacks_ is ok.
      //
      // Also, the typical expectation of a remove callback function is that
      // once the function returns, nothing will invoke that callback again.
      // This becomes weird if the remove function is being called from the
      // callback itself. However, there isn't a lifetime ambiguity because the
      // callback function is running and it shouldn't expect to be able to
      // destroy whatever object containing the function.
      eventCallbacks_.erase(it);
      break;
    }
  }
  // Runtime thread might be on hold by didPause. Need to signal to unblock
  // that.
  signal_.notify_one();
}

bool AsyncDebuggerAPI::isWaitingForCommand() {
  return isWaitingForCommand_;
}

void AsyncDebuggerAPI::setNextCommand(debugger::Command command) {
  assert(
      isWaitingForCommand_ &&
      "can only set next command when isWaitingForCommand_ is true");
  nextCommand_ = std::move(command);
  isWaitingForCommand_ = false;
}

void AsyncDebuggerAPI::triggerInterrupt_TS(InterruptCallback callback) {
  assert(callback && "Must have callback function provided");
  std::lock_guard<std::mutex> lock(mutex_);
  interruptCallbacks_.push(std::move(callback));
  signal_.notify_one();
  runtime_.getDebugger().triggerAsyncPause(debugger::AsyncPauseKind::Implicit);
}

debugger::Command AsyncDebuggerAPI::didPause(debugger::Debugger &debugger) {
  if (inDidPause_) {
    throw std::runtime_error("unexpected recursive call to didPause");
  }
  inDidPause_ = true;
  auto clearInDidPause = llvh::make_scope_exit([this] { inDidPause_ = false; });

  debugger::PauseReason pauseReason =
      debugger.getProgramState().getPauseReason();

  if (pauseReason == debugger::PauseReason::AsyncTrigger) {
    runInterrupts();
    return debugger::Command::continueExecution();
  }

  isWaitingForCommand_ = true;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (eventCallbacks_.empty()) {
      isWaitingForCommand_ = false;
      return debugger::Command::continueExecution();
    }
  }
  DebuggerEventType event;
  switch (pauseReason) {
    case debugger::PauseReason::ScriptLoaded:
      event = DebuggerEventType::ScriptLoaded;
      break;
    case debugger::PauseReason::Exception:
      event = DebuggerEventType::Exception;
      break;
    case debugger::PauseReason::EvalComplete:
      event = DebuggerEventType::EvalComplete;
      break;
    case debugger::PauseReason::DebuggerStatement:
      event = DebuggerEventType::DebuggerStatement;
      break;
    case debugger::PauseReason::Breakpoint:
      event = DebuggerEventType::Breakpoint;
      break;
    case debugger::PauseReason::StepFinish:
      event = DebuggerEventType::StepFinish;
      break;
    case debugger::PauseReason::AsyncTrigger:
      assert(false && "Should have executed interrupts and returned already");
      break;
  }
  runEventCallbacks(event);

  // After invoking the didPauseCallback_, we'll continue to hold onto the
  // thread in order simulate an async API where didPauseCallback doesn't need
  // to immediately decide on what debugger::Command to return. While we're
  // holding onto the thread, we still need to service any interrupt requests
  // that come in.
  processInterruptWhilePaused();
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (eventCallbacks_.empty()) {
      isWaitingForCommand_ = false;
      return debugger::Command::continueExecution();
    }
  }

  assert(
      !isWaitingForCommand_ &&
      "Should only exit didPause with a debugger::Command");
  runEventCallbacks(DebuggerEventType::Resumed);
  return std::move(nextCommand_);
}

void AsyncDebuggerAPI::processInterruptWhilePaused() {
  std::unique_lock<std::mutex> lock(mutex_);
  // We check whether `debuggerEventCallbacks_` has callbacks in all the
  // while-loops below. This is because that callback could be cleared by users
  // of AsyncDebuggerAPI at any time. We don't impose requirement on callers of
  // AsyncDebuggerAPI when they must be constructed and destructed.
  while (isWaitingForCommand_ && !eventCallbacks_.empty()) {
    while (interruptCallbacks_.empty() && !eventCallbacks_.empty()) {
      signal_.wait(lock);
    }
    if (!eventCallbacks_.empty()) {
      lock.unlock();
      runInterrupts();
      lock.lock();
    }
  }
}

std::optional<InterruptCallback> AsyncDebuggerAPI::takeNextInterruptCallback() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (interruptCallbacks_.empty()) {
    return std::nullopt;
  }

  InterruptCallback func = interruptCallbacks_.front();
  interruptCallbacks_.pop();
  return func;
}

void AsyncDebuggerAPI::runInterrupts() {
  std::optional<InterruptCallback> entry = takeNextInterruptCallback();
  while (entry.has_value()) {
    InterruptCallback func = entry.value();
    if (func) {
      func(runtime_);
    }
    entry = takeNextInterruptCallback();
  }
}

std::optional<DebuggerEventCallback> AsyncDebuggerAPI::takeNextEventCallback() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (eventCallbackIterator_ == eventCallbacks_.end()) {
    return std::nullopt;
  }

  DebuggerEventCallback func = eventCallbackIterator_->callback;

  eventCallbackIterator_++;

  return func;
}

void AsyncDebuggerAPI::runEventCallbacks(DebuggerEventType event) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    eventCallbackIterator_ = eventCallbacks_.begin();
  }

  std::optional<DebuggerEventCallback> entry = takeNextEventCallback();
  while (entry.has_value()) {
    DebuggerEventCallback func = entry.value();
    if (func) {
      func(runtime_, *this, event);
    }
    entry = takeNextEventCallback();
  }
}

std::unique_ptr<AsyncDebuggerAPI> AsyncDebuggerAPI::create(
    HermesRuntime &runtime) {
  // Can't use make_unique here since the constructor is private.
  return std::unique_ptr<AsyncDebuggerAPI>(new AsyncDebuggerAPI(runtime));
}

} // namespace debugger
} // namespace hermes
} // namespace facebook
