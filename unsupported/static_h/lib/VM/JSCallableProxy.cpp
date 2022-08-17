/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSCallableProxy.h"

#include "hermes/VM/JSArray.h"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSCallableProxy

const CallableVTable JSCallableProxy::vt{
    {
        VTable(CellKind::JSCallableProxyKind, cellSize<JSCallableProxy>()),
        JSCallableProxy::_getOwnIndexedRangeImpl,
        JSCallableProxy::_haveOwnIndexedImpl,
        JSCallableProxy::_getOwnIndexedPropertyFlagsImpl,
        JSCallableProxy::_getOwnIndexedImpl,
        JSCallableProxy::_setOwnIndexedImpl,
        JSCallableProxy::_deleteOwnIndexedImpl,
        JSCallableProxy::_checkAllOwnIndexedImpl,
    },
    JSCallableProxy::_newObjectImpl,
    JSCallableProxy::_callImpl,
};

void JSCallableProxyBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSCallableProxy>());
  NativeFunctionBuildMeta(cell, mb);
  const auto *self = static_cast<const JSCallableProxy *>(cell);
  mb.setVTable(&JSCallableProxy::vt);
  mb.addField("@target", &self->slots_.target);
  mb.addField("@handler", &self->slots_.handler);
}

PseudoHandle<JSCallableProxy> JSCallableProxy::create(Runtime &runtime) {
  auto *cproxy = runtime.makeAFixed<JSCallableProxy>(
      runtime,
      Handle<JSObject>::vmcast(&runtime.objectPrototype),
      runtime.getHiddenClassForPrototype(
          runtime.objectPrototypeRawPtr,
          JSObject::numOverlapSlots<JSCallableProxy>()));

  cproxy->flags_.proxyObject = true;

  return JSObjectInit::initToPseudoHandle(runtime, cproxy);
}

CallResult<HermesValue> JSCallableProxy::create(
    Runtime &runtime,
    Handle<JSObject> prototype) {
  assert(
      prototype.get() == runtime.objectPrototypeRawPtr &&
      "JSCallableProxy::create() can only be used with object prototype");
  return create(runtime).getHermesValue();
}

void JSCallableProxy::setTargetAndHandler(
    Runtime &runtime,
    Handle<JSObject> target,
    Handle<JSObject> handler) {
  slots_.target.set(runtime, target.get(), runtime.getHeap());
  slots_.handler.set(runtime, handler.get(), runtime.getHeap());
}

CallResult<bool> JSCallableProxy::isConstructor(Runtime &runtime) {
  ScopedNativeDepthTracker depthTracker(runtime);
  if (LLVM_UNLIKELY(depthTracker.overflowed())) {
    return runtime.raiseStackOverflow(Runtime::StackOverflowKind::NativeStack);
  }
  return vm::isConstructor(
      runtime, vmcast_or_null<Callable>(slots_.target.get(runtime)));
}

CallResult<HermesValue>
JSCallableProxy::_proxyNativeCall(void *, Runtime &runtime, NativeArgs) {
  // We don't use NativeArgs; the implementations just read the current
  // stack frame directly.
  StackFramePtr callerFrame = runtime.getCurrentFrame();
  auto selfHandle =
      Handle<JSCallableProxy>::vmcast(&callerFrame.getCalleeClosureOrCBRef());
  // detail::slots() will always return the slots_ field here, but
  // this isn't a member, so it can't see it.  It doesn't seem to be
  // worth tweaking the abstractions to avoid the small overhead in
  // this case.
  Handle<JSObject> target =
      runtime.makeHandle(detail::slots(*selfHandle).target);
  Predefined::Str trapName = callerFrame->isConstructorCall()
      ? Predefined::construct
      : Predefined::apply;
  CallResult<Handle<Callable>> trapRes =
      detail::findTrap(selfHandle, runtime, trapName);
  if (trapRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  // 6. If trap is undefined, then
  if (!*trapRes) {
    //   a. Return ? Call(target, thisArgument, argumentsList).
    // OR
    //   a. Assert: IsConstructor(target) is true.
    //   b. Return ? Construct(target, argumentsList, newTarget).
    ScopedNativeCallFrame newFrame{
        runtime,
        callerFrame.getArgCount(),
        target.getHermesValue(),
        callerFrame.getNewTargetRef(),
        callerFrame.getThisArgRef()};
    if (LLVM_UNLIKELY(newFrame.overflowed()))
      return runtime.raiseStackOverflow(
          Runtime::StackOverflowKind::NativeStack);
    std::uninitialized_copy_n(
        callerFrame.argsBegin(),
        callerFrame.getArgCount(),
        newFrame->argsBegin());
    // I know statically that target is a Callable, but storing it as
    // a Callable makes it much harder to share all the JSProxy code,
    // so we cast here.
    auto res = Callable::call(Handle<Callable>::vmcast(target), runtime);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return res->get();
  }
  // 7. Let argArray be CreateArrayFromList(argumentsList).
  CallResult<Handle<JSArray>> argArrayRes = JSArray::create(
      runtime, callerFrame->getArgCount(), callerFrame->getArgCount());
  if (LLVM_UNLIKELY(argArrayRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSArray> argArray = *argArrayRes;
  JSArray::setStorageEndIndex(argArray, runtime, callerFrame->getArgCount());
  for (uint32_t i = 0; i < callerFrame->getArgCount(); ++i) {
    const auto shv =
        SmallHermesValue::encodeHermesValue(callerFrame.getArgRef(i), runtime);
    JSArray::unsafeSetExistingElementAt(*argArray, runtime, i, shv);
  }
  // 8. Let newObj be ? Call(trap, handler, « target, argArray, newTarget »).
  if (callerFrame->isConstructorCall()) {
    CallResult<PseudoHandle<>> newObjRes = Callable::executeCall3(
        *trapRes,
        runtime,
        runtime.makeHandle(detail::slots(*selfHandle).handler),
        target.getHermesValue(),
        argArray.getHermesValue(),
        callerFrame.getNewTargetRef());
    if (LLVM_UNLIKELY(newObjRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // 9. If Type(newObj) is not Object, throw a TypeError exception.
    if (!vmisa<JSObject>(newObjRes->get())) {
      return runtime.raiseTypeError("Proxy construct trap returned non-Object");
    }
    // 10. return newObj.
    return newObjRes->get();
  } else {
    // 8. Return ? Call(trap, handler, « target, thisArgument, argArray »).
    auto res = Callable::executeCall3(
        *trapRes,
        runtime,
        runtime.makeHandle(detail::slots(*selfHandle).handler),
        target.getHermesValue(),
        callerFrame.getThisArgRef(),
        argArray.getHermesValue());
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return res->get();
  }
}

CallResult<PseudoHandle<JSObject>> JSCallableProxy::_newObjectImpl(
    Handle<Callable> callable,
    Runtime &runtime,
    Handle<JSObject> protoHandle) {
  CallResult<bool> isConstructorRes =
      vmcast<JSCallableProxy>(*callable)->isConstructor(runtime);
  if (LLVM_UNLIKELY(isConstructorRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (!*isConstructorRes) {
    return runtime.raiseTypeError("Function is not a constructor");
  }
  return vm::Callable::newObject(
      Handle<Callable>::vmcast(
          runtime.makeHandle(detail::slots(*callable).target)),
      runtime,
      protoHandle);
}

} // namespace vm
} // namespace hermes
