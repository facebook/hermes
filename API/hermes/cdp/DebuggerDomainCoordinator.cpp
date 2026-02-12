/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "DebuggerDomainCoordinator.h"
#include "DebuggerDomainAgent.h"

#include <algorithm>

namespace facebook {
namespace hermes {
namespace cdp {

using namespace facebook::hermes::debugger;

DebuggerDomainCoordinator::DebuggerDomainCoordinator(HermesRuntime &runtime)
    : runtime_(runtime), breakpointsActive_(false), paused_(false) {}

DebuggerDomainCoordinator::~DebuggerDomainCoordinator() {
  // All DebuggerDomainAgent instances must be cleaned up before
  // DebuggerDomainCoordinator is destroyed.
  assert(
      enabledAgents_.empty() &&
      "DebuggerDomainCoordinator destroyed with enabled agents still registered. "
      "This indicates a lifecycle bug - agents must be disabled before "
      "DebuggerDomainCoordinator destruction.");
}

void DebuggerDomainCoordinator::handleDebuggerEvent(
    HermesRuntime &runtime,
    AsyncDebuggerAPI &asyncDebugger,
    DebuggerEventType event) {
  assert(
      !enabledAgents_.empty() &&
      "Should only be receiving debugger events while enabled");

  switch (event) {
    case DebuggerEventType::ScriptLoaded:
      processNewLoadedScript();
      asyncDebugger.resumeFromPaused(AsyncDebugCommand::Continue);
      break;
    case DebuggerEventType::Exception:
      if (isTopFrameLocationBlackboxed()) {
        asyncDebugger.resumeFromPaused(
            explicitPausePending_ ? AsyncDebugCommand::StepInto
                                  : AsyncDebugCommand::Continue);
      } else {
        setPaused(PausedNotificationReason::kException);
      }
      break;
    case DebuggerEventType::Resumed:
      if (paused_) {
        setUnpaused();
      }
      break;
    case DebuggerEventType::Breakpoint:
      if (breakpointsActive_) {
        setPaused(PausedNotificationReason::kOther);
      } else {
        asyncDebugger.resumeFromPaused(
            explicitPausePending_ ? AsyncDebugCommand::StepInto
                                  : AsyncDebugCommand::Continue);
      }
      break;
    case DebuggerEventType::DebuggerStatement:
      if (isTopFrameLocationBlackboxed()) {
        asyncDebugger.resumeFromPaused(AsyncDebugCommand::Continue);
      } else {
        setPaused(PausedNotificationReason::kOther);
      }
      break;
    case DebuggerEventType::StepFinish:
      if (isTopFrameLocationBlackboxed()) {
        // If we land on a blackboxed frame via a user command that is either
        // "step over" or "step out" we step out of the blackboxed frame.
        // In all the other cases (explicit pause / step into / unknown command)
        // we execute a step into (that usually results in subsequent step
        // intos) to stop on the next non-blackboxed line.
        // If the blackboxed function calls any functions in a non-blackboxed
        // range, it would eventually stop on first such a function.
        // If the blackboxed function doesn't call any non-blackboxed functions,
        // it would keep on stepping into until it exits the blackboxed function
        // itself, effectively acting like a step out.
        auto nextStep = lastUserStepRequest_.has_value() &&
                (*lastUserStepRequest_ == UserStepRequest::StepOver ||
                 *lastUserStepRequest_ == UserStepRequest::StepOut)
            ? AsyncDebugCommand::StepOut
            : AsyncDebugCommand::StepInto;

        asyncDebugger.resumeFromPaused(nextStep);
      } else {
        PausedNotificationReason pauseReason = explicitPausePending_
            ? PausedNotificationReason::kOther
            : PausedNotificationReason::kStep;
        setPaused(pauseReason);
      }
      break;
    case DebuggerEventType::ExplicitPause:
      if (isTopFrameLocationBlackboxed()) {
        // an explicit pause should eventually stop execution on the next
        // non-blackboxed line, so we can just continue to step into here
        // until we hit a non-blackboxed line
        asyncDebugger.resumeFromPaused(AsyncDebugCommand::StepInto);
      } else {
        setPaused(PausedNotificationReason::kOther);
      }
      break;
    default:
      assert(false && "unknown DebuggerEventType");
      asyncDebugger.resumeFromPaused(AsyncDebugCommand::Continue);
  }
}

void DebuggerDomainCoordinator::setPaused(
    PausedNotificationReason pausedNotificationReason) {
  paused_ = true;
  explicitPausePending_ = false;
  for (auto &agent : enabledAgents_) {
    agent->notifyPaused(pausedNotificationReason);
  }
}

void DebuggerDomainCoordinator::setUnpaused() {
  paused_ = false;
  for (auto &agent : enabledAgents_) {
    agent->notifyUnpaused();
  }
}

void DebuggerDomainCoordinator::enableAgent(
    AsyncDebuggerAPI &asyncDebugger,
    DebuggerDomainAgent &agent) {
  assert(
      std::find(enabledAgents_.begin(), enabledAgents_.end(), &agent) ==
      enabledAgents_.end());

  bool needToRegisterCallback = enabledAgents_.empty();
  enabledAgents_.push_back(&agent);

  // The debugger just got enabled; inform the client about all scripts.
  for (auto &srcLoc : runtime_.getDebugger().getLoadedScripts()) {
    agent.processScript(srcLoc);
  }

  if (needToRegisterCallback) {
    runtime_.getDebugger().setShouldPauseOnScriptLoad(true);
    debuggerEventCallbackId_ = asyncDebugger.addDebuggerEventCallback_TS(
        [this](
            HermesRuntime &runtime,
            AsyncDebuggerAPI &asyncDebugger,
            DebuggerEventType event) {
          handleDebuggerEvent(runtime, asyncDebugger, event);
        });
    // Read the initial pause state from the runtime (it could be paused due to
    // e.g. other users of the same AsyncDebuggerAPI).
    // handleDebuggerEvent() will take care of updating this for future
    // pause/unpause events.
    paused_ = asyncDebugger.isWaitingForCommand();

    // This is the first agent, so any state related to previous pause/step
    // requests (e.g. from any previously enabled and later disabled agents)
    // should have been cleared.
    assert(!explicitPausePending_);
    assert(!lastUserStepRequest_);
  }

  if (paused_) {
    // CDP clients assume that the runtime is unpaused unless we tell them
    // otherwise.
    agent.notifyPaused(PausedNotificationReason::kOther);
  }
}

void DebuggerDomainCoordinator::disableAgent(
    AsyncDebuggerAPI &asyncDebugger,
    DebuggerDomainAgent &agent) {
  auto it = std::find(enabledAgents_.begin(), enabledAgents_.end(), &agent);
  assert(it != enabledAgents_.end());
  enabledAgents_.erase(it);

  if (enabledAgents_.empty()) {
    if (debuggerEventCallbackId_ != kInvalidDebuggerEventCallbackID) {
      asyncDebugger.removeDebuggerEventCallback_TS(debuggerEventCallbackId_);
      debuggerEventCallbackId_ = kInvalidDebuggerEventCallbackID;
    }

    // If a pending explicit pause was set, clear it. Even if we later
    // resubscribe to debugger events, by then we may have missed the pause
    // event corresponding to the explicit pause.
    explicitPausePending_ = false;

    // If a user step request was set, clear it, for the same reason as
    // explicitPausePending_ above.
    lastUserStepRequest_ = std::nullopt;

    // NOTE: This assumes that no other users of the current runtime are
    // interested in debugger events for script loads.
    runtime_.getDebugger().setShouldPauseOnScriptLoad(false);
  }
}

void DebuggerDomainCoordinator::pause() {
  explicitPausePending_ = true;

  runtime_.getDebugger().triggerAsyncPause(AsyncPauseKind::Explicit);
}

bool DebuggerDomainCoordinator::isTopFrameLocationBlackboxed() {
  auto stackTrace = runtime_.getDebugger().getProgramState().getStackTrace();
  if (stackTrace.callFrameCount() < 1) {
    return false;
  }
  debugger::SourceLocation loc = stackTrace.callFrameForIndex(0).location;
  if (breakpointsActive_) {
    // Locations with manual breakpoints are not considered blackboxed.
    // For example, if a user steps inside a blackboxed function, if any of
    // the next lines in that function have a manual breakpoint, we should
    // respect them and stop on them rather than stepping over them. The
    // same logic applies to explicit pauses. While they trigger stepping
    // into until out of blackboxed ranges, or until the program continues
    // execution if one of these steps lands on a line with a manual
    // breakpoint, we should stop on it.
    if (std::any_of(
            enabledAgents_.begin(), enabledAgents_.end(), [&loc](auto &agent) {
              return agent->locationHasManualBreakpoint(loc);
            })) {
      return false;
    }
  }
  return std::all_of(
      enabledAgents_.begin(), enabledAgents_.end(), [&loc](auto &agent) {
        return agent->isLocationBlackboxed(loc);
      });
}

void DebuggerDomainCoordinator::setBreakpointsActive(bool active) {
  breakpointsActive_ = active;
}

bool DebuggerDomainCoordinator::isPaused(
    AsyncDebuggerAPI &asyncDebugger) const {
  return paused_ || asyncDebugger.isWaitingForCommand();
}

void DebuggerDomainCoordinator::processNewLoadedScript() {
  auto stackTrace = runtime_.getDebugger().getProgramState().getStackTrace();

  if (stackTrace.callFrameCount() > 0) {
    debugger::SourceLocation loc = stackTrace.callFrameForIndex(0).location;

    // Invalid fileId indicates debug info isn't included when compilation took
    // place. E.g. compiling to bytecode without -g.
    if (loc.fileId == debugger::kInvalidLocation) {
      return;
    }

    for (auto &agent : enabledAgents_) {
      agent->processScript(loc);
    }
  }
}

void DebuggerDomainCoordinator::stepFromPaused(
    AsyncDebuggerAPI &asyncDebugger,
    UserStepRequest stepRequest) {
  lastUserStepRequest_ = stepRequest;
  switch (stepRequest) {
    case UserStepRequest::StepInto:
      asyncDebugger.resumeFromPaused(AsyncDebugCommand::StepInto);
      break;
    case UserStepRequest::StepOver:
      asyncDebugger.resumeFromPaused(AsyncDebugCommand::StepOver);
      break;
    case UserStepRequest::StepOut:
      asyncDebugger.resumeFromPaused(AsyncDebugCommand::StepOut);
      break;
    default:
      assert(false && "unknown UserStepRequest");
      asyncDebugger.resumeFromPaused(AsyncDebugCommand::Continue);
  }
}

void DebuggerDomainCoordinator::resume(AsyncDebuggerAPI &asyncDebugger) {
  asyncDebugger.resumeFromPaused(AsyncDebugCommand::Continue);
}

} // namespace cdp
} // namespace hermes
} // namespace facebook
