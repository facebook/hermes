/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSLibInternal.h"

#include "hermes/Platform/Unicode/CharacterProperties.h"
#include "hermes/Support/UTF8.h"
#include "llvh/Support/ConvertUTF.h"

#include "hermes/VM/JSArray.h"
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
  Windows1252 = 3,
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
    return TextDecoderEncoding::Windows1252;
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
    case TextDecoderEncoding::Windows1252:
      return Predefined::windows1252;
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


/// Get the expected length of a UTF-8 sequence from its first byte.
static size_t utf8SequenceLength(uint8_t byte) {
  if ((byte & 0x80) == 0) return 1;       // ASCII
  if ((byte & 0xE0) == 0xC0) return 2;    // 110xxxxx
  if ((byte & 0xF0) == 0xE0) return 3;    // 1110xxxx
  if ((byte & 0xF8) == 0xF0) return 4;    // 11110xxx
  return 0;  // Continuation byte or invalid
}

/// Decode UTF-8 bytes to a string, with streaming support.
static CallResult<HermesValue> decodeUTF8(
    Runtime &runtime,
    const uint8_t *bytes,
    size_t length,
    bool fatal,
    bool ignoreBOM,
    bool stream,
    bool bomSeen,
    uint8_t outPendingBytes[4],
    size_t *outPendingCount,
    bool *outBOMSeen) {
  *outPendingCount = 0;
  *outBOMSeen = bomSeen;

  // Handle BOM (only strip once at the start of stream)
  if (!ignoreBOM && !bomSeen && length >= 3 &&
      bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF) {
    bytes += 3;
    length -= 3;
    *outBOMSeen = true;
  } else if (length > 0) {
    *outBOMSeen = true;
  }

  if (length == 0) {
    return HermesValue::encodeStringValue(
        runtime.getPredefinedString(Predefined::emptyString));
  }

  // If streaming, check for incomplete sequence at end
  size_t processLength = length;
  size_t pendingStart = length;

  if (stream) {
    size_t i = length;
    while (i > 0) {
      --i;
      uint8_t byte = bytes[i];
      if ((byte & 0xC0) != 0x80) {  // Found lead byte
        size_t seqLen = utf8SequenceLength(byte);
        size_t remaining = length - i;
        if (seqLen > 0 && remaining < seqLen) {
          pendingStart = i;
          processLength = i;
        }
        break;
      }
    }
  }

  std::u16string result;
  result.reserve(processLength);

  const llvh::UTF8 *src = bytes;
  const llvh::UTF8 *srcEnd = bytes + processLength;

  while (src < srcEnd) {
    llvh::UTF16 buf[256];
    llvh::UTF16 *dst = buf;
    llvh::ConversionResult res = llvh::ConvertUTF8toUTF16(
        &src, srcEnd, &dst, buf + 256, llvh::lenientConversion);

    result.append(reinterpret_cast<char16_t *>(buf), reinterpret_cast<char16_t *>(dst));

    if (res == llvh::conversionOK) {
      break;
    } else if (res == llvh::sourceIllegal) {
      if (fatal) {
        return runtime.raiseTypeError("Invalid UTF-8 sequence");
      }
      result.push_back(UNICODE_REPLACEMENT_CHARACTER);
      ++src;
    } else if (res == llvh::sourceExhausted) {
      if (stream) {
        break;
      }
      if (fatal) {
        return runtime.raiseTypeError("Invalid UTF-8 sequence");
      }
      result.push_back(UNICODE_REPLACEMENT_CHARACTER);
      ++src;
    }
  }

  // Store pending bytes if streaming; else emit replacement char.
  if (stream && pendingStart < length) {
    *outPendingCount = length - pendingStart;
    for (size_t i = 0; i < *outPendingCount; ++i) {
      outPendingBytes[i] = bytes[pendingStart + i];
    }
  }
  if (!stream && processLength < length) {
    if (fatal) {
      return runtime.raiseTypeError("Invalid UTF-8 sequence");
    }
    result.push_back(UNICODE_REPLACEMENT_CHARACTER);
  }

  return StringPrimitive::createEfficient(runtime, std::move(result));
}

static CallResult<HermesValue> decodeUTF16(
    Runtime &runtime,
    const uint8_t *bytes,
    size_t length,
    bool fatal,
    bool ignoreBOM,
    bool bigEndian,
    bool stream,
    bool bomSeen,
    uint8_t outPendingBytes[4],
    size_t *outPendingCount,
    bool *outBOMSeen) {
  *outPendingCount = 0;
  *outBOMSeen = bomSeen;

  bool hasTrailingByte = length % 2 != 0;
  size_t evenLength = hasTrailingByte ? length - 1 : length;

  if (hasTrailingByte && !stream) {
    if (fatal) {
      return runtime.raiseTypeError("Invalid UTF-16 data (odd byte count)");
    }
  }

  const uint8_t *start = bytes;
  const uint8_t *end = bytes + evenLength;

  // Handle BOM (only strip once at the start of stream)
  if (!ignoreBOM && !bomSeen && evenLength >= 2) {
    if ((bigEndian && start[0] == 0xFE && start[1] == 0xFF) ||
        (!bigEndian && start[0] == 0xFF && start[1] == 0xFE)) {
      start += 2;
      *outBOMSeen = true;
    }
  }
  if (evenLength > 0) {
    *outBOMSeen = true;
  }

  auto readU16 = [bigEndian](const uint8_t *p) -> char16_t {
    if (bigEndian) {
      return (static_cast<char16_t>(p[0]) << 8) | static_cast<char16_t>(p[1]);
    } else {
      return static_cast<char16_t>(p[0]) | (static_cast<char16_t>(p[1]) << 8);
    }
  };

  std::u16string result;
  result.reserve((end - start) / 2 + 1);

  const uint8_t *p = start;
  while (p < end) {
    char16_t cu = readU16(p);

    if (isHighSurrogate(cu)) {
      if (p + 4 <= end) {
        char16_t next = readU16(p + 2);
        if (isLowSurrogate(next)) {
          result.push_back(cu);
          result.push_back(next);
          p += 4;
          continue;
        }
      } else if (stream && p + 2 == end) {
        // High surrogate at end - save as pending
        outPendingBytes[0] = p[0];
        outPendingBytes[1] = p[1];
        *outPendingCount = 2;
        break;
      }
      if (fatal) {
        return runtime.raiseTypeError("Invalid UTF-16: lone surrogate");
      }
      result.push_back(UNICODE_REPLACEMENT_CHARACTER);
      p += 2;
    } else if (isLowSurrogate(cu)) {
      if (fatal) {
        return runtime.raiseTypeError("Invalid UTF-16: lone surrogate");
      }
      result.push_back(UNICODE_REPLACEMENT_CHARACTER);
      p += 2;
    } else {
      result.push_back(cu);
      p += 2;
    }
  }

  // Handle trailing odd byte
  if (hasTrailingByte) {
    if (stream) {
      // Append the odd byte to any existing pending bytes
      outPendingBytes[*outPendingCount] = bytes[length - 1];
      ++(*outPendingCount);
    } else {
      result.push_back(UNICODE_REPLACEMENT_CHARACTER);
    }
  }

  return StringPrimitive::createEfficient(runtime, std::move(result));
}

/// Windows-1252 lookup table for special characters.
static constexpr char16_t kWindows1252Table[32] = {
    0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,  // 80-87
    0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,  // 88-8F
    0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,  // 90-97
    0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178,  // 98-9F
};

/// Windows-1252 is a single-byte encoding. Bytes 0x00-0x7F and 0xA0-0xFF map
/// directly to Unicode, but 0x80-0x9F use a special lookup table.
static CallResult<HermesValue> decodeWindows1252(
    Runtime &runtime,
    const uint8_t *bytes,
    size_t length) {
  if (isAllASCII(bytes, bytes + length)) {  // Fast path for pure ASCII strings.
    return StringPrimitive::create(
        runtime, ASCIIRef(reinterpret_cast<const char *>(bytes), length));
  }

  auto builderRes =
      StringBuilder::createStringBuilder(runtime, SafeUInt32(length));
  if (LLVM_UNLIKELY(builderRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto builder = std::move(*builderRes);

  for (size_t i = 0; i < length; ++i) {
    uint8_t byte = bytes[i];
    if (byte >= 0x80 && byte <= 0x9F) {
      builder.appendCharacter(kWindows1252Table[byte - 0x80]);
    } else {
      builder.appendCharacter(static_cast<char16_t>(byte));
    }
  }

  return builder.getStringPrimitive().getHermesValue();
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

/// Creates a new JSArray with the given bytes.
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
  CallResult<HermesValue> result = ExecutionStatus::EXCEPTION;

  switch (encoding) {
    case TextDecoderEncoding::UTF8:
      result = decodeUTF8(
          runtime, inputBytes, inputLength, fatal, ignoreBOM, stream, bomSeen,
          newPendingBytes, &newPendingCount, &newBOMSeen);
      break;
    case TextDecoderEncoding::UTF16LE:
      result = decodeUTF16(
          runtime, inputBytes, inputLength, fatal, ignoreBOM, false, stream,
          bomSeen, newPendingBytes, &newPendingCount, &newBOMSeen);
      break;
    case TextDecoderEncoding::UTF16BE:
      result = decodeUTF16(
          runtime, inputBytes, inputLength, fatal, ignoreBOM, true, stream,
          bomSeen, newPendingBytes, &newPendingCount, &newBOMSeen);
      break;
    case TextDecoderEncoding::Windows1252:
      result = decodeWindows1252(runtime, inputBytes, inputLength);
      newPendingCount = 0;
      newBOMSeen = true;
      break;
  }

  if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
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

  return result;
}

} // namespace vm
} // namespace hermes
