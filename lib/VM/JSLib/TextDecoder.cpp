/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSLibInternal.h"
#include "TextDecoderUtils.h"

#include "hermes/Platform/Unicode/CharacterProperties.h"
#include "hermes/Support/UTF8.h"
#include "llvh/Support/ConvertUTF.h"

#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSArrayBuffer.h"
#include "hermes/VM/StringRefUtils.h"
#include "hermes/VM/JSDataView.h"
#include "hermes/VM/JSTypedArray.h"
#include "hermes/VM/StringBuilder.h"

namespace hermes {
namespace vm {

static bool isTextDecoderObject(
    Handle<JSObject> obj,
    Runtime &runtime,
    TextDecoderEncoding *outEncoding = nullptr,
    bool *outFatal = nullptr,
    bool *outIgnoreBOM = nullptr) {
  NamedPropertyDescriptor desc;
  bool exists = JSObject::getOwnNamedDescriptor(
      obj,
      runtime,
      Predefined::getSymbolID(Predefined::InternalPropertyTextDecoderEncoding),
      desc);
  if (!exists) {
    return false;
  }

  if (outEncoding) {  // Get encoding
    HermesValue encodingVal =
        JSObject::getNamedSlotValueUnsafe(obj.get(), runtime, desc)
            .unboxToHV(runtime);
    if (!encodingVal.isNumber()) {
      return false;
    }
    *outEncoding =
        static_cast<TextDecoderEncoding>(static_cast<uint8_t>(
            encodingVal.getNumber()));
  }

  if (outFatal) {  // Get fatal flag
    exists = JSObject::getOwnNamedDescriptor(
        obj,
        runtime,
        Predefined::getSymbolID(Predefined::InternalPropertyTextDecoderFatal),
        desc);
    if (!exists) {
      return false;
    }
    HermesValue fatalVal =
        JSObject::getNamedSlotValueUnsafe(obj.get(), runtime, desc)
            .unboxToHV(runtime);
    *outFatal = fatalVal.getBool();
  }

  if (outIgnoreBOM) {  // Get ignoreBOM flag
    exists = JSObject::getOwnNamedDescriptor(
        obj,
        runtime,
        Predefined::getSymbolID(
            Predefined::InternalPropertyTextDecoderIgnoreBOM),
        desc);
    if (!exists) {
      return false;
    }
    HermesValue ignoreBOMVal =
        JSObject::getNamedSlotValueUnsafe(obj.get(), runtime, desc)
            .unboxToHV(runtime);
    *outIgnoreBOM = ignoreBOMVal.getBool();
  }

  return true;
}

Handle<JSObject> createTextDecoderConstructor(Runtime &runtime) {
  auto textDecoderPrototype =
      Handle<JSObject>::vmcast(&runtime.textDecoderPrototype);

  // Per https://webidl.spec.whatwg.org/#javascript-binding, @@toStringTag
  // should be writable=false, enumerable=false, and configurable=true.
  DefinePropertyFlags dpf = DefinePropertyFlags::getNewNonEnumerableFlags();
  dpf.writable = 0;
  defineProperty(
      runtime,
      textDecoderPrototype,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime.getPredefinedStringHandle(Predefined::TextDecoder),
      dpf);

  // Define the 'encoding' accessor property
  defineAccessor(
      runtime,
      textDecoderPrototype,
      Predefined::getSymbolID(Predefined::encoding),
      nullptr,
      textDecoderPrototypeEncoding,
      nullptr,
      /* enumerable */ true,
      /* configurable */ true);

  // Define the 'fatal' accessor property
  defineAccessor(
      runtime,
      textDecoderPrototype,
      Predefined::getSymbolID(Predefined::fatal),
      nullptr,
      textDecoderPrototypeFatal,
      nullptr,
      /* enumerable */ true,
      /* configurable */ true);

  // Define the 'ignoreBOM' accessor property
  defineAccessor(
      runtime,
      textDecoderPrototype,
      Predefined::getSymbolID(Predefined::ignoreBOM),
      nullptr,
      textDecoderPrototypeIgnoreBOM,
      nullptr,
      /* enumerable */ true,
      /* configurable */ true);

  // Define the 'decode' method
  defineMethod(
      runtime,
      textDecoderPrototype,
      Predefined::getSymbolID(Predefined::decode),
      nullptr,
      textDecoderPrototypeDecode,
      1);

  auto cons = defineSystemConstructor<JSObject>(
      runtime,
      Predefined::getSymbolID(Predefined::TextDecoder),
      textDecoderConstructor,
      textDecoderPrototype,
      0,
      CellKind::JSObjectKind);

  defineProperty(
      runtime,
      textDecoderPrototype,
      Predefined::getSymbolID(Predefined::constructor),
      cons);

  return cons;
}

CallResult<HermesValue>
textDecoderConstructor(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  if (LLVM_UNLIKELY(!args.isConstructorCall())) {
    return runtime.raiseTypeError(
        "TextDecoder must be called as a constructor");
  }

  auto selfHandle = args.vmcastThis<JSObject>();

  // Parse the encoding label (default is "utf-8")
  TextDecoderEncoding encoding = TextDecoderEncoding::UTF8;
  if (args.getArgCount() > 0 && !args.getArg(0).isUndefined()) {
    auto labelRes = toString_RJS(runtime, args.getArgHandle(0));
    if (LLVM_UNLIKELY(labelRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    Handle<StringPrimitive> label = runtime.makeHandle(std::move(*labelRes));
    auto labelView = StringPrimitive::createStringView(runtime, label);

    auto parsedEncoding = parseEncodingLabel(labelView);
    if (!parsedEncoding) {
      return runtime.raiseRangeError(
          TwineChar16("Unknown encoding: ") +
          StringPrimitive::createStringView(runtime, label));
    }
    encoding = *parsedEncoding;
  }

  // Parse options (second argument)
  bool fatal = false;
  bool ignoreBOM = false;
  if (args.getArgCount() > 1 && !args.getArg(1).isUndefined()) {
    auto optionsHandle = args.getArgHandle(1);
    if (!optionsHandle->isObject()) {
      return runtime.raiseTypeError("Options must be an object");
    }

    auto optionsObj = Handle<JSObject>::vmcast(optionsHandle);

    // Get 'fatal' option
    auto fatalRes = JSObject::getNamed_RJS(
        optionsObj, runtime, Predefined::getSymbolID(Predefined::fatal));
    if (LLVM_UNLIKELY(fatalRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!(*fatalRes)->isUndefined()) {
      fatal = toBoolean(fatalRes->get());
    }

    // Get 'ignoreBOM' option
    auto ignoreBOMRes = JSObject::getNamed_RJS(
        optionsObj, runtime, Predefined::getSymbolID(Predefined::ignoreBOM));
    if (LLVM_UNLIKELY(ignoreBOMRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (!(*ignoreBOMRes)->isUndefined()) {
      ignoreBOM = toBoolean(ignoreBOMRes->get());
    }
  }

  // Store the encoding as an internal property
  auto encodingHandle = runtime.makeHandle(
      HermesValue::encodeTrustedNumberValue(static_cast<uint8_t>(encoding)));
  if (LLVM_UNLIKELY(
          JSObject::defineNewOwnProperty(
              selfHandle,
              runtime,
              Predefined::getSymbolID(
                  Predefined::InternalPropertyTextDecoderEncoding),
              PropertyFlags::defaultNewNamedPropertyFlags(),
              encodingHandle) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // Store the fatal flag as an internal property
  auto fatalHandle =
      runtime.makeHandle(HermesValue::encodeBoolValue(fatal));
  if (LLVM_UNLIKELY(
          JSObject::defineNewOwnProperty(
              selfHandle,
              runtime,
              Predefined::getSymbolID(
                  Predefined::InternalPropertyTextDecoderFatal),
              PropertyFlags::defaultNewNamedPropertyFlags(),
              fatalHandle) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // Store the ignoreBOM flag as an internal property
  auto ignoreBOMHandle =
      runtime.makeHandle(HermesValue::encodeBoolValue(ignoreBOM));
  if (LLVM_UNLIKELY(
          JSObject::defineNewOwnProperty(
              selfHandle,
              runtime,
              Predefined::getSymbolID(
                  Predefined::InternalPropertyTextDecoderIgnoreBOM),
              PropertyFlags::defaultNewNamedPropertyFlags(),
              ignoreBOMHandle) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // Initialize streaming state: pending bytes (empty array) and BOM seen flag
  auto pendingArrRes = JSArray::create(runtime, 0, 0);
  if (LLVM_UNLIKELY(pendingArrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto pendingHandle = runtime.makeHandle((*pendingArrRes).getHermesValue());
  if (LLVM_UNLIKELY(
          JSObject::defineNewOwnProperty(
              selfHandle,
              runtime,
              Predefined::getSymbolID(
                  Predefined::InternalPropertyTextDecoderPending),
              PropertyFlags::defaultNewNamedPropertyFlags(),
              pendingHandle) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto bomSeenHandle =
      runtime.makeHandle(HermesValue::encodeBoolValue(false));
  if (LLVM_UNLIKELY(
          JSObject::defineNewOwnProperty(
              selfHandle,
              runtime,
              Predefined::getSymbolID(
                  Predefined::InternalPropertyTextDecoderBOMSeen),
              PropertyFlags::defaultNewNamedPropertyFlags(),
              bomSeenHandle) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return selfHandle.getHermesValue();
}

CallResult<HermesValue>
textDecoderPrototypeEncoding(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  auto selfHandle = args.dyncastThis<JSObject>();
  if (!selfHandle) {
    return runtime.raiseTypeError(
        "TextDecoder.prototype.encoding called on non-TextDecoder object");
  }

  TextDecoderEncoding encoding;
  if (!isTextDecoderObject(selfHandle, runtime, &encoding)) {
    return runtime.raiseTypeError(
        "TextDecoder.prototype.encoding called on non-TextDecoder object");
  }

  return HermesValue::encodeStringValue(
      runtime.getPredefinedString(getEncodingName(encoding)));
}

CallResult<HermesValue>
textDecoderPrototypeFatal(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  auto selfHandle = args.dyncastThis<JSObject>();
  if (!selfHandle) {
    return runtime.raiseTypeError(
        "TextDecoder.prototype.fatal called on non-TextDecoder object");
  }

  bool fatal;
  if (!isTextDecoderObject(selfHandle, runtime, nullptr, &fatal)) {
    return runtime.raiseTypeError(
        "TextDecoder.prototype.fatal called on non-TextDecoder object");
  }

  return HermesValue::encodeBoolValue(fatal);
}

CallResult<HermesValue>
textDecoderPrototypeIgnoreBOM(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  auto selfHandle = args.dyncastThis<JSObject>();
  if (!selfHandle) {
    return runtime.raiseTypeError(
        "TextDecoder.prototype.ignoreBOM called on non-TextDecoder object");
  }

  bool ignoreBOM;
  if (!isTextDecoderObject(selfHandle, runtime, nullptr, nullptr, &ignoreBOM)) {
    return runtime.raiseTypeError(
        "TextDecoder.prototype.ignoreBOM called on non-TextDecoder object");
  }

  return HermesValue::encodeBoolValue(ignoreBOM);
}

// Returns the count and fills the buffer (up to 4 bytes).
static size_t getPendingBytes(
    Handle<JSObject> obj,
    Runtime &runtime,
    uint8_t *buf) {
  NamedPropertyDescriptor desc;
  JSObject::getOwnNamedDescriptor(
      obj,
      runtime,
      Predefined::getSymbolID(Predefined::InternalPropertyTextDecoderPending),
      desc);
  HermesValue val =
      JSObject::getNamedSlotValueUnsafe(obj.get(), runtime, desc)
          .unboxToHV(runtime);
  if (!val.isObject()) {
    return 0;
  }
  auto *arr = dyn_vmcast<JSArray>(val);
  if (!arr) {
    return 0;
  }
  size_t count = JSArray::getLength(arr, runtime);
  for (size_t i = 0; i < count && i < 4; ++i) {
    HermesValue elem = arr->at(runtime, i).unboxToHV(runtime);
    buf[i] = static_cast<uint8_t>(elem.getNumber());
  }
  return count;
}

// Creates a new JSArray with the given bytes.
static ExecutionStatus setPendingBytes(
    Handle<JSObject> obj,
    Runtime &runtime,
    const uint8_t *bytes,
    size_t count) {
  auto arrRes = JSArray::create(runtime, count, count);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto arr = *arrRes;
  MutableHandle<> val{runtime};
  for (size_t i = 0; i < count; ++i) {
    val = HermesValue::encodeTrustedNumberValue(bytes[i]);
    JSArray::setElementAt(arr, runtime, i, val);
  }

  NamedPropertyDescriptor desc;
  JSObject::getOwnNamedDescriptor(
      obj,
      runtime,
      Predefined::getSymbolID(Predefined::InternalPropertyTextDecoderPending),
      desc);
  JSObject::setNamedSlotValueUnsafe(
      obj.get(), runtime, desc,
      SmallHermesValue::encodeObjectValue(arr.get(), runtime));
  return ExecutionStatus::RETURNED;
}

static bool getBOMSeen(Handle<JSObject> obj, Runtime &runtime) {
  NamedPropertyDescriptor desc;
  JSObject::getOwnNamedDescriptor(
      obj, runtime,
      Predefined::getSymbolID(Predefined::InternalPropertyTextDecoderBOMSeen),
      desc);
  return JSObject::getNamedSlotValueUnsafe(obj.get(), runtime, desc)
      .unboxToHV(runtime).getBool();
}

static void setBOMSeen(Handle<JSObject> obj, Runtime &runtime, bool val) {
  NamedPropertyDescriptor desc;
  JSObject::getOwnNamedDescriptor(
      obj, runtime,
      Predefined::getSymbolID(Predefined::InternalPropertyTextDecoderBOMSeen),
      desc);
  JSObject::setNamedSlotValueUnsafe(
      obj.get(), runtime, desc, SmallHermesValue::encodeBoolValue(val));
}

CallResult<HermesValue>
textDecoderPrototypeDecode(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  auto selfHandle = args.dyncastThis<JSObject>();
  if (LLVM_UNLIKELY(!selfHandle)) {
    return runtime.raiseTypeError(
        "TextDecoder.prototype.decode() called on non-TextDecoder object");
  }

  TextDecoderEncoding encoding;
  bool fatal;
  bool ignoreBOM;
  if (!isTextDecoderObject(selfHandle, runtime, &encoding, &fatal, &ignoreBOM)) {
    return runtime.raiseTypeError(
        "TextDecoder.prototype.decode() called on non-TextDecoder object");
  }

  // Parse stream option
  bool stream = false;
  if (args.getArgCount() > 1 && !args.getArg(1).isUndefined()) {
    auto optionsHandle = args.getArgHandle(1);
    if (optionsHandle->isObject()) {
      auto optionsObj = Handle<JSObject>::vmcast(optionsHandle);
      auto streamRes = JSObject::getNamed_RJS(
          optionsObj, runtime, Predefined::getSymbolID(Predefined::stream));
      if (LLVM_UNLIKELY(streamRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      if (!(*streamRes)->isUndefined()) {
        stream = toBoolean(streamRes->get());
      }
    }
  }

  // Get pending bytes and bomSeen from a previous execution.
  uint8_t pendingBuf[4];
  size_t pendingCount = getPendingBytes(selfHandle, runtime, pendingBuf);
  bool bomSeen = getBOMSeen(selfHandle, runtime);
  const uint8_t *bytes = nullptr;
  size_t length = 0;

  if (args.getArgCount() > 0 && !args.getArg(0).isUndefined()) {
    auto inputHandle = args.getArgHandle(0);

    if (auto *typedArray = dyn_vmcast<JSTypedArrayBase>(*inputHandle)) {
      if (LLVM_UNLIKELY(!typedArray->attached(runtime))) {
        return runtime.raiseTypeError(
            "TextDecoder.prototype.decode() called with detached buffer");
      }
      bytes = typedArray->begin(runtime);
      length = typedArray->getByteLength();
    } else if (auto *dataView = dyn_vmcast<JSDataView>(*inputHandle)) {
      if (LLVM_UNLIKELY(!dataView->attached(runtime))) {
        return runtime.raiseTypeError(
            "TextDecoder.prototype.decode() called with detached buffer");
      }
      auto buffer = dataView->getBuffer(runtime);
      bytes = buffer->getDataBlock(runtime) + dataView->byteOffset();
      length = dataView->byteLength();
    } else if (auto *arrayBuffer = dyn_vmcast<JSArrayBuffer>(*inputHandle)) {
      if (LLVM_UNLIKELY(!arrayBuffer->attached())) {
        return runtime.raiseTypeError(
            "TextDecoder.prototype.decode() called with detached buffer");
      }
      bytes = arrayBuffer->getDataBlock(runtime);
      length = arrayBuffer->size();
    } else {
      return runtime.raiseTypeError(
          "TextDecoder.prototype.decode() requires an ArrayBuffer or ArrayBufferView");
    }
  }  // else: user called decode() without params.

  bool isByteEncoding =
      encoding == TextDecoderEncoding::UTF8 || isSingleByteEncoding(encoding);
  if (pendingCount == 0 && isByteEncoding) {
    const uint8_t *asciiBytes = bytes;
    size_t asciiLength = length;
    // Skip UTF-8 BOM if present, not ignored, and not already seen.
    if (encoding == TextDecoderEncoding::UTF8 &&
        !ignoreBOM && !bomSeen && asciiLength >= 3 &&
        asciiBytes[0] == 0xEF && asciiBytes[1] == 0xBB && asciiBytes[2] == 0xBF) {
      asciiBytes += 3;
      asciiLength -= 3;
    }
    if (isAllASCII(asciiBytes, asciiBytes + asciiLength)) {
      // Update streaming state if needed (UTF-8 only).
      if (encoding == TextDecoderEncoding::UTF8 &&
          stream && asciiLength > 0 && !bomSeen) {
        setBOMSeen(selfHandle, runtime, true);
      }
      return StringPrimitive::createEfficient(
          runtime,
          ASCIIRef(reinterpret_cast<const char *>(asciiBytes), asciiLength));
    }
  }

  // Combine pending bytes with new input
  const uint8_t *inputBytes;
  size_t inputLength;
  std::vector<uint8_t> combined;

  if (pendingCount > 0) {
    // Need to combine pending bytes with new input
    combined.reserve(pendingCount + length);
    combined.insert(combined.end(), pendingBuf, pendingBuf + pendingCount);
    if (bytes && length > 0) {
      combined.insert(combined.end(), bytes, bytes + length);
    }
    inputBytes = combined.data();
    inputLength = combined.size();
  } else {
    // No pending bytes, use input directly without copying
    inputBytes = bytes;
    inputLength = length;
  }

  uint8_t newPendingBytes[4];
  size_t newPendingCount = 0;
  bool newBOMSeen = bomSeen;
  std::u16string decoded;
  DecodeError err = DecodeError::None;

  if (encoding == TextDecoderEncoding::UTF8) {
    err = decodeUTF8(
        inputBytes, inputLength, fatal, ignoreBOM, stream, bomSeen, &decoded,
        newPendingBytes, &newPendingCount, &newBOMSeen);
  } else if (encoding == TextDecoderEncoding::UTF16LE) {
    err = decodeUTF16(
        inputBytes, inputLength, fatal, ignoreBOM, false, stream, bomSeen,
        &decoded, newPendingBytes, &newPendingCount, &newBOMSeen);
  } else if (encoding == TextDecoderEncoding::UTF16BE) {
    err = decodeUTF16(
        inputBytes, inputLength, fatal, ignoreBOM, true, stream, bomSeen,
        &decoded, newPendingBytes, &newPendingCount, &newBOMSeen);
  } else if (isSingleByteEncoding(encoding)) {
    // All single-byte encodings: no BOM, no streaming state, 1:1 byte mapping.
    size_t tableIndex =
        static_cast<uint8_t>(encoding) - kFirstSingleByteEncoding;
    err = decodeSingleByteEncoding(
        inputBytes, inputLength, kSingleByteEncodings[tableIndex], fatal,
        &decoded);
    newPendingCount = 0;
    newBOMSeen = true;
  }

  // Update or clear streaming state
  if (stream) {
    if (LLVM_UNLIKELY(
            setPendingBytes(
                selfHandle, runtime, newPendingBytes, newPendingCount) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    setBOMSeen(selfHandle, runtime, newBOMSeen);
  } else {
    if (LLVM_UNLIKELY(
            setPendingBytes(selfHandle, runtime, nullptr, 0) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    setBOMSeen(selfHandle, runtime, false);
  }

  if (LLVM_UNLIKELY(err != DecodeError::None)) {
    switch (err) {
      case DecodeError::InvalidSequence:
        return runtime.raiseTypeError("Invalid UTF-8 sequence");
      case DecodeError::InvalidSurrogate:
        return runtime.raiseTypeError("Invalid UTF-16: lone surrogate");
      case DecodeError::OddByteCount:
        return runtime.raiseTypeError("Invalid UTF-16 data (odd byte count)");
      default:
        return runtime.raiseTypeError("Decoding error");
    }
  }

  return StringPrimitive::createEfficient(runtime, std::move(decoded));
}

} // namespace vm
} // namespace hermes
