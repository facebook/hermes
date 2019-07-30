/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
//===----------------------------------------------------------------------===//
/// \file
/// ES5.1 15.11 Initialize the Error constructor.
//===----------------------------------------------------------------------===//

#include "JSLibInternal.h"

#include "hermes/VM/Operations.h"
#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
/// ErrorObject.

Handle<JSObject> createErrorConstructor(Runtime *runtime) {
  auto errorPrototype = Handle<JSError>::vmcast(&runtime->ErrorPrototype);

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
  auto defaultName = runtime->getPredefinedString(Predefined::Error);
  defineProperty(
      runtime,
      errorPrototype,
      Predefined::getSymbolID(Predefined::name),
      runtime->makeHandle(HermesValue::encodeStringValue(defaultName)));

  auto defaultMessage = runtime->getPredefinedString(Predefined::emptyString);
  defineProperty(
      runtime,
      errorPrototype,
      Predefined::getSymbolID(Predefined::message),
      runtime->makeHandle(HermesValue::encodeStringValue(defaultMessage)));

  return defineSystemConstructor<JSError>(
      runtime,
      Predefined::getSymbolID(Predefined::Error),
      ErrorConstructor,
      errorPrototype,
      1,
      CellKind::ErrorKind);
}

// The constructor creation functions have to be expanded from macros because
// the constructor functions are expanded from macros.
#define NATIVE_ERROR_TYPE(error_name)                                        \
  Handle<JSObject> create##error_name##Constructor(Runtime *runtime) {       \
    auto errorPrototype =                                                    \
        Handle<JSObject>::vmcast(&runtime->error_name##Prototype);           \
    auto defaultName = runtime->getPredefinedString(Predefined::error_name); \
    defineProperty(                                                          \
        runtime,                                                             \
        errorPrototype,                                                      \
        Predefined::getSymbolID(Predefined::name),                           \
        runtime->makeHandle(HermesValue::encodeStringValue(defaultName)));   \
    defineProperty(                                                          \
        runtime,                                                             \
        errorPrototype,                                                      \
        Predefined::getSymbolID(Predefined::message),                        \
        runtime->getPredefinedStringHandle(Predefined::emptyString));        \
    return defineSystemConstructor(                                          \
        runtime,                                                             \
        Predefined::getSymbolID(Predefined::error_name),                     \
        error_name##Constructor,                                             \
        errorPrototype,                                                      \
        Handle<JSObject>::vmcast(&runtime->errorConstructor),                \
        1,                                                                   \
        JSError::create,                                                     \
        CellKind::ErrorKind);                                                \
  }
#include "hermes/VM/NativeErrorTypes.def"

static CallResult<HermesValue> constructErrorObject(
    Runtime *runtime,
    NativeArgs args,
    Handle<JSObject> prototype) {
  MutableHandle<JSError> selfHandle{runtime};

  // If constructor, use the allocated object, otherwise allocate a new one.
  // Everything else is the same after that.
  if (args.isConstructorCall()) {
    selfHandle = vmcast<JSError>(args.getThisArg());
  } else {
    auto errRes = JSError::create(runtime, prototype);
    if (LLVM_UNLIKELY(errRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    selfHandle = vmcast<JSError>(*errRes);
  }

  // Record the stack trace, skipping this entry.
  JSError::recordStackTrace(selfHandle, runtime, true);
  // Initialize stack accessor.
  JSError::setupStack(selfHandle, runtime);

  // new Error(message).
  if (args.getArgCount() >= 1) {
    if (LLVM_UNLIKELY(
            JSError::setMessage(
                selfHandle, runtime, runtime->makeHandle(args.getArg(0))) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
  }

  return selfHandle.getHermesValue();
}

// Constructor functions have to be expanded from macro because they are
// native calls, and their interface are restricted. No extra parameters
// can be passed in.
#define ALL_ERROR_TYPE(name)                                      \
  CallResult<HermesValue> name##Constructor(                      \
      void *, Runtime *runtime, NativeArgs args) {                \
    return constructErrorObject(                                  \
        runtime,                                                  \
        args,                                                     \
        runtime->makeHandle<JSObject>(runtime->name##Prototype)); \
  }
#include "hermes/VM/NativeErrorTypes.def"

CallResult<HermesValue>
errorPrototypeToString(void *, Runtime *runtime, NativeArgs args) {
  auto objRes = toObject(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto error = runtime->makeHandle<JSObject>(objRes.getValue());

  auto propRes = JSObject::getNamed_RJS(
      error, runtime, Predefined::getSymbolID(Predefined::name), PropOpFlags());
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<> name = runtime->makeHandle(*propRes);
  MutableHandle<StringPrimitive> nameStr{runtime};
  if (name->isUndefined()) {
    // If name is undefined, then let name be "Error"
    nameStr = runtime->getPredefinedString(Predefined::Error);
  } else {
    auto strRes = toString_RJS(runtime, name);
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    nameStr = strRes->get();
  }

  if (LLVM_UNLIKELY(
          (propRes = JSObject::getNamed_RJS(
               error,
               runtime,
               Predefined::getSymbolID(Predefined::message),
               PropOpFlags())) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<> message = runtime->makeHandle(*propRes);
  MutableHandle<StringPrimitive> messageStr{runtime};
  if (message->isUndefined()) {
    // If msg is undefined, then let msg be the empty String.
    messageStr = runtime->getPredefinedString(Predefined::emptyString);
  } else {
    auto strRes = toString_RJS(runtime, message);
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    messageStr = strRes->get();
  }

  if (nameStr->getStringLength() == 0) {
    // According to ES5.1, if both name and message are the empty string,
    // we should return "Error". However all modern VMs are now using the
    // ES6 semantics, and this condition was dropped in ES6.
    return messageStr.getHermesValue();
  }
  if (messageStr->getStringLength() == 0) {
    return nameStr.getHermesValue();
  } else {
    auto separator = runtime->makeHandle<StringPrimitive>(
        runtime->getPredefinedString(Predefined::errorSeparator));

    auto strRes = StringPrimitive::concat(runtime, nameStr, separator);
    if (strRes == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    if ((strRes = StringPrimitive::concat(
             runtime,
             runtime->makeHandle<StringPrimitive>(*strRes),
             messageStr)) == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    return strRes;
  }
}

} // namespace vm
} // namespace hermes
