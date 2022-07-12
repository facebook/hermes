/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Parser/JSLexer.h"

#include "dtoa/dtoa.h"
#include "hermes/Support/Conversions.h"

#include "llvh/ADT/ScopeExit.h"
#include "llvh/ADT/StringSwitch.h"

namespace hermes {
namespace parser {

namespace {

const char *g_tokenStr[] = {
#define TOK(name, str) str,
#include "hermes/Parser/TokenKinds.def"
};

const int UTF8_LINE_TERMINATOR_CHAR0 = 0xe2;

inline bool matchUnicodeLineTerminatorOffset1(const char *curCharPtr_) {
  // Line separator \u2028 UTF8 encoded is      : e2 80 a8
  // Paragraph separator \u2029 UTF8 encoded is: e2 80 a9
  return (unsigned char)curCharPtr_[1] == 0x80 &&
      ((unsigned char)curCharPtr_[2] == 0xa8 ||
       (unsigned char)curCharPtr_[2] == 0xa9);
}
} // namespace

const char *tokenKindStr(TokenKind kind) {
  assert(kind <= TokenKind::_last_token);
  return g_tokenStr[static_cast<unsigned>(kind)];
}

#if HERMES_PARSE_JSX
static llvh::DenseMap<StringRef, uint32_t> initializeHTMLEntities() {
  llvh::DenseMap<StringRef, uint32_t> entities{};

#define HTML_ENTITY(NAME, VALUE) \
  entities.insert({llvh::StringLiteral(#NAME), VALUE});
#include "hermes/Parser/HTMLEntities.def"

  return entities;
}

static const llvh::DenseMap<StringRef, uint32_t> &getHTMLEntities() {
  static const auto entities = initializeHTMLEntities();
  return entities;
}
#endif

JSLexer::JSLexer(
    uint32_t bufId,
    SourceErrorManager &sm,
    Allocator &allocator,
    StringTable *strTab,
    bool strictMode,
    bool convertSurrogates)
    : sm_(sm),
      allocator_(allocator),
      ownStrTab_(strTab ? nullptr : new StringTable(allocator_)),
      strTab_(strTab ? *strTab : *ownStrTab_),
#if HERMES_PARSE_JSX
      htmlEntities_(getHTMLEntities()),
#endif
      strictMode_(strictMode),
      convertSurrogates_(convertSurrogates) {
  initializeWithBufferId(bufId);
  initializeReservedIdentifiers();
}

JSLexer::JSLexer(
    std::unique_ptr<llvh::MemoryBuffer> input,
    SourceErrorManager &sm,
    Allocator &allocator,
    StringTable *strTab,
    bool strictMode,
    bool convertSurrogates)
    : sm_(sm),
      allocator_(allocator),
      ownStrTab_(strTab ? nullptr : new StringTable(allocator_)),
      strTab_(strTab ? *strTab : *ownStrTab_),
#if HERMES_PARSE_JSX
      htmlEntities_(getHTMLEntities()),
#endif
      strictMode_(strictMode),
      convertSurrogates_(convertSurrogates) {
  auto bufId = sm_.addNewSourceBuffer(std::move(input));
  initializeWithBufferId(bufId);
  initializeReservedIdentifiers();
}

void JSLexer::initializeWithBufferId(uint32_t bufId) {
  auto *buffer = sm_.getSourceBuffer(bufId);
  bufId_ = bufId;
  bufferStart_ = buffer->getBufferStart();
  bufferEnd_ = buffer->getBufferEnd();
  curCharPtr_ = bufferStart_;
  assert(*bufferEnd_ == 0 && "buffer must be zero terminated");
}

void JSLexer::initializeReservedIdentifiers() {
  // Add all reserved words to the identifier table
#define RESWORD(name) resWordIdent(TokenKind::rw_##name) = getIdentifier(#name);
#include "hermes/Parser/TokenKinds.def"
}

const Token *JSLexer::advance(GrammarContext grammarContext) {
  newLineBeforeCurrentToken_ = false;

  for (;;) {
    assert(curCharPtr_ <= bufferEnd_ && "lexing past end of input");
#define PUNC_L1_1(ch, tok)        \
  case ch:                        \
    token_.setStart(curCharPtr_); \
    token_.setPunctuator(tok);    \
    ++curCharPtr_;                \
    break

#define PUNC_L2_3(ch1, tok1, ch2a, tok2a, ch2b, tok2b) \
  case ch1:                                            \
    token_.setStart(curCharPtr_);                      \
    if (curCharPtr_[1] == ch2a) {                      \
      token_.setPunctuator(tok2a);                     \
      curCharPtr_ += 2;                                \
    } else if (curCharPtr_[1] == ch2b) {               \
      token_.setPunctuator(tok2b);                     \
      curCharPtr_ += 2;                                \
    } else {                                           \
      token_.setPunctuator(tok1);                      \
      curCharPtr_ += 1;                                \
    }                                                  \
    break

#define PUNC_L2_2(ch1, tok1, ch2, tok2) \
  case ch1:                             \
    token_.setStart(curCharPtr_);       \
    if (curCharPtr_[1] == (ch2)) {      \
      token_.setPunctuator(tok2);       \
      curCharPtr_ += 2;                 \
    } else {                            \
      token_.setPunctuator(tok1);       \
      curCharPtr_ += 1;                 \
    }                                   \
    break

#define PUNC_L3_3(ch1, tok1, ch2, tok2, ch3, tok3) \
  case ch1:                                        \
    token_.setStart(curCharPtr_);                  \
    if (curCharPtr_[1] != (ch2)) {                 \
      token_.setPunctuator(tok1);                  \
      curCharPtr_ += 1;                            \
    } else if (curCharPtr_[2] == (ch3)) {          \
      token_.setPunctuator(tok3);                  \
      curCharPtr_ += 3;                            \
    } else {                                       \
      token_.setPunctuator(tok2);                  \
      curCharPtr_ += 2;                            \
    }                                              \
    break

    switch ((unsigned char)*curCharPtr_) {
      case 0:
        token_.setStart(curCharPtr_);
        if (curCharPtr_ == bufferEnd_) {
          token_.setEof();
        } else {
          if (!error(
                  token_.getStartLoc(),
                  "unrecognized Unicode character \\u0000")) {
            token_.setEof();
          } else {
            ++curCharPtr_;
            continue;
          }
        }
        break;

        // clang-format off
      PUNC_L1_1('}', TokenKind::r_brace);
      PUNC_L1_1('(', TokenKind::l_paren);
      PUNC_L1_1(')', TokenKind::r_paren);
      PUNC_L1_1('[', TokenKind::l_square);
      PUNC_L1_1(']', TokenKind::r_square);
      PUNC_L1_1(';', TokenKind::semi);
      PUNC_L1_1(',', TokenKind::comma);
      PUNC_L1_1('~', TokenKind::tilde);
      PUNC_L1_1(':', TokenKind::colon);

      // { {|
      case '{':
        token_.setStart(curCharPtr_);
        if (HERMES_PARSE_FLOW &&
            LLVM_UNLIKELY(grammarContext == GrammarContext::Type) &&
            curCharPtr_[1] == '|') {
          token_.setPunctuator(TokenKind::l_bracepipe);
          curCharPtr_ += 2;
        } else {
          token_.setPunctuator(TokenKind::l_brace);
          curCharPtr_ += 1;
        }
        break;

      // = => == ===
      case '=':
        token_.setStart(curCharPtr_);
        if (curCharPtr_[1] == '>') {
          token_.setPunctuator(TokenKind::equalgreater);
          curCharPtr_ += 2;
        } else if (curCharPtr_[1] != '=') {
          token_.setPunctuator(TokenKind::equal);
          curCharPtr_ += 1;
        } else if (curCharPtr_[2] == '=') {
          token_.setPunctuator(TokenKind::equalequalequal);
          curCharPtr_ += 3;
        } else {
          token_.setPunctuator(TokenKind::equalequal);
          curCharPtr_ += 2;
        }
        break;

      // ! != !==
      PUNC_L3_3('!', TokenKind::exclaim, '=', TokenKind::exclaimequal, '=', TokenKind::exclaimequalequal);

      // + ++ +=
      // - -- -=
      // & && &=
      // | || |=
      PUNC_L2_3('+', TokenKind::plus,  '+', TokenKind::plusplus,   '=', TokenKind::plusequal);
      PUNC_L2_3('-', TokenKind::minus, '-', TokenKind::minusminus, '=', TokenKind::minusequal);

      case '&':
        token_.setStart(curCharPtr_);
        if (curCharPtr_[1] == '&') {
          if (curCharPtr_[2] == '=') {
            token_.setPunctuator(TokenKind::ampampequal);
            curCharPtr_ += 3;
          } else {
            token_.setPunctuator(TokenKind::ampamp);
            curCharPtr_ += 2;
          }
        } else if (curCharPtr_[1] == '=') {
          token_.setPunctuator(TokenKind::ampequal);
          curCharPtr_ += 2;
        } else {
          token_.setPunctuator(TokenKind::amp);
          curCharPtr_ += 1;
        }
        break;

      case '|':
        token_.setStart(curCharPtr_);
        if (HERMES_PARSE_FLOW &&
            LLVM_UNLIKELY(grammarContext == GrammarContext::Type) &&
            curCharPtr_[1] == '}') {
          token_.setPunctuator(TokenKind::piper_brace);
          curCharPtr_ += 2;
        } else {
          if (curCharPtr_[1] == '|') {
            if (curCharPtr_[2] == '=') {
              token_.setPunctuator(TokenKind::pipepipeequal);
              curCharPtr_ += 3;
            } else {
              token_.setPunctuator(TokenKind::pipepipe);
              curCharPtr_ += 2;
            }
          } else if (curCharPtr_[1] == '=') {
            token_.setPunctuator(TokenKind::pipeequal);
            curCharPtr_ += 2;
          } else {
            token_.setPunctuator(TokenKind::pipe);
            curCharPtr_ += 1;
          }
        }
        break;

      // ? ?? ?.
      case '?':
        token_.setStart(curCharPtr_);
        if (curCharPtr_[1] == '.' && !isdigit(curCharPtr_[2])) {
          // OptionalChainingPunctuator ::
          // ?. [lookahead does not contain DecimalDigit]
          // This is done to prevent `x?.3:y` from being recognized
          // as `x ?. 3 : y` instead of `x ? .3 : y`.
          token_.setPunctuator(TokenKind::questiondot);
          curCharPtr_ += 2;
        } else if (
            curCharPtr_[1] == '?' &&
            LLVM_LIKELY(grammarContext != GrammarContext::Type)) {
          if (curCharPtr_[2] == '=') {
            token_.setPunctuator(TokenKind::questionquestionequal);
            curCharPtr_ += 3;
          } else {
            token_.setPunctuator(TokenKind::questionquestion);
            curCharPtr_ += 2;
          }
        } else {
          token_.setPunctuator(TokenKind::question);
          curCharPtr_ += 1;
        }
        break;

      // * *= ** **=
      case '*':
        token_.setStart(curCharPtr_);
        if (curCharPtr_[1] == '=') {
          token_.setPunctuator(TokenKind::starequal);
          curCharPtr_ += 2;
        } else if (curCharPtr_[1] != '*') {
          token_.setPunctuator(TokenKind::star);
          curCharPtr_ += 1;
        } else if (curCharPtr_[2] == '=') {
          token_.setPunctuator(TokenKind::starstarequal);
          curCharPtr_ += 3;
        } else {
          token_.setPunctuator(TokenKind::starstar);
          curCharPtr_ += 2;
        }
        break;

        // * *=
        // ^ ^=
        // / /=
        PUNC_L2_2('^', TokenKind::caret, '=', TokenKind::caretequal);

      // % %=
      case '%':
        token_.setStart(curCharPtr_);
        if (HERMES_PARSE_FLOW &&
            LLVM_UNLIKELY(grammarContext == GrammarContext::Type) &&
            curCharPtr_ + 7 <= bufferEnd_ &&
            llvh::StringRef(curCharPtr_, 7) == "%checks") {
          token_.setIdentifier(getStringLiteral("%checks"));
          curCharPtr_ += 7;
        } else if (curCharPtr_[1] == ('=')) {
          token_.setPunctuator(TokenKind::percentequal);
          curCharPtr_ += 2;
        } else {
          token_.setPunctuator(TokenKind::percent);
          curCharPtr_ += 1;
        }
        break;

        // clang-format on

      case '\r':
      case '\n':
        ++curCharPtr_;
        newLineBeforeCurrentToken_ = true;
        continue;

      // Line separator \u2028 UTF8 encoded is      : e2 80 a8
      // Paragraph separator \u2029 UTF8 encoded is : e2 80 a9
      case UTF8_LINE_TERMINATOR_CHAR0:
        if (matchUnicodeLineTerminatorOffset1(curCharPtr_)) {
          curCharPtr_ += 3;
          newLineBeforeCurrentToken_ = true;
          continue;
        } else {
          goto default_label;
        }

      case '\v':
      case '\f':
        ++curCharPtr_;
        continue;

      case '\t':
      case ' ':
        // Spaces frequently come in groups, so use a tight inner loop to skip.
        do
          ++curCharPtr_;
        while (*curCharPtr_ == '\t' || *curCharPtr_ == ' ');
        continue;

      // No-break space \u00A0 is UTF8 encoded as: c2 a0
      case 0xc2:
        if ((unsigned char)curCharPtr_[1] == 0xa0) {
          curCharPtr_ += 2;
          continue;
        } else {
          goto default_label;
        }

      // Byte-order mark \uFEFF is encoded as: ef bb bf
      case 0xef:
        if ((unsigned char)curCharPtr_[1] == 0xbb &&
            (unsigned char)curCharPtr_[2] == 0xbf) {
          curCharPtr_ += 3;
          continue;
        } else {
          goto default_label;
        }

      case '/':
        if (curCharPtr_[1] == '/') { // Line comment?
          scanLineComment(curCharPtr_);
          continue;
        } else if (curCharPtr_[1] == '*') { // Block comment?
          curCharPtr_ = skipBlockComment(curCharPtr_);
          continue;
        } else {
          token_.setStart(curCharPtr_);
          if (grammarContext == AllowRegExp) {
            scanRegExp();
          } else if (curCharPtr_[1] == '=') {
            token_.setPunctuator(TokenKind::slashequal);
            curCharPtr_ += 2;
          } else {
            token_.setPunctuator(TokenKind::slash);
            curCharPtr_ += 1;
          }
        }
        break;

      case '#':
        if (LLVM_UNLIKELY(
                curCharPtr_ == bufferStart_ && curCharPtr_[1] == '!')) {
          // #! (hashbang) at the very start of the buffer.
          scanLineComment(curCharPtr_);
          continue;
        }
        if (!scanPrivateIdentifier()) {
          continue;
        }
        break;

      // <  <= << <<=
      case '<':
        token_.setStart(curCharPtr_);
        if (HERMES_PARSE_FLOW &&
            LLVM_UNLIKELY(grammarContext == JSLexer::GrammarContext::Type)) {
          token_.setPunctuator(TokenKind::less);
          curCharPtr_ += 1;
        } else if (curCharPtr_[1] == '=') {
          token_.setPunctuator(TokenKind::lessequal);
          curCharPtr_ += 2;
        } else if (curCharPtr_[1] == '<') {
          if (curCharPtr_[2] == '=') {
            token_.setPunctuator(TokenKind::lesslessequal);
            curCharPtr_ += 3;
          } else {
            token_.setPunctuator(TokenKind::lessless);
            curCharPtr_ += 2;
          }
        } else {
          token_.setPunctuator(TokenKind::less);
          curCharPtr_ += 1;
        }
        break;

      // > >= >> >>> >>= >>>=
      case '>':
        token_.setStart(curCharPtr_);
        if ((HERMES_PARSE_FLOW &&
             LLVM_UNLIKELY(grammarContext == JSLexer::GrammarContext::Type)) ||
            (HERMES_PARSE_JSX &&
             LLVM_UNLIKELY(
                 grammarContext ==
                 JSLexer::GrammarContext::AllowJSXIdentifier))) {
          token_.setPunctuator(TokenKind::greater);
          curCharPtr_ += 1;
        } else if (curCharPtr_[1] == '=') { // >=
          token_.setPunctuator(TokenKind::greaterequal);
          curCharPtr_ += 2;
        } else if (curCharPtr_[1] == '>') { // >>
          if (curCharPtr_[2] == '=') { // >>=
            token_.setPunctuator(TokenKind::greatergreaterequal);
            curCharPtr_ += 3;
          } else if (curCharPtr_[2] == '>') { // >>>
            if (curCharPtr_[3] == '=') { // >>>=
              token_.setPunctuator(TokenKind::greatergreatergreaterequal);
              curCharPtr_ += 4;
            } else {
              token_.setPunctuator(TokenKind::greatergreatergreater);
              curCharPtr_ += 3;
            }
          } else {
            token_.setPunctuator(TokenKind::greatergreater);
            curCharPtr_ += 2;
          }
        } else {
          token_.setPunctuator(TokenKind::greater);
          curCharPtr_ += 1;
        }
        break;

      case '.':
        token_.setStart(curCharPtr_);
        if (curCharPtr_[1] >= '0' && curCharPtr_[1] <= '9') {
          scanNumber(grammarContext);
        } else if (curCharPtr_[1] == '.' && curCharPtr_[2] == '.') {
          token_.setPunctuator(TokenKind::dotdotdot);
          curCharPtr_ += 3;
        } else {
          token_.setPunctuator(TokenKind::period);
          ++curCharPtr_;
        }
        break;

        // clang-format off
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        // clang-format on
        token_.setStart(curCharPtr_);
        scanNumber(grammarContext);
        break;

        // clang-format off
      case '_': case '$':
      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
      case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
      case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
      case 'v': case 'w': case 'x': case 'y': case 'z':
      case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
      case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
      case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
      case 'V': case 'W': case 'X': case 'Y': case 'Z':
        // clang-format on
        token_.setStart(curCharPtr_);
        scanIdentifierFastPathInContext(curCharPtr_, grammarContext);
        break;

      case '@':
        token_.setStart(curCharPtr_);
        if (HERMES_PARSE_FLOW &&
            LLVM_UNLIKELY(grammarContext == GrammarContext::Type)) {
          scanIdentifierFastPathInContext(curCharPtr_, grammarContext);
        } else {
          curCharPtr_ += 1;
          errorRange(token_.getStartLoc(), "unrecognized character '@'");
          continue;
        }
        break;

      case '\\': {
        token_.setStart(curCharPtr_);
        tmpStorage_.clear();
        uint32_t cp = consumeUnicodeEscape();
        if (!isUnicodeIdentifierStart(cp)) {
          errorRange(
              token_.getStartLoc(),
              "Unicode escape \\u" + Twine::utohexstr(cp) +
                  " is not a valid identifier start");
          continue;
        } else {
          appendUnicodeToStorage(cp);
        }
        scanIdentifierPartsInContext(grammarContext);
        break;
      }

      case '\'':
      case '"':
        token_.setStart(curCharPtr_);
        scanStringInContext(grammarContext);
        break;

      case '`':
        token_.setStart(curCharPtr_);
        scanTemplateLiteral();
        break;

      default_label:
      default: {
        token_.setStart(curCharPtr_);
        uint32_t ch = decodeUTF8();

        if (isUnicodeOnlyLetter(ch)) {
          tmpStorage_.clear();
          appendUnicodeToStorage(ch);
          scanIdentifierPartsInContext(grammarContext);
        } else if (isUnicodeOnlySpace(ch)) {
          continue;
        } else {
          if (ch > 31 && ch < 127)
            errorRange(
                token_.getStartLoc(),
                "unrecognized character '" + Twine((char)ch) + "'");
          else
            errorRange(
                token_.getStartLoc(),
                "unrecognized Unicode character \\u" + Twine::utohexstr(ch));
          continue;
        }

        break;
      }
    }

    // Always terminate the loop unless "continue" was used.
    break;
  } // for(;;)

  finishToken(curCharPtr_);

  return &token_;
}

#if HERMES_PARSE_JSX

const Token *JSLexer::advanceInJSXChild() {
  token_.setStart(curCharPtr_);
  for (;;) {
    assert(curCharPtr_ <= bufferEnd_ && "lexing past end of input");
    switch (*curCharPtr_) {
      PUNC_L1_1('{', TokenKind::l_brace);
      PUNC_L1_1('<', TokenKind::less);

      case 0:
        if (curCharPtr_ == bufferEnd_) {
          token_.setEof();
          break;
        }
        // Fall-through to start scanning text.
        LLVM_FALLTHROUGH;

      default: {
        const char *start = curCharPtr_;
        token_.setStart(start);

        // Build up cooked value using XHTML entities
        tmpStorage_.clear();
        rawStorage_.clear();
        for (;;) {
          char c = *curCharPtr_;

          if (LLVM_UNLIKELY(isUTF8Start(*curCharPtr_))) {
            uint32_t codepoint = _decodeUTF8SlowPath(curCharPtr_);
            appendUnicodeToStorage(codepoint);
            appendUnicodeToStorage(codepoint, rawStorage_);
            continue;
          } else if (c == '&') {
            const char *htmlStart = curCharPtr_;
            auto codePoint = consumeHTMLEntityOptional();
            if (codePoint.hasValue()) {
              appendUnicodeToStorage(*codePoint);
              rawStorage_.append(
                  {htmlStart, (size_t)(curCharPtr_ - htmlStart)});
              continue;
            }
          } else if (
              (c == 0 && curCharPtr_ == bufferEnd_) || c == '{' || c == '<') {
            token_.setJSXText(
                getStringLiteral(tmpStorage_.str()),
                getStringLiteral(rawStorage_.str()));
            break;
          }
          tmpStorage_.push_back(c);
          rawStorage_.push_back(c);
          ++curCharPtr_;
        }
        break;
      }
    }

    // Always terminate the loop unless "continue" was used.
    break;
  }
  finishToken(curCharPtr_);
  return &token_;
}

llvh::Optional<uint32_t> JSLexer::consumeHTMLEntityOptional() {
  assert(*curCharPtr_ == '&');
  const char *start = curCharPtr_;

  if (curCharPtr_[1] == '#') {
    if (curCharPtr_[2] == 'x') {
      // HTML entity with form &#xHEX>;
      curCharPtr_ += 3;
      const char *numberStart = curCharPtr_;

      uint32_t codePoint = 0;
      char ch = *curCharPtr_;

      // Calculate code point from non-empty sequence of hex digits followed by
      // a semicolon.
      for (;;) {
        if (ch == ';' && curCharPtr_ != numberStart) {
          curCharPtr_++;
          return codePoint;
        } else if (isdigit(ch)) {
          ch -= '0';
        } else {
          ch |= 32;
          if (ch >= 'a' && ch <= 'f') {
            ch -= 'a' - 10;
          } else {
            break;
          }
        }

        // Check that this number is representable as a code point
        codePoint = (codePoint << 4) + ch;
        if (codePoint > UNICODE_MAX_VALUE) {
          break;
        }

        ++curCharPtr_;
        ch = *curCharPtr_;
      }
    } else {
      // HTML entity with form &#NUMBER;
      curCharPtr_ += 2;
      const char *numberStart = curCharPtr_;

      uint32_t codePoint = 0;
      char ch = *curCharPtr_;

      // Calculate code point from non-empty sequence of decimal digits followed
      // by a semicolon.
      for (;;) {
        if (ch == ';' && curCharPtr_ != numberStart) {
          curCharPtr_++;
          return codePoint;
        } else if (isdigit(ch)) {
          // Check that this number is representable as a code point
          codePoint = codePoint * 10 + (ch - '0');
          if (codePoint > UNICODE_MAX_VALUE) {
            break;
          }
        } else {
          break;
        }

        ++curCharPtr_;
        ch = *curCharPtr_;
      }
    }
  } else {
    // HTML entity with form &NAME;
    ++curCharPtr_;

    // Gather HTML entity name and lookup name in table. HTML entity names are
    // composed of a sequence of up to 8 alphanumeric characters followed by a
    // semicolon. To minimize backtracking due to an `&` without a following
    // semicolon we only need to look at most 9 characters ahead (8 for the
    // name, 1 for the semicolon).
    for (int i = 0; i < 9; i++) {
      char ch = *curCharPtr_;
      if (ch == ';') {
        auto it = htmlEntities_.find(StringRef(curCharPtr_ - i, i));
        if (it == htmlEntities_.end()) {
          break;
        }

        curCharPtr_++;
        return it->second;
      } else if (((ch | 32) >= 'a' && (ch | 32) <= 'z') || isdigit(ch)) {
        ++curCharPtr_;
      } else {
        break;
      }
    }
  }

  curCharPtr_ = start;
  return llvh::None;
}

#endif

bool JSLexer::isCurrentTokenADirective() {
  // The current token must be a string literal without escapes.
  if (token_.getKind() != TokenKind::string_literal ||
      token_.getStringLiteralContainsEscapes()) {
    return false;
  }

  const char *ptr = curCharPtr_;

  // A directive is a string literal (the current token, directly behind
  // curCharPtr_), followed by a semicolon, new line, or eof that we will now
  // try to find. There can also be comments. So, we loop, consuming whitespace
  // until we encounter:
  // - EOF. Don't consume it and succeed.
  // - Semicolon. Don't consume it and succeed.
  // - Right brace. Don't consume it and succeed.
  // - A new line. Don't consume it and succeed.
  // - A line comment. It implies a new line. Don't consume it and succeed.
  // - A block comment. Consume it and continue.
  // - Anything else. We consume nothing and fail.

  for (;;) {
    assert(ptr <= bufferEnd_ && "lexing past end of input");

    switch (*((const unsigned char *)ptr)) {
      case 0:
        // EOF?
        if (ptr == bufferEnd_)
          return true;
        // We encountered a stray 0 character.
        return false;

      case ';':
      case '}':
        return true;

      case '\r':
      case '\n':
        return true;

      // Line separator \u2028 UTF8 encoded is      : e2 80 a8
      // Paragraph separator \u2029 UTF8 encoded is : e2 80 a9
      case UTF8_LINE_TERMINATOR_CHAR0:
        if (matchUnicodeLineTerminatorOffset1(ptr))
          return true;
        return false;

      case '\v':
      case '\f':
        // Skip whitespace.
        ++ptr;
        continue;

      case '\t':
      case ' ':
        // Spaces frequently come in groups, so use a tight inner loop to skip.
        do
          ++ptr;
        while (*ptr == '\t' || *ptr == ' ');
        continue;

      // No-break space \u00A0 is UTF8 encoded as: c2 a0
      case 0xc2:
        if ((unsigned char)ptr[1] == 0xa0) {
          ptr += 2;
          continue;
        } else {
          goto default_label;
        }

      // Byte-order mark \uFEFF is encoded as: ef bb bf
      case 0xef:
        if ((unsigned char)ptr[1] == 0xbb && (unsigned char)ptr[2] == 0xbf) {
          ptr += 3;
          continue;
        } else {
          goto default_label;
        }

      case '/':
        if (ptr[1] == '/') { // Line comment?
          // It implies a new line, so we are good.
          return true;
        } else if (ptr[1] == '*') { // Block comment?
          auto savedCommentStorageSize = commentStorage_.size();
          auto commentScope = llvh::make_scope_exit([&] {
            if (storeComments_)
              commentStorage_.erase(
                  commentStorage_.begin() + savedCommentStorageSize,
                  commentStorage_.end());
          });
          SourceErrorManager::SaveAndSuppressMessages suppress(&sm_);
          ptr = skipBlockComment(ptr);
          continue;
        } else {
          return false;
        }

      // Handle all other characters: if it is a unicode space, skip it.
      // Otherwise we have failed.
      default_label:
      default: {
        if (hermes::isUTF8Start(*ptr)) {
          auto peeked = _peekUTF8(ptr);
          if (isUnicodeOnlySpace(peeked.first)) {
            ptr = peeked.second;
            continue;
          }
        }
        return false;
      }
    }
  }

  // We arrive here if we matched a directive. 'ptr' is the final character.
  return true;
}

const Token *JSLexer::rescanRBraceInTemplateLiteral() {
  assert(token_.getKind() == TokenKind::r_brace && "need } to rescan");
  --curCharPtr_;
  // Undo the storage for the '}'.
  if (LLVM_UNLIKELY(storeTokens_)) {
    tokenStorage_.pop_back();
  }
  assert(*curCharPtr_ == '}' && "non-} was scanned as r_brace");
  token_.setStart(curCharPtr_);
  scanTemplateLiteral();
  finishToken(curCharPtr_);
  return &token_;
}

OptValue<TokenKind> JSLexer::lookahead1(OptValue<TokenKind> expectedToken) {
  assert(
      (token_.getKind() == TokenKind::identifier || token_.isResWord()) &&
      "unsupported current token");
  UniqueString *savedIdent = token_.getResWordOrIdentifier();
  TokenKind savedKind = token_.getKind();
  SMLoc start = token_.getStartLoc();
  SMLoc end = token_.getEndLoc();
  const char *cur = curCharPtr_;
  SourceErrorManager::SaveAndSuppressMessages suppress(&sm_);

  // Remove any comments that were stored during the lookahead
  auto savedCommentStorageSize = commentStorage_.size();
  auto commentScope = llvh::make_scope_exit([&] {
    if (storeComments_)
      commentStorage_.erase(
          commentStorage_.begin() + savedCommentStorageSize,
          commentStorage_.end());
  });

  advance();
  OptValue<TokenKind> kind = token_.getKind();
  if (isNewLineBeforeCurrentToken()) {
    // Disregard anything after LineTerminator.
    kind = llvh::None;
  } else if (expectedToken == kind) {
    // Do not move the cursor back.
    return kind;
  }

  token_.setStart(start.getPointer());
  token_.setEnd(end.getPointer());
  if (savedKind == TokenKind::identifier) {
    token_.setIdentifier(savedIdent);
  } else {
    token_.setResWord(savedKind, savedIdent);
  }
  seek(SMLoc::getFromPointer(cur));

  // Undo the storage for the token we just advanced to.
  if (LLVM_UNLIKELY(storeTokens_)) {
    tokenStorage_.pop_back();
  }

  return kind;
}

uint32_t JSLexer::consumeUnicodeEscape() {
  assert(*curCharPtr_ == '\\');
  ++curCharPtr_;

  if (*curCharPtr_ != 'u') {
    error(
        {SMLoc::getFromPointer(curCharPtr_ - 1),
         SMLoc::getFromPointer(curCharPtr_ + 1)},
        "invalid Unicode escape");
    return UNICODE_REPLACEMENT_CHARACTER;
  }
  ++curCharPtr_;

  if (*curCharPtr_ == '{') {
    auto cp = consumeBracedCodePoint();
    if (!cp.hasValue()) {
      // consumeBracedCodePoint has reported an error.
      return UNICODE_REPLACEMENT_CHARACTER;
    }
    return *cp;
  }

  auto cp = consumeHex(4);
  if (!cp)
    return UNICODE_REPLACEMENT_CHARACTER;

  // We don't need t check for valid UTF-16. JavaScript allows invalid surrogate
  // pairs, so we just encode every UTF-16 code into a UTF-8 sequence, even
  // though theoretically it is not a valid UTF-8. (UTF-8 would be "valid" if we
  // collected the surrogate pair, decoded it into UTF-32 and encoded that into
  // UTF-16).
  return cp.getValue();
}

llvh::Optional<uint32_t> JSLexer::consumeUnicodeEscapeOptional() {
  const char *start = curCharPtr_;
  assert(*curCharPtr_ == '\\');
  ++curCharPtr_;

  if (*curCharPtr_ != 'u') {
    curCharPtr_ = start;
    return llvh::None;
  }
  ++curCharPtr_;

  if (*curCharPtr_ == '{') {
    // Avoid reporting an error because we are consuming the escape optionally.
    auto cp = consumeBracedCodePoint(false);
    if (!cp) {
      curCharPtr_ = start;
      return llvh::None;
    }
    return *cp;
  }

  auto cp = consumeHex(4, false);
  if (!cp) {
    curCharPtr_ = start;
    return llvh::None;
  }

  // We don't need t check for valid UTF-16. JavaScript allows invalid surrogate
  // pairs, so we just encode every UTF-16 code into a UTF-8 sequence, even
  // though theoretically it is not a valid UTF-8. (UTF-8 would be "valid" if we
  // collected the surrogate pair, decoded it into UTF-32 and encoded that into
  // UTF-16).
  return cp.getValue();
}

bool JSLexer::consumeIdentifierStart() {
  if (*curCharPtr_ == '_' || *curCharPtr_ == '$' ||
      ((*curCharPtr_ | 32) >= 'a' && (*curCharPtr_ | 32) <= 'z')) {
    tmpStorage_.clear();
    tmpStorage_.push_back(*curCharPtr_++);
    return true;
  }

  if (*curCharPtr_ == '\\') {
    SMLoc startLoc = SMLoc::getFromPointer(curCharPtr_);
    tmpStorage_.clear();
    uint32_t cp = consumeUnicodeEscape();
    if (!isUnicodeIdentifierStart(cp)) {
      errorRange(
          startLoc,
          "Unicode escape \\u" + Twine::utohexstr(cp) +
              "is not a valid identifier start");
    } else {
      appendUnicodeToStorage(cp);
    }
    return true;
  }

  if (LLVM_LIKELY(!isUTF8Start(*curCharPtr_)))
    return false;

  auto decoded = _peekUTF8();
  if (isUnicodeIdentifierStart(decoded.first)) {
    tmpStorage_.clear();
    appendUnicodeToStorage(decoded.first);
    curCharPtr_ = decoded.second;
    return true;
  }

  return false;
}

template <JSLexer::IdentifierMode Mode>
bool JSLexer::consumeOneIdentifierPartNoEscape() {
  char ch = *curCharPtr_;
  if (ch == '_' || ch == '$' || ((ch | 32) >= 'a' && (ch | 32) <= 'z') ||
      (ch >= '0' && ch <= '9') || (Mode == IdentifierMode::JSX && ch == '-') ||
      (Mode == IdentifierMode::Flow && ch == '@')) {
    tmpStorage_.push_back(*curCharPtr_++);
    return true;
  } else if (LLVM_UNLIKELY(isUTF8Start(ch))) {
    // If we have encountered a Unicode character, we try to decode it. If it
    // can be a part of the identifier, we consume it, otherwise we leave it
    // alone.
    auto decoded = _peekUTF8();
    if (isUnicodeIdentifierPart(decoded.first)) {
      appendUnicodeToStorage(decoded.first);
      curCharPtr_ = decoded.second;
      return true;
    }
  }
  return false;
}

template <JSLexer::IdentifierMode Mode>
void JSLexer::consumeIdentifierParts() {
  for (;;) {
    // Try consuming an non-escaped identifier part. Failing that, check for an
    // escape.
    if (consumeOneIdentifierPartNoEscape<Mode>())
      continue;
    else if (*curCharPtr_ == '\\') {
      // Decode the escape.
      SMLoc startLoc = SMLoc::getFromPointer(curCharPtr_);
      uint32_t cp = consumeUnicodeEscape();
      if (!isUnicodeIdentifierPart(cp)) {
        errorRange(
            startLoc,
            "Unicode escape \\u" + Twine::utohexstr(cp) +
                "is not a valid identifier codepoint");
      } else {
        appendUnicodeToStorage(cp);
      }
    } else
      break;
  }
}

unsigned char JSLexer::consumeOctal(unsigned maxLen) {
  assert(*curCharPtr_ >= '0' && *curCharPtr_ <= '7');

  if (strictMode_) {
    if (!error(
            SMLoc::getFromPointer(curCharPtr_ - 1),
            "octals not allowed in strict mode")) {
      return 0;
    }
  }

  auto res = (unsigned char)(*curCharPtr_++ - '0');
  while (--maxLen && *curCharPtr_ >= '0' && *curCharPtr_ <= '7')
    res = (res << 3) + *curCharPtr_++ - '0';

  return res;
}

llvh::Optional<uint32_t> JSLexer::consumeHex(
    unsigned requiredLen,
    bool errorOnFail) {
  uint32_t cp = 0;
  for (unsigned i = 0; i != requiredLen; ++i) {
    unsigned ch = *curCharPtr_;
    if (ch >= '0' && ch <= '9') {
      ch -= '0';
    } else {
      // Now that we know it is not a digit, it is safe to lowercase.
      ch |= 32;
      if (ch >= 'a' && ch <= 'f') {
        ch -= 'a' - 10;
      } else {
        if (errorOnFail) {
          error(SMLoc::getFromPointer(curCharPtr_), "invalid hex number");
        }
        return llvh::None;
      }
    }
    cp = (cp << 4) + ch;
    ++curCharPtr_;
  }

  return cp;
}

llvh::Optional<uint32_t> JSLexer::consumeBracedCodePoint(bool errorOnFail) {
  assert(*curCharPtr_ == '{' && "braced codepoint must begin with {");
  ++curCharPtr_;
  const char *start = curCharPtr_;

  // Set to true if we failed to get a code point that is in bounds or saw
  // an invalid character.
  bool failed = false;

  // Loop until we hit the } or eof, max out the value, or see an invalid char.
  uint32_t cp = 0;
  for (; *curCharPtr_ != '}'; ++curCharPtr_) {
    int ch = *curCharPtr_;
    if (ch >= '0' && ch <= '9') {
      ch -= '0';
    } else if (ch >= 'a' && ch <= 'f') {
      ch -= 'a' - 10;
    } else if (ch >= 'A' && ch <= 'F') {
      ch -= 'A' - 10;
    } else {
      // The only way this can be the end of the buffer is if this is a \0.
      // Check if this is the end of the buffer, else continue so that we
      // may report more errors after this braced code point.
      if (curCharPtr_ == bufferEnd_) {
        if (!failed && errorOnFail) {
          error(
              SMLoc::getFromPointer(start),
              "non-terminated unicode codepoint escape");
        }
        return llvh::None;
      }
      // Invalid character, set the failed flag and continue.
      if (!failed && errorOnFail) {
        if (!error(
                SMLoc::getFromPointer(curCharPtr_),
                "invalid character in unicode codepoint escape")) {
          return llvh::None;
        }
      }
      failed = true;
      continue;
    }
    cp = (cp << 4) + ch;
    if (cp > UNICODE_MAX_VALUE) {
      // Number grew too big, set the failed flag and continue.
      if (!failed && errorOnFail) {
        if (!error(
                SMLoc::getFromPointer(start),
                "unicode codepoint escape is too large")) {
          return llvh::None;
        }
      }
      failed = true;
    }
  }

  assert(curCharPtr_ < bufferEnd_ && "bufferEnd_ should cause early return");

  // An empty escape sequence is invalid.
  if (curCharPtr_ == start) {
    if (!failed && errorOnFail) {
      if (!error(
              SMLoc::getFromPointer(start), "empty unicode codepoint escape")) {
        return llvh::None;
      }
    }
    failed = true;
  }

  // Consume the final } and return.
  ++curCharPtr_;
  return failed ? llvh::None : llvh::Optional<uint32_t>{cp};
}

llvh::StringRef JSLexer::lineCommentHelper(const char *start) {
  assert(
      (start[0] == '/' && start[1] == '/') ||
      (start[0] == '#' && start[1] == '!'));
  const char *lineCommentEnd;
  const char *cur = start + 2;

  for (;;) {
    switch ((unsigned char)*cur) {
      case 0:
        if (cur == bufferEnd_) {
          lineCommentEnd = cur;
          goto endLoop;
        } else {
          ++cur;
        }
        break;

      case '\r':
      case '\n':
        lineCommentEnd = cur;
        ++cur;
        newLineBeforeCurrentToken_ = true;
        goto endLoop;

        // Line separator \u2028 UTF8 encoded is      : e2 80 a8
        // Paragraph separator \u2029 UTF8 encoded is: e2 80 a9
      case UTF8_LINE_TERMINATOR_CHAR0:
        if (matchUnicodeLineTerminatorOffset1(cur)) {
          lineCommentEnd = cur;
          cur += 3;
          newLineBeforeCurrentToken_ = true;
          goto endLoop;
        } else {
          _decodeUTF8SlowPath(cur);
        }
        break;

      default:
        if (LLVM_UNLIKELY(isUTF8Start(*cur)))
          _decodeUTF8SlowPath(cur);
        else
          ++cur;
        break;
    }
  }
endLoop:

  curCharPtr_ = cur;
  return llvh::StringRef(start, lineCommentEnd - start);
}

void JSLexer::scanLineComment(const char *start) {
  llvh::StringRef comment = lineCommentHelper(start);

  if (storeComments_) {
    commentStorage_.emplace_back(
        start[0] == '/' ? StoredComment::Kind::Line
                        : StoredComment::Kind::Hashbang,
        SMRange{
            SMLoc::getFromPointer(comment.begin()),
            SMLoc::getFromPointer(comment.end())});
  }

  // Check for magic comments, which excludes #!.
  // Syntax is //# name=value
  if (!comment.consume_front(llvh::StringLiteral("//# ")))
    return;

  if (comment.consume_front(llvh::StringLiteral("sourceURL=")))
    sm_.setSourceUrl(bufId_, comment);
  else if (comment.consume_front(llvh::StringLiteral("sourceMappingURL=")))
    sm_.setSourceMappingUrl(bufId_, comment);
}

const char *JSLexer::skipBlockComment(const char *start) {
  assert(start[0] == '/' && start[1] == '*');
  SMLoc blockCommentStart = SMLoc::getFromPointer(start);
  const char *cur = start + 2;

  for (;;) {
    switch ((unsigned char)*cur) {
      case 0:
        if (cur == bufferEnd_) {
          error(SMLoc::getFromPointer(cur), "non-terminated block comment");
          sm_.note(blockCommentStart, "comment started here");
          goto endLoop;
        } else {
          ++cur;
        }
        break;

      case '\r':
      case '\n':
        ++cur;
        newLineBeforeCurrentToken_ = true;
        break;

      // Line separator \u2028 UTF8 encoded is      : e2 80 a8
      // Paragraph separator \u2029 UTF8 encoded is: e2 80 a9
      case UTF8_LINE_TERMINATOR_CHAR0:
        if (matchUnicodeLineTerminatorOffset1(cur)) {
          cur += 3;
          newLineBeforeCurrentToken_ = true;
        } else {
          _decodeUTF8SlowPath(cur);
        }
        break;

      case '*':
        ++cur;
        if (*cur == '/') {
          ++cur;
          goto endLoop;
        }
        break;

      default:
        if (LLVM_UNLIKELY(isUTF8Start(*cur)))
          _decodeUTF8SlowPath(cur);
        else
          ++cur;
        break;
    }
  }
endLoop:

  if (storeComments_) {
    commentStorage_.emplace_back(
        StoredComment::Kind::Block,
        SMRange{blockCommentStart, SMLoc::getFromPointer(cur)});
  }

  return cur;
}

void JSLexer::scanNumber(GrammarContext grammarContext) {
  // A somewhat ugly state machine for scanning a number

  unsigned radix = 10;
  bool real = false;
  bool ok = true;
  const char *rawStart = curCharPtr_;
  const char *start = curCharPtr_;

  // True when we encounter the numeric literal separator: '_'.
  bool seenSeparator = false;

  // True when we encounter a legacy octal number (starts with '0').
  bool legacyOctal = false;

  // Detect the radix
  if (*curCharPtr_ == '0') {
    if ((curCharPtr_[1] | 32) == 'x') {
      radix = 16;
      curCharPtr_ += 2;
      start += 2;
    } else if ((curCharPtr_[1] | 32) == 'o') {
      radix = 8;
      curCharPtr_ += 2;
      start += 2;
    } else if ((curCharPtr_[1] | 32) == 'b') {
      radix = 2;
      curCharPtr_ += 2;
      start += 2;
    } else if (curCharPtr_[1] == '.') {
      curCharPtr_ += 2;
      goto fraction;
    } else if ((curCharPtr_[1] | 32) == 'e') {
      curCharPtr_ += 2;
      goto exponent;
    } else {
      radix = 8;
      legacyOctal = true;
      ++curCharPtr_;
    }
  }

  while (isdigit(*curCharPtr_) ||
         (radix == 16 && (*curCharPtr_ | 32) >= 'a' &&
          (*curCharPtr_ | 32) <= 'f') ||
         (*curCharPtr_ == '_')) {
    seenSeparator |= *curCharPtr_ == '_';
    ++curCharPtr_;
  }

  if (radix == 10 || legacyOctal) {
    // It is not necessarily an integer.
    // We could have interpreted as legacyOctal initially but will have to
    // change to decimal later.
    if (*curCharPtr_ == '.') {
      ++curCharPtr_;
      goto fraction;
    }

    if ((*curCharPtr_ | 32) == 'e') {
      ++curCharPtr_;
      goto exponent;
    }
  }

  goto end;

fraction:
  // We arrive here after we have consumed the decimal dot ".".
  //
  real = true;
  while (isdigit(*curCharPtr_) || *curCharPtr_ == '_') {
    seenSeparator |= *curCharPtr_ == '_';
    ++curCharPtr_;
  }

  if ((*curCharPtr_ | 32) == 'e') {
    ++curCharPtr_;
    goto exponent;
  } else {
    goto end;
  }

exponent:
  // We arrive here after we have consumed the exponent character 'e' or 'E'.
  //
  real = true;
  if (*curCharPtr_ == '+' || *curCharPtr_ == '-')
    ++curCharPtr_;
  if (isdigit(*curCharPtr_)) {
    do {
      seenSeparator |= *curCharPtr_ == '_';
      ++curCharPtr_;
    } while (isdigit(*curCharPtr_) || *curCharPtr_ == '_');
  } else {
    ok = false;
  }

end:
  // We arrive here after we have consumed all we can from the number. Now,
  // as per the spec, we consume a sequence of identifier characters if they
  // follow directly, which means the number is invalid if it's not BigInt.
  if (consumeIdentifierStart()) {
    consumeIdentifierParts<IdentifierMode::JS>();

    llvh::StringRef raw{rawStart, (size_t)(curCharPtr_ - rawStart)};
    if (ok && !real && (!legacyOctal || raw == "0n") && tmpStorage_ == "n") {
      assert(curCharPtr_ > start && "Must consume at least the trailing n.");
      llvh::ArrayRef<char> digits{start, curCharPtr_ - 1};
      // Use parseIntWithRadixDigits to validate the bigint literal's digits.
      // The digits themselves can be ignored, since we're only interested in
      // whether the string was parsed correctly.
      if (digits.size() &&
          parseIntWithRadixDigits</* AllowNumericSeparator */ true>(
              digits, radix, [](uint8_t) {})) {
        // This is a BigInt.
        rawStorage_.clear();
        rawStorage_.append(raw);
        token_.setBigIntLiteral(getStringLiteral(rawStorage_));
        return;
      }

      // This is a BigInt with invalid digits; fail.
    }

    ok = false;
  }

  double val;

  /// ES6.0 B.1.1
  /// If we encounter a "legacy" octal number (starting with a '0') but if
  /// the integer contains '8' or '9' we interpret it as decimal.
  const auto updateLegacyOctalRadix =
      [this, &radix, start, &legacyOctal]() -> void {
    assert(
        legacyOctal &&
        "updateLegacyOctalRadix can only be called in legacyOctal mode");
    (void)legacyOctal;
    for (auto *scanPtr = start; scanPtr != curCharPtr_; ++scanPtr) {
      if (*scanPtr == '.' || *scanPtr == 'e') {
        break;
      }
      if (LLVM_UNLIKELY(*scanPtr >= '8') && LLVM_LIKELY(*scanPtr != '_')) {
        sm_.warning(
            SMRange(token_.getStartLoc(), SMLoc::getFromPointer(curCharPtr_)),
            "Numeric literal starts with 0 but contains an 8 or 9 digit. "
            "Interpreting as decimal (not octal).");
        radix = 10;
        break;
      }
    }
  };

  if (!ok) {
    errorRange(token_.getStartLoc(), "invalid numeric literal");
    val = std::numeric_limits<double>::quiet_NaN();
  } else if (
      !real && radix == 10 && curCharPtr_ - start <= 9 &&
      LLVM_LIKELY(!seenSeparator)) {
    // If this is a decimal integer of at most 9 digits (log10(2**31-1), it
    // can fit in a 32-bit integer. Use a faster conversion.
    int32_t ival = *start - '0';
    while (++start != curCharPtr_)
      ival = ival * 10 + (*start - '0');
    val = ival;
  } else if (real || radix == 10) {
    if (legacyOctal) {
      if (strictMode_ || grammarContext == GrammarContext::Type) {
        if (!errorRange(
                token_.getStartLoc(),
                "Decimals with leading zeros are not allowed in strict mode")) {
          val = std::numeric_limits<double>::quiet_NaN();
          goto done;
        }
      } else {
        // Check to see if we can actually scan this as radix 10.
        // Non-integer numbers must be in base 10, otherwise we error.
        updateLegacyOctalRadix();
        if (LLVM_LIKELY(radix != 10)) {
          if (!errorRange(
                  token_.getStartLoc(),
                  "Octal numeric literals must be integers")) {
            val = std::numeric_limits<double>::quiet_NaN();
            goto done;
          }
        }
      }
    }

    // We need a zero-terminated buffer for hermes_g_strtod().
    llvh::SmallString<32> buf;
    buf.reserve(curCharPtr_ - start + 1);
    if (LLVM_UNLIKELY(seenSeparator)) {
      for (const char *it = start; it != curCharPtr_; ++it) {
        if (LLVM_LIKELY(*it != '_')) {
          buf.push_back(*it);
        } else {
          // Check to ensure that '_' is surrounded by digits.
          // This is safe because the source buffer is zero-terminated and
          // we know that the numeric literal didn't start with '_'.
          // Note that we could have a 0b_11 literal, but we'd still fail
          // properly because of the radix==16 check.
          char prev = *(it - 1);
          char next = *(it + 1);
          if (!isdigit(prev) &&
              !(radix == 16 && 'a' <= (prev | 32) && (prev | 32) <= 'f')) {
            errorRange(
                token_.getStartLoc(),
                "numeric separator must come after a digit");
          } else if (
              !isdigit(next) &&
              !(radix == 16 && 'a' <= (next | 32) && (next | 32) <= 'f')) {
            errorRange(
                token_.getStartLoc(),
                "numeric separator must come before a digit");
          }
        }
      }
    } else {
      buf.append(start, curCharPtr_);
    }
    buf.push_back(0);
    char *endPtr;
    val = ::hermes_g_strtod(buf.data(), &endPtr);
    if (endPtr != &buf.back()) {
      errorRange(token_.getStartLoc(), "invalid numeric literal");
      val = std::numeric_limits<double>::quiet_NaN();
    }
  } else {
    if (legacyOctal &&
        (strictMode_ || grammarContext == GrammarContext::Type) &&
        curCharPtr_ - start > 1) {
      if (!errorRange(
              token_.getStartLoc(),
              "Octal literals must use '0o' in strict mode")) {
        val = std::numeric_limits<double>::quiet_NaN();
        goto done;
      }
    }

    // Handle the zero-radix case. This could only happen with radix 16
    // because otherwise start wouldn't have been changed.
    if (curCharPtr_ == start) {
      errorRange(
          token_.getStartLoc(),
          llvh::Twine("No digits after ") + StringRef(start - 2, 2));
      val = std::numeric_limits<double>::quiet_NaN();
    } else {
      // Parse the rest of the number:
      if (legacyOctal) {
        updateLegacyOctalRadix();
        // LegacyOctalLikeDecimalIntegerLiteral cannot contain separators.
        if (LLVM_UNLIKELY(seenSeparator)) {
          errorRange(
              token_.getStartLoc(),
              "Numeric separator cannot be used in literal after leading 0");
        }
      }
      auto parsedInt = parseIntWithRadix</* AllowNumericSeparator */ true>(
          llvh::ArrayRef<char>{start, (size_t)(curCharPtr_ - start)}, radix);
      if (!parsedInt) {
        errorRange(token_.getStartLoc(), "invalid integer literal");
        val = std::numeric_limits<double>::quiet_NaN();
      } else {
        val = parsedInt.getValue();
      }
    }
  }

done:
  token_.setNumericLiteral(val);
}

static TokenKind matchReservedWord(const char *str, unsigned len) {
  return llvh::StringSwitch<TokenKind>(StringRef(str, len))
#define RESWORD(name) .Case(#name, TokenKind::rw_##name)
#include "hermes/Parser/TokenKinds.def"
      .Default(TokenKind::identifier);
}

TokenKind JSLexer::scanReservedWord(const char *start, unsigned length) {
  TokenKind rw = matchReservedWord(start, length);

  // Check for "Future reserved words" which should not be recognised in non-
  // strict mode.
  if (!strictMode_ && rw != TokenKind::identifier) {
    switch (rw) {
      case TokenKind::rw_implements:
      case TokenKind::rw_interface:
      case TokenKind::rw_package:
      case TokenKind::rw_private:
      case TokenKind::rw_protected:
      case TokenKind::rw_public:
      case TokenKind::rw_static:
      case TokenKind::rw_yield:
        rw = TokenKind::identifier;
      default:
        break;
    }
  }
  return rw;
}

template <JSLexer::IdentifierMode Mode>
void JSLexer::scanIdentifierFastPath(const char *start) {
  const char *end = start;

  // Quickly consume the ASCII identifier part.
  char ch;
  do
    ch = (unsigned char)*++end;
  while (ch == '_' || ch == '$' || ((ch | 32) >= 'a' && (ch | 32) <= 'z') ||
         (ch >= '0' && ch <= '9') ||
         (Mode == IdentifierMode::JSX && ch == '-') ||
         (Mode == IdentifierMode::Flow && ch == '@'));

  // Check whether a slow part of the identifier follows.
  if (LLVM_UNLIKELY(ch == '\\')) {
    // An escape. Pass the baton to the slow path.
    initStorageWith(start, end);
    curCharPtr_ = end;
    scanIdentifierParts<Mode>();
    return;
  } else if (LLVM_UNLIKELY(isUTF8Start(ch))) {
    // If we have encountered a Unicode character, we try to decode it. If it
    // can be a part of the identifier,
    // we consume it, otherwise we leave it alone.
    auto decoded = _peekUTF8(end);
    if (isUnicodeIdentifierPart(decoded.first)) {
      initStorageWith(start, end);
      appendUnicodeToStorage(decoded.first);
      curCharPtr_ = decoded.second;
      scanIdentifierParts<Mode>();
      return;
    }
  }

  curCharPtr_ = end;

  size_t length = end - start;

  auto rw = scanReservedWord(start, (unsigned)length);
  if (rw != TokenKind::identifier) {
    token_.setResWord(rw, resWordIdent(rw));
  } else {
    token_.setIdentifier(getIdentifier(StringRef(start, length)));
  }
}

template <JSLexer::IdentifierMode Mode>
void JSLexer::scanIdentifierParts() {
  consumeIdentifierParts<Mode>();
  token_.setIdentifier(getIdentifier(tmpStorage_.str()));
}

bool JSLexer::scanPrivateIdentifier() {
  assert(*curCharPtr_ == '#');

  // Skip the '#'.
  const char *start = curCharPtr_;
  ++curCharPtr_;

  // Scan the actual identifier.
  if (LLVM_LIKELY(isASCIIIdentifierStart(*curCharPtr_))) {
    scanIdentifierFastPath<IdentifierMode::JS>(curCharPtr_);
  } else if (consumeIdentifierStart()) {
    // curCharPtr_ has been updated by consumeIdentifierStart.
    scanIdentifierParts<IdentifierMode::JS>();
  } else {
    error(SMLoc::getFromPointer(start), "empty private identifier");
    return false;
  }

  // Reset the start to the '#' because the scanIdentifier functions were
  // not aware of the true start of the token.
  token_.setStart(start);
  // Parsed a resword or identifier.
  // Convert the TokenKind to private_identifier after the fact.
  // This avoids adding another Mode to IdentifierMode.
  token_.setPrivateIdentifier(token_.getResWordOrIdentifier());

  return true;
}

template <bool JSX>
void JSLexer::scanString() {
  assert(*curCharPtr_ == '\'' || *curCharPtr_ == '"');
  char quoteCh = *curCharPtr_++;

  // Track whether we encounter any escapes or new line continuations. We need
  // that information in order to detect directives.
  bool escapes = false;

  tmpStorage_.clear();

  for (;;) {
    if (*curCharPtr_ == quoteCh) {
      ++curCharPtr_;
      break;
    } else if (!JSX && *curCharPtr_ == '\\') {
      escapes = true;
      ++curCharPtr_;
      switch ((unsigned char)*curCharPtr_) {
        case '\'':
        case '"':
        case '\\':
          tmpStorage_.push_back((unsigned char)*curCharPtr_++);
          break;

        case 'b':
          ++curCharPtr_;
          tmpStorage_.push_back(8);
          break;
        case 'f':
          ++curCharPtr_;
          tmpStorage_.push_back(12);
          break;
        case 'n':
          ++curCharPtr_;
          tmpStorage_.push_back(10);
          break;
        case 'r':
          ++curCharPtr_;
          tmpStorage_.push_back(13);
          break;
        case 't':
          ++curCharPtr_;
          tmpStorage_.push_back(9);
          break;
        case 'v':
          ++curCharPtr_;
          tmpStorage_.push_back(11);
          break;

        case '\0': // EOF?
          if (curCharPtr_ == bufferEnd_) { // eof?
            error(SMLoc::getFromPointer(curCharPtr_), "non-terminated string");
            sm_.note(token_.getStartLoc(), "string started here");
            goto breakLoop;
          } else {
            tmpStorage_.push_back((unsigned char)*curCharPtr_++);
          }
          break;

        case '0':
          // '\0' is not an octal so handle it separately.
          if (!(curCharPtr_[1] >= '0' && curCharPtr_[1] <= '7')) {
            ++curCharPtr_;
            appendUnicodeToStorage(0);
            break;
          }
          LLVM_FALLTHROUGH;
        case '1':
        case '2':
        case '3':
          appendUnicodeToStorage(consumeOctal(3));
          break;
        case '4':
        case '5':
        case '6':
        case '7':
          appendUnicodeToStorage(consumeOctal(2));
          break;

        case 'x': {
          ++curCharPtr_;
          auto v = consumeHex(2);
          appendUnicodeToStorage(v ? *v : 0);
          break;
        }

        case 'u':
          --curCharPtr_;
          appendUnicodeToStorage(consumeUnicodeEscape());
          break;

        // Escaped line terminator. We just need to skip it.
        case '\n':
          ++curCharPtr_;
          break;
        case '\r':
          ++curCharPtr_;
          if (*curCharPtr_ == '\n') // skip CR LF
            ++curCharPtr_;
          break;
        case UTF8_LINE_TERMINATOR_CHAR0:
          if (matchUnicodeLineTerminatorOffset1(curCharPtr_)) {
            curCharPtr_ += 3;
            break;
          }
          appendUnicodeToStorage(_decodeUTF8SlowPath(curCharPtr_));
          break;

        default:
          if (LLVM_UNLIKELY(isUTF8Start(*curCharPtr_)))
            appendUnicodeToStorage(_decodeUTF8SlowPath(curCharPtr_));
          else
            tmpStorage_.push_back((unsigned char)*curCharPtr_++);
          break;
      }
    } else if (LLVM_UNLIKELY(*curCharPtr_ == '\n' || *curCharPtr_ == '\r')) {
      if (JSX) {
        tmpStorage_.push_back(*curCharPtr_++);
      } else {
        error(SMLoc::getFromPointer(curCharPtr_), "non-terminated string");
        sm_.note(token_.getStartLoc(), "string started here");
        break;
      }
#if HERMES_PARSE_JSX
    } else if (LLVM_UNLIKELY(JSX && *curCharPtr_ == '&')) {
      auto codePoint = consumeHTMLEntityOptional();
      if (codePoint.hasValue()) {
        appendUnicodeToStorage(*codePoint);
      } else {
        tmpStorage_.push_back(*curCharPtr_++);
      }
#endif
    } else if (LLVM_UNLIKELY(*curCharPtr_ == 0 && curCharPtr_ == bufferEnd_)) {
      error(SMLoc::getFromPointer(curCharPtr_), "non-terminated string");
      sm_.note(token_.getStartLoc(), "string started here");
      break;
    } else {
      if (LLVM_UNLIKELY(isUTF8Start(*curCharPtr_))) {
        // Decode and re-encode the character and append it to the string
        // storage
        appendUnicodeToStorage(_decodeUTF8SlowPath(curCharPtr_));
      } else {
        tmpStorage_.push_back(*curCharPtr_++);
      }
    }
  }
breakLoop:
  token_.setStringLiteral(getStringLiteral(tmpStorage_.str()), escapes);
}

void JSLexer::scanTemplateLiteral() {
  assert(*curCharPtr_ == '`' || *curCharPtr_ == '}');

  // Whether the token will result in TemplateHead upon encountering ${.
  // If we end the literal with `, then the result is NoSubstitutionTemplate,
  // so this will be ignored.
  bool isHead = *curCharPtr_ == '`';

  // If the token ended with a ` then it's a tail (or NoSubstitutionTemplate),
  // and if it ended with a ${ then it's not a tail.
  bool isTail = false;

  // Advance past the initial `.
  ++curCharPtr_;

  // Track whether we encounter any NotEscapeSequence instances,
  // which will be used to error out on non-tagged sequences.
  bool foundNotEscapeSequence = false;

  // Store the Template Value (TV) in the tmpStorage_.
  tmpStorage_.clear();

  // Store the Template Raw Value (TRV) in the rawStorage_.
  rawStorage_.clear();

  /// Return the Template Raw Value (TRV) of character \p c.
  /// The only time the TRV is different from c is when c is a <CR>.
  /// In that case, this function will return 0x0a (LINE FEED).
  const auto trv = [](char c) -> char {
    if (c == '\r') {
      // This case takes \r and \r\n into account.
      // The code below which consumes line separators will skip the following
      // \n if there is a \r\n.
      // For the purposes of finding the TRV it doesn't matter.
      return 0x0a;
    }
    return c;
  };

  for (;;) {
    if (*curCharPtr_ == '`') {
      isTail = true;
      ++curCharPtr_;
      break;
    } else if (*curCharPtr_ == '$' && curCharPtr_[1] == '{') {
      // End of the TemplateCharacters.
      isTail = false;
      curCharPtr_ += 2;
      break;
    } else if (*curCharPtr_ == '\\') {
      rawStorage_.push_back(*curCharPtr_);
      ++curCharPtr_;
      rawStorage_.push_back(trv(*curCharPtr_));
      switch ((unsigned char)*curCharPtr_) {
        case '\'':
        case '"':
        case '\\':
          tmpStorage_.push_back((unsigned char)*curCharPtr_++);
          break;

        case 'b':
          ++curCharPtr_;
          tmpStorage_.push_back(8);
          break;
        case 'f':
          ++curCharPtr_;
          tmpStorage_.push_back(12);
          break;
        case 'n':
          ++curCharPtr_;
          tmpStorage_.push_back(10);
          break;
        case 'r':
          ++curCharPtr_;
          tmpStorage_.push_back(13);
          break;
        case 't':
          ++curCharPtr_;
          tmpStorage_.push_back(9);
          break;
        case 'v':
          ++curCharPtr_;
          tmpStorage_.push_back(11);
          break;

        case '\0': // EOF?
          if (curCharPtr_ == bufferEnd_) { // eof?
            error(
                SMLoc::getFromPointer(curCharPtr_),
                "non-terminated template literal");
            sm_.note(token_.getStartLoc(), "template literal started here");
            goto breakLoop;
          } else {
            tmpStorage_.push_back((unsigned char)*curCharPtr_++);
          }
          break;

        case '0':
          // '\0' is only a valid escape sequence if not followed by a
          // DecimalDigit.
          if (!(curCharPtr_[1] >= '0' && curCharPtr_[1] <= '9')) {
            ++curCharPtr_;
            appendUnicodeToStorage(0);
            break;
          }
          // fall-through

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          // NotEscapeSequence :: DecimalDigit but not 0
          // NotEscapeSequence :: 0 DecimalDigit
          // Octal numbers are not supported in template strings,
          // so leave the number in the raw storage (done above) and move on.
          ++curCharPtr_;
          foundNotEscapeSequence = true;
          break;

        case 'x': {
          ++curCharPtr_;
          const char *start = curCharPtr_;
          auto v = consumeHex(2, false);
          if (!v) {
            foundNotEscapeSequence = true;
          }
          appendUnicodeToStorage(v ? *v : 0);
          rawStorage_.append({start, (size_t)(curCharPtr_ - start)});
          break;
        }

        case 'u': {
          // Pointer to the first character after the 'u', which is where we
          // can continue scanning from if we fail to decode an escape.
          const char *start = curCharPtr_ + 1;
          // Reset the pointer to the '\' to scan the unicode escape.
          --curCharPtr_;
          assert(*curCharPtr_ == '\\' && "must have started with \\");
          auto codepoint = consumeUnicodeEscapeOptional();
          if (!codepoint) {
            foundNotEscapeSequence = true;
            curCharPtr_ = start;
            break;
          }
          appendUnicodeToStorage(*codepoint);
          rawStorage_.append({start, (size_t)(curCharPtr_ - start)});
          break;
        }

        // Escaped line terminator. We just need to skip it, because it was
        // added to the raw storage at the start of the switch statement.
        case '\n':
          ++curCharPtr_;
          break;
        case '\r':
          ++curCharPtr_;
          if (*curCharPtr_ == '\n') // skip CR LF
            ++curCharPtr_;
          break;
        case UTF8_LINE_TERMINATOR_CHAR0: {
          bool isLineTerminator =
              matchUnicodeLineTerminatorOffset1(curCharPtr_);
          uint32_t codepoint = _decodeUTF8SlowPath(curCharPtr_);
          // Needs to be added to the rawStorage_ regardless,
          // but we first need to pop off the byte that was added prior to the
          // switch statement.
          rawStorage_.pop_back();
          appendUnicodeToStorage(codepoint, rawStorage_);
          if (!isLineTerminator) {
            // Only add the codepoint to the tmpStorage if it wasn't a line
            // terminator.
            appendUnicodeToStorage(codepoint);
          }
          break;
        }

        default:
          if (LLVM_UNLIKELY(isUTF8Start(*curCharPtr_))) {
            uint32_t codepoint = _decodeUTF8SlowPath(curCharPtr_);
            appendUnicodeToStorage(codepoint);
            // Remove the last byte from rawStorage_ and then append the
            // unicode codepoint to it. The already inserted byte will change
            // if this codepoint is in Supplementary Planes.
            rawStorage_.pop_back();
            appendUnicodeToStorage(codepoint, rawStorage_);
          } else {
            // The TV of EscapeSequence is the SV of EscapeSequence.
            tmpStorage_.push_back((unsigned char)*curCharPtr_++);
          }
          break;
      }
    } else if (LLVM_UNLIKELY(*curCharPtr_ == 0 && curCharPtr_ == bufferEnd_)) {
      error(
          SMLoc::getFromPointer(curCharPtr_),
          "non-terminated template literal");
      sm_.note(token_.getStartLoc(), "template literal started here");
      break;
    } else if (*curCharPtr_ == '\r') {
      // The TV of LineTerminatorSequence is the TRV of
      // LineTerminatorSequence. The only time this differs from the same
      // characters as the bytes in the file is when the sequence begins with
      // a <CR>.
      tmpStorage_.push_back(trv(*curCharPtr_));
      rawStorage_.push_back(trv(*curCharPtr_));
      curCharPtr_++;
      if (*curCharPtr_ == '\n') {
        // Skip the <CR> <LF>
        curCharPtr_++;
      }
    } else {
      if (LLVM_UNLIKELY(isUTF8Start(*curCharPtr_))) {
        // Decode and re-encode the character and append it to the string
        // storage
        uint32_t codepoint = _decodeUTF8SlowPath(curCharPtr_);
        appendUnicodeToStorage(codepoint);
        appendUnicodeToStorage(codepoint, rawStorage_);
      } else {
        rawStorage_.push_back(*curCharPtr_);
        tmpStorage_.push_back(*curCharPtr_++);
      }
    }
  }
breakLoop:
  // If the template literal is tagged and contains invalid escapes, then
  // cooked should be null because there is no way to cook it, per the ESTree
  // 2018 spec. The parser will error when encountering an untagged literal
  // with invalid escapes, so we place nullptr here.
  UniqueString *cookedStr =
      foundNotEscapeSequence ? nullptr : getStringLiteral(tmpStorage_.str());
  UniqueString *rawStr = getStringLiteral(rawStorage_.str());
  if (isHead) {
    if (isTail) {
      // ` characters `
      token_.setTemplateLiteral(
          TokenKind::no_substitution_template, cookedStr, rawStr);
    } else {
      // ` characters ${
      token_.setTemplateLiteral(TokenKind::template_head, cookedStr, rawStr);
    }
  } else {
    if (isTail) {
      // } characters `
      token_.setTemplateLiteral(TokenKind::template_tail, cookedStr, rawStr);
    } else {
      // } characters ${
      token_.setTemplateLiteral(TokenKind::template_middle, cookedStr, rawStr);
    }
  }
}

/// TODO: this has to be implemented properly.
void JSLexer::scanRegExp() {
  SMLoc startLoc = SMLoc::getFromPointer(curCharPtr_);
  assert(*curCharPtr_ == '/');
  ++curCharPtr_;

  tmpStorage_.clear();
  bool inClass = false;

  for (;;) {
    switch ((unsigned char)*curCharPtr_) {
      case '/':
        if (!inClass) {
          ++curCharPtr_;
          goto exitLoop;
        }
        break;

      case '[':
        inClass = true; // It may be true already, but so what.
        break;

      case ']':
        inClass = false; // It may be false already, but so what.
        break;

      case '\\': // an escape
        tmpStorage_.push_back((unsigned char)*curCharPtr_);
        ++curCharPtr_;
        switch ((unsigned char)*curCharPtr_) {
          case '\0':
            if (curCharPtr_ == bufferEnd_)
              goto unterminated;
            break;
          case UTF8_LINE_TERMINATOR_CHAR0:
            if (matchUnicodeLineTerminatorOffset1(curCharPtr_))
              goto unterminated;
            break;
          case '\n':
          case '\r':
            goto unterminated;
        }
        break;

      case '\0':
        if (curCharPtr_ == bufferEnd_)
          goto unterminated;
        break;
      case UTF8_LINE_TERMINATOR_CHAR0:
        if (matchUnicodeLineTerminatorOffset1(curCharPtr_))
          goto unterminated;
        break;

      case '\n':
      case '\r':
      unterminated:
        error(
            SMLoc::getFromPointer(curCharPtr_),
            "non-terminated regular expression literal");
        sm_.note(startLoc, "regular expression started here");
        goto exitLoop;
    }

    if (LLVM_UNLIKELY(isUTF8Start((unsigned char)*curCharPtr_)))
      appendUnicodeToStorage(_decodeUTF8SlowPath(curCharPtr_));
    else
      tmpStorage_.push_back((unsigned char)*curCharPtr_++);
  }
exitLoop:
  UniqueString *body = getStringLiteral(tmpStorage_.str());

  // Scan the flags. We must not interpret escape sequences.
  // E6 5.1 7.8.5: "The Strings of characters comprising the
  // RegularExpressionBody and the RegularExpressionFlags are passed
  // uninterpreted to the regular expression constructor"
  tmpStorage_.clear();
  bool escapingBackslash = false;
  for (;;) {
    if (consumeOneIdentifierPartNoEscape<IdentifierMode::JS>()) {
      escapingBackslash = false;
      continue;
    } else if (*curCharPtr_ == '\\') {
      tmpStorage_.push_back(*curCharPtr_++);

      // ES6 11.8.5.1: It is a Syntax Error if IdentifierPart contains a
      // Unicode escape sequence.
      escapingBackslash = !escapingBackslash;
      if (escapingBackslash && *curCharPtr_ == 'u') {
        error(
            SMLoc::getFromPointer(curCharPtr_),
            "Unicode escape sequences are not allowed in regular expression flags");
      }
    } else {
      break;
    }
  }

  UniqueString *flags = getStringLiteral(tmpStorage_.str());

  token_.setRegExpLiteral(new (allocator_.Allocate<RegExpLiteral>(1))
                              RegExpLiteral(body, flags));
}

UniqueString *JSLexer::convertSurrogatesInString(StringRef str) {
  llvh::SmallVector<char16_t, 8> ustr;
  ustr.reserve(str.size());
  char16_t *ustrEnd =
      convertUTF8WithSurrogatesToUTF16(ustr.data(), str.begin(), str.end());
  std::string output;
  convertUTF16ToUTF8WithReplacements(
      output, llvh::makeArrayRef(ustr.data(), ustrEnd));
  return strTab_.getString(output);
}

bool JSLexer::error(llvh::SMLoc loc, const llvh::Twine &msg) {
  sm_.error(loc, msg, Subsystem::Lexer);
  if (!sm_.isErrorLimitReached())
    return true;
  forceEOF();
  return false;
}

bool JSLexer::error(llvh::SMRange range, const llvh::Twine &msg) {
  sm_.error(range, msg, Subsystem::Lexer);
  if (!sm_.isErrorLimitReached())
    return true;
  forceEOF();
  return false;
}

bool JSLexer::error(
    llvh::SMLoc loc,
    llvh::SMRange range,
    const llvh::Twine &msg) {
  sm_.error(loc, range, msg, Subsystem::Lexer);
  if (!sm_.isErrorLimitReached())
    return true;
  forceEOF();
  return false;
}

} // namespace parser
} // namespace hermes
