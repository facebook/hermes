/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSLibInternal.h"

#include "llvh/Support/ConvertUTF.h"

#include "hermes/VM/JSArrayBuffer.h"
#include "hermes/VM/JSDataView.h"
#include "hermes/VM/JSTypedArray.h"
#include "hermes/VM/StringBuilder.h"

namespace hermes {
namespace vm {

/// Encoding types supported by TextDecoder.
enum class TextDecoderEncoding : uint8_t {
  UTF8 = 0,
  UTF16LE = 1,
  UTF16BE = 2,
  Latin1 = 3,
};

/// Parse the encoding label and return the corresponding encoding type.
/// Returns llvh::None if the encoding is not supported.
static llvh::Optional<TextDecoderEncoding> parseEncodingLabel(
    StringView label) {
  if (!label.isASCII()) {  // Encoding labels must be ascii.
    return llvh::None;
  }

  auto isASCIIWhitespace = [](char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
  };
  const char *begin = label.castToCharPtr();
  const char *end = begin + label.length();
  while (begin != end && isASCIIWhitespace(*begin))
    ++begin;
  while (end != begin && isASCIIWhitespace(*(end - 1)))
    --end;

  llvh::StringRef trimmed(begin, end - begin);
  if (trimmed.equals_lower("utf-8") || trimmed.equals_lower("utf8") ||
      trimmed.equals_lower("unicode-1-1-utf-8")) {
    return TextDecoderEncoding::UTF8;
  }

  if (trimmed.equals_lower("utf-16le") || trimmed.equals_lower("utf-16")) {
    return TextDecoderEncoding::UTF16LE;
  }

  if (trimmed.equals_lower("utf-16be")) {
    return TextDecoderEncoding::UTF16BE;
  }

  if (trimmed.equals_lower("iso-8859-1") ||
      trimmed.equals_lower("iso8859-1") || trimmed.equals_lower("iso88591") ||
      trimmed.equals_lower("latin1") || trimmed.equals_lower("latin-1") ||
      trimmed.equals_lower("l1") || trimmed.equals_lower("ascii") ||
      trimmed.equals_lower("us-ascii") || trimmed.equals_lower("iso-ir-100") ||
      trimmed.equals_lower("csisolatin1") ||
      trimmed.equals_lower("windows-1252") || trimmed.equals_lower("cp1252")) {
    return TextDecoderEncoding::Latin1;
  }

  return llvh::None;
}

/// Get the canonical encoding name for the given encoding type.
static Predefined::Str getEncodingName(TextDecoderEncoding encoding) {
  switch (encoding) {
    case TextDecoderEncoding::UTF8:
      return Predefined::utf8;
    case TextDecoderEncoding::UTF16LE:
      return Predefined::utf16le;
    case TextDecoderEncoding::UTF16BE:
      return Predefined::utf16be;
    case TextDecoderEncoding::Latin1:
      return Predefined::latin1;
  }
  llvm_unreachable("Invalid encoding");
}

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

/// Decode UTF-8 bytes to a string.
static CallResult<HermesValue> decodeUTF8(
    Runtime &runtime,
    const uint8_t *bytes,
    size_t length,
    bool fatal,
    bool ignoreBOM) {
  // Skip BOM if present and ignoreBOM is false. UTF-8 BOM is EF BB BF.
  if (!ignoreBOM && length >= 3 &&
      bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF) {
    bytes += 3;
    length -= 3;
  }

  if (length == 0) {
    return HermesValue::encodeStringValue(
        runtime.getPredefinedString(Predefined::emptyString));
  }

  // In fatal mode, we need to validate the UTF-8 first since createEfficient
  // with IgnoreInputErrors=false raises RangeError, but we need TypeError.
  if (fatal) {
    const uint8_t *tmp = bytes;
    if (!llvh::isLegalUTF8String(&tmp, bytes + length)) {
      return runtime.raiseTypeError("Invalid UTF-8 sequence");
    }
  }

  return StringPrimitive::createEfficient(
      runtime, llvh::makeArrayRef(bytes, length), /*IgnoreInputErrors=*/true);
}

static CallResult<HermesValue> decodeUTF16LE(
    Runtime &runtime,
    const uint8_t *bytes,
    size_t length,
    bool fatal,
    bool ignoreBOM) {
  // UTF-16 requires even number of bytes
  if (length % 2 != 0) {
    if (fatal) {
      return runtime.raiseTypeError("Invalid UTF-16 data (odd byte count)");
    }
    // Truncate to even
    length = length - 1;
  }

  if (length == 0) {
    return HermesValue::encodeStringValue(
        runtime.getPredefinedString(Predefined::emptyString));
  }

  const uint8_t *start = bytes;
  const uint8_t *end = bytes + length;

  // Skip BOM if present and ignoreBOM is false; UTF-16LE BOM is FF FE
  if (!ignoreBOM && length >= 2 && start[0] == 0xFF && start[1] == 0xFE) {
    start += 2;
  }

  size_t numCodeUnits = (end - start) / 2;
  if (numCodeUnits == 0) {
    return HermesValue::encodeStringValue(
        runtime.getPredefinedString(Predefined::emptyString));
  }

  auto builderRes =
      StringBuilder::createStringBuilder(runtime, SafeUInt32(numCodeUnits));
  if (LLVM_UNLIKELY(builderRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto builder = std::move(*builderRes);

  for (const uint8_t *p = start; p < end; p += 2) {
    char16_t codeUnit = static_cast<char16_t>(p[0]) |
        (static_cast<char16_t>(p[1]) << 8);
    builder.appendCharacter(codeUnit);
  }

  return builder.getStringPrimitive().getHermesValue();
}

static CallResult<HermesValue> decodeUTF16BE(
    Runtime &runtime,
    const uint8_t *bytes,
    size_t length,
    bool fatal,
    bool ignoreBOM) {
  // UTF-16 requires even number of bytes
  if (length % 2 != 0) {
    if (fatal) {
      return runtime.raiseTypeError("Invalid UTF-16 data (odd byte count)");
    }
    // Truncate to even
    length = length - 1;
  }

  if (length == 0) {
    return HermesValue::encodeStringValue(
        runtime.getPredefinedString(Predefined::emptyString));
  }

  const uint8_t *start = bytes;
  const uint8_t *end = bytes + length;

  // Skip BOM if present and ignoreBOM is false
  // UTF-16BE BOM is FE FF
  if (!ignoreBOM && length >= 2 && start[0] == 0xFE && start[1] == 0xFF) {
    start += 2;
  }

  size_t numCodeUnits = (end - start) / 2;
  if (numCodeUnits == 0) {
    return HermesValue::encodeStringValue(
        runtime.getPredefinedString(Predefined::emptyString));
  }

  auto builderRes =
      StringBuilder::createStringBuilder(runtime, SafeUInt32(numCodeUnits));
  if (LLVM_UNLIKELY(builderRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto builder = std::move(*builderRes);

  for (const uint8_t *p = start; p < end; p += 2) {
    char16_t codeUnit = (static_cast<char16_t>(p[0]) << 8) |
        static_cast<char16_t>(p[1]);
    builder.appendCharacter(codeUnit);
  }

  return builder.getStringPrimitive().getHermesValue();
}

static CallResult<HermesValue> decodeLatin1(
    Runtime &runtime,
    const uint8_t *bytes,
    size_t length,
    bool /* fatal */,
    bool /* ignoreBOM */) {
  // Latin-1 is a subset of Unicode where each byte maps directly to
  // a code point. No BOM handling needed for Latin-1.

  if (length == 0) {
    return HermesValue::encodeStringValue(
        runtime.getPredefinedString(Predefined::emptyString));
  }

  // Check if all bytes are ASCII
  bool isASCII = true;
  for (size_t i = 0; i < length; ++i) {
    if (bytes[i] >= 0x80) {
      isASCII = false;
      break;
    }
  }

  if (isASCII) {
    // All ASCII, can create directly
    auto strRes = StringPrimitive::create(
        runtime,
        ASCIIRef(reinterpret_cast<const char *>(bytes), length));
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return strRes;
  }

  // Need to create UTF-16 string (Latin-1 bytes above 0x7F need char16_t)
  auto builderRes =
      StringBuilder::createStringBuilder(runtime, SafeUInt32(length));
  if (LLVM_UNLIKELY(builderRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto builder = std::move(*builderRes);

  for (size_t i = 0; i < length; ++i) {
    builder.appendCharacter(static_cast<char16_t>(bytes[i]));
  }

  return builder.getStringPrimitive().getHermesValue();
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

  // Handle the case where no input is provided
  if (args.getArgCount() == 0 || args.getArg(0).isUndefined()) {
    return HermesValue::encodeStringValue(
        runtime.getPredefinedString(Predefined::emptyString));
  }

  // Get the input buffer
  auto inputHandle = args.getArgHandle(0);
  const uint8_t *bytes = nullptr;
  size_t length = 0;

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

  // Decode based on encoding
  switch (encoding) {
    case TextDecoderEncoding::UTF8:
      return decodeUTF8(runtime, bytes, length, fatal, ignoreBOM);
    case TextDecoderEncoding::UTF16LE:
      return decodeUTF16LE(runtime, bytes, length, fatal, ignoreBOM);
    case TextDecoderEncoding::UTF16BE:
      return decodeUTF16BE(runtime, bytes, length, fatal, ignoreBOM);
    case TextDecoderEncoding::Latin1:
      return decodeLatin1(runtime, bytes, length, fatal, ignoreBOM);
  }

  llvm_unreachable("Invalid encoding");
}

} // namespace vm
} // namespace hermes
