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

Handle<NativeConstructor> createTextEncoderConstructor(Runtime &runtime) {
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

  defineMethod(
      runtime,
      textEncoderPrototype,
      Predefined::getSymbolID(Predefined::encodeInto),
      nullptr,
      textEncoderPrototypeEncodeInto,
      2);

  auto cons = defineSystemConstructor(
      runtime,
      Predefined::getSymbolID(Predefined::TextEncoder),
      textEncoderConstructor,
      textEncoderPrototype,
      0);

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

  struct : public Locals {
    PinnedValue<JSObject> selfParent;
    PinnedValue<JSObject> self;
  } lv;
  LocalsRAII lraii(runtime, &lv);
  if (LLVM_LIKELY(
          args.getNewTarget().getRaw() ==
          runtime.textEncoderConstructor.getHermesValue().getRaw())) {
    lv.self = JSObject::create(runtime, runtime.textEncoderPrototype);
  } else {
    CallResult<PseudoHandle<JSObject>> thisParentRes =
        NativeConstructor::parentForNewThis_RJS(
            runtime,
            Handle<Callable>::vmcast(&args.getNewTarget()),
            runtime.textEncoderPrototype);
    if (LLVM_UNLIKELY(thisParentRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    lv.selfParent = std::move(*thisParentRes);
    lv.self = JSObject::create(runtime, lv.selfParent);
  }

  auto valueHandle = Runtime::getUndefinedValue();
  if (LLVM_UNLIKELY(
          JSObject::defineNewOwnProperty(
              lv.self,
              runtime,
              Predefined::getSymbolID(
                  Predefined::InternalPropertyTextEncoderType),
              PropertyFlags::defaultNewNamedPropertyFlags(),
              valueHandle) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return lv.self.getHermesValue();
}

CallResult<HermesValue>
textEncoderPrototypeEncoding(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  auto selfHandle = args.dyncastThis<JSObject>();
  if (!selfHandle) {
    return runtime.raiseTypeError(
        "TextEncoder.prototype.encoding called on non-TextEncoder object");
  }

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
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError(
        "TextEncoder.prototype.encode() called on non-TextEncoder object");
  }
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

CallResult<HermesValue>
textEncoderPrototypeEncodeInto(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};
  auto selfHandle = args.dyncastThis<JSObject>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError(
        "TextEncoder.prototype.encodeInto() called on non-TextEncoder object");
  }
  NamedPropertyDescriptor desc;
  bool exists = JSObject::getOwnNamedDescriptor(
      selfHandle,
      runtime,
      Predefined::getSymbolID(Predefined::InternalPropertyTextEncoderType),
      desc);
  if (LLVM_UNLIKELY(!exists)) {
    return runtime.raiseTypeError(
        "TextEncoder.prototype.encodeInto() called on non-TextEncoder object");
  }

  auto strRes = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<StringPrimitive> string = runtime.makeHandle(std::move(*strRes));

  Handle<Uint8Array> typedArray = args.dyncastArg<Uint8Array>(1);
  if (LLVM_UNLIKELY(!typedArray)) {
    return runtime.raiseTypeError("The second argument should be a Uint8Array");
  }

  if (LLVM_UNLIKELY(!typedArray->attached(runtime))) {
    return runtime.raiseTypeError(
        "TextEncoder.prototype.encodeInto() called on a detached Uint8Array");
  }

  PseudoHandle<JSObject> objRes = JSObject::create(runtime, 2);
  Handle<JSObject> obj = runtime.makeHandle(objRes.get());

  uint32_t numRead = 0;
  uint32_t numWritten = 0;

  if (LLVM_UNLIKELY(string->getStringLength() == 0)) {
    numRead = 0;
    numWritten = 0;
  } else if (string->isASCII()) {
    // ASCII string can trivially be converted to UTF-8 because ASCII is a
    // strict subset. However, since the output array size is provided by the
    // caller, we will only copy as much length as provided.
    llvh::ArrayRef<char> strRef = string->getStringRef<char>();

    uint32_t copiedLength =
        std::min(string->getStringLength(), typedArray->getLength());

    std::memcpy(typedArray->begin(runtime), strRef.data(), copiedLength);

    numRead = copiedLength;
    numWritten = copiedLength;
  } else {
    // Convert UTF-16 to the given Uint8Array
    llvh::ArrayRef<char16_t> strRef = string->getStringRef<char16_t>();
    std::pair<uint32_t, uint32_t> result =
        convertUTF16ToUTF8BufferWithReplacements(
            llvh::makeMutableArrayRef<uint8_t>(
                typedArray->begin(runtime), typedArray->getLength()),
            strRef);
    numRead = result.first;
    numWritten = result.second;
  }

  // Construct the result JSObject containing information about how much data
  // was converted
  auto numReadHandle =
      runtime.makeHandle(HermesValue::encodeTrustedNumberValue(numRead));
  auto numWrittenHandle =
      runtime.makeHandle(HermesValue::encodeTrustedNumberValue(numWritten));

  auto res = JSObject::defineNewOwnProperty(
      obj,
      runtime,
      Predefined::getSymbolID(Predefined::read),
      PropertyFlags::defaultNewNamedPropertyFlags(),
      numReadHandle);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  res = JSObject::defineNewOwnProperty(
      obj,
      runtime,
      Predefined::getSymbolID(Predefined::written),
      PropertyFlags::defaultNewNamedPropertyFlags(),
      numWrittenHandle);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return obj.getHermesValue();
}

} // namespace vm
} // namespace hermes
