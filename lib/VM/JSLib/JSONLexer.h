/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PARSER_JSONLEXER_H
#define HERMES_PARSER_JSONLEXER_H

#include "hermes/Support/UTF16Stream.h"
#include "hermes/VM/IdentifierTable.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/TwineChar16.h"

namespace hermes {
namespace vm {

class Runtime;

enum class JSONTokenKind {
  Number,
  String,
  True,
  False,
  Null,
  LBrace,
  RBrace,
  LSquare,
  RSquare,
  Comma,
  Colon,
  Eof,
  None
};

/// Encapsulates the information contained in the current token.
/// We only ever create one of these, but it is cleaner to keep the data
/// in a separate class.
class JSONToken {
  JSONTokenKind kind_{JSONTokenKind::None};
  double numberValue_{};

  /// The starting character of this token.
  char16_t firstChar_{};

  JSONToken(const JSONToken &) = delete;
  const JSONToken &operator=(const JSONToken &) = delete;

  struct : public Locals {
    PinnedValue<StringPrimitive> stringValue;
    PinnedValue<SymbolID> symbolValue;
  } lv_;
  LocalsRAII lraii_;

 public:
  explicit JSONToken(Runtime &runtime) : lraii_(runtime, &lv_) {}

  JSONTokenKind getKind() const {
    return kind_;
  }

  double getNumber() const {
    assert(getKind() == JSONTokenKind::Number);
    return numberValue_;
  }

  Handle<StringPrimitive> getStrAsPrim() const {
    assert(getKind() == JSONTokenKind::String);
    return lv_.stringValue;
  }

  Handle<SymbolID> getStrAsSymbol() const {
    assert(getKind() == JSONTokenKind::String);
    return lv_.symbolValue;
  }

  char16_t getFirstChar() const {
    return firstChar_;
  }
  void setFirstChar(char16_t firstChar) {
    firstChar_ = firstChar;
  }

  void setPunctuator(JSONTokenKind kind) {
    kind_ = kind;
  }
  void setEof() {
    kind_ = JSONTokenKind::Eof;
    firstChar_ = 0;
  }
  void invalidate() {
    kind_ = JSONTokenKind::None;
  }

  void setNumber(double number) {
    kind_ = JSONTokenKind::Number;
    numberValue_ = number;
  }
  void setString(StringPrimitive *str) {
    kind_ = JSONTokenKind::String;
    lv_.stringValue = str;
  }
  void setSymbol(SymbolID sym) {
    kind_ = JSONTokenKind::String;
    lv_.symbolValue = sym;
  }
};

/// These are all the different input encodings that JSONLexer supports.
enum class EncodingKind { ASCII, UTF16, UTF8 };

/// EncodingTraits contains the information that the lexer needs in order to
/// iterate through the encoded input.
template <EncodingKind>
struct EncodingTraits;

/// ASCII specialization
template <>
struct EncodingTraits<EncodingKind::ASCII> {
  using CharT = char;
  static constexpr bool UsesRawPtr = true;
  struct Iterator {
    const CharT *cur;
    const CharT *end;
  };
};

/// UTF16 specialization
template <>
struct EncodingTraits<EncodingKind::UTF16> {
  using CharT = char16_t;
  static constexpr bool UsesRawPtr = true;
  struct Iterator {
    const CharT *cur;
    const CharT *end;
  };
};

/// UTF8 specialization
template <>
struct EncodingTraits<EncodingKind::UTF8> {
  /// Even though the underlying storage is char8_t, the API that
  /// UTF16Stream exposes is char16_t.
  using CharT = char16_t;
  static constexpr bool UsesRawPtr = false;
  struct Iterator {
    /// UTF16Stream is capable of converting UTF8 input data to a stream of
    /// UTF16 values.
    UTF16Stream cur;
  };
};

template <EncodingKind Kind>
class JSONLexer {
 private:
  using Traits = EncodingTraits<Kind>;
  using CharT = typename Traits::CharT;
  using StrAsSymbol = std::true_type;
  using StrAsValue = std::false_type;

  /// It's important to keep the iterator fields at the very beginning of the
  /// layout. There was a consistent, measureable perf regression when the
  /// cursor pointer was not located at offset 0.
  typename EncodingTraits<Kind>::Iterator iter_;
  Runtime &runtime_;
  JSONToken token_;

 public:
  /// Constructor for when we can work directly with raw pointers.
  template <typename U = Traits, typename = std::enable_if_t<U::UsesRawPtr>>
  JSONLexer(Runtime &runtime, llvh::ArrayRef<CharT> str)
      : iter_{str.begin(), str.end()}, runtime_(runtime), token_(runtime) {}

  /// Constructor for when we *cannot* work directly with raw pointers.
  template <typename U = Traits, typename = std::enable_if_t<!U::UsesRawPtr>>
  JSONLexer(Runtime &runtime, UTF16Stream &&jsonString)
      : iter_{std::move(jsonString)}, runtime_(runtime), token_(runtime) {}

  /// \return the current token.
  const JSONToken *getCurToken() const {
    assert(
        token_.getKind() != JSONTokenKind::None &&
        "Obtaining an invalid token");
    return &token_;
  }

  /// Scan the next token, and store it in token_.
  /// \return Exception if error occurs.
  /// All whitespace is skipped before the new token.
  LLVM_NODISCARD ExecutionStatus advance();

  /// Same as advance, except if a string is encountered, it will parse it into
  /// the current token's symbol field.
  LLVM_NODISCARD ExecutionStatus advanceStrAsSymbol();

  /// Raise a JSON parse exception with message \p msg.
  /// token_ will also be invalidated.
  LLVM_NODISCARD ExecutionStatus error(const TwineChar16 &msg) {
    token_.invalidate();
    return runtime_.raiseSyntaxError(
        TwineChar16("JSON Parse error: ").concat(msg));
  }

  /// Report a JSON parse exception with message \p msg,
  /// at the character \p ch.
  LLVM_NODISCARD ExecutionStatus
  errorWithChar(const TwineChar16 &msg, char16_t ch) {
    return error(msg.concat(UTF16Ref(&ch, 1)));
  }

  /// Raise a JSON parse unexpected character error.
  LLVM_NODISCARD ExecutionStatus errorUnexpectedChar() {
    if (token_.getKind() == JSONTokenKind::Eof) {
      return error("Unexpected end of input");
    }
    return errorWithChar(
        u"Unexpected character: ", getCurToken()->getFirstChar());
  }

 private:
  /// \return true if there is more input left for the lexer to consume.
  inline bool hasChar() {
    if constexpr (Traits::UsesRawPtr) {
      return iter_.cur < iter_.end;
    } else {
      return iter_.cur.hasChar();
    }
  }

  /// Advance the lexer by a single token. The parameter forKey determines how
  /// strings are stored in the lexer.
  LLVM_NODISCARD ExecutionStatus advanceHelper(bool forKey);

  /// Parse a JSONNumber.
  LLVM_NODISCARD ExecutionStatus scanNumber();

  /// Parse a JSONString. If ForKey is std::true_type, then the string will be
  /// parsed into a symbol. If ForKey is std::false_type, the scanned string
  /// will be turned into a new StringPrimitive.
  template <typename ForKey>
  LLVM_NODISCARD ExecutionStatus scanString();

  /// Parse a reserved keyword.
  LLVM_NODISCARD ExecutionStatus scanWord(const char *word, JSONTokenKind kind);

  /// Parse a unicode code point and \return the char16 value.
  /// On error, \return llvh::None.
  CallResult<char16_t> consumeUnicode();
};

} // namespace vm
} // namespace hermes

#endif // HERMES_PARSER_JSONLEXER_H
