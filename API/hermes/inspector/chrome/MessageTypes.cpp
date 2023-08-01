// Copyright (c) Meta Platforms, Inc. and affiliates. All Rights Reserved.
// @generated SignedSource<<8bdb8c171c256400dff60d6d1bf2ad28>>

#include "MessageTypes.h"

#include "MessageTypesInlines.h"

namespace facebook {
namespace hermes {
namespace inspector {
namespace chrome {
namespace message {

using RequestBuilder = std::unique_ptr<Request> (*)(const JSONObject *obj);

namespace {

template <typename T>
std::unique_ptr<Request> makeUnique(const JSONObject *obj) {
  return std::make_unique<T>(obj);
}

void assignJsonBlob(
    optional<std::string> &field,
    const JSONObject *obj,
    const std::string &key) {
  JSONValue *v = safeGet(obj, key);
  if (v != nullptr) {
    field = jsonValToStr(v);
  } else {
    field.reset();
  }
}

void putJsonBlob(
    Properties &props,
    const std::string &key,
    optional<std::string> blob,
    JSONFactory &factory) {
  if (blob.has_value()) {
    JSONString *jsStr = factory.getString(key);
    JSONValue *jsVal = parseStr(*blob, factory);
    props.push_back({jsStr, jsVal});
  }
}

} // namespace

std::unique_ptr<Request> Request::fromJsonThrowOnError(const std::string &str) {
  static std::unordered_map<std::string, RequestBuilder> builders = {
      {"Debugger.disable", makeUnique<debugger::DisableRequest>},
      {"Debugger.enable", makeUnique<debugger::EnableRequest>},
      {"Debugger.evaluateOnCallFrame",
       makeUnique<debugger::EvaluateOnCallFrameRequest>},
      {"Debugger.pause", makeUnique<debugger::PauseRequest>},
      {"Debugger.removeBreakpoint",
       makeUnique<debugger::RemoveBreakpointRequest>},
      {"Debugger.resume", makeUnique<debugger::ResumeRequest>},
      {"Debugger.setBreakpoint", makeUnique<debugger::SetBreakpointRequest>},
      {"Debugger.setBreakpointByUrl",
       makeUnique<debugger::SetBreakpointByUrlRequest>},
      {"Debugger.setBreakpointsActive",
       makeUnique<debugger::SetBreakpointsActiveRequest>},
      {"Debugger.setInstrumentationBreakpoint",
       makeUnique<debugger::SetInstrumentationBreakpointRequest>},
      {"Debugger.setPauseOnExceptions",
       makeUnique<debugger::SetPauseOnExceptionsRequest>},
      {"Debugger.stepInto", makeUnique<debugger::StepIntoRequest>},
      {"Debugger.stepOut", makeUnique<debugger::StepOutRequest>},
      {"Debugger.stepOver", makeUnique<debugger::StepOverRequest>},
      {"HeapProfiler.collectGarbage",
       makeUnique<heapProfiler::CollectGarbageRequest>},
      {"HeapProfiler.getHeapObjectId",
       makeUnique<heapProfiler::GetHeapObjectIdRequest>},
      {"HeapProfiler.getObjectByHeapObjectId",
       makeUnique<heapProfiler::GetObjectByHeapObjectIdRequest>},
      {"HeapProfiler.startSampling",
       makeUnique<heapProfiler::StartSamplingRequest>},
      {"HeapProfiler.startTrackingHeapObjects",
       makeUnique<heapProfiler::StartTrackingHeapObjectsRequest>},
      {"HeapProfiler.stopSampling",
       makeUnique<heapProfiler::StopSamplingRequest>},
      {"HeapProfiler.stopTrackingHeapObjects",
       makeUnique<heapProfiler::StopTrackingHeapObjectsRequest>},
      {"HeapProfiler.takeHeapSnapshot",
       makeUnique<heapProfiler::TakeHeapSnapshotRequest>},
      {"Profiler.start", makeUnique<profiler::StartRequest>},
      {"Profiler.stop", makeUnique<profiler::StopRequest>},
      {"Runtime.callFunctionOn", makeUnique<runtime::CallFunctionOnRequest>},
      {"Runtime.compileScript", makeUnique<runtime::CompileScriptRequest>},
      {"Runtime.disable", makeUnique<runtime::DisableRequest>},
      {"Runtime.enable", makeUnique<runtime::EnableRequest>},
      {"Runtime.evaluate", makeUnique<runtime::EvaluateRequest>},
      {"Runtime.getHeapUsage", makeUnique<runtime::GetHeapUsageRequest>},
      {"Runtime.getProperties", makeUnique<runtime::GetPropertiesRequest>},
      {"Runtime.globalLexicalScopeNames",
       makeUnique<runtime::GlobalLexicalScopeNamesRequest>},
      {"Runtime.runIfWaitingForDebugger",
       makeUnique<runtime::RunIfWaitingForDebuggerRequest>},
  };

  JSLexer::Allocator alloc;
  JSONFactory factory(alloc);
  auto *jsonObj = parseStrAsJsonObj(str, factory);

  std::string method;
  assign(method, jsonObj, "method");

  auto it = builders.find(method);
  if (it == builders.end()) {
    return std::make_unique<UnknownRequest>(jsonObj);
  }

  auto builder = it->second;
  return builder(jsonObj);
}

Request::ParseResult Request::fromJson(const std::string &str) {
  try {
    return Request::fromJsonThrowOnError(str);
  } catch (const std::exception &e) {
    return e.what();
  }
}

/// Types
debugger::Location::Location(const JSONObject *obj) {
  assign(scriptId, obj, "scriptId");
  assign(lineNumber, obj, "lineNumber");
  assign(columnNumber, obj, "columnNumber");
}

JSONValue *debugger::Location::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> props;

  put(props, "scriptId", scriptId, factory);
  put(props, "lineNumber", lineNumber, factory);
  put(props, "columnNumber", columnNumber, factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::PropertyPreview::PropertyPreview(const JSONObject *obj) {
  assign(name, obj, "name");
  assign(type, obj, "type");
  assign(value, obj, "value");
  assign(valuePreview, obj, "valuePreview");
  assign(subtype, obj, "subtype");
}

JSONValue *runtime::PropertyPreview::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 5> props;

  put(props, "name", name, factory);
  put(props, "type", type, factory);
  put(props, "value", value, factory);
  put(props, "valuePreview", valuePreview, factory);
  put(props, "subtype", subtype, factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::EntryPreview::EntryPreview(const JSONObject *obj) {
  assign(key, obj, "key");
  assign(value, obj, "value");
}

JSONValue *runtime::EntryPreview::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> props;

  put(props, "key", key, factory);
  put(props, "value", value, factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::ObjectPreview::ObjectPreview(const JSONObject *obj) {
  assign(type, obj, "type");
  assign(subtype, obj, "subtype");
  assign(description, obj, "description");
  assign(overflow, obj, "overflow");
  assign(properties, obj, "properties");
  assign(entries, obj, "entries");
}

JSONValue *runtime::ObjectPreview::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 6> props;

  put(props, "type", type, factory);
  put(props, "subtype", subtype, factory);
  put(props, "description", description, factory);
  put(props, "overflow", overflow, factory);
  put(props, "properties", properties, factory);
  put(props, "entries", entries, factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::CustomPreview::CustomPreview(const JSONObject *obj) {
  assign(header, obj, "header");
  assign(bodyGetterId, obj, "bodyGetterId");
}

JSONValue *runtime::CustomPreview::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> props;

  put(props, "header", header, factory);
  put(props, "bodyGetterId", bodyGetterId, factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::RemoteObject::RemoteObject(const JSONObject *obj) {
  assign(type, obj, "type");
  assign(subtype, obj, "subtype");
  assign(className, obj, "className");
  assignJsonBlob(value, obj, "value");
  assign(unserializableValue, obj, "unserializableValue");
  assign(description, obj, "description");
  assign(objectId, obj, "objectId");
  assign(preview, obj, "preview");
  assign(customPreview, obj, "customPreview");
}

JSONValue *runtime::RemoteObject::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 9> props;

  put(props, "type", type, factory);
  put(props, "subtype", subtype, factory);
  put(props, "className", className, factory);
  putJsonBlob(props, "value", value, factory);
  put(props, "unserializableValue", unserializableValue, factory);
  put(props, "description", description, factory);
  put(props, "objectId", objectId, factory);
  put(props, "preview", preview, factory);
  put(props, "customPreview", customPreview, factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::CallFrame::CallFrame(const JSONObject *obj) {
  assign(functionName, obj, "functionName");
  assign(scriptId, obj, "scriptId");
  assign(url, obj, "url");
  assign(lineNumber, obj, "lineNumber");
  assign(columnNumber, obj, "columnNumber");
}

JSONValue *runtime::CallFrame::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 5> props;

  put(props, "functionName", functionName, factory);
  put(props, "scriptId", scriptId, factory);
  put(props, "url", url, factory);
  put(props, "lineNumber", lineNumber, factory);
  put(props, "columnNumber", columnNumber, factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::StackTrace::StackTrace(const JSONObject *obj) {
  assign(description, obj, "description");
  assign(callFrames, obj, "callFrames");
  assign(parent, obj, "parent");
}

JSONValue *runtime::StackTrace::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> props;

  put(props, "description", description, factory);
  put(props, "callFrames", callFrames, factory);
  put(props, "parent", parent, factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::ExceptionDetails::ExceptionDetails(const JSONObject *obj) {
  assign(exceptionId, obj, "exceptionId");
  assign(text, obj, "text");
  assign(lineNumber, obj, "lineNumber");
  assign(columnNumber, obj, "columnNumber");
  assign(scriptId, obj, "scriptId");
  assign(url, obj, "url");
  assign(stackTrace, obj, "stackTrace");
  assign(exception, obj, "exception");
  assign(executionContextId, obj, "executionContextId");
}

JSONValue *runtime::ExceptionDetails::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 9> props;

  put(props, "exceptionId", exceptionId, factory);
  put(props, "text", text, factory);
  put(props, "lineNumber", lineNumber, factory);
  put(props, "columnNumber", columnNumber, factory);
  put(props, "scriptId", scriptId, factory);
  put(props, "url", url, factory);
  put(props, "stackTrace", stackTrace, factory);
  put(props, "exception", exception, factory);
  put(props, "executionContextId", executionContextId, factory);
  return factory.newObject(props.begin(), props.end());
}

debugger::Scope::Scope(const JSONObject *obj) {
  assign(type, obj, "type");
  assign(object, obj, "object");
  assign(name, obj, "name");
  assign(startLocation, obj, "startLocation");
  assign(endLocation, obj, "endLocation");
}

JSONValue *debugger::Scope::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 5> props;

  put(props, "type", type, factory);
  put(props, "object", object, factory);
  put(props, "name", name, factory);
  put(props, "startLocation", startLocation, factory);
  put(props, "endLocation", endLocation, factory);
  return factory.newObject(props.begin(), props.end());
}

debugger::CallFrame::CallFrame(const JSONObject *obj) {
  assign(callFrameId, obj, "callFrameId");
  assign(functionName, obj, "functionName");
  assign(functionLocation, obj, "functionLocation");
  assign(location, obj, "location");
  assign(url, obj, "url");
  assign(scopeChain, obj, "scopeChain");
  assign(thisObj, obj, "this");
  assign(returnValue, obj, "returnValue");
}

JSONValue *debugger::CallFrame::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 8> props;

  put(props, "callFrameId", callFrameId, factory);
  put(props, "functionName", functionName, factory);
  put(props, "functionLocation", functionLocation, factory);
  put(props, "location", location, factory);
  put(props, "url", url, factory);
  put(props, "scopeChain", scopeChain, factory);
  put(props, "this", thisObj, factory);
  put(props, "returnValue", returnValue, factory);
  return factory.newObject(props.begin(), props.end());
}

heapProfiler::SamplingHeapProfileNode::SamplingHeapProfileNode(
    const JSONObject *obj) {
  assign(callFrame, obj, "callFrame");
  assign(selfSize, obj, "selfSize");
  assign(id, obj, "id");
  assign(children, obj, "children");
}

JSONValue *heapProfiler::SamplingHeapProfileNode::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 4> props;

  put(props, "callFrame", callFrame, factory);
  put(props, "selfSize", selfSize, factory);
  put(props, "id", id, factory);
  put(props, "children", children, factory);
  return factory.newObject(props.begin(), props.end());
}

heapProfiler::SamplingHeapProfileSample::SamplingHeapProfileSample(
    const JSONObject *obj) {
  assign(size, obj, "size");
  assign(nodeId, obj, "nodeId");
  assign(ordinal, obj, "ordinal");
}

JSONValue *heapProfiler::SamplingHeapProfileSample::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> props;

  put(props, "size", size, factory);
  put(props, "nodeId", nodeId, factory);
  put(props, "ordinal", ordinal, factory);
  return factory.newObject(props.begin(), props.end());
}

heapProfiler::SamplingHeapProfile::SamplingHeapProfile(const JSONObject *obj) {
  assign(head, obj, "head");
  assign(samples, obj, "samples");
}

JSONValue *heapProfiler::SamplingHeapProfile::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> props;

  put(props, "head", head, factory);
  put(props, "samples", samples, factory);
  return factory.newObject(props.begin(), props.end());
}

profiler::PositionTickInfo::PositionTickInfo(const JSONObject *obj) {
  assign(line, obj, "line");
  assign(ticks, obj, "ticks");
}

JSONValue *profiler::PositionTickInfo::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> props;

  put(props, "line", line, factory);
  put(props, "ticks", ticks, factory);
  return factory.newObject(props.begin(), props.end());
}

profiler::ProfileNode::ProfileNode(const JSONObject *obj) {
  assign(id, obj, "id");
  assign(callFrame, obj, "callFrame");
  assign(hitCount, obj, "hitCount");
  assign(children, obj, "children");
  assign(deoptReason, obj, "deoptReason");
  assign(positionTicks, obj, "positionTicks");
}

JSONValue *profiler::ProfileNode::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 6> props;

  put(props, "id", id, factory);
  put(props, "callFrame", callFrame, factory);
  put(props, "hitCount", hitCount, factory);
  put(props, "children", children, factory);
  put(props, "deoptReason", deoptReason, factory);
  put(props, "positionTicks", positionTicks, factory);
  return factory.newObject(props.begin(), props.end());
}

profiler::Profile::Profile(const JSONObject *obj) {
  assign(nodes, obj, "nodes");
  assign(startTime, obj, "startTime");
  assign(endTime, obj, "endTime");
  assign(samples, obj, "samples");
  assign(timeDeltas, obj, "timeDeltas");
}

JSONValue *profiler::Profile::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 5> props;

  put(props, "nodes", nodes, factory);
  put(props, "startTime", startTime, factory);
  put(props, "endTime", endTime, factory);
  put(props, "samples", samples, factory);
  put(props, "timeDeltas", timeDeltas, factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::CallArgument::CallArgument(const JSONObject *obj) {
  assignJsonBlob(value, obj, "value");
  assign(unserializableValue, obj, "unserializableValue");
  assign(objectId, obj, "objectId");
}

JSONValue *runtime::CallArgument::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> props;

  putJsonBlob(props, "value", value, factory);
  put(props, "unserializableValue", unserializableValue, factory);
  put(props, "objectId", objectId, factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::ExecutionContextDescription::ExecutionContextDescription(
    const JSONObject *obj) {
  assign(id, obj, "id");
  assign(origin, obj, "origin");
  assign(name, obj, "name");
  assignJsonBlob(auxData, obj, "auxData");
}

JSONValue *runtime::ExecutionContextDescription::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 4> props;

  put(props, "id", id, factory);
  put(props, "origin", origin, factory);
  put(props, "name", name, factory);
  putJsonBlob(props, "auxData", auxData, factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::PropertyDescriptor::PropertyDescriptor(const JSONObject *obj) {
  assign(name, obj, "name");
  assign(value, obj, "value");
  assign(writable, obj, "writable");
  assign(get, obj, "get");
  assign(set, obj, "set");
  assign(configurable, obj, "configurable");
  assign(enumerable, obj, "enumerable");
  assign(wasThrown, obj, "wasThrown");
  assign(isOwn, obj, "isOwn");
  assign(symbol, obj, "symbol");
}

JSONValue *runtime::PropertyDescriptor::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 10> props;

  put(props, "name", name, factory);
  put(props, "value", value, factory);
  put(props, "writable", writable, factory);
  put(props, "get", get, factory);
  put(props, "set", set, factory);
  put(props, "configurable", configurable, factory);
  put(props, "enumerable", enumerable, factory);
  put(props, "wasThrown", wasThrown, factory);
  put(props, "isOwn", isOwn, factory);
  put(props, "symbol", symbol, factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::InternalPropertyDescriptor::InternalPropertyDescriptor(
    const JSONObject *obj) {
  assign(name, obj, "name");
  assign(value, obj, "value");
}

JSONValue *runtime::InternalPropertyDescriptor::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> props;

  put(props, "name", name, factory);
  put(props, "value", value, factory);
  return factory.newObject(props.begin(), props.end());
}

/// Requests
UnknownRequest::UnknownRequest() {}

UnknownRequest::UnknownRequest(const JSONObject *obj) {
  assign(id, obj, "id");
  assign(method, obj, "method");
  assignJsonBlob(params, obj, "params");
}

JSONValue *UnknownRequest::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  putJsonBlob(props, "params", params, factory);
  return factory.newObject(props.begin(), props.end());
}

void UnknownRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

debugger::DisableRequest::DisableRequest() : Request("Debugger.disable") {}

debugger::DisableRequest::DisableRequest(const JSONObject *obj)
    : Request("Debugger.disable") {
  assign(id, obj, "id");
  assign(method, obj, "method");
}

JSONValue *debugger::DisableRequest::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  return factory.newObject(props.begin(), props.end());
}

void debugger::DisableRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

debugger::EnableRequest::EnableRequest() : Request("Debugger.enable") {}

debugger::EnableRequest::EnableRequest(const JSONObject *obj)
    : Request("Debugger.enable") {
  assign(id, obj, "id");
  assign(method, obj, "method");
}

JSONValue *debugger::EnableRequest::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  return factory.newObject(props.begin(), props.end());
}

void debugger::EnableRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

debugger::EvaluateOnCallFrameRequest::EvaluateOnCallFrameRequest()
    : Request("Debugger.evaluateOnCallFrame") {}

debugger::EvaluateOnCallFrameRequest::EvaluateOnCallFrameRequest(
    const JSONObject *obj)
    : Request("Debugger.evaluateOnCallFrame") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(callFrameId, params, "callFrameId");
  assign(expression, params, "expression");
  assign(objectGroup, params, "objectGroup");
  assign(includeCommandLineAPI, params, "includeCommandLineAPI");
  assign(silent, params, "silent");
  assign(returnByValue, params, "returnByValue");
  assign(generatePreview, params, "generatePreview");
  assign(throwOnSideEffect, params, "throwOnSideEffect");
}

JSONValue *debugger::EvaluateOnCallFrameRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 8> paramsProps;
  put(paramsProps, "callFrameId", callFrameId, factory);
  put(paramsProps, "expression", expression, factory);
  put(paramsProps, "objectGroup", objectGroup, factory);
  put(paramsProps, "includeCommandLineAPI", includeCommandLineAPI, factory);
  put(paramsProps, "silent", silent, factory);
  put(paramsProps, "returnByValue", returnByValue, factory);
  put(paramsProps, "generatePreview", generatePreview, factory);
  put(paramsProps, "throwOnSideEffect", throwOnSideEffect, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void debugger::EvaluateOnCallFrameRequest::accept(
    RequestHandler &handler) const {
  handler.handle(*this);
}

debugger::PauseRequest::PauseRequest() : Request("Debugger.pause") {}

debugger::PauseRequest::PauseRequest(const JSONObject *obj)
    : Request("Debugger.pause") {
  assign(id, obj, "id");
  assign(method, obj, "method");
}

JSONValue *debugger::PauseRequest::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  return factory.newObject(props.begin(), props.end());
}

void debugger::PauseRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

debugger::RemoveBreakpointRequest::RemoveBreakpointRequest()
    : Request("Debugger.removeBreakpoint") {}

debugger::RemoveBreakpointRequest::RemoveBreakpointRequest(
    const JSONObject *obj)
    : Request("Debugger.removeBreakpoint") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(breakpointId, params, "breakpointId");
}

JSONValue *debugger::RemoveBreakpointRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> paramsProps;
  put(paramsProps, "breakpointId", breakpointId, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void debugger::RemoveBreakpointRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

debugger::ResumeRequest::ResumeRequest() : Request("Debugger.resume") {}

debugger::ResumeRequest::ResumeRequest(const JSONObject *obj)
    : Request("Debugger.resume") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  JSONValue *p = safeGet(obj, "params");
  if (p != nullptr) {
    auto *params = valueFromJson<JSONObject *>(p);
    assign(terminateOnResume, params, "terminateOnResume");
  }
}

JSONValue *debugger::ResumeRequest::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> paramsProps;
  put(paramsProps, "terminateOnResume", terminateOnResume, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void debugger::ResumeRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

debugger::SetBreakpointRequest::SetBreakpointRequest()
    : Request("Debugger.setBreakpoint") {}

debugger::SetBreakpointRequest::SetBreakpointRequest(const JSONObject *obj)
    : Request("Debugger.setBreakpoint") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(location, params, "location");
  assign(condition, params, "condition");
}

JSONValue *debugger::SetBreakpointRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> paramsProps;
  put(paramsProps, "location", location, factory);
  put(paramsProps, "condition", condition, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void debugger::SetBreakpointRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

debugger::SetBreakpointByUrlRequest::SetBreakpointByUrlRequest()
    : Request("Debugger.setBreakpointByUrl") {}

debugger::SetBreakpointByUrlRequest::SetBreakpointByUrlRequest(
    const JSONObject *obj)
    : Request("Debugger.setBreakpointByUrl") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(lineNumber, params, "lineNumber");
  assign(url, params, "url");
  assign(urlRegex, params, "urlRegex");
  assign(scriptHash, params, "scriptHash");
  assign(columnNumber, params, "columnNumber");
  assign(condition, params, "condition");
}

JSONValue *debugger::SetBreakpointByUrlRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 6> paramsProps;
  put(paramsProps, "lineNumber", lineNumber, factory);
  put(paramsProps, "url", url, factory);
  put(paramsProps, "urlRegex", urlRegex, factory);
  put(paramsProps, "scriptHash", scriptHash, factory);
  put(paramsProps, "columnNumber", columnNumber, factory);
  put(paramsProps, "condition", condition, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void debugger::SetBreakpointByUrlRequest::accept(
    RequestHandler &handler) const {
  handler.handle(*this);
}

debugger::SetBreakpointsActiveRequest::SetBreakpointsActiveRequest()
    : Request("Debugger.setBreakpointsActive") {}

debugger::SetBreakpointsActiveRequest::SetBreakpointsActiveRequest(
    const JSONObject *obj)
    : Request("Debugger.setBreakpointsActive") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(active, params, "active");
}

JSONValue *debugger::SetBreakpointsActiveRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> paramsProps;
  put(paramsProps, "active", active, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void debugger::SetBreakpointsActiveRequest::accept(
    RequestHandler &handler) const {
  handler.handle(*this);
}

debugger::SetInstrumentationBreakpointRequest::
    SetInstrumentationBreakpointRequest()
    : Request("Debugger.setInstrumentationBreakpoint") {}

debugger::SetInstrumentationBreakpointRequest::
    SetInstrumentationBreakpointRequest(const JSONObject *obj)
    : Request("Debugger.setInstrumentationBreakpoint") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(instrumentation, params, "instrumentation");
}

JSONValue *debugger::SetInstrumentationBreakpointRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> paramsProps;
  put(paramsProps, "instrumentation", instrumentation, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void debugger::SetInstrumentationBreakpointRequest::accept(
    RequestHandler &handler) const {
  handler.handle(*this);
}

debugger::SetPauseOnExceptionsRequest::SetPauseOnExceptionsRequest()
    : Request("Debugger.setPauseOnExceptions") {}

debugger::SetPauseOnExceptionsRequest::SetPauseOnExceptionsRequest(
    const JSONObject *obj)
    : Request("Debugger.setPauseOnExceptions") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(state, params, "state");
}

JSONValue *debugger::SetPauseOnExceptionsRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> paramsProps;
  put(paramsProps, "state", state, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void debugger::SetPauseOnExceptionsRequest::accept(
    RequestHandler &handler) const {
  handler.handle(*this);
}

debugger::StepIntoRequest::StepIntoRequest() : Request("Debugger.stepInto") {}

debugger::StepIntoRequest::StepIntoRequest(const JSONObject *obj)
    : Request("Debugger.stepInto") {
  assign(id, obj, "id");
  assign(method, obj, "method");
}

JSONValue *debugger::StepIntoRequest::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  return factory.newObject(props.begin(), props.end());
}

void debugger::StepIntoRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

debugger::StepOutRequest::StepOutRequest() : Request("Debugger.stepOut") {}

debugger::StepOutRequest::StepOutRequest(const JSONObject *obj)
    : Request("Debugger.stepOut") {
  assign(id, obj, "id");
  assign(method, obj, "method");
}

JSONValue *debugger::StepOutRequest::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  return factory.newObject(props.begin(), props.end());
}

void debugger::StepOutRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

debugger::StepOverRequest::StepOverRequest() : Request("Debugger.stepOver") {}

debugger::StepOverRequest::StepOverRequest(const JSONObject *obj)
    : Request("Debugger.stepOver") {
  assign(id, obj, "id");
  assign(method, obj, "method");
}

JSONValue *debugger::StepOverRequest::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  return factory.newObject(props.begin(), props.end());
}

void debugger::StepOverRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

heapProfiler::CollectGarbageRequest::CollectGarbageRequest()
    : Request("HeapProfiler.collectGarbage") {}

heapProfiler::CollectGarbageRequest::CollectGarbageRequest(
    const JSONObject *obj)
    : Request("HeapProfiler.collectGarbage") {
  assign(id, obj, "id");
  assign(method, obj, "method");
}

JSONValue *heapProfiler::CollectGarbageRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  return factory.newObject(props.begin(), props.end());
}

void heapProfiler::CollectGarbageRequest::accept(
    RequestHandler &handler) const {
  handler.handle(*this);
}

heapProfiler::GetHeapObjectIdRequest::GetHeapObjectIdRequest()
    : Request("HeapProfiler.getHeapObjectId") {}

heapProfiler::GetHeapObjectIdRequest::GetHeapObjectIdRequest(
    const JSONObject *obj)
    : Request("HeapProfiler.getHeapObjectId") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(objectId, params, "objectId");
}

JSONValue *heapProfiler::GetHeapObjectIdRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> paramsProps;
  put(paramsProps, "objectId", objectId, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void heapProfiler::GetHeapObjectIdRequest::accept(
    RequestHandler &handler) const {
  handler.handle(*this);
}

heapProfiler::GetObjectByHeapObjectIdRequest::GetObjectByHeapObjectIdRequest()
    : Request("HeapProfiler.getObjectByHeapObjectId") {}

heapProfiler::GetObjectByHeapObjectIdRequest::GetObjectByHeapObjectIdRequest(
    const JSONObject *obj)
    : Request("HeapProfiler.getObjectByHeapObjectId") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(objectId, params, "objectId");
  assign(objectGroup, params, "objectGroup");
}

JSONValue *heapProfiler::GetObjectByHeapObjectIdRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> paramsProps;
  put(paramsProps, "objectId", objectId, factory);
  put(paramsProps, "objectGroup", objectGroup, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void heapProfiler::GetObjectByHeapObjectIdRequest::accept(
    RequestHandler &handler) const {
  handler.handle(*this);
}

heapProfiler::StartSamplingRequest::StartSamplingRequest()
    : Request("HeapProfiler.startSampling") {}

heapProfiler::StartSamplingRequest::StartSamplingRequest(const JSONObject *obj)
    : Request("HeapProfiler.startSampling") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  JSONValue *p = safeGet(obj, "params");
  if (p != nullptr) {
    auto *params = valueFromJson<JSONObject *>(p);
    assign(samplingInterval, params, "samplingInterval");
    assign(
        includeObjectsCollectedByMajorGC,
        params,
        "includeObjectsCollectedByMajorGC");
    assign(
        includeObjectsCollectedByMinorGC,
        params,
        "includeObjectsCollectedByMinorGC");
  }
}

JSONValue *heapProfiler::StartSamplingRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> paramsProps;
  put(paramsProps, "samplingInterval", samplingInterval, factory);
  put(paramsProps,
      "includeObjectsCollectedByMajorGC",
      includeObjectsCollectedByMajorGC,
      factory);
  put(paramsProps,
      "includeObjectsCollectedByMinorGC",
      includeObjectsCollectedByMinorGC,
      factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void heapProfiler::StartSamplingRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

heapProfiler::StartTrackingHeapObjectsRequest::StartTrackingHeapObjectsRequest()
    : Request("HeapProfiler.startTrackingHeapObjects") {}

heapProfiler::StartTrackingHeapObjectsRequest::StartTrackingHeapObjectsRequest(
    const JSONObject *obj)
    : Request("HeapProfiler.startTrackingHeapObjects") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  JSONValue *p = safeGet(obj, "params");
  if (p != nullptr) {
    auto *params = valueFromJson<JSONObject *>(p);
    assign(trackAllocations, params, "trackAllocations");
  }
}

JSONValue *heapProfiler::StartTrackingHeapObjectsRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> paramsProps;
  put(paramsProps, "trackAllocations", trackAllocations, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void heapProfiler::StartTrackingHeapObjectsRequest::accept(
    RequestHandler &handler) const {
  handler.handle(*this);
}

heapProfiler::StopSamplingRequest::StopSamplingRequest()
    : Request("HeapProfiler.stopSampling") {}

heapProfiler::StopSamplingRequest::StopSamplingRequest(const JSONObject *obj)
    : Request("HeapProfiler.stopSampling") {
  assign(id, obj, "id");
  assign(method, obj, "method");
}

JSONValue *heapProfiler::StopSamplingRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  return factory.newObject(props.begin(), props.end());
}

void heapProfiler::StopSamplingRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

heapProfiler::StopTrackingHeapObjectsRequest::StopTrackingHeapObjectsRequest()
    : Request("HeapProfiler.stopTrackingHeapObjects") {}

heapProfiler::StopTrackingHeapObjectsRequest::StopTrackingHeapObjectsRequest(
    const JSONObject *obj)
    : Request("HeapProfiler.stopTrackingHeapObjects") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  JSONValue *p = safeGet(obj, "params");
  if (p != nullptr) {
    auto *params = valueFromJson<JSONObject *>(p);
    assign(reportProgress, params, "reportProgress");
    assign(treatGlobalObjectsAsRoots, params, "treatGlobalObjectsAsRoots");
    assign(captureNumericValue, params, "captureNumericValue");
  }
}

JSONValue *heapProfiler::StopTrackingHeapObjectsRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> paramsProps;
  put(paramsProps, "reportProgress", reportProgress, factory);
  put(paramsProps,
      "treatGlobalObjectsAsRoots",
      treatGlobalObjectsAsRoots,
      factory);
  put(paramsProps, "captureNumericValue", captureNumericValue, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void heapProfiler::StopTrackingHeapObjectsRequest::accept(
    RequestHandler &handler) const {
  handler.handle(*this);
}

heapProfiler::TakeHeapSnapshotRequest::TakeHeapSnapshotRequest()
    : Request("HeapProfiler.takeHeapSnapshot") {}

heapProfiler::TakeHeapSnapshotRequest::TakeHeapSnapshotRequest(
    const JSONObject *obj)
    : Request("HeapProfiler.takeHeapSnapshot") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  JSONValue *p = safeGet(obj, "params");
  if (p != nullptr) {
    auto *params = valueFromJson<JSONObject *>(p);
    assign(reportProgress, params, "reportProgress");
    assign(treatGlobalObjectsAsRoots, params, "treatGlobalObjectsAsRoots");
    assign(captureNumericValue, params, "captureNumericValue");
  }
}

JSONValue *heapProfiler::TakeHeapSnapshotRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> paramsProps;
  put(paramsProps, "reportProgress", reportProgress, factory);
  put(paramsProps,
      "treatGlobalObjectsAsRoots",
      treatGlobalObjectsAsRoots,
      factory);
  put(paramsProps, "captureNumericValue", captureNumericValue, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void heapProfiler::TakeHeapSnapshotRequest::accept(
    RequestHandler &handler) const {
  handler.handle(*this);
}

profiler::StartRequest::StartRequest() : Request("Profiler.start") {}

profiler::StartRequest::StartRequest(const JSONObject *obj)
    : Request("Profiler.start") {
  assign(id, obj, "id");
  assign(method, obj, "method");
}

JSONValue *profiler::StartRequest::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  return factory.newObject(props.begin(), props.end());
}

void profiler::StartRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

profiler::StopRequest::StopRequest() : Request("Profiler.stop") {}

profiler::StopRequest::StopRequest(const JSONObject *obj)
    : Request("Profiler.stop") {
  assign(id, obj, "id");
  assign(method, obj, "method");
}

JSONValue *profiler::StopRequest::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  return factory.newObject(props.begin(), props.end());
}

void profiler::StopRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

runtime::CallFunctionOnRequest::CallFunctionOnRequest()
    : Request("Runtime.callFunctionOn") {}

runtime::CallFunctionOnRequest::CallFunctionOnRequest(const JSONObject *obj)
    : Request("Runtime.callFunctionOn") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(functionDeclaration, params, "functionDeclaration");
  assign(objectId, params, "objectId");
  assign(arguments, params, "arguments");
  assign(silent, params, "silent");
  assign(returnByValue, params, "returnByValue");
  assign(generatePreview, params, "generatePreview");
  assign(userGesture, params, "userGesture");
  assign(awaitPromise, params, "awaitPromise");
  assign(executionContextId, params, "executionContextId");
  assign(objectGroup, params, "objectGroup");
}

JSONValue *runtime::CallFunctionOnRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 10> paramsProps;
  put(paramsProps, "functionDeclaration", functionDeclaration, factory);
  put(paramsProps, "objectId", objectId, factory);
  put(paramsProps, "arguments", arguments, factory);
  put(paramsProps, "silent", silent, factory);
  put(paramsProps, "returnByValue", returnByValue, factory);
  put(paramsProps, "generatePreview", generatePreview, factory);
  put(paramsProps, "userGesture", userGesture, factory);
  put(paramsProps, "awaitPromise", awaitPromise, factory);
  put(paramsProps, "executionContextId", executionContextId, factory);
  put(paramsProps, "objectGroup", objectGroup, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void runtime::CallFunctionOnRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

runtime::CompileScriptRequest::CompileScriptRequest()
    : Request("Runtime.compileScript") {}

runtime::CompileScriptRequest::CompileScriptRequest(const JSONObject *obj)
    : Request("Runtime.compileScript") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(expression, params, "expression");
  assign(sourceURL, params, "sourceURL");
  assign(persistScript, params, "persistScript");
  assign(executionContextId, params, "executionContextId");
}

JSONValue *runtime::CompileScriptRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 4> paramsProps;
  put(paramsProps, "expression", expression, factory);
  put(paramsProps, "sourceURL", sourceURL, factory);
  put(paramsProps, "persistScript", persistScript, factory);
  put(paramsProps, "executionContextId", executionContextId, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void runtime::CompileScriptRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

runtime::DisableRequest::DisableRequest() : Request("Runtime.disable") {}

runtime::DisableRequest::DisableRequest(const JSONObject *obj)
    : Request("Runtime.disable") {
  assign(id, obj, "id");
  assign(method, obj, "method");
}

JSONValue *runtime::DisableRequest::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  return factory.newObject(props.begin(), props.end());
}

void runtime::DisableRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

runtime::EnableRequest::EnableRequest() : Request("Runtime.enable") {}

runtime::EnableRequest::EnableRequest(const JSONObject *obj)
    : Request("Runtime.enable") {
  assign(id, obj, "id");
  assign(method, obj, "method");
}

JSONValue *runtime::EnableRequest::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  return factory.newObject(props.begin(), props.end());
}

void runtime::EnableRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

runtime::EvaluateRequest::EvaluateRequest() : Request("Runtime.evaluate") {}

runtime::EvaluateRequest::EvaluateRequest(const JSONObject *obj)
    : Request("Runtime.evaluate") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(expression, params, "expression");
  assign(objectGroup, params, "objectGroup");
  assign(includeCommandLineAPI, params, "includeCommandLineAPI");
  assign(silent, params, "silent");
  assign(contextId, params, "contextId");
  assign(returnByValue, params, "returnByValue");
  assign(generatePreview, params, "generatePreview");
  assign(userGesture, params, "userGesture");
  assign(awaitPromise, params, "awaitPromise");
}

JSONValue *runtime::EvaluateRequest::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 9> paramsProps;
  put(paramsProps, "expression", expression, factory);
  put(paramsProps, "objectGroup", objectGroup, factory);
  put(paramsProps, "includeCommandLineAPI", includeCommandLineAPI, factory);
  put(paramsProps, "silent", silent, factory);
  put(paramsProps, "contextId", contextId, factory);
  put(paramsProps, "returnByValue", returnByValue, factory);
  put(paramsProps, "generatePreview", generatePreview, factory);
  put(paramsProps, "userGesture", userGesture, factory);
  put(paramsProps, "awaitPromise", awaitPromise, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void runtime::EvaluateRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

runtime::GetHeapUsageRequest::GetHeapUsageRequest()
    : Request("Runtime.getHeapUsage") {}

runtime::GetHeapUsageRequest::GetHeapUsageRequest(const JSONObject *obj)
    : Request("Runtime.getHeapUsage") {
  assign(id, obj, "id");
  assign(method, obj, "method");
}

JSONValue *runtime::GetHeapUsageRequest::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  return factory.newObject(props.begin(), props.end());
}

void runtime::GetHeapUsageRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

runtime::GetPropertiesRequest::GetPropertiesRequest()
    : Request("Runtime.getProperties") {}

runtime::GetPropertiesRequest::GetPropertiesRequest(const JSONObject *obj)
    : Request("Runtime.getProperties") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(objectId, params, "objectId");
  assign(ownProperties, params, "ownProperties");
  assign(generatePreview, params, "generatePreview");
}

JSONValue *runtime::GetPropertiesRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> paramsProps;
  put(paramsProps, "objectId", objectId, factory);
  put(paramsProps, "ownProperties", ownProperties, factory);
  put(paramsProps, "generatePreview", generatePreview, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void runtime::GetPropertiesRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

runtime::GlobalLexicalScopeNamesRequest::GlobalLexicalScopeNamesRequest()
    : Request("Runtime.globalLexicalScopeNames") {}

runtime::GlobalLexicalScopeNamesRequest::GlobalLexicalScopeNamesRequest(
    const JSONObject *obj)
    : Request("Runtime.globalLexicalScopeNames") {
  assign(id, obj, "id");
  assign(method, obj, "method");

  JSONValue *p = safeGet(obj, "params");
  if (p != nullptr) {
    auto *params = valueFromJson<JSONObject *>(p);
    assign(executionContextId, params, "executionContextId");
  }
}

JSONValue *runtime::GlobalLexicalScopeNamesRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> paramsProps;
  put(paramsProps, "executionContextId", executionContextId, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

void runtime::GlobalLexicalScopeNamesRequest::accept(
    RequestHandler &handler) const {
  handler.handle(*this);
}

runtime::RunIfWaitingForDebuggerRequest::RunIfWaitingForDebuggerRequest()
    : Request("Runtime.runIfWaitingForDebugger") {}

runtime::RunIfWaitingForDebuggerRequest::RunIfWaitingForDebuggerRequest(
    const JSONObject *obj)
    : Request("Runtime.runIfWaitingForDebugger") {
  assign(id, obj, "id");
  assign(method, obj, "method");
}

JSONValue *runtime::RunIfWaitingForDebuggerRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  return factory.newObject(props.begin(), props.end());
}

void runtime::RunIfWaitingForDebuggerRequest::accept(
    RequestHandler &handler) const {
  handler.handle(*this);
}

/// Responses
ErrorResponse::ErrorResponse(const JSONObject *obj) {
  assign(id, obj, "id");

  auto *error = valueFromJson<JSONObject *>(get(obj, "error"));

  assign(code, error, "code");
  assign(message, error, "message");
  assignJsonBlob(data, error, "data");
}

JSONValue *ErrorResponse::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> errProps;
  put(errProps, "code", code, factory);
  put(errProps, "message", message, factory);
  putJsonBlob(errProps, "data", data, factory);

  llvh::SmallVector<JSONFactory::Prop, 2> props;
  put(props, "id", id, factory);
  put(props,
      "error",
      factory.newObject(errProps.begin(), errProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

OkResponse::OkResponse(const JSONObject *obj) {
  assign(id, obj, "id");
}

JSONValue *OkResponse::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 0> resProps;

  llvh::SmallVector<JSONFactory::Prop, 2> props;
  put(props, "id", id, factory);
  put(props,
      "result",
      factory.newObject(resProps.begin(), resProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

debugger::EvaluateOnCallFrameResponse::EvaluateOnCallFrameResponse(
    const JSONObject *obj) {
  assign(id, obj, "id");

  auto *res = valueFromJson<JSONObject *>(get(obj, "result"));
  assign(result, res, "result");
  assign(exceptionDetails, res, "exceptionDetails");
}

JSONValue *debugger::EvaluateOnCallFrameResponse::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> resProps;
  put(resProps, "result", result, factory);
  put(resProps, "exceptionDetails", exceptionDetails, factory);

  llvh::SmallVector<JSONFactory::Prop, 2> props;
  put(props, "id", id, factory);
  put(props,
      "result",
      factory.newObject(resProps.begin(), resProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

debugger::SetBreakpointResponse::SetBreakpointResponse(const JSONObject *obj) {
  assign(id, obj, "id");

  auto *res = valueFromJson<JSONObject *>(get(obj, "result"));
  assign(breakpointId, res, "breakpointId");
  assign(actualLocation, res, "actualLocation");
}

JSONValue *debugger::SetBreakpointResponse::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> resProps;
  put(resProps, "breakpointId", breakpointId, factory);
  put(resProps, "actualLocation", actualLocation, factory);

  llvh::SmallVector<JSONFactory::Prop, 2> props;
  put(props, "id", id, factory);
  put(props,
      "result",
      factory.newObject(resProps.begin(), resProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

debugger::SetBreakpointByUrlResponse::SetBreakpointByUrlResponse(
    const JSONObject *obj) {
  assign(id, obj, "id");

  auto *res = valueFromJson<JSONObject *>(get(obj, "result"));
  assign(breakpointId, res, "breakpointId");
  assign(locations, res, "locations");
}

JSONValue *debugger::SetBreakpointByUrlResponse::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> resProps;
  put(resProps, "breakpointId", breakpointId, factory);
  put(resProps, "locations", locations, factory);

  llvh::SmallVector<JSONFactory::Prop, 2> props;
  put(props, "id", id, factory);
  put(props,
      "result",
      factory.newObject(resProps.begin(), resProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

debugger::SetInstrumentationBreakpointResponse::
    SetInstrumentationBreakpointResponse(const JSONObject *obj) {
  assign(id, obj, "id");

  auto *res = valueFromJson<JSONObject *>(get(obj, "result"));
  assign(breakpointId, res, "breakpointId");
}

JSONValue *debugger::SetInstrumentationBreakpointResponse::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> resProps;
  put(resProps, "breakpointId", breakpointId, factory);

  llvh::SmallVector<JSONFactory::Prop, 2> props;
  put(props, "id", id, factory);
  put(props,
      "result",
      factory.newObject(resProps.begin(), resProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

heapProfiler::GetHeapObjectIdResponse::GetHeapObjectIdResponse(
    const JSONObject *obj) {
  assign(id, obj, "id");

  auto *res = valueFromJson<JSONObject *>(get(obj, "result"));
  assign(heapSnapshotObjectId, res, "heapSnapshotObjectId");
}

JSONValue *heapProfiler::GetHeapObjectIdResponse::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> resProps;
  put(resProps, "heapSnapshotObjectId", heapSnapshotObjectId, factory);

  llvh::SmallVector<JSONFactory::Prop, 2> props;
  put(props, "id", id, factory);
  put(props,
      "result",
      factory.newObject(resProps.begin(), resProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

heapProfiler::GetObjectByHeapObjectIdResponse::GetObjectByHeapObjectIdResponse(
    const JSONObject *obj) {
  assign(id, obj, "id");

  auto *res = valueFromJson<JSONObject *>(get(obj, "result"));
  assign(result, res, "result");
}

JSONValue *heapProfiler::GetObjectByHeapObjectIdResponse::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> resProps;
  put(resProps, "result", result, factory);

  llvh::SmallVector<JSONFactory::Prop, 2> props;
  put(props, "id", id, factory);
  put(props,
      "result",
      factory.newObject(resProps.begin(), resProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

heapProfiler::StopSamplingResponse::StopSamplingResponse(
    const JSONObject *obj) {
  assign(id, obj, "id");

  auto *res = valueFromJson<JSONObject *>(get(obj, "result"));
  assign(profile, res, "profile");
}

JSONValue *heapProfiler::StopSamplingResponse::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> resProps;
  put(resProps, "profile", profile, factory);

  llvh::SmallVector<JSONFactory::Prop, 2> props;
  put(props, "id", id, factory);
  put(props,
      "result",
      factory.newObject(resProps.begin(), resProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

profiler::StopResponse::StopResponse(const JSONObject *obj) {
  assign(id, obj, "id");

  auto *res = valueFromJson<JSONObject *>(get(obj, "result"));
  assign(profile, res, "profile");
}

JSONValue *profiler::StopResponse::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> resProps;
  put(resProps, "profile", profile, factory);

  llvh::SmallVector<JSONFactory::Prop, 2> props;
  put(props, "id", id, factory);
  put(props,
      "result",
      factory.newObject(resProps.begin(), resProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::CallFunctionOnResponse::CallFunctionOnResponse(const JSONObject *obj) {
  assign(id, obj, "id");

  auto *res = valueFromJson<JSONObject *>(get(obj, "result"));
  assign(result, res, "result");
  assign(exceptionDetails, res, "exceptionDetails");
}

JSONValue *runtime::CallFunctionOnResponse::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> resProps;
  put(resProps, "result", result, factory);
  put(resProps, "exceptionDetails", exceptionDetails, factory);

  llvh::SmallVector<JSONFactory::Prop, 2> props;
  put(props, "id", id, factory);
  put(props,
      "result",
      factory.newObject(resProps.begin(), resProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::CompileScriptResponse::CompileScriptResponse(const JSONObject *obj) {
  assign(id, obj, "id");

  auto *res = valueFromJson<JSONObject *>(get(obj, "result"));
  assign(scriptId, res, "scriptId");
  assign(exceptionDetails, res, "exceptionDetails");
}

JSONValue *runtime::CompileScriptResponse::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> resProps;
  put(resProps, "scriptId", scriptId, factory);
  put(resProps, "exceptionDetails", exceptionDetails, factory);

  llvh::SmallVector<JSONFactory::Prop, 2> props;
  put(props, "id", id, factory);
  put(props,
      "result",
      factory.newObject(resProps.begin(), resProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::EvaluateResponse::EvaluateResponse(const JSONObject *obj) {
  assign(id, obj, "id");

  auto *res = valueFromJson<JSONObject *>(get(obj, "result"));
  assign(result, res, "result");
  assign(exceptionDetails, res, "exceptionDetails");
}

JSONValue *runtime::EvaluateResponse::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> resProps;
  put(resProps, "result", result, factory);
  put(resProps, "exceptionDetails", exceptionDetails, factory);

  llvh::SmallVector<JSONFactory::Prop, 2> props;
  put(props, "id", id, factory);
  put(props,
      "result",
      factory.newObject(resProps.begin(), resProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::GetHeapUsageResponse::GetHeapUsageResponse(const JSONObject *obj) {
  assign(id, obj, "id");

  auto *res = valueFromJson<JSONObject *>(get(obj, "result"));
  assign(usedSize, res, "usedSize");
  assign(totalSize, res, "totalSize");
}

JSONValue *runtime::GetHeapUsageResponse::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> resProps;
  put(resProps, "usedSize", usedSize, factory);
  put(resProps, "totalSize", totalSize, factory);

  llvh::SmallVector<JSONFactory::Prop, 2> props;
  put(props, "id", id, factory);
  put(props,
      "result",
      factory.newObject(resProps.begin(), resProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::GetPropertiesResponse::GetPropertiesResponse(const JSONObject *obj) {
  assign(id, obj, "id");

  auto *res = valueFromJson<JSONObject *>(get(obj, "result"));
  assign(result, res, "result");
  assign(internalProperties, res, "internalProperties");
  assign(exceptionDetails, res, "exceptionDetails");
}

JSONValue *runtime::GetPropertiesResponse::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> resProps;
  put(resProps, "result", result, factory);
  put(resProps, "internalProperties", internalProperties, factory);
  put(resProps, "exceptionDetails", exceptionDetails, factory);

  llvh::SmallVector<JSONFactory::Prop, 2> props;
  put(props, "id", id, factory);
  put(props,
      "result",
      factory.newObject(resProps.begin(), resProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::GlobalLexicalScopeNamesResponse::GlobalLexicalScopeNamesResponse(
    const JSONObject *obj) {
  assign(id, obj, "id");

  auto *res = valueFromJson<JSONObject *>(get(obj, "result"));
  assign(names, res, "names");
}

JSONValue *runtime::GlobalLexicalScopeNamesResponse::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> resProps;
  put(resProps, "names", names, factory);

  llvh::SmallVector<JSONFactory::Prop, 2> props;
  put(props, "id", id, factory);
  put(props,
      "result",
      factory.newObject(resProps.begin(), resProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

/// Notifications
debugger::BreakpointResolvedNotification::BreakpointResolvedNotification()
    : Notification("Debugger.breakpointResolved") {}

debugger::BreakpointResolvedNotification::BreakpointResolvedNotification(
    const JSONObject *obj)
    : Notification("Debugger.breakpointResolved") {
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(breakpointId, params, "breakpointId");
  assign(location, params, "location");
}

JSONValue *debugger::BreakpointResolvedNotification::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> paramsProps;
  put(paramsProps, "breakpointId", breakpointId, factory);
  put(paramsProps, "location", location, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

debugger::PausedNotification::PausedNotification()
    : Notification("Debugger.paused") {}

debugger::PausedNotification::PausedNotification(const JSONObject *obj)
    : Notification("Debugger.paused") {
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(callFrames, params, "callFrames");
  assign(reason, params, "reason");
  assignJsonBlob(data, params, "data");
  assign(hitBreakpoints, params, "hitBreakpoints");
  assign(asyncStackTrace, params, "asyncStackTrace");
}

JSONValue *debugger::PausedNotification::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 5> paramsProps;
  put(paramsProps, "callFrames", callFrames, factory);
  put(paramsProps, "reason", reason, factory);
  putJsonBlob(paramsProps, "data", data, factory);
  put(paramsProps, "hitBreakpoints", hitBreakpoints, factory);
  put(paramsProps, "asyncStackTrace", asyncStackTrace, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

debugger::ResumedNotification::ResumedNotification()
    : Notification("Debugger.resumed") {}

debugger::ResumedNotification::ResumedNotification(const JSONObject *obj)
    : Notification("Debugger.resumed") {
  assign(method, obj, "method");
}

JSONValue *debugger::ResumedNotification::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "method", method, factory);
  return factory.newObject(props.begin(), props.end());
}

debugger::ScriptParsedNotification::ScriptParsedNotification()
    : Notification("Debugger.scriptParsed") {}

debugger::ScriptParsedNotification::ScriptParsedNotification(
    const JSONObject *obj)
    : Notification("Debugger.scriptParsed") {
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(scriptId, params, "scriptId");
  assign(url, params, "url");
  assign(startLine, params, "startLine");
  assign(startColumn, params, "startColumn");
  assign(endLine, params, "endLine");
  assign(endColumn, params, "endColumn");
  assign(executionContextId, params, "executionContextId");
  assign(hash, params, "hash");
  assignJsonBlob(executionContextAuxData, params, "executionContextAuxData");
  assign(sourceMapURL, params, "sourceMapURL");
  assign(hasSourceURL, params, "hasSourceURL");
  assign(isModule, params, "isModule");
  assign(length, params, "length");
}

JSONValue *debugger::ScriptParsedNotification::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 13> paramsProps;
  put(paramsProps, "scriptId", scriptId, factory);
  put(paramsProps, "url", url, factory);
  put(paramsProps, "startLine", startLine, factory);
  put(paramsProps, "startColumn", startColumn, factory);
  put(paramsProps, "endLine", endLine, factory);
  put(paramsProps, "endColumn", endColumn, factory);
  put(paramsProps, "executionContextId", executionContextId, factory);
  put(paramsProps, "hash", hash, factory);
  putJsonBlob(
      paramsProps, "executionContextAuxData", executionContextAuxData, factory);
  put(paramsProps, "sourceMapURL", sourceMapURL, factory);
  put(paramsProps, "hasSourceURL", hasSourceURL, factory);
  put(paramsProps, "isModule", isModule, factory);
  put(paramsProps, "length", length, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

heapProfiler::AddHeapSnapshotChunkNotification::
    AddHeapSnapshotChunkNotification()
    : Notification("HeapProfiler.addHeapSnapshotChunk") {}

heapProfiler::AddHeapSnapshotChunkNotification::
    AddHeapSnapshotChunkNotification(const JSONObject *obj)
    : Notification("HeapProfiler.addHeapSnapshotChunk") {
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(chunk, params, "chunk");
}

JSONValue *heapProfiler::AddHeapSnapshotChunkNotification::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> paramsProps;
  put(paramsProps, "chunk", chunk, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

heapProfiler::HeapStatsUpdateNotification::HeapStatsUpdateNotification()
    : Notification("HeapProfiler.heapStatsUpdate") {}

heapProfiler::HeapStatsUpdateNotification::HeapStatsUpdateNotification(
    const JSONObject *obj)
    : Notification("HeapProfiler.heapStatsUpdate") {
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(statsUpdate, params, "statsUpdate");
}

JSONValue *heapProfiler::HeapStatsUpdateNotification::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> paramsProps;
  put(paramsProps, "statsUpdate", statsUpdate, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

heapProfiler::LastSeenObjectIdNotification::LastSeenObjectIdNotification()
    : Notification("HeapProfiler.lastSeenObjectId") {}

heapProfiler::LastSeenObjectIdNotification::LastSeenObjectIdNotification(
    const JSONObject *obj)
    : Notification("HeapProfiler.lastSeenObjectId") {
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(lastSeenObjectId, params, "lastSeenObjectId");
  assign(timestamp, params, "timestamp");
}

JSONValue *heapProfiler::LastSeenObjectIdNotification::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> paramsProps;
  put(paramsProps, "lastSeenObjectId", lastSeenObjectId, factory);
  put(paramsProps, "timestamp", timestamp, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

heapProfiler::ReportHeapSnapshotProgressNotification::
    ReportHeapSnapshotProgressNotification()
    : Notification("HeapProfiler.reportHeapSnapshotProgress") {}

heapProfiler::ReportHeapSnapshotProgressNotification::
    ReportHeapSnapshotProgressNotification(const JSONObject *obj)
    : Notification("HeapProfiler.reportHeapSnapshotProgress") {
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(done, params, "done");
  assign(total, params, "total");
  assign(finished, params, "finished");
}

JSONValue *heapProfiler::ReportHeapSnapshotProgressNotification::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> paramsProps;
  put(paramsProps, "done", done, factory);
  put(paramsProps, "total", total, factory);
  put(paramsProps, "finished", finished, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::ConsoleAPICalledNotification::ConsoleAPICalledNotification()
    : Notification("Runtime.consoleAPICalled") {}

runtime::ConsoleAPICalledNotification::ConsoleAPICalledNotification(
    const JSONObject *obj)
    : Notification("Runtime.consoleAPICalled") {
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(type, params, "type");
  assign(args, params, "args");
  assign(executionContextId, params, "executionContextId");
  assign(timestamp, params, "timestamp");
  assign(stackTrace, params, "stackTrace");
}

JSONValue *runtime::ConsoleAPICalledNotification::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 5> paramsProps;
  put(paramsProps, "type", type, factory);
  put(paramsProps, "args", args, factory);
  put(paramsProps, "executionContextId", executionContextId, factory);
  put(paramsProps, "timestamp", timestamp, factory);
  put(paramsProps, "stackTrace", stackTrace, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::ExecutionContextCreatedNotification::
    ExecutionContextCreatedNotification()
    : Notification("Runtime.executionContextCreated") {}

runtime::ExecutionContextCreatedNotification::
    ExecutionContextCreatedNotification(const JSONObject *obj)
    : Notification("Runtime.executionContextCreated") {
  assign(method, obj, "method");

  auto *params = valueFromJson<JSONObject *>(get(obj, "params"));
  assign(context, params, "context");
}

JSONValue *runtime::ExecutionContextCreatedNotification::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> paramsProps;
  put(paramsProps, "context", context, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

} // namespace message
} // namespace chrome
} // namespace inspector
} // namespace hermes
} // namespace facebook
