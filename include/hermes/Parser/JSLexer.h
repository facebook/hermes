/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PARSER_JSLEXER_H
#define HERMES_PARSER_JSLEXER_H

#include "hermes/Support/Allocator.h"
#include "hermes/Support/SourceErrorManager.h"
#include "hermes/Support/StringTable.h"
#include "hermes/Support/UTF8.h"

#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallString.h"

#include <functional>
#include <utility>

namespace hermes {
namespace parser {

using llvm::SMLoc;
using llvm::SMRange;

class Token;
class JSLexer;

enum class TokenKind {
#define TOK(name, str) name,
#include "TokenKinds.def"
};

inline constexpr int ord(TokenKind kind) {
  return static_cast<int>(kind);
}

const unsigned NUM_JS_TOKENS = ord(TokenKind::_last_token) + 1;
const char *tokenKindStr(TokenKind kind);

class RegExpLiteral {
  UniqueString *body_;
  UniqueString *flags_;

 public:
  RegExpLiteral(UniqueString *body, UniqueString *flags)
      : body_(body), flags_(flags) {}

  UniqueString *getBody() const {
    return body_;
  }
  UniqueString *getFlags() const {
    return flags_;
  }
};

/// Encapsulates the information contained in the current token.
/// We only ever create one of these, but it is cleaner to keep the data
/// in a separate class.
class Token {
  TokenKind kind_{TokenKind::none};
  SMRange range_{};
  double numeric_{};
  UniqueString *ident_{nullptr};

  /// Representation of the string literal for tokens that are strings.
  /// If the current token is part of a template literal, this is null
  /// when it contains a NotEscapeSequence. This is necessary for syntax errors
  /// for NotEscapeSequence in untagged string literals.
  UniqueString *stringLiteral_{nullptr};

  RegExpLiteral *regExpLiteral_{nullptr};

  /// The Template Raw Value (TRV) associated with the token if it represents
  /// a part or whole of a template literal.
  UniqueString *templateRawValue_{nullptr};

  /// If the current token is a string literal, this flag indicates whether it
  /// contains any escapes or new line continuations. We need this in order to
  /// detect directives.
  bool stringLiteralContainsEscapes_ = false;

  Token(const Token &) = delete;
  const Token &operator=(const Token &) = delete;

 public:
  Token() = default;
  ~Token() = default;

  TokenKind getKind() const {
    return kind_;
  }
  bool isResWord() const {
    return kind_ > TokenKind::_first_resword &&
        kind_ < TokenKind::_last_resword;
  }
  bool isTemplateLiteral() const {
    return getKind() == TokenKind::no_substitution_template ||
        getKind() == TokenKind::template_head ||
        getKind() == TokenKind::template_middle ||
        getKind() == TokenKind::template_tail;
  }

  SMLoc getStartLoc() const {
    return range_.Start;
  }
  SMLoc getEndLoc() const {
    return range_.End;
  }
  SMRange getSourceRange() const {
    return range_;
  }

  StringRef inputStr() const {
    return StringRef(
        range_.Start.getPointer(),
        range_.End.getPointer() - range_.Start.getPointer());
  }

  double getNumericLiteral() const {
    assert(getKind() == TokenKind::numeric_literal);
    return numeric_;
  }

  UniqueString *getIdentifier() const {
    assert(getKind() == TokenKind::identifier);
    return ident_;
  }
  UniqueString *getResWordIdentifier() const {
    assert(isResWord());
    return ident_;
  }
  UniqueString *getResWordOrIdentifier() const {
    assert(getKind() == TokenKind::identifier || isResWord());
    return ident_;
  }

  UniqueString *getStringLiteral() const {
    assert(getKind() == TokenKind::string_literal);
    return stringLiteral_;
  }
  bool getStringLiteralContainsEscapes() const {
    assert(getKind() == TokenKind::string_literal);
    return stringLiteralContainsEscapes_;
  }

  /// \return whether the template literal token contains a NotEscapeSequence,
  /// as defined in the ES9.0 spec:
  /// http://ecma-international.org/ecma-262/9.0/#prod-NotEscapeSequence
  bool getTemplateLiteralContainsNotEscapes() const {
    assert(isTemplateLiteral());
    return stringLiteral_ == nullptr;
  }

  UniqueString *getTemplateValue() const {
    assert(isTemplateLiteral());
    return stringLiteral_;
  }

  UniqueString *getTemplateRawValue() const {
    assert(isTemplateLiteral());
    return templateRawValue_;
  }

  RegExpLiteral *getRegExpLiteral() const {
    assert(getKind() == TokenKind::regexp_literal);
    return regExpLiteral_;
  }

 private:
  void setStart(const char *start) {
    range_.Start = SMLoc::getFromPointer(start);
  }
  void setEnd(const char *end) {
    range_.End = SMLoc::getFromPointer(end);
  }

  void setPunctuator(TokenKind kind) {
    kind_ = kind;
  }
  void setEof() {
    kind_ = TokenKind::eof;
  }

  void setNumericLiteral(double literal) {
    kind_ = TokenKind::numeric_literal;
    numeric_ = literal;
  }
  void setIdentifier(UniqueString *ident) {
    kind_ = TokenKind::identifier;
    ident_ = ident;
  }
  void setStringLiteral(UniqueString *literal, bool containsEscapes) {
    kind_ = TokenKind::string_literal;
    stringLiteral_ = literal;
    stringLiteralContainsEscapes_ = containsEscapes;
  }
  void setRegExpLiteral(RegExpLiteral *literal) {
    kind_ = TokenKind::regexp_literal;
    regExpLiteral_ = literal;
  }
  void setResWord(TokenKind kind, UniqueString *ident) {
    assert(kind > TokenKind::_first_resword && kind < TokenKind::_last_resword);
    kind_ = kind;
    ident_ = ident;
  }

  void
  setTemplateLiteral(TokenKind kind, UniqueString *cooked, UniqueString *raw) {
    assert(
        kind == TokenKind::no_substitution_template ||
        kind == TokenKind::template_head ||
        kind == TokenKind::template_middle || kind == TokenKind::template_tail);
    kind_ = kind;
    stringLiteral_ = cooked;
    templateRawValue_ = raw;
  }

  friend class JSLexer;
};

class JSLexer {
 public:
  using Allocator = hermes::BumpPtrAllocator;

 private:
  SourceErrorManager &sm_;
  Allocator &allocator_;

  /// ID of the buffer in the SourceErrorManager.
  uint32_t bufId_;

  /// When operating without an external StringTable, store own own copy
  /// here.
  std::unique_ptr<StringTable> ownStrTab_;

  StringTable &strTab_;

  bool strictMode_;

  /// If true, when a surrogate pair sequence is encountered in a string literal
  /// in the source, convert that string literal to its canonical UTF-8
  /// sequence.
  bool convertSurrogates_;

  Token token_;

  const char *bufferStart_;
  const char *curCharPtr_;
  const char *bufferEnd_;

  bool newLineBeforeCurrentToken_ = false;

  llvm::SmallString<256> tmpStorage_;

  /// Storage used for the Template Raw Value when scanning template literals.
  llvm::SmallString<256> rawStorage_;

  /// Pre-allocated identifiers for all reserved words.
  UniqueString *resWordIdent_
      [ord(TokenKind::_last_resword) - ord(TokenKind::_first_resword) + 1];

  UniqueString *&resWordIdent(TokenKind kind) {
    assert(
        kind >= TokenKind::_first_resword && kind <= TokenKind::_last_resword);
    return resWordIdent_[ord(kind) - ord(TokenKind::_first_resword)];
  }

 public:
  /// \param convertSurrogates See member variable \p convertSurrogates_.
  explicit JSLexer(
      std::unique_ptr<llvm::MemoryBuffer> input,
      SourceErrorManager &sm,
      Allocator &allocator,
      StringTable *strTab = nullptr,
      bool strictMode = true,
      bool convertSurrogates = false);

  /// \param convertSurrogates See member variable \p convertSurrogates_.
  explicit JSLexer(
      uint32_t bufId,
      SourceErrorManager &sm,
      Allocator &allocator,
      StringTable *strTab = nullptr,
      bool strictMode = true,
      bool convertSurrogates = false);

  /// \param convertSurrogates See member variable \p convertSurrogates_.
  JSLexer(
      StringRef input,
      SourceErrorManager &sm,
      Allocator &allocator,
      StringTable *strTab = nullptr,
      bool strictMode = true,
      bool convertSurrogates = false)
      : JSLexer(
            llvm::MemoryBuffer::getMemBuffer(input, "JavaScript"),
            sm,
            allocator,
            strTab,
            strictMode,
            convertSurrogates) {}

  /// \param convertSurrogates See member variable \p convertSurrogates_.
  JSLexer(
      llvm::MemoryBufferRef input,
      SourceErrorManager &sm,
      Allocator &allocator,
      StringTable *strTab = nullptr,
      bool strictMode = true,
      bool convertSurrogates = false)
      : JSLexer(
            llvm::MemoryBuffer::getMemBuffer(input),
            sm,
            allocator,
            strTab,
            strictMode,
            convertSurrogates) {}

  SourceErrorManager &getSourceMgr() {
    return sm_;
  }

  Allocator &getAllocator() {
    return allocator_;
  }
  StringTable &getStringTable() {
    return strTab_;
  }

  bool isStrictMode() const {
    return strictMode_;
  }

  void setStrictMode(bool strictMode) {
    strictMode_ = strictMode;
  }

  /// \return true if a line terminator was consumed before the current token.
  ///
  /// The caller can use that information to make decisions about inserting
  /// missing semicolons.
  bool isNewLineBeforeCurrentToken() const {
    return newLineBeforeCurrentToken_;
  }

  /// \return the current token.
  const Token *getCurToken() const {
    return &token_;
  }

  /// Force an EOF at the next token.
  void forceEOF() {
    curCharPtr_ = bufferEnd_;
  }

  /// Grammar context to be passed to advance() determining whether "/" or
  /// RegExp can follow at that point.
  enum GrammarContext { AllowRegExp, AllowDiv };

  /// Consume the current token and scan the next one, which becomes the new
  /// current token. All whitespace is skipped befire the new token and if
  /// a line terminator was encountered, the newLineBefireCurrentToken_ flag is
  /// set.
  /// \param grammarContext enable recognizing either "/" and "/=", or a regexp.
  const Token *advance(GrammarContext grammarContext = AllowRegExp);

  /// Check whether the current token is a directive, in other words is it a
  /// string literal without escapes or new line continuations, followed by
  /// either new line, semicolon or right brace.
  /// This doesn't move the input pointer, so the optional semicolon, brace
  /// or the new line will be consumed normally by the next \c advance() call.
  ///
  /// \return true if the token can be interpreted as a directive.
  bool isCurrentTokenADirective();

  /// Rescan the } token as a TemplateMiddle or TemplateTail.
  /// Should be called in the middle of parsing a template literal.
  const Token *rescanRBraceInTemplateLiteral();

  /// Report an error for the range from startLoc to curCharPtr.
  bool errorRange(SMLoc startLoc, const llvm::Twine &msg) {
    return error({startLoc, SMLoc::getFromPointer(curCharPtr_)}, msg);
  }

  /// Report an error using the current token's location.
  bool error(const llvm::Twine &msg) {
    return error(token_.getSourceRange(), msg);
  }

  /// Emit an error at the specified source location. If the maximum number of
  /// errors has been reached, return false and move the scanning pointer to
  /// EOF.
  /// \return false if too many errors have been emitted and we need to abort.
  bool error(SMLoc loc, const llvm::Twine &msg);

  /// Emit an error at the specified source range. If the maximum number of
  /// errors has been reached, return false and move the scanning pointer to
  /// EOF.
  /// \return false if too many errors have been emitted and we need to abort.
  bool error(SMRange range, const llvm::Twine &msg);

  /// Emit an error at the specified source location and range. If the maximum
  /// number of errors has been reached, return false and move the scanning
  /// pointer to EOF.
  /// \return false if too many errors have been emitted and we need to abort.
  bool error(SMLoc loc, SMRange range, const llvm::Twine &msg);

  UniqueString *getIdentifier(StringRef name) {
    return strTab_.getString(name);
  }

  UniqueString *getStringLiteral(StringRef str) {
    if (LLVM_UNLIKELY(convertSurrogates_)) {
      return convertSurrogatesInString(str);
    }
    return strTab_.getString(str);
  }

  /// Move the lexer to the specific spot. Any future \p advance calls
  /// will start from this position (the current token is not updated until
  /// such a call).
  void seek(SMLoc loc) {
    curCharPtr_ = loc.getPointer();
  }

  /// Get the source buffer id we're currently parsing.
  uint32_t getBufferId() {
    return bufId_;
  }

  /// \return a pointer to the beginning of the buffer.
  const char *getBufferStart() const {
    return bufferStart_;
  }

  /// \return a pointer to the end of the buffer.
  const char *getBufferEnd() const {
    return bufferEnd_;
  }

 private:
  /// Initialize the storage with the characters between \p begin and \p end.
  inline void initStorageWith(const char *begin, const char *end);

  /// Encode a Unicode codepoint into a UTF8 sequence and append it to \p
  /// storage. Code points above 0xFFFF are encoded into UTF16, and the
  /// resulting surrogate pair values are encoded individually into UTF8.
  static inline void appendUnicodeToStorage(
      uint32_t cp,
      llvm::SmallVectorImpl<char> &storage);

  /// Encode a Unicode codepoint into a UTF8 sequence and append it to \ref
  /// tmpStorage_. Code points above 0xFFFF are encoded into UTF16, and the
  /// resulting surrogate pair values are encoded individually into UTF8.
  inline void appendUnicodeToStorage(uint32_t cp) {
    appendUnicodeToStorage(cp, tmpStorage_);
  }

  /// Decode a UTF8 sequence, advancing the current pointer and reporting
  /// errors.
  inline uint32_t decodeUTF8();

  /// Decode a UTF8 sequence when it is already known that it is not ASCII.
  /// In other words, isUTF8Start(*at) == true
  /// \param at points to the sequence and is incremented past it.
  inline uint32_t _decodeUTF8SlowPath(const char *&at);

  /// Decode a UTF8 sequence at the specified address \p at without
  /// reporting errors.
  ///
  /// The caller must have already ensured that the first byte is not ASCII.
  /// The caller can verify the decoded value and optionally update the current
  /// pointer.
  ///
  /// \return the decoded value and the incremented pointer
  inline std::pair<uint32_t, const char *> _peekUTF8(const char *at) const;

  /// Decode a UTF8 sequence without advancing the current pointer and without
  /// reporting errors.
  ///
  /// The caller must have already ensured that the first byte is not ASCII.
  /// The caller can verify the decoded value and optionally update the current
  /// pointer.
  ///
  /// \return the decoded value and the incremented current pointer
  inline std::pair<uint32_t, const char *> _peekUTF8() const;

  static inline bool isUnicodeIdentifierStart(uint32_t ch);
  static inline bool isUnicodeIdentifierPart(uint32_t ch);

  /// Decode a unicode escape sequence in the form "\uXXXX".
  ///
  /// \ref curCharPtr_  must point to the backslash of the escape. It will be
  /// updated to point to the first character after the escape.
  uint32_t consumeUnicodeEscape();

  /// Decode a unicode escape sequence in the form "\uXXXX".
  /// Does not report any errors.
  ///
  /// \ref curCharPtr_  must point to the backslash of the escape. It will be
  /// updated to point to the first character after the escape.
  /// On failure, curCharPtr_ will be reset to where it was when the function
  /// was called.
  /// \return the resultant code point on success, None on error.
  llvm::Optional<uint32_t> consumeUnicodeEscapeOptional();

  /// Decode an IdentifierStart production per ES5.1 7.6.
  /// \return whether an IdentifierStart was successfully decoded.
  bool consumeIdentifierStart();

  /// Decode a sequence (possibly empty) of IdentifierPart.
  void consumeIdentifierParts();

  /// Decode an IdentifierPart per ES5.1 7.6 that does not begin with a
  /// backslash.
  /// \return true if an IdentifierPart was decoded, false if the current
  /// character was a backslash or not an IdentifierPart.
  bool consumeOneIdentifierPartNoEscape();

  /// Scan an octal number after the first character has been recognized
  /// (but not consumed yet). \p maxLen is the maximum length including the
  /// first character.
  unsigned char consumeOctal(unsigned maxLen);

  /// Scan a hex number after the first character has been recognized but not
  /// consumed.
  /// \param requireLen is the number of digits in the hex literal.
  /// \param err if true, report an error on failing to recognize a hex number.
  llvm::Optional<uint32_t> consumeHex(
      unsigned requiredLen,
      bool errorOnFail = true);

  /// Scan a hex number inside the braces in \u{CodePoint}.
  /// \param errorOnFail whether to report error on failure.
  /// \ref curCharPtr_ must point to the opening { of the escape, and will
  /// be updated to point after the closing }.
  /// \return the resultant number on success, None on failure.
  llvm::Optional<uint32_t> consumeBracedCodePoint(bool errorOnFail = true);

  /// Skip until after the end of the line terminaing the block comment.
  /// \return the updated source pointer.
  const char *skipLineComment(const char *start);
  /// Skip until after the end of the block comment.
  /// \return the updated source pointer.
  const char *skipBlockComment(const char *start);

  /// Try to read a "magic comment" of the form `//# name=value` at \p ptr.
  /// \return the value encoded in the comment if found, None otherwise.
  llvm::Optional<StringRef> tryReadMagicComment(
      llvm::StringRef name,
      const char *ptr);

  void scanNumber();

  /// Recognise a reserved word depending on the mode (strict vs non-strict).
  /// \return \c TokenKind::identifier if the word wasn't recognized, or the
  ///   \c TokenKind of the word otherwise.
  TokenKind scanReservedWord(const char *start, unsigned length);

  /// Attempt to scan an identifier with the assumption that there are no
  /// Unicode escapes or UTF-8 characters, meaning that there is no neet to copy
  /// it into a temporary buffer. If an escape or UTF-8 character is
  /// encountered, add the currently scanned part to the storage and continue by
  /// invoking the slow path scanIdentifierParts().
  void scanIdentifierFastPath(const char *start);
  void scanIdentifierParts();
  void scanString();
  void scanRegExp();

  /// Attempt to scan a template literal starting at ` or at }.
  void scanTemplateLiteral();

  /// Convert the surrogates into \p str into a valid UTF-8 sequence, and unique
  /// it into the string table.
  UniqueString *convertSurrogatesInString(StringRef str);

  /// Initialize the parser for a given source buffer id.
  void initializeWithBufferId(uint32_t bufId);

  /// Create/lookup identifiers for all reserved words used during parsing.
  void initializeReservedIdentifiers();
};

inline void JSLexer::initStorageWith(const char *begin, const char *end) {
  tmpStorage_.clear();
  tmpStorage_.append(begin, end);
}

inline void JSLexer::appendUnicodeToStorage(
    uint32_t cp,
    llvm::SmallVectorImpl<char> &storage) {
  // Sized to allow for two 16-bit values to be encoded.
  // A 16-bit value takes up to three bytes encoded in UTF-8.
  char buf[8];
  char *d = buf;
  // We need to normalize code points which would be encoded with a surrogate
  // pair. Note that this produces technically invalid UTF-8.
  if (LLVM_LIKELY(cp < 0x10000)) {
    hermes::encodeUTF8(d, cp);
  } else {
    assert(cp <= UNICODE_MAX_VALUE && "invalid Unicode value");
    cp -= 0x10000;
    hermes::encodeUTF8(d, UTF16_HIGH_SURROGATE + ((cp >> 10) & 0x3FF));
    hermes::encodeUTF8(d, UTF16_LOW_SURROGATE + (cp & 0x3FF));
  }
  storage.append(buf, d);
}

inline uint32_t JSLexer::decodeUTF8() {
  const char *saveStart = curCharPtr_;
  return hermes::decodeUTF8<false>(curCharPtr_, [=](const Twine &msg) {
    sm_.error(SMLoc::getFromPointer(saveStart), msg);
  });
}

inline uint32_t JSLexer::_decodeUTF8SlowPath(const char *&at) {
  return hermes::_decodeUTF8SlowPath<false>(
      at, [=](const Twine &msg) { sm_.error(SMLoc::getFromPointer(at), msg); });
}

inline std::pair<uint32_t, const char *> JSLexer::_peekUTF8(
    const char *at) const {
  uint32_t ch =
      hermes::_decodeUTF8SlowPath<false>(at, [](const llvm::Twine &) {});
  return std::make_pair(ch, at);
}

inline std::pair<uint32_t, const char *> JSLexer::_peekUTF8() const {
  return _peekUTF8(curCharPtr_);
}

inline bool JSLexer::isUnicodeIdentifierStart(uint32_t ch) {
  return ch == '_' || ch == '$' || ((ch | 32) >= 'a' && (ch | 32) <= 'z') ||
      isUnicodeOnlyLetter(ch);
}

inline bool JSLexer::isUnicodeIdentifierPart(uint32_t ch) {
  // TODO: clearly this has to be optimized somehow
  return isUnicodeIdentifierStart(ch) || isUnicodeCombiningMark(ch) ||
      isUnicodeDigit(ch) || isUnicodeConnectorPunctuation(ch) ||
      ch == UNICODE_ZWNJ || ch == UNICODE_ZWJ;
}

} // namespace parser
} // namespace hermes

#endif // HERMES_PARSER_JSLEXER_H
