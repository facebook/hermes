/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSLibInternal.h"

#include "hermes/VM/CellKind.h"
#include "hermes/VM/JSWeakMapImpl.h"
#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {

Handle<JSObject> createWeakMapConstructor(Runtime &runtime) {
  auto weakMapPrototype = Handle<JSObject>::vmcast(&runtime.weakMapPrototype);

  defineMethod(
      runtime,
      weakMapPrototype,
      Predefined::getSymbolID(Predefined::deleteStr),
      nullptr,
      weakMapPrototypeDelete,
      1);

  defineMethod(
      runtime,
      weakMapPrototype,
      Predefined::getSymbolID(Predefined::get),
      nullptr,
      weakMapPrototypeGet,
      1);

  defineMethod(
      runtime,
      weakMapPrototype,
      Predefined::getSymbolID(Predefined::has),
      nullptr,
      weakMapPrototypeHas,
      1);

  defineMethod(
      runtime,
      weakMapPrototype,
      Predefined::getSymbolID(Predefined::set),
      nullptr,
      weakMapPrototypeSet,
      2);

  DefinePropertyFlags dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  defineProperty(
      runtime,
      weakMapPrototype,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime.getPredefinedStringHandle(Predefined::WeakMap),
      dpf);

  auto cons = defineSystemConstructor<JSWeakMap>(
      runtime,
      Predefined::getSymbolID(Predefined::WeakMap),
      weakMapConstructor,
      weakMapPrototype,
      0,
      CellKind::JSWeakMapKind);

  // ES6.0 23.3.3.1
  defineProperty(
      runtime,
      weakMapPrototype,
      Predefined::getSymbolID(Predefined::constructor),
      cons);

  return cons;
}

CallResult<HermesValue>
weakMapConstructor(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  if (LLVM_UNLIKELY(!args.isConstructorCall())) {
    return runtime.raiseTypeError("WeakMap must be called as a constructor");
  }

  auto selfHandle = args.dyncastThis<JSWeakMap>();

  if (args.getArgCount() == 0 || args.getArg(0).isUndefined() ||
      args.getArg(0).isNull()) {
    return selfHandle.getHermesValue();
  }

  auto propRes = JSObject::getNamed_RJS(
      selfHandle, runtime, Predefined::getSymbolID(Predefined::set));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto adder =
      Handle<Callable>::dyn_vmcast(runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(!adder)) {
    return runtime.raiseTypeError("Property 'set' for WeakMap is not callable");
  }

  auto iterRes = getIterator(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(iterRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto iteratorRecord = *iterRes;

  MutableHandle<JSObject> nextItem{runtime};
  MutableHandle<> keyHandle{runtime};
  MutableHandle<> valueHandle{runtime};
  Handle<> zero = HandleRootOwner::getZeroValue();
  Handle<> one = HandleRootOwner::getOneValue();
  auto marker = gcScope.createMarker();

  for (;;) {
    gcScope.flushToMarker(marker);
    auto nextRes = iteratorStep(runtime, iteratorRecord);
    if (LLVM_UNLIKELY(nextRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!*nextRes) {
      return selfHandle.getHermesValue();
    }
    auto nextItemRes = JSObject::getNamed_RJS(
        *nextRes, runtime, Predefined::getSymbolID(Predefined::value));
    if (LLVM_UNLIKELY(nextItemRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!vmisa<JSObject>(nextItemRes->get())) {
      (void)runtime.raiseTypeError(
          "WeakMap([iterable]) elements must be objects");
      return iteratorCloseAndRethrow(runtime, iteratorRecord.iterator);
    }
    nextItem = vmcast<JSObject>(nextItemRes->get());
    auto keyRes = JSObject::getComputed_RJS(nextItem, runtime, zero);
    if (LLVM_UNLIKELY(keyRes == ExecutionStatus::EXCEPTION)) {
      return iteratorCloseAndRethrow(runtime, iteratorRecord.iterator);
    }
    keyHandle = std::move(*keyRes);
    auto valueRes = JSObject::getComputed_RJS(nextItem, runtime, one);
    if (LLVM_UNLIKELY(valueRes == ExecutionStatus::EXCEPTION)) {
      return iteratorCloseAndRethrow(runtime, iteratorRecord.iterator);
    }
    valueHandle = std::move(*valueRes);
    if (LLVM_UNLIKELY(
            Callable::executeCall2(
                adder,
                runtime,
                selfHandle,
                keyHandle.getHermesValue(),
                valueHandle.getHermesValue()) == ExecutionStatus::EXCEPTION)) {
      return iteratorCloseAndRethrow(runtime, iteratorRecord.iterator);
    }
  }

  return selfHandle.getHermesValue();
}

CallResult<HermesValue>
weakMapPrototypeDelete(void *, Runtime &runtime, NativeArgs args) {
  auto M = args.dyncastThis<JSWeakMap>();
  if (LLVM_UNLIKELY(!M)) {
    return runtime.raiseTypeError(
        "WeakMap.prototype.delete can only be called on a WeakMap");
  }

  auto key = args.dyncastArg<JSObject>(0);
  if (LLVM_UNLIKELY(!key)) {
    return HermesValue::encodeBoolValue(false);
  }

  return HermesValue::encodeBoolValue(JSWeakMap::deleteValue(M, runtime, key));
}

CallResult<HermesValue>
weakMapPrototypeGet(void *, Runtime &runtime, NativeArgs args) {
  auto M = args.dyncastThis<JSWeakMap>();
  if (LLVM_UNLIKELY(!M)) {
    return runtime.raiseTypeError(
        "WeakMap.prototype.get can only be called on a WeakMap");
  }

  auto key = args.dyncastArg<JSObject>(0);
  if (LLVM_UNLIKELY(!key)) {
    return HermesValue::encodeUndefinedValue();
  }

  return JSWeakMap::getValue(M, runtime, key);
}

CallResult<HermesValue>
weakMapPrototypeHas(void *, Runtime &runtime, NativeArgs args) {
  auto M = args.dyncastThis<JSWeakMap>();
  if (LLVM_UNLIKELY(!M)) {
    return runtime.raiseTypeError(
        "WeakMap.prototype.has can only be called on a WeakMap");
  }

  auto key = args.dyncastArg<JSObject>(0);
  if (LLVM_UNLIKELY(!key)) {
    return HermesValue::encodeBoolValue(false);
  }

  return HermesValue::encodeBoolValue(JSWeakMap::hasValue(M, runtime, key));
}

CallResult<HermesValue>
weakMapPrototypeSet(void *, Runtime &runtime, NativeArgs args) {
  auto M = args.dyncastThis<JSWeakMap>();
  if (LLVM_UNLIKELY(!M)) {
    return runtime.raiseTypeError(
        "WeakMap.prototype.set can only be called on a WeakMap");
  }

  auto key = args.dyncastArg<JSObject>(0);
  if (LLVM_UNLIKELY(!key)) {
    return runtime.raiseTypeError("WeakMap key must be an Object");
  }

  if (LLVM_UNLIKELY(
          JSWeakMap::setValue(M, runtime, key, args.getArgHandle(1)) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return M.getHermesValue();
}

} // namespace vm
} // namespace hermes
