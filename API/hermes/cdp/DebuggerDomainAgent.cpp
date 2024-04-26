/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "DebuggerDomainAgent.h"

#include <hermes/inspector/chrome/RemoteObjectConverters.h>

namespace facebook {
namespace hermes {
namespace cdp {

using namespace facebook::hermes::debugger;

DebuggerDomainAgent::DebuggerDomainAgent(
    int32_t executionContextID,
    HermesRuntime &runtime,
    AsyncDebuggerAPI &asyncDebugger,
    SynchronizedOutboundCallback messageCallback)
    : DomainAgent(executionContextID, std::move(messageCallback)),
      runtime_(runtime),
      asyncDebugger_(asyncDebugger),
      debuggerEventCallbackId_(kInvalidDebuggerEventCallbackID),
      enabled_(false),
      paused_(false) {}

DebuggerDomainAgent::~DebuggerDomainAgent() {
  // Also remove DebuggerEventCallback here in case we don't receive a
  // Debugger.disable command prior to destruction.
  asyncDebugger_.removeDebuggerEventCallback_TS(debuggerEventCallbackId_);
}

void DebuggerDomainAgent::handleDebuggerEvent(
    HermesRuntime &runtime,
    AsyncDebuggerAPI &asyncDebugger,
    DebuggerEventType event) {
  assert(enabled_ && "Should only be receiving debugger events while enabled");

  switch (event) {
    case DebuggerEventType::ScriptLoaded:
      processNewLoadedScript();
      asyncDebugger_.resumeFromPaused(AsyncDebugCommand::Continue);
      break;
    case DebuggerEventType::Exception:
      paused_ = true;
      sendPauseOnExceptionNotificationToClient();
      break;
    case DebuggerEventType::Resumed:
      if (paused_) {
        paused_ = false;
        sendNotificationToClient(m::debugger::ResumedNotification{});
      }
      break;
    case DebuggerEventType::DebuggerStatement:
      paused_ = true;
      sendPausedNotificationToClient();
      break;
    case DebuggerEventType::Breakpoint:
      break;
    case DebuggerEventType::StepFinish:
      paused_ = true;
      sendPausedNotificationToClient();
      break;
    case DebuggerEventType::ExplicitPause:
      paused_ = true;
      sendPausedNotificationToClient();
      break;
  }
}

void DebuggerDomainAgent::enable(const m::debugger::EnableRequest &req) {
  if (enabled_) {
    sendResponseToClient(m::makeErrorResponse(
        req.id,
        m::ErrorCode::InvalidRequest,
        "Debugger domain already enabled"));
    return;
  }
  enabled_ = true;
  sendResponseToClient(m::makeOkResponse(req.id));

  // The debugger just got enabled; inform the client about all scripts.
  for (auto &srcLoc : runtime_.getDebugger().getLoadedScripts()) {
    sendScriptParsedNotificationToClient(srcLoc);
  }

  runtime_.getDebugger().setShouldPauseOnScriptLoad(true);
  debuggerEventCallbackId_ = asyncDebugger_.addDebuggerEventCallback_TS(
      [this](
          HermesRuntime &runtime,
          AsyncDebuggerAPI &asyncDebugger,
          DebuggerEventType event) {
        handleDebuggerEvent(runtime, asyncDebugger, event);
      });

  // Check to see if debugger is currently paused. There could be multiple debug
  // clients and the program could have already been paused.
  if (asyncDebugger_.isWaitingForCommand()) {
    paused_ = true;
    sendPausedNotificationToClient();
  }
}

void DebuggerDomainAgent::disable(const m::debugger::DisableRequest &req) {
  if (!checkDebuggerEnabled(req)) {
    return;
  }

  // This doesn't support other debug clients also setting breakpoints. If we
  // need that functionality, then we might need to track breakpoints set by
  // this client and only remove those.
  runtime_.getDebugger().deleteAllBreakpoints();

  asyncDebugger_.removeDebuggerEventCallback_TS(debuggerEventCallbackId_);
  debuggerEventCallbackId_ = kInvalidDebuggerEventCallbackID;
  // This doesn't work well if there are other debug clients that also toggle
  // this flag. If we need that functionality, then DebuggerAPI needs to be
  // changed.
  runtime_.getDebugger().setShouldPauseOnScriptLoad(false);

  enabled_ = false;

  sendResponseToClient(m::makeOkResponse(req.id));
}

void DebuggerDomainAgent::pause(const m::debugger::PauseRequest &req) {
  if (!checkDebuggerEnabled(req)) {
    return;
  }

  runtime_.getDebugger().triggerAsyncPause(AsyncPauseKind::Explicit);
  sendResponseToClient(m::makeOkResponse(req.id));
}

void DebuggerDomainAgent::resume(const m::debugger::ResumeRequest &req) {
  if (!checkDebuggerPaused(req)) {
    return;
  }

  asyncDebugger_.resumeFromPaused(AsyncDebugCommand::Continue);
  sendResponseToClient(m::makeOkResponse(req.id));
}

void DebuggerDomainAgent::stepInto(const m::debugger::StepIntoRequest &req) {
  if (!checkDebuggerPaused(req)) {
    return;
  }

  asyncDebugger_.resumeFromPaused(AsyncDebugCommand::StepInto);
  sendResponseToClient(m::makeOkResponse(req.id));
}

void DebuggerDomainAgent::stepOut(const m::debugger::StepOutRequest &req) {
  if (!checkDebuggerPaused(req)) {
    return;
  }

  asyncDebugger_.resumeFromPaused(AsyncDebugCommand::StepOut);
  sendResponseToClient(m::makeOkResponse(req.id));
}

void DebuggerDomainAgent::stepOver(const m::debugger::StepOverRequest &req) {
  if (!checkDebuggerPaused(req)) {
    return;
  }

  asyncDebugger_.resumeFromPaused(AsyncDebugCommand::StepOver);
  sendResponseToClient(m::makeOkResponse(req.id));
}

void DebuggerDomainAgent::setPauseOnExceptions(
    const m::debugger::SetPauseOnExceptionsRequest &req) {
  debugger::PauseOnThrowMode mode = debugger::PauseOnThrowMode::None;

  if (req.state == "none") {
    mode = debugger::PauseOnThrowMode::None;
  } else if (req.state == "all") {
    mode = debugger::PauseOnThrowMode::All;
  } else if (req.state == "uncaught") {
    mode = debugger::PauseOnThrowMode::Uncaught;
  } else {
    sendResponseToClient(m::makeErrorResponse(
        req.id,
        m::ErrorCode::InvalidRequest,
        "Unknown pause-on-exception state: " + req.state));
    return;
  }

  runtime_.getDebugger().setPauseOnThrowMode(mode);
  sendResponseToClient(m::makeOkResponse(req.id));
}

void DebuggerDomainAgent::evaluateOnCallFrame(
    const m::debugger::EvaluateOnCallFrameRequest &req) {
  if (!checkDebuggerPaused(req)) {
    return;
  }

  uint32_t frameIndex = (uint32_t)atoi(req.callFrameId.c_str());
  asyncDebugger_.evalWhilePaused(
      req.expression,
      frameIndex,
      [&req, this](HermesRuntime &runtime, const debugger::EvalResult &result) {
        m::debugger::EvaluateOnCallFrameResponse resp;
        resp.id = req.id;
        if (result.isException) {
          resp.exceptionDetails =
              m::runtime::makeExceptionDetails(result.exceptionDetails);
        } else {
          auto remoteObjPtr = std::make_shared<m::runtime::RemoteObject>();

          std::string objectGroup = req.objectGroup.value_or("");
          bool byValue = req.returnByValue.value_or(false);
          bool generatePreview = req.generatePreview.value_or(false);
          *remoteObjPtr = m::runtime::makeRemoteObject(
              runtime_,
              result.value,
              objTable_,
              objectGroup,
              byValue,
              generatePreview);

          resp.result = std::move(*remoteObjPtr);
        }
        sendResponseToClient(resp);
      });
}

void DebuggerDomainAgent::sendPausedNotificationToClient() {
  m::debugger::PausedNotification note;
  note.reason = "other";
  note.callFrames = m::debugger::makeCallFrames(
      runtime_.getDebugger().getProgramState(), objTable_, runtime_);
  sendNotificationToClient(note);
}

void DebuggerDomainAgent::sendPauseOnExceptionNotificationToClient() {
  m::debugger::PausedNotification note;
  note.reason = "exception";
  note.callFrames = m::debugger::makeCallFrames(
      runtime_.getDebugger().getProgramState(), objTable_, runtime_);
  sendNotificationToClient(note);
}

void DebuggerDomainAgent::sendScriptParsedNotificationToClient(
    const debugger::SourceLocation srcLoc) {
  m::debugger::ScriptParsedNotification note;
  note.scriptId = std::to_string(srcLoc.fileId);
  note.url = srcLoc.fileName;
  note.executionContextId = executionContextID_;
  std::string sourceMappingUrl =
      runtime_.getDebugger().getSourceMappingUrl(srcLoc.fileId);
  if (!sourceMappingUrl.empty()) {
    note.sourceMapURL = sourceMappingUrl;
  }
  sendNotificationToClient(note);
}

void DebuggerDomainAgent::processNewLoadedScript() {
  assert(
      asyncDebugger_.isWaitingForCommand() &&
      "Can only be called while paused");

  auto stackTrace = runtime_.getDebugger().getProgramState().getStackTrace();

  if (stackTrace.callFrameCount() > 0) {
    debugger::SourceLocation loc = stackTrace.callFrameForIndex(0).location;

    // Invalid fileId indicates debug info isn't included when compilation took
    // place. E.g. compiling to bytecode without -g.
    if (loc.fileId == debugger::kInvalidLocation) {
      return;
    }

    sendScriptParsedNotificationToClient(loc);
  }
}

bool DebuggerDomainAgent::checkDebuggerEnabled(const m::Request &req) {
  if (!enabled_) {
    sendResponseToClient(m::makeErrorResponse(
        req.id, m::ErrorCode::InvalidRequest, "Debugger domain not enabled"));
    return false;
  }
  return true;
}

bool DebuggerDomainAgent::checkDebuggerPaused(const m::Request &req) {
  if (!paused_) {
    sendResponseToClient(m::makeErrorResponse(
        req.id, m::ErrorCode::InvalidRequest, "Debugger is not paused"));
    return false;
  }
  return true;
}

} // namespace cdp
} // namespace hermes
} // namespace facebook
