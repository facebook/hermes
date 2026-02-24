/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_ENABLE_DEBUGGER

#include "AsyncDebuggerAPI.h"

#include <hermes/Support/ErrorHandling.h>

#include <llvh/ADT/ScopeExit.h>

namespace facebook {
namespace hermes {
namespace debugger {

AsyncDebuggerAPI::AsyncDebuggerAPI(HermesRuntime &runtime)
    : runtime_(runtime),
      isWaitingForCommand_(false),
      nextCommand_(debugger::Command::continueExecution()) {
  runtime_.getDebugger().setEventObserver(this);
}

AsyncDebuggerAPI::~AsyncDebuggerAPI() {
  runtime_.getDebugger().setEventObserver(nullptr);

  // Before destructing, give anything that requested an interrupt a chance to
  // run on the runtime thread.
  runInterrupts();
}

void AsyncDebuggerAPI::setDebuggerEventCallback_TS(
    DebuggerEventCallback callback) {
  assert(
      callback &&
      "callback must be non-empty; use clearDebuggerEventCallback_TS to clear");
  std::lock_guard<std::mutex> lock(mutex_);
  eventCallback_ = std::move(callback);

  runtime_.getDebugger().setIsDebuggerAttached(true);

  // Runtime thread might be on hold by didPause. Need to signal to unblock
  // that.
  signal_.notify_one();
}

void AsyncDebuggerAPI::clearDebuggerEventCallback_TS() {
  std::lock_guard<std::mutex> lock(mutex_);
  eventCallback_ = nullptr;

  runtime_.getDebugger().setIsDebuggerAttached(false);

  // Runtime thread might be on hold by didPause. Need to signal to unblock
  // that.
  signal_.notify_one();
}

bool AsyncDebuggerAPI::isWaitingForCommand() {
  return isWaitingForCommand_;
}

bool AsyncDebuggerAPI::isPaused() const {
  return inDidPause_;
}

bool AsyncDebuggerAPI::resumeFromPaused(AsyncDebugCommand command) {
  if (!isWaitingForCommand_) {
    return false;
  }
  switch (command) {
    case AsyncDebugCommand::Continue:
      nextCommand_ = debugger::Command::continueExecution();
      break;
    case AsyncDebugCommand::StepInto:
      nextCommand_ = debugger::Command::step(debugger::StepMode::Into);
      break;
    case AsyncDebugCommand::StepOver:
      nextCommand_ = debugger::Command::step(debugger::StepMode::Over);
      break;
    case AsyncDebugCommand::StepOut:
      nextCommand_ = debugger::Command::step(debugger::StepMode::Out);
      break;
  }
  isWaitingForCommand_ = false;
  return true;
}

bool AsyncDebuggerAPI::evalWhilePaused(
    const std::string &expression,
    uint32_t frameIndex,
    EvalCompleteCallback callback) {
  if (!isWaitingForCommand_) {
    return false;
  }
  if (!callback) {
    throw std::runtime_error("EvalCompleteCallback cannot be empty");
  }
  oneTimeEvalCompleteCallback_ = callback;
  nextCommand_ = debugger::Command::eval(expression, frameIndex);
  isWaitingForCommand_ = false;
  return true;
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

  if (pauseReason == debugger::PauseReason::AsyncTriggerImplicit) {
    runInterrupts();
    return debugger::Command::continueExecution();
  } else if (pauseReason == debugger::PauseReason::EvalComplete) {
    // Coming back from EvalComplete means that the JavaScript program execution
    // is still paused. Even though this invocation of didPause's PauseReason is
    // EvalComplete, the actual reason is whatever it was previously. As such,
    // since the program is still paused, we're still waiting on the next
    // command.
    isWaitingForCommand_ = true;

    auto evalResult = runtime_.getDebugger().getProgramState().getEvalResult();
    assert(
        oneTimeEvalCompleteCallback_ &&
        "There should always be a non-empty callback to receive the EvalResult");
    oneTimeEvalCompleteCallback_(runtime_, evalResult);
    oneTimeEvalCompleteCallback_ = EvalCompleteCallback{};
  } else {
    isWaitingForCommand_ = true;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (!eventCallback_) {
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
      case debugger::PauseReason::DebuggerStatement:
        event = DebuggerEventType::DebuggerStatement;
        break;
      case debugger::PauseReason::Breakpoint:
        event = DebuggerEventType::Breakpoint;
        break;
      case debugger::PauseReason::StepFinish:
        event = DebuggerEventType::StepFinish;
        break;
      case debugger::PauseReason::AsyncTriggerExplicit:
        event = DebuggerEventType::ExplicitPause;
        break;
      case debugger::PauseReason::EvalComplete:
        ::hermes::hermes_fatal(
            "Should have been handled differently in the else-if above");
      case debugger::PauseReason::AsyncTriggerImplicit:
        ::hermes::hermes_fatal(
            "Should have executed interrupts and returned already");
    }
    runEventCallback(event);
  }

  // After invoking DebuggerEventCallback, we'll continue to hold onto the
  // thread in order to simulate an async API where DebuggerEventCallback
  // doesn't need to immediately decide on what AsyncDebugCommand to return.
  // While we're holding onto the thread, we still need to service any interrupt
  // requests that come in.
  processInterruptWhilePaused();
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!eventCallback_) {
      isWaitingForCommand_ = false;
      return debugger::Command::continueExecution();
    }
  }

  assert(
      !isWaitingForCommand_ &&
      "Should only exit didPause with a debugger::Command");
  if (!nextCommand_.isEval()) {
    // If the next command is an eval command, the program execution isn't
    // resumed exactly. The Runtime will execute the given eval expression, but
    // the lib/VM/Debugger/Debugger.cpp's debuggerLoop() treats the eval command
    // more like a synchronous operation. So in this special case, we skip
    // sending the "Resumed" notification because we expect evaluation of the
    // expression to complete and come right back to didPause with EvalComplete.
    // Because of the check for `runtime.debugger_.isDebugging()` in
    // runDebuggerUpdatingState of Interpreter.h, performing the eval won't
    // trigger recursive didPauses.
    runEventCallback(DebuggerEventType::Resumed);
  }
  return std::move(nextCommand_);
}

void AsyncDebuggerAPI::processInterruptWhilePaused() {
  std::unique_lock<std::mutex> lock(mutex_);
  // We check whether `eventCallback_` is set in all the while-loops
  // below. This is because that callback could be cleared by users of
  // AsyncDebuggerAPI at any time. We don't impose requirement on callers of
  // AsyncDebuggerAPI when they must be constructed and destructed.
  while (isWaitingForCommand_ && eventCallback_) {
    while (isWaitingForCommand_ && eventCallback_ &&
           interruptCallbacks_.empty()) {
      signal_.wait(lock);
    }
    if (eventCallback_) {
      lock.unlock();
      // Specifically don't run all interrupts if one of the interrupts sets
      // the next command. Once the next command has been set, we'll exit
      // didPause.
      runInterrupts(false);
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

void AsyncDebuggerAPI::runInterrupts(bool ignoreNextCommand) {
  std::optional<InterruptCallback> entry = takeNextInterruptCallback();
  while (entry.has_value()) {
    InterruptCallback func = entry.value();
    if (func) {
      func(runtime_);
    }

    if (!ignoreNextCommand && !isWaitingForCommand_) {
      break;
    }

    entry = takeNextInterruptCallback();
  }
}

void AsyncDebuggerAPI::runEventCallback(DebuggerEventType event) {
  DebuggerEventCallback callback;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    callback = eventCallback_;
  }
  if (callback) {
    callback(runtime_, *this, event);
  }
}

} // namespace debugger
} // namespace hermes
} // namespace facebook

#endif // HERMES_ENABLE_DEBUGGER
