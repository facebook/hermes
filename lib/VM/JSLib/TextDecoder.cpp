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
#include "hermes/VM/StringRefUtils.h"
#include "hermes/VM/JSDataView.h"
#include "hermes/VM/JSTypedArray.h"
#include "hermes/VM/StringBuilder.h"

namespace hermes {
namespace vm {

// Encoding types supported by TextDecoder. isSingleByteEncoding and kSingleByteEncodings
// depend on the defaule enum values.
enum class TextDecoderEncoding : uint8_t {
  UTF8,
  UTF16LE,
  UTF16BE,
  IBM866,
  ISO_8859_2,
  ISO_8859_3,
  ISO_8859_4,
  ISO_8859_5,
  ISO_8859_6,
  ISO_8859_7,
  ISO_8859_8,
  ISO_8859_8_I,
  ISO_8859_10,
  ISO_8859_13,
  ISO_8859_14,
  ISO_8859_15,
  ISO_8859_16,
  KOI8_R,
  KOI8_U,
  Macintosh,
  Windows874,
  Windows1250,
  Windows1251,
  Windows1252,
  Windows1253,
  Windows1254,
  Windows1255,
  Windows1256,
  Windows1257,
  Windows1258,
  XMacCyrillic,
};

// First single-byte encoding value.
static constexpr uint8_t kFirstSingleByteEncoding =
    static_cast<uint8_t>(TextDecoderEncoding::IBM866);

static bool isSingleByteEncoding(TextDecoderEncoding enc) {
  return static_cast<uint8_t>(enc) >= kFirstSingleByteEncoding;
}

enum class DecodeError {
  None = 0,
  InvalidSequence,  // Invalid byte sequence (fatal mode)
  InvalidSurrogate, // Invalid surrogate (fatal mode)
  OddByteCount,     // Odd byte count in UTF-16 (fatal mode)
};

// Returns the expected UTF-8 sequence length for a valid lead byte.
// Returns 0 for invalid lead bytes (continuation bytes, 0xC0-0xC1, 0xF5-0xFF).
// This is stricter than llvh::getNumBytesForUTF8 which returns non-zero for
// some invalid lead bytes.
static unsigned validUTF8SequenceLength(uint8_t byte) {
  if (byte < 0x80) {
    return 1;  // ASCII
  }
  if (byte < 0xC2) {
    return 0;  // Continuation byte or overlong (0xC0, 0xC1)
  }
  if (byte < 0xE0) {
    return 2;  // 2-byte sequence (0xC2-0xDF)
  }
  if (byte < 0xF0) {
    return 3;  // 3-byte sequence (0xE0-0xEF)
  }
  if (byte < 0xF5) {
    return 4;  // 4-byte sequence (0xF0-0xF4)
  }
  return 0;    // Invalid (0xF5-0xFF would encode > U+10FFFF)
}

// Check if a partial UTF-8 sequence could possibly be completed to form
// a valid codepoint. Returns true if the sequence could be valid with more bytes.
static bool isValidPartialUTF8(const uint8_t *bytes, size_t len) {
  if (len == 0) {
    return false;
  }

  uint8_t b0 = bytes[0];
  unsigned expectedLen = validUTF8SequenceLength(b0);
  if (expectedLen == 0 || len >= expectedLen) {
    return false;  // Invalid lead or already complete
  }

  // Check second byte constraints for 3 and 4 byte sequences
  if (len >= 2) {
    uint8_t b1 = bytes[1];
    // Must be continuation byte
    if ((b1 & 0xC0) != 0x80) {
      return false;
    }
    if (b0 == 0xE0 && b1 < 0xA0) {
      return false;  // Overlong 3-byte
    }
    if (b0 == 0xED && b1 > 0x9F) {
      return false;  // Would produce surrogate (D800-DFFF)
    }
    if (b0 == 0xF0 && b1 < 0x90) {
      return false;  // Overlong 4-byte
    }
    if (b0 == 0xF4 && b1 > 0x8F) {
      return false;  // Would be > U+10FFFF
    }
  }

  // Check third byte for 4-byte sequences
  if (len >= 3) {
    uint8_t b2 = bytes[2];
    if ((b2 & 0xC0) != 0x80) {
      return false;
    }
  }

  return true;
}

// Unicode 6.3.0, D93b:
//   Maximal subpart of an ill-formed subsequence: The longest code unit
//   subsequence starting at an unconvertible offset that is either:
//   a. the initial subsequence of a well-formed code unit sequence, or
//   b. a subsequence of length one.
static unsigned maximalSubpartLength(const uint8_t *bytes, size_t available) {
  if (available == 0) {
    return 0;
  }

  // Find the longest prefix that isValidPartialUTF8 returns true for.
  unsigned maxLen = 1;
  for (unsigned len = 2; len <= available && len <= 4; ++len) {
    if (isValidPartialUTF8(bytes, len)) {
      maxLen = len;
    } else {
      break;
    }
  }
  return maxLen;
}

// Parse the encoding label and return the corresponding encoding type.
// Returns llvh::None if the encoding is not supported.
static llvh::Optional<TextDecoderEncoding> parseEncodingLabel(
    StringView label) {
  // Copy to a char buffer, checking that all characters are ASCII.
  // Encoding labels must be ASCII per the WHATWG spec.
  llvh::SmallVector<char, 32> buf;
  buf.reserve(label.length());
  for (size_t i = 0; i < label.length(); ++i) {
    char16_t c = label[i];
    if (c > 127) {
      return llvh::None;  // Non-ASCII character in label.
    }
    buf.push_back(static_cast<char>(c));
  }

  auto isASCIIWhitespace = [](char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
  };
  const char *begin = buf.data();
  const char *end = begin + buf.size();
  while (begin != end && isASCIIWhitespace(*begin)) {
    ++begin;
  }
  while (end != begin && isASCIIWhitespace(*(end - 1))) {
    --end;
  }

  llvh::StringRef trimmed(begin, end - begin);
  if (trimmed.equals_lower("utf-8") || trimmed.equals_lower("utf8") ||
      trimmed.equals_lower("unicode-1-1-utf-8")) {
    return TextDecoderEncoding::UTF8;
  }

  if (trimmed.equals_lower("utf-16le") || trimmed.equals_lower("utf-16")) {
    return TextDecoderEncoding::UTF16LE;
  }

  if (trimmed.equals_lower("utf-16be") || trimmed.equals_lower("unicodefffe")) {
    return TextDecoderEncoding::UTF16BE;
  }

  if (trimmed.equals_lower("866") || trimmed.equals_lower("cp866") ||
      trimmed.equals_lower("csibm866") || trimmed.equals_lower("ibm866")) {
    return TextDecoderEncoding::IBM866;
  }

  if (trimmed.equals_lower("csisolatin2") || trimmed.equals_lower("iso-8859-2") ||
      trimmed.equals_lower("iso-ir-101") || trimmed.equals_lower("iso8859-2") ||
      trimmed.equals_lower("iso88592") || trimmed.equals_lower("iso_8859-2") ||
      trimmed.equals_lower("iso_8859-2:1987") || trimmed.equals_lower("l2") ||
      trimmed.equals_lower("latin2")) {
    return TextDecoderEncoding::ISO_8859_2;
  }

  if (trimmed.equals_lower("csisolatin3") || trimmed.equals_lower("iso-8859-3") ||
      trimmed.equals_lower("iso-ir-109") || trimmed.equals_lower("iso8859-3") ||
      trimmed.equals_lower("iso88593") || trimmed.equals_lower("iso_8859-3") ||
      trimmed.equals_lower("iso_8859-3:1988") || trimmed.equals_lower("l3") ||
      trimmed.equals_lower("latin3")) {
    return TextDecoderEncoding::ISO_8859_3;
  }

  if (trimmed.equals_lower("csisolatin4") || trimmed.equals_lower("iso-8859-4") ||
      trimmed.equals_lower("iso-ir-110") || trimmed.equals_lower("iso8859-4") ||
      trimmed.equals_lower("iso88594") || trimmed.equals_lower("iso_8859-4") ||
      trimmed.equals_lower("iso_8859-4:1988") || trimmed.equals_lower("l4") ||
      trimmed.equals_lower("latin4")) {
    return TextDecoderEncoding::ISO_8859_4;
  }

  if (trimmed.equals_lower("csisolatincyrillic") || trimmed.equals_lower("cyrillic") ||
      trimmed.equals_lower("iso-8859-5") || trimmed.equals_lower("iso-ir-144") ||
      trimmed.equals_lower("iso8859-5") || trimmed.equals_lower("iso88595") ||
      trimmed.equals_lower("iso_8859-5") || trimmed.equals_lower("iso_8859-5:1988")) {
    return TextDecoderEncoding::ISO_8859_5;
  }

  if (trimmed.equals_lower("arabic") || trimmed.equals_lower("asmo-708") ||
      trimmed.equals_lower("csiso88596e") || trimmed.equals_lower("csiso88596i") ||
      trimmed.equals_lower("csisolatinarabic") || trimmed.equals_lower("ecma-114") ||
      trimmed.equals_lower("iso-8859-6") || trimmed.equals_lower("iso-8859-6-e") ||
      trimmed.equals_lower("iso-8859-6-i") || trimmed.equals_lower("iso-ir-127") ||
      trimmed.equals_lower("iso8859-6") || trimmed.equals_lower("iso88596") ||
      trimmed.equals_lower("iso_8859-6") || trimmed.equals_lower("iso_8859-6:1987")) {
    return TextDecoderEncoding::ISO_8859_6;
  }

  if (trimmed.equals_lower("csisolatingreek") || trimmed.equals_lower("ecma-118") ||
      trimmed.equals_lower("elot_928") || trimmed.equals_lower("greek") ||
      trimmed.equals_lower("greek8") || trimmed.equals_lower("iso-8859-7") ||
      trimmed.equals_lower("iso-ir-126") || trimmed.equals_lower("iso8859-7") ||
      trimmed.equals_lower("iso88597") || trimmed.equals_lower("iso_8859-7") ||
      trimmed.equals_lower("iso_8859-7:1987") || trimmed.equals_lower("sun_eu_greek")) {
    return TextDecoderEncoding::ISO_8859_7;
  }

  if (trimmed.equals_lower("csiso88598e") || trimmed.equals_lower("csisolatinhebrew") ||
      trimmed.equals_lower("hebrew") || trimmed.equals_lower("iso-8859-8") ||
      trimmed.equals_lower("iso-8859-8-e") || trimmed.equals_lower("iso-ir-138") ||
      trimmed.equals_lower("iso8859-8") || trimmed.equals_lower("iso88598") ||
      trimmed.equals_lower("iso_8859-8") || trimmed.equals_lower("iso_8859-8:1988") ||
      trimmed.equals_lower("visual")) {
    return TextDecoderEncoding::ISO_8859_8;
  }

  if (trimmed.equals_lower("csiso88598i") || trimmed.equals_lower("iso-8859-8-i") ||
      trimmed.equals_lower("logical")) {
    return TextDecoderEncoding::ISO_8859_8_I;
  }

  if (trimmed.equals_lower("csisolatin6") || trimmed.equals_lower("iso-8859-10") ||
      trimmed.equals_lower("iso-ir-157") || trimmed.equals_lower("iso8859-10") ||
      trimmed.equals_lower("iso885910") || trimmed.equals_lower("l6") ||
      trimmed.equals_lower("latin6")) {
    return TextDecoderEncoding::ISO_8859_10;
  }

  if (trimmed.equals_lower("iso-8859-13") || trimmed.equals_lower("iso8859-13") ||
      trimmed.equals_lower("iso885913")) {
    return TextDecoderEncoding::ISO_8859_13;
  }

  if (trimmed.equals_lower("iso-8859-14") || trimmed.equals_lower("iso8859-14") ||
      trimmed.equals_lower("iso885914")) {
    return TextDecoderEncoding::ISO_8859_14;
  }

  if (trimmed.equals_lower("csisolatin9") || trimmed.equals_lower("iso-8859-15") ||
      trimmed.equals_lower("iso8859-15") || trimmed.equals_lower("iso885915") ||
      trimmed.equals_lower("iso_8859-15") || trimmed.equals_lower("l9")) {
    return TextDecoderEncoding::ISO_8859_15;
  }

  if (trimmed.equals_lower("iso-8859-16")) {
    return TextDecoderEncoding::ISO_8859_16;
  }

  if (trimmed.equals_lower("cskoi8r") || trimmed.equals_lower("koi") ||
      trimmed.equals_lower("koi8") || trimmed.equals_lower("koi8-r") ||
      trimmed.equals_lower("koi8_r")) {
    return TextDecoderEncoding::KOI8_R;
  }

  if (trimmed.equals_lower("koi8-ru") || trimmed.equals_lower("koi8-u")) {
    return TextDecoderEncoding::KOI8_U;
  }

  if (trimmed.equals_lower("csmacintosh") || trimmed.equals_lower("mac") ||
      trimmed.equals_lower("macintosh") || trimmed.equals_lower("x-mac-roman")) {
    return TextDecoderEncoding::Macintosh;
  }

  if (trimmed.equals_lower("dos-874") || trimmed.equals_lower("iso-8859-11") ||
      trimmed.equals_lower("iso8859-11") || trimmed.equals_lower("iso885911") ||
      trimmed.equals_lower("tis-620") || trimmed.equals_lower("windows-874")) {
    return TextDecoderEncoding::Windows874;
  }

  if (trimmed.equals_lower("cp1250") || trimmed.equals_lower("windows-1250") ||
      trimmed.equals_lower("x-cp1250")) {
    return TextDecoderEncoding::Windows1250;
  }

  if (trimmed.equals_lower("cp1251") || trimmed.equals_lower("windows-1251") ||
      trimmed.equals_lower("x-cp1251")) {
    return TextDecoderEncoding::Windows1251;
  }

  if (trimmed.equals_lower("ansi_x3.4-1968") || trimmed.equals_lower("ascii") ||
      trimmed.equals_lower("cp1252") || trimmed.equals_lower("cp819") ||
      trimmed.equals_lower("csisolatin1") || trimmed.equals_lower("ibm819") ||
      trimmed.equals_lower("iso-8859-1") || trimmed.equals_lower("iso-ir-100") ||
      trimmed.equals_lower("iso8859-1") || trimmed.equals_lower("iso88591") ||
      trimmed.equals_lower("iso_8859-1") || trimmed.equals_lower("iso_8859-1:1987") ||
      trimmed.equals_lower("l1") || trimmed.equals_lower("latin1") ||
      trimmed.equals_lower("us-ascii") || trimmed.equals_lower("windows-1252") ||
      trimmed.equals_lower("x-cp1252")) {
    return TextDecoderEncoding::Windows1252;
  }

  if (trimmed.equals_lower("cp1253") || trimmed.equals_lower("windows-1253") ||
      trimmed.equals_lower("x-cp1253")) {
    return TextDecoderEncoding::Windows1253;
  }

  if (trimmed.equals_lower("cp1254") || trimmed.equals_lower("csisolatin5") ||
      trimmed.equals_lower("iso-8859-9") || trimmed.equals_lower("iso-ir-148") ||
      trimmed.equals_lower("iso8859-9") || trimmed.equals_lower("iso88599") ||
      trimmed.equals_lower("iso_8859-9") || trimmed.equals_lower("iso_8859-9:1989") ||
      trimmed.equals_lower("l5") || trimmed.equals_lower("latin5") ||
      trimmed.equals_lower("windows-1254") || trimmed.equals_lower("x-cp1254")) {
    return TextDecoderEncoding::Windows1254;
  }

  if (trimmed.equals_lower("cp1255") || trimmed.equals_lower("windows-1255") ||
      trimmed.equals_lower("x-cp1255")) {
    return TextDecoderEncoding::Windows1255;
  }

  if (trimmed.equals_lower("cp1256") || trimmed.equals_lower("windows-1256") ||
      trimmed.equals_lower("x-cp1256")) {
    return TextDecoderEncoding::Windows1256;
  }

  if (trimmed.equals_lower("cp1257") || trimmed.equals_lower("windows-1257") ||
      trimmed.equals_lower("x-cp1257")) {
    return TextDecoderEncoding::Windows1257;
  }

  if (trimmed.equals_lower("cp1258") || trimmed.equals_lower("windows-1258") ||
      trimmed.equals_lower("x-cp1258")) {
    return TextDecoderEncoding::Windows1258;
  }

  if (trimmed.equals_lower("x-mac-cyrillic") ||
      trimmed.equals_lower("x-mac-ukrainian")) {
    return TextDecoderEncoding::XMacCyrillic;
  }

  return llvh::None;
}

// Get the canonical encoding name for the given encoding type.
static Predefined::Str getEncodingName(TextDecoderEncoding encoding) {
  switch (encoding) {
    case TextDecoderEncoding::UTF8:
      return Predefined::utf8;
    case TextDecoderEncoding::UTF16LE:
      return Predefined::utf16le;
    case TextDecoderEncoding::UTF16BE:
      return Predefined::utf16be;
    case TextDecoderEncoding::IBM866:
      return Predefined::ibm866;
    case TextDecoderEncoding::ISO_8859_2:
      return Predefined::iso88592;
    case TextDecoderEncoding::ISO_8859_3:
      return Predefined::iso88593;
    case TextDecoderEncoding::ISO_8859_4:
      return Predefined::iso88594;
    case TextDecoderEncoding::ISO_8859_5:
      return Predefined::iso88595;
    case TextDecoderEncoding::ISO_8859_6:
      return Predefined::iso88596;
    case TextDecoderEncoding::ISO_8859_7:
      return Predefined::iso88597;
    case TextDecoderEncoding::ISO_8859_8:
      return Predefined::iso88598;
    case TextDecoderEncoding::ISO_8859_8_I:
      return Predefined::iso88598i;
    case TextDecoderEncoding::ISO_8859_10:
      return Predefined::iso885910;
    case TextDecoderEncoding::ISO_8859_13:
      return Predefined::iso885913;
    case TextDecoderEncoding::ISO_8859_14:
      return Predefined::iso885914;
    case TextDecoderEncoding::ISO_8859_15:
      return Predefined::iso885915;
    case TextDecoderEncoding::ISO_8859_16:
      return Predefined::iso885916;
    case TextDecoderEncoding::KOI8_R:
      return Predefined::koi8r;
    case TextDecoderEncoding::KOI8_U:
      return Predefined::koi8u;
    case TextDecoderEncoding::Macintosh:
      return Predefined::macintosh;
    case TextDecoderEncoding::Windows874:
      return Predefined::windows874;
    case TextDecoderEncoding::Windows1250:
      return Predefined::windows1250;
    case TextDecoderEncoding::Windows1251:
      return Predefined::windows1251;
    case TextDecoderEncoding::Windows1252:
      return Predefined::windows1252;
    case TextDecoderEncoding::Windows1253:
      return Predefined::windows1253;
    case TextDecoderEncoding::Windows1254:
      return Predefined::windows1254;
    case TextDecoderEncoding::Windows1255:
      return Predefined::windows1255;
    case TextDecoderEncoding::Windows1256:
      return Predefined::windows1256;
    case TextDecoderEncoding::Windows1257:
      return Predefined::windows1257;
    case TextDecoderEncoding::Windows1258:
      return Predefined::windows1258;
    case TextDecoderEncoding::XMacCyrillic:
      return Predefined::xmaccyrillic;
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



static DecodeError decodeUTF8(
    const uint8_t *bytes,
    size_t length,
    bool fatal,
    bool ignoreBOM,
    bool stream,
    bool bomSeen,
    std::u16string *decoded,
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
  }

  // Check for incomplete sequence at end. Only process bytes that form complete sequences.
  size_t processLength = length;
  if (length > 0) {
    // Find potential incomplete sequence at end (up to 3 bytes for 4-byte seq)
    for (size_t tailLen = std::min(length, size_t(3)); tailLen > 0; --tailLen) {
      size_t tailIndex = length - tailLen;
      if (isValidPartialUTF8(bytes + tailIndex, tailLen)) {
        processLength = tailIndex;
        break;
      }
    }
  }

  decoded->reserve(processLength);

  // Mark BOM as seen once we actually process bytes (not just buffer them).
  if (!*outBOMSeen && processLength > 0) {
    *outBOMSeen = true;
  }

  const llvh::UTF8 *src = bytes;
  const llvh::UTF8 *srcEnd = bytes + processLength;

  while (src < srcEnd) {
    llvh::UTF16 buf[256];
    llvh::UTF16 *dst = buf;
    llvh::ConversionResult res = llvh::ConvertUTF8toUTF16(
        &src, srcEnd, &dst, buf + 256, llvh::lenientConversion);

    decoded->append(
        reinterpret_cast<char16_t *>(buf), reinterpret_cast<char16_t *>(dst));

    if (res == llvh::conversionOK) {
      break;
    } else if (res == llvh::sourceIllegal || res == llvh::sourceExhausted) {
      if (fatal) {
        return DecodeError::InvalidSequence;
      }
      // Consume the maximal subpart of the ill-formed sequence.
      decoded->push_back(UNICODE_REPLACEMENT_CHARACTER);
      src += maximalSubpartLength(src, srcEnd - src);
    }
  }

  // Store pending bytes if streaming; else emit replacement char.
  if (stream && processLength < length) {
    *outPendingCount = length - processLength;
    for (size_t i = 0; i < *outPendingCount; ++i) {
      outPendingBytes[i] = bytes[processLength + i];
    }
  }
  if (!stream && processLength < length) {
    if (fatal) {
      return DecodeError::InvalidSequence;
    }
    decoded->push_back(UNICODE_REPLACEMENT_CHARACTER);
  }

  return DecodeError::None;
}

static DecodeError decodeUTF16(
    const uint8_t *bytes,
    size_t length,
    bool fatal,
    bool ignoreBOM,
    bool bigEndian,
    bool stream,
    bool bomSeen,
    std::u16string *decoded,
    uint8_t outPendingBytes[4],
    size_t *outPendingCount,
    bool *outBOMSeen) {
  *outPendingCount = 0;
  *outBOMSeen = bomSeen;

  bool hasTrailingByte = length % 2 != 0;
  size_t evenLength = hasTrailingByte ? length - 1 : length;

  if (hasTrailingByte && !stream && fatal) {
    return DecodeError::OddByteCount;
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

  decoded->reserve((end - start) / 2 + 1);

  const uint8_t *p = start;
  while (p < end) {
    char16_t cu = readU16(p);

    if (isHighSurrogate(cu)) {
      if (p + 4 <= end) {
        char16_t next = readU16(p + 2);
        if (isLowSurrogate(next)) {
          decoded->push_back(cu);
          decoded->push_back(next);
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
        return DecodeError::InvalidSurrogate;
      }
      decoded->push_back(UNICODE_REPLACEMENT_CHARACTER);
      p += 2;
    } else if (isLowSurrogate(cu)) {
      if (fatal) {
        return DecodeError::InvalidSurrogate;
      }
      decoded->push_back(UNICODE_REPLACEMENT_CHARACTER);
      p += 2;
    } else {
      decoded->push_back(cu);
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
      decoded->push_back(UNICODE_REPLACEMENT_CHARACTER);
    }
  }

  return DecodeError::None;
}

// Single-byte encoding tables. Each table maps bytes 0x80-0xFF to Unicode.
// Source: https://encoding.spec.whatwg.org/#legacy-single-byte-encodings
static constexpr char16_t kIBM866[128] = {
  0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
  0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
  0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
  0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556, 0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
  0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F, 0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
  0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B, 0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
  0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F,
  0x0401, 0x0451, 0x0404, 0x0454, 0x0407, 0x0457, 0x040E, 0x045E, 0x00B0, 0x2219, 0x00B7, 0x221A, 0x2116, 0x00A4, 0x25A0, 0x00A0,
};

static constexpr char16_t kISO_8859_2[128] = {
  0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
  0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
  0x00A0, 0x0104, 0x02D8, 0x0141, 0x00A4, 0x013D, 0x015A, 0x00A7, 0x00A8, 0x0160, 0x015E, 0x0164, 0x0179, 0x00AD, 0x017D, 0x017B,
  0x00B0, 0x0105, 0x02DB, 0x0142, 0x00B4, 0x013E, 0x015B, 0x02C7, 0x00B8, 0x0161, 0x015F, 0x0165, 0x017A, 0x02DD, 0x017E, 0x017C,
  0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7, 0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
  0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7, 0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
  0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7, 0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
  0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7, 0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9,
};

static constexpr char16_t kISO_8859_3[128] = {
  0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
  0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
  0x00A0, 0x0126, 0x02D8, 0x00A3, 0x00A4, 0xFFFD, 0x0124, 0x00A7, 0x00A8, 0x0130, 0x015E, 0x011E, 0x0134, 0x00AD, 0xFFFD, 0x017B,
  0x00B0, 0x0127, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x0125, 0x00B7, 0x00B8, 0x0131, 0x015F, 0x011F, 0x0135, 0x00BD, 0xFFFD, 0x017C,
  0x00C0, 0x00C1, 0x00C2, 0xFFFD, 0x00C4, 0x010A, 0x0108, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
  0xFFFD, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x0120, 0x00D6, 0x00D7, 0x011C, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x016C, 0x015C, 0x00DF,
  0x00E0, 0x00E1, 0x00E2, 0xFFFD, 0x00E4, 0x010B, 0x0109, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
  0xFFFD, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x0121, 0x00F6, 0x00F7, 0x011D, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x016D, 0x015D, 0x02D9,
};

static constexpr char16_t kISO_8859_4[128] = {
  0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
  0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
  0x00A0, 0x0104, 0x0138, 0x0156, 0x00A4, 0x0128, 0x013B, 0x00A7, 0x00A8, 0x0160, 0x0112, 0x0122, 0x0166, 0x00AD, 0x017D, 0x00AF,
  0x00B0, 0x0105, 0x02DB, 0x0157, 0x00B4, 0x0129, 0x013C, 0x02C7, 0x00B8, 0x0161, 0x0113, 0x0123, 0x0167, 0x014A, 0x017E, 0x014B,
  0x0100, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x012E, 0x010C, 0x00C9, 0x0118, 0x00CB, 0x0116, 0x00CD, 0x00CE, 0x012A,
  0x0110, 0x0145, 0x014C, 0x0136, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x0172, 0x00DA, 0x00DB, 0x00DC, 0x0168, 0x016A, 0x00DF,
  0x0101, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x012F, 0x010D, 0x00E9, 0x0119, 0x00EB, 0x0117, 0x00ED, 0x00EE, 0x012B,
  0x0111, 0x0146, 0x014D, 0x0137, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x0173, 0x00FA, 0x00FB, 0x00FC, 0x0169, 0x016B, 0x02D9,
};

static constexpr char16_t kISO_8859_5[128] = {
  0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
  0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
  0x00A0, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407, 0x0408, 0x0409, 0x040A, 0x040B, 0x040C, 0x00AD, 0x040E, 0x040F,
  0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
  0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
  0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
  0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F,
  0x2116, 0x0451, 0x0452, 0x0453, 0x0454, 0x0455, 0x0456, 0x0457, 0x0458, 0x0459, 0x045A, 0x045B, 0x045C, 0x00A7, 0x045E, 0x045F,
};

static constexpr char16_t kISO_8859_6[128] = {
  0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
  0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
  0x00A0, 0xFFFD, 0xFFFD, 0xFFFD, 0x00A4, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x060C, 0x00AD, 0xFFFD, 0xFFFD,
  0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x061B, 0xFFFD, 0xFFFD, 0xFFFD, 0x061F,
  0xFFFD, 0x0621, 0x0622, 0x0623, 0x0624, 0x0625, 0x0626, 0x0627, 0x0628, 0x0629, 0x062A, 0x062B, 0x062C, 0x062D, 0x062E, 0x062F,
  0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x0637, 0x0638, 0x0639, 0x063A, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
  0x0640, 0x0641, 0x0642, 0x0643, 0x0644, 0x0645, 0x0646, 0x0647, 0x0648, 0x0649, 0x064A, 0x064B, 0x064C, 0x064D, 0x064E, 0x064F,
  0x0650, 0x0651, 0x0652, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
};

static constexpr char16_t kISO_8859_7[128] = {
  0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
  0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
  0x00A0, 0x2018, 0x2019, 0x00A3, 0x20AC, 0x20AF, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x037A, 0x00AB, 0x00AC, 0x00AD, 0xFFFD, 0x2015,
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x0384, 0x0385, 0x0386, 0x00B7, 0x0388, 0x0389, 0x038A, 0x00BB, 0x038C, 0x00BD, 0x038E, 0x038F,
  0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397, 0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F,
  0x03A0, 0x03A1, 0xFFFD, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7, 0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x03AC, 0x03AD, 0x03AE, 0x03AF,
  0x03B0, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7, 0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF,
  0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7, 0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03CC, 0x03CD, 0x03CE, 0xFFFD,
};

static constexpr char16_t kISO_8859_8[128] = {
  0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
  0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
  0x00A0, 0xFFFD, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00D7, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00F7, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0xFFFD,
  0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
  0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x2017,
  0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7, 0x05D8, 0x05D9, 0x05DA, 0x05DB, 0x05DC, 0x05DD, 0x05DE, 0x05DF,
  0x05E0, 0x05E1, 0x05E2, 0x05E3, 0x05E4, 0x05E5, 0x05E6, 0x05E7, 0x05E8, 0x05E9, 0x05EA, 0xFFFD, 0xFFFD, 0x200E, 0x200F, 0xFFFD,
};

static constexpr char16_t kISO_8859_10[128] = {
  0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
  0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
  0x00A0, 0x0104, 0x0112, 0x0122, 0x012A, 0x0128, 0x0136, 0x00A7, 0x013B, 0x0110, 0x0160, 0x0166, 0x017D, 0x00AD, 0x016A, 0x014A,
  0x00B0, 0x0105, 0x0113, 0x0123, 0x012B, 0x0129, 0x0137, 0x00B7, 0x013C, 0x0111, 0x0161, 0x0167, 0x017E, 0x2015, 0x016B, 0x014B,
  0x0100, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x012E, 0x010C, 0x00C9, 0x0118, 0x00CB, 0x0116, 0x00CD, 0x00CE, 0x00CF,
  0x00D0, 0x0145, 0x014C, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x0168, 0x00D8, 0x0172, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
  0x0101, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x012F, 0x010D, 0x00E9, 0x0119, 0x00EB, 0x0117, 0x00ED, 0x00EE, 0x00EF,
  0x00F0, 0x0146, 0x014D, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x0169, 0x00F8, 0x0173, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x0138,
};

static constexpr char16_t kISO_8859_13[128] = {
  0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
  0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
  0x00A0, 0x201D, 0x00A2, 0x00A3, 0x00A4, 0x201E, 0x00A6, 0x00A7, 0x00D8, 0x00A9, 0x0156, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00C6,
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x201C, 0x00B5, 0x00B6, 0x00B7, 0x00F8, 0x00B9, 0x0157, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00E6,
  0x0104, 0x012E, 0x0100, 0x0106, 0x00C4, 0x00C5, 0x0118, 0x0112, 0x010C, 0x00C9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012A, 0x013B,
  0x0160, 0x0143, 0x0145, 0x00D3, 0x014C, 0x00D5, 0x00D6, 0x00D7, 0x0172, 0x0141, 0x015A, 0x016A, 0x00DC, 0x017B, 0x017D, 0x00DF,
  0x0105, 0x012F, 0x0101, 0x0107, 0x00E4, 0x00E5, 0x0119, 0x0113, 0x010D, 0x00E9, 0x017A, 0x0117, 0x0123, 0x0137, 0x012B, 0x013C,
  0x0161, 0x0144, 0x0146, 0x00F3, 0x014D, 0x00F5, 0x00F6, 0x00F7, 0x0173, 0x0142, 0x015B, 0x016B, 0x00FC, 0x017C, 0x017E, 0x2019,
};

static constexpr char16_t kISO_8859_14[128] = {
  0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
  0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
  0x00A0, 0x1E02, 0x1E03, 0x00A3, 0x010A, 0x010B, 0x1E0A, 0x00A7, 0x1E80, 0x00A9, 0x1E82, 0x1E0B, 0x1EF2, 0x00AD, 0x00AE, 0x0178,
  0x1E1E, 0x1E1F, 0x0120, 0x0121, 0x1E40, 0x1E41, 0x00B6, 0x1E56, 0x1E81, 0x1E57, 0x1E83, 0x1E60, 0x1EF3, 0x1E84, 0x1E85, 0x1E61,
  0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
  0x0174, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x1E6A, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x0176, 0x00DF,
  0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
  0x0175, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x1E6B, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x0177, 0x00FF,
};

static constexpr char16_t kISO_8859_15[128] = {
  0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
  0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
  0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x20AC, 0x00A5, 0x0160, 0x00A7, 0x0161, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x017D, 0x00B5, 0x00B6, 0x00B7, 0x017E, 0x00B9, 0x00BA, 0x00BB, 0x0152, 0x0153, 0x0178, 0x00BF,
  0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
  0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
  0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
  0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF,
};

static constexpr char16_t kISO_8859_16[128] = {
  0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
  0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
  0x00A0, 0x0104, 0x0105, 0x0141, 0x20AC, 0x201E, 0x0160, 0x00A7, 0x0161, 0x00A9, 0x0218, 0x00AB, 0x0179, 0x00AD, 0x017A, 0x017B,
  0x00B0, 0x00B1, 0x010C, 0x0142, 0x017D, 0x201D, 0x00B6, 0x00B7, 0x017E, 0x010D, 0x0219, 0x00BB, 0x0152, 0x0153, 0x0178, 0x017C,
  0x00C0, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0106, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
  0x0110, 0x0143, 0x00D2, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x015A, 0x0170, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0118, 0x021A, 0x00DF,
  0x00E0, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x0107, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
  0x0111, 0x0144, 0x00F2, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x015B, 0x0171, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x0119, 0x021B, 0x00FF,
};

static constexpr char16_t kKOI8_R[128] = {
  0x2500, 0x2502, 0x250C, 0x2510, 0x2514, 0x2518, 0x251C, 0x2524, 0x252C, 0x2534, 0x253C, 0x2580, 0x2584, 0x2588, 0x258C, 0x2590,
  0x2591, 0x2592, 0x2593, 0x2320, 0x25A0, 0x2219, 0x221A, 0x2248, 0x2264, 0x2265, 0x00A0, 0x2321, 0x00B0, 0x00B2, 0x00B7, 0x00F7,
  0x2550, 0x2551, 0x2552, 0x0451, 0x2553, 0x2554, 0x2555, 0x2556, 0x2557, 0x2558, 0x2559, 0x255A, 0x255B, 0x255C, 0x255D, 0x255E,
  0x255F, 0x2560, 0x2561, 0x0401, 0x2562, 0x2563, 0x2564, 0x2565, 0x2566, 0x2567, 0x2568, 0x2569, 0x256A, 0x256B, 0x256C, 0x00A9,
  0x044E, 0x0430, 0x0431, 0x0446, 0x0434, 0x0435, 0x0444, 0x0433, 0x0445, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E,
  0x043F, 0x044F, 0x0440, 0x0441, 0x0442, 0x0443, 0x0436, 0x0432, 0x044C, 0x044B, 0x0437, 0x0448, 0x044D, 0x0449, 0x0447, 0x044A,
  0x042E, 0x0410, 0x0411, 0x0426, 0x0414, 0x0415, 0x0424, 0x0413, 0x0425, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E,
  0x041F, 0x042F, 0x0420, 0x0421, 0x0422, 0x0423, 0x0416, 0x0412, 0x042C, 0x042B, 0x0417, 0x0428, 0x042D, 0x0429, 0x0427, 0x042A,
};

static constexpr char16_t kKOI8_U[128] = {
  0x2500, 0x2502, 0x250C, 0x2510, 0x2514, 0x2518, 0x251C, 0x2524, 0x252C, 0x2534, 0x253C, 0x2580, 0x2584, 0x2588, 0x258C, 0x2590,
  0x2591, 0x2592, 0x2593, 0x2320, 0x25A0, 0x2219, 0x221A, 0x2248, 0x2264, 0x2265, 0x00A0, 0x2321, 0x00B0, 0x00B2, 0x00B7, 0x00F7,
  0x2550, 0x2551, 0x2552, 0x0451, 0x0454, 0x2554, 0x0456, 0x0457, 0x2557, 0x2558, 0x2559, 0x255A, 0x255B, 0x0491, 0x045E, 0x255E,
  0x255F, 0x2560, 0x2561, 0x0401, 0x0404, 0x2563, 0x0406, 0x0407, 0x2566, 0x2567, 0x2568, 0x2569, 0x256A, 0x0490, 0x040E, 0x00A9,
  0x044E, 0x0430, 0x0431, 0x0446, 0x0434, 0x0435, 0x0444, 0x0433, 0x0445, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E,
  0x043F, 0x044F, 0x0440, 0x0441, 0x0442, 0x0443, 0x0436, 0x0432, 0x044C, 0x044B, 0x0437, 0x0448, 0x044D, 0x0449, 0x0447, 0x044A,
  0x042E, 0x0410, 0x0411, 0x0426, 0x0414, 0x0415, 0x0424, 0x0413, 0x0425, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E,
  0x041F, 0x042F, 0x0420, 0x0421, 0x0422, 0x0423, 0x0416, 0x0412, 0x042C, 0x042B, 0x0417, 0x0428, 0x042D, 0x0429, 0x0427, 0x042A,
};

static constexpr char16_t kMacintosh[128] = {
  0x00C4, 0x00C5, 0x00C7, 0x00C9, 0x00D1, 0x00D6, 0x00DC, 0x00E1, 0x00E0, 0x00E2, 0x00E4, 0x00E3, 0x00E5, 0x00E7, 0x00E9, 0x00E8,
  0x00EA, 0x00EB, 0x00ED, 0x00EC, 0x00EE, 0x00EF, 0x00F1, 0x00F3, 0x00F2, 0x00F4, 0x00F6, 0x00F5, 0x00FA, 0x00F9, 0x00FB, 0x00FC,
  0x2020, 0x00B0, 0x00A2, 0x00A3, 0x00A7, 0x2022, 0x00B6, 0x00DF, 0x00AE, 0x00A9, 0x2122, 0x00B4, 0x00A8, 0x2260, 0x00C6, 0x00D8,
  0x221E, 0x00B1, 0x2264, 0x2265, 0x00A5, 0x00B5, 0x2202, 0x2211, 0x220F, 0x03C0, 0x222B, 0x00AA, 0x00BA, 0x03A9, 0x00E6, 0x00F8,
  0x00BF, 0x00A1, 0x00AC, 0x221A, 0x0192, 0x2248, 0x2206, 0x00AB, 0x00BB, 0x2026, 0x00A0, 0x00C0, 0x00C3, 0x00D5, 0x0152, 0x0153,
  0x2013, 0x2014, 0x201C, 0x201D, 0x2018, 0x2019, 0x00F7, 0x25CA, 0x00FF, 0x0178, 0x2044, 0x20AC, 0x2039, 0x203A, 0xFB01, 0xFB02,
  0x2021, 0x00B7, 0x201A, 0x201E, 0x2030, 0x00C2, 0x00CA, 0x00C1, 0x00CB, 0x00C8, 0x00CD, 0x00CE, 0x00CF, 0x00CC, 0x00D3, 0x00D4,
  0xF8FF, 0x00D2, 0x00DA, 0x00DB, 0x00D9, 0x0131, 0x02C6, 0x02DC, 0x00AF, 0x02D8, 0x02D9, 0x02DA, 0x00B8, 0x02DD, 0x02DB, 0x02C7,
};

static constexpr char16_t kWindows874[128] = {
  0x20AC, 0x0081, 0x0082, 0x0083, 0x0084, 0x2026, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
  0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
  0x00A0, 0x0E01, 0x0E02, 0x0E03, 0x0E04, 0x0E05, 0x0E06, 0x0E07, 0x0E08, 0x0E09, 0x0E0A, 0x0E0B, 0x0E0C, 0x0E0D, 0x0E0E, 0x0E0F,
  0x0E10, 0x0E11, 0x0E12, 0x0E13, 0x0E14, 0x0E15, 0x0E16, 0x0E17, 0x0E18, 0x0E19, 0x0E1A, 0x0E1B, 0x0E1C, 0x0E1D, 0x0E1E, 0x0E1F,
  0x0E20, 0x0E21, 0x0E22, 0x0E23, 0x0E24, 0x0E25, 0x0E26, 0x0E27, 0x0E28, 0x0E29, 0x0E2A, 0x0E2B, 0x0E2C, 0x0E2D, 0x0E2E, 0x0E2F,
  0x0E30, 0x0E31, 0x0E32, 0x0E33, 0x0E34, 0x0E35, 0x0E36, 0x0E37, 0x0E38, 0x0E39, 0x0E3A, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x0E3F,
  0x0E40, 0x0E41, 0x0E42, 0x0E43, 0x0E44, 0x0E45, 0x0E46, 0x0E47, 0x0E48, 0x0E49, 0x0E4A, 0x0E4B, 0x0E4C, 0x0E4D, 0x0E4E, 0x0E4F,
  0x0E50, 0x0E51, 0x0E52, 0x0E53, 0x0E54, 0x0E55, 0x0E56, 0x0E57, 0x0E58, 0x0E59, 0x0E5A, 0x0E5B, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
};

static constexpr char16_t kWindows1250[128] = {
  0x20AC, 0x0081, 0x201A, 0x0083, 0x201E, 0x2026, 0x2020, 0x2021, 0x0088, 0x2030, 0x0160, 0x2039, 0x015A, 0x0164, 0x017D, 0x0179,
  0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x0098, 0x2122, 0x0161, 0x203A, 0x015B, 0x0165, 0x017E, 0x017A,
  0x00A0, 0x02C7, 0x02D8, 0x0141, 0x00A4, 0x0104, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x015E, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x017B,
  0x00B0, 0x00B1, 0x02DB, 0x0142, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x0105, 0x015F, 0x00BB, 0x013D, 0x02DD, 0x013E, 0x017C,
  0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7, 0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
  0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7, 0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
  0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7, 0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
  0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7, 0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9,
};

static constexpr char16_t kWindows1251[128] = {
  0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021, 0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F,
  0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x0098, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F,
  0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7, 0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407,
  0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7, 0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455, 0x0457,
  0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
  0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
  0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
  0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F,
};

static constexpr char16_t kWindows1252[128] = {
  0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
  0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178,
  0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
  0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
  0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
  0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
  0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF,
};

static constexpr char16_t kWindows1253[128] = {
  0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x0088, 0x2030, 0x008A, 0x2039, 0x008C, 0x008D, 0x008E, 0x008F,
  0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x0098, 0x2122, 0x009A, 0x203A, 0x009C, 0x009D, 0x009E, 0x009F,
  0x00A0, 0x0385, 0x0386, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0xFFFD, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x2015,
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x0384, 0x00B5, 0x00B6, 0x00B7, 0x0388, 0x0389, 0x038A, 0x00BB, 0x038C, 0x00BD, 0x038E, 0x038F,
  0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397, 0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F,
  0x03A0, 0x03A1, 0xFFFD, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7, 0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x03AC, 0x03AD, 0x03AE, 0x03AF,
  0x03B0, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7, 0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF,
  0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7, 0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03CC, 0x03CD, 0x03CE, 0xFFFD,
};

static constexpr char16_t kWindows1254[128] = {
  0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x008E, 0x008F,
  0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x009E, 0x0178,
  0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
  0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
  0x011E, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0130, 0x015E, 0x00DF,
  0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
  0x011F, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x0131, 0x015F, 0x00FF,
};

static constexpr char16_t kWindows1255[128] = {
  0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x008A, 0x2039, 0x008C, 0x008D, 0x008E, 0x008F,
  0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x009A, 0x203A, 0x009C, 0x009D, 0x009E, 0x009F,
  0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x20AA, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00D7, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00F7, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
  0x05B0, 0x05B1, 0x05B2, 0x05B3, 0x05B4, 0x05B5, 0x05B6, 0x05B7, 0x05B8, 0x05B9, 0x05BA, 0x05BB, 0x05BC, 0x05BD, 0x05BE, 0x05BF,
  0x05C0, 0x05C1, 0x05C2, 0x05C3, 0x05F0, 0x05F1, 0x05F2, 0x05F3, 0x05F4, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
  0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7, 0x05D8, 0x05D9, 0x05DA, 0x05DB, 0x05DC, 0x05DD, 0x05DE, 0x05DF,
  0x05E0, 0x05E1, 0x05E2, 0x05E3, 0x05E4, 0x05E5, 0x05E6, 0x05E7, 0x05E8, 0x05E9, 0x05EA, 0xFFFD, 0xFFFD, 0x200E, 0x200F, 0xFFFD,
};

static constexpr char16_t kWindows1256[128] = {
  0x20AC, 0x067E, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0679, 0x2039, 0x0152, 0x0686, 0x0698, 0x0688,
  0x06AF, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x06A9, 0x2122, 0x0691, 0x203A, 0x0153, 0x200C, 0x200D, 0x06BA,
  0x00A0, 0x060C, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x06BE, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x061B, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x061F,
  0x06C1, 0x0621, 0x0622, 0x0623, 0x0624, 0x0625, 0x0626, 0x0627, 0x0628, 0x0629, 0x062A, 0x062B, 0x062C, 0x062D, 0x062E, 0x062F,
  0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x00D7, 0x0637, 0x0638, 0x0639, 0x063A, 0x0640, 0x0641, 0x0642, 0x0643,
  0x00E0, 0x0644, 0x00E2, 0x0645, 0x0646, 0x0647, 0x0648, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x0649, 0x064A, 0x00EE, 0x00EF,
  0x064B, 0x064C, 0x064D, 0x064E, 0x00F4, 0x064F, 0x0650, 0x00F7, 0x0651, 0x00F9, 0x0652, 0x00FB, 0x00FC, 0x200E, 0x200F, 0x06D2,
};

static constexpr char16_t kWindows1257[128] = {
  0x20AC, 0x0081, 0x201A, 0x0083, 0x201E, 0x2026, 0x2020, 0x2021, 0x0088, 0x2030, 0x008A, 0x2039, 0x008C, 0x00A8, 0x02C7, 0x00B8,
  0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x0098, 0x2122, 0x009A, 0x203A, 0x009C, 0x00AF, 0x02DB, 0x009F,
  0x00A0, 0xFFFD, 0x00A2, 0x00A3, 0x00A4, 0xFFFD, 0x00A6, 0x00A7, 0x00D8, 0x00A9, 0x0156, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00C6,
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00F8, 0x00B9, 0x0157, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00E6,
  0x0104, 0x012E, 0x0100, 0x0106, 0x00C4, 0x00C5, 0x0118, 0x0112, 0x010C, 0x00C9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012A, 0x013B,
  0x0160, 0x0143, 0x0145, 0x00D3, 0x014C, 0x00D5, 0x00D6, 0x00D7, 0x0172, 0x0141, 0x015A, 0x016A, 0x00DC, 0x017B, 0x017D, 0x00DF,
  0x0105, 0x012F, 0x0101, 0x0107, 0x00E4, 0x00E5, 0x0119, 0x0113, 0x010D, 0x00E9, 0x017A, 0x0117, 0x0123, 0x0137, 0x012B, 0x013C,
  0x0161, 0x0144, 0x0146, 0x00F3, 0x014D, 0x00F5, 0x00F6, 0x00F7, 0x0173, 0x0142, 0x015B, 0x016B, 0x00FC, 0x017C, 0x017E, 0x02D9,
};

static constexpr char16_t kWindows1258[128] = {
  0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x008A, 0x2039, 0x0152, 0x008D, 0x008E, 0x008F,
  0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x009A, 0x203A, 0x0153, 0x009D, 0x009E, 0x0178,
  0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
  0x00C0, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x0300, 0x00CD, 0x00CE, 0x00CF,
  0x0110, 0x00D1, 0x0309, 0x00D3, 0x00D4, 0x01A0, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x01AF, 0x0303, 0x00DF,
  0x00E0, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x0301, 0x00ED, 0x00EE, 0x00EF,
  0x0111, 0x00F1, 0x0323, 0x00F3, 0x00F4, 0x01A1, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x01B0, 0x20AB, 0x00FF,
};

static constexpr char16_t kXMacCyrillic[128] = {
  0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
  0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
  0x2020, 0x00B0, 0x0490, 0x00A3, 0x00A7, 0x2022, 0x00B6, 0x0406, 0x00AE, 0x00A9, 0x2122, 0x0402, 0x0452, 0x2260, 0x0403, 0x0453,
  0x221E, 0x00B1, 0x2264, 0x2265, 0x0456, 0x00B5, 0x0491, 0x0408, 0x0404, 0x0454, 0x0407, 0x0457, 0x0409, 0x0459, 0x040A, 0x045A,
  0x0458, 0x0405, 0x00AC, 0x221A, 0x0192, 0x2248, 0x2206, 0x00AB, 0x00BB, 0x2026, 0x00A0, 0x040B, 0x045B, 0x040C, 0x045C, 0x0455,
  0x2013, 0x2014, 0x201C, 0x201D, 0x2018, 0x2019, 0x00F7, 0x201E, 0x040E, 0x045E, 0x040F, 0x045F, 0x2116, 0x0401, 0x0451, 0x044F,
  0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
  0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x20AC,
};

// Array of pointers to single-byte encoding tables, indexed by
// (TextDecoderEncoding - kFirstSingleByteEncoding).
static const char16_t *const kSingleByteEncodings[] = {
    kIBM866,       kISO_8859_2,   kISO_8859_3,   kISO_8859_4,  kISO_8859_5,
    kISO_8859_6,   kISO_8859_7,   kISO_8859_8,   kISO_8859_8,  kISO_8859_10,
    kISO_8859_13,  kISO_8859_14,  kISO_8859_15,  kISO_8859_16, kKOI8_R,
    kKOI8_U,       kMacintosh,    kWindows874,   kWindows1250, kWindows1251,
    kWindows1252,  kWindows1253,  kWindows1254,  kWindows1255, kWindows1256,
    kWindows1257,  kWindows1258,  kXMacCyrillic,
};

static DecodeError decodeSingleByteEncoding(
    const uint8_t *bytes,
    size_t length,
    const char16_t *table,
    bool fatal,
    std::u16string *decoded) {
  decoded->reserve(length);
  for (size_t i = 0; i < length; ++i) {
    uint8_t byte = bytes[i];
    if (byte < 0x80) {
      decoded->push_back(static_cast<char16_t>(byte));
    } else {
      char16_t cp = table[byte - 0x80];
      if (cp == UNICODE_REPLACEMENT_CHARACTER && fatal) {
        return DecodeError::InvalidSequence;
      }
      decoded->push_back(cp);
    }
  }
  return DecodeError::None;
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
