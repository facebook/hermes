/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "RemoteObjectConverters.h"

#include <hermes/inspector/chrome/MessageConverters.h>

namespace facebook {
namespace hermes {
namespace inspector_modern {
namespace chrome {

namespace h = ::facebook::hermes;
namespace m = ::facebook::hermes::inspector_modern::chrome::message;

constexpr size_t kMaxPreviewProperties = 10;

static m::runtime::PropertyPreview generatePropertyPreview(
    facebook::jsi::Runtime &runtime,
    const std::string &name,
    const facebook::jsi::Value &value) {
  m::runtime::PropertyPreview preview;
  preview.name = name;
  if (value.isUndefined()) {
    preview.type = "undefined";
    preview.value = "undefined";
  } else if (value.isNull()) {
    preview.type = "object";
    preview.subtype = "null";
    preview.value = "null";
  } else if (value.isBool()) {
    preview.type = "boolean";
    preview.value = value.toString(runtime).utf8(runtime);
  } else if (value.isNumber()) {
    preview.type = "number";
    preview.value = value.toString(runtime).utf8(runtime);
  } else if (value.isSymbol()) {
    preview.type = "symbol";
    preview.value = value.toString(runtime).utf8(runtime);
  } else if (value.isBigInt()) {
    preview.type = "bigint";
    preview.value =
        value.getBigInt(runtime).toString(runtime).utf8(runtime) + 'n';
  } else if (value.isString()) {
    preview.type = "string";
    preview.value = value.toString(runtime).utf8(runtime);
  } else if (value.isObject()) {
    jsi::Object obj = value.asObject(runtime);
    if (obj.isFunction(runtime)) {
      preview.type = "function";
    } else if (obj.isArray(runtime)) {
      preview.type = "object";
      preview.subtype = "array";
      preview.value = "Array(" +
          std::to_string(obj.getArray(runtime).length(runtime)) + ")";
    } else {
      preview.type = "object";
      preview.value = "Object";
    }
  } else {
    preview.type = "string";
    preview.value = "<UnknownValueKind>";
  }
  return preview;
}

static m::runtime::ObjectPreview generateArrayPreview(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Array &obj) {
  m::runtime::ObjectPreview preview{};
  preview.type = "object";
  preview.subtype = "array";

  size_t count = obj.length(runtime);
  for (size_t i = 0; i < std::min(kMaxPreviewProperties, count); i++) {
    m::runtime::PropertyPreview desc;
    std::string indexString = std::to_string(i);
    try {
      desc = generatePropertyPreview(
          runtime, indexString, obj.getValueAtIndex(runtime, i));
    } catch (const jsi::JSError &) {
      desc.name = indexString;
      desc.type = "string";
      desc.value = "<Exception>";
    }
    preview.properties.push_back(std::move(desc));
  }
  preview.description =
      "Array(" + std::to_string(obj.getArray(runtime).length(runtime)) + ")";
  preview.overflow = count > kMaxPreviewProperties;
  return preview;
}

static m::runtime::ObjectPreview generateObjectPreview(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Object &obj) {
  m::runtime::ObjectPreview preview{};
  preview.type = "object";

  // n.b. own properties only
  jsi::Array propNames =
      runtime.global()
          .getPropertyAsObject(runtime, "Object")
          .getPropertyAsFunction(runtime, "getOwnPropertyNames")
          .call(runtime, obj)
          .getObject(runtime)
          .getArray(runtime);

  size_t propCount = propNames.length(runtime);
  for (size_t i = 0; i < std::min(kMaxPreviewProperties, propCount); i++) {
    jsi::String propName =
        propNames.getValueAtIndex(runtime, i).getString(runtime);

    m::runtime::PropertyPreview desc;
    try {
      // Currently, we fetch the property even if it runs code.
      // Chrome instead detects getters and makes you click to invoke.
      desc = generatePropertyPreview(
          runtime, propName.utf8(runtime), obj.getProperty(runtime, propName));
    } catch (const jsi::JSError &) {
      desc.name = propName.utf8(runtime);
      desc.type = "string";
      desc.value = "<Exception>";
    }
    preview.properties.push_back(std::move(desc));
  }
  preview.description = "Object";
  preview.overflow = propCount > kMaxPreviewProperties;
  return preview;
}

m::debugger::CallFrame m::debugger::makeCallFrame(
    uint32_t callFrameIndex,
    const h::debugger::CallFrameInfo &callFrameInfo,
    const h::debugger::LexicalInfo &lexicalInfo,
    RemoteObjectsTable &objTable,
    jsi::Runtime &runtime,
    const facebook::hermes::debugger::ProgramState &state) {
  m::debugger::CallFrame result;

  result.callFrameId = std::to_string(callFrameIndex);
  result.functionName = callFrameInfo.functionName;
  result.location = makeLocation(callFrameInfo.location);

  uint32_t scopeCount = lexicalInfo.getScopesCount();

  // First we have our local scope (unless we're in the global function)
  if (scopeCount > 1) {
    m::debugger::Scope scope;
    scope.type = "local";
    scope.object.objectId = objTable.addScope(
        std::make_pair(callFrameIndex, 0), BacktraceObjectGroup);
    scope.object.type = "object";
    scope.object.className = "Object";
    result.scopeChain.emplace_back(std::move(scope));
  }

  // Then we have zero or more parent closure scopes
  for (uint32_t scopeIndex = 1; scopeIndex < scopeCount - 1; scopeIndex++) {
    m::debugger::Scope scope;

    scope.type = "closure";
    // TODO: Get the parent closure's name
    scope.name = std::to_string(scopeIndex);
    scope.object.objectId = objTable.addScope(
        std::make_pair(callFrameIndex, scopeIndex), BacktraceObjectGroup);
    scope.object.type = "object";
    scope.object.className = "Object";
    result.scopeChain.emplace_back(std::move(scope));
  }

  // Finally, we always have the global scope
  {
    m::debugger::Scope scope;
    scope.type = "global";
    scope.object.objectId =
        objTable.addValue(runtime.global(), BacktraceObjectGroup);
    scope.object.type = "object";
    scope.object.className = "Object";
    result.scopeChain.emplace_back(std::move(scope));
  }

  result.thisObj.type = "object";
  result.thisObj.objectId = objTable.addValue(
      state.getVariableInfoForThis(callFrameIndex).value, BacktraceObjectGroup);

  return result;
}

std::vector<m::debugger::CallFrame> m::debugger::makeCallFrames(
    const h::debugger::ProgramState &state,
    RemoteObjectsTable &objTable,
    jsi::Runtime &runtime) {
  const h::debugger::StackTrace &stackTrace = state.getStackTrace();
  uint32_t count = stackTrace.callFrameCount();

  std::vector<m::debugger::CallFrame> result;
  result.reserve(count);

  for (uint32_t i = 0; i < count; i++) {
    h::debugger::CallFrameInfo callFrameInfo = stackTrace.callFrameForIndex(i);
    h::debugger::LexicalInfo lexicalInfo = state.getLexicalInfo(i);

    result.emplace_back(
        makeCallFrame(i, callFrameInfo, lexicalInfo, objTable, runtime, state));
  }

  return result;
}

m::runtime::RemoteObject m::runtime::makeRemoteObject(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value,
    RemoteObjectsTable &objTable,
    const std::string &objectGroup,
    bool byValue,
    bool generatePreview) {
  m::runtime::RemoteObject result;
  if (value.isUndefined()) {
    result.type = "undefined";
  } else if (value.isNull()) {
    result.type = "object";
    result.subtype = "null";
    result.value = "null";
  } else if (value.isBool()) {
    result.type = "boolean";
    result.value = value.getBool() ? "true" : "false";
  } else if (value.isNumber()) {
    double numberValue = value.getNumber();
    result.type = "number";
    if (std::isnan(numberValue)) {
      result.description = result.unserializableValue = "NaN";
    } else if (numberValue == -std::numeric_limits<double>::infinity()) {
      result.description = result.unserializableValue = "-Infinity";
    } else if (numberValue == std::numeric_limits<double>::infinity()) {
      result.description = result.unserializableValue = "Infinity";
    } else if (numberValue == 0.0 && std::signbit(numberValue)) {
      result.description = result.unserializableValue = "-0";
    } else {
      result.value = std::to_string(numberValue);
    }
  } else if (value.isString()) {
    result.type = "string";

    // result.value is a blob of well-formed JSON. Therefore, we need to encode
    // the string as JSON.
    std::string encodedValue;
    llvh::raw_string_ostream stream{encodedValue};
    ::hermes::JSONEmitter json{stream};
    json.emitValue(value.getString(runtime).utf8(runtime));
    stream.flush();
    result.value = std::move(encodedValue);
  } else if (value.isSymbol()) {
    result.type = "symbol";
    auto sym = value.getSymbol(runtime);
    result.description = sym.toString(runtime);
    result.objectId =
        objTable.addValue(jsi::Value(std::move(sym)), objectGroup);
  } else if (value.isBigInt()) {
    auto strRepresentation =
        value.getBigInt(runtime).toString(runtime).utf8(runtime) + 'n';
    result.description = result.unserializableValue = strRepresentation;
  } else if (value.isObject()) {
    jsi::Object obj = value.getObject(runtime);
    if (obj.isFunction(runtime)) {
      result.type = "function";
      result.value = "\"\"";
    } else if (obj.isArray(runtime)) {
      auto array = obj.getArray(runtime);
      size_t arrayCount = array.length(runtime);

      result.type = "object";
      result.subtype = "array";
      result.className = "Array";
      result.description = "Array(" + std::to_string(arrayCount) + ")";
      if (generatePreview) {
        result.preview = generateArrayPreview(runtime, array);
      }
    } else {
      result.type = "object";
      result.description = result.className = "Object";
      if (generatePreview) {
        result.preview = generateObjectPreview(runtime, obj);
      }
    }

    if (byValue) {
      // Use JSON.stringify to serialize this object.
      auto JSON = runtime.global().getPropertyAsObject(runtime, "JSON");
      auto stringify = JSON.getPropertyAsFunction(runtime, "stringify");
      facebook::jsi::Value stringifyRes = stringify.call(runtime, value);
      if (!stringifyRes.isString()) {
        result.value = "\"\"";
      } else {
        result.value = stringifyRes.getString(runtime).utf8(runtime);
      }
    } else {
      result.objectId =
          objTable.addValue(jsi::Value(std::move(obj)), objectGroup);
    }
  }

  return result;
}

} // namespace chrome
} // namespace inspector_modern
} // namespace hermes
} // namespace facebook
