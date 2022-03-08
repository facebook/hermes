/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// Initialize the internal CallSite prototype.
//===----------------------------------------------------------------------===//

#include "JSLibInternal.h"
#include "hermes/VM/JSCallSite.h"
#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {

void populateCallSitePrototype(Runtime &runtime) {
  auto callSitePrototype = Handle<JSObject>::vmcast(&runtime.callSitePrototype);

  // CallSite.prototype.xxx methods.
  defineMethod(
      runtime,
      callSitePrototype,
      Predefined::getSymbolID(Predefined::getFunctionName),
      nullptr,
      callSitePrototypeGetFunctionName,
      0);
  defineMethod(
      runtime,
      callSitePrototype,
      Predefined::getSymbolID(Predefined::getFileName),
      nullptr,
      callSitePrototypeGetFileName,
      0);
  defineMethod(
      runtime,
      callSitePrototype,
      Predefined::getSymbolID(Predefined::getLineNumber),
      nullptr,
      callSitePrototypeGetLineNumber,
      0);
  defineMethod(
      runtime,
      callSitePrototype,
      Predefined::getSymbolID(Predefined::getColumnNumber),
      nullptr,
      callSitePrototypeGetColumnNumber,
      0);
  defineMethod(
      runtime,
      callSitePrototype,
      Predefined::getSymbolID(Predefined::getBytecodeAddress),
      nullptr,
      callSitePrototypeGetBytecodeAddress,
      0);
  defineMethod(
      runtime,
      callSitePrototype,
      Predefined::getSymbolID(Predefined::isNative),
      nullptr,
      callSitePrototypeIsNative,
      0);
  defineMethod(
      runtime,
      callSitePrototype,
      Predefined::getSymbolID(Predefined::getThis),
      nullptr,
      callSitePrototypeGetThis,
      0);
  defineMethod(
      runtime,
      callSitePrototype,
      Predefined::getSymbolID(Predefined::getTypeName),
      nullptr,
      callSitePrototypeGetTypeName,
      0);
  defineMethod(
      runtime,
      callSitePrototype,
      Predefined::getSymbolID(Predefined::getFunction),
      nullptr,
      callSitePrototypeGetFunction,
      0);
  defineMethod(
      runtime,
      callSitePrototype,
      Predefined::getSymbolID(Predefined::getMethodName),
      nullptr,
      callSitePrototypeGetMethodName,
      0);
  defineMethod(
      runtime,
      callSitePrototype,
      Predefined::getSymbolID(Predefined::getEvalOrigin),
      nullptr,
      callSitePrototypeGetEvalOrigin,
      0);
  defineMethod(
      runtime,
      callSitePrototype,
      Predefined::getSymbolID(Predefined::isToplevel),
      nullptr,
      callSitePrototypeIsToplevel,
      0);
  defineMethod(
      runtime,
      callSitePrototype,
      Predefined::getSymbolID(Predefined::isEval),
      nullptr,
      callSitePrototypeIsEval,
      0);
  defineMethod(
      runtime,
      callSitePrototype,
      Predefined::getSymbolID(Predefined::isConstructor),
      nullptr,
      callSitePrototypeIsConstructor,
      0);
  defineMethod(
      runtime,
      callSitePrototype,
      Predefined::getSymbolID(Predefined::isAsync),
      nullptr,
      callSitePrototypeIsAsync,
      0);
  defineMethod(
      runtime,
      callSitePrototype,
      Predefined::getSymbolID(Predefined::isPromiseAll),
      nullptr,
      callSitePrototypeIsPromiseAll,
      0);
  defineMethod(
      runtime,
      callSitePrototype,
      Predefined::getSymbolID(Predefined::getPromiseIndex),
      nullptr,
      callSitePrototypeGetPromiseIndex,
      0);

  DefinePropertyFlags dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  defineProperty(
      runtime,
      callSitePrototype,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime.getPredefinedStringHandle(Predefined::CallSite),
      dpf);
}

CallResult<HermesValue>
callSitePrototypeGetFunctionName(void *, Runtime &runtime, NativeArgs args) {
  return JSCallSite::getFunctionName(runtime, args.getThisHandle());
}

CallResult<HermesValue>
callSitePrototypeGetFileName(void *, Runtime &runtime, NativeArgs args) {
  return JSCallSite::getFileName(runtime, args.getThisHandle());
}

CallResult<HermesValue>
callSitePrototypeGetLineNumber(void *, Runtime &runtime, NativeArgs args) {
  return JSCallSite::getLineNumber(runtime, args.getThisHandle());
}

CallResult<HermesValue>
callSitePrototypeGetColumnNumber(void *, Runtime &runtime, NativeArgs args) {
  return JSCallSite::getColumnNumber(runtime, args.getThisHandle());
}

CallResult<HermesValue>
callSitePrototypeGetBytecodeAddress(void *, Runtime &runtime, NativeArgs args) {
  return JSCallSite::getBytecodeAddress(runtime, args.getThisHandle());
}

CallResult<HermesValue>
callSitePrototypeIsNative(void *, Runtime &runtime, NativeArgs args) {
  return JSCallSite::isNative(runtime, args.getThisHandle());
}

CallResult<HermesValue>
callSitePrototypeGetThis(void *, Runtime &runtime, NativeArgs args) {
  return JSCallSite::getThis(runtime, args.getThisHandle());
}

CallResult<HermesValue>
callSitePrototypeGetTypeName(void *, Runtime &runtime, NativeArgs args) {
  return JSCallSite::getTypeName(runtime, args.getThisHandle());
}

CallResult<HermesValue>
callSitePrototypeGetFunction(void *, Runtime &runtime, NativeArgs args) {
  return JSCallSite::getFunction(runtime, args.getThisHandle());
}

CallResult<HermesValue>
callSitePrototypeGetMethodName(void *, Runtime &runtime, NativeArgs args) {
  return JSCallSite::getMethodName(runtime, args.getThisHandle());
}

CallResult<HermesValue>
callSitePrototypeGetEvalOrigin(void *, Runtime &runtime, NativeArgs args) {
  return JSCallSite::getEvalOrigin(runtime, args.getThisHandle());
}

CallResult<HermesValue>
callSitePrototypeIsToplevel(void *, Runtime &runtime, NativeArgs args) {
  return JSCallSite::isToplevel(runtime, args.getThisHandle());
}

CallResult<HermesValue>
callSitePrototypeIsEval(void *, Runtime &runtime, NativeArgs args) {
  return JSCallSite::isEval(runtime, args.getThisHandle());
}

CallResult<HermesValue>
callSitePrototypeIsConstructor(void *, Runtime &runtime, NativeArgs args) {
  return JSCallSite::isConstructor(runtime, args.getThisHandle());
}

CallResult<HermesValue>
callSitePrototypeIsAsync(void *, Runtime &runtime, NativeArgs args) {
  return JSCallSite::isAsync(runtime, args.getThisHandle());
}

CallResult<HermesValue>
callSitePrototypeIsPromiseAll(void *, Runtime &runtime, NativeArgs args) {
  return JSCallSite::isPromiseAll(runtime, args.getThisHandle());
}

CallResult<HermesValue>
callSitePrototypeGetPromiseIndex(void *, Runtime &runtime, NativeArgs args) {
  return JSCallSite::getPromiseIndex(runtime, args.getThisHandle());
}

} // namespace vm
} // namespace hermes
