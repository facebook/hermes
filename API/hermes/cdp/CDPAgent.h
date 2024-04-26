/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_CDP_CDPAGENT_H
#define HERMES_CDP_CDPAGENT_H

#include <string>

#include <hermes/AsyncDebuggerAPI.h>
#include <hermes/Public/HermesExport.h>
#include <hermes/RuntimeTaskRunner.h>
#include <hermes/hermes.h>

namespace facebook {
namespace hermes {
namespace cdp {

using OutboundMessageFunc = std::function<void(const std::string &)>;

class CDPAgentImpl;

/// An agent for interacting with the provided \p runtime and
/// \p asyncDebuggerAPI via CDP messages in the Debugger, Runtime, Profiler,
/// HeapProfiler domains.
/// The integrator of the agent is expected to manage a queue of tasks to be
/// executed with exclusive access to the runtime (i.e. executed when
/// JavaScript is not running). Tasks to be run are delivered to the integrator
/// via the provided \p enqueueRuntimeTaskCallback, and should be executed in
/// order, at the first opportunity between evaluating JavaScript.
/// The integrator can deliver CDP commands to the agent via the
/// \p handleCommand method. When a CDP response or event is generated, it will
/// be delivered to the integrator via the provided \p messageCallback.
/// Both callbacks may be invoked from arbitrary threads.
class HERMES_EXPORT CDPAgent {
  /// Hide the constructor so users can only construct via static create
  /// methods.
  CDPAgent(
      int32_t executionContextID,
      HermesRuntime &runtime,
      debugger::AsyncDebuggerAPI &asyncDebuggerAPI,
      debugger::EnqueueRuntimeTaskFunc enqueueRuntimeTaskCallback,
      OutboundMessageFunc messageCallback);

 public:
  /// Create a new CDP Agent. This can be done on an arbitrary thread; the
  /// runtime will not be accessed during execution of this function.
  static std::unique_ptr<CDPAgent> create(
      int32_t executionContextID,
      HermesRuntime &runtime,
      debugger::AsyncDebuggerAPI &asyncDebuggerAPI,
      debugger::EnqueueRuntimeTaskFunc enqueueRuntimeTaskCallback,
      OutboundMessageFunc messageCallback);

  /// Destroy the CDP Agent. This can be done on an arbitrary thread.
  /// It's expected that the integrator will continue to process any runtime
  /// tasks enqueued during destruction.
  ~CDPAgent();

  /// Process a CDP command encoded in \p json. This can be called from
  /// arbitrary threads.
  void handleCommand(std::string json);

 private:
  /// This should be a unique_ptr to provide predictable destruction time lined
  /// up with when CDPAgent is destroyed. Do not use shared_ptr.
  std::unique_ptr<CDPAgentImpl> impl_;
};

} // namespace cdp
} // namespace hermes
} // namespace facebook

#endif // HERMES_CDP_CDPAGENT_H
