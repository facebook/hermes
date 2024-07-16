/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CDPAgent.h"
#include "CDPDebugAPI.h"
#include "ConsoleMessage.h"
#include "DebuggerDomainAgent.h"
#include "HeapProfilerDomainAgent.h"
#include "ProfilerDomainAgent.h"
#include "RuntimeDomainAgent.h"

#include <hermes/cdp/MessageConverters.h>
#include <hermes/cdp/MessageTypes.h>

namespace facebook {
namespace hermes {
namespace cdp {

using namespace facebook::hermes::debugger;

/// Internal state that can be exported from a CDPAgent and imported into
/// another.
struct State::Private {
  /// Create a new state object, storing any values that should be persisted
  /// across sessions.
  explicit Private(std::unique_ptr<DomainState> debuggerState)
      : debuggerAgentState(std::move(debuggerState)) {
    assert(debuggerAgentState != nullptr && "State cannot be null");
  }

  std::unique_ptr<DomainState> debuggerAgentState;
};

State::State() : privateState_(nullptr) {}

State::State(std::unique_ptr<Private> privateState)
    : privateState_(std::move(privateState)) {}

State::State(State &&other) noexcept = default;

State &State::operator=(State &&other) noexcept = default;

State::~State() = default;

/// Determine which queues should be used to handle a CDP message, based
/// on the \p method of the message.
static TaskQueues messageTaskQueues(const std::string &method) {
  if (method == "Runtime.evaluate") {
    return TaskQueues::Integrator;
  }

  return TaskQueues::All;
}

/// Implementation of the CDP Agent. This class accepts CDP commands from
/// arbitrary threads and delivers them to the appropriate, domain-specific
/// message handler on the runtime thread. This class also manages the lifetime
/// of the domain-specific message handlers. Methods on this class can be
/// called from arbitrary threads.
class CDPAgentImpl {
 public:
  CDPAgentImpl(
      int32_t executionContextID,
      CDPDebugAPI &cdpDebugAPI,
      EnqueueRuntimeTaskFunc enqueueRuntimeTaskCallback,
      SynchronizedOutboundCallback messageCallback,
      State &state,
      std::shared_ptr<std::atomic_bool> destroyedDomainAgentsImpl);
  ~CDPAgentImpl();

  /// Schedule initialization of handlers for each message domain.
  void initializeDomainAgents();

  /// Process a CDP command encoded in \p json.
  void handleCommand(std::string json);

  /// Enable the Runtime domain without processing a CDP command or sending a
  /// CDP response.
  void enableRuntimeDomain();

  /// Enable the Debugger domain without processing a CDP command or sending a
  /// CDP response.
  void enableDebuggerDomain();

  /// Extract state to be persisted across reloads.
  State getState();

 private:
  /// Collection of domain-specific message handlers. These handlers require
  /// exclusive access to the runtime (whereas the CDP Agent can be used from)
  /// arbitrary threads), so all methods on this struct are expected to be
  /// called with exclusive access to the runtime.
  struct DomainAgentsImpl {
    // Create a new collection of domain agents.
    DomainAgentsImpl(
        int32_t executionContextID,
        HermesRuntime &runtime,
        debugger::AsyncDebuggerAPI &asyncDebuggerAPI,
        ConsoleMessageStorage &consoleMessageStorage,
        ConsoleMessageDispatcher &consoleMessageDispatcher,
        SynchronizedOutboundCallback messageCallback,
        std::unique_ptr<DomainState> debuggerAgentState,
        std::shared_ptr<std::atomic_bool> destroyedDomainAgentsImpl);
    ~DomainAgentsImpl();

    /// Create the domain handlers and subscribing to any external events.
    void initialize();

    /// Process a CDP \p command encoded in JSON using the appropriate domain
    /// handler.
    void handleCommand(std::shared_ptr<message::Request> command);

    /// Enable the Runtime domain without processing a CDP command or sending a
    /// CDP response.
    void enableRuntimeDomain();

    /// Enable the Debugger domain without processing a CDP command or sending a
    /// CDP response.
    void enableDebuggerDomain();

    /// Get the Debugger domain state to be persisted.
    std::unique_ptr<DomainState> getDebuggerAgentState();

   private:
    /// Execution context ID associated with the HermesRuntime. This is used by
    /// domain agents when sending notifications to identify the runtime the
    /// notification is coming from.
    int32_t executionContextID_;
    HermesRuntime &runtime_;
    debugger::AsyncDebuggerAPI &asyncDebuggerAPI_;
    ConsoleMessageStorage &consoleMessageStorage_;
    ConsoleMessageDispatcher &consoleMessageDispatcher_;

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
    std::unique_ptr<HeapProfilerDomainAgent> heapProfilerAgent_;

    std::unique_ptr<DomainState> debuggerAgentState_;

    std::shared_ptr<std::atomic_bool> destroyedDomainAgentsImpl_;
  };

  /// Wrapper for DomainAgentsImpl. This is used to ensure the entirety of
  /// DomainAgentsImpl gets cleaned up on the runtime thread. Since
  /// DomainAgentsImpl and domain agents might hold onto JSI values, via
  /// RemoteObjectsTable, we want to be sure that everything is cleaned up prior
  /// to the HermesRuntime going away.
  struct DomainAgents {
    // Create a new collection of domain agents.
    DomainAgents(
        int32_t executionContextID,
        CDPDebugAPI &cdpDebugAPI,
        SynchronizedOutboundCallback messageCallback,
        std::unique_ptr<DomainState> debuggerAgentState,
        std::shared_ptr<std::atomic_bool> destroyedDomainAgentsImpl);

    /// Forwards the call to the underlying DomainAgentsImpl.
    void initialize();

    /// Releasing any domain handlers and event subscriptions.
    void dispose();

    /// Get the Debugger domain state to be persisted.
    std::unique_ptr<DomainState> getDebuggerAgentState();

    /// Whenever we need to schedule tasks with RuntimeTaskRunner, we must only
    /// pass weak pointer of DomainAgentsImpl and not a strong reference. Using
    /// a weak pointer will not prevent DomainAgents::dispose() from destroying
    /// DomainAgentsImpl if there is no Runtime.evaluate running. If there is
    /// Runtime.evaluate running, then the weak pointer would need to be upgrade
    /// to a strong reference and then the actual destruction of
    /// DomainAgentsImpl would happen at the end of the Runtime.evaluate.
    std::weak_ptr<DomainAgentsImpl> getImplWeakPtr() const;

   private:
    /// This is a shared_ptr so that we can give out weak_ptr.
    std::shared_ptr<DomainAgentsImpl> impl_{};
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
    CDPDebugAPI &cdpDebugAPI,
    EnqueueRuntimeTaskFunc enqueueRuntimeTaskCallback,
    SynchronizedOutboundCallback messageCallback,
    State &state,
    std::shared_ptr<std::atomic_bool> destroyedDomainAgentsImpl)
    : messageCallback_(std::move(messageCallback)),
      runtimeTaskRunner_(
          cdpDebugAPI.asyncDebuggerAPI(),
          std::move(enqueueRuntimeTaskCallback)),
      domainAgents_(std::make_shared<DomainAgents>(
          executionContextID,
          cdpDebugAPI,
          messageCallback_,
          (state && state->debuggerAgentState)
              ? std::move(state->debuggerAgentState)
              : std::make_unique<DomainState>(),
          std::move(destroyedDomainAgentsImpl))) {}

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
      [domainAgents = domainAgents_](HermesRuntime &) {
        domainAgents->dispose();
      });

  // Call invalidate() to prevent the OutboundMessageFunc from being used again
  // after this destructor. We have to be careful here because both
  // messageCallback_ and DomainAgents use mutex internally, so there are 2
  // locks. Wrong ordering of obtaining those locks will cause deadlock.
  // Fortunately, in this function only the messageCallback_ lock is being used,
  // and on the runtime thread, the domain agents' destructors don't use
  // messageCallback_. Therefore, no lock conflicts. If those conditions change
  // in the future, we have to be careful.
  messageCallback_.invalidate();
}

void CDPAgentImpl::initializeDomainAgents() {
  runtimeTaskRunner_.enqueueTask(
      [domainAgents = domainAgents_](HermesRuntime &) {
        domainAgents->initialize();
      });
}

void CDPAgentImpl::handleCommand(std::string json) {
  std::shared_ptr<message::Request> command = message::Request::fromJson(json);
  if (!command) {
    m::ErrorResponse resp;
    resp.code = static_cast<int>(message::ErrorCode::ParseError);
    resp.message = "Malformed JSON";
    messageCallback_(resp.toJsonStr());
    return;
  }

  // Call DomainAgents::handleCommand on the runtime thread.
  TaskQueues queues = messageTaskQueues(command->method);
  RuntimeTask task = [weakDomainAgentsImpl = domainAgents_->getImplWeakPtr(),
                      command = std::move(command)](HermesRuntime &) mutable {
    if (auto strongDomainAgents = weakDomainAgentsImpl.lock()) {
      strongDomainAgents->handleCommand(std::move(command));
    }
  };
  runtimeTaskRunner_.enqueueTask(task, queues);
}

void CDPAgentImpl::enableRuntimeDomain() {
  runtimeTaskRunner_.enqueueTask(
      [weakDomainAgentsImpl =
           domainAgents_->getImplWeakPtr()](HermesRuntime &) {
        if (auto strongDomainAgents = weakDomainAgentsImpl.lock()) {
          strongDomainAgents->enableRuntimeDomain();
        }
      });
}

void CDPAgentImpl::enableDebuggerDomain() {
  runtimeTaskRunner_.enqueueTask(
      [weakDomainAgentsImpl =
           domainAgents_->getImplWeakPtr()](HermesRuntime &) {
        if (auto strongDomainAgents = weakDomainAgentsImpl.lock()) {
          strongDomainAgents->enableDebuggerDomain();
        }
      });
}

State CDPAgentImpl::getState() {
  return State(
      std::make_unique<State::Private>(domainAgents_->getDebuggerAgentState()));
}

CDPAgentImpl::DomainAgentsImpl::DomainAgentsImpl(
    int32_t executionContextID,
    HermesRuntime &runtime,
    debugger::AsyncDebuggerAPI &asyncDebuggerAPI,
    ConsoleMessageStorage &consoleMessageStorage,
    ConsoleMessageDispatcher &consoleMessageDispatcher,
    SynchronizedOutboundCallback messageCallback,
    std::unique_ptr<DomainState> debuggerAgentState,
    std::shared_ptr<std::atomic_bool> destroyedDomainAgentsImpl)
    : executionContextID_(executionContextID),
      runtime_(runtime),
      asyncDebuggerAPI_(asyncDebuggerAPI),
      consoleMessageStorage_(consoleMessageStorage),
      consoleMessageDispatcher_(consoleMessageDispatcher),
      messageCallback_(std::move(messageCallback)),
      objTable_(std::make_shared<RemoteObjectsTable>()),
      debuggerAgentState_(std::move(debuggerAgentState)),
      destroyedDomainAgentsImpl_(std::move(destroyedDomainAgentsImpl)) {
  assert(
      debuggerAgentState_ != nullptr &&
      "debuggerAgentState_ shouldn't ever be null");
}

void CDPAgentImpl::DomainAgentsImpl::initialize() {
  debuggerAgent_ = std::make_unique<DebuggerDomainAgent>(
      executionContextID_,
      runtime_,
      asyncDebuggerAPI_,
      messageCallback_,
      objTable_,
      *debuggerAgentState_);
  runtimeAgent_ = std::make_unique<RuntimeDomainAgent>(
      executionContextID_,
      runtime_,
      asyncDebuggerAPI_,
      messageCallback_,
      objTable_,
      consoleMessageStorage_,
      consoleMessageDispatcher_);
  profilerAgent_ = std::make_unique<ProfilerDomainAgent>(
      executionContextID_, runtime_, messageCallback_, objTable_);
  heapProfilerAgent_ = std::make_unique<HeapProfilerDomainAgent>(
      executionContextID_, runtime_, messageCallback_, objTable_);
}

CDPAgentImpl::DomainAgentsImpl::~DomainAgentsImpl() {
  if (destroyedDomainAgentsImpl_) {
    *destroyedDomainAgentsImpl_ = true;
  }
}

void CDPAgentImpl::DomainAgentsImpl::handleCommand(
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
  } else if (command->method == "Runtime.callFunctionOn") {
    runtimeAgent_->callFunctionOn(
        static_cast<m::runtime::CallFunctionOnRequest &>(*command));
  } else if (command->method == "Runtime.discardConsoleEntries") {
    runtimeAgent_->discardConsoleEntries(
        static_cast<m::runtime::DiscardConsoleEntriesRequest &>(*command));
  } else if (command->method == "Profiler.start") {
    profilerAgent_->start(static_cast<m::profiler::StartRequest &>(*command));
  } else if (command->method == "Profiler.stop") {
    profilerAgent_->stop(static_cast<m::profiler::StopRequest &>(*command));
  } else if (command->method == "HeapProfiler.takeHeapSnapshot") {
    heapProfilerAgent_->takeHeapSnapshot(
        static_cast<m::heapProfiler::TakeHeapSnapshotRequest &>(*command));
  } else if (command->method == "HeapProfiler.getObjectByHeapObjectId") {
    heapProfilerAgent_->getObjectByHeapObjectId(
        static_cast<m::heapProfiler::GetObjectByHeapObjectIdRequest &>(
            *command));
  } else if (command->method == "HeapProfiler.getHeapObjectId") {
    heapProfilerAgent_->getHeapObjectId(
        static_cast<m::heapProfiler::GetHeapObjectIdRequest &>(*command));
  } else if (command->method == "HeapProfiler.collectGarbage") {
    heapProfilerAgent_->collectGarbage(
        static_cast<m::heapProfiler::CollectGarbageRequest &>(*command));
  } else if (command->method == "HeapProfiler.startTrackingHeapObjects") {
    heapProfilerAgent_->startTrackingHeapObjects(
        static_cast<m::heapProfiler::StartTrackingHeapObjectsRequest &>(
            *command));
  } else if (command->method == "HeapProfiler.stopTrackingHeapObjects") {
    heapProfilerAgent_->stopTrackingHeapObjects(
        static_cast<m::heapProfiler::StopTrackingHeapObjectsRequest &>(
            *command));
  } else if (command->method == "HeapProfiler.startSampling") {
    heapProfilerAgent_->startSampling(
        static_cast<m::heapProfiler::StartSamplingRequest &>(*command));
  } else if (command->method == "HeapProfiler.stopSampling") {
    heapProfilerAgent_->stopSampling(
        static_cast<m::heapProfiler::StopSamplingRequest &>(*command));
  } else {
    messageCallback_(message::makeErrorResponse(
                         command->id,
                         message::ErrorCode::MethodNotFound,
                         "Unsupported method '" + command->method + "'")
                         .toJsonStr());
  }
}

void CDPAgentImpl::DomainAgentsImpl::enableRuntimeDomain() {
  runtimeAgent_->enable();
}

void CDPAgentImpl::DomainAgentsImpl::enableDebuggerDomain() {
  debuggerAgent_->enable();
}

std::unique_ptr<DomainState>
CDPAgentImpl::DomainAgentsImpl::getDebuggerAgentState() {
  return debuggerAgentState_->copy();
}

CDPAgentImpl::DomainAgents::DomainAgents(
    int32_t executionContextID,
    CDPDebugAPI &cdpDebugAPI,
    SynchronizedOutboundCallback messageCallback,
    std::unique_ptr<DomainState> debuggerAgentState,
    std::shared_ptr<std::atomic_bool> destroyedDomainAgentsImpl)
    // Allocate using new to ensure the memory is separate from the shared_ptr
    // control block. Since we don't control when the integrator queue discards
    // their queued tasks, allocating this way allows us to be in control of
    // when DomainAgentsImpl gets cleaned up.
    : impl_(std::shared_ptr<DomainAgentsImpl>(new DomainAgentsImpl(
          executionContextID,
          cdpDebugAPI.runtime(),
          cdpDebugAPI.asyncDebuggerAPI(),
          cdpDebugAPI.consoleMessageStorage_,
          cdpDebugAPI.consoleMessageDispatcher_,
          std::move(messageCallback),
          std::move(debuggerAgentState),
          std::move(destroyedDomainAgentsImpl)))) {}

void CDPAgentImpl::DomainAgents::initialize() {
  impl_->initialize();
}

void CDPAgentImpl::DomainAgents::dispose() {
  impl_.reset();
}

std::unique_ptr<DomainState>
CDPAgentImpl::DomainAgents::getDebuggerAgentState() {
  assert(impl_ != nullptr);
  return impl_->getDebuggerAgentState();
}

std::weak_ptr<CDPAgentImpl::DomainAgentsImpl>
CDPAgentImpl::DomainAgents::getImplWeakPtr() const {
  assert(impl_ != nullptr);
  return impl_;
}

std::unique_ptr<CDPAgent> CDPAgent::create(
    int32_t executionContextID,
    CDPDebugAPI &cdpDebugAPI,
    EnqueueRuntimeTaskFunc enqueueRuntimeTaskCallback,
    OutboundMessageFunc messageCallback,
    State state) {
  return std::unique_ptr<CDPAgent>(new CDPAgent(
      executionContextID,
      cdpDebugAPI,
      enqueueRuntimeTaskCallback,
      messageCallback,
      std::move(state),
      nullptr));
}

CDPAgent::CDPAgent(
    int32_t executionContextID,
    CDPDebugAPI &cdpDebugAPI,
    EnqueueRuntimeTaskFunc enqueueRuntimeTaskCallback,
    OutboundMessageFunc messageCallback,
    State state,
    std::shared_ptr<std::atomic_bool> destroyedDomainAgents)
    : impl_(std::make_unique<CDPAgentImpl>(
          executionContextID,
          cdpDebugAPI,
          enqueueRuntimeTaskCallback,
          SynchronizedOutboundCallback(messageCallback),
          state,
          destroyedDomainAgents)) {
  impl_->initializeDomainAgents();
}

CDPAgent::~CDPAgent() {}

void CDPAgent::handleCommand(std::string json) {
  impl_->handleCommand(json);
}

void CDPAgent::enableRuntimeDomain() {
  impl_->enableRuntimeDomain();
}

void CDPAgent::enableDebuggerDomain() {
  impl_->enableDebuggerDomain();
}

State CDPAgent::getState() {
  return impl_->getState();
}

} // namespace cdp
} // namespace hermes
} // namespace facebook
