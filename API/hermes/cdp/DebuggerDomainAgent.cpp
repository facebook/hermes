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
using namespace facebook::hermes::inspector_modern::chrome;

DebuggerDomainAgent::DebuggerDomainAgent(
    int32_t executionContextID,
    HermesRuntime &runtime,
    AsyncDebuggerAPI &asyncDebugger,
    SynchronizedOutboundCallback messageCallback,
    std::shared_ptr<RemoteObjectsTable> objTable)
    : DomainAgent(
          executionContextID,
          std::move(messageCallback),
          std::move(objTable)),
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
        objTable_->releaseObjectGroup(BacktraceObjectGroup);
        sendNotificationToClient(m::debugger::ResumedNotification{});
      }
      break;
    case DebuggerEventType::Breakpoint:
      if (breakpointsActive_) {
        paused_ = true;
        sendPausedNotificationToClient();
      } else {
        asyncDebugger_.resumeFromPaused(AsyncDebugCommand::Continue);
      }
      break;
    case DebuggerEventType::DebuggerStatement:
    case DebuggerEventType::StepFinish:
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

    // TODO: Add test for this once state persistence is implemented
    // Notify the client about all breakpoints in this script
    for (const auto &[cdpBreakpointID, cdpBreakpoint] : cdpBreakpoints_) {
      for (const HermesBreakpoint &hermesBreakpoint :
           cdpBreakpoint.hermesBreakpoints) {
        if (hermesBreakpoint.scriptID == srcLoc.fileId) {
          // This should have been checked before storing the Hermes
          // breakpoint in the CDP breakpoint.
          assert(
              hermesBreakpoint.breakpointID != debugger::kInvalidBreakpoint &&
              "Invalid breakpoint");
          debugger::BreakpointInfo breakpointInfo =
              runtime_.getDebugger().getBreakpointInfo(
                  hermesBreakpoint.breakpointID);
          if (!breakpointInfo.resolved) {
            // Resolved state changed between breakpoint creation and
            // notification
            assert(false && "Previously resolved breakpoint unresolved");
            continue;
          }

          m::debugger::BreakpointResolvedNotification resolved;
          resolved.breakpointId = std::to_string(cdpBreakpointID);
          resolved.location =
              m::debugger::makeLocation(breakpointInfo.resolvedLocation);
          sendNotificationToClient(resolved);
        }
      }
    }
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
  if (!checkDebuggerEnabled(req)) {
    return;
  }

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
              *objTable_,
              objectGroup,
              byValue,
              generatePreview);

          resp.result = std::move(*remoteObjPtr);
        }
        sendResponseToClient(resp);
      });
}

void DebuggerDomainAgent::setBreakpoint(
    const m::debugger::SetBreakpointRequest &req) {
  if (!checkDebuggerEnabled(req)) {
    return;
  }

  CDPBreakpointDescription description;
  description.line = req.location.lineNumber;
  description.column = req.location.columnNumber;
  description.condition = req.location.scriptId;

  auto scriptID = std::stoull(req.location.scriptId);

  // Create the Hermes breakpoint
  std::optional<HermesBreakpointLocation> hermesBreakpoint =
      createHermesBreakpont(
          static_cast<debugger::ScriptID>(scriptID), description);
  if (!hermesBreakpoint) {
    sendResponseToClient(m::makeErrorResponse(
        req.id, m::ErrorCode::ServerError, "Breakpoint creation failed"));
    return;
  }

  // Create the CDP breakpoint
  auto [breakpointID, breakpoint] = createCDPBreakpoint(
      std::move(description),
      HermesBreakpoint{
          hermesBreakpoint.value().id,
          static_cast<debugger::ScriptID>(scriptID)});

  // Send the response
  m::debugger::SetBreakpointResponse resp;
  resp.id = req.id;
  resp.breakpointId = std::to_string(breakpointID);
  resp.actualLocation =
      m::debugger::makeLocation(hermesBreakpoint.value().location);

  sendResponseToClient(resp);
}

void DebuggerDomainAgent::setBreakpointByUrl(
    const m::debugger::SetBreakpointByUrlRequest &req) {
  if (!checkDebuggerEnabled(req)) {
    return;
  }

  // TODO: getLocationByBreakpointRequest(req);
  // TODO: failure to parse
  if (!req.url.has_value()) {
    sendResponseToClient(m::makeErrorResponse(
        req.id,
        m::ErrorCode::InvalidRequest,
        "URL required; regex unsupported"));
    return;
  }

  // Create the CDP breakpoint
  CDPBreakpointDescription description;
  description.line = req.lineNumber;
  description.column = req.columnNumber;
  description.condition = req.condition;
  const std::string &url = req.url.value();
  description.url = url;
  auto [breakpointID, breakpoint] = createCDPBreakpoint(std::move(description));

  // Create the response
  m::debugger::SetBreakpointByUrlResponse resp;
  resp.id = req.id;
  resp.breakpointId = std::to_string(breakpointID);

  // Apply the breakpoint to all matching scripts that are already present,
  // populating the response with any successful applications.
  for (auto &srcLoc : runtime_.getDebugger().getLoadedScripts()) {
    if (srcLoc.fileName == url) {
      if (std::optional<HermesBreakpointLocation> hermesBreakpoint =
              applyBreakpoint(breakpoint, srcLoc.fileId)) {
        resp.locations.emplace_back(
            m::debugger::makeLocation(hermesBreakpoint.value().location));
      }
    }
  }
  sendResponseToClient(resp);
}

void DebuggerDomainAgent::removeBreakpoint(
    const m::debugger::RemoveBreakpointRequest &req) {
  if (!checkDebuggerEnabled(req)) {
    return;
  }

  auto cdpID = std::stoull(req.breakpointId);
  auto cdpBreakpoint = cdpBreakpoints_.find(cdpID);
  if (cdpBreakpoint == cdpBreakpoints_.end()) {
    sendResponseToClient(m::makeErrorResponse(
        req.id,
        m::ErrorCode::InvalidRequest,
        "Unknown breakpoint ID: " + req.breakpointId));
    return;
  }

  // Remove all the Hermes breakpoints implied by the CDP breakpoint
  for (HermesBreakpoint hermesBreakpoint :
       cdpBreakpoint->second.hermesBreakpoints) {
    runtime_.getDebugger().deleteBreakpoint(hermesBreakpoint.breakpointID);
  }

  // Remove the CDP breakpoint
  cdpBreakpoints_.erase(cdpBreakpoint);
  sendResponseToClient(m::makeOkResponse(req.id));
}

void DebuggerDomainAgent::setBreakpointsActive(
    const m::debugger::SetBreakpointsActiveRequest &req) {
  if (!checkDebuggerEnabled(req)) {
    return;
  }
  breakpointsActive_ = req.active;
  sendResponseToClient(m::makeOkResponse(req.id));
}

void DebuggerDomainAgent::sendPausedNotificationToClient() {
  m::debugger::PausedNotification note;
  note.reason = "other";
  note.callFrames = m::debugger::makeCallFrames(
      runtime_.getDebugger().getProgramState(), *objTable_, runtime_);
  sendNotificationToClient(note);
}

void DebuggerDomainAgent::sendPauseOnExceptionNotificationToClient() {
  m::debugger::PausedNotification note;
  note.reason = "exception";
  note.callFrames = m::debugger::makeCallFrames(
      runtime_.getDebugger().getProgramState(), *objTable_, runtime_);
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

    // Apply existing breakpoints to the new script.
    for (auto &[id, breakpoint] : cdpBreakpoints_) {
      if (loc.fileName == breakpoint.description.url) {
        auto breakpointInfo = applyBreakpoint(breakpoint, loc.fileId);
        if (breakpointInfo) {
          m::debugger::BreakpointResolvedNotification resolved;
          resolved.breakpointId = std::to_string(id);
          resolved.location =
              m::debugger::makeLocation(breakpointInfo.value().location);
          sendNotificationToClient(resolved);
        }
      }
    }
  }
}

/// Create a CDP breakpoint with a \p description of where to break, and
/// (optionally) a \p hermesBreakpointID that has already been applied.
/// Returns a pair containing the newly created breakpoint ID and value.
std::pair<unsigned int, CDPBreakpoint &>
DebuggerDomainAgent::createCDPBreakpoint(
    CDPBreakpointDescription &&description,
    std::optional<HermesBreakpoint> hermesBreakpoint) {
  unsigned int breakpointID = nextBreakpointID_++;
  CDPBreakpoint &breakpoint =
      cdpBreakpoints_.emplace(breakpointID, CDPBreakpoint(description))
          .first->second;

  if (hermesBreakpoint) {
    breakpoint.hermesBreakpoints.push_back(hermesBreakpoint.value());
  }

  return {breakpointID, breakpoint};
}

/// Attempt to create a breakpoint in the Hermes script identified by
/// \p scriptID at the location described by \p description.
std::optional<HermesBreakpointLocation>
DebuggerDomainAgent::createHermesBreakpont(
    debugger::ScriptID scriptID,
    const CDPBreakpointDescription &description) {
  // Convert the location description to a Hermes location
  debugger::SourceLocation hermesLoc;
  hermesLoc.fileId = scriptID;
  // CDP Locations are 0-based, Hermes lines/columns are 1-based
  hermesLoc.line = description.line + 1;

  if (description.column.has_value()) {
    if (description.column.value() == 0) {
      // TODO: When CDTP sends a column number of 0, we send Hermes a column
      // number of 1. For some reason, this causes Hermes to not be
      // able to resolve breakpoints.
      hermesLoc.column = debugger::kInvalidLocation;
    } else {
      hermesLoc.column = description.column.value() + 1;
    }
  }

  // Set the breakpoint in Hermes
  debugger::BreakpointID breakpointID =
      runtime_.getDebugger().setBreakpoint(hermesLoc);
  if (breakpointID == debugger::kInvalidBreakpoint) {
    return {};
  }

  debugger::BreakpointInfo info =
      runtime_.getDebugger().getBreakpointInfo(breakpointID);
  if (!info.resolved) {
    // Only accept immediately-resolvable breakpoints
    return {};
  }

  // Apply any break conditions
  if (description.condition) {
    runtime_.getDebugger().setBreakpointCondition(
        breakpointID, description.condition.value());
  }

  return HermesBreakpointLocation{breakpointID, info.resolvedLocation};
}

/// Apply a CDP breakpoint to a script, creating a Hermes breakpoint and
/// associating it with the specified CDP breakpoint. Returns the newly-
/// created Hermes breakpoint if successful, nullopt otherwise.
std::optional<HermesBreakpointLocation> DebuggerDomainAgent::applyBreakpoint(
    CDPBreakpoint &breakpoint,
    debugger::ScriptID scriptID) {
  // Create the Hermes breakpoint
  std::optional<HermesBreakpointLocation> hermesBreakpoint =
      createHermesBreakpont(scriptID, breakpoint.description);
  if (!hermesBreakpoint) {
    return {};
  }

  // Associate this Hermes breakpoint with the CDP breakpoint
  breakpoint.hermesBreakpoints.push_back(
      HermesBreakpoint{hermesBreakpoint.value().id, scriptID});

  return hermesBreakpoint;
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
