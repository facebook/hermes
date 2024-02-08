/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/inspector/chrome/MessageConverters.h>
#include <jsi/instrumentation.h>

#include "RuntimeDomainAgent.h"

namespace facebook {
namespace hermes {
namespace cdp {

static const char *const kUserEnteredScriptIdPrefix = "userScript";

RuntimeDomainAgent::RuntimeDomainAgent(
    int32_t executionContextID,
    HermesRuntime &runtime,
    SynchronizedOutboundCallback messageCallback)
    : DomainAgent(executionContextID, std::move(messageCallback)),
      runtime_(runtime),
      enabled_(false) {}

RuntimeDomainAgent::~RuntimeDomainAgent() {}

void RuntimeDomainAgent::enable(const m::runtime::EnableRequest &req) {
  if (enabled_) {
    // Can't enable twice without disabling
    sendResponseToClient(m::makeErrorResponse(
        req.id,
        m::ErrorCode::InvalidRequest,
        "Runtime domain already enabled"));
    return;
  }

  // Enable
  enabled_ = true;
  sendResponseToClient(m::makeOkResponse(req.id));

  // Notify the client about the hard-coded Hermes execution context.
  m::runtime::ExecutionContextCreatedNotification note;
  note.context.id = kHermesExecutionContextId;
  note.context.name = "hermes";
  sendNotificationToClient(note);
}

void RuntimeDomainAgent::disable(const m::runtime::DisableRequest &req) {
  if (!checkRuntimeEnabled(req)) {
    return;
  }
  enabled_ = false;
  sendResponseToClient(m::makeOkResponse(req.id));
}

void RuntimeDomainAgent::getHeapUsage(
    const m::runtime::GetHeapUsageRequest &req) {
  if (!checkRuntimeEnabled(req)) {
    return;
  }

  auto heapInfo = runtime_.instrumentation().getHeapInfo(false);
  m::runtime::GetHeapUsageResponse resp;
  resp.id = req.id;
  resp.usedSize = heapInfo["hermes_allocatedBytes"];
  resp.totalSize = heapInfo["hermes_heapSize"];
  sendResponseToClient(resp);
}

void RuntimeDomainAgent::globalLexicalScopeNames(
    const m::runtime::GlobalLexicalScopeNamesRequest &req) {
  if (!checkRuntimeEnabled(req)) {
    return;
  }

  const debugger::ProgramState &state =
      runtime_.getDebugger().getProgramState();
  const debugger::LexicalInfo &lexicalInfo = state.getLexicalInfo(0);
  debugger::ScopeDepth scopeCount = lexicalInfo.getScopesCount();
  if (scopeCount == 0) {
    return;
  }
  const debugger::ScopeDepth globalScopeIndex = scopeCount - 1;
  uint32_t variableCount =
      lexicalInfo.getVariablesCountInScope(globalScopeIndex);

  m::runtime::GlobalLexicalScopeNamesResponse resp;
  resp.id = req.id;
  resp.names.reserve(variableCount);
  for (uint32_t i = 0; i < variableCount; i++) {
    debugger::String name = state.getVariableInfo(0, globalScopeIndex, i).name;
    // The global scope has some entries prefixed with '?', which
    // are not valid identifiers.
    if (!name.empty() && name.front() != '?') {
      resp.names.push_back(name);
    }
  }
  sendResponseToClient(resp);
}

void RuntimeDomainAgent::compileScript(
    const m::runtime::CompileScriptRequest &req) {
  if (!checkRuntimeEnabled(req)) {
    return;
  }

  m::runtime::CompileScriptResponse resp;
  resp.id = req.id;

  auto source = std::make_shared<jsi::StringBuffer>(req.expression);
  std::shared_ptr<const jsi::PreparedJavaScript> preparedScript;
  try {
    preparedScript = runtime_.prepareJavaScript(source, req.sourceURL);
  } catch (const facebook::jsi::JSIException &err) {
    resp.exceptionDetails = m::runtime::ExceptionDetails();
    resp.exceptionDetails->text = err.what();
    sendResponseToClient(resp);
    return;
  }

  if (req.persistScript) {
    auto scriptId =
        kUserEnteredScriptIdPrefix + std::to_string(preparedScripts_.size());
    preparedScripts_.push_back(std::move(preparedScript));
    resp.scriptId = scriptId;
  }
  sendResponseToClient(resp);
}

bool RuntimeDomainAgent::checkRuntimeEnabled(const m::Request &req) {
  if (!enabled_) {
    sendResponseToClient(m::makeErrorResponse(
        req.id, m::ErrorCode::InvalidRequest, "Runtime domain not enabled"));
    return false;
  }
  return true;
}

} // namespace cdp
} // namespace hermes
} // namespace facebook
