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
      OutboundMessageFunc messageCallback);
  ~DebuggerDomainAgent();

  /// Handles Debugger.enable request
  void enable(const m::debugger::EnableRequest &req);
  /// Handles Debugger.disable request
  void disable(const m::debugger::DisableRequest &req);

 private:
  /// Fixed execution context ID because Hermes doesn't currently support realms
  /// or Web Workers.
  static constexpr int32_t kHermesExecutionContextId = 1;

  /// Handle an event originating from the runtime.
  void handleDebuggerEvent(
      HermesRuntime &runtime,
      debugger::AsyncDebuggerAPI &asyncDebugger,
      debugger::DebuggerEventType event);

  /// Send a notification to the debug client
  void sendPausedNotificationToClient();
  /// Send a Debugger.scriptParsed notification to the debug client
  void sendScriptParsedNotificationToClient(
      const debugger::SourceLocation srcLoc);

  /// Obtain the newly loaded script and send a ScriptParsed notification to the
  /// debug client
  void processNewLoadedScript();

  HermesRuntime &runtime_;
  debugger::AsyncDebuggerAPI &asyncDebugger_;

  /// ID for the registered DebuggerEventCallback
  debugger::DebuggerEventCallbackID debuggerEventCallbackId_;

  old_cdp::RemoteObjectsTable objTable_{};

  /// Whether Debugger.enable was received and wasn't disabled by receiving
  /// Debugger.disable
  bool enabled_;
};

} // namespace cdp
} // namespace hermes
} // namespace facebook

#endif // HERMES_CDP_DEBUGGERDOMAINAGENT_H
