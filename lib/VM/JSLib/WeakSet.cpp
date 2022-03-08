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

Handle<JSObject> createWeakSetConstructor(Runtime &runtime) {
  auto weakSetPrototype = Handle<JSObject>::vmcast(&runtime.weakSetPrototype);

  defineMethod(
      runtime,
      weakSetPrototype,
      Predefined::getSymbolID(Predefined::add),
      nullptr,
      weakSetPrototypeAdd,
      1);

  defineMethod(
      runtime,
      weakSetPrototype,
      Predefined::getSymbolID(Predefined::deleteStr),
      nullptr,
      weakSetPrototypeDelete,
      1);

  defineMethod(
      runtime,
      weakSetPrototype,
      Predefined::getSymbolID(Predefined::has),
      nullptr,
      weakSetPrototypeHas,
      1);

  DefinePropertyFlags dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  defineProperty(
      runtime,
      weakSetPrototype,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime.getPredefinedStringHandle(Predefined::WeakSet),
      dpf);

  auto cons = defineSystemConstructor<JSWeakSet>(
      runtime,
      Predefined::getSymbolID(Predefined::WeakSet),
      weakSetConstructor,
      weakSetPrototype,
      0,
      CellKind::JSWeakSetKind);

  // ES6.0 23.4.3.1
  defineProperty(
      runtime,
      weakSetPrototype,
      Predefined::getSymbolID(Predefined::constructor),
      cons);

  return cons;
}

CallResult<HermesValue>
weakSetConstructor(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  if (LLVM_UNLIKELY(!args.isConstructorCall())) {
    return runtime.raiseTypeError("WeakSet must be called as a constructor");
  }

  auto selfHandle = args.dyncastThis<JSWeakSet>();

  if (args.getArgCount() == 0 || args.getArg(0).isUndefined() ||
      args.getArg(0).isNull()) {
    return selfHandle.getHermesValue();
  }

  auto propRes = JSObject::getNamed_RJS(
      selfHandle, runtime, Predefined::getSymbolID(Predefined::add));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto adder =
      Handle<Callable>::dyn_vmcast(runtime.makeHandle(std::move(*propRes)));
  if (LLVM_UNLIKELY(!adder)) {
    return runtime.raiseTypeError("Property 'add' for WeakSet is not callable");
  }

  auto iterRes = getIterator(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(iterRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto iteratorRecord = *iterRes;

  auto marker = gcScope.createMarker();
  for (;;) {
    gcScope.flushToMarker(marker);
    CallResult<Handle<JSObject>> nextRes =
        iteratorStep(runtime, iteratorRecord);
    if (LLVM_UNLIKELY(nextRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!*nextRes) {
      // Done with iteration.
      return selfHandle.getHermesValue();
    }
    auto nextValueRes = JSObject::getNamed_RJS(
        *nextRes, runtime, Predefined::getSymbolID(Predefined::value));
    if (LLVM_UNLIKELY(nextValueRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    if (LLVM_UNLIKELY(
            Callable::executeCall1(
                adder, runtime, selfHandle, nextValueRes->get()) ==
            ExecutionStatus::EXCEPTION)) {
      return iteratorCloseAndRethrow(runtime, iteratorRecord.iterator);
    }
  }

  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
weakSetPrototypeAdd(void *, Runtime &runtime, NativeArgs args) {
  auto M = args.dyncastThis<JSWeakSet>();
  if (LLVM_UNLIKELY(!M)) {
    return runtime.raiseTypeError(
        "WeakSet.prototype.add can only be called on a WeakSet");
  }

  auto key = args.dyncastArg<JSObject>(0);
  if (LLVM_UNLIKELY(!key)) {
    return runtime.raiseTypeError("WeakSet key must be an Object");
  }

  if (LLVM_UNLIKELY(
          JSWeakSet::setValue(
              M, runtime, key, HandleRootOwner::getUndefinedValue()) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return M.getHermesValue();
}

CallResult<HermesValue>
weakSetPrototypeDelete(void *, Runtime &runtime, NativeArgs args) {
  auto M = args.dyncastThis<JSWeakSet>();
  if (LLVM_UNLIKELY(!M)) {
    return runtime.raiseTypeError(
        "WeakSet.prototype.delete can only be called on a WeakSet");
  }

  auto key = args.dyncastArg<JSObject>(0);
  if (LLVM_UNLIKELY(!key)) {
    return HermesValue::encodeBoolValue(false);
  }

  return HermesValue::encodeBoolValue(JSWeakSet::deleteValue(M, runtime, key));
}

CallResult<HermesValue>
weakSetPrototypeHas(void *, Runtime &runtime, NativeArgs args) {
  auto M = args.dyncastThis<JSWeakSet>();
  if (LLVM_UNLIKELY(!M)) {
    return runtime.raiseTypeError(
        "WeakSet.prototype.has can only be called on a WeakSet");
  }

  auto key = args.dyncastArg<JSObject>(0);
  if (LLVM_UNLIKELY(!key)) {
    return HermesValue::encodeBoolValue(false);
  }

  return HermesValue::encodeBoolValue(JSWeakSet::hasValue(M, runtime, key));
}

} // namespace vm
} // namespace hermes
