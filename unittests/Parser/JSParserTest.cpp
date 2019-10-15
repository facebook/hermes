/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Parser/JSParser.h"
#include "DiagContext.h"

#include "gtest/gtest.h"

using llvm::cast;
using llvm::dyn_cast;
using llvm::isa;

using namespace hermes::parser;
using namespace hermes;

namespace {

TEST(JSParserTest, Function1) {
  Context context;
  JSParser parser(context, "function func (a, b) { return 10 }");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());
}

TEST(JSParserTest, Function1Err) {
  Context context;
  DiagContext diag(context);
  JSParser parser(context, "function func (a+) {}");
  auto parsed = parser.parse();
  ASSERT_FALSE(parsed.hasValue());
  ASSERT_EQ(1, diag.getErrCountClear());
}

TEST(JSParserTest, TestIf) {
  Context context;

  JSParser parser(context, "var a; if (a) ++a; else --a;");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());
}

TEST(JSParserTest, TestWith) {
  Context context;
  JSParser parser(context, "with (x) { prop = 0; }");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  auto *programNode = dyn_cast<ESTree::ProgramNode>(parsed.getValue());
  ASSERT_NE(nullptr, programNode);

  ASSERT_TRUE(isa<ESTree::WithStatementNode>(programNode->_body.front()));
}

TEST(JSParserTest, TestSwitch) {
  Context context;
  JSParser parser(context, "switch (0) {case 1: break; case 2: break}");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  auto *programNode = dyn_cast<ESTree::ProgramNode>(parsed.getValue());
  ASSERT_NE(nullptr, programNode);

  auto *switchNode =
      dyn_cast<ESTree::SwitchStatementNode>(&programNode->_body.front());
  ASSERT_NE(nullptr, switchNode);
  ASSERT_EQ(2u, switchNode->_cases.size());
}

TEST(JSParserTest, TestSwitchTwoDefaults) {
  Context context;
  DiagContext diag(context);
  JSParser parser(
      context, "switch (0) {default: break; case 2: break; default: }");
  auto parsed = parser.parse();
  ASSERT_FALSE(parsed.hasValue());
}

TEST(JSParserTest, TestThrow) {
  Context context;
  JSParser parser(context, "throw 1");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  auto *programNode = dyn_cast<ESTree::ProgramNode>(parsed.getValue());
  ASSERT_NE(nullptr, programNode);

  ASSERT_TRUE(isa<ESTree::ThrowStatementNode>(programNode->_body.front()));
}

TEST(JSParserTest, TestBadThrowNewLine) {
  Context context;
  DiagContext diag(context);
  JSParser parser(context, "throw\n1;");
  auto parsed = parser.parse();
  ASSERT_FALSE(parsed.hasValue());
}

TEST(JSParserTest, TestTry) {
  Context context;
  JSParser parser(context, "try {1;} catch (e) {2} finally {3}");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  auto *programNode = dyn_cast<ESTree::ProgramNode>(parsed.getValue());
  ASSERT_NE(nullptr, programNode);

  ASSERT_TRUE(isa<ESTree::TryStatementNode>(programNode->_body.front()));
}

TEST(JSParserTest, TestBadTry) {
  Context context;
  DiagContext diag(context);
  JSParser parser(context, "try {1;} cetch {2;}");
  auto parsed = parser.parse();
  ASSERT_FALSE(parsed.hasValue());
}

TEST(JSParserTest, TestDebugger) {
  Context context;
  JSParser parser(context, "debugger;");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  auto *programNode = dyn_cast<ESTree::ProgramNode>(parsed.getValue());
  ASSERT_NE(nullptr, programNode);

  ASSERT_TRUE(isa<ESTree::DebuggerStatementNode>(programNode->_body.front()));
}

TEST(JSParserTest, TestDiv) {
  Context context;
  JSParser parser(
      context,
      "a / b; "
      "a + b / c; "
      "(a) / b; "
      "a /= b; "
      "a[1] / b; "
      "func().prop / b;");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());
}

TEST(JSParserTest, TestRegExp) {
  Context context;
  JSParser parser(
      context,
      "/aaa/; "
      "var a = /aaa/;"
      "/aaa/ / /aaa/;");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());
}

}; // anonymous namespace
