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

Handle<NativeConstructor> createWeakSetConstructor(Runtime &runtime) {
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

  auto cons = defineSystemConstructor(
      runtime,
      Predefined::getSymbolID(Predefined::WeakSet),
      weakSetConstructor,
      weakSetPrototype,
      0);

  // ES6.0 23.4.3.1
  defineProperty(
      runtime,
      weakSetPrototype,
      Predefined::getSymbolID(Predefined::constructor),
      cons);

  return cons;
}

CallResult<HermesValue> weakSetConstructor(void *, Runtime &runtime) {
  NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
  GCScope gcScope{runtime};

  if (LLVM_UNLIKELY(!args.isConstructorCall())) {
    return runtime.raiseTypeError("WeakSet must be called as a constructor");
  }

  struct : public Locals {
    PinnedValue<JSObject> selfParent;
    PinnedValue<JSWeakSet> self;
    PinnedValue<Callable> adder;
  } lv;
  LocalsRAII lraii(runtime, &lv);
  if (LLVM_LIKELY(
          args.getNewTarget().getRaw() ==
          runtime.weakSetConstructor.getHermesValue().getRaw())) {
    lv.selfParent = runtime.weakSetPrototype;
  } else {
    CallResult<PseudoHandle<JSObject>> thisParentRes =
        NativeConstructor::parentForNewThis_RJS(
            runtime,
            Handle<Callable>::vmcast(&args.getNewTarget()),
            runtime.weakSetPrototype);
    if (LLVM_UNLIKELY(thisParentRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    lv.selfParent = std::move(*thisParentRes);
  }
  auto selfRes = JSWeakSet::create(runtime, lv.selfParent);
  if (LLVM_UNLIKELY(selfRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  lv.self = std::move(*selfRes);

  if (args.getArgCount() == 0 || args.getArg(0).isUndefined() ||
      args.getArg(0).isNull()) {
    return lv.self.getHermesValue();
  }

  auto propRes = JSObject::getNamed_RJS(
      lv.self, runtime, Predefined::getSymbolID(Predefined::add));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (auto callable = dyn_vmcast<Callable>(propRes->getHermesValue())) {
    lv.adder = callable;
  } else {
    return runtime.raiseTypeError("Property 'add' for WeakSet is not callable");
  }

  auto iterRes = getCheckedIterator(runtime, args.getArgHandle(0));
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
      return lv.self.getHermesValue();
    }
    auto nextValueRes = JSObject::getNamed_RJS(
        *nextRes, runtime, Predefined::getSymbolID(Predefined::value));
    if (LLVM_UNLIKELY(nextValueRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    if (LLVM_UNLIKELY(
            Callable::executeCall1(
                lv.adder, runtime, lv.self, nextValueRes->get()) ==
            ExecutionStatus::EXCEPTION)) {
      return iteratorCloseAndRethrow(runtime, iteratorRecord.iterator);
    }
  }

  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue> weakSetPrototypeAdd(void *, Runtime &runtime) {
  NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
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

CallResult<HermesValue> weakSetPrototypeDelete(void *, Runtime &runtime) {
  NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
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

CallResult<HermesValue> weakSetPrototypeHas(void *, Runtime &runtime) {
  NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
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
