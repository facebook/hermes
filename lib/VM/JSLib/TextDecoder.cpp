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
  // Normalize the label by trimming whitespace and converting to lowercase
  // for comparison. Per the WHATWG Encoding spec, labels are ASCII
  // case-insensitive.

  auto begin = label.begin();
  auto end = label.end();

  // Trim leading whitespace
  while (begin != end && (*begin == ' ' || *begin == '\t' || *begin == '\n' ||
                          *begin == '\r' || *begin == '\f')) {
    ++begin;
  }

  // Trim trailing whitespace
  while (begin != end) {
    auto last = end - 1;
    if (*last == ' ' || *last == '\t' || *last == '\n' || *last == '\r' ||
        *last == '\f') {
      end = last;
    } else {
      break;
    }
  }

  // Compare case-insensitively
  auto compareIgnoreCase = [](StringView sv, const char *str) -> bool {
    size_t len = strlen(str);
    if (sv.length() != len) {
      return false;
    }
    auto it = sv.begin();
    for (size_t i = 0; i < len; ++i, ++it) {
      char16_t c = *it;
      // Convert to lowercase for ASCII letters
      if (c >= 'A' && c <= 'Z') {
        c = c - 'A' + 'a';
      }
      if (c != str[i]) {
        return false;
      }
    }
    return true;
  };

  StringView trimmed = label.slice(begin, end);

  // UTF-8 encodings (per WHATWG Encoding spec)
  if (compareIgnoreCase(trimmed, "utf-8") ||
      compareIgnoreCase(trimmed, "utf8") ||
      compareIgnoreCase(trimmed, "unicode-1-1-utf-8")) {
    return TextDecoderEncoding::UTF8;
  }

  // UTF-16LE encodings
  if (compareIgnoreCase(trimmed, "utf-16le") ||
      compareIgnoreCase(trimmed, "utf-16")) {
    return TextDecoderEncoding::UTF16LE;
  }

  // UTF-16BE encodings
  if (compareIgnoreCase(trimmed, "utf-16be")) {
    return TextDecoderEncoding::UTF16BE;
  }

  // Latin-1 / ISO-8859-1 encodings (per WHATWG Encoding spec)
  if (compareIgnoreCase(trimmed, "iso-8859-1") ||
      compareIgnoreCase(trimmed, "iso8859-1") ||
      compareIgnoreCase(trimmed, "iso88591") ||
      compareIgnoreCase(trimmed, "latin1") ||
      compareIgnoreCase(trimmed, "latin-1") ||
      compareIgnoreCase(trimmed, "l1") ||
      compareIgnoreCase(trimmed, "ascii") ||
      compareIgnoreCase(trimmed, "us-ascii") ||
      compareIgnoreCase(trimmed, "iso-ir-100") ||
      compareIgnoreCase(trimmed, "csisolatin1") ||
      compareIgnoreCase(trimmed, "windows-1252") ||
      compareIgnoreCase(trimmed, "cp1252")) {
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

/// Check if this object is a valid TextDecoder instance.
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

  // Get encoding
  if (outEncoding) {
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

  // Get fatal flag
  if (outFatal) {
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

  // Get ignoreBOM flag
  if (outIgnoreBOM) {
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
  // Handle empty input
  if (length == 0) {
    return HermesValue::encodeStringValue(
        runtime.getPredefinedString(Predefined::emptyString));
  }

  const uint8_t *start = bytes;
  const uint8_t *end = bytes + length;

  // Skip BOM if present and ignoreBOM is false
  // UTF-8 BOM is EF BB BF
  if (!ignoreBOM && length >= 3 && start[0] == 0xEF && start[1] == 0xBB &&
      start[2] == 0xBF) {
    start += 3;
  }

  if (start == end) {
    return HermesValue::encodeStringValue(
        runtime.getPredefinedString(Predefined::emptyString));
  }

  // First pass: determine if result is ASCII-only and calculate output size
  bool isASCII = true;
  size_t utf16Length = 0;
  const uint8_t *p = start;

  while (p < end) {
    uint8_t b = *p;
    if (b < 0x80) {
      // ASCII byte
      utf16Length++;
      p++;
    } else if ((b & 0xE0) == 0xC0) {
      // 2-byte sequence
      if (p + 1 >= end || (p[1] & 0xC0) != 0x80) {
        if (fatal) {
          return runtime.raiseTypeError("Invalid UTF-8 sequence");
        }
        // Replacement character
        utf16Length++;
        p++;
        isASCII = false;
        continue;
      }
      uint32_t cp = ((b & 0x1F) << 6) | (p[1] & 0x3F);
      // Check for overlong encoding
      if (cp < 0x80) {
        if (fatal) {
          return runtime.raiseTypeError("Invalid UTF-8 sequence (overlong)");
        }
        utf16Length++;
        p += 2;
        isASCII = false;
        continue;
      }
      utf16Length++;
      p += 2;
      isASCII = false;
    } else if ((b & 0xF0) == 0xE0) {
      // 3-byte sequence
      if (p + 2 >= end || (p[1] & 0xC0) != 0x80 || (p[2] & 0xC0) != 0x80) {
        if (fatal) {
          return runtime.raiseTypeError("Invalid UTF-8 sequence");
        }
        utf16Length++;
        p++;
        isASCII = false;
        continue;
      }
      uint32_t cp =
          ((b & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
      // Check for overlong encoding and surrogate range
      if (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF)) {
        if (fatal) {
          return runtime.raiseTypeError("Invalid UTF-8 sequence");
        }
        utf16Length++;
        p += 3;
        isASCII = false;
        continue;
      }
      utf16Length++;
      p += 3;
      isASCII = false;
    } else if ((b & 0xF8) == 0xF0) {
      // 4-byte sequence
      if (p + 3 >= end || (p[1] & 0xC0) != 0x80 || (p[2] & 0xC0) != 0x80 ||
          (p[3] & 0xC0) != 0x80) {
        if (fatal) {
          return runtime.raiseTypeError("Invalid UTF-8 sequence");
        }
        utf16Length++;
        p++;
        isASCII = false;
        continue;
      }
      uint32_t cp = ((b & 0x07) << 18) | ((p[1] & 0x3F) << 12) |
          ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
      // Check for overlong encoding and valid range
      if (cp < 0x10000 || cp > 0x10FFFF) {
        if (fatal) {
          return runtime.raiseTypeError("Invalid UTF-8 sequence");
        }
        utf16Length++;
        p += 4;
        isASCII = false;
        continue;
      }
      // Surrogate pair
      utf16Length += 2;
      p += 4;
      isASCII = false;
    } else {
      // Invalid leading byte
      if (fatal) {
        return runtime.raiseTypeError("Invalid UTF-8 sequence");
      }
      utf16Length++;
      p++;
      isASCII = false;
    }
  }

  // Create the result string
  if (isASCII) {
    // All ASCII, can create directly
    auto strRes = StringPrimitive::create(
        runtime,
        ASCIIRef(reinterpret_cast<const char *>(start), end - start));
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return strRes;
  }

  // Need to decode to UTF-16
  auto builderRes =
      StringBuilder::createStringBuilder(runtime, SafeUInt32(utf16Length));
  if (LLVM_UNLIKELY(builderRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto builder = std::move(*builderRes);

  p = start;
  while (p < end) {
    uint8_t b = *p;
    if (b < 0x80) {
      builder.appendCharacter(b);
      p++;
    } else if ((b & 0xE0) == 0xC0) {
      if (p + 1 >= end || (p[1] & 0xC0) != 0x80) {
        builder.appendCharacter(0xFFFD); // Replacement character
        p++;
        continue;
      }
      uint32_t cp = ((b & 0x1F) << 6) | (p[1] & 0x3F);
      if (cp < 0x80) {
        builder.appendCharacter(0xFFFD);
        p += 2;
        continue;
      }
      builder.appendCharacter(static_cast<char16_t>(cp));
      p += 2;
    } else if ((b & 0xF0) == 0xE0) {
      if (p + 2 >= end || (p[1] & 0xC0) != 0x80 || (p[2] & 0xC0) != 0x80) {
        builder.appendCharacter(0xFFFD);
        p++;
        continue;
      }
      uint32_t cp =
          ((b & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
      if (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF)) {
        builder.appendCharacter(0xFFFD);
        p += 3;
        continue;
      }
      builder.appendCharacter(static_cast<char16_t>(cp));
      p += 3;
    } else if ((b & 0xF8) == 0xF0) {
      if (p + 3 >= end || (p[1] & 0xC0) != 0x80 || (p[2] & 0xC0) != 0x80 ||
          (p[3] & 0xC0) != 0x80) {
        builder.appendCharacter(0xFFFD);
        p++;
        continue;
      }
      uint32_t cp = ((b & 0x07) << 18) | ((p[1] & 0x3F) << 12) |
          ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
      if (cp < 0x10000 || cp > 0x10FFFF) {
        builder.appendCharacter(0xFFFD);
        p += 4;
        continue;
      }
      // Encode as surrogate pair
      cp -= 0x10000;
      builder.appendCharacter(static_cast<char16_t>(0xD800 + (cp >> 10)));
      builder.appendCharacter(static_cast<char16_t>(0xDC00 + (cp & 0x3FF)));
      p += 4;
    } else {
      builder.appendCharacter(0xFFFD);
      p++;
    }
  }

  return builder.getStringPrimitive().getHermesValue();
}

/// Decode UTF-16LE bytes to a string.
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

  // Skip BOM if present and ignoreBOM is false
  // UTF-16LE BOM is FF FE
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

/// Decode UTF-16BE bytes to a string.
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

/// Decode Latin-1 (ISO-8859-1) bytes to a string.
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
