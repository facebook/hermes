/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_CDP_RUNTIMEDOMAINAGENT_H
#define HERMES_CDP_RUNTIMEDOMAINAGENT_H

#include "DomainAgent.h"

namespace facebook {
namespace hermes {
namespace cdp {

namespace m = ::facebook::hermes::inspector_modern::chrome::message;

/// Handler for the "Runtime" domain of CDP. Accepts CDP requests belonging to
/// the "Runtime" domain from the debug client. Produces CDP responses and
/// events belonging to the "Runtime" domain. All methods expect to be invoked
/// with exclusive access to the runtime.
class RuntimeDomainAgent : public DomainAgent {
 public:
  RuntimeDomainAgent(
      int32_t executionContextID,
      HermesRuntime &runtime_,
      SynchronizedOutboundCallback messageCallback,
      std::shared_ptr<RemoteObjectsTable> objTable);
  ~RuntimeDomainAgent();

  /// Handles Runtime.enable request
  void enable(const m::runtime::EnableRequest &req);
  /// Handles Runtime.disable request
  void disable(const m::runtime::DisableRequest &req);
  /// Handles Runtime.getHeapUsage request
  void getHeapUsage(const m::runtime::GetHeapUsageRequest &req);
  /// Handles Runtime.globalLexicalScopeNames request
  void globalLexicalScopeNames(
      const m::runtime::GlobalLexicalScopeNamesRequest &req);
  /// Handles Runtime.compileScript request
  void compileScript(const m::runtime::CompileScriptRequest &req);
  /// Handles Runtime.getProperties request
  void getProperties(const m::runtime::GetPropertiesRequest &req);
  /// Handles Runtime.evaluate request
  void evaluate(const m::runtime::EvaluateRequest &req);

 private:
  bool checkRuntimeEnabled(const m::Request &req);

  std::vector<m::runtime::PropertyDescriptor> makePropsFromScope(
      std::pair<uint32_t, uint32_t> frameAndScopeIndex,
      const std::string &objectGroup,
      const debugger::ProgramState &state,
      bool generatePreview);
  std::vector<m::runtime::PropertyDescriptor> makePropsFromValue(
      const jsi::Value &value,
      const std::string &objectGroup,
      bool onlyOwnProperties,
      bool generatePreview);

  HermesRuntime &runtime_;

  /// Whether Runtime.enable was received and wasn't disabled by receiving
  /// Runtime.disable
  bool enabled_;

  // preparedScripts_ stores user-entered scripts that have been prepared for
  // execution, and may be invoked by a later command.
  std::vector<std::shared_ptr<const jsi::PreparedJavaScript>> preparedScripts_;
};

} // namespace cdp
} // namespace hermes
} // namespace facebook

#endif // HERMES_CDP_RUNTIMEDOMAINAGENT_H
