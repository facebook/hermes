/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// ES9 21.1 The Reflect Object
//===----------------------------------------------------------------------===//
#include "JSLibInternal.h"
#include "Object.h"

namespace hermes {
namespace vm {

CallResult<HermesValue>
reflectApply(void *, Runtime &runtime, NativeArgs args) {
  auto target = args.dyncastArg<Callable>(0);
  if (LLVM_UNLIKELY(!target)) {
    return runtime.raiseTypeError("target is not callable");
  }

  auto arguments = args.dyncastArg<JSObject>(2);
  if (LLVM_UNLIKELY(!arguments)) {
    return runtime.raiseTypeError("target arguments is not an object");
  }

  return Callable::executeCall(
             target,
             runtime,
             Runtime::getUndefinedValue(),
             args.getArgHandle(1),
             arguments)
      .toCallResultHermesValue();
}

CallResult<HermesValue>
reflectConstruct(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  Handle<Callable> target = args.dyncastArg<Callable>(0);
  if (LLVM_UNLIKELY(!target)) {
    return runtime.raiseTypeError("target is not constructible");
  }
  CallResult<bool> isConstructorRes = isConstructor(runtime, *target);
  if (LLVM_UNLIKELY(isConstructorRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (LLVM_UNLIKELY(!*isConstructorRes)) {
    return runtime.raiseTypeError("target is not constructible");
  }
  Handle<Callable> newTarget = args.dyncastArg<Callable>(2);
  if (args.getArgCount() >= 3) {
    if (LLVM_UNLIKELY(!newTarget)) {
      return runtime.raiseTypeError("newTarget is not constructible");
    }
    CallResult<bool> isConstructorRes = isConstructor(runtime, *newTarget);
    if (LLVM_UNLIKELY(isConstructorRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (LLVM_UNLIKELY(!*isConstructorRes)) {
      return runtime.raiseTypeError("newTarget is not constructible");
    }
  }
  auto arguments = args.dyncastArg<JSObject>(1);
  if (LLVM_UNLIKELY(!arguments)) {
    return runtime.raiseTypeError("target arguments is not an object");
  }

  MutableHandle<JSObject> prototype{runtime};

  if (newTarget) {
    CallResult<PseudoHandle<>> ntProtoRes = JSObject::getNamed_RJS(
        newTarget, runtime, Predefined::getSymbolID(Predefined::prototype));
    if (LLVM_UNLIKELY(ntProtoRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    prototype = PseudoHandle<JSObject>::dyn_vmcast(std::move(*ntProtoRes));
  }
  if (!prototype) {
    // If newTarget.prototype is not an object, then we need to
    // use a built-in-specific intrinsicDefaultPrototype in
    // OrdinaryCreateFromConstructor (es9 9.1.13).  We don't have
    // this directly available.  However, target.prototype has the
    // right value for builtins, and the prototype property is
    // non-configurable and non-writable for all of them (enforced
    // in Callable::defineNameLengthAndPrototype).  So if
    // newTarget.prototype is not an object, we fall back to using
    // target.prototype.
    CallResult<PseudoHandle<>> tProtoRes = JSObject::getNamed_RJS(
        target, runtime, Predefined::getSymbolID(Predefined::prototype));
    if (LLVM_UNLIKELY(tProtoRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    prototype = PseudoHandle<JSObject>::dyn_vmcast(std::move(*tProtoRes));
    if (!prototype) {
      // If all else fails, use %ObjectPrototype%
      prototype = runtime.objectPrototypeRawPtr;
    }
  }
  assert(prototype && "prototype was never set");
  CallResult<PseudoHandle<JSObject>> thisValRes =
      Callable::newObject(target, runtime, prototype);
  if (LLVM_UNLIKELY(thisValRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  Handle<JSObject> thisVal = runtime.makeHandle(std::move(*thisValRes));
  CallResult<PseudoHandle<>> objRes = Callable::executeCall(
      target, runtime, newTarget ? newTarget : target, thisVal, arguments);
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return (*objRes)->isObject() ? objRes.toCallResultHermesValue()
                               : thisVal.getHermesValue();
}

namespace {

CallResult<HermesValue> toHV(CallResult<bool> boolRes) {
  if (boolRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  return HermesValue::encodeBoolValue(*boolRes);
}

} // namespace

CallResult<HermesValue>
reflectDefineProperty(void *, Runtime &runtime, NativeArgs args) {
  return toHV(defineProperty(runtime, args, PropOpFlags()));
}

CallResult<HermesValue>
reflectDeleteProperty(void *, Runtime &runtime, NativeArgs args) {
  Handle<JSObject> target = args.dyncastArg<JSObject>(0);
  if (LLVM_UNLIKELY(!target)) {
    return runtime.raiseTypeError("target is not an object");
  }

  return toHV(JSObject::deleteComputed(target, runtime, args.getArgHandle(1)));
}

CallResult<HermesValue> reflectGet(void *, Runtime &runtime, NativeArgs args) {
  Handle<JSObject> target = args.dyncastArg<JSObject>(0);
  if (LLVM_UNLIKELY(!target)) {
    return runtime.raiseTypeError("target is not an object");
  }

  return JSObject::getComputedWithReceiver_RJS(
             target,
             runtime,
             args.getArgHandle(1),
             (args.getArgCount() >= 3) ? args.getArgHandle(2) : target)
      .toCallResultHermesValue();
}

CallResult<HermesValue>
reflectGetOwnPropertyDescriptor(void *, Runtime &runtime, NativeArgs args) {
  Handle<JSObject> target = args.dyncastArg<JSObject>(0);
  if (LLVM_UNLIKELY(!target)) {
    return runtime.raiseTypeError("target is not an object");
  }

  return getOwnPropertyDescriptor(runtime, target, args.getArgHandle(1));
}

CallResult<HermesValue>
reflectGetPrototypeOf(void *, Runtime &runtime, NativeArgs args) {
  Handle<JSObject> target = args.dyncastArg<JSObject>(0);
  if (LLVM_UNLIKELY(!target)) {
    return runtime.raiseTypeError("target is not an object");
  }

  return getPrototypeOf(runtime, target);
}

CallResult<HermesValue> reflectHas(void *, Runtime &runtime, NativeArgs args) {
  Handle<JSObject> target = args.dyncastArg<JSObject>(0);
  if (LLVM_UNLIKELY(!target)) {
    return runtime.raiseTypeError("target is not an object");
  }

  return toHV(JSObject::hasComputed(target, runtime, args.getArgHandle(1)));
}

CallResult<HermesValue>
reflectIsExtensible(void *, Runtime &runtime, NativeArgs args) {
  Handle<JSObject> target = args.dyncastArg<JSObject>(0);
  if (LLVM_UNLIKELY(!target)) {
    return runtime.raiseTypeError("target is not an object");
  }

  return toHV(JSObject::isExtensible(createPseudoHandle(*target), runtime));
}

CallResult<HermesValue>
reflectOwnKeys(void *, Runtime &runtime, NativeArgs args) {
  Handle<JSObject> target = args.dyncastArg<JSObject>(0);
  if (LLVM_UNLIKELY(!target)) {
    return runtime.raiseTypeError("target is not an object");
  }

  return getOwnPropertyKeysAsStrings(
      target,
      runtime,
      OwnKeysFlags()
          .plusIncludeSymbols()
          .plusIncludeNonSymbols()
          .plusIncludeNonEnumerable());
}

CallResult<HermesValue>
reflectPreventExtensions(void *, Runtime &runtime, NativeArgs args) {
  Handle<JSObject> target = args.dyncastArg<JSObject>(0);
  if (LLVM_UNLIKELY(!target)) {
    return runtime.raiseTypeError("target is not an object");
  }

  return toHV(JSObject::preventExtensions(target, runtime));
}

CallResult<HermesValue> reflectSet(void *, Runtime &runtime, NativeArgs args) {
  Handle<JSObject> target = args.dyncastArg<JSObject>(0);
  if (LLVM_UNLIKELY(!target)) {
    return runtime.raiseTypeError("target is not an object");
  }

  return toHV(JSObject::putComputedWithReceiver_RJS(
      target,
      runtime,
      args.getArgHandle(1),
      args.getArgHandle(2),
      (args.getArgCount() >= 4) ? args.getArgHandle(3) : target));
}

CallResult<HermesValue>
reflectSetPrototypeOf(void *, Runtime &runtime, NativeArgs args) {
  Handle<JSObject> target = args.dyncastArg<JSObject>(0);
  if (LLVM_UNLIKELY(!target)) {
    return runtime.raiseTypeError("target is not an object");
  }

  HermesValue proto = args.getArg(1);
  if (LLVM_UNLIKELY(!proto.isObject() && !proto.isNull())) {
    return runtime.raiseTypeError("target is not an object and not null");
  }

  return toHV(JSObject::setParent(
      *target, runtime, proto.isObject() ? vmcast<JSObject>(proto) : nullptr));
}

Handle<JSObject> createReflectObject(Runtime &runtime) {
  Handle<JSObject> reflect = runtime.makeHandle(JSObject::create(runtime));

  auto defineReflectMethod =
      [&](Predefined::Str symID, NativeFunctionPtr func, uint8_t count) {
        (void)defineMethod(
            runtime,
            reflect,
            Predefined::getSymbolID(symID),
            nullptr /* context */,
            func,
            count);
      };

  defineReflectMethod(Predefined::apply, reflectApply, 3);
  defineReflectMethod(Predefined::construct, reflectConstruct, 2);
  defineReflectMethod(Predefined::defineProperty, reflectDefineProperty, 3);
  defineReflectMethod(Predefined::deleteProperty, reflectDeleteProperty, 2);
  defineReflectMethod(Predefined::get, reflectGet, 2);
  defineReflectMethod(
      Predefined::getOwnPropertyDescriptor, reflectGetOwnPropertyDescriptor, 2);
  defineReflectMethod(Predefined::getPrototypeOf, reflectGetPrototypeOf, 1);
  defineReflectMethod(Predefined::has, reflectHas, 2);
  defineReflectMethod(Predefined::isExtensible, reflectIsExtensible, 1);
  defineReflectMethod(Predefined::ownKeys, reflectOwnKeys, 1);
  defineReflectMethod(
      Predefined::preventExtensions, reflectPreventExtensions, 1);
  defineReflectMethod(Predefined::set, reflectSet, 3);
  defineReflectMethod(Predefined::setPrototypeOf, reflectSetPrototypeOf, 2);

  DefinePropertyFlags dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  dpf.configurable = 1;
  defineProperty(
      runtime,
      reflect,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime.getPredefinedStringHandle(Predefined::Reflect),
      dpf);

  return reflect;
}

} // namespace vm
} // namespace hermes
