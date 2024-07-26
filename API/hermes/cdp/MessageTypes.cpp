// Copyright (c) Meta Platforms, Inc. and affiliates. All Rights Reserved.
// @generated SignedSource<<1aad3e25cb0b790476e59a2c064e6ccb>>

#include "MessageTypes.h"

#include "MessageTypesInlines.h"

namespace facebook {
namespace hermes {
namespace cdp {
namespace message {

using RequestBuilder = std::unique_ptr<Request> (*)(const JSONObject *obj);

namespace {

template <typename T>
std::unique_ptr<Request> tryMake(const JSONObject *obj) {
  std::unique_ptr<T> t = T::tryMake(obj);
  if (t == nullptr) {
    return nullptr;
  }
  return t;
}

#define TRY_ASSIGN(lhs, obj, key) \
  if (!assign(lhs, obj, key)) {   \
    return nullptr;               \
  }
#define TRY_ASSIGN_JSON_BLOB(lhs, obj, key) \
  if (!assignJsonBlob(lhs, obj, key)) {     \
    return nullptr;                         \
  }
#define TRY_ASSIGN_OPTIONAL_JSON_BLOB(lhs, obj, key) \
  if (!assignOptionalJsonBlob(lhs, obj, key)) {      \
    return nullptr;                                  \
  }

bool assignJsonBlob(
    std::string &field,
    const JSONObject *obj,
    const std::string &key) {
  JSONValue *v = obj->get(key);
  if (v == nullptr) {
    return false;
  }
  field = jsonValToStr(v);
  return true;
}

bool assignOptionalJsonBlob(
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
    std::string blob,
    JSONFactory &factory) {
  JSONString *jsStr = factory.getString(key);
  std::optional<JSONValue *> jsVal = parseStr(blob, factory);

  // Expecting the conversion from string to JSONValue to succeed because
  // it was originally parsed via assignJsonBlob.
  assert(jsVal);

  props.push_back({jsStr, *jsVal});
}

void putOptionalJsonBlob(
    Properties &props,
    const std::string &key,
    optional<std::string> blob,
    JSONFactory &factory) {
  if (blob.has_value()) {
    putJsonBlob(props, key, *blob, factory);
  }
}

} // namespace

std::unique_ptr<Request> Request::fromJson(const std::string &str) {
  static std::unordered_map<std::string, RequestBuilder> builders = {
      {"Debugger.disable", tryMake<debugger::DisableRequest>},
      {"Debugger.enable", tryMake<debugger::EnableRequest>},
      {"Debugger.evaluateOnCallFrame",
       tryMake<debugger::EvaluateOnCallFrameRequest>},
      {"Debugger.pause", tryMake<debugger::PauseRequest>},
      {"Debugger.removeBreakpoint", tryMake<debugger::RemoveBreakpointRequest>},
      {"Debugger.resume", tryMake<debugger::ResumeRequest>},
      {"Debugger.setBreakpoint", tryMake<debugger::SetBreakpointRequest>},
      {"Debugger.setBreakpointByUrl",
       tryMake<debugger::SetBreakpointByUrlRequest>},
      {"Debugger.setBreakpointsActive",
       tryMake<debugger::SetBreakpointsActiveRequest>},
      {"Debugger.setInstrumentationBreakpoint",
       tryMake<debugger::SetInstrumentationBreakpointRequest>},
      {"Debugger.setPauseOnExceptions",
       tryMake<debugger::SetPauseOnExceptionsRequest>},
      {"Debugger.stepInto", tryMake<debugger::StepIntoRequest>},
      {"Debugger.stepOut", tryMake<debugger::StepOutRequest>},
      {"Debugger.stepOver", tryMake<debugger::StepOverRequest>},
      {"HeapProfiler.collectGarbage",
       tryMake<heapProfiler::CollectGarbageRequest>},
      {"HeapProfiler.getHeapObjectId",
       tryMake<heapProfiler::GetHeapObjectIdRequest>},
      {"HeapProfiler.getObjectByHeapObjectId",
       tryMake<heapProfiler::GetObjectByHeapObjectIdRequest>},
      {"HeapProfiler.startSampling",
       tryMake<heapProfiler::StartSamplingRequest>},
      {"HeapProfiler.startTrackingHeapObjects",
       tryMake<heapProfiler::StartTrackingHeapObjectsRequest>},
      {"HeapProfiler.stopSampling", tryMake<heapProfiler::StopSamplingRequest>},
      {"HeapProfiler.stopTrackingHeapObjects",
       tryMake<heapProfiler::StopTrackingHeapObjectsRequest>},
      {"HeapProfiler.takeHeapSnapshot",
       tryMake<heapProfiler::TakeHeapSnapshotRequest>},
      {"Profiler.start", tryMake<profiler::StartRequest>},
      {"Profiler.stop", tryMake<profiler::StopRequest>},
      {"Runtime.callFunctionOn", tryMake<runtime::CallFunctionOnRequest>},
      {"Runtime.compileScript", tryMake<runtime::CompileScriptRequest>},
      {"Runtime.disable", tryMake<runtime::DisableRequest>},
      {"Runtime.discardConsoleEntries",
       tryMake<runtime::DiscardConsoleEntriesRequest>},
      {"Runtime.enable", tryMake<runtime::EnableRequest>},
      {"Runtime.evaluate", tryMake<runtime::EvaluateRequest>},
      {"Runtime.getHeapUsage", tryMake<runtime::GetHeapUsageRequest>},
      {"Runtime.getProperties", tryMake<runtime::GetPropertiesRequest>},
      {"Runtime.globalLexicalScopeNames",
       tryMake<runtime::GlobalLexicalScopeNamesRequest>},
      {"Runtime.runIfWaitingForDebugger",
       tryMake<runtime::RunIfWaitingForDebuggerRequest>},
  };

  JSLexer::Allocator alloc;
  JSONFactory factory(alloc);
  std::optional<JSONObject *> parseResult = parseStrAsJsonObj(str, factory);
  if (!parseResult) {
    return nullptr;
  }
  JSONObject *jsonObj = *parseResult;

  std::string method;

  TRY_ASSIGN(method, jsonObj, "method");

  auto it = builders.find(method);
  if (it == builders.end()) {
    return UnknownRequest::tryMake(jsonObj);
  }

  auto builder = it->second;
  return builder(jsonObj);
}

/// Types
std::unique_ptr<debugger::Location> debugger::Location::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<debugger::Location> type =
      std::make_unique<debugger::Location>();
  TRY_ASSIGN(type->scriptId, obj, "scriptId");
  TRY_ASSIGN(type->lineNumber, obj, "lineNumber");
  TRY_ASSIGN(type->columnNumber, obj, "columnNumber");
  return type;
}

JSONValue *debugger::Location::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> props;

  put(props, "scriptId", scriptId, factory);
  put(props, "lineNumber", lineNumber, factory);
  put(props, "columnNumber", columnNumber, factory);
  return factory.newObject(props.begin(), props.end());
}

std::unique_ptr<runtime::PropertyPreview> runtime::PropertyPreview::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<runtime::PropertyPreview> type =
      std::make_unique<runtime::PropertyPreview>();
  TRY_ASSIGN(type->name, obj, "name");
  TRY_ASSIGN(type->type, obj, "type");
  TRY_ASSIGN(type->value, obj, "value");
  TRY_ASSIGN(type->valuePreview, obj, "valuePreview");
  TRY_ASSIGN(type->subtype, obj, "subtype");
  return type;
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

std::unique_ptr<runtime::EntryPreview> runtime::EntryPreview::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<runtime::EntryPreview> type =
      std::make_unique<runtime::EntryPreview>();
  TRY_ASSIGN(type->key, obj, "key");
  TRY_ASSIGN(type->value, obj, "value");
  return type;
}

JSONValue *runtime::EntryPreview::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> props;

  put(props, "key", key, factory);
  put(props, "value", value, factory);
  return factory.newObject(props.begin(), props.end());
}

std::unique_ptr<runtime::ObjectPreview> runtime::ObjectPreview::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<runtime::ObjectPreview> type =
      std::make_unique<runtime::ObjectPreview>();
  TRY_ASSIGN(type->type, obj, "type");
  TRY_ASSIGN(type->subtype, obj, "subtype");
  TRY_ASSIGN(type->description, obj, "description");
  TRY_ASSIGN(type->overflow, obj, "overflow");
  TRY_ASSIGN(type->properties, obj, "properties");
  TRY_ASSIGN(type->entries, obj, "entries");
  return type;
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

std::unique_ptr<runtime::CustomPreview> runtime::CustomPreview::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<runtime::CustomPreview> type =
      std::make_unique<runtime::CustomPreview>();
  TRY_ASSIGN(type->header, obj, "header");
  TRY_ASSIGN(type->bodyGetterId, obj, "bodyGetterId");
  return type;
}

JSONValue *runtime::CustomPreview::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> props;

  put(props, "header", header, factory);
  put(props, "bodyGetterId", bodyGetterId, factory);
  return factory.newObject(props.begin(), props.end());
}

std::unique_ptr<runtime::RemoteObject> runtime::RemoteObject::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<runtime::RemoteObject> type =
      std::make_unique<runtime::RemoteObject>();
  TRY_ASSIGN(type->type, obj, "type");
  TRY_ASSIGN(type->subtype, obj, "subtype");
  TRY_ASSIGN(type->className, obj, "className");
  TRY_ASSIGN_OPTIONAL_JSON_BLOB(type->value, obj, "value");
  TRY_ASSIGN(type->unserializableValue, obj, "unserializableValue");
  TRY_ASSIGN(type->description, obj, "description");
  TRY_ASSIGN(type->objectId, obj, "objectId");
  TRY_ASSIGN(type->preview, obj, "preview");
  TRY_ASSIGN(type->customPreview, obj, "customPreview");
  return type;
}

JSONValue *runtime::RemoteObject::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 9> props;

  put(props, "type", type, factory);
  put(props, "subtype", subtype, factory);
  put(props, "className", className, factory);
  putOptionalJsonBlob(props, "value", value, factory);
  put(props, "unserializableValue", unserializableValue, factory);
  put(props, "description", description, factory);
  put(props, "objectId", objectId, factory);
  put(props, "preview", preview, factory);
  put(props, "customPreview", customPreview, factory);
  return factory.newObject(props.begin(), props.end());
}

std::unique_ptr<runtime::CallFrame> runtime::CallFrame::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<runtime::CallFrame> type =
      std::make_unique<runtime::CallFrame>();
  TRY_ASSIGN(type->functionName, obj, "functionName");
  TRY_ASSIGN(type->scriptId, obj, "scriptId");
  TRY_ASSIGN(type->url, obj, "url");
  TRY_ASSIGN(type->lineNumber, obj, "lineNumber");
  TRY_ASSIGN(type->columnNumber, obj, "columnNumber");
  return type;
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

std::unique_ptr<runtime::StackTrace> runtime::StackTrace::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<runtime::StackTrace> type =
      std::make_unique<runtime::StackTrace>();
  TRY_ASSIGN(type->description, obj, "description");
  TRY_ASSIGN(type->callFrames, obj, "callFrames");
  TRY_ASSIGN(type->parent, obj, "parent");
  return type;
}

JSONValue *runtime::StackTrace::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> props;

  put(props, "description", description, factory);
  put(props, "callFrames", callFrames, factory);
  put(props, "parent", parent, factory);
  return factory.newObject(props.begin(), props.end());
}

std::unique_ptr<runtime::ExceptionDetails> runtime::ExceptionDetails::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<runtime::ExceptionDetails> type =
      std::make_unique<runtime::ExceptionDetails>();
  TRY_ASSIGN(type->exceptionId, obj, "exceptionId");
  TRY_ASSIGN(type->text, obj, "text");
  TRY_ASSIGN(type->lineNumber, obj, "lineNumber");
  TRY_ASSIGN(type->columnNumber, obj, "columnNumber");
  TRY_ASSIGN(type->scriptId, obj, "scriptId");
  TRY_ASSIGN(type->url, obj, "url");
  TRY_ASSIGN(type->stackTrace, obj, "stackTrace");
  TRY_ASSIGN(type->exception, obj, "exception");
  TRY_ASSIGN(type->executionContextId, obj, "executionContextId");
  return type;
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

std::unique_ptr<debugger::Scope> debugger::Scope::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<debugger::Scope> type = std::make_unique<debugger::Scope>();
  TRY_ASSIGN(type->type, obj, "type");
  TRY_ASSIGN(type->object, obj, "object");
  TRY_ASSIGN(type->name, obj, "name");
  TRY_ASSIGN(type->startLocation, obj, "startLocation");
  TRY_ASSIGN(type->endLocation, obj, "endLocation");
  return type;
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

std::unique_ptr<debugger::CallFrame> debugger::CallFrame::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<debugger::CallFrame> type =
      std::make_unique<debugger::CallFrame>();
  TRY_ASSIGN(type->callFrameId, obj, "callFrameId");
  TRY_ASSIGN(type->functionName, obj, "functionName");
  TRY_ASSIGN(type->functionLocation, obj, "functionLocation");
  TRY_ASSIGN(type->location, obj, "location");
  TRY_ASSIGN(type->url, obj, "url");
  TRY_ASSIGN(type->scopeChain, obj, "scopeChain");
  TRY_ASSIGN(type->thisObj, obj, "this");
  TRY_ASSIGN(type->returnValue, obj, "returnValue");
  return type;
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

std::unique_ptr<heapProfiler::SamplingHeapProfileNode>
heapProfiler::SamplingHeapProfileNode::tryMake(const JSONObject *obj) {
  std::unique_ptr<heapProfiler::SamplingHeapProfileNode> type =
      std::make_unique<heapProfiler::SamplingHeapProfileNode>();
  TRY_ASSIGN(type->callFrame, obj, "callFrame");
  TRY_ASSIGN(type->selfSize, obj, "selfSize");
  TRY_ASSIGN(type->id, obj, "id");
  TRY_ASSIGN(type->children, obj, "children");
  return type;
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

std::unique_ptr<heapProfiler::SamplingHeapProfileSample>
heapProfiler::SamplingHeapProfileSample::tryMake(const JSONObject *obj) {
  std::unique_ptr<heapProfiler::SamplingHeapProfileSample> type =
      std::make_unique<heapProfiler::SamplingHeapProfileSample>();
  TRY_ASSIGN(type->size, obj, "size");
  TRY_ASSIGN(type->nodeId, obj, "nodeId");
  TRY_ASSIGN(type->ordinal, obj, "ordinal");
  return type;
}

JSONValue *heapProfiler::SamplingHeapProfileSample::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> props;

  put(props, "size", size, factory);
  put(props, "nodeId", nodeId, factory);
  put(props, "ordinal", ordinal, factory);
  return factory.newObject(props.begin(), props.end());
}

std::unique_ptr<heapProfiler::SamplingHeapProfile>
heapProfiler::SamplingHeapProfile::tryMake(const JSONObject *obj) {
  std::unique_ptr<heapProfiler::SamplingHeapProfile> type =
      std::make_unique<heapProfiler::SamplingHeapProfile>();
  TRY_ASSIGN(type->head, obj, "head");
  TRY_ASSIGN(type->samples, obj, "samples");
  return type;
}

JSONValue *heapProfiler::SamplingHeapProfile::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> props;

  put(props, "head", head, factory);
  put(props, "samples", samples, factory);
  return factory.newObject(props.begin(), props.end());
}

std::unique_ptr<profiler::PositionTickInfo> profiler::PositionTickInfo::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<profiler::PositionTickInfo> type =
      std::make_unique<profiler::PositionTickInfo>();
  TRY_ASSIGN(type->line, obj, "line");
  TRY_ASSIGN(type->ticks, obj, "ticks");
  return type;
}

JSONValue *profiler::PositionTickInfo::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 2> props;

  put(props, "line", line, factory);
  put(props, "ticks", ticks, factory);
  return factory.newObject(props.begin(), props.end());
}

std::unique_ptr<profiler::ProfileNode> profiler::ProfileNode::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<profiler::ProfileNode> type =
      std::make_unique<profiler::ProfileNode>();
  TRY_ASSIGN(type->id, obj, "id");
  TRY_ASSIGN(type->callFrame, obj, "callFrame");
  TRY_ASSIGN(type->hitCount, obj, "hitCount");
  TRY_ASSIGN(type->children, obj, "children");
  TRY_ASSIGN(type->deoptReason, obj, "deoptReason");
  TRY_ASSIGN(type->positionTicks, obj, "positionTicks");
  return type;
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

std::unique_ptr<profiler::Profile> profiler::Profile::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<profiler::Profile> type =
      std::make_unique<profiler::Profile>();
  TRY_ASSIGN(type->nodes, obj, "nodes");
  TRY_ASSIGN(type->startTime, obj, "startTime");
  TRY_ASSIGN(type->endTime, obj, "endTime");
  TRY_ASSIGN(type->samples, obj, "samples");
  TRY_ASSIGN(type->timeDeltas, obj, "timeDeltas");
  return type;
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

std::unique_ptr<runtime::CallArgument> runtime::CallArgument::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<runtime::CallArgument> type =
      std::make_unique<runtime::CallArgument>();
  TRY_ASSIGN_OPTIONAL_JSON_BLOB(type->value, obj, "value");
  TRY_ASSIGN(type->unserializableValue, obj, "unserializableValue");
  TRY_ASSIGN(type->objectId, obj, "objectId");
  return type;
}

JSONValue *runtime::CallArgument::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> props;

  putOptionalJsonBlob(props, "value", value, factory);
  put(props, "unserializableValue", unserializableValue, factory);
  put(props, "objectId", objectId, factory);
  return factory.newObject(props.begin(), props.end());
}

std::unique_ptr<runtime::ExecutionContextDescription>
runtime::ExecutionContextDescription::tryMake(const JSONObject *obj) {
  std::unique_ptr<runtime::ExecutionContextDescription> type =
      std::make_unique<runtime::ExecutionContextDescription>();
  TRY_ASSIGN(type->id, obj, "id");
  TRY_ASSIGN(type->origin, obj, "origin");
  TRY_ASSIGN(type->name, obj, "name");
  TRY_ASSIGN_OPTIONAL_JSON_BLOB(type->auxData, obj, "auxData");
  return type;
}

JSONValue *runtime::ExecutionContextDescription::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 4> props;

  put(props, "id", id, factory);
  put(props, "origin", origin, factory);
  put(props, "name", name, factory);
  putOptionalJsonBlob(props, "auxData", auxData, factory);
  return factory.newObject(props.begin(), props.end());
}

std::unique_ptr<runtime::PropertyDescriptor>
runtime::PropertyDescriptor::tryMake(const JSONObject *obj) {
  std::unique_ptr<runtime::PropertyDescriptor> type =
      std::make_unique<runtime::PropertyDescriptor>();
  TRY_ASSIGN(type->name, obj, "name");
  TRY_ASSIGN(type->value, obj, "value");
  TRY_ASSIGN(type->writable, obj, "writable");
  TRY_ASSIGN(type->get, obj, "get");
  TRY_ASSIGN(type->set, obj, "set");
  TRY_ASSIGN(type->configurable, obj, "configurable");
  TRY_ASSIGN(type->enumerable, obj, "enumerable");
  TRY_ASSIGN(type->wasThrown, obj, "wasThrown");
  TRY_ASSIGN(type->isOwn, obj, "isOwn");
  TRY_ASSIGN(type->symbol, obj, "symbol");
  return type;
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

std::unique_ptr<runtime::InternalPropertyDescriptor>
runtime::InternalPropertyDescriptor::tryMake(const JSONObject *obj) {
  std::unique_ptr<runtime::InternalPropertyDescriptor> type =
      std::make_unique<runtime::InternalPropertyDescriptor>();
  TRY_ASSIGN(type->name, obj, "name");
  TRY_ASSIGN(type->value, obj, "value");
  return type;
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

std::unique_ptr<UnknownRequest> UnknownRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<UnknownRequest> req = std::make_unique<UnknownRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");
  TRY_ASSIGN_OPTIONAL_JSON_BLOB(req->params, obj, "params");
  return req;
}

JSONValue *UnknownRequest::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  putOptionalJsonBlob(props, "params", params, factory);
  return factory.newObject(props.begin(), props.end());
}

void UnknownRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}

debugger::DisableRequest::DisableRequest() : Request("Debugger.disable") {}

std::unique_ptr<debugger::DisableRequest> debugger::DisableRequest::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<debugger::DisableRequest> req =
      std::make_unique<debugger::DisableRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  return req;
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

std::unique_ptr<debugger::EnableRequest> debugger::EnableRequest::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<debugger::EnableRequest> req =
      std::make_unique<debugger::EnableRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  return req;
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

std::unique_ptr<debugger::EvaluateOnCallFrameRequest>
debugger::EvaluateOnCallFrameRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<debugger::EvaluateOnCallFrameRequest> req =
      std::make_unique<debugger::EvaluateOnCallFrameRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(req->callFrameId, params, "callFrameId");
  TRY_ASSIGN(req->expression, params, "expression");
  TRY_ASSIGN(req->objectGroup, params, "objectGroup");
  TRY_ASSIGN(req->includeCommandLineAPI, params, "includeCommandLineAPI");
  TRY_ASSIGN(req->silent, params, "silent");
  TRY_ASSIGN(req->returnByValue, params, "returnByValue");
  TRY_ASSIGN(req->generatePreview, params, "generatePreview");
  TRY_ASSIGN(req->throwOnSideEffect, params, "throwOnSideEffect");
  return req;
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

std::unique_ptr<debugger::PauseRequest> debugger::PauseRequest::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<debugger::PauseRequest> req =
      std::make_unique<debugger::PauseRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  return req;
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

std::unique_ptr<debugger::RemoveBreakpointRequest>
debugger::RemoveBreakpointRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<debugger::RemoveBreakpointRequest> req =
      std::make_unique<debugger::RemoveBreakpointRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(req->breakpointId, params, "breakpointId");
  return req;
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

std::unique_ptr<debugger::ResumeRequest> debugger::ResumeRequest::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<debugger::ResumeRequest> req =
      std::make_unique<debugger::ResumeRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *p = obj->get("params");
  if (p != nullptr) {
    auto convertResult = valueFromJson<JSONObject *>(p);
    if (!convertResult) {
      return nullptr;
    }
    auto *params = *convertResult;
    TRY_ASSIGN(req->terminateOnResume, params, "terminateOnResume");
  }
  return req;
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

std::unique_ptr<debugger::SetBreakpointRequest>
debugger::SetBreakpointRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<debugger::SetBreakpointRequest> req =
      std::make_unique<debugger::SetBreakpointRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(req->location, params, "location");
  TRY_ASSIGN(req->condition, params, "condition");
  return req;
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

std::unique_ptr<debugger::SetBreakpointByUrlRequest>
debugger::SetBreakpointByUrlRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<debugger::SetBreakpointByUrlRequest> req =
      std::make_unique<debugger::SetBreakpointByUrlRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(req->lineNumber, params, "lineNumber");
  TRY_ASSIGN(req->url, params, "url");
  TRY_ASSIGN(req->urlRegex, params, "urlRegex");
  TRY_ASSIGN(req->scriptHash, params, "scriptHash");
  TRY_ASSIGN(req->columnNumber, params, "columnNumber");
  TRY_ASSIGN(req->condition, params, "condition");
  return req;
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

std::unique_ptr<debugger::SetBreakpointsActiveRequest>
debugger::SetBreakpointsActiveRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<debugger::SetBreakpointsActiveRequest> req =
      std::make_unique<debugger::SetBreakpointsActiveRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(req->active, params, "active");
  return req;
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

std::unique_ptr<debugger::SetInstrumentationBreakpointRequest>
debugger::SetInstrumentationBreakpointRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<debugger::SetInstrumentationBreakpointRequest> req =
      std::make_unique<debugger::SetInstrumentationBreakpointRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(req->instrumentation, params, "instrumentation");
  return req;
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

std::unique_ptr<debugger::SetPauseOnExceptionsRequest>
debugger::SetPauseOnExceptionsRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<debugger::SetPauseOnExceptionsRequest> req =
      std::make_unique<debugger::SetPauseOnExceptionsRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(req->state, params, "state");
  return req;
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

std::unique_ptr<debugger::StepIntoRequest> debugger::StepIntoRequest::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<debugger::StepIntoRequest> req =
      std::make_unique<debugger::StepIntoRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  return req;
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

std::unique_ptr<debugger::StepOutRequest> debugger::StepOutRequest::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<debugger::StepOutRequest> req =
      std::make_unique<debugger::StepOutRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  return req;
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

std::unique_ptr<debugger::StepOverRequest> debugger::StepOverRequest::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<debugger::StepOverRequest> req =
      std::make_unique<debugger::StepOverRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  return req;
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

std::unique_ptr<heapProfiler::CollectGarbageRequest>
heapProfiler::CollectGarbageRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<heapProfiler::CollectGarbageRequest> req =
      std::make_unique<heapProfiler::CollectGarbageRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  return req;
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

std::unique_ptr<heapProfiler::GetHeapObjectIdRequest>
heapProfiler::GetHeapObjectIdRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<heapProfiler::GetHeapObjectIdRequest> req =
      std::make_unique<heapProfiler::GetHeapObjectIdRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(req->objectId, params, "objectId");
  return req;
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

std::unique_ptr<heapProfiler::GetObjectByHeapObjectIdRequest>
heapProfiler::GetObjectByHeapObjectIdRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<heapProfiler::GetObjectByHeapObjectIdRequest> req =
      std::make_unique<heapProfiler::GetObjectByHeapObjectIdRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(req->objectId, params, "objectId");
  TRY_ASSIGN(req->objectGroup, params, "objectGroup");
  return req;
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

std::unique_ptr<heapProfiler::StartSamplingRequest>
heapProfiler::StartSamplingRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<heapProfiler::StartSamplingRequest> req =
      std::make_unique<heapProfiler::StartSamplingRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *p = obj->get("params");
  if (p != nullptr) {
    auto convertResult = valueFromJson<JSONObject *>(p);
    if (!convertResult) {
      return nullptr;
    }
    auto *params = *convertResult;
    TRY_ASSIGN(req->samplingInterval, params, "samplingInterval");
    TRY_ASSIGN(
        req->includeObjectsCollectedByMajorGC,
        params,
        "includeObjectsCollectedByMajorGC");
    TRY_ASSIGN(
        req->includeObjectsCollectedByMinorGC,
        params,
        "includeObjectsCollectedByMinorGC");
  }
  return req;
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

std::unique_ptr<heapProfiler::StartTrackingHeapObjectsRequest>
heapProfiler::StartTrackingHeapObjectsRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<heapProfiler::StartTrackingHeapObjectsRequest> req =
      std::make_unique<heapProfiler::StartTrackingHeapObjectsRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *p = obj->get("params");
  if (p != nullptr) {
    auto convertResult = valueFromJson<JSONObject *>(p);
    if (!convertResult) {
      return nullptr;
    }
    auto *params = *convertResult;
    TRY_ASSIGN(req->trackAllocations, params, "trackAllocations");
  }
  return req;
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

std::unique_ptr<heapProfiler::StopSamplingRequest>
heapProfiler::StopSamplingRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<heapProfiler::StopSamplingRequest> req =
      std::make_unique<heapProfiler::StopSamplingRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  return req;
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

std::unique_ptr<heapProfiler::StopTrackingHeapObjectsRequest>
heapProfiler::StopTrackingHeapObjectsRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<heapProfiler::StopTrackingHeapObjectsRequest> req =
      std::make_unique<heapProfiler::StopTrackingHeapObjectsRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *p = obj->get("params");
  if (p != nullptr) {
    auto convertResult = valueFromJson<JSONObject *>(p);
    if (!convertResult) {
      return nullptr;
    }
    auto *params = *convertResult;
    TRY_ASSIGN(req->reportProgress, params, "reportProgress");
    TRY_ASSIGN(
        req->treatGlobalObjectsAsRoots, params, "treatGlobalObjectsAsRoots");
    TRY_ASSIGN(req->captureNumericValue, params, "captureNumericValue");
  }
  return req;
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

std::unique_ptr<heapProfiler::TakeHeapSnapshotRequest>
heapProfiler::TakeHeapSnapshotRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<heapProfiler::TakeHeapSnapshotRequest> req =
      std::make_unique<heapProfiler::TakeHeapSnapshotRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *p = obj->get("params");
  if (p != nullptr) {
    auto convertResult = valueFromJson<JSONObject *>(p);
    if (!convertResult) {
      return nullptr;
    }
    auto *params = *convertResult;
    TRY_ASSIGN(req->reportProgress, params, "reportProgress");
    TRY_ASSIGN(
        req->treatGlobalObjectsAsRoots, params, "treatGlobalObjectsAsRoots");
    TRY_ASSIGN(req->captureNumericValue, params, "captureNumericValue");
  }
  return req;
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

std::unique_ptr<profiler::StartRequest> profiler::StartRequest::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<profiler::StartRequest> req =
      std::make_unique<profiler::StartRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  return req;
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

std::unique_ptr<profiler::StopRequest> profiler::StopRequest::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<profiler::StopRequest> req =
      std::make_unique<profiler::StopRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  return req;
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

std::unique_ptr<runtime::CallFunctionOnRequest>
runtime::CallFunctionOnRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<runtime::CallFunctionOnRequest> req =
      std::make_unique<runtime::CallFunctionOnRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(req->functionDeclaration, params, "functionDeclaration");
  TRY_ASSIGN(req->objectId, params, "objectId");
  TRY_ASSIGN(req->arguments, params, "arguments");
  TRY_ASSIGN(req->silent, params, "silent");
  TRY_ASSIGN(req->returnByValue, params, "returnByValue");
  TRY_ASSIGN(req->generatePreview, params, "generatePreview");
  TRY_ASSIGN(req->userGesture, params, "userGesture");
  TRY_ASSIGN(req->awaitPromise, params, "awaitPromise");
  TRY_ASSIGN(req->executionContextId, params, "executionContextId");
  TRY_ASSIGN(req->objectGroup, params, "objectGroup");
  return req;
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

std::unique_ptr<runtime::CompileScriptRequest>
runtime::CompileScriptRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<runtime::CompileScriptRequest> req =
      std::make_unique<runtime::CompileScriptRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(req->expression, params, "expression");
  TRY_ASSIGN(req->sourceURL, params, "sourceURL");
  TRY_ASSIGN(req->persistScript, params, "persistScript");
  TRY_ASSIGN(req->executionContextId, params, "executionContextId");
  return req;
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

std::unique_ptr<runtime::DisableRequest> runtime::DisableRequest::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<runtime::DisableRequest> req =
      std::make_unique<runtime::DisableRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  return req;
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

runtime::DiscardConsoleEntriesRequest::DiscardConsoleEntriesRequest()
    : Request("Runtime.discardConsoleEntries") {}

std::unique_ptr<runtime::DiscardConsoleEntriesRequest>
runtime::DiscardConsoleEntriesRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<runtime::DiscardConsoleEntriesRequest> req =
      std::make_unique<runtime::DiscardConsoleEntriesRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  return req;
}

JSONValue *runtime::DiscardConsoleEntriesRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  return factory.newObject(props.begin(), props.end());
}

void runtime::DiscardConsoleEntriesRequest::accept(
    RequestHandler &handler) const {
  handler.handle(*this);
}

runtime::EnableRequest::EnableRequest() : Request("Runtime.enable") {}

std::unique_ptr<runtime::EnableRequest> runtime::EnableRequest::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<runtime::EnableRequest> req =
      std::make_unique<runtime::EnableRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  return req;
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

std::unique_ptr<runtime::EvaluateRequest> runtime::EvaluateRequest::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<runtime::EvaluateRequest> req =
      std::make_unique<runtime::EvaluateRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(req->expression, params, "expression");
  TRY_ASSIGN(req->objectGroup, params, "objectGroup");
  TRY_ASSIGN(req->includeCommandLineAPI, params, "includeCommandLineAPI");
  TRY_ASSIGN(req->silent, params, "silent");
  TRY_ASSIGN(req->contextId, params, "contextId");
  TRY_ASSIGN(req->returnByValue, params, "returnByValue");
  TRY_ASSIGN(req->generatePreview, params, "generatePreview");
  TRY_ASSIGN(req->userGesture, params, "userGesture");
  TRY_ASSIGN(req->awaitPromise, params, "awaitPromise");
  return req;
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

std::unique_ptr<runtime::GetHeapUsageRequest>
runtime::GetHeapUsageRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<runtime::GetHeapUsageRequest> req =
      std::make_unique<runtime::GetHeapUsageRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  return req;
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

std::unique_ptr<runtime::GetPropertiesRequest>
runtime::GetPropertiesRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<runtime::GetPropertiesRequest> req =
      std::make_unique<runtime::GetPropertiesRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(req->objectId, params, "objectId");
  TRY_ASSIGN(req->ownProperties, params, "ownProperties");
  TRY_ASSIGN(req->accessorPropertiesOnly, params, "accessorPropertiesOnly");
  TRY_ASSIGN(req->generatePreview, params, "generatePreview");
  return req;
}

JSONValue *runtime::GetPropertiesRequest::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 4> paramsProps;
  put(paramsProps, "objectId", objectId, factory);
  put(paramsProps, "ownProperties", ownProperties, factory);
  put(paramsProps, "accessorPropertiesOnly", accessorPropertiesOnly, factory);
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

std::unique_ptr<runtime::GlobalLexicalScopeNamesRequest>
runtime::GlobalLexicalScopeNamesRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<runtime::GlobalLexicalScopeNamesRequest> req =
      std::make_unique<runtime::GlobalLexicalScopeNamesRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  JSONValue *p = obj->get("params");
  if (p != nullptr) {
    auto convertResult = valueFromJson<JSONObject *>(p);
    if (!convertResult) {
      return nullptr;
    }
    auto *params = *convertResult;
    TRY_ASSIGN(req->executionContextId, params, "executionContextId");
  }
  return req;
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

std::unique_ptr<runtime::RunIfWaitingForDebuggerRequest>
runtime::RunIfWaitingForDebuggerRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<runtime::RunIfWaitingForDebuggerRequest> req =
      std::make_unique<runtime::RunIfWaitingForDebuggerRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");

  return req;
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
std::unique_ptr<ErrorResponse> ErrorResponse::tryMake(const JSONObject *obj) {
  std::unique_ptr<ErrorResponse> resp = std::make_unique<ErrorResponse>();
  TRY_ASSIGN(resp->id, obj, "id");

  JSONValue *v = obj->get("error");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *error = *convertResult;

  TRY_ASSIGN(resp->code, error, "code");
  TRY_ASSIGN(resp->message, error, "message");
  TRY_ASSIGN_OPTIONAL_JSON_BLOB(resp->data, error, "data");

  return resp;
}

JSONValue *ErrorResponse::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> errProps;
  put(errProps, "code", code, factory);
  put(errProps, "message", message, factory);
  putOptionalJsonBlob(errProps, "data", data, factory);

  llvh::SmallVector<JSONFactory::Prop, 2> props;
  put(props, "id", id, factory);
  put(props,
      "error",
      factory.newObject(errProps.begin(), errProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

std::unique_ptr<OkResponse> OkResponse::tryMake(const JSONObject *obj) {
  std::unique_ptr<OkResponse> resp = std::make_unique<OkResponse>();
  TRY_ASSIGN(resp->id, obj, "id");
  return resp;
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

std::unique_ptr<debugger::EvaluateOnCallFrameResponse>
debugger::EvaluateOnCallFrameResponse::tryMake(const JSONObject *obj) {
  std::unique_ptr<debugger::EvaluateOnCallFrameResponse> resp =
      std::make_unique<debugger::EvaluateOnCallFrameResponse>();
  TRY_ASSIGN(resp->id, obj, "id");

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *res = *convertResult;
  TRY_ASSIGN(resp->result, res, "result");
  TRY_ASSIGN(resp->exceptionDetails, res, "exceptionDetails");
  return resp;
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

std::unique_ptr<debugger::SetBreakpointResponse>
debugger::SetBreakpointResponse::tryMake(const JSONObject *obj) {
  std::unique_ptr<debugger::SetBreakpointResponse> resp =
      std::make_unique<debugger::SetBreakpointResponse>();
  TRY_ASSIGN(resp->id, obj, "id");

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *res = *convertResult;
  TRY_ASSIGN(resp->breakpointId, res, "breakpointId");
  TRY_ASSIGN(resp->actualLocation, res, "actualLocation");
  return resp;
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

std::unique_ptr<debugger::SetBreakpointByUrlResponse>
debugger::SetBreakpointByUrlResponse::tryMake(const JSONObject *obj) {
  std::unique_ptr<debugger::SetBreakpointByUrlResponse> resp =
      std::make_unique<debugger::SetBreakpointByUrlResponse>();
  TRY_ASSIGN(resp->id, obj, "id");

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *res = *convertResult;
  TRY_ASSIGN(resp->breakpointId, res, "breakpointId");
  TRY_ASSIGN(resp->locations, res, "locations");
  return resp;
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

std::unique_ptr<debugger::SetInstrumentationBreakpointResponse>
debugger::SetInstrumentationBreakpointResponse::tryMake(const JSONObject *obj) {
  std::unique_ptr<debugger::SetInstrumentationBreakpointResponse> resp =
      std::make_unique<debugger::SetInstrumentationBreakpointResponse>();
  TRY_ASSIGN(resp->id, obj, "id");

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *res = *convertResult;
  TRY_ASSIGN(resp->breakpointId, res, "breakpointId");
  return resp;
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

std::unique_ptr<heapProfiler::GetHeapObjectIdResponse>
heapProfiler::GetHeapObjectIdResponse::tryMake(const JSONObject *obj) {
  std::unique_ptr<heapProfiler::GetHeapObjectIdResponse> resp =
      std::make_unique<heapProfiler::GetHeapObjectIdResponse>();
  TRY_ASSIGN(resp->id, obj, "id");

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *res = *convertResult;
  TRY_ASSIGN(resp->heapSnapshotObjectId, res, "heapSnapshotObjectId");
  return resp;
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

std::unique_ptr<heapProfiler::GetObjectByHeapObjectIdResponse>
heapProfiler::GetObjectByHeapObjectIdResponse::tryMake(const JSONObject *obj) {
  std::unique_ptr<heapProfiler::GetObjectByHeapObjectIdResponse> resp =
      std::make_unique<heapProfiler::GetObjectByHeapObjectIdResponse>();
  TRY_ASSIGN(resp->id, obj, "id");

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *res = *convertResult;
  TRY_ASSIGN(resp->result, res, "result");
  return resp;
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

std::unique_ptr<heapProfiler::StopSamplingResponse>
heapProfiler::StopSamplingResponse::tryMake(const JSONObject *obj) {
  std::unique_ptr<heapProfiler::StopSamplingResponse> resp =
      std::make_unique<heapProfiler::StopSamplingResponse>();
  TRY_ASSIGN(resp->id, obj, "id");

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *res = *convertResult;
  TRY_ASSIGN(resp->profile, res, "profile");
  return resp;
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

std::unique_ptr<profiler::StopResponse> profiler::StopResponse::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<profiler::StopResponse> resp =
      std::make_unique<profiler::StopResponse>();
  TRY_ASSIGN(resp->id, obj, "id");

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *res = *convertResult;
  TRY_ASSIGN(resp->profile, res, "profile");
  return resp;
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

std::unique_ptr<runtime::CallFunctionOnResponse>
runtime::CallFunctionOnResponse::tryMake(const JSONObject *obj) {
  std::unique_ptr<runtime::CallFunctionOnResponse> resp =
      std::make_unique<runtime::CallFunctionOnResponse>();
  TRY_ASSIGN(resp->id, obj, "id");

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *res = *convertResult;
  TRY_ASSIGN(resp->result, res, "result");
  TRY_ASSIGN(resp->exceptionDetails, res, "exceptionDetails");
  return resp;
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

std::unique_ptr<runtime::CompileScriptResponse>
runtime::CompileScriptResponse::tryMake(const JSONObject *obj) {
  std::unique_ptr<runtime::CompileScriptResponse> resp =
      std::make_unique<runtime::CompileScriptResponse>();
  TRY_ASSIGN(resp->id, obj, "id");

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *res = *convertResult;
  TRY_ASSIGN(resp->scriptId, res, "scriptId");
  TRY_ASSIGN(resp->exceptionDetails, res, "exceptionDetails");
  return resp;
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

std::unique_ptr<runtime::EvaluateResponse> runtime::EvaluateResponse::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<runtime::EvaluateResponse> resp =
      std::make_unique<runtime::EvaluateResponse>();
  TRY_ASSIGN(resp->id, obj, "id");

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *res = *convertResult;
  TRY_ASSIGN(resp->result, res, "result");
  TRY_ASSIGN(resp->exceptionDetails, res, "exceptionDetails");
  return resp;
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

std::unique_ptr<runtime::GetHeapUsageResponse>
runtime::GetHeapUsageResponse::tryMake(const JSONObject *obj) {
  std::unique_ptr<runtime::GetHeapUsageResponse> resp =
      std::make_unique<runtime::GetHeapUsageResponse>();
  TRY_ASSIGN(resp->id, obj, "id");

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *res = *convertResult;
  TRY_ASSIGN(resp->usedSize, res, "usedSize");
  TRY_ASSIGN(resp->totalSize, res, "totalSize");
  return resp;
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

std::unique_ptr<runtime::GetPropertiesResponse>
runtime::GetPropertiesResponse::tryMake(const JSONObject *obj) {
  std::unique_ptr<runtime::GetPropertiesResponse> resp =
      std::make_unique<runtime::GetPropertiesResponse>();
  TRY_ASSIGN(resp->id, obj, "id");

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *res = *convertResult;
  TRY_ASSIGN(resp->result, res, "result");
  TRY_ASSIGN(resp->internalProperties, res, "internalProperties");
  TRY_ASSIGN(resp->exceptionDetails, res, "exceptionDetails");
  return resp;
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

std::unique_ptr<runtime::GlobalLexicalScopeNamesResponse>
runtime::GlobalLexicalScopeNamesResponse::tryMake(const JSONObject *obj) {
  std::unique_ptr<runtime::GlobalLexicalScopeNamesResponse> resp =
      std::make_unique<runtime::GlobalLexicalScopeNamesResponse>();
  TRY_ASSIGN(resp->id, obj, "id");

  JSONValue *v = obj->get("result");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *res = *convertResult;
  TRY_ASSIGN(resp->names, res, "names");
  return resp;
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

std::unique_ptr<debugger::BreakpointResolvedNotification>
debugger::BreakpointResolvedNotification::tryMake(const JSONObject *obj) {
  std::unique_ptr<debugger::BreakpointResolvedNotification> notif =
      std::make_unique<debugger::BreakpointResolvedNotification>();
  TRY_ASSIGN(notif->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(notif->breakpointId, params, "breakpointId");
  TRY_ASSIGN(notif->location, params, "location");
  return notif;
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

std::unique_ptr<debugger::PausedNotification>
debugger::PausedNotification::tryMake(const JSONObject *obj) {
  std::unique_ptr<debugger::PausedNotification> notif =
      std::make_unique<debugger::PausedNotification>();
  TRY_ASSIGN(notif->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(notif->callFrames, params, "callFrames");
  TRY_ASSIGN(notif->reason, params, "reason");
  TRY_ASSIGN_OPTIONAL_JSON_BLOB(notif->data, params, "data");
  TRY_ASSIGN(notif->hitBreakpoints, params, "hitBreakpoints");
  TRY_ASSIGN(notif->asyncStackTrace, params, "asyncStackTrace");
  return notif;
}

JSONValue *debugger::PausedNotification::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 5> paramsProps;
  put(paramsProps, "callFrames", callFrames, factory);
  put(paramsProps, "reason", reason, factory);
  putOptionalJsonBlob(paramsProps, "data", data, factory);
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

std::unique_ptr<debugger::ResumedNotification>
debugger::ResumedNotification::tryMake(const JSONObject *obj) {
  std::unique_ptr<debugger::ResumedNotification> notif =
      std::make_unique<debugger::ResumedNotification>();
  TRY_ASSIGN(notif->method, obj, "method");

  return notif;
}

JSONValue *debugger::ResumedNotification::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "method", method, factory);
  return factory.newObject(props.begin(), props.end());
}

debugger::ScriptParsedNotification::ScriptParsedNotification()
    : Notification("Debugger.scriptParsed") {}

std::unique_ptr<debugger::ScriptParsedNotification>
debugger::ScriptParsedNotification::tryMake(const JSONObject *obj) {
  std::unique_ptr<debugger::ScriptParsedNotification> notif =
      std::make_unique<debugger::ScriptParsedNotification>();
  TRY_ASSIGN(notif->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(notif->scriptId, params, "scriptId");
  TRY_ASSIGN(notif->url, params, "url");
  TRY_ASSIGN(notif->startLine, params, "startLine");
  TRY_ASSIGN(notif->startColumn, params, "startColumn");
  TRY_ASSIGN(notif->endLine, params, "endLine");
  TRY_ASSIGN(notif->endColumn, params, "endColumn");
  TRY_ASSIGN(notif->executionContextId, params, "executionContextId");
  TRY_ASSIGN(notif->hash, params, "hash");
  TRY_ASSIGN_OPTIONAL_JSON_BLOB(
      notif->executionContextAuxData, params, "executionContextAuxData");
  TRY_ASSIGN(notif->sourceMapURL, params, "sourceMapURL");
  TRY_ASSIGN(notif->hasSourceURL, params, "hasSourceURL");
  TRY_ASSIGN(notif->isModule, params, "isModule");
  TRY_ASSIGN(notif->length, params, "length");
  return notif;
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
  putOptionalJsonBlob(
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

std::unique_ptr<heapProfiler::AddHeapSnapshotChunkNotification>
heapProfiler::AddHeapSnapshotChunkNotification::tryMake(const JSONObject *obj) {
  std::unique_ptr<heapProfiler::AddHeapSnapshotChunkNotification> notif =
      std::make_unique<heapProfiler::AddHeapSnapshotChunkNotification>();
  TRY_ASSIGN(notif->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(notif->chunk, params, "chunk");
  return notif;
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

std::unique_ptr<heapProfiler::HeapStatsUpdateNotification>
heapProfiler::HeapStatsUpdateNotification::tryMake(const JSONObject *obj) {
  std::unique_ptr<heapProfiler::HeapStatsUpdateNotification> notif =
      std::make_unique<heapProfiler::HeapStatsUpdateNotification>();
  TRY_ASSIGN(notif->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(notif->statsUpdate, params, "statsUpdate");
  return notif;
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

std::unique_ptr<heapProfiler::LastSeenObjectIdNotification>
heapProfiler::LastSeenObjectIdNotification::tryMake(const JSONObject *obj) {
  std::unique_ptr<heapProfiler::LastSeenObjectIdNotification> notif =
      std::make_unique<heapProfiler::LastSeenObjectIdNotification>();
  TRY_ASSIGN(notif->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(notif->lastSeenObjectId, params, "lastSeenObjectId");
  TRY_ASSIGN(notif->timestamp, params, "timestamp");
  return notif;
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

std::unique_ptr<heapProfiler::ReportHeapSnapshotProgressNotification>
heapProfiler::ReportHeapSnapshotProgressNotification::tryMake(
    const JSONObject *obj) {
  std::unique_ptr<heapProfiler::ReportHeapSnapshotProgressNotification> notif =
      std::make_unique<heapProfiler::ReportHeapSnapshotProgressNotification>();
  TRY_ASSIGN(notif->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(notif->done, params, "done");
  TRY_ASSIGN(notif->total, params, "total");
  TRY_ASSIGN(notif->finished, params, "finished");
  return notif;
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

std::unique_ptr<runtime::ConsoleAPICalledNotification>
runtime::ConsoleAPICalledNotification::tryMake(const JSONObject *obj) {
  std::unique_ptr<runtime::ConsoleAPICalledNotification> notif =
      std::make_unique<runtime::ConsoleAPICalledNotification>();
  TRY_ASSIGN(notif->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(notif->type, params, "type");
  TRY_ASSIGN(notif->args, params, "args");
  TRY_ASSIGN(notif->executionContextId, params, "executionContextId");
  TRY_ASSIGN(notif->timestamp, params, "timestamp");
  TRY_ASSIGN(notif->stackTrace, params, "stackTrace");
  return notif;
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

std::unique_ptr<runtime::ExecutionContextCreatedNotification>
runtime::ExecutionContextCreatedNotification::tryMake(const JSONObject *obj) {
  std::unique_ptr<runtime::ExecutionContextCreatedNotification> notif =
      std::make_unique<runtime::ExecutionContextCreatedNotification>();
  TRY_ASSIGN(notif->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(notif->context, params, "context");
  return notif;
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

runtime::InspectRequestedNotification::InspectRequestedNotification()
    : Notification("Runtime.inspectRequested") {}

std::unique_ptr<runtime::InspectRequestedNotification>
runtime::InspectRequestedNotification::tryMake(const JSONObject *obj) {
  std::unique_ptr<runtime::InspectRequestedNotification> notif =
      std::make_unique<runtime::InspectRequestedNotification>();
  TRY_ASSIGN(notif->method, obj, "method");

  JSONValue *v = obj->get("params");
  if (v == nullptr) {
    return nullptr;
  }
  auto convertResult = valueFromJson<JSONObject *>(v);
  if (!convertResult) {
    return nullptr;
  }
  auto *params = *convertResult;
  TRY_ASSIGN(notif->object, params, "object");
  TRY_ASSIGN_JSON_BLOB(notif->hints, params, "hints");
  TRY_ASSIGN(notif->executionContextId, params, "executionContextId");
  return notif;
}

JSONValue *runtime::InspectRequestedNotification::toJsonVal(
    JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> paramsProps;
  put(paramsProps, "object", object, factory);
  putJsonBlob(paramsProps, "hints", hints, factory);
  put(paramsProps, "executionContextId", executionContextId, factory);

  llvh::SmallVector<JSONFactory::Prop, 1> props;
  put(props, "method", method, factory);
  put(props,
      "params",
      factory.newObject(paramsProps.begin(), paramsProps.end()),
      factory);
  return factory.newObject(props.begin(), props.end());
}

} // namespace message
} // namespace cdp
} // namespace hermes
} // namespace facebook
