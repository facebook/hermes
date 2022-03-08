/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// ES9 26.2 Proxy Objects
//===----------------------------------------------------------------------===//

#include "JSLibInternal.h"

#include "hermes/VM/JSCallableProxy.h"
#include "hermes/VM/StackFrame-inline.h"

namespace hermes {
namespace vm {

namespace {

// Property storage slots.  This implementation for storing internal
// properties is not a great pattern, and may change.
enum ProxySlotIndexes { revocableProxy, COUNT };

SmallHermesValue getRevocableProxySlot(
    NativeFunction *revoker,
    Runtime &runtime) {
  return NativeFunction::getAdditionalSlotValue(
      revoker, runtime, ProxySlotIndexes::revocableProxy);
}

void setRevocableProxySlot(
    NativeFunction *revoker,
    Runtime &runtime,
    SmallHermesValue value) {
  NativeFunction::setAdditionalSlotValue(
      revoker, runtime, ProxySlotIndexes::revocableProxy, value);
}

// This is shared code between the proxy constructor and the native
// Proxy.revocable factory.
// \param proxy is the newly created but not yet initialized object
// used as the constructor's this.
CallResult<Handle<JSObject>> proxyCreate(
    Runtime &runtime,
    Handle<JSObject> target,
    Handle<JSObject> handler,
    Handle<JSObject> proxy) {
  // 1. If Type(target) is not Object, throw a TypeError exception.
  if (!target) {
    return runtime.raiseTypeError("new Proxy target must be an Object");
  }
  // 3. If Type(handler) is not Object, throw a TypeError exception.
  if (!handler) {
    return runtime.raiseTypeError("new Proxy handler must be an Object");
  }
  // 5. Let P be a newly created object.
  // 6. Set P’s essential internal methods (except for [[Call]] and
  // [[Construct]]) to the definitions specified in 9.5.
  // 7. If IsCallable(target) is true, then
  if (vmisa<Callable>(*target)) {
    //   a. Set P.[[Call]] as specified in 9.5.12.
    //   b. If IsConstructor(target) is true, then
    //     i. Set P.[[Construct]] as specified in 9.5.13.
    // We need to throw away the object passed as this, so we can create
    // a CallableProxy instead.  Simply being a CallableProxy has the
    // effect of setting the [[Call]] and [[Construct]] internal methods.
    proxy = runtime.makeHandle(JSCallableProxy::create(runtime));
  }

  // 8. Set the [[ProxyTarget]] internal slot of P to target.
  // 9. Set the [[ProxyHandler]] internal slot of P to handler.

  JSProxy::setTargetAndHandler(proxy, runtime, target, handler);

  // Return P.
  return proxy;
}

} // namespace

CallResult<HermesValue>
proxyConstructor(void *, Runtime &runtime, NativeArgs args) {
  // 1. If NewTarget is undefined, throw a TypeError exception.
  if (!args.isConstructorCall()) {
    return runtime.raiseTypeError(
        "Proxy() called in function context instead of constructor");
  }
  // 2. Return ? ProxyCreate(target, handler).
  auto proxyRes = proxyCreate(
      runtime,
      args.dyncastArg<JSObject>(0),
      args.dyncastArg<JSObject>(1),
      args.vmcastThis<JSProxy>());
  if (proxyRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  return proxyRes->getHermesValue();
}

CallResult<HermesValue>
proxyRevocationSteps(void *, Runtime &runtime, NativeArgs args) {
  // 1. Let p be F.[[RevocableProxy]].
  auto revoker = vmcast<NativeFunction>(
      runtime.getCurrentFrame()->getCalleeClosureUnsafe());
  SmallHermesValue proxyVal = getRevocableProxySlot(revoker, runtime);
  // 2. If p is null, return undefined.
  if (proxyVal.isNull()) {
    return HermesValue::encodeUndefinedValue();
  }
  // 3. Set F.[[RevocableProxy]] to null.
  setRevocableProxySlot(revoker, runtime, SmallHermesValue::encodeNullValue());
  // 4. Assert: p is a Proxy object.
  JSObject *proxy = vmcast<JSObject>(proxyVal.getObject(runtime));
  assert(proxy->isProxyObject() && "[[RevocableProxy]] is not a Proxy");
  // 5. Set p.[[ProxyTarget]] to null.
  // 6. Set p.[[ProxyHandler]] to null.
  JSProxy::setTargetAndHandler(
      runtime.makeHandle(proxy),
      runtime,
      runtime.makeNullHandle<JSObject>(),
      runtime.makeNullHandle<JSObject>());
  // 7. Return undefined.
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
proxyRevocable(void *, Runtime &runtime, NativeArgs args) {
  // 1. Let p be ? ProxyCreate(target, handler).
  CallResult<Handle<JSObject>> proxyRes = proxyCreate(
      runtime,
      args.dyncastArg<JSObject>(0),
      args.dyncastArg<JSObject>(1),
      runtime.makeHandle(vm::JSProxy::create(runtime)));
  if (proxyRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  // 2. Let steps be the algorithm steps defined in Proxy Revocation Functions.
  // 3. Let revoker be CreateBuiltinFunction(steps, « [[RevocableProxy]] »).
  Handle<NativeFunction> revoker = NativeFunction::createWithoutPrototype(
      runtime,
      nullptr,
      proxyRevocationSteps,
      Predefined::getSymbolID(Predefined::emptyString),
      0,
      ProxySlotIndexes::COUNT);
  // 4. Set revoker.[[RevocableProxy]] to p.
  auto shv =
      SmallHermesValue::encodeHermesValue(proxyRes->getHermesValue(), runtime);
  setRevocableProxySlot(*revoker, runtime, shv);
  // 5. Let result be ObjectCreate(%ObjectPrototype%).
  Handle<JSObject> result = runtime.makeHandle(JSObject::create(runtime));
  // 6. Perform CreateDataProperty(result, "proxy", p).
  auto res1 = JSObject::putNamed_RJS(
      result, runtime, Predefined::getSymbolID(Predefined::proxy), *proxyRes);
  if (res1 == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  assert(*res1 && "Failed to set proxy on Proxy.revocable return");
  // 7. Perform CreateDataProperty(result, "revoke", revoker).
  auto res2 = JSObject::putNamed_RJS(
      result, runtime, Predefined::getSymbolID(Predefined::revoke), revoker);
  if (res2 == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  assert(*res2 && "Failed to set revoke on Proxy.revocable return");
  // 8. Return result.
  return result.getHermesValue();
}

Handle<JSObject> createProxyConstructor(Runtime &runtime) {
  Handle<NativeConstructor> cons = defineSystemConstructor<JSProxy>(
      runtime,
      Predefined::getSymbolID(Predefined::Proxy),
      proxyConstructor,
      runtime.makeNullHandle<JSObject>(),
      2,
      CellKind::JSProxyKind);

  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::revocable),
      nullptr,
      proxyRevocable,
      2);

  return cons;
}

} // namespace vm
} // namespace hermes
