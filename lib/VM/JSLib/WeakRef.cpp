/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// ES5.1 15.4 Initialize the Array constructor.
//===----------------------------------------------------------------------===//

#include "JSLibInternal.h"
#include "hermes/VM/JSNativeFunctions.h"
#include "hermes/VM/JSWeakRef.h"
#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//

Handle<JSObject> createWeakRefConstructor(Runtime &runtime) {
  auto weakRefPrototype = Handle<JSObject>::vmcast(&runtime.weakRefPrototype);

  auto cons = defineSystemConstructor<JSWeakRef>(
      runtime,
      Predefined::getSymbolID(Predefined::WeakRef),
      weakRefConstructor,
      weakRefPrototype,
      1,
      CellKind::JSWeakRefKind);

  DefinePropertyFlags dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  dpf.configurable = 1;

  /// 26.1.3.3 WeakRef.prototype [ @@toStringTag ]
  /// The initial value is the String value "WeakRef".
  // This property has the attributes-
  // { [[Writable]]: false, [[Enumerable]]: false, [[Configurable]]: true }.
  defineProperty(
      runtime,
      weakRefPrototype,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime.getPredefinedStringHandle(Predefined::WeakRef),
      dpf);

  // WeakRef.prototype.deref method
  defineMethod(
      runtime,
      weakRefPrototype,
      Predefined::getSymbolID(Predefined::deref),
      nullptr,
      weakRefPrototypeDeref,
      0);

  return cons;
}

// ES2021 26.1.1.1
CallResult<HermesValue>
weakRefConstructor(void *, Runtime &runtime, NativeArgs args) {
  // 1. If NewTarget is undefined, throw a TypeError exception.
  if (!args.isConstructorCall()) {
    return runtime.raiseTypeError(
        "WeakRef() called in function context instead of constructor");
  }

  auto target = args.dyncastArg<JSObject>(0);
  // 2. If Type(target) is not Object, throw a TypeError exception.
  if (LLVM_UNLIKELY(!target)) {
    return runtime.raiseTypeError("target argument is not an object");
  }

  // When a WeakRef is created with a target:
  // 4. Perform AddToKeptObjects(target).
  runtime.addToKeptObjects(target);

  // 5. Set weakRef.[[WeakRefTarget]] to target.
  auto self = args.vmcastThis<JSWeakRef>();
  self->setTarget(runtime, target);

  // 6. Return weakRef.
  return self.getHermesValue();
}

// ES2021 26.1.4.1
CallResult<HermesValue>
weakRefPrototypeDeref(void *, Runtime &runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSWeakRef>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError(
        "WeakRef.prototype.deref() called on non-WeakRef object");
  }

  auto val = selfHandle->deref(runtime);
  if (val.isUndefined())
    return val;

  Handle<JSObject> targetHandle = runtime.makeHandle<JSObject>(val);
  // If the target is not empty, then
  // 2a. Perform AddToKeptObjects
  runtime.addToKeptObjects(targetHandle);
  return targetHandle.getHermesValue();
}

} // namespace vm
} // namespace hermes
