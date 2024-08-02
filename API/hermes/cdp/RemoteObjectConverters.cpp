/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "RemoteObjectConverters.h"

#include <hermes/cdp/MessageConverters.h>

namespace facebook {
namespace hermes {
namespace cdp {

namespace h = ::facebook::hermes;
namespace m = ::facebook::hermes::cdp::message;

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
    cdp::RemoteObjectsTable &objTable,
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
        std::make_pair(callFrameIndex, 0), cdp::BacktraceObjectGroup);
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
        std::make_pair(callFrameIndex, scopeIndex), cdp::BacktraceObjectGroup);
    scope.object.type = "object";
    scope.object.className = "Object";
    result.scopeChain.emplace_back(std::move(scope));
  }

  // Finally, we always have the global scope
  {
    m::debugger::Scope scope;
    scope.type = "global";
    scope.object.objectId =
        objTable.addValue(runtime.global(), cdp::BacktraceObjectGroup);
    scope.object.type = "object";
    scope.object.className = "Object";
    result.scopeChain.emplace_back(std::move(scope));
  }

  result.thisObj = runtime::makeRemoteObject(
      runtime,
      state.getVariableInfoForThis(callFrameIndex).value,
      objTable,
      cdp::BacktraceObjectGroup,
      {});

  return result;
}

std::vector<m::debugger::CallFrame> m::debugger::makeCallFrames(
    const h::debugger::ProgramState &state,
    cdp::RemoteObjectsTable &objTable,
    jsi::Runtime &runtime) {
  const h::debugger::StackTrace &stackTrace = state.getStackTrace();
  uint32_t count = stackTrace.callFrameCount();

  std::vector<m::debugger::CallFrame> result;
  result.reserve(count);

  for (uint32_t i = 0; i < count; i++) {
    h::debugger::CallFrameInfo callFrameInfo = stackTrace.callFrameForIndex(i);

    // @cdp This is not explicitly documented for Debugger.CallFrame in the
    // protocol spec, but Chrome DevTools filters out any frames that don't have
    // a valid script in DebuggerModel.ts:
    // https://github.com/facebookexperimental/rn-chrome-devtools-frontend/blob/9a23d4c7c4c2d1a3d9e913af38d6965f474c4284/front_end/core/sdk/DebuggerModel.ts#L1200
    //
    // Furthermore, V8 filters out any frame that isn't user JavaScript in
    // StackFrameBuilder as it visits each frame:
    // https://source.chromium.org/chromium/chromium/src/+/main:v8/src/execution/isolate.cc;l=1439;drc=6a1467666bf8f85bb49fe3d37fce5eba67763061
    //
    // The expectation is that there is no native frames for Debugger.CallFrame,
    // so we filter them out here as well.
    if (callFrameInfo.location.fileId ==
        ::facebook::hermes::debugger::kInvalidLocation) {
      continue;
    }

    h::debugger::LexicalInfo lexicalInfo = state.getLexicalInfo(i);

    result.emplace_back(
        makeCallFrame(i, callFrameInfo, lexicalInfo, objTable, runtime, state));
  }

  return result;
}

m::runtime::RemoteObject m::runtime::makeRemoteObject(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value,
    cdp::RemoteObjectsTable &objTable,
    const std::string &objectGroup,
    const cdp::ObjectSerializationOptions &options) {
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
      /**
       * @cdp Runtime.RemoteObject's description property is a string, but in
       * the case of functions, V8 populates it with the result of toString [1]
       * and Chrome DevTools uses a series of regexes [2] to extract structured
       * information about the function.
       * [1]
       * https://source.chromium.org/chromium/chromium/src/+/main:v8/src/debug/debug-interface.cc;l=138-174;drc=42debe0b0e6bf90175dd0d121eb0e7dc11a6d29c
       * [2]
       * https://github.com/facebookexperimental/rn-chrome-devtools-frontend/blob/9a23d4c7c4c2d1a3d9e913af38d6965f474c4284/front_end/ui/legacy/components/object_ui/ObjectPropertiesSection.ts#L311-L391
       */
      result.description = value.toString(runtime).utf8(runtime);
    } else if (obj.isArray(runtime)) {
      auto array = obj.getArray(runtime);
      size_t arrayCount = array.length(runtime);

      result.type = "object";
      result.subtype = "array";
      result.className = "Array";
      result.description = "Array(" + std::to_string(arrayCount) + ")";
      if (options.generatePreview) {
        result.preview = generateArrayPreview(runtime, array);
      }
    } else {
      result.type = "object";
      result.description = result.className = "Object";
      if (options.generatePreview) {
        result.preview = generateObjectPreview(runtime, obj);
      }
    }

    if (options.returnByValue) {
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

m::runtime::RemoteObject m::runtime::makeRemoteObjectForError(
    jsi::Runtime &runtime,
    const jsi::Value &value,
    cdp::RemoteObjectsTable &objTable,
    const std::string &objectGroup) {
  ObjectSerializationOptions errorSerializationOptions;
  // NOTE: V8 omits the preview for actual Error objects, but we don't
  // make this distinction here.
  errorSerializationOptions.generatePreview = true;
  return m::runtime::makeRemoteObject(
      runtime, value, objTable, objectGroup, errorSerializationOptions);
}

m::runtime::ExceptionDetails m::runtime::makeExceptionDetails(
    jsi::Runtime &runtime,
    const jsi::JSError &error,
    cdp::RemoteObjectsTable &objTable,
    const std::string &objectGroup) {
  m::runtime::ExceptionDetails exceptionDetails;
  exceptionDetails.text = error.getMessage() + "\n" + error.getStack();
  exceptionDetails.exception = m::runtime::makeRemoteObjectForError(
      runtime, error.value(), objTable, objectGroup);
  return exceptionDetails;
}

m::runtime::ExceptionDetails m::runtime::makeExceptionDetails(
    const jsi::JSIException &err) {
  m::runtime::ExceptionDetails exceptionDetails;
  exceptionDetails.text = err.what();
  return exceptionDetails;
}

m::runtime::ExceptionDetails m::runtime::makeExceptionDetails(
    facebook::jsi::Runtime &runtime,
    const h::debugger::EvalResult &result,
    cdp::RemoteObjectsTable &objTable,
    const std::string &objectGroup) {
  assert(result.isException);
  m::runtime::ExceptionDetails exceptionDetails;

  exceptionDetails.text = result.exceptionDetails.text;
  exceptionDetails.scriptId =
      std::to_string(result.exceptionDetails.location.fileId);
  exceptionDetails.url = result.exceptionDetails.location.fileName;
  exceptionDetails.stackTrace = m::runtime::StackTrace();
  exceptionDetails.stackTrace->callFrames =
      makeCallFrames(result.exceptionDetails.getStackTrace());
  m::setChromeLocation(exceptionDetails, result.exceptionDetails.location);
  exceptionDetails.exception = m::runtime::makeRemoteObjectForError(
      runtime, result.value, objTable, objectGroup);

  return exceptionDetails;
}

} // namespace cdp
} // namespace hermes
} // namespace facebook
