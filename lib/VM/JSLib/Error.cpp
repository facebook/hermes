/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// ES5.1 15.11 Initialize the Error constructor.
//===----------------------------------------------------------------------===//

#include "JSLibInternal.h"

#include "hermes/VM/CallResult.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/PropertyAccessor.h"
#include "hermes/VM/StringBuilder.h"
#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
/// ErrorObject.

Handle<JSObject> createErrorConstructor(Runtime &runtime) {
  auto errorPrototype = Handle<JSObject>::vmcast(&runtime.ErrorPrototype);

  // Error.prototype.xxx methods.
  defineMethod(
      runtime,
      errorPrototype,
      Predefined::getSymbolID(Predefined::toString),
      nullptr,
      errorPrototypeToString,
      0);

  // Error.prototype.xxx properties.
  // Error.prototype has three own properties: name, message, and stack.
  auto defaultName = runtime.getPredefinedString(Predefined::Error);
  defineProperty(
      runtime,
      errorPrototype,
      Predefined::getSymbolID(Predefined::name),
      runtime.makeHandle(HermesValue::encodeStringValue(defaultName)));

  auto defaultMessage = runtime.getPredefinedString(Predefined::emptyString);
  defineProperty(
      runtime,
      errorPrototype,
      Predefined::getSymbolID(Predefined::message),
      runtime.makeHandle(HermesValue::encodeStringValue(defaultMessage)));

  auto getter = NativeFunction::create(
      runtime,
      Handle<JSObject>::vmcast(&runtime.functionPrototype),
      nullptr,
      errorStackGetter,
      Predefined::getSymbolID(Predefined::emptyString),
      0,
      Runtime::makeNullHandle<JSObject>());

  auto setter = NativeFunction::create(
      runtime,
      Handle<JSObject>::vmcast(&runtime.functionPrototype),
      nullptr,
      errorStackSetter,
      Predefined::getSymbolID(Predefined::emptyString),
      1,
      Runtime::makeNullHandle<JSObject>());

  // Save the accessors on the runtime so we can use them for captureStackTrace.
  runtime.jsErrorStackAccessor =
      PropertyAccessor::create(runtime, getter, setter);

  auto accessor =
      Handle<PropertyAccessor>::vmcast(&runtime.jsErrorStackAccessor);

  DefinePropertyFlags dpf{};
  dpf.setEnumerable = 1;
  dpf.setConfigurable = 1;
  dpf.setGetter = 1;
  dpf.setSetter = 1;
  dpf.enumerable = 0;
  dpf.configurable = 1;

  auto stackRes = JSObject::defineOwnProperty(
      errorPrototype,
      runtime,
      Predefined::getSymbolID(Predefined::stack),
      dpf,
      accessor);
  (void)stackRes;

  assert(*stackRes && "Failed to define stack accessor");

  auto cons = defineSystemConstructor<JSError>(
      runtime,
      Predefined::getSymbolID(Predefined::Error),
      ErrorConstructor,
      errorPrototype,
      1,
      CellKind::JSErrorKind);

  defineMethod(
      runtime,
      cons,
      Predefined::getSymbolID(Predefined::captureStackTrace),
      nullptr,
      errorCaptureStackTrace,
      2);

  return cons;
}

// The constructor creation functions have to be expanded from macros because
// the constructor functions are expanded from macros.
#define ERR_HELPER(error_name, argCount)                                    \
  Handle<JSObject> create##error_name##Constructor(Runtime &runtime) {      \
    auto errorPrototype =                                                   \
        Handle<JSObject>::vmcast(&runtime.error_name##Prototype);           \
    auto defaultName = runtime.getPredefinedString(Predefined::error_name); \
    defineProperty(                                                         \
        runtime,                                                            \
        errorPrototype,                                                     \
        Predefined::getSymbolID(Predefined::name),                          \
        runtime.makeHandle(HermesValue::encodeStringValue(defaultName)));   \
    defineProperty(                                                         \
        runtime,                                                            \
        errorPrototype,                                                     \
        Predefined::getSymbolID(Predefined::message),                       \
        runtime.getPredefinedStringHandle(Predefined::emptyString));        \
    return defineSystemConstructor(                                         \
        runtime,                                                            \
        Predefined::getSymbolID(Predefined::error_name),                    \
        error_name##Constructor,                                            \
        errorPrototype,                                                     \
        Handle<JSObject>::vmcast(&runtime.errorConstructor),                \
        argCount,                                                           \
        NativeConstructor::creatorFunction<JSError>,                        \
        CellKind::JSErrorKind);                                             \
  }
// The AggregateError constructor takes in two parameters, while all the other
// Error types take in one.
#define NATIVE_ERROR_TYPE(name) ERR_HELPER(name, 1)
#define AGGREGATE_ERROR_TYPE(name) ERR_HELPER(name, 2)
#include "hermes/FrontEndDefs/NativeErrorTypes.def"

static CallResult<HermesValue> constructErrorObject(
    Runtime &runtime,
    NativeArgs args,
    Handle<> message,
    Handle<> opts,
    Handle<JSObject> prototype) {
  MutableHandle<JSError> selfHandle{runtime};
  // If constructor, use the allocated object, otherwise allocate a new one.
  if (args.isConstructorCall()) {
    selfHandle = vmcast<JSError>(args.getThisArg());
  } else {
    selfHandle = JSError::create(runtime, prototype).get();
  }
  // Record the stack trace, skipping this entry.
  if (LLVM_UNLIKELY(
          JSError::recordStackTrace(selfHandle, runtime, true) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // new Error(message).
  // Only proceed when 'typeof message' isn't undefined.
  if (!message->isUndefined()) {
    if (LLVM_UNLIKELY(
            JSError::setMessage(selfHandle, runtime, message) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
  }

  // https://tc39.es/proposal-error-cause/
  // InstallErrorCause(O, options).
  // If Type(options) is Object and ? HasProperty(options, "cause") is true
  if (auto options = Handle<JSObject>::dyn_vmcast(opts)) {
    GCScopeMarkerRAII marker{runtime};
    NamedPropertyDescriptor desc;
    Handle<JSObject> propObj =
        runtime.makeHandle(JSObject::getNamedDescriptorPredefined(
            options, runtime, Predefined::cause, desc));
    if (propObj) {
      // a. Let cause be ? Get(options, "cause").
      auto causeRes = JSObject::getNamedPropertyValue_RJS(
          selfHandle, runtime, std::move(propObj), desc);
      if (LLVM_UNLIKELY(causeRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      Handle<> cause = runtime.makeHandle(std::move(*causeRes));
      // b. Perform ! CreateNonEnumerableDataPropertyOrThrow(O, "cause", cause).
      if (LLVM_UNLIKELY(
              JSObject::defineOwnProperty(
                  selfHandle,
                  runtime,
                  Predefined::getSymbolID(Predefined::cause),
                  DefinePropertyFlags::getNewNonEnumerableFlags(),
                  cause,
                  PropOpFlags().plusThrowOnError()) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    }
  }

  return selfHandle.getHermesValue();
}

// ES2023 20.5.7.1.1
static CallResult<HermesValue> constructAggregateErrorObject(
    Runtime &runtime,
    NativeArgs args,
    Handle<JSObject> prototype) {
  // 1-4 handled in constructErrorObject
  CallResult<HermesValue> errorObj = constructErrorObject(
      runtime, args, args.getArgHandle(1), args.getArgHandle(2), prototype);
  if (LLVM_UNLIKELY(errorObj == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto errorObjHandle = runtime.makeHandle<JSObject>(*errorObj);

  // 5. Let errorsList be ? IterableToList(errors).
  CallResult<Handle<JSArray>> errorsList =
      iterableToArray(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(errorsList == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 6. Perform ! DefinePropertyOrThrow(O, "errors", PropertyDescriptor {
  // [[Configurable]]: true, [[Enumerable]]: false, [[Writable]]: true,
  // [[Value]]: CreateArrayFromList(errorsList) }).

  CallResult<bool> res = JSObject::defineOwnProperty(
      errorObjHandle,
      runtime,
      Predefined::getSymbolID(Predefined::errors),
      DefinePropertyFlags::getNewNonEnumerableFlags(),
      *errorsList);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 7. Return O.
  return errorObjHandle.getHermesValue();
}

// Note- the following names for the error and aggregate functions are spelled
// to exactly match the names in NativeErrorTypes.def. They are used in the
// ERR_HELPER macro defined above.
CallResult<HermesValue>
ErrorConstructor(void *, Runtime &runtime, NativeArgs args) {
  auto prototype = runtime.makeHandle<JSObject>(runtime.ErrorPrototype);
  return constructErrorObject(
      runtime, args, args.getArgHandle(0), args.getArgHandle(1), prototype);
}

// AggregateError has a different constructor body than the other errors.
CallResult<HermesValue>
AggregateErrorConstructor(void *, Runtime &runtime, NativeArgs args) {
  return constructAggregateErrorObject(
      runtime,
      args,
      runtime.makeHandle<JSObject>(runtime.AggregateErrorPrototype));
}

// Constructor functions have to be expanded from macro because they are
// native calls, and their interface are restricted. No extra parameters
// can be passed in. We can't use the #ALL_ERROR_TYPE macro since AggregateError
// requires a different constructor.
#define NATIVE_ERROR_TYPE(name)                                                \
  CallResult<HermesValue> name##Constructor(                                   \
      void *, Runtime &runtime, NativeArgs args) {                             \
    auto prototype = runtime.makeHandle<JSObject>(runtime.name##Prototype);    \
    return constructErrorObject(                                               \
        runtime, args, args.getArgHandle(0), args.getArgHandle(1), prototype); \
  }
#include "hermes/FrontEndDefs/NativeErrorTypes.def"

/// ES2023 20.5.3.4
CallResult<HermesValue>
errorPrototypeToString(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  // 1. Let O be the this value.
  auto O = args.dyncastThis<JSObject>();

  // 2. If Type(O) is not Object, throw a TypeError exception.
  if (LLVM_UNLIKELY(!O)) {
    return runtime.raiseTypeErrorForValue(
        "Error.prototype.toString called on incompatible receiver ",
        args.getThisHandle(),
        "");
  }

  // Steps 3. through 9 are implemented in JSError::toString.
  CallResult<Handle<StringPrimitive>> toStringRes =
      JSError::toString(O, runtime);
  if (LLVM_UNLIKELY(toStringRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return toStringRes->getHermesValue();
}

/// captureStackTrace(target, sentinelOpt?) { ... }
///
/// Adds a `stack` getter to the given `target` object that yields the current
/// stack trace. This is the exact same getter as the one available on Error
/// instances.
///
/// If the second argument to captureStackTrace is a function, when collecting
/// the stack trace all frames above the topmost call to the provided function,
/// including that call, are left out of the stack trace.
CallResult<HermesValue>
errorCaptureStackTrace(void *, Runtime &runtime, NativeArgs args) {
  // Get the target object.
  auto targetHandle = args.dyncastArg<JSObject>(0);
  if (!targetHandle || targetHandle->isHostObject() ||
      targetHandle->isProxyObject()) {
    return runtime.raiseTypeError("Invalid argument");
  }

  // Construct a temporary Error instance.
  auto errorPrototype = Handle<JSObject>::vmcast(&runtime.ErrorPrototype);
  auto errorHandle =
      runtime.makeHandle(JSError::create(runtime, errorPrototype));

  // Record the stack trace, skipping the entry for captureStackTrace itself.
  const bool skipTopFrame = true;
  JSError::recordStackTrace(errorHandle, runtime, skipTopFrame);

  // Skip frames until and including the sentinel function, if any.
  if (auto sentinel = args.dyncastArg<Callable>(1)) {
    JSError::popFramesUntilInclusive(runtime, errorHandle, sentinel);
  }

  // Initialize the target's [[CapturedError]] slot with the error instance.
  auto res = JSObject::defineOwnProperty(
      targetHandle,
      runtime,
      Predefined::getSymbolID(Predefined::InternalPropertyCapturedError),
      DefinePropertyFlags::getDefaultNewPropertyFlags(),
      errorHandle);

  // Even though highly unlikely, something could have happened that caused
  // defineOwnProperty to throw an exception.
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (LLVM_UNLIKELY(!*res)) {
    return runtime.raiseTypeError(
        TwineChar16("Cannot add new properties to object"));
  }

  // Initialize the `stack` accessor on the target object.
  DefinePropertyFlags dpf{};
  dpf.setEnumerable = 1;
  dpf.setConfigurable = 1;
  dpf.setGetter = 1;
  dpf.setSetter = 1;
  dpf.enumerable = 0;
  dpf.configurable = 1;

  auto stackRes = JSObject::defineOwnProperty(
      targetHandle,
      runtime,
      Predefined::getSymbolID(Predefined::stack),
      dpf,
      Handle<>(&runtime.jsErrorStackAccessor));

  // Ignore failures to set the "stack" property as other engines do.
  if (LLVM_UNLIKELY(stackRes == ExecutionStatus::EXCEPTION)) {
    runtime.clearThrownValue();
  }

  return HermesValue::encodeUndefinedValue();
}

} // namespace vm
} // namespace hermes
