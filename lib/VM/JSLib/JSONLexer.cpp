/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSONLexer.h"

#include "hermes/VM/StringPrimitive.h"

#include "hermes/Support/FastStrToDouble.h"

namespace hermes {
namespace vm {

// clang-format off
/// Helper macro to define an array of 256 values, by invoking another macro with numbers from 0 to 0xFF.
#define BUILD_TABLE_256(E)                                                                                                      \
E(0x00) E(0x01) E(0x02) E(0x03) E(0x04) E(0x05) E(0x06) E(0x07) E(0x08) E(0x09) E(0x0A) E(0x0B) E(0x0C) E(0x0D) E(0x0E) E(0x0F) \
E(0x10) E(0x11) E(0x12) E(0x13) E(0x14) E(0x15) E(0x16) E(0x17) E(0x18) E(0x19) E(0x1A) E(0x1B) E(0x1C) E(0x1D) E(0x1E) E(0x1F) \
E(0x20) E(0x21) E(0x22) E(0x23) E(0x24) E(0x25) E(0x26) E(0x27) E(0x28) E(0x29) E(0x2A) E(0x2B) E(0x2C) E(0x2D) E(0x2E) E(0x2F) \
E(0x30) E(0x31) E(0x32) E(0x33) E(0x34) E(0x35) E(0x36) E(0x37) E(0x38) E(0x39) E(0x3A) E(0x3B) E(0x3C) E(0x3D) E(0x3E) E(0x3F) \
E(0x40) E(0x41) E(0x42) E(0x43) E(0x44) E(0x45) E(0x46) E(0x47) E(0x48) E(0x49) E(0x4A) E(0x4B) E(0x4C) E(0x4D) E(0x4E) E(0x4F) \
E(0x50) E(0x51) E(0x52) E(0x53) E(0x54) E(0x55) E(0x56) E(0x57) E(0x58) E(0x59) E(0x5A) E(0x5B) E(0x5C) E(0x5D) E(0x5E) E(0x5F) \
E(0x60) E(0x61) E(0x62) E(0x63) E(0x64) E(0x65) E(0x66) E(0x67) E(0x68) E(0x69) E(0x6A) E(0x6B) E(0x6C) E(0x6D) E(0x6E) E(0x6F) \
E(0x70) E(0x71) E(0x72) E(0x73) E(0x74) E(0x75) E(0x76) E(0x77) E(0x78) E(0x79) E(0x7A) E(0x7B) E(0x7C) E(0x7D) E(0x7E) E(0x7F) \
E(0x80) E(0x81) E(0x82) E(0x83) E(0x84) E(0x85) E(0x86) E(0x87) E(0x88) E(0x89) E(0x8A) E(0x8B) E(0x8C) E(0x8D) E(0x8E) E(0x8F) \
E(0x90) E(0x91) E(0x92) E(0x93) E(0x94) E(0x95) E(0x96) E(0x97) E(0x98) E(0x99) E(0x9A) E(0x9B) E(0x9C) E(0x9D) E(0x9E) E(0x9F) \
E(0xA0) E(0xA1) E(0xA2) E(0xA3) E(0xA4) E(0xA5) E(0xA6) E(0xA7) E(0xA8) E(0xA9) E(0xAA) E(0xAB) E(0xAC) E(0xAD) E(0xAE) E(0xAF) \
E(0xB0) E(0xB1) E(0xB2) E(0xB3) E(0xB4) E(0xB5) E(0xB6) E(0xB7) E(0xB8) E(0xB9) E(0xBA) E(0xBB) E(0xBC) E(0xBD) E(0xBE) E(0xBF) \
E(0xC0) E(0xC1) E(0xC2) E(0xC3) E(0xC4) E(0xC5) E(0xC6) E(0xC7) E(0xC8) E(0xC9) E(0xCA) E(0xCB) E(0xCC) E(0xCD) E(0xCE) E(0xCF) \
E(0xD0) E(0xD1) E(0xD2) E(0xD3) E(0xD4) E(0xD5) E(0xD6) E(0xD7) E(0xD8) E(0xD9) E(0xDA) E(0xDB) E(0xDC) E(0xDD) E(0xDE) E(0xDF) \
E(0xE0) E(0xE1) E(0xE2) E(0xE3) E(0xE4) E(0xE5) E(0xE6) E(0xE7) E(0xE8) E(0xE9) E(0xEA) E(0xEB) E(0xEC) E(0xED) E(0xEE) E(0xEF) \
E(0xF0) E(0xF1) E(0xF2) E(0xF3) E(0xF4) E(0xF5) E(0xF6) E(0xF7) E(0xF8) E(0xF9) E(0xFA) E(0xFB) E(0xFC) E(0xFD) E(0xFE) E(0xFF)
// clang-format on

static constexpr JSONTokenKind tokenTableBuilder(uint8_t ch) {
  switch (ch) {
    case '\t':
    case '\r':
    case '\n':
    case ' ':
      return JSONTokenKind::Whitespace;
    case '"':
      return JSONTokenKind::String;
    case ',':
      return JSONTokenKind::Comma;
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return JSONTokenKind::Number;
    case ':':
      return JSONTokenKind::Colon;
    case '[':
      return JSONTokenKind::LSquare;
    case ']':
      return JSONTokenKind::RSquare;
    case '{':
      return JSONTokenKind::LBrace;
    case '}':
      return JSONTokenKind::RBrace;
    case 'f':
      return JSONTokenKind::False;
    case 'n':
      return JSONTokenKind::Null;
    case 't':
      return JSONTokenKind::True;
    default:
      return JSONTokenKind::Error;
  }
}

/// This table maps an ASCII character to a JSONTokenKind.
static const constexpr JSONTokenKind TOKEN_TABLE[256] = {
#define TABLE_ELEMENT(N) tokenTableBuilder(N),
    BUILD_TABLE_256(TABLE_ELEMENT)
#undef TABLE_ELEMENT
};

static const char *TrueString = "true";
static const char *FalseString = "false";
static const char *NullString = "null";

template <EncodingKind Kind>
ExecutionStatus JSONLexer<Kind>::advance() {
  auto res = advanceHelper(false);
  assert(
      res == ExecutionStatus::EXCEPTION ||
      (token_.getKind() != JSONTokenKind::Whitespace &&
       token_.getKind() != JSONTokenKind::Error) &&
          "Error and Whitespace should never be set after a successful advance");
  return res;
}

template <EncodingKind Kind>
ExecutionStatus JSONLexer<Kind>::advanceStrAsSymbol() {
  auto res = advanceHelper(true);
  assert(
      res == ExecutionStatus::EXCEPTION ||
      (token_.getKind() != JSONTokenKind::Whitespace &&
       token_.getKind() != JSONTokenKind::Error) &&
          "Error and Whitespace should never be set after a successful advance");
  return res;
}

template <EncodingKind Kind>
ExecutionStatus JSONLexer<Kind>::advanceHelper(bool forKey) {
  // Skip whitespaces.
  JSONTokenKind curKind;
  CharT curVal;
  while (true) {
    if (LLVM_UNLIKELY(!hasChar())) {
      token_.setEof();
      return ExecutionStatus::RETURNED;
    }
    curVal = *iter_.cur;
    if constexpr (sizeof(CharT) > 1) {
      if (LLVM_UNLIKELY(curVal >= 0xFF)) {
        token_.setFirstChar(curVal);
        return errorWithChar(u"Unexpected character: ", curVal);
      }
    }
    curKind = TOKEN_TABLE[(uint8_t)curVal];
    if (curKind == JSONTokenKind::Whitespace) {
      ++iter_.cur;
    } else {
      break;
    }
  }
  token_.setPunctuator(curKind);
  token_.setFirstChar(curVal);

  switch (curKind) {
    case JSONTokenKind::String:
      if (forKey) {
        return scanString<StrAsSymbol>();
      } else {
        return scanString<StrAsValue>();
      }
    case JSONTokenKind::Number:
      return scanNumber();
    case JSONTokenKind::True:
      return scanWord(TrueString);
    case JSONTokenKind::False:
      return scanWord(FalseString);
    case JSONTokenKind::Null:
      return scanWord(NullString);
    case JSONTokenKind::Comma:
    case JSONTokenKind::Colon:
    case JSONTokenKind::LBrace:
    case JSONTokenKind::RBrace:
    case JSONTokenKind::LSquare:
    case JSONTokenKind::RSquare:
      ++iter_.cur;
      return ExecutionStatus::RETURNED;
    case JSONTokenKind::Error:
      return errorWithChar(u"Unexpected character: ", curVal);
    case JSONTokenKind::Whitespace:
    case JSONTokenKind::Eof:
    case JSONTokenKind::None:
      llvm_unreachable("Invalid token kind");
  }

  return ExecutionStatus::RETURNED;
}

template <EncodingKind Kind>
CallResult<char16_t> JSONLexer<Kind>::consumeUnicode() {
  uint16_t val = 0;
  for (unsigned i = 0; i < 4; ++i) {
    if (!hasChar()) {
      return error("Unexpected end of input");
    }
    int ch = *iter_.cur | 32;
    if (ch >= '0' && ch <= '9') {
      ch -= '0';
    } else if (ch >= 'a' && ch <= 'f') {
      ch -= 'a' - 10;
    } else {
      return errorWithChar(u"Invalid unicode point character: ", *iter_.cur);
    }
    val = (val << 4) + ch;
    ++iter_.cur;
  }

  return static_cast<char16_t>(val);
}

template <EncodingKind Kind>
ExecutionStatus JSONLexer<Kind>::scanNumber() {
  // Mark the beginning of the number contents.
  const CharT *beginNumContents;
  if constexpr (Traits::UsesRawPtr) {
    beginNumContents = iter_.cur;
  } else {
    iter_.cur.beginCapture();
  }

  // Check for initial negative sign.
  double multiplier = 1;
  if (*iter_.cur == '-') {
    multiplier = -1;
    ++iter_.cur;
  }

  bool isTrivialInteger = true;
  uint64_t integer = 0;
  constexpr uint64_t kFirstIntThatCanOverflow = (UINT64_MAX - 9) / 10;
  while (hasChar()) {
    auto ch = *iter_.cur;
    if (ch >= '0' && ch <= '9') {
      if (integer > kFirstIntThatCanOverflow) {
        isTrivialInteger = false;
      }
      integer = integer * 10 + (ch - '0');
    } else if (ch == '-' || ch == '+' || ch == '.' || (ch | 32) == 'e') {
      isTrivialInteger = false;
    } else {
      break;
    }
    ++iter_.cur;
  }

  // Obtain a view over the number contents.
  llvh::ArrayRef<CharT> numRef;
  if constexpr (Traits::UsesRawPtr) {
    numRef = llvh::ArrayRef<CharT>{beginNumContents, iter_.cur};
  } else {
    numRef = iter_.cur.endCapture();
  }

  // Check for illegal leading 0.
  size_t len = numRef.size();
  assert(len > 0 && "scanNumber must be called on a number-looking char");
  if (LLVM_UNLIKELY(
          numRef[0] == '0' && len > 1 && numRef[1] >= '0' &&
          numRef[1] <= '9')) {
    // The integer part cannot start with 0, unless it's 0.
    return errorWithChar(u"Unexpected character in number: ", numRef[1]);
  }

  if (isTrivialInteger) {
    token_.setNumber((double)integer * multiplier);
    return ExecutionStatus::RETURNED;
  }

  OptValue<double> result = fastStrToDouble(numRef);
  if (LLVM_UNLIKELY(!result)) {
    return error("Invalid number input");
  }
  token_.setNumber(*result);
  return ExecutionStatus::RETURNED;
}

template <EncodingKind Kind>
template <typename ForKey>
ExecutionStatus JSONLexer<Kind>::scanString() {
  assert(*iter_.cur == '"');
  // Advance over opening quote.
  ++iter_.cur;
  const CharT *beginNonEscaped;
  if constexpr (Traits::UsesRawPtr) {
    beginNonEscaped = iter_.cur;
  } else {
    iter_.cur.beginCapture();
  }
  hermes::JenkinsHash hash = hermes::JenkinsHashInit;

  // This loop can only handle simple strings that have no escapes. If an escape
  // is encountered, iteration stops and the string is finished processing
  // below.
  while (true) {
    if (LLVM_UNLIKELY(!hasChar())) {
      return error("Unexpected end of input");
    }
    CharT curVal = *iter_.cur;
    if (curVal == '"') {
      // Reached the end of string.
      llvh::ArrayRef<CharT> strRef;
      if constexpr (Traits::UsesRawPtr) {
        strRef = llvh::ArrayRef<CharT>{beginNonEscaped, iter_.cur};
      } else {
        strRef = iter_.cur.endCapture();
      }
      ++iter_.cur;
      if constexpr (ForKey::value) {
        auto symRes = runtime_.getIdentifierTable().getSymbolHandle(
            runtime_, strRef, hash);
        if (symRes == ExecutionStatus::EXCEPTION)
          return ExecutionStatus::EXCEPTION;
        token_.setSymbol(symRes->get());
        return ExecutionStatus::RETURNED;
      }
      auto strRes = StringPrimitive::create(runtime_, strRef);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      token_.setString(vmcast<StringPrimitive>(*strRes));
      return ExecutionStatus::RETURNED;
    } else if (LLVM_UNLIKELY(curVal <= '\u001F')) {
      return error(u"U+0000 thru U+001F is not allowed in string");
    }
    if (LLVM_LIKELY(curVal != '\\')) {
      if constexpr (ForKey::value) {
        hash = hermes::updateJenkinsHash(hash, curVal);
      }
      ++iter_.cur;
      continue;
    }
    // escape found, break out.
    break;
  }

  // We reach this point after an escape has occured. This means that the
  // contents of the string we are parsing don't actually exist byte for byte in
  // the input string. Instead, we must build the contents of the string from
  // scratch. The contents of the string are placed into escapedStr.
  llvh::SmallVector<char16_t, 32> escapedStr;
  // Append all the 'simple' string contents that were observed up to this
  // point.
  if constexpr (Traits::UsesRawPtr) {
    escapedStr.append(beginNonEscaped, iter_.cur);
  } else {
    llvh::ArrayRef<char16_t> nonEscapedRef = iter_.cur.endCapture();
    escapedStr.append(nonEscapedRef.begin(), nonEscapedRef.end());
  }
  while (true) {
    if (LLVM_UNLIKELY(!hasChar())) {
      return error("Unexpected end of input");
    }
    CharT curVal = *iter_.cur;
    if (curVal == '"') {
      // Reached the end of string.
      ++iter_.cur;
      if constexpr (ForKey::value) {
        auto symRes = runtime_.getIdentifierTable().getSymbolHandle(
            runtime_, llvh::ArrayRef<char16_t>{escapedStr}, hash);
        if (symRes == ExecutionStatus::EXCEPTION)
          return ExecutionStatus::EXCEPTION;
        token_.setSymbol(symRes->get());
        return ExecutionStatus::RETURNED;
      }
      auto strRes = StringPrimitive::create(
          runtime_, llvh::ArrayRef<char16_t>{escapedStr});
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      token_.setString(vmcast<StringPrimitive>(*strRes));
      return ExecutionStatus::RETURNED;
    } else if (LLVM_UNLIKELY(curVal <= '\u001F')) {
      return error(u"U+0000 thru U+001F is not allowed in string");
    }
    if (LLVM_LIKELY(curVal != '\\')) {
      escapedStr.push_back(curVal);
      if constexpr (ForKey::value) {
        hash = hermes::updateJenkinsHash(hash, curVal);
      }
      ++iter_.cur;
      continue;
    }
    // Advance over starting backslash.
    ++iter_.cur;
    if (!hasChar()) {
      return error("Unexpected end of input");
    }
    switch (*iter_.cur) {
      case '"':
        escapedStr.push_back('"');
        ++iter_.cur;
        break;
      case '/':
        escapedStr.push_back('/');
        ++iter_.cur;
        break;
      case '\\':
        escapedStr.push_back('\\');
        ++iter_.cur;
        break;
      case 'b':
        // Backspace (Unicode U+0008)
        escapedStr.push_back(0x08);
        ++iter_.cur;
        break;
      case 'f':
        // Form feed (Unicode U+000C)
        escapedStr.push_back(0x0C);
        ++iter_.cur;
        break;
      case 'n':
        // Line feed (Unicode U+000A)
        escapedStr.push_back(0x0A);
        ++iter_.cur;
        break;
      case 'r':
        // Carriage return (Unicode U+000D)
        escapedStr.push_back(0x0D);
        ++iter_.cur;
        break;
      case 't':
        // Horiztonal tab (Unicode U+0009)
        escapedStr.push_back(0x09);
        ++iter_.cur;
        break;
      case 'u': {
        ++iter_.cur;
        CallResult<char16_t> cr = consumeUnicode();
        if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        escapedStr.push_back(*cr);
        break;
      }

      default:
        return errorWithChar(u"Invalid escape sequence: ", *iter_.cur);
    }
    char16_t escapedCharVal = escapedStr.back();
    if constexpr (ForKey::value) {
      hash = hermes::updateJenkinsHash(hash, escapedCharVal);
    }
  }
  return error("Unexpected end of input");
}

template <EncodingKind Kind>
ExecutionStatus JSONLexer<Kind>::scanWord(const char *word) {
  while (*word && hasChar()) {
    if (*iter_.cur != *word) {
      return errorWithChar(u"Unexpected character: ", *iter_.cur);
    }
    ++iter_.cur;
    ++word;
  }
  if (*word) {
    return error(u"Unexpected end of input");
  }
  return ExecutionStatus::RETURNED;
}

template class JSONLexer<EncodingKind::ASCII>;
template class JSONLexer<EncodingKind::UTF8>;
template class JSONLexer<EncodingKind::UTF16>;

} // namespace vm
} // namespace hermes
