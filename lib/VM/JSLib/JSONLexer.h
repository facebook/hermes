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
#include "hermes/VM/SmallXString.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"
#include "hermes/VM/TwineChar16.h"

#include "llvh/ADT/Optional.h"

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
  // A string token can be represented as either a StringPrimitive or SymbolID.
  MutableHandle<StringPrimitive> stringValue_;
  MutableHandle<SymbolID> symbolValue_;

  /// The starting character of this token.
  char16_t firstChar_{};

  JSONToken(const JSONToken &) = delete;
  const JSONToken &operator=(const JSONToken &) = delete;

 public:
  explicit JSONToken(Runtime &runtime)
      : stringValue_(runtime), symbolValue_(runtime) {}

  JSONTokenKind getKind() const {
    return kind_;
  }

  double getNumber() const {
    assert(getKind() == JSONTokenKind::Number);
    return numberValue_;
  }

  Handle<StringPrimitive> getStrAsPrim() const {
    assert(getKind() == JSONTokenKind::String);
    return stringValue_;
  }

  Handle<SymbolID> getStrAsSymbol() const {
    assert(getKind() == JSONTokenKind::String);
    return symbolValue_;
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
  void setString(Handle<StringPrimitive> str) {
    kind_ = JSONTokenKind::String;
    stringValue_ = str.get();
  }
  void setSymbol(Handle<SymbolID> sym) {
    kind_ = JSONTokenKind::String;
    symbolValue_ = sym;
  }
};

class JSONLexer {
 private:
  UTF16Stream curCharPtr_;

  Runtime &runtime_;

  JSONToken token_;
  using StrAsSymbol = std::true_type;
  using StrAsValue = std::false_type;

 public:
  JSONLexer(Runtime &runtime, UTF16Stream &&stream)
      : curCharPtr_(std::move(stream)), runtime_(runtime), token_(runtime) {}

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
