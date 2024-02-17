/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CDPAgent.h"
#include "DebuggerDomainAgent.h"
#include "ProfilerDomainAgent.h"
#include "RuntimeDomainAgent.h"

#include <hermes/inspector/chrome/MessageConverters.h>
#include <hermes/inspector/chrome/MessageTypes.h>

namespace facebook {
namespace hermes {
namespace cdp {

using namespace facebook::hermes::debugger;
using namespace facebook::hermes::inspector_modern::chrome;

/// Implementation of the CDP Agent. This class accepts CDP commands from
/// arbitrary threads and delivers them to the appropriate, domain-specific
/// message handler on the runtime thread. This class also manages the lifetime
/// of the domain-specific message handlers. Methods on this class can be
/// called from arbitrary threads.
class CDPAgentImpl {
 public:
  CDPAgentImpl(
      int32_t executionContextID,
      HermesRuntime &runtime,
      AsyncDebuggerAPI &asyncDebuggerAPI,
      EnqueueRuntimeTaskFunc enqueueRuntimeTaskCallback,
      SynchronizedOutboundCallback messageCallback);
  ~CDPAgentImpl();

  /// Schedule initialization of handlers for each message domain.
  void initializeDomainAgents();

  /// Process a CDP command encoded in \p json.
  void handleCommand(std::string json);

 private:
  /// Collection of domain-specific message handlers. These handlers require
  /// exclusive access to the runtime (whereas the CDP Agent can be used from)
  /// arbitrary threads), so all methods on this struct are expected to be
  /// called with exclusive access to the runtime.
  struct DomainAgents {
    // Create a new collection of domain agents.
    DomainAgents(
        int32_t executionContextID,
        HermesRuntime &runtime,
        AsyncDebuggerAPI &asyncDebuggerAPI,
        SynchronizedOutboundCallback messageCallback);

    /// Create the domain handlers and subscribing to any external events.
    void initialize();

    /// Releasing any domain handlers and event subscriptions.
    void dispose();

    /// Process a CDP \p command encoded in JSON using the appropriate domain
    /// handler.
    void handleCommand(std::shared_ptr<message::Request> command);

    /// Execution context ID associated with the HermesRuntime. This is used by
    /// domain agents when sending notifications to identify the runtime the
    /// notification is coming from.
    int32_t executionContextID_;
    HermesRuntime &runtime_;
    debugger::AsyncDebuggerAPI &asyncDebuggerAPI_;

    /// Callback function for sending CDP response back. Same as the one in
    /// CDPAgentImpl.
    SynchronizedOutboundCallback messageCallback_;

    // Collection of objects that can be referenced by the debug client.
    // Object IDs generated in one domain can be referenced from another,
    // so this collection is shared amongst them all. No locking is needed,
    // as it's guaranteed that there will never be multiple domain agents
    // running at the same time.
    std::shared_ptr<RemoteObjectsTable> objTable_;

    std::unique_ptr<DebuggerDomainAgent> debuggerAgent_;
    std::unique_ptr<RuntimeDomainAgent> runtimeAgent_;
    std::unique_ptr<ProfilerDomainAgent> profilerAgent_;
  };

  /// Callback function for sending CDP response back. This is using the
  /// SynchronizedOutboundCallback wrapper because we want to be able to
  /// guarantee the callback will never be used after CDPAgentImpl is
  /// destroyed. But since the callback is passed to other domain agents and
  /// being used on the runtime thread, we need a thread-safe way to deal with
  /// it.
  SynchronizedOutboundCallback messageCallback_;

  RuntimeTaskRunner runtimeTaskRunner_;
  std::shared_ptr<DomainAgents> domainAgents_;
};

CDPAgentImpl::CDPAgentImpl(
    int32_t executionContextID,
    HermesRuntime &runtime,
    debugger::AsyncDebuggerAPI &asyncDebuggerAPI,
    EnqueueRuntimeTaskFunc enqueueRuntimeTaskCallback,
    SynchronizedOutboundCallback messageCallback)
    : messageCallback_(std::move(messageCallback)),
      runtimeTaskRunner_(asyncDebuggerAPI, enqueueRuntimeTaskCallback),
      domainAgents_(std::make_shared<DomainAgents>(
          executionContextID,
          runtime,
          asyncDebuggerAPI,
          messageCallback_)) {}

CDPAgentImpl::~CDPAgentImpl() {
  // Call DomainAgents::dispose on the runtime thread, only keeping a copy of
  // the domainAgents shared pointer (as opposed to the usual shared "this"),
  // so the remainder of the CDP Agent is free to be released.
  // AsyncDebuggerAPI::triggerInterrupt (which is used by the
  // runtimeTaskRunner) guarantees the callback will run even if
  // AsyncDebuggerAPI is being destructed.
  // The runtimeTaskRunner is still alive at this point, so it can be used
  // to enqueue the task. enqueueTask will add the task to the relevant
  // execution queues before returning, with no lingering references to the
  // runtimeTaskRunner itself.
  runtimeTaskRunner_.enqueueTask(
      [domainAgents = domainAgents_](HermesRuntime &runtime) {
        domainAgents->dispose();
      });

  // Call invalidate() to prevent the OutboundMessageFunc from being used again
  // after this destructor.
  messageCallback_.invalidate();
}

void CDPAgentImpl::initializeDomainAgents() {
  // Call DomainAgents::initialize on the runtime thread.
  runtimeTaskRunner_.enqueueTask(
      [domainAgents = domainAgents_](HermesRuntime &runtime) {
        domainAgents->initialize();
      });
}

void CDPAgentImpl::handleCommand(std::string json) {
  std::shared_ptr<message::Request> command = message::Request::fromJson(json);
  if (!command) {
    // Can't even parse the command to get the command ID, so there's no ID
    // to respond to with an error message.
    // TODO: return an error message
    return;
  }

  // Call DomainAgents::handleCommand on the runtime thread.
  runtimeTaskRunner_.enqueueTask(
      [domainAgents = domainAgents_,
       command = std::move(command)](HermesRuntime &) {
        domainAgents->handleCommand(std::move(command));
      });
}

CDPAgentImpl::DomainAgents::DomainAgents(
    int32_t executionContextID,
    HermesRuntime &runtime,
    AsyncDebuggerAPI &asyncDebuggerAPI,
    SynchronizedOutboundCallback messageCallback)
    : executionContextID_(executionContextID),
      runtime_(runtime),
      asyncDebuggerAPI_(asyncDebuggerAPI),
      messageCallback_(std::move(messageCallback)),
      objTable_(std::make_shared<RemoteObjectsTable>()) {}

void CDPAgentImpl::DomainAgents::initialize() {
  debuggerAgent_ = std::make_unique<DebuggerDomainAgent>(
      executionContextID_,
      runtime_,
      asyncDebuggerAPI_,
      messageCallback_,
      objTable_);
  runtimeAgent_ = std::make_unique<RuntimeDomainAgent>(
      executionContextID_, runtime_, messageCallback_, objTable_);
  profilerAgent_ = std::make_unique<ProfilerDomainAgent>(
      executionContextID_, runtime_, messageCallback_, objTable_);
}

void CDPAgentImpl::DomainAgents::dispose() {
  debuggerAgent_.reset();
  runtimeAgent_.reset();
  profilerAgent_.reset();
}

void CDPAgentImpl::DomainAgents::handleCommand(
    std::shared_ptr<message::Request> command) {
  size_t domainLength = command->method.find('.');
  if (domainLength == std::string::npos) {
    messageCallback_(message::makeErrorResponse(
                         command->id,
                         message::ErrorCode::ParseError,
                         "Malformed domain '" + command->method + "'")
                         .toJsonStr());
    return;
  }
  std::string domain = command->method.substr(0, domainLength);

  // TODO: Do better dispatch
  if (command->method == "Debugger.enable") {
    debuggerAgent_->enable(static_cast<m::debugger::EnableRequest &>(*command));
  } else if (command->method == "Debugger.disable") {
    debuggerAgent_->disable(
        static_cast<m::debugger::DisableRequest &>(*command));
  } else if (command->method == "Debugger.pause") {
    debuggerAgent_->pause(static_cast<m::debugger::PauseRequest &>(*command));
  } else if (command->method == "Debugger.resume") {
    debuggerAgent_->resume(static_cast<m::debugger::ResumeRequest &>(*command));
  } else if (command->method == "Debugger.stepInto") {
    debuggerAgent_->stepInto(
        static_cast<m::debugger::StepIntoRequest &>(*command));
  } else if (command->method == "Debugger.stepOut") {
    debuggerAgent_->stepOut(
        static_cast<m::debugger::StepOutRequest &>(*command));
  } else if (command->method == "Debugger.stepOver") {
    debuggerAgent_->stepOver(
        static_cast<m::debugger::StepOverRequest &>(*command));
  } else if (command->method == "Debugger.setPauseOnExceptions") {
    debuggerAgent_->setPauseOnExceptions(
        static_cast<m::debugger::SetPauseOnExceptionsRequest &>(*command));
  } else if (command->method == "Debugger.evaluateOnCallFrame") {
    debuggerAgent_->evaluateOnCallFrame(
        static_cast<m::debugger::EvaluateOnCallFrameRequest &>(*command));
  } else if (command->method == "Debugger.setBreakpoint") {
    debuggerAgent_->setBreakpoint(
        static_cast<m::debugger::SetBreakpointRequest &>(*command));
  } else if (command->method == "Debugger.setBreakpointByUrl") {
    debuggerAgent_->setBreakpointByUrl(
        static_cast<m::debugger::SetBreakpointByUrlRequest &>(*command));
  } else if (command->method == "Debugger.removeBreakpoint") {
    debuggerAgent_->removeBreakpoint(
        static_cast<m::debugger::RemoveBreakpointRequest &>(*command));
  } else if (command->method == "Debugger.setBreakpointsActive") {
    debuggerAgent_->setBreakpointsActive(
        static_cast<m::debugger::SetBreakpointsActiveRequest &>(*command));
  } else if (command->method == "Runtime.enable") {
    runtimeAgent_->enable(static_cast<m::runtime::EnableRequest &>(*command));
  } else if (command->method == "Runtime.disable") {
    runtimeAgent_->disable(static_cast<m::runtime::DisableRequest &>(*command));
  } else if (command->method == "Runtime.getHeapUsage") {
    runtimeAgent_->getHeapUsage(
        static_cast<m::runtime::GetHeapUsageRequest &>(*command));
  } else if (command->method == "Runtime.globalLexicalScopeNames") {
    runtimeAgent_->globalLexicalScopeNames(
        static_cast<m::runtime::GlobalLexicalScopeNamesRequest &>(*command));
  } else if (command->method == "Runtime.compileScript") {
    runtimeAgent_->compileScript(
        static_cast<m::runtime::CompileScriptRequest &>(*command));
  } else if (command->method == "Runtime.getProperties") {
    runtimeAgent_->getProperties(
        static_cast<m::runtime::GetPropertiesRequest &>(*command));
  } else if (command->method == "Runtime.evaluate") {
    runtimeAgent_->evaluate(
        static_cast<m::runtime::EvaluateRequest &>(*command));
  } else if (command->method == "Profiler.start") {
    profilerAgent_->start(static_cast<m::profiler::StartRequest &>(*command));
  } else if (command->method == "Profiler.stop") {
    profilerAgent_->stop(static_cast<m::profiler::StopRequest &>(*command));
  } else {
    messageCallback_(message::makeErrorResponse(
                         command->id,
                         message::ErrorCode::MethodNotFound,
                         "Unsupported domain '" + command->method + "'")
                         .toJsonStr());
  }
}

std::unique_ptr<CDPAgent> CDPAgent::create(
    int32_t executionContextID,
    HermesRuntime &runtime,
    debugger::AsyncDebuggerAPI &asyncDebuggerAPI,
    EnqueueRuntimeTaskFunc enqueueRuntimeTaskCallback,
    OutboundMessageFunc messageCallback) {
  return std::unique_ptr<CDPAgent>(new CDPAgent(
      executionContextID,
      runtime,
      asyncDebuggerAPI,
      enqueueRuntimeTaskCallback,
      messageCallback));
}

CDPAgent::CDPAgent(
    int32_t executionContextID,
    HermesRuntime &runtime,
    debugger::AsyncDebuggerAPI &asyncDebuggerAPI,
    EnqueueRuntimeTaskFunc enqueueRuntimeTaskCallback,
    OutboundMessageFunc messageCallback)
    : impl_(std::make_unique<CDPAgentImpl>(
          executionContextID,
          runtime,
          asyncDebuggerAPI,
          enqueueRuntimeTaskCallback,
          SynchronizedOutboundCallback(messageCallback))) {
  impl_->initializeDomainAgents();
}

CDPAgent::~CDPAgent() {}

void CDPAgent::handleCommand(std::string json) {
  impl_->handleCommand(json);
}

} // namespace cdp
} // namespace hermes
} // namespace facebook
