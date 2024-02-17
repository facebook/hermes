/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/inspector/chrome/MessageConverters.h>
#include <hermes/inspector/chrome/RemoteObjectConverters.h>
#include <jsi/instrumentation.h>

#include "RuntimeDomainAgent.h"

namespace facebook {
namespace hermes {
namespace cdp {

static const char *const kUserEnteredScriptIdPrefix = "userScript";
static const char *const kEvaluatedCodeUrl = "?eval";

RuntimeDomainAgent::RuntimeDomainAgent(
    int32_t executionContextID,
    HermesRuntime &runtime,
    SynchronizedOutboundCallback messageCallback,
    std::shared_ptr<RemoteObjectsTable> objTable)
    : DomainAgent(
          executionContextID,
          std::move(messageCallback),
          std::move(objTable)),
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

void RuntimeDomainAgent::getProperties(
    const m::runtime::GetPropertiesRequest &req) {
  if (!checkRuntimeEnabled(req)) {
    return;
  }

  bool generatePreview = req.generatePreview.value_or(false);
  bool ownProperties = req.ownProperties.value_or(true);

  std::string objGroup = objTable_->getObjectGroup(req.objectId);
  auto scopePtr = objTable_->getScope(req.objectId);
  auto valuePtr = objTable_->getValue(req.objectId);

  m::runtime::GetPropertiesResponse resp;
  resp.id = req.id;
  if (scopePtr != nullptr) {
    const debugger::ProgramState &state =
        runtime_.getDebugger().getProgramState();
    resp.result =
        makePropsFromScope(*scopePtr, objGroup, state, generatePreview);
  } else if (valuePtr != nullptr) {
    resp.result =
        makePropsFromValue(*valuePtr, objGroup, ownProperties, generatePreview);
  }
  sendResponseToClient(resp);
}

void RuntimeDomainAgent::evaluate(const m::runtime::EvaluateRequest &req) {
  if (!checkRuntimeEnabled(req)) {
    return;
  }

  m::runtime::EvaluateResponse resp;
  resp.id = req.id;

  std::string objectGroup = req.objectGroup.value_or("");
  try {
    // Evaluate the expression using the runtime's normal script evaluation
    // mechanism. This ensures the expression is evaluated in the global scope,
    // regardless of where the runtime happens to be paused.
    jsi::Value result = runtime_.evaluateJavaScript(
        std::unique_ptr<jsi::StringBuffer>(
            new jsi::StringBuffer(req.expression)),
        kEvaluatedCodeUrl);

    bool byValue = req.returnByValue.value_or(false);
    bool generatePreview = req.generatePreview.value_or(false);
    auto remoteObjPtr = m::runtime::makeRemoteObject(
        runtime_, result, *objTable_, objectGroup, byValue, generatePreview);
    resp.result = std::move(remoteObjPtr);
  } catch (const facebook::jsi::JSError &error) {
    resp.exceptionDetails = m::runtime::ExceptionDetails();
    resp.exceptionDetails->text = error.getMessage() + "\n" + error.getStack();
    resp.exceptionDetails->exception = m::runtime::makeRemoteObject(
        runtime_, error.value(), *objTable_, objectGroup, false, false);
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

std::vector<m::runtime::PropertyDescriptor>
RuntimeDomainAgent::makePropsFromScope(
    std::pair<uint32_t, uint32_t> frameAndScopeIndex,
    const std::string &objectGroup,
    const debugger::ProgramState &state,
    bool generatePreview) {
  // Chrome represents variables in a scope as properties on a dummy object.
  // We don't instantiate such dummy objects, we just pretended to have one.
  // Chrome has now asked for its properties, so it's time to synthesize
  // descriptions of the properties that the dummy object would have had.
  std::vector<m::runtime::PropertyDescriptor> result;

  uint32_t frameIndex = frameAndScopeIndex.first;
  uint32_t scopeIndex = frameAndScopeIndex.second;
  debugger::LexicalInfo lexicalInfo = state.getLexicalInfo(frameIndex);
  uint32_t varCount = lexicalInfo.getVariablesCountInScope(scopeIndex);

  // If this is the frame's local scope, include 'this'.
  if (scopeIndex == 0) {
    auto varInfo = state.getVariableInfoForThis(frameIndex);
    m::runtime::PropertyDescriptor desc;
    desc.name = varInfo.name;
    desc.value = m::runtime::makeRemoteObject(
        runtime_,
        varInfo.value,
        *objTable_,
        objectGroup,
        false,
        generatePreview);
    // Chrome only shows enumerable properties.
    desc.enumerable = true;
    result.emplace_back(std::move(desc));
  }

  // Then add each of the variables in this lexical scope.
  for (uint32_t varIndex = 0; varIndex < varCount; varIndex++) {
    debugger::VariableInfo varInfo =
        state.getVariableInfo(frameIndex, scopeIndex, varIndex);

    m::runtime::PropertyDescriptor desc;
    desc.name = varInfo.name;
    desc.value = m::runtime::makeRemoteObject(
        runtime_,
        varInfo.value,
        *objTable_,
        objectGroup,
        false,
        generatePreview);
    desc.enumerable = true;

    result.emplace_back(std::move(desc));
  }

  return result;
}

std::vector<m::runtime::PropertyDescriptor>
RuntimeDomainAgent::makePropsFromValue(
    const jsi::Value &value,
    const std::string &objectGroup,
    bool onlyOwnProperties,
    bool generatePreview) {
  std::vector<m::runtime::PropertyDescriptor> result;

  if (value.isObject()) {
    jsi::Runtime &runtime = runtime_;
    jsi::Object obj = value.getObject(runtime);

    // TODO(hypuk): obj.getPropertyNames only returns enumerable properties.
    jsi::Array propNames = onlyOwnProperties
        ? runtime.global()
              .getPropertyAsObject(runtime, "Object")
              .getPropertyAsFunction(runtime, "getOwnPropertyNames")
              .call(runtime, obj)
              .getObject(runtime)
              .getArray(runtime)
        : obj.getPropertyNames(runtime);

    size_t propCount = propNames.length(runtime);
    for (size_t i = 0; i < propCount; i++) {
      jsi::String propName =
          propNames.getValueAtIndex(runtime, i).getString(runtime);

      m::runtime::PropertyDescriptor desc;
      desc.name = propName.utf8(runtime);

      try {
        // Currently, we fetch the property even if it runs code.
        // Chrome instead detects getters and makes you click to invoke.
        jsi::Value propValue = obj.getProperty(runtime, propName);
        desc.value = m::runtime::makeRemoteObject(
            runtime,
            propValue,
            *objTable_,
            objectGroup,
            false,
            generatePreview);
      } catch (const jsi::JSError &err) {
        // We fetched a property with a getter that threw. Show a placeholder.
        // We could have added additional info, but the UI quickly gets messy.
        desc.value = m::runtime::makeRemoteObject(
            runtime,
            jsi::String::createFromUtf8(runtime, "(Exception)"),
            *objTable_,
            objectGroup,
            false,
            generatePreview);
      }

      result.emplace_back(std::move(desc));
    }

    if (onlyOwnProperties) {
      jsi::Value proto = runtime.global()
                             .getPropertyAsObject(runtime, "Object")
                             .getPropertyAsFunction(runtime, "getPrototypeOf")
                             .call(runtime, obj);
      if (!proto.isNull()) {
        m::runtime::PropertyDescriptor desc;
        desc.name = "__proto__";
        desc.value = m::runtime::makeRemoteObject(
            runtime, proto, *objTable_, objectGroup, false, generatePreview);
        result.emplace_back(std::move(desc));
      }
    }
  }

  return result;
}

} // namespace cdp
} // namespace hermes
} // namespace facebook
