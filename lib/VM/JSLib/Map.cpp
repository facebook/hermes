/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
//===----------------------------------------------------------------------===//
/// \file
/// ES6.0 23.1 Initialize the Map constructor.
//===----------------------------------------------------------------------===//

#include "JSLibInternal.h"

#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {

Handle<JSObject> createMapConstructor(Runtime *runtime) {
  auto mapPrototype = Handle<JSMap>::vmcast(&runtime->mapPrototype);

  // Map.prototype.xxx methods.
  defineMethod(
      runtime,
      mapPrototype,
      Predefined::getSymbolID(Predefined::clear),
      nullptr,
      mapPrototypeClear,
      0);

  defineMethod(
      runtime,
      mapPrototype,
      Predefined::getSymbolID(Predefined::deleteStr),
      nullptr,
      mapPrototypeDelete,
      1);

  defineMethod(
      runtime,
      mapPrototype,
      Predefined::getSymbolID(Predefined::entries),
      nullptr,
      mapPrototypeEntries,
      0);

  defineMethod(
      runtime,
      mapPrototype,
      Predefined::getSymbolID(Predefined::forEach),
      nullptr,
      mapPrototypeForEach,
      1);

  defineMethod(
      runtime,
      mapPrototype,
      Predefined::getSymbolID(Predefined::get),
      nullptr,
      mapPrototypeGet,
      1);

  defineMethod(
      runtime,
      mapPrototype,
      Predefined::getSymbolID(Predefined::has),
      nullptr,
      mapPrototypeHas,
      1);

  defineMethod(
      runtime,
      mapPrototype,
      Predefined::getSymbolID(Predefined::keys),
      nullptr,
      mapPrototypeKeys,
      0);

  defineMethod(
      runtime,
      mapPrototype,
      Predefined::getSymbolID(Predefined::set),
      nullptr,
      mapPrototypeSet,
      2);

  defineAccessor(
      runtime,
      mapPrototype,
      Predefined::getSymbolID(Predefined::size),
      nullptr,
      mapPrototypeSizeGetter,
      nullptr,
      false,
      true);

  defineMethod(
      runtime,
      mapPrototype,
      Predefined::getSymbolID(Predefined::values),
      nullptr,
      mapPrototypeValues,
      0);

  DefinePropertyFlags dpf{};
  dpf.setEnumerable = 1;
  dpf.setWritable = 1;
  dpf.setConfigurable = 1;
  dpf.setValue = 1;
  dpf.enumerable = 0;
  dpf.writable = 1;
  dpf.configurable = 1;

  auto propValue = runtime->ignoreAllocationFailure(JSObject::getNamed_RJS(
      mapPrototype, runtime, Predefined::getSymbolID(Predefined::entries)));
  runtime->ignoreAllocationFailure(JSObject::defineOwnProperty(
      mapPrototype,
      runtime,
      Predefined::getSymbolID(Predefined::SymbolIterator),
      dpf,
      runtime->makeHandle<NativeFunction>(propValue)));

  dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  defineProperty(
      runtime,
      mapPrototype,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime->getPredefinedStringHandle(Predefined::Map),
      dpf);

  auto cons = defineSystemConstructor<JSMap>(
      runtime,
      Predefined::getSymbolID(Predefined::Map),
      mapConstructor,
      mapPrototype,
      0,
      CellKind::MapKind);

  return cons;
}

CallResult<HermesValue>
mapConstructor(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope{runtime};
  if (LLVM_UNLIKELY(!args.isConstructorCall())) {
    return runtime->raiseTypeError("Constructor Map requires 'new'");
  }
  auto selfHandle = args.dyncastThis<JSMap>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime->raiseTypeError(
        "Map Constructor only applies to Map object");
  }
  JSMap::initializeStorage(selfHandle, runtime);
  if (args.getArgCount() == 0 || args.getArg(0).isUndefined() ||
      args.getArg(0).isNull()) {
    return selfHandle.getHermesValue();
  }

  auto propRes = JSObject::getNamed_RJS(
      selfHandle, runtime, Predefined::getSymbolID(Predefined::set));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // ES6.0 23.1.1.1.7: Cache adder across all iterations of the loop.
  auto adder = Handle<Callable>::dyn_vmcast(runtime->makeHandle(*propRes));
  if (!adder) {
    return runtime->raiseTypeError("Property 'set' for Map is not callable");
  }

  auto iterRes = getIterator(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(iterRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto iteratorRecord = *iterRes;

  MutableHandle<JSObject> pairHandle{runtime};
  MutableHandle<> keyHandle{runtime};
  MutableHandle<> valueHandle{runtime};
  Handle<> zero{runtime, HermesValue::encodeNumberValue(0)};
  Handle<> one{runtime, HermesValue::encodeNumberValue(1)};
  auto marker = gcScope.createMarker();

  // Check the length of the array after every iteration,
  // to allow for the fact that the length could be modified during iteration.
  for (;;) {
    gcScope.flushToMarker(marker);
    auto nextRes = iteratorStep(runtime, iteratorRecord);
    if (LLVM_UNLIKELY(nextRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!*nextRes) {
      // Done with iteration.
      return selfHandle.getHermesValue();
    }
    pairHandle = vmcast<JSObject>(nextRes->getHermesValue());
    auto nextItemRes = JSObject::getNamed_RJS(
        pairHandle, runtime, Predefined::getSymbolID(Predefined::value));
    if (LLVM_UNLIKELY(nextItemRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!vmisa<JSObject>(*nextItemRes)) {
      runtime->raiseTypeError("Iterator value must be an object");
      return iteratorCloseAndRethrow(runtime, iteratorRecord.iterator);
    }
    pairHandle = vmcast<JSObject>(*nextItemRes);
    auto keyRes = JSObject::getComputed_RJS(pairHandle, runtime, zero);
    if (LLVM_UNLIKELY(keyRes == ExecutionStatus::EXCEPTION)) {
      return iteratorCloseAndRethrow(runtime, iteratorRecord.iterator);
    }
    keyHandle = *keyRes;
    auto valueRes = JSObject::getComputed_RJS(pairHandle, runtime, one);
    if (LLVM_UNLIKELY(valueRes == ExecutionStatus::EXCEPTION)) {
      return iteratorCloseAndRethrow(runtime, iteratorRecord.iterator);
    }
    valueHandle = *valueRes;
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
mapPrototypeClear(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMap>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime->raiseTypeError(
        "Non-Map object called on Map.prototype.clear");
  }
  if (LLVM_UNLIKELY(!selfHandle->isInitialized())) {
    return runtime->raiseTypeError(
        "Method Map.prototype.clear called on incompatible receiver");
  }
  JSMap::clear(selfHandle, runtime);
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
mapPrototypeDelete(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMap>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime->raiseTypeError(
        "Non-Map object called on Map.prototype.delete");
  }
  if (LLVM_UNLIKELY(!selfHandle->isInitialized())) {
    return runtime->raiseTypeError(
        "Method Map.prototype.delete called on incompatible receiver");
  }
  return HermesValue::encodeBoolValue(
      JSMap::deleteKey(selfHandle, runtime, args.getArgHandle(0)));
}

CallResult<HermesValue>
mapPrototypeEntries(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMap>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime->raiseTypeError(
        "Non-Map object called on Map.prototype.entries");
  }
  if (LLVM_UNLIKELY(!selfHandle->isInitialized())) {
    return runtime->raiseTypeError(
        "Method Map.prototype.entries called on incompatible receiver");
  }
  auto mapRes = JSMapIterator::create(
      runtime, Handle<JSObject>::vmcast(&runtime->mapIteratorPrototype));
  if (LLVM_UNLIKELY(mapRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto iterator = runtime->makeHandle<JSMapIterator>(*mapRes);
  iterator->initializeIterator(runtime, selfHandle, IterationKind::Entry);
  return iterator.getHermesValue();
}

CallResult<HermesValue>
mapPrototypeForEach(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMap>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime->raiseTypeError(
        "Non-Map object called on Map.prototype.forEach");
  }
  if (LLVM_UNLIKELY(!selfHandle->isInitialized())) {
    return runtime->raiseTypeError(
        "Method Map.prototype.forEach called on incompatible receiver");
  }
  auto callbackfn = args.dyncastArg<Callable>(0);
  if (LLVM_UNLIKELY(!callbackfn)) {
    return runtime->raiseTypeError(
        "callbackfn must be Callable in Map.prototype.forEach");
  }
  auto thisArg = args.getArgHandle(1);
  if (JSMap::forEach(selfHandle, runtime, callbackfn, thisArg) ==
      ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
mapPrototypeGet(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMap>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime->raiseTypeError(
        "Non-Map object called on Map.prototype.get");
  }
  if (LLVM_UNLIKELY(!selfHandle->isInitialized())) {
    return runtime->raiseTypeError(
        "Method Map.prototype.get called on incompatible receiver");
  }
  return JSMap::getValue(selfHandle, runtime, args.getArgHandle(0));
}

CallResult<HermesValue>
mapPrototypeHas(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMap>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime->raiseTypeError(
        "Non-Map object called on Map.prototype.has");
  }
  if (LLVM_UNLIKELY(!selfHandle->isInitialized())) {
    return runtime->raiseTypeError(
        "Method Map.prototype.has called on incompatible receiver");
  }
  return HermesValue::encodeBoolValue(
      JSMap::hasKey(selfHandle, runtime, args.getArgHandle(0)));
}

CallResult<HermesValue>
mapPrototypeKeys(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMap>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime->raiseTypeError(
        "Non-Map object called on Map.prototype.keys");
  }
  if (LLVM_UNLIKELY(!selfHandle->isInitialized())) {
    return runtime->raiseTypeError(
        "Method Map.prototype.keys called on incompatible receiver");
  }

  auto mapRes = JSMapIterator::create(
      runtime, Handle<JSObject>::vmcast(&runtime->mapIteratorPrototype));
  if (LLVM_UNLIKELY(mapRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto iterator = runtime->makeHandle<JSMapIterator>(*mapRes);
  iterator->initializeIterator(runtime, selfHandle, IterationKind::Key);
  return iterator.getHermesValue();
}

CallResult<HermesValue>
mapPrototypeSet(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMap>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime->raiseTypeError(
        "Non-Map object called on Map.prototype.set");
  }
  if (LLVM_UNLIKELY(!selfHandle->isInitialized())) {
    return runtime->raiseTypeError(
        "Method Map.prototype.set called on incompatible receiver");
  }
  JSMap::addValue(
      selfHandle, runtime, args.getArgHandle(0), args.getArgHandle(1));
  return selfHandle.getHermesValue();
}

CallResult<HermesValue>
mapPrototypeSizeGetter(void *, Runtime *runtime, NativeArgs args) {
  auto self = dyn_vmcast<JSMap>(args.getThisArg());
  if (LLVM_UNLIKELY(!self)) {
    return runtime->raiseTypeError(
        "Non-Map object called on Map.prototype.size");
  }
  if (LLVM_UNLIKELY(!self->isInitialized())) {
    return runtime->raiseTypeError(
        "Method Map.prototype.size called on incompatible receiver");
  }
  return HermesValue::encodeNumberValue(JSMap::getSize(self, runtime));
}

CallResult<HermesValue>
mapPrototypeValues(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMap>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime->raiseTypeError(
        "Non-Map object called on Map.prototype.values");
  }
  if (LLVM_UNLIKELY(!selfHandle->isInitialized())) {
    return runtime->raiseTypeError(
        "Method Map.prototype.values called on incompatible receiver");
  }
  auto mapRes = JSMapIterator::create(
      runtime, Handle<JSObject>::vmcast(&runtime->mapIteratorPrototype));
  if (LLVM_UNLIKELY(mapRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto iterator = runtime->makeHandle<JSMapIterator>(*mapRes);
  iterator->initializeIterator(runtime, selfHandle, IterationKind::Value);
  return iterator.getHermesValue();
}

Handle<JSObject> createMapIteratorPrototype(Runtime *runtime) {
  auto parentHandle = toHandle(
      runtime,
      JSObject::create(
          runtime, Handle<JSObject>::vmcast(&runtime->iteratorPrototype)));
  defineMethod(
      runtime,
      parentHandle,
      Predefined::getSymbolID(Predefined::next),
      nullptr,
      mapIteratorPrototypeNext,
      0);

  auto dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  defineProperty(
      runtime,
      parentHandle,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime->getPredefinedStringHandle(Predefined::MapIterator),
      dpf);

  return parentHandle;
}

CallResult<HermesValue>
mapIteratorPrototypeNext(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMapIterator>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime->raiseTypeError(
        "Non-MapIterator object called on MapIterator.prototype.next");
  }
  if (LLVM_UNLIKELY(!selfHandle->isInitialized())) {
    return runtime->raiseTypeError(
        "Method MapIterator.prototype.next called on incompatible receiver");
  }

  auto cr = JSMapIterator::nextElement(selfHandle, runtime);
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return *cr;
}
} // namespace vm
} // namespace hermes
