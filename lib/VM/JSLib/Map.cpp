//===----------------------------------------------------------------------===//
/// \file
/// ES6.0 23.1 Initialize the Map constructor.
//===----------------------------------------------------------------------===//

#include "JSLibInternal.h"

#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {

/// @name Map
/// @{

/// ES6.0 23.1.1  Mep() invoked as a function and as a constructor.
static CallResult<HermesValue>
mapConstructor(void *, Runtime *runtime, NativeArgs args);

/// @}

/// @name Map.prototype
/// @{

// TODO: Implement ES6.0 23.1.2.2: get Map [ @@species ]

/// ES6.0 23.1.3.1.
static CallResult<HermesValue>
mapPrototypeClear(void *, Runtime *runtime, NativeArgs args);

/// ES6.0 23.1.3.3.
static CallResult<HermesValue>
mapPrototypeDelete(void *, Runtime *runtime, NativeArgs args);

/// ES6.0 23.1.3.4.
static CallResult<HermesValue>
mapPrototypeEntries(void *, Runtime *runtime, NativeArgs args);

/// ES6.0 23.1.3.5.
static CallResult<HermesValue>
mapPrototypeForEach(void *, Runtime *runtime, NativeArgs args);

/// ES6.0 23.1.3.6.
static CallResult<HermesValue>
mapPrototypeGet(void *, Runtime *runtime, NativeArgs args);

/// ES6.0 23.1.3.7.
static CallResult<HermesValue>
mapPrototypeHas(void *, Runtime *runtime, NativeArgs args);

/// ES6.0 23.1.3.8.
static CallResult<HermesValue>
mapPrototypeKeys(void *, Runtime *runtime, NativeArgs args);

/// ES6.0 23.1.3.9.
static CallResult<HermesValue>
mapPrototypeSet(void *, Runtime *runtime, NativeArgs args);

/// ES6.0 23.1.3.10.
static CallResult<HermesValue>
mapPrototypeSizeGetter(void *, Runtime *runtime, NativeArgs args);

/// ES6.0 23.1.3.11.
static CallResult<HermesValue>
mapPrototypeValues(void *, Runtime *runtime, NativeArgs args);

// TODO: Implement ES6.0 23.1.3.12: Map.prototype [ @@iterator ]()

// TODO: Implement ES6.0 23.1.3.13: Map.prototype [ @@toStringTag ]

/// @}

/// @name MapIterator.prototype
/// @{

/// ES6.0 23.1.5.2.1.
static CallResult<HermesValue>
mapIteratorPrototypeNext(void *, Runtime *runtime, NativeArgs args);

/// @}

Handle<JSObject> createMapConstructor(Runtime *runtime) {
  auto mapPrototype = Handle<JSMap>::vmcast(&runtime->mapPrototype);

  // Map.prototype.xxx methods.
  defineMethod(
      runtime,
      mapPrototype,
      runtime->getPredefinedSymbolID(Predefined::clear),
      nullptr,
      mapPrototypeClear,
      0);

  defineMethod(
      runtime,
      mapPrototype,
      runtime->getPredefinedSymbolID(Predefined::deleteStr),
      nullptr,
      mapPrototypeDelete,
      1);

  defineMethod(
      runtime,
      mapPrototype,
      runtime->getPredefinedSymbolID(Predefined::entries),
      nullptr,
      mapPrototypeEntries,
      0);

  defineMethod(
      runtime,
      mapPrototype,
      runtime->getPredefinedSymbolID(Predefined::forEach),
      nullptr,
      mapPrototypeForEach,
      1);

  defineMethod(
      runtime,
      mapPrototype,
      runtime->getPredefinedSymbolID(Predefined::get),
      nullptr,
      mapPrototypeGet,
      1);

  defineMethod(
      runtime,
      mapPrototype,
      runtime->getPredefinedSymbolID(Predefined::has),
      nullptr,
      mapPrototypeHas,
      1);

  defineMethod(
      runtime,
      mapPrototype,
      runtime->getPredefinedSymbolID(Predefined::keys),
      nullptr,
      mapPrototypeKeys,
      0);

  defineMethod(
      runtime,
      mapPrototype,
      runtime->getPredefinedSymbolID(Predefined::set),
      nullptr,
      mapPrototypeSet,
      2);

  defineAccessor(
      runtime,
      mapPrototype,
      runtime->getPredefinedSymbolID(Predefined::size),
      nullptr,
      mapPrototypeSizeGetter,
      nullptr,
      false,
      true);

  defineMethod(
      runtime,
      mapPrototype,
      runtime->getPredefinedSymbolID(Predefined::values),
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

  auto propValue = runtime->ignoreAllocationFailure(JSObject::getNamed(
      mapPrototype,
      runtime,
      runtime->getPredefinedSymbolID(Predefined::entries)));
  runtime->ignoreAllocationFailure(JSObject::defineOwnProperty(
      mapPrototype,
      runtime,
      runtime->getPredefinedSymbolID(Predefined::SymbolIterator),
      dpf,
      runtime->makeHandle<NativeFunction>(propValue)));

  dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  defineProperty(
      runtime,
      mapPrototype,
      runtime->getPredefinedSymbolID(Predefined::SymbolToStringTag),
      runtime->getPredefinedStringHandle(Predefined::Map),
      dpf);

  auto cons = defineSystemConstructor<JSMap>(
      runtime,
      runtime->getPredefinedSymbolID(Predefined::Map),
      mapConstructor,
      mapPrototype,
      0,
      CellKind::MapKind);

  return cons;
}

static CallResult<HermesValue>
mapConstructor(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope{runtime};
  if (LLVM_UNLIKELY(!args.isConstructorCall())) {
    return runtime->raiseTypeError("Constructor Map requires 'new'");
  }
  auto selfHandle = args.dyncastThis<JSMap>(runtime);
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime->raiseTypeError(
        "Map Constructor only applies to Map object");
  }
  JSMap::initializeStorage(selfHandle, runtime);
  if (args.getArgCount() == 0 || args.getArg(0).isUndefined() ||
      args.getArg(0).isNull()) {
    return selfHandle.getHermesValue();
  }

  auto propRes = JSObject::getNamed(
      selfHandle, runtime, runtime->getPredefinedSymbolID(Predefined::set));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // ES6.0 23.1.1.1.7: Cache adder across all iterations of the loop.
  auto adder =
      Handle<Callable>::dyn_vmcast(runtime, runtime->makeHandle(*propRes));
  if (!adder) {
    return runtime->raiseTypeError("Property 'set' for Map is not callable");
  }

  auto iterRes = getIterator(runtime, args.getArgHandle(runtime, 0));
  if (LLVM_UNLIKELY(iterRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto iter = toHandle(runtime, std::move(*iterRes));

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
    auto nextRes = iteratorStep(runtime, iter);
    if (LLVM_UNLIKELY(nextRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!*nextRes) {
      // Done with iteration.
      return selfHandle.getHermesValue();
    }
    pairHandle = vmcast<JSObject>(nextRes->getHermesValue());
    auto nextItemRes = JSObject::getNamed(
        pairHandle, runtime, runtime->getPredefinedSymbolID(Predefined::value));
    if (LLVM_UNLIKELY(nextItemRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!vmisa<JSObject>(*nextItemRes)) {
      runtime->raiseTypeError("Iterator value must be an object");
      return iteratorCloseAndRethrow(runtime, iter);
    }
    pairHandle = vmcast<JSObject>(*nextItemRes);
    auto keyRes = JSObject::getComputed(pairHandle, runtime, zero);
    if (LLVM_UNLIKELY(keyRes == ExecutionStatus::EXCEPTION)) {
      return iteratorCloseAndRethrow(runtime, iter);
    }
    keyHandle = *keyRes;
    auto valueRes = JSObject::getComputed(pairHandle, runtime, one);
    if (LLVM_UNLIKELY(valueRes == ExecutionStatus::EXCEPTION)) {
      return iteratorCloseAndRethrow(runtime, iter);
    }
    valueHandle = *valueRes;
    if (LLVM_UNLIKELY(
            Callable::executeCall2(
                adder,
                runtime,
                selfHandle,
                keyHandle.getHermesValue(),
                valueHandle.getHermesValue()) == ExecutionStatus::EXCEPTION)) {
      return iteratorCloseAndRethrow(runtime, iter);
    }
  }

  return selfHandle.getHermesValue();
}

static CallResult<HermesValue>
mapPrototypeClear(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMap>(runtime);
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

static CallResult<HermesValue>
mapPrototypeDelete(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMap>(runtime);
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime->raiseTypeError(
        "Non-Map object called on Map.prototype.delete");
  }
  if (LLVM_UNLIKELY(!selfHandle->isInitialized())) {
    return runtime->raiseTypeError(
        "Method Map.prototype.delete called on incompatible receiver");
  }
  return HermesValue::encodeBoolValue(
      JSMap::deleteKey(selfHandle, runtime, args.getArgHandle(runtime, 0)));
}

static CallResult<HermesValue>
mapPrototypeEntries(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMap>(runtime);
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
  iterator->initializeIterator(
      &runtime->getHeap(), selfHandle, IterationKind::Entry);
  return iterator.getHermesValue();
}

static CallResult<HermesValue>
mapPrototypeForEach(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMap>(runtime);
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime->raiseTypeError(
        "Non-Map object called on Map.prototype.forEach");
  }
  if (LLVM_UNLIKELY(!selfHandle->isInitialized())) {
    return runtime->raiseTypeError(
        "Method Map.prototype.forEach called on incompatible receiver");
  }
  auto callbackfn = args.dyncastArg<Callable>(runtime, 0);
  if (LLVM_UNLIKELY(!callbackfn)) {
    return runtime->raiseTypeError(
        "callbackfn must be Callable in Map.prototype.forEach");
  }
  auto thisArg = args.getArgHandle(runtime, 1);
  if (JSMap::forEach(selfHandle, runtime, callbackfn, thisArg) ==
      ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;
  return HermesValue::encodeUndefinedValue();
}

static CallResult<HermesValue>
mapPrototypeGet(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMap>(runtime);
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime->raiseTypeError(
        "Non-Map object called on Map.prototype.get");
  }
  if (LLVM_UNLIKELY(!selfHandle->isInitialized())) {
    return runtime->raiseTypeError(
        "Method Map.prototype.get called on incompatible receiver");
  }
  return JSMap::getValue(selfHandle, runtime, args.getArgHandle(runtime, 0));
}

static CallResult<HermesValue>
mapPrototypeHas(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMap>(runtime);
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime->raiseTypeError(
        "Non-Map object called on Map.prototype.has");
  }
  if (LLVM_UNLIKELY(!selfHandle->isInitialized())) {
    return runtime->raiseTypeError(
        "Method Map.prototype.has called on incompatible receiver");
  }
  return HermesValue::encodeBoolValue(
      JSMap::hasKey(selfHandle, runtime, args.getArgHandle(runtime, 0)));
}

static CallResult<HermesValue>
mapPrototypeKeys(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMap>(runtime);
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
  iterator->initializeIterator(
      &runtime->getHeap(), selfHandle, IterationKind::Key);
  return iterator.getHermesValue();
}

static CallResult<HermesValue>
mapPrototypeSet(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMap>(runtime);
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime->raiseTypeError(
        "Non-Map object called on Map.prototype.set");
  }
  if (LLVM_UNLIKELY(!selfHandle->isInitialized())) {
    return runtime->raiseTypeError(
        "Method Map.prototype.set called on incompatible receiver");
  }
  JSMap::addValue(
      selfHandle,
      runtime,
      args.getArgHandle(runtime, 0),
      args.getArgHandle(runtime, 1));
  return selfHandle.getHermesValue();
}

static CallResult<HermesValue>
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
  return HermesValue::encodeNumberValue(JSMap::getSize(self));
}

static CallResult<HermesValue>
mapPrototypeValues(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMap>(runtime);
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
  iterator->initializeIterator(
      &runtime->getHeap(), selfHandle, IterationKind::Value);
  return iterator.getHermesValue();
}

Handle<JSObject> createMapIteratorPrototype(Runtime *runtime) {
  auto protoHandle = toHandle(
      runtime,
      JSObject::create(
          runtime, Handle<JSObject>::vmcast(&runtime->iteratorPrototype)));
  defineMethod(
      runtime,
      protoHandle,
      runtime->getPredefinedSymbolID(Predefined::next),
      nullptr,
      mapIteratorPrototypeNext,
      0);

  auto dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  defineProperty(
      runtime,
      protoHandle,
      runtime->getPredefinedSymbolID(Predefined::SymbolToStringTag),
      runtime->getPredefinedStringHandle(Predefined::MapIterator),
      dpf);

  return protoHandle;
}

static CallResult<HermesValue>
mapIteratorPrototypeNext(void *, Runtime *runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSMapIterator>(runtime);
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
