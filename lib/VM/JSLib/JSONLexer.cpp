/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSONLexer.h"

#include "hermes/VM/StringPrimitive.h"

#include "dtoa/dtoa.h"

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
  llvh::SmallVector<char, 32> str8;
  while (hasChar()) {
    auto ch = *iter_.cur;
    if (!(ch == u'-' || ch == u'+' || ch == u'.' || (ch | 32) == u'e' ||
          (ch >= u'0' && ch <= u'9'))) {
      break;
    }
    str8.push_back(ch);
    ++iter_.cur;
  }

  size_t len = str8.size();
  assert(len > 0 && "scanNumber must be called on a number-looking char");
  if (str8[0] == '0' && len > 1 && str8[1] >= '0' && str8[1] <= '9') {
    // The integer part cannot start with 0, unless it's 0.
    return errorWithChar(u"Unexpected character in number: ", str8[1]);
  }

  str8.push_back('\0');

  char *endPtr;
  double value = ::hermes_g_strtod(str8.data(), &endPtr);
  if (endPtr != str8.data() + len) {
    return errorWithChar(u"Unexpected character in number: ", *endPtr);
  }
  token_.setNumber(value);
  return ExecutionStatus::RETURNED;
}

template <EncodingKind Kind>
template <typename ForKey>
ExecutionStatus JSONLexer<Kind>::scanString() {
  assert(*iter_.cur == '"');
  ++iter_.cur;
  bool hasEscape = false;
  // Ideally we don't have to use escapedStr. In the case of a plain string with
  // no escapes, we construct an ArrayRef at the end of scanning that points to
  // the beginning and end of the string.
  llvh::SmallVector<char16_t, 32> escapedStr;
  const CharT *beginNonEscaped;
  if constexpr (Traits::UsesRawPtr) {
    beginNonEscaped = iter_.cur;
  } else {
    iter_.cur.beginCapture();
  }
  hermes::JenkinsHash hash = hermes::JenkinsHashInit;

  while (hasChar()) {
    if (*iter_.cur == '"') {
      // Reached the end of string.
      llvh::ArrayRef<CharT> strRef;
      if (LLVM_LIKELY(!hasEscape)) {
        if constexpr (Traits::UsesRawPtr) {
          strRef = llvh::ArrayRef<CharT>{beginNonEscaped, iter_.cur};
        } else {
          strRef = iter_.cur.endCapture();
        }
      }
      ++iter_.cur;
      if constexpr (ForKey::value) {
        auto symRes = LLVM_UNLIKELY(hasEscape)
            ? runtime_.getIdentifierTable().getSymbolHandle(
                  runtime_, llvh::ArrayRef<char16_t>{escapedStr}, hash)
            : runtime_.getIdentifierTable().getSymbolHandle(
                  runtime_, strRef, hash);
        if (symRes == ExecutionStatus::EXCEPTION)
          return ExecutionStatus::EXCEPTION;
        token_.setSymbol(symRes->get());
        return ExecutionStatus::RETURNED;
      }
      auto strRes = LLVM_UNLIKELY(hasEscape)
          ? StringPrimitive::create(
                runtime_, llvh::ArrayRef<char16_t>{escapedStr})
          : StringPrimitive::create(runtime_, strRef);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      token_.setString(vmcast<StringPrimitive>(*strRes));
      return ExecutionStatus::RETURNED;
    } else if (*iter_.cur <= '\u001F') {
      return error(u"U+0000 thru U+001F is not allowed in string");
    }
    char16_t scannedChar = -1;
    if (*iter_.cur == u'\\') {
      if (!hasEscape) {
        // This is the first escape character encountered, so append everything
        // we've seen so far to escapedStr.
        if constexpr (Traits::UsesRawPtr) {
          escapedStr.append(beginNonEscaped, iter_.cur);
        } else {
          llvh::ArrayRef<char16_t> nonEscapedRef = iter_.cur.endCapture();
          escapedStr.append(nonEscapedRef.begin(), nonEscapedRef.end());
        }
      }
      hasEscape = true;
      ++iter_.cur;
      if (!hasChar()) {
        return error("Unexpected end of input");
      }
      switch (*iter_.cur) {
#define CONSUME_VAL(v)     \
  escapedStr.push_back(v); \
  ++iter_.cur;

        case u'"':
        case u'/':
        case u'\\':
          CONSUME_VAL(*iter_.cur)
          break;
        case 'b':
          CONSUME_VAL(8)
          break;
        case 'f':
          CONSUME_VAL(12)
          break;
        case 'n':
          CONSUME_VAL(10)
          break;
        case 'r':
          CONSUME_VAL(13)
          break;
        case 't':
          CONSUME_VAL(9)
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
      scannedChar = escapedStr.back();
    } else {
      scannedChar = *iter_.cur;
      if (hasEscape)
        escapedStr.push_back(scannedChar);
      ++iter_.cur;
    }
    if constexpr (ForKey::value) {
      hash = hermes::updateJenkinsHash(hash, scannedChar);
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
