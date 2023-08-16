// Copyright (c) Meta Platforms, Inc. and affiliates. All Rights Reserved.
// @generated SignedSource<<488072d3c60d2115a97f9d316269a8b8>>

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

bool assignJsonBlob(
    optional<std::string> &field,
    const JSONObject *obj,
    const std::string &key) {
  JSONValue *v = obj->get(key);
  if (v != nullptr) {
    field = jsonValToStr(v);
  } else {
    field.reset();
  }
  return true;
}

void putJsonBlob(
    Properties &props,
    const std::string &key,
    optional<std::string> blob,
    JSONFactory &factory) {
  if (blob.has_value()) {
    JSONString *jsStr = factory.getString(key);
    std::optional<JSONValue *> jsVal = parseStr(*blob, factory);
    if (!jsVal) {
      throw std::runtime_error("Failed to parse string to JSONValue");
    }
    props.push_back({jsStr, *jsVal});
  }
}

} // namespace

std::unique_ptr<Request> Request::fromJson(const std::string &str) {
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
  std::optional<JSONObject *> parseResult = parseStrAsJsonObj(str, factory);
  if (!parseResult) {
    return nullptr;
  }
  JSONObject *jsonObj = *parseResult;

  std::string method;
  if (!assign(method, jsonObj, "method")) {
    return nullptr;
  }

  auto it = builders.find(method);
  if (it == builders.end()) {
    return std::make_unique<UnknownRequest>(jsonObj);
  }

  auto builder = it->second;
  return builder(jsonObj);
}

/// Types
debugger::Location::Location(const JSONObject *obj) {
  if (!assign(scriptId, obj, "scriptId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(lineNumber, obj, "lineNumber")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(columnNumber, obj, "columnNumber")) {
    throw std::runtime_error("Failed assign");
  }
}

JSONValue *debugger::Location::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> props;

  put(props, "scriptId", scriptId, factory);
  put(props, "lineNumber", lineNumber, factory);
  put(props, "columnNumber", columnNumber, factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::PropertyPreview::PropertyPreview(const JSONObject *obj) {
  if (!assign(name, obj, "name")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(type, obj, "type")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(value, obj, "value")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(valuePreview, obj, "valuePreview")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(subtype, obj, "subtype")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(key, obj, "key")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(value, obj, "value")) {
    throw std::runtime_error("Failed assign");
  }
}

JSONValue *runtime::EntryPreview::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> props;

  put(props, "key", key, factory);
  put(props, "value", value, factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::ObjectPreview::ObjectPreview(const JSONObject *obj) {
  if (!assign(type, obj, "type")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(subtype, obj, "subtype")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(description, obj, "description")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(overflow, obj, "overflow")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(properties, obj, "properties")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(entries, obj, "entries")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(header, obj, "header")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(bodyGetterId, obj, "bodyGetterId")) {
    throw std::runtime_error("Failed assign");
  }
}

JSONValue *runtime::CustomPreview::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> props;

  put(props, "header", header, factory);
  put(props, "bodyGetterId", bodyGetterId, factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::RemoteObject::RemoteObject(const JSONObject *obj) {
  if (!assign(type, obj, "type")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(subtype, obj, "subtype")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(className, obj, "className")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assignJsonBlob(value, obj, "value")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(unserializableValue, obj, "unserializableValue")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(description, obj, "description")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(objectId, obj, "objectId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(preview, obj, "preview")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(customPreview, obj, "customPreview")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(functionName, obj, "functionName")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(scriptId, obj, "scriptId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(url, obj, "url")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(lineNumber, obj, "lineNumber")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(columnNumber, obj, "columnNumber")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(description, obj, "description")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(callFrames, obj, "callFrames")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(parent, obj, "parent")) {
    throw std::runtime_error("Failed assign");
  }
}

JSONValue *runtime::StackTrace::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> props;

  put(props, "description", description, factory);
  put(props, "callFrames", callFrames, factory);
  put(props, "parent", parent, factory);
  return factory.newObject(props.begin(), props.end());
}

runtime::ExceptionDetails::ExceptionDetails(const JSONObject *obj) {
  if (!assign(exceptionId, obj, "exceptionId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(text, obj, "text")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(lineNumber, obj, "lineNumber")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(columnNumber, obj, "columnNumber")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(scriptId, obj, "scriptId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(url, obj, "url")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(stackTrace, obj, "stackTrace")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(exception, obj, "exception")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(executionContextId, obj, "executionContextId")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(type, obj, "type")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(object, obj, "object")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(name, obj, "name")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(startLocation, obj, "startLocation")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(endLocation, obj, "endLocation")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(callFrameId, obj, "callFrameId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(functionName, obj, "functionName")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(functionLocation, obj, "functionLocation")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(location, obj, "location")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(url, obj, "url")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(scopeChain, obj, "scopeChain")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(thisObj, obj, "this")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(returnValue, obj, "returnValue")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(callFrame, obj, "callFrame")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(selfSize, obj, "selfSize")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(children, obj, "children")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(size, obj, "size")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(nodeId, obj, "nodeId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(ordinal, obj, "ordinal")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(head, obj, "head")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(samples, obj, "samples")) {
    throw std::runtime_error("Failed assign");
  }
}

JSONValue *heapProfiler::SamplingHeapProfile::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> props;

  put(props, "head", head, factory);
  put(props, "samples", samples, factory);
  return factory.newObject(props.begin(), props.end());
}

profiler::PositionTickInfo::PositionTickInfo(const JSONObject *obj) {
  if (!assign(line, obj, "line")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(ticks, obj, "ticks")) {
    throw std::runtime_error("Failed assign");
  }
}

JSONValue *profiler::PositionTickInfo::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> props;

  put(props, "line", line, factory);
  put(props, "ticks", ticks, factory);
  return factory.newObject(props.begin(), props.end());
}

profiler::ProfileNode::ProfileNode(const JSONObject *obj) {
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(callFrame, obj, "callFrame")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(hitCount, obj, "hitCount")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(children, obj, "children")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(deoptReason, obj, "deoptReason")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(positionTicks, obj, "positionTicks")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(nodes, obj, "nodes")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(startTime, obj, "startTime")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(endTime, obj, "endTime")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(samples, obj, "samples")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(timeDeltas, obj, "timeDeltas")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assignJsonBlob(value, obj, "value")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(unserializableValue, obj, "unserializableValue")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(objectId, obj, "objectId")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(origin, obj, "origin")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(name, obj, "name")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assignJsonBlob(auxData, obj, "auxData")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(name, obj, "name")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(value, obj, "value")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(writable, obj, "writable")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(get, obj, "get")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(set, obj, "set")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(configurable, obj, "configurable")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(enumerable, obj, "enumerable")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(wasThrown, obj, "wasThrown")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(isOwn, obj, "isOwn")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(symbol, obj, "symbol")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(name, obj, "name")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(value, obj, "value")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assignJsonBlob(params, obj, "params")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(callFrameId, params, "callFrameId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(expression, params, "expression")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(objectGroup, params, "objectGroup")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(includeCommandLineAPI, params, "includeCommandLineAPI")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(silent, params, "silent")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(returnByValue, params, "returnByValue")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(generatePreview, params, "generatePreview")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(throwOnSideEffect, params, "throwOnSideEffect")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(breakpointId, params, "breakpointId")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *p = obj->get("params");
  if (p != nullptr) {
    auto convertResult = valueFromJson<JSONObject *>(p);
    if (convertResult == nullptr) {
      throw std::runtime_error("Failed to convert to JSONObject");
    }
    auto *params = *convertResult;
    if (!assign(terminateOnResume, params, "terminateOnResume")) {
      throw std::runtime_error("Failed assign");
    }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(location, params, "location")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(condition, params, "condition")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(lineNumber, params, "lineNumber")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(url, params, "url")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(urlRegex, params, "urlRegex")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(scriptHash, params, "scriptHash")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(columnNumber, params, "columnNumber")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(condition, params, "condition")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(active, params, "active")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(instrumentation, params, "instrumentation")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(state, params, "state")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(objectId, params, "objectId")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(objectId, params, "objectId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(objectGroup, params, "objectGroup")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *p = obj->get("params");
  if (p != nullptr) {
    auto convertResult = valueFromJson<JSONObject *>(p);
    if (convertResult == nullptr) {
      throw std::runtime_error("Failed to convert to JSONObject");
    }
    auto *params = *convertResult;
    if (!assign(samplingInterval, params, "samplingInterval")) {
      throw std::runtime_error("Failed assign");
    }
    if (!assign(
            includeObjectsCollectedByMajorGC,
            params,
            "includeObjectsCollectedByMajorGC")) {
      throw std::runtime_error("Failed assign");
    }
    if (!assign(
            includeObjectsCollectedByMinorGC,
            params,
            "includeObjectsCollectedByMinorGC")) {
      throw std::runtime_error("Failed assign");
    }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *p = obj->get("params");
  if (p != nullptr) {
    auto convertResult = valueFromJson<JSONObject *>(p);
    if (convertResult == nullptr) {
      throw std::runtime_error("Failed to convert to JSONObject");
    }
    auto *params = *convertResult;
    if (!assign(trackAllocations, params, "trackAllocations")) {
      throw std::runtime_error("Failed assign");
    }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *p = obj->get("params");
  if (p != nullptr) {
    auto convertResult = valueFromJson<JSONObject *>(p);
    if (convertResult == nullptr) {
      throw std::runtime_error("Failed to convert to JSONObject");
    }
    auto *params = *convertResult;
    if (!assign(reportProgress, params, "reportProgress")) {
      throw std::runtime_error("Failed assign");
    }
    if (!assign(
            treatGlobalObjectsAsRoots, params, "treatGlobalObjectsAsRoots")) {
      throw std::runtime_error("Failed assign");
    }
    if (!assign(captureNumericValue, params, "captureNumericValue")) {
      throw std::runtime_error("Failed assign");
    }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *p = obj->get("params");
  if (p != nullptr) {
    auto convertResult = valueFromJson<JSONObject *>(p);
    if (convertResult == nullptr) {
      throw std::runtime_error("Failed to convert to JSONObject");
    }
    auto *params = *convertResult;
    if (!assign(reportProgress, params, "reportProgress")) {
      throw std::runtime_error("Failed assign");
    }
    if (!assign(
            treatGlobalObjectsAsRoots, params, "treatGlobalObjectsAsRoots")) {
      throw std::runtime_error("Failed assign");
    }
    if (!assign(captureNumericValue, params, "captureNumericValue")) {
      throw std::runtime_error("Failed assign");
    }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(functionDeclaration, params, "functionDeclaration")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(objectId, params, "objectId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(arguments, params, "arguments")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(silent, params, "silent")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(returnByValue, params, "returnByValue")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(generatePreview, params, "generatePreview")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(userGesture, params, "userGesture")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(awaitPromise, params, "awaitPromise")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(executionContextId, params, "executionContextId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(objectGroup, params, "objectGroup")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(expression, params, "expression")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(sourceURL, params, "sourceURL")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(persistScript, params, "persistScript")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(executionContextId, params, "executionContextId")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(expression, params, "expression")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(objectGroup, params, "objectGroup")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(includeCommandLineAPI, params, "includeCommandLineAPI")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(silent, params, "silent")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(contextId, params, "contextId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(returnByValue, params, "returnByValue")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(generatePreview, params, "generatePreview")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(userGesture, params, "userGesture")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(awaitPromise, params, "awaitPromise")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(objectId, params, "objectId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(ownProperties, params, "ownProperties")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(generatePreview, params, "generatePreview")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *p = obj->get("params");
  if (p != nullptr) {
    auto convertResult = valueFromJson<JSONObject *>(p);
    if (convertResult == nullptr) {
      throw std::runtime_error("Failed to convert to JSONObject");
    }
    auto *params = *convertResult;
    if (!assign(executionContextId, params, "executionContextId")) {
      throw std::runtime_error("Failed assign");
    }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("error");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *error = *convertResult;

  if (!assign(code, error, "code")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(message, error, "message")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assignJsonBlob(data, error, "data")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *res = *convertResult;
  if (!assign(result, res, "result")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(exceptionDetails, res, "exceptionDetails")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *res = *convertResult;
  if (!assign(breakpointId, res, "breakpointId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(actualLocation, res, "actualLocation")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *res = *convertResult;
  if (!assign(breakpointId, res, "breakpointId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(locations, res, "locations")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *res = *convertResult;
  if (!assign(breakpointId, res, "breakpointId")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *res = *convertResult;
  if (!assign(heapSnapshotObjectId, res, "heapSnapshotObjectId")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *res = *convertResult;
  if (!assign(result, res, "result")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *res = *convertResult;
  if (!assign(profile, res, "profile")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *res = *convertResult;
  if (!assign(profile, res, "profile")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *res = *convertResult;
  if (!assign(result, res, "result")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(exceptionDetails, res, "exceptionDetails")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *res = *convertResult;
  if (!assign(scriptId, res, "scriptId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(exceptionDetails, res, "exceptionDetails")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *res = *convertResult;
  if (!assign(result, res, "result")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(exceptionDetails, res, "exceptionDetails")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *res = *convertResult;
  if (!assign(usedSize, res, "usedSize")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(totalSize, res, "totalSize")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *res = *convertResult;
  if (!assign(result, res, "result")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(internalProperties, res, "internalProperties")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(exceptionDetails, res, "exceptionDetails")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(id, obj, "id")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *res = *convertResult;
  if (!assign(names, res, "names")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(breakpointId, params, "breakpointId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(location, params, "location")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(callFrames, params, "callFrames")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(reason, params, "reason")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assignJsonBlob(data, params, "data")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(hitBreakpoints, params, "hitBreakpoints")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(asyncStackTrace, params, "asyncStackTrace")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(scriptId, params, "scriptId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(url, params, "url")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(startLine, params, "startLine")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(startColumn, params, "startColumn")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(endLine, params, "endLine")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(endColumn, params, "endColumn")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(executionContextId, params, "executionContextId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(hash, params, "hash")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assignJsonBlob(
          executionContextAuxData, params, "executionContextAuxData")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(sourceMapURL, params, "sourceMapURL")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(hasSourceURL, params, "hasSourceURL")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(isModule, params, "isModule")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(length, params, "length")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(chunk, params, "chunk")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(statsUpdate, params, "statsUpdate")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(lastSeenObjectId, params, "lastSeenObjectId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(timestamp, params, "timestamp")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(done, params, "done")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(total, params, "total")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(finished, params, "finished")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(type, params, "type")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(args, params, "args")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(executionContextId, params, "executionContextId")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(timestamp, params, "timestamp")) {
    throw std::runtime_error("Failed assign");
  }
  if (!assign(stackTrace, params, "stackTrace")) {
    throw std::runtime_error("Failed assign");
  }
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
  if (!assign(method, obj, "method")) {
    throw std::runtime_error("Failed assign");
  }

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    throw std::runtime_error("Key not found in JSONObject");
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (convertResult == nullptr) {
    throw std::runtime_error("Failed to convert to JSONObject");
  }
  auto *params = *convertResult;
  if (!assign(context, params, "context")) {
    throw std::runtime_error("Failed assign");
  }
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
