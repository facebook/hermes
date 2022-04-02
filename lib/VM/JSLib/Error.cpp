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

#include "hermes/VM/Operations.h"
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
  // Error.prototype has two own properties: name and message.
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
#define NATIVE_ERROR_TYPE(error_name)                                       \
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
        1,                                                                  \
        NativeConstructor::creatorFunction<JSError>,                        \
        CellKind::JSErrorKind);                                             \
  }
#include "hermes/VM/NativeErrorTypes.def"

static CallResult<HermesValue> constructErrorObject(
    Runtime &runtime,
    NativeArgs args,
    Handle<JSObject> prototype) {
  MutableHandle<JSError> selfHandle{runtime};

  // If constructor, use the allocated object, otherwise allocate a new one.
  // Everything else is the same after that.
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

  // Initialize stack accessor.
  if (LLVM_UNLIKELY(
          JSError::setupStack(selfHandle, runtime) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // new Error(message).
  // Only proceed when 'typeof message' isn't undefined.
  if (!args.getArg(0).isUndefined()) {
    if (LLVM_UNLIKELY(
            JSError::setMessage(
                selfHandle, runtime, runtime.makeHandle(args.getArg(0))) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
  }

  // https://tc39.es/proposal-error-cause/
  // InstallErrorCause(O, options).
  // If Type(options) is Object and ? HasProperty(options, "cause") is true
  if (Handle<JSObject> options = args.dyncastArg<JSObject>(1)) {
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

// Constructor functions have to be expanded from macro because they are
// native calls, and their interface are restricted. No extra parameters
// can be passed in.
#define ALL_ERROR_TYPE(name)                                                   \
  CallResult<HermesValue> name##Constructor(                                   \
      void *, Runtime &runtime, NativeArgs args) {                             \
    return constructErrorObject(                                               \
        runtime, args, runtime.makeHandle<JSObject>(runtime.name##Prototype)); \
  }
#include "hermes/VM/NativeErrorTypes.def"

/// ES11.0 19.5.3.4
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

  // 3. Let name be ? Get(O, "name").
  auto propRes = JSObject::getNamed_RJS(
      O, runtime, Predefined::getSymbolID(Predefined::name), PropOpFlags());
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<> name = runtime.makeHandle(std::move(*propRes));

  // 4. If name is undefined, set name to "Error"; otherwise set name to ?
  // ToString(name).
  MutableHandle<StringPrimitive> nameStr{runtime};
  if (name->isUndefined()) {
    nameStr = runtime.getPredefinedString(Predefined::Error);
  } else {
    auto strRes = toString_RJS(runtime, name);
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    nameStr = strRes->get();
  }

  // 5. Let msg be ? Get(O, "message").
  if (LLVM_UNLIKELY(
          (propRes = JSObject::getNamed_RJS(
               O,
               runtime,
               Predefined::getSymbolID(Predefined::message),
               PropOpFlags())) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<> msg = runtime.makeHandle(std::move(*propRes));

  // 6. If msg is undefined, set msg to the empty String;
  //    otherwise set msg to ? ToString(msg).
  MutableHandle<StringPrimitive> msgStr{runtime};
  if (msg->isUndefined()) {
    // If msg is undefined, then let msg be the empty String.
    msgStr = runtime.getPredefinedString(Predefined::emptyString);
  } else {
    auto strRes = toString_RJS(runtime, msg);
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    msgStr = strRes->get();
  }

  // 7. If name is the empty String, return msg.
  if (nameStr->getStringLength() == 0) {
    return msgStr.getHermesValue();
  }

  // 8. If msg is the empty String, return name.
  if (msgStr->getStringLength() == 0) {
    return nameStr.getHermesValue();
  }

  // 9. Return the string-concatenation of name, the code unit 0x003A (COLON),
  // the code unit 0x0020 (SPACE), and msg.
  SafeUInt32 length{nameStr->getStringLength()};
  length.add(2);
  length.add(msgStr->getStringLength());
  CallResult<StringBuilder> builderRes =
      StringBuilder::createStringBuilder(runtime, length);
  if (LLVM_UNLIKELY(builderRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto builder = std::move(*builderRes);
  builder.appendStringPrim(nameStr);
  builder.appendASCIIRef({": ", 2});
  builder.appendStringPrim(msgStr);
  return builder.getStringPrimitive().getHermesValue();
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
  if (LLVM_UNLIKELY(
          JSError::setupStack(targetHandle, runtime) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return HermesValue::encodeUndefinedValue();
}

} // namespace vm
} // namespace hermes
