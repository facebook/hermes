/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Parser/JSParser.h"
#include "DiagContext.h"

#include "gtest/gtest.h"

using llvh::cast;
using llvh::dyn_cast;
using llvh::isa;

using namespace hermes::parser;
using namespace hermes;

namespace {

/// Utility for asserting the type of the first expression in a program.
void assertFirstExpressionType(
    llvh::Optional<ESTree::ProgramNode *> parsed,
    std::function<bool(const hermes::ESTree::NodePtr &)> isType) {
  ASSERT_TRUE(parsed.hasValue());
  auto *programNode = llvh::dyn_cast<ESTree::ProgramNode>(parsed.getValue());
  ASSERT_NE(nullptr, programNode);

  ASSERT_TRUE(
      llvh::isa<ESTree::ExpressionStatementNode>(programNode->_body.front()));
  auto *exprStmtNode = llvh::dyn_cast<ESTree::ExpressionStatementNode>(
      &programNode->_body.front());

  ASSERT_TRUE(isType(exprStmtNode->_expression));
}

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

  auto *programNode = llvh::dyn_cast<ESTree::ProgramNode>(parsed.getValue());
  ASSERT_NE(nullptr, programNode);

  ASSERT_TRUE(llvh::isa<ESTree::WithStatementNode>(programNode->_body.front()));
}

TEST(JSParserTest, TestSwitch) {
  Context context;
  JSParser parser(context, "switch (0) {case 1: break; case 2: break}");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  auto *programNode = llvh::dyn_cast<ESTree::ProgramNode>(parsed.getValue());
  ASSERT_NE(nullptr, programNode);

  auto *switchNode =
      llvh::dyn_cast<ESTree::SwitchStatementNode>(&programNode->_body.front());
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

  auto *programNode = llvh::dyn_cast<ESTree::ProgramNode>(parsed.getValue());
  ASSERT_NE(nullptr, programNode);

  ASSERT_TRUE(
      llvh::isa<ESTree::ThrowStatementNode>(programNode->_body.front()));
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

  auto *programNode = llvh::dyn_cast<ESTree::ProgramNode>(parsed.getValue());
  ASSERT_NE(nullptr, programNode);

  ASSERT_TRUE(llvh::isa<ESTree::TryStatementNode>(programNode->_body.front()));
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

  auto *programNode = llvh::dyn_cast<ESTree::ProgramNode>(parsed.getValue());
  ASSERT_NE(nullptr, programNode);

  ASSERT_TRUE(
      llvh::isa<ESTree::DebuggerStatementNode>(programNode->_body.front()));
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

TEST(JSParserTest, TestUnterminatedBOMInDirective) {
  Context context;
  // Begin an unterminated Byte Order Mark (BOM) after the first one.
  JSParser parser(context, "\"\"\xef\xbb\xbf\xef");
  auto parsed = parser.parse();
  ASSERT_FALSE(parsed.hasValue());

  SourceErrorManager &sm = context.getSourceErrorManager();
  EXPECT_EQ(sm.getErrorCount(), 2);
}

TEST(JSParserTest, TestAmbiguousCallFlowSyntax) {
  // In regular Flow mode, call expressions with type arguments are parsed
  Context context1;
  context1.setParseFlow(ParseFlowSetting::ALL);

  JSParser parser1(context1, "foo<T>(x)");
  auto parsed1 = parser1.parse();

  assertFirstExpressionType(
      parsed1, llvh::isa<ESTree::CallExpressionNode, hermes::ESTree::NodePtr>);

  // In unambiguous Flow mode, call expressions with type arguments are instead
  // parsed as BinaryExpressions.
  Context context2;
  context2.setParseFlow(ParseFlowSetting::UNAMBIGUOUS);

  JSParser parser2(context2, "foo<T>(x)");
  auto parsed2 = parser2.parse();

  assertFirstExpressionType(
      parsed2,
      llvh::isa<ESTree::BinaryExpressionNode, hermes::ESTree::NodePtr>);
}

TEST(JSParserTest, TestAmbiguousNewFlowSyntax) {
  // In regular Flow mode, new expressions with type arguments are parsed
  Context context1;
  context1.setParseFlow(ParseFlowSetting::ALL);

  JSParser parser1(context1, "new foo<T>(x)");
  auto parsed1 = parser1.parse();

  assertFirstExpressionType(
      parsed1, llvh::isa<ESTree::NewExpressionNode, hermes::ESTree::NodePtr>);

  // In unambiguous Flow mode, new expressions with type arguments are instead
  // parsed as BinaryExpressions.
  Context context2;
  context2.setParseFlow(ParseFlowSetting::UNAMBIGUOUS);

  JSParser parser2(context2, "new foo<T>(x)");
  auto parsed2 = parser2.parse();

  assertFirstExpressionType(
      parsed2,
      llvh::isa<ESTree::BinaryExpressionNode, hermes::ESTree::NodePtr>);
}

}; // anonymous namespace
