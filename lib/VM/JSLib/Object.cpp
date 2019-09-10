/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
//===----------------------------------------------------------------------===//
/// \file
/// ES5.1 15.2 Initialize the Object constructor.
//===----------------------------------------------------------------------===//
#include "JSLibInternal.h"

#include "hermes/VM/HermesValueTraits.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/StringBuilder.h"

namespace hermes {
namespace vm {

/// Initialize a freshly created instance of Object.
static inline HermesValue objectInitInstance(
    Handle<JSObject> thisHandle,
    Runtime *) {
  return thisHandle.getHermesValue();
}

//===----------------------------------------------------------------------===//
/// Object.

Handle<JSObject> createObjectConstructor(Runtime *runtime) {
  auto objectPrototype = Handle<JSObject>::vmcast(&runtime->objectPrototype);

  auto cons = defineSystemConstructor(
      runtime,
      Predefined::getSymbolID(Predefined::Object),
      objectConstructor,
      Handle<JSObject>::vmcast(&runtime->objectPrototype),
      1,
      JSObject::createWithException,
      CellKind::ObjectKind);
  void *ctx = nullptr;

  // Object.prototype.xxx methods.
  defineMethod(
      runtime,
      objectPrototype,
      Predefined::getSymbolID(Predefined::toString),
      ctx,
      objectPrototypeToString,
      0);
  defineMethod(
      runtime,
      objectPrototype,
      Predefined::getSymbolID(Predefined::toLocaleString),
      ctx,
      objectPrototypeToLocaleString,
      0);
  defineMethod(
      runtime,
      objectPrototype,
      Predefined::getSymbolID(Predefined::valueOf),
      ctx,
      objectPrototypeValueOf,
      0);
  defineMethod(
      runtime,
      objectPrototype,
      Predefined::getSymbolID(Predefined::hasOwnProperty),
      ctx,
      objectPrototypeHasOwnProperty,
      1);
  defineMethod(
      runtime,
      objectPrototype,
      Predefined::getSymbolID(Predefined::isPrototypeOf),
      ctx,
      objectPrototypeIsPrototypeOf,
      1);
  defineMethod(
      runtime,
      objectPrototype,
      Predefined::getSymbolID(Predefined::propertyIsEnumerable),
      ctx,
      objectPrototypePropertyIsEnumerable,
      1);
  defineAccessor(
      runtime,
      objectPrototype,
      Predefined::getSymbolID(Predefined::underscore_proto),
      ctx,
      objectPrototypeProto_getter,
      objectPrototypeProto_setter,
      false,
      true);
  defineMethod(
      runtime,
      objectPrototype,
      Predefined::getSymbolID(Predefined::__defineGetter__),
      ctx,
      objectPrototypeDefineGetter,
      2);
  defineMethod(
      runtime,
      objectPrototype,
      Predefined::getSymbolID(Predefined::__defineSetter__),
      ctx,
      objectPrototypeDefineSetter,
      2);
  defineMethod(
      runtime,
      objectPrototype,
      Predefined::getSymbolID(Predefined::__lookupGetter__),
      ctx,
      objectPrototypeLookupGetter,
      1);
  defineMethod(
      runtime,
      objectPrototype,
      Predefined::getSymbolID(Predefined::__lookupSetter__),
      ctx,
      objectPrototypeLookupSetter,
      1);

  // Object.xxx() methods.
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::getPrototypeOf),
      ctx,
      objectGetPrototypeOf,
      1);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::getOwnPropertyDescriptor),
      ctx,
      objectGetOwnPropertyDescriptor,
      2);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::getOwnPropertyNames),
      ctx,
      objectGetOwnPropertyNames,
      1);
  if (runtime->hasES6Symbol()) {
    defineMethod(
        runtime,
        cons,
        Predefined::getSymbolID(Predefined::getOwnPropertySymbols),
        ctx,
        objectGetOwnPropertySymbols,
        1);
  }
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::seal),
      ctx,
      objectSeal,
      1);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::freeze),
      ctx,
      objectFreeze,
      1);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::preventExtensions),
      ctx,
      objectPreventExtensions,
      1);

  defineMethod(
      runtime, cons, Predefined::getSymbolID(Predefined::is), ctx, objectIs, 2);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::isSealed),
      ctx,
      objectIsSealed,
      1);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::isFrozen),
      ctx,
      objectIsFrozen,
      1);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::isExtensible),
      ctx,
      objectIsExtensible,
      1);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::keys),
      ctx,
      objectKeys,
      1);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::values),
      ctx,
      objectValues,
      1);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::entries),
      ctx,
      objectEntries,
      1);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::create),
      ctx,
      objectCreate,
      2);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::defineProperty),
      ctx,
      objectDefineProperty,
      3);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::defineProperties),
      ctx,
      objectDefineProperties,
      2);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::assign),
      ctx,
      objectAssign,
      2);
  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::setPrototypeOf),
      ctx,
      objectSetPrototypeOf,
      2);

  return cons;
}

/// ES5.1 15.2.1.1 and 15.2.2.1. Object() invoked as a function and as a
/// constructor.
CallResult<HermesValue>
objectConstructor(void *, Runtime *runtime, NativeArgs args) {
  auto arg0 = args.getArgHandle(0);

  // If arg0 is supplied and is not null or undefined, call ToObject().
  {
    if (!arg0->isUndefined() && !arg0->isNull()) {
      return toObject(runtime, arg0);
    }
  }

  // The other cases must have been handled above.
  assert(arg0->isUndefined() || arg0->isNull());

  if (args.isConstructorCall()) {
    assert(
        args.getThisArg().isObject() &&
        "'this' must be an object in a constructor call");
    return objectInitInstance(
        Handle<JSObject>::vmcast(&args.getThisArg()), runtime);
  }

  // This is a function call that must act as a constructor and create a new
  // object.
  auto thisHandle = toHandle(runtime, JSObject::create(runtime));

  return objectInitInstance(thisHandle, runtime);
}

CallResult<HermesValue>
objectGetPrototypeOf(void *, Runtime *runtime, NativeArgs args) {
  auto res = toObject(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSObject> obj = runtime->makeHandle<JSObject>(res.getValue());

  // Note that we must return 'null' if there is no prototype.
  JSObject *parent = obj->getParent(runtime);
  return parent ? HermesValue::encodeObjectValue(parent)
                : HermesValue::encodeNullValue();
}

CallResult<HermesValue>
objectGetOwnPropertyDescriptor(void *, Runtime *runtime, NativeArgs args) {
  auto objRes = toObject(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSObject> O = runtime->makeHandle<JSObject>(objRes.getValue());

  ComputedPropertyDescriptor desc;
  {
    auto result = JSObject::getOwnComputedDescriptor(
        O, runtime, args.getArgHandle(1), desc);
    if (result == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!*result) {
      if (LLVM_LIKELY(!O->isHostObject()))
        return HermesValue::encodeUndefinedValue();
      // For compatibility with polyfills we want to pretend that all HostObject
      // properties are "own" properties in hasOwnProperty() and in
      // getOwnPropertyDescriptor(). Since there is no way to check for a
      // HostObject property, we must always assume the property exists.
      desc.flags.enumerable = true;
      desc.flags.writable = true;
      desc.flags.hostObject = true;
    }
  }

  auto obj = toHandle(runtime, JSObject::create(runtime));

  DefinePropertyFlags dpf{};
  dpf.setValue = 1;
  dpf.setWritable = 1;
  dpf.setEnumerable = 1;
  dpf.setConfigurable = 1;
  dpf.writable = 1;
  dpf.enumerable = 1;
  dpf.configurable = 1;

  if (!desc.flags.accessor) {
    // Data Descriptor
    MutableHandle<> value{runtime};
    if (LLVM_LIKELY(!desc.flags.hostObject)) {
      value = JSObject::getComputedSlotValue(O.get(), runtime, desc);
    } else {
      auto propRes =
          JSObject::getComputed_RJS(O, runtime, args.getArgHandle(1));
      if (propRes == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      value = *propRes;
    }

    auto result = JSObject::defineOwnProperty(
        obj,
        runtime,
        Predefined::getSymbolID(Predefined::value),
        dpf,
        value,
        PropOpFlags().plusThrowOnError());
    assert(
        result != ExecutionStatus::EXCEPTION &&
        "defineOwnProperty() failed on a new object");
    if (result == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }

    result = JSObject::defineOwnProperty(
        obj,
        runtime,
        Predefined::getSymbolID(Predefined::writable),
        dpf,
        Runtime::getBoolValue(desc.flags.writable),
        PropOpFlags().plusThrowOnError());
    assert(
        result != ExecutionStatus::EXCEPTION &&
        "defineOwnProperty() failed on a new object");
    if (result == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  } else {
    // Accessor
    auto *accessor = vmcast<PropertyAccessor>(
        JSObject::getComputedSlotValue(O.get(), runtime, desc));

    auto getter = runtime->makeHandle(
        accessor->getter
            ? HermesValue::encodeObjectValue(accessor->getter.get(runtime))
            : HermesValue::encodeUndefinedValue());

    auto setter = runtime->makeHandle(
        accessor->setter
            ? HermesValue::encodeObjectValue(accessor->setter.get(runtime))
            : HermesValue::encodeUndefinedValue());

    auto result = JSObject::defineOwnProperty(
        obj,
        runtime,
        Predefined::getSymbolID(Predefined::get),
        dpf,
        getter,
        PropOpFlags().plusThrowOnError());
    assert(
        result != ExecutionStatus::EXCEPTION &&
        "defineOwnProperty() failed on a new object");
    if (result == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }

    result = JSObject::defineOwnProperty(
        obj,
        runtime,
        Predefined::getSymbolID(Predefined::set),
        dpf,
        setter,
        PropOpFlags().plusThrowOnError());
    assert(
        result != ExecutionStatus::EXCEPTION &&
        "defineOwnProperty() failed on a new object");
    if (result == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  }

  auto result = JSObject::defineOwnProperty(
      obj,
      runtime,
      Predefined::getSymbolID(Predefined::enumerable),
      dpf,
      Runtime::getBoolValue(desc.flags.enumerable),
      PropOpFlags().plusThrowOnError());
  assert(
      result != ExecutionStatus::EXCEPTION &&
      "defineOwnProperty() failed on a new object");
  if (result == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  result = JSObject::defineOwnProperty(
      obj,
      runtime,
      Predefined::getSymbolID(Predefined::configurable),
      dpf,
      Runtime::getBoolValue(desc.flags.configurable),
      PropOpFlags().plusThrowOnError());
  assert(
      result != ExecutionStatus::EXCEPTION &&
      "defineOwnProperty() failed on a new object");
  if (result == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  return obj.getHermesValue();
}

/// Return a list of property names belonging to this object. All
/// properties are converted into strings. The order of
/// properties will remain the same as Object::getOwnPropertyNames.
/// \param onlyEnumerable if true, only enumerable properties will be
///   returned.
/// \returns a JSArray containing the names, encoded in HermesValue.
static CallResult<HermesValue> getOwnPropertyNamesAsStrings(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    bool onlyEnumerable) {
  auto cr = JSObject::getOwnPropertyNames(selfHandle, runtime, onlyEnumerable);
  if (cr == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto array = *cr;
  MutableHandle<> prop(runtime);
  GCScope gcScope(runtime);
  auto marker = gcScope.createMarker();
  for (unsigned i = 0, e = array->getEndIndex(); i < e; ++i) {
    gcScope.flushToMarker(marker);
    prop = array->at(runtime, i);
    if (prop->isString()) {
      // Nothing to do if it's already a string.
      continue;
    }
    assert(prop->isNumber() && "Property name is either string or number");
    // Otherwise convert it to a string and replace the element.
    auto status = toString_RJS(runtime, prop);
    assert(
        status != ExecutionStatus::EXCEPTION &&
        "toString() on property name cannot fail");
    JSArray::setElementAt(
        array, runtime, i, toHandle(runtime, std::move(*status)));
  }
  return array.getHermesValue();
}

CallResult<HermesValue>
objectGetOwnPropertyNames(void *, Runtime *runtime, NativeArgs args) {
  auto objRes = toObject(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto objHandle = runtime->makeHandle<JSObject>(objRes.getValue());
  auto cr = getOwnPropertyNamesAsStrings(
      objHandle, runtime, false /*onlyEnumerable*/);
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return *cr;
}

CallResult<HermesValue>
objectGetOwnPropertySymbols(void *, Runtime *runtime, NativeArgs args) {
  auto objRes = toObject(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto objHandle = runtime->makeHandle<JSObject>(objRes.getValue());
  auto cr = JSObject::getOwnPropertySymbols(objHandle, runtime);
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return cr->getHermesValue();
}

/// ES5.1 8.10.5. Object.toPropertyDescriptor(O). The result is written into
/// \p flags and \p valueOrAccessor together to represent a descriptor.
static ExecutionStatus objectToPropertyDescriptor(
    Handle<> obj,
    Runtime *runtime,
    DefinePropertyFlags &flags,
    MutableHandle<> &valueOrAccessor) {
  GCScopeMarkerRAII gcMarker{runtime};

  // Verify that the attributes argument is also an object.
  auto attributes = Handle<JSObject>::dyn_vmcast(obj);
  if (!attributes) {
    return runtime->raiseTypeError(
        "Object.defineProperty() Attributes argument is not an object");
  }

  NamedPropertyDescriptor desc;

  // Get enumerable property of the attributes.
  if (JSObject::getNamedDescriptor(
          attributes,
          runtime,
          Predefined::getSymbolID(Predefined::enumerable),
          desc)) {
    auto propRes = JSObject::getNamed_RJS(
        attributes,
        runtime,
        Predefined::getSymbolID(Predefined::enumerable),
        PropOpFlags().plusThrowOnError());
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    flags.enumerable = toBoolean(*propRes);
    flags.setEnumerable = true;
  }

  // Get configurable property of the attributes.
  if (JSObject::getNamedDescriptor(
          attributes,
          runtime,
          Predefined::getSymbolID(Predefined::configurable),
          desc)) {
    auto propRes = JSObject::getNamed_RJS(
        attributes,
        runtime,
        Predefined::getSymbolID(Predefined::configurable),
        PropOpFlags().plusThrowOnError());
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    flags.configurable = toBoolean(*propRes);
    flags.setConfigurable = true;
  }

  // Get value property of the attributes.
  if (JSObject::getNamedDescriptor(
          attributes,
          runtime,
          Predefined::getSymbolID(Predefined::value),
          desc)) {
    auto propRes = JSObject::getNamed_RJS(
        attributes,
        runtime,
        Predefined::getSymbolID(Predefined::value),
        PropOpFlags().plusThrowOnError());
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    valueOrAccessor = *propRes;
    flags.setValue = true;
  }

  // Get writable property of the attributes.
  if (JSObject::getNamedDescriptor(
          attributes,
          runtime,
          Predefined::getSymbolID(Predefined::writable),
          desc)) {
    auto propRes = JSObject::getNamed_RJS(
        attributes,
        runtime,
        Predefined::getSymbolID(Predefined::writable),
        PropOpFlags().plusThrowOnError());
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    flags.writable = toBoolean(*propRes);
    flags.setWritable = true;
  }

  // Get getter property of the attributes.
  MutableHandle<Callable> getterPtr{runtime};
  if (JSObject::getNamedDescriptor(
          attributes,
          runtime,
          Predefined::getSymbolID(Predefined::get),
          desc)) {
    auto propRes = JSObject::getNamed_RJS(
        attributes,
        runtime,
        Predefined::getSymbolID(Predefined::get),
        PropOpFlags().plusThrowOnError());
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    flags.setGetter = true;
    auto getter = *propRes;
    if (LLVM_LIKELY(!getter.isUndefined())) {
      getterPtr = dyn_vmcast<Callable>(getter);
      if (LLVM_UNLIKELY(!getterPtr)) {
        return runtime->raiseTypeError(
            "Invalid property descriptor. Getter must be a function.");
      }
    }
  }

  // Get setter property of the attributes.
  MutableHandle<Callable> setterPtr{runtime};
  if (JSObject::getNamedDescriptor(
          attributes,
          runtime,
          Predefined::getSymbolID(Predefined::set),
          desc)) {
    auto propRes = JSObject::getNamed_RJS(
        attributes,
        runtime,
        Predefined::getSymbolID(Predefined::set),
        PropOpFlags().plusThrowOnError());
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    flags.setSetter = true;
    auto setter = *propRes;
    if (LLVM_LIKELY(!setter.isUndefined())) {
      setterPtr = dyn_vmcast<Callable>(setter);
      if (LLVM_UNLIKELY(!setterPtr)) {
        return runtime->raiseTypeError(
            "Invalid property descriptor. Setter must be a function.");
      }
    }
  }

  // Construct property accessor if getter/setter is set.
  if (flags.setSetter || flags.setGetter) {
    if (flags.setValue) {
      return runtime->raiseTypeError(
          "Invalid property descriptor. Can't set both accessor and value.");
    }
    if (flags.setWritable) {
      return runtime->raiseTypeError(
          "Invalid property descriptor. Can't set both accessor and writable.");
    }
    auto crtRes = PropertyAccessor::create(runtime, getterPtr, setterPtr);
    if (LLVM_UNLIKELY(crtRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    valueOrAccessor = *crtRes;
  }

  return ExecutionStatus::RETURNED;
}

CallResult<HermesValue>
objectDefineProperty(void *, Runtime *runtime, NativeArgs args) {
  auto O = args.dyncastArg<JSObject>(0);
  // Verify this method is called on an object.
  if (!O) {
    return runtime->raiseTypeError(
        "Object.defineProperty() argument is not an object");
  }

  // Convert the property name to string if it's an object.
  auto nameValHandle = args.getArgHandle(1);

  DefinePropertyFlags flags;
  MutableHandle<> valueOrAccessor{runtime};
  if (objectToPropertyDescriptor(
          args.getArgHandle(2), runtime, flags, valueOrAccessor) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  // Define the property.
  // We should handle the exception here instead of depending on runtime to do
  // it.
  CallResult<bool> res = JSObject::defineOwnComputed(
      O,
      runtime,
      nameValHandle,
      flags,
      valueOrAccessor,
      PropOpFlags().plusThrowOnError());
  if (res == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;
  return O.getHermesValue();
}

static CallResult<HermesValue>
objectDefinePropertiesInternal(Runtime *runtime, Handle<> obj, Handle<> props) {
  // Verify this method is called on an object.
  auto *objPtr = dyn_vmcast<JSObject>(obj.get());
  if (!objPtr) {
    return runtime->raiseTypeError(
        "Object.defineProperties() argument is not an object");
  }
  auto objHandle = runtime->makeHandle(objPtr);

  // Verify that the properties argument is also an object.
  auto objRes = toObject(runtime, props);
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto propsHandle = runtime->makeHandle<JSObject>(objRes.getValue());

  // Get the list of identifiers in props.
  auto cr = JSObject::getOwnPropertyNames(propsHandle, runtime, true);
  if (cr == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto enumerablePropNames = *cr;

  // This function may create an unbounded number of GC handles.
  GCScope scope{runtime, "objectDefinePropertiesInternal", UINT_MAX};

  // We store each enumerable property name here. This is hoisted out of the
  // loop to avoid allocating a handler per property.
  MutableHandle<> propName{runtime};

  // Iterate through every identifier, get the property descriptor object,
  // and store it in a list, according to Step 5.
  llvm::SmallVector<std::pair<DefinePropertyFlags, MutableHandle<>>, 4>
      descriptors;
  for (unsigned i = 0, e = enumerablePropNames->getEndIndex(); i < e; ++i) {
    propName = enumerablePropNames->at(runtime, i);
    auto propRes = JSObject::getComputed_RJS(propsHandle, runtime, propName);
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    DefinePropertyFlags flags;
    MutableHandle<> valueOrAccessor{runtime};
    if (LLVM_UNLIKELY(
            objectToPropertyDescriptor(
                runtime->makeHandle(*propRes),
                runtime,
                flags,
                valueOrAccessor) == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    descriptors.push_back(std::make_pair(flags, std::move(valueOrAccessor)));
  }

  // For each descriptor in the list, add it to the object.
  for (unsigned i = 0, e = descriptors.size(); i < e; ++i) {
    propName = enumerablePropNames->at(runtime, i);
    auto result = JSObject::defineOwnComputedPrimitive(
        objHandle,
        runtime,
        propName,
        descriptors[i].first,
        descriptors[i].second,
        PropOpFlags().plusThrowOnError());
    if (result == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  }
  return objHandle.getHermesValue();
}

CallResult<HermesValue>
objectCreate(void *, Runtime *runtime, NativeArgs args) {
  // Verify this method is called with an object or with 'null'.
  auto obj = args.dyncastArg<JSObject>(0);
  if (!obj && !args.getArg(0).isNull()) {
    return runtime->raiseTypeError(
        "Object prototype argument must be an Object or null");
  }

  auto newObj = objectInitInstance(
      toHandle(runtime, JSObject::create(runtime, obj)), runtime);
  auto arg1 = args.getArgHandle(1);
  if (arg1->isUndefined()) {
    return newObj;
  }
  // Properties argument is present and not undefined.
  auto cr = objectDefinePropertiesInternal(
      runtime, runtime->makeHandle(newObj), arg1);
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return *cr;
}

CallResult<HermesValue>
objectDefineProperties(void *, Runtime *runtime, NativeArgs args) {
  auto cr = objectDefinePropertiesInternal(
      runtime, args.getArgHandle(0), args.getArgHandle(1));
  if (cr == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  return *cr;
}

CallResult<HermesValue> objectSeal(void *, Runtime *runtime, NativeArgs args) {
  auto objHandle = args.dyncastArg<JSObject>(0);

  if (!objHandle) {
    return args.getArg(0);
  }

  JSObject::seal(objHandle, runtime);
  return objHandle.getHermesValue();
}

CallResult<HermesValue>
objectFreeze(void *, Runtime *runtime, NativeArgs args) {
  auto objHandle = args.dyncastArg<JSObject>(0);

  if (!objHandle) {
    return args.getArg(0);
  }

  JSObject::freeze(objHandle, runtime);
  return objHandle.getHermesValue();
}

CallResult<HermesValue>
objectPreventExtensions(void *, Runtime *runtime, NativeArgs args) {
  auto *obj = dyn_vmcast<JSObject>(args.getArg(0));

  if (!obj) {
    return args.getArg(0);
  }

  JSObject::preventExtensions(obj);
  return args.getArg(0);
}

CallResult<HermesValue> objectIs(void *, Runtime *runtime, NativeArgs args) {
  return HermesValue::encodeBoolValue(
      isSameValue(args.getArg(0), args.getArg(1)));
}

CallResult<HermesValue>
objectIsSealed(void *, Runtime *runtime, NativeArgs args) {
  auto objHandle = args.dyncastArg<JSObject>(0);

  if (!objHandle) {
    // ES6.0 19.1.2.13: If Type(O) is not Object, return true.
    return HermesValue::encodeBoolValue(true);
  }

  return HermesValue::encodeBoolValue(JSObject::isSealed(objHandle, runtime));
}

CallResult<HermesValue>
objectIsFrozen(void *, Runtime *runtime, NativeArgs args) {
  auto objHandle = args.dyncastArg<JSObject>(0);

  if (!objHandle) {
    // ES6.0 19.1.2.12: If Type(O) is not Object, return true.
    return HermesValue::encodeBoolValue(true);
  }

  return HermesValue::encodeBoolValue(JSObject::isFrozen(objHandle, runtime));
}

CallResult<HermesValue>
objectIsExtensible(void *, Runtime *runtime, NativeArgs args) {
  auto *obj = dyn_vmcast<JSObject>(args.getArg(0));

  if (!obj) {
    // ES6.0 19.1.2.11: If Type(O) is not Object, return false.
    return HermesValue::encodeBoolValue(false);
  }

  return HermesValue::encodeBoolValue(obj->isExtensible());
}

namespace {
/// "Kind" provided to enumerableOwnProperties to request different
/// representation of the properties in the object.
enum class EnumerableOwnPropertiesKind {
  Key,
  Value,
  KeyValue,
};
} // namespace

/// ES8.0 7.3.21.
/// EnumerableOwnProperties gets the requested properties based on \p kind.
static CallResult<HermesValue> enumerableOwnProperties(
    Runtime *runtime,
    NativeArgs args,
    EnumerableOwnPropertiesKind kind) {
  GCScope gcScope{runtime};

  auto objRes = toObject(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto objHandle = runtime->makeHandle<JSObject>(objRes.getValue());

  auto namesRes =
      getOwnPropertyNamesAsStrings(objHandle, runtime, true /*onlyEnumerable*/);
  if (namesRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  if (kind == EnumerableOwnPropertiesKind::Key) {
    return *namesRes;
  }
  auto names = runtime->makeHandle<JSArray>(*namesRes);
  uint32_t len = JSArray::getLength(*names);

  auto propertiesRes = JSArray::create(runtime, len, len);
  if (LLVM_UNLIKELY(propertiesRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto properties = toHandle(runtime, std::move(*propertiesRes));

  MutableHandle<StringPrimitive> name{runtime};
  MutableHandle<JSObject> propObj{runtime};
  MutableHandle<> value{runtime};
  MutableHandle<> entry{runtime};

  uint32_t targetIdx = 0;

  // Add the requested elements to properties.
  // We must keep track of the targetIdx because elements' enumerability may be
  // modified by a getter at any point in the loop, so `i` will not necessarily
  // correspond to `targetIdx`.
  auto marker = gcScope.createMarker();
  for (uint32_t i = 0, len = JSArray::getLength(*names); i < len; ++i) {
    gcScope.flushToMarker(marker);

    name = names->at(runtime, i).getString();
    ComputedPropertyDescriptor desc;
    if (LLVM_UNLIKELY(
            JSObject::getComputedDescriptor(
                objHandle, runtime, name, propObj, desc) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!propObj || !desc.flags.enumerable) {
      // Ensure that the property is still there and that it is enumerable,
      // as descriptors can be modified by a getter at any point.
      continue;
    }

    auto valueRes =
        JSObject::getComputedPropertyValue(objHandle, runtime, propObj, desc);
    if (LLVM_UNLIKELY(valueRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    value = *valueRes;

    if (kind == EnumerableOwnPropertiesKind::KeyValue) {
      auto entryRes = JSArray::create(runtime, 2, 2);
      if (LLVM_UNLIKELY(entryRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      entry = entryRes->getHermesValue();
      JSArray::setElementAt(Handle<JSArray>::vmcast(entry), runtime, 0, name);
      JSArray::setElementAt(Handle<JSArray>::vmcast(entry), runtime, 1, value);
    } else {
      assert(
          kind == EnumerableOwnPropertiesKind::Value &&
          "Name kind should have returned early");
      entry = value.getHermesValue();
    }

    // The element must exist because we just read it.
    JSArray::setElementAt(properties, runtime, targetIdx++, entry);
  }

  // Set length at the end only, because properties may be shorter than
  // names.size() - some properties may have been made non-enumerable by getters
  // in the loop.
  if (LLVM_UNLIKELY(
          JSArray::setLengthProperty(properties, runtime, targetIdx) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return properties.getHermesValue();
}

CallResult<HermesValue> objectKeys(void *, Runtime *runtime, NativeArgs args) {
  return enumerableOwnProperties(
      runtime, args, EnumerableOwnPropertiesKind::Key);
}

CallResult<HermesValue>
objectValues(void *, Runtime *runtime, NativeArgs args) {
  return enumerableOwnProperties(
      runtime, args, EnumerableOwnPropertiesKind::Value);
}

CallResult<HermesValue>
objectEntries(void *, Runtime *runtime, NativeArgs args) {
  return enumerableOwnProperties(
      runtime, args, EnumerableOwnPropertiesKind::KeyValue);
}

CallResult<HermesValue>
objectAssign(void *, Runtime *runtime, NativeArgs args) {
  vm::GCScope gcScope(runtime);

  // 1. Let to be ToObject(target).
  auto objRes = toObject(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    // 2. ReturnIfAbrupt(to).
    return ExecutionStatus::EXCEPTION;
  }
  auto toHandle = runtime->makeHandle<JSObject>(objRes.getValue());

  // 3. If only one argument was passed, return to.
  if (LLVM_UNLIKELY(args.getArgCount() == 1)) {
    return toHandle.getHermesValue();
  }

  // 4. Let sources be the List of argument values starting with the second
  // argument.
  // 5. For each element nextSource of sources, in ascending index order,

  // Handle for the current object being copied from.
  MutableHandle<JSObject> fromHandle{runtime};
  // Handle for the next key to be processed when copying properties.
  MutableHandle<> nextKeyHandle{runtime};
  // Handle for the property value being copied.
  MutableHandle<> propValueHandle{runtime};

  for (uint32_t argIdx = 1; argIdx < args.getArgCount(); argIdx++) {
    GCScopeMarkerRAII markerOuter(gcScope);
    auto nextSource = args.getArgHandle(argIdx);
    // 5.a. If nextSource is undefined or null, let keys be an empty List.
    if (nextSource->isNull() || nextSource->isUndefined()) {
      continue;
    }

    // 5.b.i. Let from be ToObject(nextSource).
    if (LLVM_UNLIKELY(
            (objRes = toObject(runtime, nextSource)) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    fromHandle = vmcast<JSObject>(objRes.getValue());

    // 5.b.ii. Let keys be from.[[OwnPropertyKeys]]().
    auto cr = JSObject::getOwnPropertyNames(fromHandle, runtime, true);
    if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
      // 5.c.ii. ReturnIfAbrupt(keys).
      return ExecutionStatus::EXCEPTION;
    }

    auto keys = *cr;
    ComputedPropertyDescriptor desc;
    // 5.c. Repeat for each element nextKey of keys in List order,
    for (uint32_t nextKeyIdx = 0, endIdx = keys->getEndIndex();
         nextKeyIdx < endIdx;
         ++nextKeyIdx) {
      GCScopeMarkerRAII markerInner(gcScope);

      nextKeyHandle = keys->at(runtime, nextKeyIdx);

      // 5.c.i. Let desc be from.[[GetOwnProperty]](nextKey).
      auto descCr = JSObject::getOwnComputedDescriptor(
          fromHandle, runtime, nextKeyHandle, desc);
      if (LLVM_UNLIKELY(descCr == ExecutionStatus::EXCEPTION)) {
        // 5.c.ii. ReturnIfAbrupt(desc).
        return ExecutionStatus::EXCEPTION;
      }
      // 5.c.iii. if desc is not undefined and desc.[[Enumerable]] is true, then
      if (LLVM_UNLIKELY(!*descCr) || LLVM_UNLIKELY(!desc.flags.enumerable)) {
        continue;
      }

      // 5.c.iii.1. Let propValue be Get(from, nextKey).
      auto propRes = JSObject::getComputedPropertyValue(
          fromHandle, runtime, fromHandle, desc);
      if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
        // 5.c.iii.2. ReturnIfAbrupt(propValue).
        return ExecutionStatus::EXCEPTION;
      }
      propValueHandle = propRes.getValue();

      // 5.c.iii.3. Let status be Set(to, nextKey, propValue, true).
      auto statusCr = JSObject::putComputed_RJS(
          toHandle,
          runtime,
          nextKeyHandle,
          propValueHandle,
          PropOpFlags().plusThrowOnError());
      if (LLVM_UNLIKELY(statusCr == ExecutionStatus::EXCEPTION)) {
        // 5.c.ii.4. ReturnIfAbrupt(status).
        return ExecutionStatus::EXCEPTION;
      }
    }
  }

  // 6 Return to.
  return toHandle.getHermesValue();
}

CallResult<HermesValue>
objectSetPrototypeOf(void *, Runtime *runtime, NativeArgs args) {
  Handle<> O = args.getArgHandle(0);
  Handle<> proto = args.getArgHandle(1);
  // 1. Let O be RequireObjectCoercible(O).
  if (O->isNull() || O->isUndefined()) {
    return runtime->raiseTypeError(
        "setPrototypeOf argument is not coercible to Object");
  }

  // 3. If Type(proto) is neither Object nor Null, throw a TypeError exception.
  if (!(proto->isObject() || proto->isNull())) {
    return runtime->raiseTypeError(
        "setPrototypeOf new prototype must be object or null");
  }
  // 4. If Type(O) is not Object, return O.
  if (!vmisa<JSObject>(*O)) {
    return *O;
  }
  // 5. Let status be O.[[SetPrototypeOf]](proto).
  auto status = JSObject::setParent(
      vmcast<JSObject>(*O), runtime, dyn_vmcast<JSObject>(*proto));
  // 7. If status is false, throw a TypeError exception.
  // Note that JSObject::setParent throws instead of returning false.
  if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // 8. Return O.
  return *O;
}

//===----------------------------------------------------------------------===//
/// Object.prototype.

CallResult<HermesValue> directObjectPrototypeToString(
    Runtime *runtime,
    Handle<> arg) {
  StringPrimitive *str;

  if (arg->isUndefined()) {
    str = runtime->getPredefinedString(Predefined::squareObjectUndefined);
  } else if (arg->isNull()) {
    str = runtime->getPredefinedString(Predefined::squareObjectNull);
  } else if (arg->getRaw() == runtime->getGlobal().getHermesValue().getRaw()) {
    str = runtime->getPredefinedString(Predefined::squareObjectGlobal);
  } else {
    auto res = toObject(runtime, arg);
    if (res == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }

    auto O = runtime->makeHandle<JSObject>(res.getValue());
    // 16. Let tag be Get (O, @@toStringTag).
    auto tagRes = JSObject::getNamed_RJS(
        O, runtime, Predefined::getSymbolID(Predefined::SymbolToStringTag));
    if (LLVM_UNLIKELY(tagRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    if (tagRes->isString()) {
      auto tag = runtime->makeHandle<StringPrimitive>(*tagRes);
      SafeUInt32 tagLen(tag->getStringLength());
      tagLen.add(9);
      CallResult<StringBuilder> builder =
          StringBuilder::createStringBuilder(runtime, tagLen);
      // 19. Return the String that is the result of concatenating
      // "[object ", tag, and "]".
      builder->appendASCIIRef(ASCIIRef{"[object ", 8});
      builder->appendStringPrim(tag);
      builder->appendCharacter(']');
      return builder->getStringPrimitive().getHermesValue();
    }

    // 18. If Type(tag) is not String, let tag be builtinTag.
    if (vmisa<JSArray>(O.getHermesValue())) {
      // 6. If isArray is true, let builtinTag be "Array".
      str = runtime->getPredefinedString(Predefined::squareObject_Array);
    } else if (vmisa<JSString>(O.getHermesValue())) {
      // 7. Else, if O is an exotic String object, let builtinTag be "String".
      str = runtime->getPredefinedString(Predefined::squareObject_StringObject);
    } else if (vmisa<Arguments>(O.getHermesValue())) {
      // 8. Else, if O has an [[ParameterMap]] internal slot, let builtinTag be
      // "Arguments".
      str = runtime->getPredefinedString(Predefined::squareObject_Arguments);
    } else if (vmisa<Callable>(O.getHermesValue())) {
      // 9. Else, if O has a [[Call]] internal method, let builtinTag be
      // "Function".
      str = runtime->getPredefinedString(Predefined::squareObject_Function);
    } else if (vmisa<JSError>(O.getHermesValue())) {
      // 10. Else, if O has an [[ErrorData]] internal slot, let builtinTag be
      // "Error".
      str = runtime->getPredefinedString(Predefined::squareObject_Error);
    } else if (vmisa<JSBoolean>(O.getHermesValue())) {
      // 11. Else, if O has a [[BooleanData]] internal slot, let builtinTag be
      // "Boolean".
      str =
          runtime->getPredefinedString(Predefined::squareObject_BooleanObject);
    } else if (vmisa<JSNumber>(O.getHermesValue())) {
      // 12. Else, if O has a [[NumberData]] internal slot, let builtinTag be
      // "Number".
      str = runtime->getPredefinedString(Predefined::squareObject_NumberObject);
    } else if (vmisa<JSDate>(O.getHermesValue())) {
      // 13. Else, if O has a [[DateValue]] internal slot, let builtinTag be
      // "Date".
      str = runtime->getPredefinedString(Predefined::squareObject_Date);
    } else if (vmisa<JSRegExp>(O.getHermesValue())) {
      // 14. Else, if O has a [[RegExpMatcher]] internal slot, let builtinTag be
      // "RegExp".
      str = runtime->getPredefinedString(Predefined::squareObject_RegExp);
    } else {
      str = runtime->getPredefinedString(Predefined::squareObject_Object);
    }
  }

  return HermesValue::encodeStringValue(str);
}

CallResult<HermesValue>
objectPrototypeToString(void *, Runtime *runtime, NativeArgs args) {
  return directObjectPrototypeToString(runtime, args.getThisHandle());
}

CallResult<HermesValue>
objectPrototypeToLocaleString(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto selfHandle = runtime->makeHandle<JSObject>(objRes.getValue());
  auto propRes = JSObject::getNamed_RJS(
      selfHandle, runtime, Predefined::getSymbolID(Predefined::toString));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (auto func = Handle<Callable>::dyn_vmcast(runtime->makeHandle(*propRes))) {
    return Callable::executeCall0(func, runtime, selfHandle);
  }
  return runtime->raiseTypeError("toString must be callable");
}

CallResult<HermesValue>
objectPrototypeValueOf(void *, Runtime *runtime, NativeArgs args) {
  auto res = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return res;
}

CallResult<HermesValue>
objectPrototypeHasOwnProperty(void *, Runtime *runtime, NativeArgs args) {
  auto res = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto objHandle = runtime->makeHandle<JSObject>(res.getValue());
  ComputedPropertyDescriptor desc;
  auto status = JSObject::getOwnComputedDescriptor(
      objHandle, runtime, args.getArgHandle(0), desc);
  if (status == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  if (*status) {
    return HermesValue::encodeBoolValue(true);
  }
  // For compatibility with polyfills we want to pretend that all HostObject
  // properties are "own" properties in hasOwnProperty() and in
  // getOwnPropertyDescriptor(). Since there is no way to check for a
  // HostObject property, we must always assume success. In practice the
  // property name would have been obtained from enumerating the properties in
  // JS code that looks something like this:
  //    for(key in hostObj) {
  //      if (Object.hasOwnProperty(hostObj, key))
  //        ...
  //    }
  if (LLVM_UNLIKELY(objHandle->isHostObject())) {
    return HermesValue::encodeBoolValue(true);
  }
  return HermesValue::encodeBoolValue(false);
}

CallResult<HermesValue>
objectPrototypeIsPrototypeOf(void *, Runtime *runtime, NativeArgs args) {
  if (LLVM_UNLIKELY(!args.getArg(0).isObject())) {
    // If arg[0] is not an object, return false.
    return HermesValue::encodeBoolValue(false);
  }
  auto res = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto *obj = vmcast<JSObject>(res.getValue());
  auto *parent = vmcast<JSObject>(args.getArg(0));
  while ((parent = parent->getParent(runtime))) {
    if (parent == obj) {
      return HermesValue::encodeBoolValue(true);
    }
  }
  return HermesValue::encodeBoolValue(false);
}

CallResult<HermesValue>
objectPrototypePropertyIsEnumerable(void *, Runtime *runtime, NativeArgs args) {
  auto res = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  ComputedPropertyDescriptor desc;
  auto status = JSObject::getOwnComputedDescriptor(
      runtime->makeHandle<JSObject>(res.getValue()),
      runtime,
      args.getArgHandle(0),
      desc);
  if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return HermesValue::encodeBoolValue(
      status.getValue() && desc.flags.enumerable);
}

CallResult<HermesValue>
objectPrototypeProto_getter(void *, Runtime *runtime, NativeArgs args) {
  // thisArg = toObject(thisArg).
  auto res = toObject(runtime, args.getThisHandle());
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  // Note that we must return 'null' if there is no prototype.
  JSObject *parent = vmcast<JSObject>(res.getValue())->getParent(runtime);
  return parent ? HermesValue::encodeObjectValue(parent)
                : HermesValue::encodeNullValue();
}

CallResult<HermesValue>
objectPrototypeProto_setter(void *, Runtime *runtime, NativeArgs args) {
  // thisArg must be coercible to Object.
  if (args.getThisArg().isNull() || args.getThisArg().isUndefined()) {
    return runtime->raiseTypeError("'this' is not coercible to JSObject");
  }
  // But if it isn't an actual object, do nothing.
  if (!args.getThisArg().isObject()) {
    return HermesValue::encodeUndefinedValue();
  }

  HermesValue proto = args.getArg(0);
  JSObject *protoPtr;
  if (proto.isObject())
    protoPtr = vmcast<JSObject>(proto);
  else if (proto.isNull())
    protoPtr = nullptr;
  else
    return HermesValue::encodeUndefinedValue();

  if (LLVM_UNLIKELY(
          JSObject::setParent(
              vmcast<JSObject>(args.getThisArg()), runtime, protoPtr) ==
          ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
objectPrototypeDefineGetter(void *, Runtime *runtime, NativeArgs args) {
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime->makeHandle<JSObject>(objRes.getValue());

  auto getter = args.dyncastArg<Callable>(1);
  if (!getter) {
    return runtime->raiseTypeError("__defineGetter__ getter not callable");
  }

  auto crtRes = PropertyAccessor::create(
      runtime, getter, Runtime::makeNullHandle<Callable>());
  if (crtRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto accessor = runtime->makeHandle<PropertyAccessor>(*crtRes);

  DefinePropertyFlags dpf;
  dpf.setEnumerable = 1;
  dpf.enumerable = 1;
  dpf.setConfigurable = 1;
  dpf.configurable = 1;
  dpf.setGetter = 1;

  auto res = JSObject::defineOwnComputed(
      O,
      runtime,
      args.getArgHandle(0),
      dpf,
      accessor,
      PropOpFlags().plusThrowOnError());
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
objectPrototypeDefineSetter(void *, Runtime *runtime, NativeArgs args) {
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime->makeHandle<JSObject>(objRes.getValue());

  auto setter = args.dyncastArg<Callable>(1);
  if (!setter) {
    return runtime->raiseTypeError("__defineSetter__ setter not callable");
  }

  auto crtRes = PropertyAccessor::create(
      runtime, Runtime::makeNullHandle<Callable>(), setter);
  if (crtRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto accessor = runtime->makeHandle<PropertyAccessor>(*crtRes);

  DefinePropertyFlags dpf;
  dpf.setEnumerable = 1;
  dpf.enumerable = 1;
  dpf.setConfigurable = 1;
  dpf.configurable = 1;
  dpf.setSetter = 1;

  auto res = JSObject::defineOwnComputed(
      O,
      runtime,
      args.getArgHandle(0),
      dpf,
      accessor,
      PropOpFlags().plusThrowOnError());
  if (res == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
objectPrototypeLookupGetter(void *, Runtime *runtime, NativeArgs args) {
  auto res = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime->makeHandle<JSObject>(res.getValue());

  auto key = args.getArgHandle(0);

  ComputedPropertyDescriptor desc;
  MutableHandle<JSObject> propObj{runtime};
  if (JSObject::getComputedDescriptor(O, runtime, key, propObj, desc) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  // The spec loops through prototypes in this function.
  // Do that internally in getComputedSlotValue.
  if (propObj && desc.flags.accessor) {
    auto *accessor = vmcast<PropertyAccessor>(
        JSObject::getComputedSlotValue(propObj.get(), runtime, desc));
    if (accessor->getter) {
      return HermesValue::encodeObjectValue(accessor->getter.get(runtime));
    }
  }
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue>
objectPrototypeLookupSetter(void *, Runtime *runtime, NativeArgs args) {
  auto res = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto O = runtime->makeHandle<JSObject>(res.getValue());

  auto key = args.getArgHandle(0);

  ComputedPropertyDescriptor desc;
  MutableHandle<JSObject> propObj{runtime};
  if (JSObject::getComputedDescriptor(O, runtime, key, propObj, desc) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  // The spec loops through prototypes in this function.
  // Do that internally in getComputedSlotValue.
  if (propObj && desc.flags.accessor) {
    auto *accessor = vmcast<PropertyAccessor>(
        JSObject::getComputedSlotValue(propObj.get(), runtime, desc));
    if (accessor->setter) {
      return HermesValue::encodeObjectValue(accessor->setter.get(runtime));
    }
  }
  return HermesValue::encodeUndefinedValue();
}

} // namespace vm
} // namespace hermes
