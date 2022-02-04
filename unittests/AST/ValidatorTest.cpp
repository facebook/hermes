/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "../Parser/DiagContext.h"
#include "hermes/AST/SemValidate.h"
#include "hermes/Parser/JSParser.h"

#include "gtest/gtest.h"

using namespace hermes;
using namespace hermes::parser;

namespace {

/// Left side of assignment must be an LValue.
TEST(ValidatorTest, TestBadAssignmentLValue) {
  Context ctx;
  sem::SemContext semCtx{};
  DiagContext diag(ctx);
  JSParser parser(ctx, "a + 1 = 10;");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_FALSE(validateAST(ctx, semCtx, *parsed));
  EXPECT_EQ(1, diag.getErrCount());
}

/// For-in control expression must be an LValue.
TEST(ValidatorTest, TestBadForLValue) {
  Context ctx;
  sem::SemContext semCtx{};
  DiagContext diag(ctx);
  JSParser parser(ctx, "for(a + 1 in x);");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_FALSE(validateAST(ctx, semCtx, *parsed));
  EXPECT_EQ(1, diag.getErrCount());
}

/// Test an anonymous break outside of a loop.
TEST(ValidatorTest, UnnamedBreakLabelTest) {
  Context ctx;
  sem::SemContext semCtx{};
  DiagContext diag(ctx);
  JSParser parser(ctx, "break; for(;;) break; break;");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_FALSE(validateAST(ctx, semCtx, *parsed));
  ASSERT_EQ(2, diag.getErrCountClear());
}

/// Test an anonymous continue outside of a loop.
TEST(ValidatorTest, UnnamedContinueLabelTest) {
  Context ctx;
  sem::SemContext semCtx{};
  DiagContext diag(ctx);
  JSParser parser(ctx, "continue;");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_FALSE(validateAST(ctx, semCtx, *parsed));
  ASSERT_EQ(1, diag.getErrCountClear());
}

/// Test a continue with a block label.
TEST(ValidatorTest, ContinueWithBlockLabelTest) {
  Context ctx;
  sem::SemContext semCtx{};
  DiagContext diag(ctx);
  JSParser parser(ctx, "label1: { continue label1; }");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_FALSE(validateAST(ctx, semCtx, *parsed));
  ASSERT_EQ(1, diag.getErrCountClear());
}

/// Test that multiple labels are correctly attached to the same statement.
TEST(ValidatorTest, ChainedNamedLabelsTest) {
  Context ctx;
  sem::SemContext semCtx{};
  DiagContext diag(ctx);
  JSParser parser(
      ctx,
      "label1: label2: label3: for(;;) { continue label1; continue "
      "label2; continue label3; }");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_TRUE(validateAST(ctx, semCtx, *parsed));
}

/// Duplicated label in the scope of the previous one.
TEST(ValidatorTest, DuplicateNamedLabelTest) {
  Context ctx;
  sem::SemContext semCtx{};
  DiagContext diag(ctx);
  JSParser parser(
      ctx,
      "label1: { label1: ; }\n"
      "label2: label2: ;");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_FALSE(validateAST(ctx, semCtx, *parsed));
  ASSERT_EQ(2, diag.getErrCountClear());
}

TEST(ValidatorTest, CorrectDuplicateNamedLabelTest) {
  Context ctx;
  sem::SemContext semCtx{};
  DiagContext diag(ctx);
  JSParser parser(
      ctx, "label1: { break label1; } label1: for(;;) break label1;");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());
  ASSERT_TRUE(validateAST(ctx, semCtx, *parsed));
}

TEST(ValidatorTest, ScopeNamedLabelTest) {
  Context ctx;
  sem::SemContext semCtx{};
  DiagContext diag(ctx);
  JSParser parser(ctx, "label1: ; for(;;) break label1;");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_FALSE(validateAST(ctx, semCtx, *parsed));
  ASSERT_EQ(1, diag.getErrCountClear());
}

TEST(ValidatorTest, NamedBreakLabelTest) {
  Context ctx;
  sem::SemContext semCtx{};
  DiagContext diag(ctx);
  JSParser parser(
      ctx, "break exitLoop; exitLoop: for(;;) break exitLoop; break exitLoop;");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_FALSE(validateAST(ctx, semCtx, *parsed));
  ASSERT_EQ(2, diag.getErrCountClear());
}

void assertFunctionLikeSourceVisibility(
    llvh::Optional<ESTree::FunctionLikeNode *> funcLikeNode,
    SourceVisibility sourceVisibility) {
  ASSERT_TRUE(funcLikeNode.hasValue());
  ASSERT_EQ((*funcLikeNode)->sourceVisibility, sourceVisibility);
}

void assertFirstNodeAsFunctionLikeWithSourceVisibility(
    llvh::Optional<ESTree::ProgramNode *> parsed,
    SourceVisibility sourceVisibility) {
  ASSERT_TRUE(parsed.hasValue());
  auto *programNode = llvh::cast<ESTree::ProgramNode>(parsed.getValue());
  ASSERT_TRUE(llvh::isa<ESTree::FunctionLikeNode>(programNode->_body.front()));
  auto *funcLikeNode =
      llvh::cast<ESTree::FunctionLikeNode>(&programNode->_body.front());

  ASSERT_EQ(funcLikeNode->sourceVisibility, sourceVisibility);
}

void assertSecondNodeAsFunctionLikeWithSourceVisibility(
    llvh::Optional<ESTree::ProgramNode *> parsed,
    SourceVisibility sourceVisibility) {
  ASSERT_TRUE(parsed.hasValue());
  auto *programNode = llvh::cast<ESTree::ProgramNode>(parsed.getValue());
  auto it = programNode->_body.begin();
  // Step to the 2nd node.
  it++;
  ASSERT_TRUE(llvh::isa<ESTree::FunctionLikeNode>(*it));
  auto *funcLikeNode = llvh::cast<ESTree::FunctionLikeNode>(it);

  ASSERT_EQ(funcLikeNode->sourceVisibility, sourceVisibility);
}

TEST(ValidatorTest, SourceVisibilityTest) {
  Context context;
  sem::SemContext semCtx{};
  // Top-level program node.
  {
    JSParser parser(context, "");
    auto parsed = parser.parse();
    validateAST(context, semCtx, *parsed);
    assertFunctionLikeSourceVisibility(*parsed, SourceVisibility::Default);
  }
  {
    JSParser parser(context, "'show source'");
    auto parsed = parser.parse();
    // source visibility is set to default before semantic validation.
    assertFunctionLikeSourceVisibility(*parsed, SourceVisibility::Default);
    validateAST(context, semCtx, *parsed);
    // source visibility is correctly updated after semantic validation.
    assertFunctionLikeSourceVisibility(*parsed, SourceVisibility::ShowSource);
  }
  // Singleton function node.
  {
    JSParser parser(context, "function func (a, b) { return 10 }");
    auto parsed = parser.parse();
    validateAST(context, semCtx, *parsed);
    assertFirstNodeAsFunctionLikeWithSourceVisibility(
        parsed, SourceVisibility::Default);
  }
  {
    JSParser parser(context, "function func (a, b) { 'sensitive'; return 10 }");
    auto parsed = parser.parse();
    validateAST(context, semCtx, *parsed);
    assertFirstNodeAsFunctionLikeWithSourceVisibility(
        parsed, SourceVisibility::Sensitive);
  }
  {
    JSParser parser(
        context, "function func (a, b) { 'hide source'; return 10 }");
    auto parsed = parser.parse();
    validateAST(context, semCtx, *parsed);
    assertFirstNodeAsFunctionLikeWithSourceVisibility(
        parsed, SourceVisibility::HideSource);
  }
  {
    JSParser parser(
        context, "function func (a, b) { 'show source'; return 10 }");
    auto parsed = parser.parse();
    validateAST(context, semCtx, *parsed);
    assertFirstNodeAsFunctionLikeWithSourceVisibility(
        parsed, SourceVisibility::ShowSource);
  }
  // Visibility is correctly restored.
  {
    JSParser parser(context, "function foo(x) { 'sensitive' }function bar(){}");
    auto parsed = parser.parse();
    validateAST(context, semCtx, *parsed);
    assertFirstNodeAsFunctionLikeWithSourceVisibility(
        parsed, SourceVisibility::Sensitive);
    assertSecondNodeAsFunctionLikeWithSourceVisibility(
        parsed, SourceVisibility::Default);
  }
  // Overriding.
  {
    // ShowSource > Default
    JSParser parser(
        context, "'show source'; function func (a, b) { return 10 }");
    auto parsed = parser.parse();
    validateAST(context, semCtx, *parsed);
    assertSecondNodeAsFunctionLikeWithSourceVisibility(
        parsed, SourceVisibility::ShowSource);
  }
  {
    // HideSource > ShowSource
    JSParser parser(
        context,
        "'show source'; function func (a, b) { 'hide source'; return 10 }");
    auto parsed = parser.parse();
    validateAST(context, semCtx, *parsed);
    assertSecondNodeAsFunctionLikeWithSourceVisibility(
        parsed, SourceVisibility::HideSource);
  }
  {
    // ShowSource < HideSource
    JSParser parser(
        context,
        "'hide source'; function func (a, b) { 'show source'; return 10 }");
    auto parsed = parser.parse();
    validateAST(context, semCtx, *parsed);
    assertSecondNodeAsFunctionLikeWithSourceVisibility(
        parsed, SourceVisibility::HideSource);
  }
  {
    // Sensitive > HideSource
    JSParser parser(
        context,
        "'hide source'; function func (a, b) { 'sensitive'; return 10 }");
    auto parsed = parser.parse();
    validateAST(context, semCtx, *parsed);
    assertSecondNodeAsFunctionLikeWithSourceVisibility(
        parsed, SourceVisibility::Sensitive);
  }
}

} // anonymous namespace
