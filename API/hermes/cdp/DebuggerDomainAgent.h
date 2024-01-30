/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_CDP_DEBUGGERDOMAINAGENT_H
#define HERMES_CDP_DEBUGGERDOMAINAGENT_H

#include <functional>
#include <string>

#include <hermes/AsyncDebuggerAPI.h>
#include <hermes/hermes.h>
#include <hermes/inspector/chrome/MessageConverters.h>
#include <hermes/inspector/chrome/RemoteObjectsTable.h>

#include "DomainAgent.h"

namespace facebook {
namespace hermes {
namespace cdp {

namespace m = ::facebook::hermes::inspector_modern::chrome::message;
namespace old_cdp = ::facebook::hermes::inspector_modern::chrome;

/// Handler for the "Debugger" domain of CDP. Accepts events from the runtime,
/// and CDP requests from the debug client belonging to the "Debugger" domain.
/// Produces CDP responses and events belonging to the "Debugger" domain. All
/// methods expect to be invoked with exclusive access to the runtime.
class DebuggerDomainAgent : public DomainAgent {
 public:
  DebuggerDomainAgent(
      HermesRuntime &runtime,
      debugger::AsyncDebuggerAPI &asyncDebugger,
      SynchronizedOutboundCallback messageCallback);
  ~DebuggerDomainAgent();

  /// Handles Debugger.enable request
  void enable(const m::debugger::EnableRequest &req);
  /// Handles Debugger.disable request
  void disable(const m::debugger::DisableRequest &req);

  /// Handles Debugger.pause request
  void pause(const m::debugger::PauseRequest &req);
  /// Handles Debugger.resume request
  void resume(const m::debugger::ResumeRequest &req);

  // Handles Debugger.stepInto request
  void stepInto(const m::debugger::StepIntoRequest &req);
  // Handles Debugger.stepOut request
  void stepOut(const m::debugger::StepOutRequest &req);
  // Handles Debugger.stepOver request
  void stepOver(const m::debugger::StepOverRequest &req);

  // Handles Debugger.setPauseOnExceptions
  void setPauseOnExceptions(
      const m::debugger::SetPauseOnExceptionsRequest &req);

 private:
  /// Fixed execution context ID because Hermes doesn't currently support realms
  /// or Web Workers.
  static constexpr int32_t kHermesExecutionContextId = 1;

  /// Handle an event originating from the runtime.
  void handleDebuggerEvent(
      HermesRuntime &runtime,
      debugger::AsyncDebuggerAPI &asyncDebugger,
      debugger::DebuggerEventType event);

  /// Send a Pause notification to the debug client with "other" being the
  /// reason
  void sendPausedNotificationToClient();
  /// Send a Pause notification to the debug client with "exception" being the
  /// reason
  void sendPauseOnExceptionNotificationToClient();
  /// Send a Debugger.scriptParsed notification to the debug client
  void sendScriptParsedNotificationToClient(
      const debugger::SourceLocation srcLoc);

  /// Obtain the newly loaded script and send a ScriptParsed notification to the
  /// debug client
  void processNewLoadedScript();

  bool checkDebuggerEnabled(const m::Request &req);
  bool checkDebuggerPaused(const m::Request &req);

  HermesRuntime &runtime_;
  debugger::AsyncDebuggerAPI &asyncDebugger_;

  /// ID for the registered DebuggerEventCallback
  debugger::DebuggerEventCallbackID debuggerEventCallbackId_;

  old_cdp::RemoteObjectsTable objTable_{};

  /// Whether Debugger.enable was received and wasn't disabled by receiving
  /// Debugger.disable
  bool enabled_;

  /// Whether to consider the debugger as currently paused. There are some
  /// debugger events such as ScriptLoaded where we don't consider the debugger
  /// to be paused.
  bool paused_;
};

} // namespace cdp
} // namespace hermes
} // namespace facebook

#endif // HERMES_CDP_DEBUGGERDOMAINAGENT_H
