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

static const char *TrueString = "true";
static const char *FalseString = "false";
static const char *NullString = "null";

static bool isJSONWhiteSpace(char16_t ch) {
  // JSONWhiteSpace includes <TAB>, <CR>, <LF>, <SP>.
  return (ch == u'\t' || ch == u'\r' || ch == u'\n' || ch == u' ');
}

template <EncodingKind Kind>
ExecutionStatus JSONLexer<Kind>::advance() {
  return advanceHelper(false);
}

template <EncodingKind Kind>
ExecutionStatus JSONLexer<Kind>::advanceStrAsSymbol() {
  return advanceHelper(true);
}

template <EncodingKind Kind>
ExecutionStatus JSONLexer<Kind>::advanceHelper(bool forKey) {
  // Skip whitespaces.
  while (hasChar() && isJSONWhiteSpace(*iter_.cur)) {
    ++iter_.cur;
  }

  // End of buffer.
  if (!hasChar()) {
    token_.setEof();
    return ExecutionStatus::RETURNED;
  }

  token_.setFirstChar(*iter_.cur);

#define PUNC(ch, tok)          \
  case ch:                     \
    token_.setPunctuator(tok); \
    ++iter_.cur;               \
    return ExecutionStatus::RETURNED

#define WORD(ch, word, tok) \
  case ch:                  \
    return scanWord(word, tok)

  switch (*iter_.cur) {
    PUNC(u'{', JSONTokenKind::LBrace);
    PUNC(u'}', JSONTokenKind::RBrace);
    PUNC(u'[', JSONTokenKind::LSquare);
    PUNC(u']', JSONTokenKind::RSquare);
    PUNC(u',', JSONTokenKind::Comma);
    PUNC(u':', JSONTokenKind::Colon);
    WORD(u't', TrueString, JSONTokenKind::True);
    WORD(u'f', FalseString, JSONTokenKind::False);
    WORD(u'n', NullString, JSONTokenKind::Null);

    case u'-':
    case u'0':
    case u'1':
    case u'2':
    case u'3':
    case u'4':
    case u'5':
    case u'6':
    case u'7':
    case u'8':
    case u'9':
      return scanNumber();

    case u'"':
      if (forKey) {
        return scanString<StrAsSymbol>();
      } else {
        return scanString<StrAsValue>();
      }

    default:
      return errorWithChar(u"Unexpected character: ", *iter_.cur);
  }
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
  while (hasChar()) {
    auto ch = *iter_.cur;
    if (!(ch == u'-' || ch == u'+' || ch == u'.' || (ch | 32) == u'e' ||
          (ch >= u'0' && ch <= u'9'))) {
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
  size_t len = numRef.size();
  assert(len > 0 && "scanNumber must be called on a number-looking char");
  if (numRef[0] == '0' && len > 1 && numRef[1] >= '0' && numRef[1] <= '9') {
    // The integer part cannot start with 0, unless it's 0.
    return errorWithChar(u"Unexpected character in number: ", numRef[1]);
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
ExecutionStatus JSONLexer<Kind>::scanWord(
    const char *word,
    JSONTokenKind kind) {
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
  token_.setPunctuator(kind);
  return ExecutionStatus::RETURNED;
}

template class JSONLexer<EncodingKind::ASCII>;
template class JSONLexer<EncodingKind::UTF8>;
template class JSONLexer<EncodingKind::UTF16>;

} // namespace vm
} // namespace hermes
