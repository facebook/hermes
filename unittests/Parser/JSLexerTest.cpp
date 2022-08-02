/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Parser/JSLexer.h"

#include "DiagContext.h"

#include "llvh/ADT/APFloat.h"
#include "llvh/ADT/APInt.h"
#include "llvh/ADT/SmallString.h"

#include "gtest/gtest.h"

using namespace hermes;
using namespace hermes::parser;

using llvh::APFloat;
using llvh::APInt;

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

#ifndef NDEBUG
#define PUNCTUATOR(name, str) ASSERT_TRUE(isPunctuatorDbg(TokenKind::name));
#include "hermes/Parser/TokenKinds.def"
#endif
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

TEST(JSLexerTest, HashbangTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  JSLexer lex(
      "#! hashbang comment\n"
      ";\n"
      "#! // not a hashbang comment\n",
      sm,
      alloc);
  ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());
  ASSERT_TRUE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(TokenKind::exclaim, lex.advance()->getKind());
  ASSERT_EQ(1, diag.getErrCountClear());
  ASSERT_TRUE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
  ASSERT_TRUE(lex.isNewLineBeforeCurrentToken());
}

TEST(JSLexerTest, NumberTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;

#define _GEN_TESTS(flt, dec)                                              \
  flt(1235) flt(1234567890123) flt(0) dec(0x10) flt(1.2) dec(055) flt(.1) \
      flt(1.) flt(1e2) flt(5e+3) flt(4e-3) flt(.1e-3) flt(12.34e+5)

#define _MK_STR(num) " " #num

// Bitwise comparison of a floating numeric literal against a
// APFloat(test_input)
#define _FLT(num)                                                        \
  {                                                                      \
    APFloat fval(APFloat::IEEEdouble(), #num);                           \
    llvh::SmallString<16> actual, expected;                              \
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
    llvh::SmallString<16> actual, expected;                              \
                                                                         \
    const Token *tok = lex.advance();                                    \
    ASSERT_EQ(TokenKind::numeric_literal, tok->getKind());               \
                                                                         \
    llvh::StringRef(#num).getAsInteger(0, ival);                         \
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

TEST(JSLexerTest, NumericSeparatorTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  JSLexer lex(
      " 1_2 12"
      " 0x1_2 0x12"
      " 0xdead_beef 0xdeadbeef"
      " 0b1_1 0b11"
      " 0o1_1 0o11"
      " 123_456_789 123456789"
      " 12_345e1_2 12345e12"
      " 1_1.1_2 1_1.12",
      sm,
      alloc);

  const Token *tok = lex.advance();
  while (tok->getKind() != TokenKind::eof) {
    ASSERT_EQ(TokenKind::numeric_literal, tok->getKind());
    double withSep = tok->getNumericLiteral();

    tok = lex.advance();
    ASSERT_EQ(TokenKind::numeric_literal, tok->getKind());
    double noSep = tok->getNumericLiteral();

    ASSERT_EQ(withSep, noSep);
    tok = lex.advance();
  }
}

#define LEX_EXPECT_BIGINT(s, lex)                                 \
  ASSERT_EQ(TokenKind::bigint_literal, lex.advance()->getKind()); \
  EXPECT_STREQ(s, lex.getCurToken()->getBigIntLiteral()->c_str())

TEST(JSLexerTest, BigIntTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  {
    JSLexer lex(
        " 0n"
        " 1n"
        " 1000n"
        " 1928371289378129381212398n"
        " 0xdeadbeefn"
        " 0b10101100101n",
        sm,
        alloc);

    LEX_EXPECT_BIGINT("0n", lex);
    LEX_EXPECT_BIGINT("1n", lex);
    LEX_EXPECT_BIGINT("1000n", lex);
    LEX_EXPECT_BIGINT("1928371289378129381212398n", lex);
    LEX_EXPECT_BIGINT("0xdeadbeefn", lex);
    LEX_EXPECT_BIGINT("0b10101100101n", lex);
  }

  {
    JSLexer lex("09n", sm, alloc);
    lex.advance();
    ASSERT_EQ(1, diag.getErrCountClear());
  }
  {
    JSLexer lex("1.1n", sm, alloc);
    lex.advance();
    ASSERT_EQ(1, diag.getErrCountClear());
  }
  {
    JSLexer lex("1e2n", sm, alloc);
    lex.advance();
    ASSERT_EQ(1, diag.getErrCountClear());
  }
}

TEST(JSLexerTest, BadNumbersTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  JSLexer lex(
      "123hhhh; 123e ; .4.5 ; 0_7 1__23 0b_11 123_ 1._2 12e_3", sm, alloc);

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

  ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());

  lex.setStrictMode(false);
  ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
  ASSERT_EQ(1, diag.getErrCountClear());
  ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
  ASSERT_EQ(2, diag.getErrCountClear());
  ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
  ASSERT_EQ(1, diag.getErrCountClear());
  ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
  ASSERT_EQ(1, diag.getErrCountClear());
  ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
  ASSERT_EQ(1, diag.getErrCountClear());
  ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
  ASSERT_EQ(1, diag.getErrCountClear());

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
    llvh::SmallString<32> actual;
    APFloat(lex.getCurToken()->getNumericLiteral()).toString(actual, 20);
    // We need dtoa.c for the correct string representation
    // EXPECT_STREQ("18446744073709552000", actual.c_str());
    EXPECT_STREQ("18446744073709551616", actual.c_str());
  }

  {
    ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
    llvh::SmallString<32> actual;
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

  {
    JSLexer lex("01 010 09 019 0o11 0O11", sm, alloc);

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

    tok = lex.advance();
    ASSERT_EQ(tok->getKind(), TokenKind::numeric_literal);
    ASSERT_EQ(tok->getNumericLiteral(), 9.0);

    tok = lex.advance();
    ASSERT_EQ(tok->getKind(), TokenKind::numeric_literal);
    ASSERT_EQ(tok->getNumericLiteral(), 9.0);

    ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
  }

  {
    JSLexer lex("08.1_1 07.11 07.9 08.9", sm, alloc);
    lex.setStrictMode(false);

    auto tok = lex.advance();
    ASSERT_EQ(tok->getKind(), TokenKind::numeric_literal);
    ASSERT_EQ(tok->getNumericLiteral(), 8.11);
    ASSERT_EQ(diag.getWarnCountClear(), 1);

    tok = lex.advance();
    ASSERT_EQ(tok->getKind(), TokenKind::numeric_literal);
    ASSERT_EQ(diag.getErrCountClear(), 1);

    tok = lex.advance();
    ASSERT_EQ(tok->getKind(), TokenKind::numeric_literal);
    ASSERT_EQ(diag.getErrCountClear(), 1);

    tok = lex.advance();
    ASSERT_EQ(tok->getKind(), TokenKind::numeric_literal);
    ASSERT_EQ(tok->getNumericLiteral(), 8.9);
    ASSERT_EQ(diag.getWarnCountClear(), 1);

    ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
  }

  {
    JSLexer lex("08.1_1 07.11 07.9 08.9", sm, alloc);
    lex.setStrictMode(true);

    auto tok = lex.advance();
    ASSERT_EQ(tok->getKind(), TokenKind::numeric_literal);
    ASSERT_EQ(diag.getErrCountClear(), 1);

    tok = lex.advance();
    ASSERT_EQ(tok->getKind(), TokenKind::numeric_literal);
    ASSERT_EQ(diag.getErrCountClear(), 1);

    tok = lex.advance();
    ASSERT_EQ(tok->getKind(), TokenKind::numeric_literal);
    ASSERT_EQ(diag.getErrCountClear(), 1);

    tok = lex.advance();
    ASSERT_EQ(tok->getKind(), TokenKind::numeric_literal);
    ASSERT_EQ(diag.getErrCountClear(), 1);

    ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
  }
}

#if HERMES_PARSE_FLOW
TEST(JSLexerTest, FlowOctalLiteralTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  JSLexer lex("01", sm, alloc);
  auto tok = lex.advance(JSLexer::GrammarContext::Type);
  ASSERT_EQ(tok->getKind(), TokenKind::numeric_literal);
  ASSERT_EQ(tok->getNumericLiteral(), 1.0);
  ASSERT_EQ(1, diag.getErrCountClear());
}
#endif

TEST(JSLexerTest, BinaryLiteralTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  JSLexer lex("0b1 0B1 0b101", sm, alloc);

  auto tok = lex.advance();
  ASSERT_EQ(tok->getKind(), TokenKind::numeric_literal);
  ASSERT_EQ(tok->getNumericLiteral(), 1.0);

  tok = lex.advance();
  ASSERT_EQ(tok->getKind(), TokenKind::numeric_literal);
  ASSERT_EQ(tok->getNumericLiteral(), 1.0);

  tok = lex.advance();
  ASSERT_EQ(tok->getKind(), TokenKind::numeric_literal);
  ASSERT_EQ(tok->getNumericLiteral(), 5.0);

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

TEST(JSLexerTest, PrivateIdentifierTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  JSLexer lex(
      " #foo"
      " # foo"
      " #64",
      sm,
      alloc);

  ASSERT_EQ(TokenKind::private_identifier, lex.advance()->getKind());
  EXPECT_EQ("foo", lex.getCurToken()->getPrivateIdentifier()->str());

  ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
  EXPECT_EQ(1, diag.getErrCountClear());
  EXPECT_EQ("foo", lex.getCurToken()->getIdentifier()->str());

  ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
  EXPECT_EQ(1, diag.getErrCountClear());
  EXPECT_EQ(64, lex.getCurToken()->getNumericLiteral());
}

TEST(JSLexerTest, StringTest1) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  JSLexer lex(
      "'aa' \"bb\" 'open1\n"
      "\"open2",
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

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
  ASSERT_FALSE(lex.isNewLineBeforeCurrentToken());
}

TEST(JSLexerTest, StringLineParaSepTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  // Test that Unicode line and paragraph separatot are valid in a string
  // (since ES10).
  JSLexer lex(
      "'\xe2\x80\xa8' "
      "'\xe2\x80\xa9' "
      "'\\\xe2\x80\xa8' "
      "'\\\xe2\x80\xa9' ",
      sm,
      alloc);

  ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
  ASSERT_EQ(0, diag.getErrCountClear());
  EXPECT_STREQ("\xe2\x80\xa8", lex.getCurToken()->getStringLiteral()->c_str());

  ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
  ASSERT_EQ(0, diag.getErrCountClear());
  EXPECT_STREQ("\xe2\x80\xa9", lex.getCurToken()->getStringLiteral()->c_str());

  ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
  ASSERT_EQ(0, diag.getErrCountClear());
  EXPECT_STREQ("", lex.getCurToken()->getStringLiteral()->c_str());

  ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
  ASSERT_EQ(0, diag.getErrCountClear());
  EXPECT_STREQ("", lex.getCurToken()->getStringLiteral()->c_str());

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
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

TEST(JSLexerTest, templateLiterals) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext ctx(sm);

  {
    JSLexer lex("`abc` `\\x41` `\\u0041` `\\\xe2\x80\xa8`", sm, alloc);

    ASSERT_EQ(TokenKind::no_substitution_template, lex.advance()->getKind());
    EXPECT_STREQ("abc", lex.getCurToken()->getTemplateValue()->c_str());
    EXPECT_STREQ("abc", lex.getCurToken()->getTemplateRawValue()->c_str());

    ASSERT_EQ(TokenKind::no_substitution_template, lex.advance()->getKind());
    EXPECT_STREQ("\x41", lex.getCurToken()->getTemplateValue()->c_str());
    EXPECT_STREQ("\\x41", lex.getCurToken()->getTemplateRawValue()->c_str());

    ASSERT_EQ(TokenKind::no_substitution_template, lex.advance()->getKind());
    EXPECT_STREQ("\u0041", lex.getCurToken()->getTemplateValue()->c_str());
    EXPECT_STREQ("\\u0041", lex.getCurToken()->getTemplateRawValue()->c_str());

    ASSERT_EQ(TokenKind::no_substitution_template, lex.advance()->getKind());
    EXPECT_STREQ("", lex.getCurToken()->getTemplateValue()->c_str());
    EXPECT_STREQ(
        "\\\xe2\x80\xa8", lex.getCurToken()->getTemplateRawValue()->c_str());
  }

  {
    JSLexer lex("`abc${x}def${y}ghi`", sm, alloc);

    ASSERT_EQ(TokenKind::template_head, lex.advance()->getKind());
    EXPECT_STREQ("abc", lex.getCurToken()->getTemplateValue()->c_str());
    EXPECT_STREQ("abc", lex.getCurToken()->getTemplateRawValue()->c_str());

    ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
    EXPECT_STREQ("x", lex.getCurToken()->getIdentifier()->c_str());

    ASSERT_EQ(TokenKind::r_brace, lex.advance()->getKind());
    lex.rescanRBraceInTemplateLiteral();
    ASSERT_EQ(TokenKind::template_middle, lex.getCurToken()->getKind());
    EXPECT_STREQ("def", lex.getCurToken()->getTemplateValue()->c_str());
    EXPECT_STREQ("def", lex.getCurToken()->getTemplateRawValue()->c_str());

    ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
    EXPECT_STREQ("y", lex.getCurToken()->getIdentifier()->c_str());

    ASSERT_EQ(TokenKind::r_brace, lex.advance()->getKind());
    lex.rescanRBraceInTemplateLiteral();
    ASSERT_EQ(TokenKind::template_tail, lex.getCurToken()->getKind());
    EXPECT_STREQ("ghi", lex.getCurToken()->getTemplateValue()->c_str());
    EXPECT_STREQ("ghi", lex.getCurToken()->getTemplateRawValue()->c_str());
  }

  {
    JSLexer lex("`\\0`", sm, alloc);
    ASSERT_EQ(TokenKind::no_substitution_template, lex.advance()->getKind());
    EXPECT_STREQ("\0", lex.getCurToken()->getTemplateValue()->c_str());
    EXPECT_STREQ("\\0", lex.getCurToken()->getTemplateRawValue()->c_str());
  }

  {
    JSLexer lex("`\r\n \n \r`", sm, alloc);
    ASSERT_EQ(TokenKind::no_substitution_template, lex.advance()->getKind());
    EXPECT_STREQ("\n \n \n", lex.getCurToken()->getTemplateValue()->c_str());
    EXPECT_STREQ("\n \n \n", lex.getCurToken()->getTemplateRawValue()->c_str());
  }
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

TEST(JSLexerTest, LookaheadNewlineTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  // Test the lookahead function which will not revert to the current
  // token after lookahead if an optional expected token is provided.
  JSLexer lex("function\n(", sm, alloc);

  ASSERT_EQ(TokenKind::rw_function, lex.advance()->getKind());

  {
    // Revert since there is no expected token
    // Expect None returned since there is a newline before the next token
    auto optNext = lex.lookahead1(llvh::None);
    ASSERT_FALSE(optNext.hasValue());
  }

  ASSERT_EQ(TokenKind::rw_function, lex.getCurToken()->getKind());
  ASSERT_EQ(TokenKind::l_paren, lex.advance()->getKind());

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
}

TEST(JSLexerTest, LookaheadTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  // Test the lookahead function which will not revert to the current
  // token after lookahead if an optional expected token is provided.
  JSLexer lex("function( foo,", sm, alloc);

  ASSERT_EQ(TokenKind::rw_function, lex.advance()->getKind());

  {
    // Without the expected token, always revert.
    auto optNext = lex.lookahead1(llvh::None);
    ASSERT_TRUE(optNext.hasValue());
    EXPECT_EQ(TokenKind::l_paren, optNext.getValue());
  }

  ASSERT_EQ(TokenKind::rw_function, lex.getCurToken()->getKind());
  ASSERT_EQ(TokenKind::l_paren, lex.advance()->getKind());

  // With the expected token, revert iff it doesn't match.
  ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());

  {
    auto optNext = lex.lookahead1(TokenKind::plus);
    ASSERT_TRUE(optNext.hasValue());
    EXPECT_EQ(TokenKind::comma, optNext.getValue());
    // Revert to original token.
    ASSERT_EQ(TokenKind::identifier, lex.getCurToken()->getKind());
  }

  {
    auto optNext = lex.lookahead1(TokenKind::comma);
    ASSERT_TRUE(optNext.hasValue());
    EXPECT_EQ(TokenKind::comma, optNext.getValue());
    // Match with expected, keep the lookahead token.
    ASSERT_EQ(TokenKind::comma, lex.getCurToken()->getKind());
  }

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
}

TEST(JSLexerTest, RegressConsumeBadHexTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  // Test a hex escape where the two following characters are ('5' & ~32).
  // This catches a bug where we were or-ing 32 before checking whether the
  // character is a digit.
  JSLexer lex("'\\x\x15\x15'", sm, alloc);

  ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
  ASSERT_EQ(1, diag.getErrCountClear());

  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
  ASSERT_FALSE(lex.isNewLineBeforeCurrentToken());

  ASSERT_EQ(0, diag.getErrCountClear());
}

TEST(JSLexerTest, ConsumeBadBracedCodePoint) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  // Test an invalid curly brace escape without a terminating curly brace
  // This catches an out of bounds read where we hit the error limit and
  // curCharPtr_ was set to eof, but JSLexer::consumeBracedCodePoint continued
  // to operate on curCharPtr_
  // We configure a low error limit to reach the interesting code path faster
  // then we use invalid characters in a non terminated curly brace code point
  // to trigger JSLexer:error
  sm.setErrorLimit(1);
  JSLexer lex("'\\u{12XXXXXXXXXXX'", sm, alloc);

  ASSERT_EQ(TokenKind::string_literal, lex.advance()->getKind());
  ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
}

TEST(JSLexerTest, AtSignTest) {
  {
    JSLexer::Allocator alloc;
    SourceErrorManager sm;
    DiagContext diag(sm);
    sm.setErrorLimit(10);
    JSLexer lex("`${{}@", sm, alloc);

    ASSERT_EQ(TokenKind::template_head, lex.advance()->getKind());

    ASSERT_EQ(TokenKind::l_brace, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::r_brace, lex.advance()->getKind());

    ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
    EXPECT_EQ(1, sm.getErrorCount());
  }
  {
    JSLexer::Allocator alloc;
    SourceErrorManager sm;
    DiagContext diag(sm);
    sm.setErrorLimit(1);
    JSLexer lex("`${{}@", sm, alloc);

    ASSERT_EQ(TokenKind::template_head, lex.advance()->getKind());

    ASSERT_EQ(TokenKind::l_brace, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::r_brace, lex.advance()->getKind());

    ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());
    EXPECT_EQ(1, sm.getErrorCount());
  }
}

TEST(JSLexerTest, JSXTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  JSLexer lex("abc def{xyz<qwerty", sm, alloc);

  ASSERT_EQ(TokenKind::jsx_text, lex.advanceInJSXChild()->getKind());
  EXPECT_STREQ("abc def", lex.getCurToken()->getJSXTextRaw()->c_str());

  ASSERT_EQ(TokenKind::l_brace, lex.advanceInJSXChild()->getKind());
  ASSERT_EQ(TokenKind::jsx_text, lex.advanceInJSXChild()->getKind());
  EXPECT_STREQ("xyz", lex.getCurToken()->getJSXTextRaw()->c_str());

  ASSERT_EQ(TokenKind::less, lex.advanceInJSXChild()->getKind());
  ASSERT_EQ(TokenKind::jsx_text, lex.advanceInJSXChild()->getKind());
  EXPECT_STREQ("qwerty", lex.getCurToken()->getJSXTextRaw()->c_str());
}

TEST(JSLexerTest, StoreCommentsTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  {
    JSLexer lex("// hello\n;\n// world", sm, alloc, nullptr, true, false);
    lex.setStoreComments(true);

    ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());

    ASSERT_EQ(2, lex.getStoredComments().size());
    EXPECT_EQ(StoredComment::Kind::Line, lex.getStoredComments()[0].getKind());
    EXPECT_EQ(" hello", lex.getStoredComments()[0].getString());
    EXPECT_EQ(StoredComment::Kind::Line, lex.getStoredComments()[1].getKind());
    EXPECT_EQ(" world", lex.getStoredComments()[1].getString());
  }

  {
    JSLexer lex("/* hello */;/*world*/", sm, alloc, nullptr, true, false);
    lex.setStoreComments(true);

    ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());

    ASSERT_EQ(2, lex.getStoredComments().size());
    EXPECT_EQ(StoredComment::Kind::Block, lex.getStoredComments()[0].getKind());
    EXPECT_EQ(" hello ", lex.getStoredComments()[0].getString());
    EXPECT_EQ(StoredComment::Kind::Block, lex.getStoredComments()[1].getKind());
    EXPECT_EQ("world", lex.getStoredComments()[1].getString());
  }

  {
    JSLexer lex("#! hello world\n;", sm, alloc, nullptr, true, false);
    lex.setStoreComments(true);

    ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());

    ASSERT_EQ(1, lex.getStoredComments().size());
    EXPECT_EQ(
        StoredComment::Kind::Hashbang, lex.getStoredComments()[0].getKind());
    EXPECT_EQ(" hello world", lex.getStoredComments()[0].getString());
  }

  {
    JSLexer lex("/**/;//\n", sm, alloc, nullptr, true, false);
    lex.setStoreComments(true);

    ASSERT_EQ(TokenKind::semi, lex.advance()->getKind());
    ASSERT_EQ(TokenKind::eof, lex.advance()->getKind());

    ASSERT_EQ(2, lex.getStoredComments().size());
    EXPECT_EQ(StoredComment::Kind::Block, lex.getStoredComments()[0].getKind());
    EXPECT_EQ("", lex.getStoredComments()[0].getString());
    EXPECT_EQ(StoredComment::Kind::Line, lex.getStoredComments()[1].getKind());
    EXPECT_EQ("", lex.getStoredComments()[1].getString());
  }

  {
    JSLexer lex("/*one*/ < /*two*/ >", sm, alloc, nullptr, true, false);
    lex.setStoreComments(true);

    // Create save point between two comments
    ASSERT_EQ(TokenKind::less, lex.advance()->getKind());
    JSLexer::SavePoint savePoint{&lex};
    ASSERT_EQ(TokenKind::greater, lex.advance()->getKind());

    ASSERT_EQ(2, lex.getStoredComments().size());
    EXPECT_EQ(StoredComment::Kind::Block, lex.getStoredComments()[0].getKind());
    EXPECT_EQ("one", lex.getStoredComments()[0].getString());
    EXPECT_EQ(StoredComment::Kind::Block, lex.getStoredComments()[1].getKind());
    EXPECT_EQ("two", lex.getStoredComments()[1].getString());

    // After restoring, comment after save point is removed from comment storage
    savePoint.restore();

    ASSERT_EQ(1, lex.getStoredComments().size());
    EXPECT_EQ(StoredComment::Kind::Block, lex.getStoredComments()[0].getKind());
    EXPECT_EQ("one", lex.getStoredComments()[0].getString());
  }

  {
    JSLexer lex("/*one*/ A /*two*/ >", sm, alloc, nullptr, true, false);
    lex.setStoreComments(true);

    ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
    ASSERT_EQ(1, lex.getStoredComments().size());
    EXPECT_EQ("one", lex.getStoredComments()[0].getString());

    // Lookahead does not store comments
    lex.lookahead1(TokenKind::semi);
    ASSERT_EQ(1, lex.getStoredComments().size());
    EXPECT_EQ("one", lex.getStoredComments()[0].getString());

    ASSERT_EQ(TokenKind::greater, lex.advance()->getKind());
    ASSERT_EQ(2, lex.getStoredComments().size());
  }
}

TEST(JSLexerTest, PrevTokenEndLocTest) {
  JSLexer::Allocator alloc;
  SourceErrorManager sm;
  DiagContext diag(sm);

  {
    JSLexer lex("var x = 1", sm, alloc);

    ASSERT_EQ(TokenKind::rw_var, lex.advance()->getKind());
    SMLoc varEndLoc = lex.getCurToken()->getEndLoc();

    ASSERT_EQ(TokenKind::identifier, lex.advance()->getKind());
    ASSERT_EQ(varEndLoc, lex.getPrevTokenEndLoc());
    SMLoc idEndLoc = lex.getCurToken()->getEndLoc();

    // Create save point at identifier
    JSLexer::SavePoint savePoint{&lex};

    ASSERT_EQ(TokenKind::equal, lex.advance()->getKind());
    ASSERT_EQ(idEndLoc, lex.getPrevTokenEndLoc());
    SMLoc equalEndLoc = lex.getCurToken()->getEndLoc();

    ASSERT_EQ(TokenKind::numeric_literal, lex.advance()->getKind());
    ASSERT_EQ(equalEndLoc, lex.getPrevTokenEndLoc());

    // Restoring to identifier. Last token is now the var keyword.
    savePoint.restore();
    ASSERT_EQ(varEndLoc, lex.getPrevTokenEndLoc());
  }
}

} // namespace
