/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSLibInternal.h"

#include "llvh/Support/ConvertUTF.h"

#include "hermes/VM/JSTypedArray.h"

namespace hermes {
namespace vm {

Handle<JSObject> createTextEncoderConstructor(Runtime &runtime) {
  auto textEncoderPrototype =
      Handle<JSObject>::vmcast(&runtime.textEncoderPrototype);

  // Per https://webidl.spec.whatwg.org/#javascript-binding, @@toStringTag
  // should be writable=false, enumerable=false, and configurable=true.
  DefinePropertyFlags dpf = DefinePropertyFlags::getNewNonEnumerableFlags();
  dpf.writable = 0;
  defineProperty(
      runtime,
      textEncoderPrototype,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime.getPredefinedStringHandle(Predefined::TextEncoder),
      dpf);

  // Based on
  // Object.getOwnPropertyDescriptor(TextEncoder.prototype, 'encoding'), both
  // Chrome and Safari have the 'encoding' property as enumerable and
  // configurable. We set things up to be the same.
  defineAccessor(
      runtime,
      textEncoderPrototype,
      Predefined::getSymbolID(Predefined::encoding),
      nullptr,
      textEncoderPrototypeEncoding,
      nullptr,
      /* enumerable */ true,
      /* configurable */ true);

  defineMethod(
      runtime,
      textEncoderPrototype,
      Predefined::getSymbolID(Predefined::encode),
      nullptr,
      textEncoderPrototypeEncode,
      1);

  auto cons = defineSystemConstructor<JSObject>(
      runtime,
      Predefined::getSymbolID(Predefined::TextEncoder),
      textEncoderConstructor,
      textEncoderPrototype,
      0,
      CellKind::JSObjectKind);

  defineProperty(
      runtime,
      textEncoderPrototype,
      Predefined::getSymbolID(Predefined::constructor),
      cons);

  return cons;
}

CallResult<HermesValue>
textEncoderConstructor(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  if (LLVM_UNLIKELY(!args.isConstructorCall())) {
    return runtime.raiseTypeError(
        "TextEncoder must be called as a constructor");
  }

  auto selfHandle = args.vmcastThis<JSObject>();

  auto valueHandle = Runtime::getUndefinedValue();
  if (LLVM_UNLIKELY(
          JSObject::defineNewOwnProperty(
              selfHandle,
              runtime,
              Predefined::getSymbolID(
                  Predefined::InternalPropertyTextEncoderType),
              PropertyFlags::defaultNewNamedPropertyFlags(),
              valueHandle) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return selfHandle.getHermesValue();
}

CallResult<HermesValue>
textEncoderPrototypeEncoding(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  auto selfHandle = args.dyncastThis<JSObject>();

  NamedPropertyDescriptor desc;
  bool exists = JSObject::getOwnNamedDescriptor(
      selfHandle,
      runtime,
      Predefined::getSymbolID(Predefined::InternalPropertyTextEncoderType),
      desc);
  if (LLVM_UNLIKELY(!exists)) {
    return runtime.raiseTypeError(
        "TextEncoder.prototype.encoding called on non-TextEncoder object");
  }

  return HermesValue::encodeStringValue(
      runtime.getPredefinedString(Predefined::utf8));
}

CallResult<HermesValue>
textEncoderPrototypeEncode(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};
  auto selfHandle = args.dyncastThis<JSObject>();
  NamedPropertyDescriptor desc;
  bool exists = JSObject::getOwnNamedDescriptor(
      selfHandle,
      runtime,
      Predefined::getSymbolID(Predefined::InternalPropertyTextEncoderType),
      desc);
  if (LLVM_UNLIKELY(!exists)) {
    return runtime.raiseTypeError(
        "TextEncoder.prototype.encode() called on non-TextEncoder object");
  }

  auto strRes = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<StringPrimitive> string = runtime.makeHandle(std::move(*strRes));

  // If input string is empty, then the function can return early. This also
  // avoids having to check later before calling std::memcpy to avoid undefined
  // behavior.
  if (LLVM_UNLIKELY(string->getStringLength() == 0)) {
    auto result = Uint8Array::allocate(runtime);
    if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return result->getHermesValue();
  }

  if (string->isASCII()) {
    // ASCII string can trivially be converted to UTF-8 because ASCII is a
    // strict subset.
    auto result = Uint8Array::allocate(runtime, string->getStringLength());
    if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    Handle<JSTypedArrayBase> typedArray = result.getValue();
    llvh::ArrayRef<char> strRef = string->getStringRef<char>();

    std::memcpy(
        typedArray->begin(runtime), strRef.data(), string->getStringLength());
    return typedArray.getHermesValue();
  } else {
    // Convert UTF-16 to UTF-8
    llvh::ArrayRef<char16_t> strRef = string->getStringRef<char16_t>();
    std::string converted;
    bool success = convertUTF16ToUTF8WithReplacements(converted, strRef);
    if (LLVM_UNLIKELY(!success)) {
      return runtime.raiseError("Failed to convert from UTF-16 to UTF-8");
    }

    auto result = Uint8Array::allocate(runtime, converted.length());
    if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    Handle<JSTypedArrayBase> typedArray = result.getValue();
    std::memcpy(
        typedArray->begin(runtime), converted.data(), converted.length());
    return typedArray.getHermesValue();
  }
}

} // namespace vm
} // namespace hermes
