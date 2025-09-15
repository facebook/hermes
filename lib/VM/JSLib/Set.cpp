/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSLibInternal.h"
#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {

Handle<JSObject> createSetConstructor(Runtime &runtime) {
  auto setPrototype = Handle<JSObject>::vmcast(&runtime.setPrototype);

  DefinePropertyFlags dpf = DefinePropertyFlags::getNewNonEnumerableFlags();

  defineMethod(
      runtime,
      setPrototype,
      Predefined::getSymbolID(Predefined::add),
      nullptr,
      setPrototypeAdd,
      1);

  defineMethod(
      runtime,
      setPrototype,
      Predefined::getSymbolID(Predefined::clear),
      nullptr,
      setPrototypeClear,
      0);

  defineMethod(
      runtime,
      setPrototype,
      Predefined::getSymbolID(Predefined::deleteStr),
      nullptr,
      setPrototypeDelete,
      1);

  defineMethod(
      runtime,
      setPrototype,
      Predefined::getSymbolID(Predefined::entries),
      nullptr,
      setPrototypeEntries,
      0);

  defineMethod(
      runtime,
      setPrototype,
      Predefined::getSymbolID(Predefined::forEach),
      nullptr,
      setPrototypeForEach,
      1);

  defineMethod(
      runtime,
      setPrototype,
      Predefined::getSymbolID(Predefined::has),
      nullptr,
      setPrototypeHas,
      1);

  defineAccessor(
      runtime,
      setPrototype,
      Predefined::getSymbolID(Predefined::size),
      nullptr,
      setPrototypeSizeGetter,
      nullptr,
      false,
      true);

  defineMethod(
      runtime,
      setPrototype,
      Predefined::getSymbolID(Predefined::values),
      nullptr,
      setPrototypeValues,
      0);

  // ES2025 Set methods
  defineMethod(
      runtime,
      setPrototype,
      Predefined::getSymbolID(Predefined::intersection),
      nullptr,
      setPrototypeIntersection,
      1);

  defineMethod(
      runtime,
      setPrototype,
      Predefined::getSymbolID(Predefined::unionStr),
      nullptr,
      setPrototypeUnion,
      1);

  defineMethod(
      runtime,
      setPrototype,
      Predefined::getSymbolID(Predefined::difference),
      nullptr,
      setPrototypeDifference,
      1);

  defineMethod(
      runtime,
      setPrototype,
      Predefined::getSymbolID(Predefined::symmetricDifference),
      nullptr,
      setPrototypeSymmetricDifference,
      1);

  defineMethod(
      runtime,
      setPrototype,
      Predefined::getSymbolID(Predefined::isSubsetOf),
      nullptr,
      setPrototypeIsSubsetOf,
      1);

  defineMethod(
      runtime,
      setPrototype,
      Predefined::getSymbolID(Predefined::isSupersetOf),
      nullptr,
      setPrototypeIsSupersetOf,
      1);

  defineMethod(
      runtime,
      setPrototype,
      Predefined::getSymbolID(Predefined::isDisjointFrom),
      nullptr,
      setPrototypeIsDisjointFrom,
      1);

  // Set [Symbol.iterator] and keys to be the same as values.
  Handle<NativeFunction> propValue = Handle<NativeFunction>::vmcast(
      runtime.makeHandle(runtime.ignoreAllocationFailure(
          JSObject::getNamed_RJS(
              setPrototype,
              runtime,
              Predefined::getSymbolID(Predefined::values)))));
  runtime.ignoreAllocationFailure(
      JSObject::defineOwnProperty(
          setPrototype,
          runtime,
          Predefined::getSymbolID(Predefined::keys),
          dpf,
          propValue));
  runtime.ignoreAllocationFailure(
      JSObject::defineOwnProperty(
          setPrototype,
          runtime,
          Predefined::getSymbolID(Predefined::SymbolIterator),
          dpf,
          propValue));

  dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  defineProperty(
      runtime,
      setPrototype,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime.getPredefinedStringHandle(Predefined::Set),
      dpf);

  auto cons = defineSystemConstructor<JSSet>(
      runtime,
      Predefined::getSymbolID(Predefined::Set),
      setConstructor,
      setPrototype,
      0,
      CellKind::JSSetKind);

  return cons;
}

CallResult<HermesValue>
setConstructor(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};
  if (LLVM_UNLIKELY(!args.isConstructorCall())) {
    return runtime.raiseTypeError("Constructor Set requires 'new'");
  }
  auto selfHandle = args.dyncastThis<JSSet>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError("Set Constructor only applies to Set object");
  }

  JSSet::initializeStorage(selfHandle, runtime);

  if (args.getArgCount() == 0 || args.getArg(0).isUndefined() ||
      args.getArg(0).isNull()) {
    return selfHandle.getHermesValue();
  }

  auto propRes = JSObject::getNamed_RJS(
      selfHandle, runtime, Predefined::getSymbolID(Predefined::add));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // ES6.0 23.2.1.1.7: Cache adder across all iterations of the loop.
  auto adder =
      Handle<Callable>::dyn_vmcast(runtime.makeHandle(std::move(*propRes)));
  if (!adder) {
    return runtime.raiseTypeError("Property 'add' for Set is not callable");
  }

  auto iterRes = getIterator(runtime, args.getArgHandle(0));
  if (iterRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto iterRecord = *iterRes;

  while (true) {
    auto nextRes = iteratorNext(runtime, iterRecord);
    if (nextRes == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    auto next = runtime.makeHandle<JSObject>(std::move(*nextRes));

    auto doneRes = JSObject::getNamed_RJS(
        next, runtime, Predefined::getSymbolID(Predefined::done));
    if (doneRes == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    if (toBoolean(doneRes->get())) {
      return selfHandle.getHermesValue();
    }

    auto valueRes = JSObject::getNamed_RJS(
        next, runtime, Predefined::getSymbolID(Predefined::value));
    if (valueRes == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    if (Callable::executeCall1(
            adder, runtime, selfHandle, valueRes->get(), true) ==
        ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  }

  return selfHandle.getHermesValue();
}
// ES12 23.2.3.1 Set.prototype.add ( value )
CallResult<HermesValue>
setPrototypeAdd(void *, Runtime &runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSSet>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError("Non-Set object called on Set.prototype.add");
  }
  auto valueHandle = args.getArgHandle(0);
  // 5. If value is -0, set value to +0.
  // N.B. in the case of Set, the value is used as both the value and the key of
  // the entry. They are both observed as normalized from Set iterators.
  auto value = valueHandle->isNumber() && valueHandle->getNumber() == 0
      ? HandleRootOwner::getZeroValue()
      : valueHandle;
  JSSet::addValue(selfHandle, runtime, value, value);
  return selfHandle.getHermesValue();
}

// ES12 23.2.3.2 Set.prototype.clear ( )
CallResult<HermesValue>
setPrototypeClear(void *, Runtime &runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSSet>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError(
        "Non-Set object called on Set.prototype.clear");
  }
  JSSet::clear(selfHandle, runtime);
  return HermesValue::encodeUndefinedValue();
}

// ES12 23.2.3.4 Set.prototype.delete ( value )
CallResult<HermesValue>
setPrototypeDelete(void *, Runtime &runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSSet>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError(
        "Non-Set object called on Set.prototype.delete");
  }
  bool deleted = JSSet::deleteKey(selfHandle, runtime, args.getArgHandle(0));
  return HermesValue::encodeBoolValue(deleted);
}

// ES12 23.2.3.5 Set.prototype.entries ( )
CallResult<HermesValue>
setPrototypeEntries(void *, Runtime &runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSSet>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError(
        "Non-Set object called on Set.prototype.entries");
  }
  auto iterator = runtime.makeHandle(
      JSSetIterator::create(
          runtime, Handle<JSObject>::vmcast(&runtime.setIteratorPrototype)));
  iterator->initializeIterator(runtime, selfHandle, IterationKind::Entry);
  return iterator.getHermesValue();
}

// ES12 23.2.3.6 Set.prototype.forEach ( callbackfn [ , thisArg ] )
CallResult<HermesValue>
setPrototypeForEach(void *, Runtime &runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSSet>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError(
        "Non-Set object called on Set.prototype.forEach");
  }

  auto callbackfn = Handle<Callable>::dyn_vmcast(args.getArgHandle(0));
  if (!callbackfn) {
    return runtime.raiseTypeError(
        "callbackfn must be a function in Set.prototype.forEach");
  }

  auto thisArg = args.getArgHandle(1);

  if (JSSet::forEach(selfHandle, runtime, callbackfn, thisArg) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  return HermesValue::encodeUndefinedValue();
}

// ES12 23.2.3.7 Set.prototype.has ( value )
CallResult<HermesValue>
setPrototypeHas(void *, Runtime &runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSSet>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError("Non-Set object called on Set.prototype.has");
  }
  bool has = JSSet::hasKey(selfHandle, runtime, args.getArgHandle(0));
  return HermesValue::encodeBoolValue(has);
}

CallResult<HermesValue>
setPrototypeSizeGetter(void *, Runtime &runtime, NativeArgs args) {
  auto self = args.dyncastThis<JSSet>();
  if (LLVM_UNLIKELY(!self)) {
    return runtime.raiseTypeError(
        "Non-Set object called on Set.prototype.size getter");
  }
  return HermesValue::encodeUntrustedNumberValue(
      JSSet::getSize(self.get(), runtime));
}

CallResult<HermesValue>
setPrototypeValues(void *, Runtime &runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSSet>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError(
        "Non-Set object called on Set.prototype.values");
  }
  auto iterator = runtime.makeHandle(
      JSSetIterator::create(
          runtime, Handle<JSObject>::vmcast(&runtime.setIteratorPrototype)));
  iterator->initializeIterator(runtime, selfHandle, IterationKind::Value);
  return iterator.getHermesValue();
}

// Helper functions for Set-like object support
namespace {

/// Get the size of a Set-like object
CallResult<uint32_t> getSetLikeSize(Runtime &runtime, Handle<> obj) {
  // If it's a real Set, use optimized path
  if (auto setHandle = Handle<JSSet>::dyn_vmcast(obj)) {
    return JSSet::getSize(setHandle.get(), runtime);
  }

  // Convert to object
  auto objRes = toObject(runtime, obj);
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto objHandle = runtime.makeHandle<JSObject>(objRes.getValue());

  // Get size property
  auto sizeRes = JSObject::getNamed_RJS(
      objHandle, runtime, Predefined::getSymbolID(Predefined::size));
  if (LLVM_UNLIKELY(sizeRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // Convert to number
  auto numRes = toNumber_RJS(runtime, runtime.makeHandle(std::move(*sizeRes)));
  if (LLVM_UNLIKELY(numRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  double size = numRes->getNumber();
  if (size < 0)
    size = 0;
  if (size > UINT32_MAX)
    size = UINT32_MAX;

  return static_cast<uint32_t>(size);
}

/// Check if a Set-like object has a value
CallResult<bool> setLikeHas(Runtime &runtime, Handle<> obj, Handle<> value) {
  // If it's a real Set, use optimized path
  if (auto setHandle = Handle<JSSet>::dyn_vmcast(obj)) {
    return JSSet::hasKey(setHandle, runtime, value);
  }

  // Convert to object
  auto objRes = toObject(runtime, obj);
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto objHandle = runtime.makeHandle<JSObject>(objRes.getValue());

  // Get has method
  auto hasRes = JSObject::getNamed_RJS(
      objHandle, runtime, Predefined::getSymbolID(Predefined::has));
  if (LLVM_UNLIKELY(hasRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // Check if callable
  auto hasMethod =
      Handle<Callable>::dyn_vmcast(runtime.makeHandle(std::move(*hasRes)));
  if (!hasMethod) {
    return runtime.raiseTypeError("Set-like object 'has' is not callable");
  }

  // Call has method
  auto callRes =
      Callable::executeCall1(hasMethod, runtime, objHandle, value.get());
  if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return toBoolean(callRes->get());
}

/// Iterate a Set-like object using keys() method
template <typename Callback>
CallResult<HermesValue>
iterateSetLike(Runtime &runtime, Handle<> obj, Callback callback) {
  // If it's a real Set, use optimized path
  if (auto setHandle = Handle<JSSet>::dyn_vmcast(obj)) {
    MutableHandle<HashMapEntry> entry{runtime};
    GCScopeMarkerRAII marker{runtime};
    for (entry = setHandle->iteratorNext(runtime, nullptr); entry;
         entry = setHandle->iteratorNext(runtime, entry.get())) {
      marker.flush();
      Handle<> value = runtime.makeHandle(entry->key);
      auto res = callback(value);
      if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    }
    return HermesValue::encodeUndefinedValue();
  }

  // Convert to object
  auto objRes = toObject(runtime, obj);
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto objHandle = runtime.makeHandle<JSObject>(objRes.getValue());

  // Get keys method
  auto keysRes = JSObject::getNamed_RJS(
      objHandle, runtime, Predefined::getSymbolID(Predefined::keys));
  if (LLVM_UNLIKELY(keysRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // Check if callable
  auto keysMethod =
      Handle<Callable>::dyn_vmcast(runtime.makeHandle(std::move(*keysRes)));
  if (!keysMethod) {
    return runtime.raiseTypeError("Set-like object 'keys' is not callable");
  }

  // Call keys() to get iterator
  auto iterRes = Callable::executeCall0(keysMethod, runtime, objHandle);
  if (LLVM_UNLIKELY(iterRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // Use iterator protocol
  auto getIterRes =
      getIterator(runtime, runtime.makeHandle(std::move(*iterRes)));
  if (LLVM_UNLIKELY(getIterRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto iterRecord = *getIterRes;

  GCScopeMarkerRAII marker{runtime};

  // Iterate
  while (true) {
    marker.flush();

    auto nextRes = iteratorNext(runtime, iterRecord);
    if (LLVM_UNLIKELY(nextRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    auto next = runtime.makeHandle<JSObject>(std::move(*nextRes));

    // Check done
    auto doneRes = JSObject::getNamed_RJS(
        next, runtime, Predefined::getSymbolID(Predefined::done));
    if (LLVM_UNLIKELY(doneRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (toBoolean(doneRes->get())) {
      break;
    }

    // Get value
    auto valueRes = JSObject::getNamed_RJS(
        next, runtime, Predefined::getSymbolID(Predefined::value));
    if (LLVM_UNLIKELY(valueRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    auto value = runtime.makeHandle(std::move(*valueRes));
    auto res = callback(value);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
  }

  return HermesValue::encodeUndefinedValue();
}

} // namespace

// ES2025 Set.prototype.intersection ( other )
CallResult<HermesValue>
setPrototypeIntersection(void *, Runtime &runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSSet>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError(
        "Set.prototype.intersection called on non-Set object");
  }

  if (args.getArgCount() == 0) {
    return runtime.raiseTypeError(
        "Set.prototype.intersection requires 1 argument");
  }

  auto other = args.getArgHandle(0);

  // Create result Set
  auto resultHandle = runtime.makeHandle<JSSet>(
      JSSet::create(runtime, Handle<JSObject>::vmcast(&runtime.setPrototype)));
  JSSet::initializeStorage(resultHandle, runtime);

  // Get sizes for optimization
  uint32_t selfSize = JSSet::getSize(selfHandle.get(), runtime);
  auto otherSizeRes = getSetLikeSize(runtime, other);
  if (LLVM_UNLIKELY(otherSizeRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint32_t otherSize = *otherSizeRes;

  // Iterate the smaller set for efficiency
  if (selfSize <= otherSize) {
    // Iterate self, check membership in other
    MutableHandle<HashMapEntry> entry{runtime};
    GCScopeMarkerRAII marker{runtime};
    for (entry = selfHandle->iteratorNext(runtime, nullptr); entry;
         entry = selfHandle->iteratorNext(runtime, entry.get())) {
      marker.flush();
      Handle<> value = runtime.makeHandle(entry->key);

      auto hasRes = setLikeHas(runtime, other, value);
      if (LLVM_UNLIKELY(hasRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }

      if (*hasRes) {
        JSSet::addValue(resultHandle, runtime, value, value);
      }
    }
  } else {
    // Iterate other, check membership in self
    auto iterRes =
        iterateSetLike(runtime, other, [&](Handle<> value) -> ExecutionStatus {
          if (JSSet::hasKey(selfHandle, runtime, value)) {
            JSSet::addValue(resultHandle, runtime, value, value);
          }
          return ExecutionStatus::RETURNED;
        });
    if (LLVM_UNLIKELY(iterRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
  }

  return resultHandle.getHermesValue();
}

// ES2025 Set.prototype.union ( other )
CallResult<HermesValue>
setPrototypeUnion(void *, Runtime &runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSSet>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError(
        "Set.prototype.union called on non-Set object");
  }

  if (args.getArgCount() == 0) {
    return runtime.raiseTypeError("Set.prototype.union requires 1 argument");
  }

  auto other = args.getArgHandle(0);

  // Create result Set
  auto resultHandle = runtime.makeHandle<JSSet>(
      JSSet::create(runtime, Handle<JSObject>::vmcast(&runtime.setPrototype)));
  JSSet::initializeStorage(resultHandle, runtime);

  // Add all elements from self
  MutableHandle<HashMapEntry> entry{runtime};
  GCScopeMarkerRAII marker{runtime};
  for (entry = selfHandle->iteratorNext(runtime, nullptr); entry;
       entry = selfHandle->iteratorNext(runtime, entry.get())) {
    marker.flush();
    Handle<> value = runtime.makeHandle(entry->key);
    JSSet::addValue(resultHandle, runtime, value, value);
  }

  // Add all elements from other
  auto iterRes =
      iterateSetLike(runtime, other, [&](Handle<> value) -> ExecutionStatus {
        JSSet::addValue(resultHandle, runtime, value, value);
        return ExecutionStatus::RETURNED;
      });
  if (LLVM_UNLIKELY(iterRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return resultHandle.getHermesValue();
}

// ES2025 Set.prototype.difference ( other )
CallResult<HermesValue>
setPrototypeDifference(void *, Runtime &runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSSet>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError(
        "Set.prototype.difference called on non-Set object");
  }

  if (args.getArgCount() == 0) {
    return runtime.raiseTypeError(
        "Set.prototype.difference requires 1 argument");
  }

  auto other = args.getArgHandle(0);

  // Create result Set
  auto resultHandle = runtime.makeHandle<JSSet>(
      JSSet::create(runtime, Handle<JSObject>::vmcast(&runtime.setPrototype)));
  JSSet::initializeStorage(resultHandle, runtime);

  // Add elements from self that are not in other
  MutableHandle<HashMapEntry> entry{runtime};
  GCScopeMarkerRAII marker{runtime};
  for (entry = selfHandle->iteratorNext(runtime, nullptr); entry;
       entry = selfHandle->iteratorNext(runtime, entry.get())) {
    marker.flush();
    Handle<> value = runtime.makeHandle(entry->key);

    auto hasRes = setLikeHas(runtime, other, value);
    if (LLVM_UNLIKELY(hasRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    if (!*hasRes) {
      JSSet::addValue(resultHandle, runtime, value, value);
    }
  }

  return resultHandle.getHermesValue();
}

// ES2025 Set.prototype.symmetricDifference ( other )
CallResult<HermesValue>
setPrototypeSymmetricDifference(void *, Runtime &runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSSet>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError(
        "Set.prototype.symmetricDifference called on non-Set object");
  }

  if (args.getArgCount() == 0) {
    return runtime.raiseTypeError(
        "Set.prototype.symmetricDifference requires 1 argument");
  }

  auto other = args.getArgHandle(0);

  // Create result Set
  auto resultHandle = runtime.makeHandle<JSSet>(
      JSSet::create(runtime, Handle<JSObject>::vmcast(&runtime.setPrototype)));
  JSSet::initializeStorage(resultHandle, runtime);

  // Add elements from self that are not in other
  MutableHandle<HashMapEntry> entry{runtime};
  GCScopeMarkerRAII marker{runtime};
  for (entry = selfHandle->iteratorNext(runtime, nullptr); entry;
       entry = selfHandle->iteratorNext(runtime, entry.get())) {
    marker.flush();
    Handle<> value = runtime.makeHandle(entry->key);

    auto hasRes = setLikeHas(runtime, other, value);
    if (LLVM_UNLIKELY(hasRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    if (!*hasRes) {
      JSSet::addValue(resultHandle, runtime, value, value);
    }
  }

  // Add elements from other that are not in self
  auto iterRes =
      iterateSetLike(runtime, other, [&](Handle<> value) -> ExecutionStatus {
        if (!JSSet::hasKey(selfHandle, runtime, value)) {
          JSSet::addValue(resultHandle, runtime, value, value);
        }
        return ExecutionStatus::RETURNED;
      });
  if (LLVM_UNLIKELY(iterRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return resultHandle.getHermesValue();
}

// ES2025 Set.prototype.isSubsetOf ( other )
CallResult<HermesValue>
setPrototypeIsSubsetOf(void *, Runtime &runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSSet>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError(
        "Set.prototype.isSubsetOf called on non-Set object");
  }

  if (args.getArgCount() == 0) {
    return runtime.raiseTypeError(
        "Set.prototype.isSubsetOf requires 1 argument");
  }

  auto other = args.getArgHandle(0);

  // Check sizes first - if self is larger, it cannot be a subset
  uint32_t selfSize = JSSet::getSize(selfHandle.get(), runtime);
  auto otherSizeRes = getSetLikeSize(runtime, other);
  if (LLVM_UNLIKELY(otherSizeRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint32_t otherSize = *otherSizeRes;

  if (selfSize > otherSize) {
    return HermesValue::encodeBoolValue(false);
  }

  // Check if every element of self is in other
  MutableHandle<HashMapEntry> entry{runtime};
  GCScopeMarkerRAII marker{runtime};
  for (entry = selfHandle->iteratorNext(runtime, nullptr); entry;
       entry = selfHandle->iteratorNext(runtime, entry.get())) {
    marker.flush();
    Handle<> value = runtime.makeHandle(entry->key);

    auto hasRes = setLikeHas(runtime, other, value);
    if (LLVM_UNLIKELY(hasRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    if (!*hasRes) {
      return HermesValue::encodeBoolValue(false);
    }
  }

  return HermesValue::encodeBoolValue(true);
}

// ES2025 Set.prototype.isSupersetOf ( other )
CallResult<HermesValue>
setPrototypeIsSupersetOf(void *, Runtime &runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSSet>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError(
        "Set.prototype.isSupersetOf called on non-Set object");
  }

  if (args.getArgCount() == 0) {
    return runtime.raiseTypeError(
        "Set.prototype.isSupersetOf requires 1 argument");
  }

  auto other = args.getArgHandle(0);

  // Check sizes first - if other is larger, self cannot be a superset
  uint32_t selfSize = JSSet::getSize(selfHandle.get(), runtime);
  auto otherSizeRes = getSetLikeSize(runtime, other);
  if (LLVM_UNLIKELY(otherSizeRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint32_t otherSize = *otherSizeRes;

  if (otherSize > selfSize) {
    return HermesValue::encodeBoolValue(false);
  }

  // Check if every element of other is in self
  bool allInSelf = true;
  auto iterRes =
      iterateSetLike(runtime, other, [&](Handle<> value) -> ExecutionStatus {
        if (!JSSet::hasKey(selfHandle, runtime, value)) {
          allInSelf = false;
        }
        return ExecutionStatus::RETURNED;
      });
  if (LLVM_UNLIKELY(iterRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return HermesValue::encodeBoolValue(allInSelf);
}

// ES2025 Set.prototype.isDisjointFrom ( other )
CallResult<HermesValue>
setPrototypeIsDisjointFrom(void *, Runtime &runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSSet>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError(
        "Set.prototype.isDisjointFrom called on non-Set object");
  }

  if (args.getArgCount() == 0) {
    return runtime.raiseTypeError(
        "Set.prototype.isDisjointFrom requires 1 argument");
  }

  auto other = args.getArgHandle(0);

  // Get sizes for optimization - iterate the smaller set
  uint32_t selfSize = JSSet::getSize(selfHandle.get(), runtime);
  auto otherSizeRes = getSetLikeSize(runtime, other);
  if (LLVM_UNLIKELY(otherSizeRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint32_t otherSize = *otherSizeRes;

  if (selfSize <= otherSize) {
    // Check if any element of self is in other
    MutableHandle<HashMapEntry> entry{runtime};
    GCScopeMarkerRAII marker{runtime};
    for (entry = selfHandle->iteratorNext(runtime, nullptr); entry;
         entry = selfHandle->iteratorNext(runtime, entry.get())) {
      marker.flush();
      Handle<> value = runtime.makeHandle(entry->key);

      auto hasRes = setLikeHas(runtime, other, value);
      if (LLVM_UNLIKELY(hasRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }

      if (*hasRes) {
        return HermesValue::encodeBoolValue(false);
      }
    }
  } else {
    // Check if any element of other is in self
    bool hasCommon = false;
    auto iterRes =
        iterateSetLike(runtime, other, [&](Handle<> value) -> ExecutionStatus {
          if (JSSet::hasKey(selfHandle, runtime, value)) {
            hasCommon = true;
          }
          return ExecutionStatus::RETURNED;
        });
    if (LLVM_UNLIKELY(iterRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    if (hasCommon) {
      return HermesValue::encodeBoolValue(false);
    }
  }

  return HermesValue::encodeBoolValue(true);
}
Handle<JSObject> createSetIteratorPrototype(Runtime &runtime) {
  auto parentHandle = runtime.makeHandle(
      JSObject::create(
          runtime, Handle<JSObject>::vmcast(&runtime.iteratorPrototype)));
  defineMethod(
      runtime,
      parentHandle,
      Predefined::getSymbolID(Predefined::next),
      nullptr,
      setIteratorPrototypeNext,
      0);

  DefinePropertyFlags dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  defineProperty(
      runtime,
      parentHandle,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime.getPredefinedStringHandle(Predefined::SetIterator),
      dpf);

  return parentHandle;
}

CallResult<HermesValue>
setIteratorPrototypeNext(void *, Runtime &runtime, NativeArgs args) {
  auto selfHandle = args.dyncastThis<JSSetIterator>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError(
        "Non-SetIterator object called on SetIterator.prototype.next");
  }
  auto cr = JSSetIterator::nextElement(selfHandle, runtime);
  if (cr == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  return *cr;
}
} // namespace vm
} // namespace hermes