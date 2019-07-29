/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/Parser/JSLexer.h"

#include "DiagContext.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/SmallString.h"

#include "gtest/gtest.h"

using namespace hermes;
using namespace hermes::parser;

using llvm::APFloat;
using llvm::APInt;

namespace {

TEST(JSLexerTest, PunctuatorTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;

  static const char puncts[] =
#define PUNCTUATOR(name, str) str " "
#include "hermes/Parser/TokenKinds.def"
      ;

  JSLexer lex(puncts, sm, alloc);

#define PUNCTUATOR(name, str) \
  ASSERT_EQ(lex.advance(JSLexer::AllowDiv)->getKind(), TokenKind::name);
#include "hermes/Parser/TokenKinds.def"
  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());

  // "/=" and "/" require context or they could be interpreted as a regexp
  // literal
}

TEST(JSLexerTest, PunctuatorDivTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  JSLexer lex("a / b /= c", sm, alloc);

  ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
  ASSERT_EQ(TokenKind::slash, lex.advance(JSLexer::AllowDiv)->getKind());
  ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
  ASSERT_EQ(TokenKind::slashequal, lex.advance(JSLexer::AllowDiv)->getKind());
  ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
}

TEST(JSLexerTest, WhiteSpaceTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;

  JSLexer lex("{  ; \n} \n \n ;", sm, alloc);

  ASSERT_EQ(TokenKind::l_brace, lex.advance()->getKind());
  ASSERT_FALSE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());
  ASSERT_FALSE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(TokenKind::r_brace, lex.advance()->getKind());
  ASSERT_TRUE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());
  ASSERT_TRUE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
  ASSERT_FALSE(lex.isNewLineBeforeCurrentToken());
}

TEST(JSLexerTest, UnicodeWhiteSpaceTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  // Source with unicode whitespace characters.
  JSLexer lex("{\xe2\x80\x80;\xe2\x80\x8a \n} \xe2\x81\x9f\n \n ;", sm, alloc);

  // lexer behavior should be the same as in WhiteSpaceTest
  ASSERT_EQ(TokenKind::l_brace, lex.advance()->getKind());
  ASSERT_FALSE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());
  ASSERT_FALSE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(TokenKind::r_brace, lex.advance()->getKind());
  ASSERT_TRUE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());
  ASSERT_TRUE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
  ASSERT_FALSE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(0, diag.getErrCountClear());
}

TEST(JSLexerTest, CommentTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  JSLexer lex(
      "; /* foo */ { /* bar \n"
      "    *****      */ } // hello\n"
      " /* comment */ ;\n"
      " /* not closed",
      sm,
      alloc);

  ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());
  ASSERT_FALSE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(TokenKind::l_brace, lex.advance()->getKind());
  ASSERT_FALSE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(TokenKind::r_brace, lex.advance()->getKind());
  ASSERT_TRUE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());
  ASSERT_TRUE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
  ASSERT_EQ(1, diag.getErrCountClear()); // comment not closed
  ASSERT_TRUE(lex.isNewLineBeforeCurrentToken());
}

TEST(JSLexerTest, NumberTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;

#define _GEN_TESTS(flt, dec)                                            \
  flt(0) dec(0x10) flt(1.2) dec(055) flt(.1) flt(1.) flt(1e2) flt(5e+3) \
      flt(4e-3) flt(.1e-3) flt(12.34e+5)

#define _MK_STR(num) " " #num

// Bitwise comparison of a floating numeric literal against a
// APFloat(test_input)
#define _FLT(num)                                                        \
  {                                                                      \
    APFloat fval(APFloat::IEEEdouble(), #num);                           \
    llvm::SmallString<16> actual, expected;                              \
                                                                         \
    const Token *tok = lex.advance();                                    \
    ASSERT_EQ(TokenKind::numeric_literal, tok->getKind());               \
                                                                         \
    fval.toString(expected);                                             \
    APFloat(tok->getNumericLiteral()).toString(actual);                  \
    EXPECT_STREQ(expected.c_str(), actual.c_str());                      \
    EXPECT_TRUE(APFloat(tok->getNumericLiteral()).bitwiseIsEqual(fval)); \
  }

// Bitwise comparison of a integer numeric literal against
// APFloat(APInt(test_input))
#define _DEC(num)                                                        \
  {                                                                      \
    APInt ival;                                                          \
    APFloat fval(APFloat::IEEEdouble());                                 \
    llvm::SmallString<16> actual, expected;                              \
                                                                         \
    const Token *tok = lex.advance();                                    \
    ASSERT_EQ(TokenKind::numeric_literal, tok->getKind());               \
                                                                         \
    StringRef(#num).getAsInteger(0, ival);                               \
    fval.convertFromAPInt(ival, false, APFloat::rmNearestTiesToEven);    \
    fval.toString(expected);                                             \
    APFloat(tok->getNumericLiteral()).toString(actual);                  \
    EXPECT_STREQ(expected.c_str(), actual.c_str());                      \
    EXPECT_TRUE(APFloat(tok->getNumericLiteral()).bitwiseIsEqual(fval)); \
  }

  JSLexer lex(_GEN_TESTS(_MK_STR, _MK_STR), sm, alloc, nullptr, false);
  _GEN_TESTS(_FLT, _DEC);
  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());

#undef _DEC
#undef _FLT
#undef _MK_STR
#undef _GEN_TESTS
}

TEST(JSLexerTest, BadNumbersTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  JSLexer lex("123hhhh; 123e ; .4.5", sm, alloc);

  ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
  ASSERT_EQ(1, diag.getErrCountClear());

  ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());

  ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
  ASSERT_EQ(1, diag.getErrCountClear());

  ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());

  ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
  ASSERT_EQ(0, diag.getErrCountClear());
  ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
  ASSERT_EQ(0, diag.getErrCountClear());

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
}

TEST(JSLexerTest, BigIntegerTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  JSLexer lex(
      " 0xFFFFFFFFFFFFFFFF"
      " 99999999999999999999",
      sm,
      alloc);

  // Test more then 52 bits of integer
  {
    ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
    llvm::SmallString<32> actual;
    APFloat(lex.getCurToken()->getNumericLiteral()).toString(actual, 20);
    // We need dtoa.c for the correct string representation
    // EXPECT_STREQ("18446744073709552000", actual.c_str());
    EXPECT_STREQ("18446744073709551616", actual.c_str());
  }

  {
    ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
    llvm::SmallString<32> actual;
    APFloat(lex.getCurToken()->getNumericLiteral()).toString(actual, 30, 30);
    EXPECT_STREQ("100000000000000000000", actual.c_str());
  }

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
}

TEST(JSLexerTest, ZeroRadixTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  JSLexer lex(" 0x", sm, alloc);

  // Test a malformed hex number.
  {
    ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
    ASSERT_EQ(1, diag.getErrCountClear());
  }

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
}

TEST(JSLexerTest, OctalLiteralTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  JSLexer lex("01 010 09 019", sm, alloc);

  auto tok = lex.advance();
  ASSERT_EQ(tok->getKind(), TokenKind::numeric_literal);
  ASSERT_EQ(tok->getNumericLiteral(), 1.0);

  tok = lex.advance();
  ASSERT_EQ(tok->getKind(), TokenKind::numeric_literal);
  ASSERT_EQ(tok->getNumericLiteral(), 8.0);

  ASSERT_EQ(diag.getWarnCountClear(), 0);

  tok = lex.advance();
  ASSERT_EQ(tok->getKind(), TokenKind::numeric_literal);
  ASSERT_EQ(tok->getNumericLiteral(), 9.0);
  ASSERT_EQ(diag.getWarnCountClear(), 1);

  tok = lex.advance();
  ASSERT_EQ(tok->getKind(), TokenKind::numeric_literal);
  ASSERT_EQ(tok->getNumericLiteral(), 19.0);
  ASSERT_EQ(diag.getWarnCountClear(), 1);

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
}

#define LEX_EXPECT_IDENT(s, lex)                              \
  ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind()); \
  EXPECT_STREQ(s, lex.getCurToken()->getIdentifier()->c_str())

TEST(JSLexerTest, SimpleIdentifierTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;

  JSLexer lex("true foo bar foo", sm, alloc);

  ASSERT_EQ(TokenKind::rw_true, lex.advance()->getKind());

  LEX_EXPECT_IDENT("foo", lex);
  UniqueString *foo = lex.getCurToken()->getIdentifier();

  LEX_EXPECT_IDENT("bar", lex);

  LEX_EXPECT_IDENT("foo", lex);
  ASSERT_EQ(foo, lex.getCurToken()->getIdentifier());

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
}

TEST(JSLexerTest, IdentifierTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;

  JSLexer lex(
      " _foo$123"
      " $123"
      " a\\u0061"
      " \\u0061\\u0061"
      " \\u0061a",
      sm,
      alloc);

  LEX_EXPECT_IDENT("_foo$123", lex);
  LEX_EXPECT_IDENT("$123", lex);

  LEX_EXPECT_IDENT("aa", lex);
  UniqueString *aa = lex.getCurToken()->getIdentifier();
  LEX_EXPECT_IDENT("aa", lex);
  ASSERT_EQ(aa, lex.getCurToken()->getIdentifier());
  LEX_EXPECT_IDENT("aa", lex);
  ASSERT_EQ(aa, lex.getCurToken()->getIdentifier());

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
}

TEST(JSLexerTest, StringTest1) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  JSLexer lex(
      "'aa' \"bb\" 'open1\n"
      "'open2\xe2\x80\xa8"
      "\"open3",
      sm,
      alloc);

  ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
  ASSERT_EQ(0, diag.getErrCountClear());
  EXPECT_STREQ("aa", lex.getCurToken()->getStringLiteral()->c_str());

  ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
  ASSERT_EQ(0, diag.getErrCountClear());
  EXPECT_STREQ("bb", lex.getCurToken()->getStringLiteral()->c_str());

  ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
  ASSERT_EQ(1, diag.getErrCountClear());
  EXPECT_STREQ("open1", lex.getCurToken()->getStringLiteral()->c_str());
  ASSERT_FALSE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
  ASSERT_EQ(1, diag.getErrCountClear());
  EXPECT_STREQ("open2", lex.getCurToken()->getStringLiteral()->c_str());
  ASSERT_TRUE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
  ASSERT_EQ(1, diag.getErrCountClear());
  EXPECT_STREQ("open3", lex.getCurToken()->getStringLiteral()->c_str());
  ASSERT_TRUE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
  ASSERT_FALSE(lex.isNewLineBeforeCurrentToken());
}

TEST(JSLexerTest, StringTest2) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  JSLexer lex(
      "'a\\u0061\x62\143' "
      "'\\w\\'\\\"\\b\\f\\n\\r\\t\\v\\\na' "
      "'\\x1g' "
      "'\\u123g'",
      sm,
      alloc);

  ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
  ASSERT_EQ(0, diag.getErrCountClear());
  EXPECT_STREQ("aabc", lex.getCurToken()->getStringLiteral()->c_str());

  ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
  ASSERT_EQ(0, diag.getErrCountClear());
  EXPECT_STREQ(
      "w'\"\b\f\n\r\t\va", lex.getCurToken()->getStringLiteral()->c_str());

  ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
  ASSERT_EQ(1, diag.getErrCountClear());

  ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
  ASSERT_EQ(1, diag.getErrCountClear());

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
}

TEST(JSLexerTest, StringOctalTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  {
    // non-strict mode.
    JSLexer lex("'\\0' '\\000' '\\05'", sm, alloc, nullptr, false);

    ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
    ASSERT_EQ(0, diag.getErrCountClear());

    ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
    ASSERT_EQ(0, diag.getErrCountClear());

    ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
    ASSERT_EQ(0, diag.getErrCountClear());

    ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
  }

  {
    // strict mode.
    JSLexer lex("'\\0' '\\000' '\\05'", sm, alloc, nullptr, true);

    ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
    ASSERT_EQ(0, diag.getErrCountClear());

    ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
    EXPECT_EQ(1, diag.getErrCountClear());

    ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
    EXPECT_EQ(1, diag.getErrCountClear());

    ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
  }
}

TEST(JSLexerTest, UnicodeEscapeTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  {
    JSLexer lex("'\\u0f3b' '\\u{0f3b}' '\\u{0062}'", sm, alloc);

    ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
    ASSERT_EQ(0, diag.getErrCountClear());
    EXPECT_STREQ(
        "\xe0\xbc\xbb", lex.getCurToken()->getStringLiteral()->c_str());

    ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
    ASSERT_EQ(0, diag.getErrCountClear());
    EXPECT_STREQ(
        "\xe0\xbc\xbb", lex.getCurToken()->getStringLiteral()->c_str());

    ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
    ASSERT_EQ(0, diag.getErrCountClear());
    EXPECT_STREQ("\x62", lex.getCurToken()->getStringLiteral()->c_str());

    ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
    ASSERT_EQ(0, diag.getErrCountClear());
  }

  {
    JSLexer lex("'\\u{ffffffff}'", sm, alloc);
    ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
    ASSERT_EQ(1, diag.getErrCountClear());
  }

  {
    JSLexer lex("'\\u{}'", sm, alloc);
    ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
    ASSERT_EQ(1, diag.getErrCountClear());
  }
}

TEST(JSLexerTest, RegexpSmoke) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;

  JSLexer lex("; /aa/bc", sm, alloc);

  ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());

  ASSERT_EQ(
      TokenKind::regexp_literal, lex.advance(JSLexer::AllowRegExp)->getKind());
  EXPECT_STREQ("aa", lex.getCurToken()->getRegExpLiteral()->getBody()->c_str());
  EXPECT_STREQ(
      "bc", lex.getCurToken()->getRegExpLiteral()->getFlags()->c_str());

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
}

TEST(JSLexerTest, RegexpSmoke2) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;

  JSLexer lex("; /(\\w+)\\s(\\w+)/g", sm, alloc);

  ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());

  ASSERT_EQ(
      TokenKind::regexp_literal, lex.advance(JSLexer::AllowRegExp)->getKind());
  EXPECT_STREQ(
      "(\\w+)\\s(\\w+)",
      lex.getCurToken()->getRegExpLiteral()->getBody()->c_str());
  EXPECT_STREQ("g", lex.getCurToken()->getRegExpLiteral()->getFlags()->c_str());

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
}

// Make sure that we *allow* invalid UTF-16 surrogate pairs.
TEST(JSLexerTest, UTF16BadSurrogatePairs) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext ctx(sm);

  JSLexer lex("' \\udc01 \\ud805 '", sm, alloc);

  ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
  EXPECT_STREQ(
      "\x20\xed\xb0\x81\x20\xed\xa0\x85\x20",
      lex.getCurToken()->getStringLiteral()->c_str());

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
  ASSERT_EQ(0, ctx.getErrCountClear());
}

// Make sure '/' can be used in a regexp class
TEST(JSLexerTest, classInRegexp) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext ctx(sm);

  JSLexer lex("/[a/]/", sm, alloc);

  ASSERT_EQ(TokenKind::regexp_literal, lex.advance()->getKind());
  EXPECT_STREQ(
      "[a/]", lex.getCurToken()->getRegExpLiteral()->getBody()->c_str());

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
  ASSERT_EQ(0, ctx.getErrCountClear());
}

// UTF-8-encoded codepoints which are larger than 0xFFFF should be normalized
// into a UTF-8 encoded surrogate pair.
TEST(JSLexerTest, normalizeUTF8) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext ctx(sm);

  JSLexer lex("'\xf0\x90\x80\x81'", sm, alloc);

  ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
  EXPECT_STREQ(
      "\xed\xa0\x80\xed\xb0\x81",
      lex.getCurToken()->getStringLiteral()->c_str());

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
}

TEST(JSLexerTest, reservedTokens) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext ctx(sm);

  static const char str[] =
      "implements private public "
      "interface package protected static "
      "yield";

  // Ensure we recognize the words in strict mode.
  {
    JSLexer lex(str, sm, alloc, nullptr, true);
    ASSERT_EQ(TokenKind::rw_implements, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::rw_private, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::rw_public, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::rw_interface, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::rw_package, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::rw_protected, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::rw_static, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::rw_yield, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
    ASSERT_EQ(0, ctx.getErrCountClear());
  }

  // Ensure we don't recognize the words in strict mode.
  {
    JSLexer lex(str, sm, alloc, nullptr, false);
    ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
    ASSERT_EQ(0, ctx.getErrCountClear());
  }
}

TEST(JSLexerTest, SourceMappingUrl) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext ctx(sm);

  // Ensure it works at the end of a file.
  {
    static const char str[] =
        "var x = 1;"
        "//# sourceMappingURL=localhost:8000/this_is_the_url.map";

    JSLexer lex(str, sm, alloc);
    ASSERT_EQ(TokenKind::rw_var, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::equal, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());
    lex.advance();
    ASSERT_EQ("localhost:8000/this_is_the_url.map", sm.getSourceMappingUrl(1));
  }

  // Ensure it works in the middle of a file.
  {
    static const char str[] =
        "var x = 1;\n"
        "//# sourceMappingURL=second-map.map\n"
        "var y = 2;";

    JSLexer lex(str, sm, alloc);
    ASSERT_EQ(TokenKind::rw_var, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::equal, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());
    lex.advance();
    ASSERT_EQ("second-map.map", sm.getSourceMappingUrl(2));
  }

  // Ensure it doesn't read invalid source map comments.
  {
    static const char str[] =
        "var x = 1;\n"
        "// sourceMappingURL=localhost:8000/this_is_the_url.map\n"
        "//# sourceMappingURL =localhost:8000/this_is_the_url.map\n"
        "//#sourceMappingURL=localhost:8000/this_is_the_url.map\n"
        "//# sourceMappingURL=\nlocalhost:8000/this_is_the_url.map\n";

    JSLexer lex(str, sm, alloc);
    ASSERT_EQ(TokenKind::rw_var, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::equal, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());
    lex.advance();
    lex.advance();
    lex.advance();
    lex.advance();
    ASSERT_TRUE(sm.getSourceMappingUrl(3).empty());
  }

  // Ensure it overwrites the first URL with another one.
  {
    static const char str[] =
        "var x = 1;\n"
        "//# sourceMappingURL=url1\n"
        "//# sourceMappingURL=url2\n";

    JSLexer lex(str, sm, alloc);
    ASSERT_EQ(TokenKind::rw_var, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::equal, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());
    lex.advance();
    lex.advance();
    ASSERT_EQ("url2", sm.getSourceMappingUrl(4));
  }
}

} // namespace
