/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "DebuggerDomainAgent.h"

#include <hermes/Regex/Executor.h>
#include <hermes/Regex/Regex.h>
#include <hermes/Regex/RegexTraits.h>
#include <hermes/Support/UTF8.h>
#include <hermes/cdp/RemoteObjectConverters.h>

namespace facebook {
namespace hermes {
namespace cdp {

using namespace facebook::hermes::debugger;

static const char *const kBreakpointsKey = "breakpoints";
static const char *const kBreakpointsActiveKey = "breakpointsActive";

enum class PausedNotificationReason { kException, kOther, kStep };

DebuggerDomainAgent::DebuggerDomainAgent(
    int32_t executionContextID,
    HermesRuntime &runtime,
    AsyncDebuggerAPI &asyncDebugger,
    SynchronizedOutboundCallback messageCallback,
    std::shared_ptr<RemoteObjectsTable> objTable,
    DomainState &state)
    : DomainAgent(
          executionContextID,
          std::move(messageCallback),
          std::move(objTable)),
      runtime_(runtime),
      asyncDebugger_(asyncDebugger),
      debuggerEventCallbackId_(kInvalidDebuggerEventCallbackID),
      state_(state),
      enabled_(false),
      paused_(false) {
  std::unique_ptr<StateValue> breakpointsActiveStateValue =
      state_.getCopy({kBreakpointsActiveKey});
  if (breakpointsActiveStateValue) {
    BooleanStateValue *boolStateValue =
        dynamic_cast<BooleanStateValue *>(breakpointsActiveStateValue.get());
    breakpointsActive_ = boolStateValue->value;
  } else {
    // If the flag is not persisted in the state, then we default to true
    // https://source.chromium.org/chromium/chromium/src/+/main:v8/src/inspector/v8-debugger-agent-impl.cc;l=450-454;drc=27d34700b83f381c62e3a348de2e6dfdc08364b8
    breakpointsActive_ = true;
  }

  std::unique_ptr<StateValue> value = state_.getCopy({kBreakpointsKey});
  if (value) {
    DictionaryStateValue *dict =
        dynamic_cast<DictionaryStateValue *>(value.get());
    assert(
        dict != nullptr &&
        "Expecting a DictionaryStateValue for the \"breakpoints\" key");
    for (auto &[key, stateValue] : dict->values) {
      CDPBreakpointDescription *bpStateValue =
          dynamic_cast<CDPBreakpointDescription *>(stateValue.get());
      assert(
          dict != nullptr &&
          "Expecting a CDPBreakpointDescription for each breakpoint");

      CDPBreakpointID id = std::stoull(key);
      cdpBreakpoints_.emplace(id, CDPBreakpoint(*bpStateValue));

      // Ensure we don't re-use persisted breakpoint IDs; advance the ID counter
      // past any imported breakpoints.
      if (id >= nextBreakpointID_) {
        nextBreakpointID_ = id + 1;
      }
    }
  }
}

DebuggerDomainAgent::~DebuggerDomainAgent() {
  cleanUp();
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
      if (isTopFrameLocationBlackboxed()) {
        asyncDebugger_.resumeFromPaused(
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
        asyncDebugger_.resumeFromPaused(
            explicitPausePending_ ? AsyncDebugCommand::StepInto
                                  : AsyncDebugCommand::Continue);
      }
      break;
    case DebuggerEventType::DebuggerStatement:
      if (isTopFrameLocationBlackboxed()) {
        asyncDebugger_.resumeFromPaused(AsyncDebugCommand::Continue);
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
                (*lastUserStepRequest_ == LastUserStepRequest::StepOver ||
                 *lastUserStepRequest_ == LastUserStepRequest::StepOut)
            ? AsyncDebugCommand::StepOut
            : AsyncDebugCommand::StepInto;

        asyncDebugger_.resumeFromPaused(nextStep);
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
        asyncDebugger_.resumeFromPaused(AsyncDebugCommand::StepInto);
      } else {
        setPaused(PausedNotificationReason::kOther);
      }
      break;
    default:
      assert(false && "unknown DebuggerEventType");
      asyncDebugger_.resumeFromPaused(AsyncDebugCommand::Continue);
  }
}

void DebuggerDomainAgent::setPaused(
    PausedNotificationReason pausedNotificationReason) {
  paused_ = true;
  explicitPausePending_ = false;
  sendPausedNotificationToClient(pausedNotificationReason);
}

void DebuggerDomainAgent::setUnpaused() {
  paused_ = false;
  objTable_->releaseObjectGroup(BacktraceObjectGroup);
  sendNotificationToClient(m::debugger::ResumedNotification{});
}

void DebuggerDomainAgent::enable() {
  if (enabled_) {
    return;
  }
  enabled_ = true;

#ifndef NDEBUG
  for (const auto &[_, cdpBreakpoint] : cdpBreakpoints_) {
    assert(
        cdpBreakpoint.hermesBreakpoints.empty() &&
        "Unexpected linked internal breakpoint");
  }
#endif

  // The debugger just got enabled; inform the client about all scripts.
  for (auto &srcLoc : runtime_.getDebugger().getLoadedScripts()) {
    sendScriptParsedNotificationToClient(srcLoc);

    for (auto &[cdpBreakpointID, cdpBreakpoint] : cdpBreakpoints_) {
      if (srcLoc.fileName == cdpBreakpoint.description.url) {
        applyBreakpointAndSendNotification(
            cdpBreakpointID, cdpBreakpoint, srcLoc);
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
    setPaused(PausedNotificationReason::kOther);
  }
}

void DebuggerDomainAgent::enable(const m::debugger::EnableRequest &req) {
  // Match V8 behavior of returning success even if domain is already enabled
  enable();
  sendResponseToClient(m::makeOkResponse(req.id));
}

void DebuggerDomainAgent::cleanUp() {
  // This doesn't support other debug clients also setting breakpoints. If we
  // need that functionality, then we might need to track breakpoints set by
  // this client and only remove those.
  runtime_.getDebugger().deleteAllBreakpoints();

  // Unlink all persisted CDPBreakpoints from their HermesBreakpoints, in case
  // domain will be re-enabled. This is required, because all internal Hermes
  // breakpoints were deleted above.
  for (auto &[_, cdpBreakpoint] : cdpBreakpoints_) {
    cdpBreakpoint.hermesBreakpoints.clear();
  }

  if (debuggerEventCallbackId_ != kInvalidDebuggerEventCallbackID) {
    asyncDebugger_.removeDebuggerEventCallback_TS(debuggerEventCallbackId_);
    debuggerEventCallbackId_ = kInvalidDebuggerEventCallbackID;
  }

  // This doesn't work well if there are other debug clients that also toggle
  // this flag. If we need that functionality, then DebuggerAPI needs to be
  // changed.
  runtime_.getDebugger().setShouldPauseOnScriptLoad(false);
}

void DebuggerDomainAgent::disable(const m::debugger::DisableRequest &req) {
  if (enabled_) {
    cleanUp();
  }
  enabled_ = false;
  sendResponseToClient(m::makeOkResponse(req.id));
}

void DebuggerDomainAgent::pause(const m::debugger::PauseRequest &req) {
  if (!checkDebuggerEnabled(req)) {
    return;
  }

  explicitPausePending_ = true;

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

  lastUserStepRequest_ = LastUserStepRequest::StepInto;

  asyncDebugger_.resumeFromPaused(AsyncDebugCommand::StepInto);
  sendResponseToClient(m::makeOkResponse(req.id));
}

void DebuggerDomainAgent::stepOut(const m::debugger::StepOutRequest &req) {
  if (!checkDebuggerPaused(req)) {
    return;
  }

  lastUserStepRequest_ = LastUserStepRequest::StepOut;

  asyncDebugger_.resumeFromPaused(AsyncDebugCommand::StepOut);
  sendResponseToClient(m::makeOkResponse(req.id));
}

void DebuggerDomainAgent::stepOver(const m::debugger::StepOverRequest &req) {
  if (!checkDebuggerPaused(req)) {
    return;
  }

  lastUserStepRequest_ = LastUserStepRequest::StepOver;

  asyncDebugger_.resumeFromPaused(AsyncDebugCommand::StepOver);
  sendResponseToClient(m::makeOkResponse(req.id));
}

static inline bool blackboxRangeComparator(
    const std::pair<int, int> &a,
    const std::pair<int, int> &b) {
  if (a.first != b.first)
    return a.first < b.first;
  return a.second < b.second;
}

void DebuggerDomainAgent::setBlackboxPatterns(
    const m::debugger::SetBlackboxPatternsRequest &req) {
  blackboxAnonymousScripts_ = req.skipAnonymous.value_or(false);

  if (req.patterns.empty()) {
    compiledBlackboxPatternRegex_ = std::nullopt;
    sendResponseToClient(m::makeOkResponse(req.id));
    return;
  }

  std::string combinedPattern = u8"(";
  for (auto &pattern : req.patterns) {
    combinedPattern.append(pattern + u8"|");
  }
  combinedPattern.back() = u8')';

  // We expect req.patterns to be encoded as UTF-8 in accordance with RFC-8259
  // See comment in CDPAgent::handleCommand
  std::vector<char16_t> combinedPatternUTF16;
  ::hermes::convertUTF8WithSurrogatesToUTF16(
      std::back_inserter(combinedPatternUTF16),
      combinedPattern.data(),
      combinedPattern.data() + combinedPattern.size());

  ::hermes::regex::Regex<::hermes::regex::UTF16RegexTraits> blackboxPatternRegex{
      combinedPatternUTF16,
      // Use empty regex flags (not ignore case, not multiline). See
      // https://source.chromium.org/chromium/chromium/src/+/41d42cec13ea567f461d98dfb04d641e30d6bb5b:v8/src/inspector/v8-debugger-agent-impl.cc;l=1726-1727;
      {}};

  // We expect the client to handle the regex validation on patterns before
  // sending it to us.
  // If it's not valid, we respond with an error allowing the client to handle
  // that situation.
  if (!blackboxPatternRegex.valid()) {
    sendResponseToClient(m::makeErrorResponse(
        req.id, m::ErrorCode::InvalidParams, "Invalid regex pattern"));
    return;
  }

  compiledBlackboxPatternRegex_ = blackboxPatternRegex.compile();

  sendResponseToClient(m::makeOkResponse(req.id));
}

void DebuggerDomainAgent::setBlackboxedRanges(
    const m::debugger::SetBlackboxedRangesRequest &req) {
  auto scriptID = std::stoull(req.scriptId);

  if (req.positions.empty()) {
    blackboxedRanges_.erase(scriptID);
    sendResponseToClient(m::makeOkResponse(req.id));
    return;
  }

  auto blackboxedRanges = std::vector<std::pair<int, int>>();
  for (auto &position : req.positions) {
    blackboxedRanges.push_back(std::pair<int, int>(
        /* cdp line and column numbers are 0 based */
        position.lineNumber + 1,
        position.columnNumber + 1));
  }

  assert(std::is_sorted(
      blackboxedRanges.begin(),
      blackboxedRanges.end(),
      blackboxRangeComparator));

  blackboxedRanges_[scriptID] = std::move(blackboxedRanges);

  sendResponseToClient(m::makeOkResponse(req.id));
}

bool DebuggerDomainAgent::isLocationBlackboxed(
    ScriptID scriptID,
    std::string scriptName,
    int lineNumber,
    int columnNumber) {
  if (blackboxAnonymousScripts_ && scriptName.empty()) {
    return true;
  }

  if (compiledBlackboxPatternRegex_.has_value()) {
    // We expect scriptName to be encoded as UTF-8 in accordance with RFC-8259
    // See comment in CDPAgent::handleCommand
    std::vector<char16_t> scriptNameUTF16;
    ::hermes::convertUTF8WithSurrogatesToUTF16(
        std::back_inserter(scriptNameUTF16),
        scriptName.data(),
        scriptName.data() + scriptName.size());

    uint32_t searchStart = 0;
    std::vector<::hermes::regex::CapturedRange> captures{};
    ::hermes::regex::MatchRuntimeResult matchResult =
        ::hermes::regex::searchWithBytecode(
            *compiledBlackboxPatternRegex_,
            scriptNameUTF16.data(),
            searchStart,
            scriptNameUTF16.size(),
            &captures,
            ::hermes::regex::constants::MatchFlagType::matchDefault);
    if (matchResult == ::hermes::regex::MatchRuntimeResult::Match) {
      return true;
    }
  }

  if (blackboxedRanges_.count(scriptID) == 0) {
    return false;
  }

  auto const &positions = blackboxedRanges_.at(scriptID);

  auto locationPosition = std::lower_bound(
      positions.begin(),
      positions.end(),
      std::make_pair(lineNumber, columnNumber),
      blackboxRangeComparator);

  // Since locationPosition is the first position that is *_NOT BEFORE_* the
  // passed location, for a location that falls within a blackboxed range, that
  // position's index would be odd. See the comment for blackboxedRanges_
  // For example, for blackboxedRanges_ [[1,1],[4,4]], the location [2,2]
  // would be correctly blackboxed because locationPosition would hold [4,4]
  // which is on index 1
  return std::distance(positions.begin(), locationPosition) % 2 == 1;
}

bool DebuggerDomainAgent::isTopFrameLocationBlackboxed() {
  auto stackTrace = runtime_.getDebugger().getProgramState().getStackTrace();
  if (stackTrace.callFrameCount() < 1) {
    return false;
  }
  debugger::SourceLocation loc = stackTrace.callFrameForIndex(0).location;
  if (breakpointsActive_) {
    for (const auto &[cdpBreakpointID, cdpBreakpoint] : cdpBreakpoints_) {
      for (const HermesBreakpoint &hermesBreakpoint :
           cdpBreakpoint.hermesBreakpoints) {
        auto breakpointLoc =
            runtime_.getDebugger()
                .getBreakpointInfo(hermesBreakpoint.breakpointID)
                .resolvedLocation;

        auto locationHasManualBreakpoint =
            (loc.fileId == breakpointLoc.fileId &&
             loc.line == breakpointLoc.line &&
             loc.column == breakpointLoc.column);
        // Locations with manual breakpoints are not considered blackboxed.
        // For example, if a user steps inside a blackboxed function, if any of
        // the next lines in that function have a manual breakpoint, we should
        // respect them and stop on them rather than stepping over them. The
        // same logic applies to explicit pauses. While they trigger stepping
        // into until out of blackboxed ranges, or until the program continues
        // execution if one of these steps lands on a line with a manual
        // breakpoint, we should stop on it.
        if (locationHasManualBreakpoint) {
          return false;
        }
      }
    }
  }
  return isLocationBlackboxed(loc.fileId, loc.fileName, loc.line, loc.column);
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

  // Copy members of req before returning from this function.
  long long reqId = req.id;
  std::string objectGroup = req.objectGroup.value_or("");
  ObjectSerializationOptions serializationOptions;
  serializationOptions.returnByValue = req.returnByValue.value_or(false);
  serializationOptions.generatePreview = req.generatePreview.value_or(false);

  uint32_t frameIndex = (uint32_t)atoi(req.callFrameId.c_str());
  asyncDebugger_.evalWhilePaused(
      req.expression,
      frameIndex,
      [reqId,
       objectGroup = std::move(objectGroup),
       serializationOptions = std::move(serializationOptions),
       this](HermesRuntime &runtime, const debugger::EvalResult &result) {
        m::debugger::EvaluateOnCallFrameResponse resp;
        resp.id = reqId;
        if (result.isException) {
          resp.exceptionDetails = m::runtime::makeExceptionDetails(
              runtime, result, *objTable_, objectGroup);
          // In V8, @cdp Debugger.evaluateOnCallFrame populates the `result`
          // field with the exception value.
          resp.result = m::runtime::makeRemoteObjectForError(
              runtime, result.value, *objTable_, objectGroup);
        } else {
          resp.result = m::runtime::makeRemoteObject(
              runtime,
              result.value,
              *objTable_,
              objectGroup,
              serializationOptions);
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
      createHermesBreakpoint(
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

  // Get a Transaction, which will be committed when exiting the scope of the
  // function. All state modifications should be done within this Transaction.
  DomainState::Transaction transaction = state_.transaction();
  transaction.remove({kBreakpointsKey, req.breakpointId});

  sendResponseToClient(m::makeOkResponse(req.id));
}

void DebuggerDomainAgent::setBreakpointsActive(
    const m::debugger::SetBreakpointsActiveRequest &req) {
  // DomainState modifications are commited on Transaction destruction. With
  // sendResponseToClient being called prior to scope exit, in the test
  // environment this could cause a race condition.
  {
    // We don't check for `enabled_` here because V8 allows
    // `setBreakpointsActive` to be called while debugger is disabled:
    // https://source.chromium.org/chromium/chromium/src/+/main:v8/src/inspector/v8-debugger-agent-impl.cc;l=562-563;drc=db2ef55b78602346f67f7f015ec6ebb9e554d228
    BooleanStateValue breakpointsActiveStateValue;
    breakpointsActiveStateValue.value = req.active;
    DomainState::Transaction transaction = state_.transaction();
    transaction.add({kBreakpointsActiveKey}, breakpointsActiveStateValue);
  }

  breakpointsActive_ = req.active;
  sendResponseToClient(m::makeOkResponse(req.id));
}

void DebuggerDomainAgent::sendPausedNotificationToClient(
    PausedNotificationReason reason) {
  m::debugger::PausedNotification note;
  switch (reason) {
    case PausedNotificationReason::kException: {
      note.reason = "exception";

      // Although the documentation lists the "data" field as optional for the
      // @cdp Debugger.paused event:
      // https://chromedevtools.github.io/devtools-protocol/tot/Debugger/#event-paused
      // it is accessed unconditionally by the front-end when the pause reason
      // is "exception". "data" is passed in as "auxData" via:
      // https://github.com/facebookexperimental/rn-chrome-devtools-frontend/blob/9a23d4c7c4c2d1a3d9e913af38d6965f474c4284/front_end/core/sdk/DebuggerModel.ts#L994
      // and "auxData" stored in a DebuggerPausedDetails instance:
      // https://github.com/facebookexperimental/rn-chrome-devtools-frontend/blob/9a23d4c7c4c2d1a3d9e913af38d6965f474c4284/front_end/core/sdk/DebuggerModel.ts#L642
      // which then has its fields accessed in:
      // https://github.com/facebookexperimental/rn-chrome-devtools-frontend/blob/main/front_end/panels/sources/DebuggerPausedMessage.ts#L225
      // If the "data" ("auxData") object is absent, accessing its fields will
      // throw, breaking the display of pause information. Thus, we always
      // populate "data" with an object. The "data" field has no schema in the
      // protocol metadata that we use to generate message structures, so we
      // need to manually construct a JSON object here. The structure expected
      // by the front-end (specifically, the "description" field) can be
      // inferred from the field access at the URL above. The front-end does
      // gracefully handle missing fields on the "data" object, so we can
      // consider the "description" field optional.
      std::string data;
      llvh::raw_string_ostream dataStream{data};
      ::hermes::JSONEmitter dataJson{dataStream};
      dataJson.openDict();

      jsi::Value thrownValue = runtime_.getDebugger().getThrownValue();
      if (!thrownValue.isUndefined()) {
        std::string description = thrownValue.toString(runtime_).utf8(runtime_);
        dataJson.emitKeyValue("description", description);
      } else {
        // No exception description to report, but emitting an empty "data"
        // object will at least prevent the front-end from throwing.
        assert(false && "Exception pause missing thrown value");
      }

      dataJson.closeDict();
      dataStream.flush();
      note.data = std::move(data);
    } break;
    case PausedNotificationReason::kOther:
      note.reason = "other";
      break;
    case PausedNotificationReason::kStep:
      note.reason = "step";
      break;
    default:
      assert(false && "unknown PausedNotificationReason");
      note.reason = "other";
  }
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
  // Either JavaScript or WebAssembly. See:
  // https://source.chromium.org/chromium/chromium/src/+/main:v8/src/inspector/v8-debugger-agent-impl.cc;l=1875-1882
  note.scriptLanguage = "JavaScript";
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
        applyBreakpointAndSendNotification(id, breakpoint, loc);
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

  // Get a Transaction, which will be committed when exiting the scope of the
  // function. All state modifications should be done within this Transaction.
  DomainState::Transaction transaction = state_.transaction();
  if (description.persistable()) {
    transaction.add(
        {kBreakpointsKey, std::to_string(breakpointID)}, description);
  }

  return {breakpointID, breakpoint};
}

/// Attempt to create a breakpoint in the Hermes script identified by
/// \p scriptID at the location described by \p description.
std::optional<HermesBreakpointLocation>
DebuggerDomainAgent::createHermesBreakpoint(
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

/// Applies a CDP breakpoint to a script. If successful, will send
/// Debugger.breakpointResolved notification to client.
void DebuggerDomainAgent::applyBreakpointAndSendNotification(
    CDPBreakpointID cdpBreakpointID,
    CDPBreakpoint &cdpBreakpoint,
    const debugger::SourceLocation &srcLoc) {
  auto hermesBreakpointLoc = applyBreakpoint(cdpBreakpoint, srcLoc.fileId);
  if (hermesBreakpointLoc) {
    m::debugger::BreakpointResolvedNotification resolved;
    resolved.breakpointId = std::to_string(cdpBreakpointID);
    resolved.location =
        m::debugger::makeLocation(hermesBreakpointLoc.value().location);
    sendNotificationToClient(resolved);
  }
}

/// Apply a CDP breakpoint to a script, creating a Hermes breakpoint and
/// associating it with the specified CDP breakpoint. Returns the newly-
/// created Hermes breakpoint if successful, nullopt otherwise.
std::optional<HermesBreakpointLocation> DebuggerDomainAgent::applyBreakpoint(
    CDPBreakpoint &cdpBreakpoint,
    debugger::ScriptID scriptID) {
  // Create the Hermes breakpoint
  std::optional<HermesBreakpointLocation> hermesBreakpoint =
      createHermesBreakpoint(scriptID, cdpBreakpoint.description);
  if (!hermesBreakpoint) {
    return {};
  }

  // Associate this Hermes breakpoint with the CDP breakpoint
  cdpBreakpoint.hermesBreakpoints.push_back(
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
  if (!paused_ && !asyncDebugger_.isWaitingForCommand()) {
    sendResponseToClient(m::makeErrorResponse(
        req.id, m::ErrorCode::InvalidRequest, "Debugger is not paused"));
    return false;
  }
  return true;
}

} // namespace cdp
} // namespace hermes
} // namespace facebook
