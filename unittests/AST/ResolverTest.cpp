/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "../Parser/DiagContext.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Sema/SemContext.h"
#include "hermes/Sema/SemResolve.h"

#include "gtest/gtest.h"

using namespace hermes;
using namespace hermes::parser;

namespace {

/// Left side of assignment must be an LValue.
TEST(ResolverTest, TestBadAssignmentLValue) {
  Context ctx;
  sema::SemContext semCtx(ctx);
  DiagContext diag(ctx);
  JSParser parser(ctx, "a + 1 = 10;");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_FALSE(sema::resolveAST(ctx, semCtx, *parsed));
  EXPECT_EQ(1, diag.getErrCount());
}

/// For-in control expression must be an LValue.
TEST(ResolverTest, TestBadForLValue) {
  Context ctx;
  sema::SemContext semCtx(ctx);
  DiagContext diag(ctx);
  JSParser parser(ctx, "for(a + 1 in x);");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_FALSE(resolveAST(ctx, semCtx, *parsed));
  EXPECT_EQ(1, diag.getErrCount());
}

/// Test an anonymous break outside of a loop.
TEST(ResolverTest, UnnamedBreakLabelTest) {
  Context ctx;
  sema::SemContext semCtx(ctx);
  DiagContext diag(ctx);
  JSParser parser(ctx, "break; for(;;) break; break;");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_FALSE(resolveAST(ctx, semCtx, *parsed));
  ASSERT_EQ(2, diag.getErrCountClear());
}

/// Test an anonymous continue outside of a loop.
TEST(ResolverTest, UnnamedContinueLabelTest) {
  Context ctx;
  sema::SemContext semCtx(ctx);
  DiagContext diag(ctx);
  JSParser parser(ctx, "continue;");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_FALSE(resolveAST(ctx, semCtx, *parsed));
  ASSERT_EQ(1, diag.getErrCountClear());
}

/// Test an anonymous continue outside of a loop.
TEST(ResolverTest, ContinueInASwitchTest) {
  Context ctx;
  sema::SemContext semCtx(ctx);
  DiagContext diag(ctx);
  JSParser parser(ctx, "switch(1) { case 1: continue; }");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_FALSE(resolveAST(ctx, semCtx, *parsed));
  ASSERT_EQ(1, diag.getErrCountClear());
}

/// Test a continue with a block label.
TEST(ResolverTest, ContinueWithBlockLabelTest) {
  Context ctx;
  sema::SemContext semCtx(ctx);
  DiagContext diag(ctx);
  JSParser parser(ctx, "label1: { continue label1; }");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_FALSE(resolveAST(ctx, semCtx, *parsed));
  ASSERT_EQ(1, diag.getErrCountClear());
}

/// Test that multiple labels are correctly attached to the same statement.
TEST(ResolverTest, ChainedNamedLabelsTest) {
  Context ctx;
  sema::SemContext semCtx(ctx);
  DiagContext diag(ctx);
  JSParser parser(
      ctx,
      "label1: label2: label3: for(;;) { continue label1; continue "
      "label2; continue label3; }");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_TRUE(resolveAST(ctx, semCtx, *parsed));
}

/// Duplicated label in the scope of the previous one.
TEST(ResolverTest, DuplicateNamedLabelTest) {
  Context ctx;
  sema::SemContext semCtx(ctx);
  DiagContext diag(ctx);
  JSParser parser(
      ctx,
      "label1: { label1: ; }\n"
      "label2: label2: ;");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_FALSE(resolveAST(ctx, semCtx, *parsed));
  ASSERT_EQ(2, diag.getErrCountClear());
}

TEST(ResolverTest, CorrectDuplicateNamedLabelTest) {
  Context ctx;
  sema::SemContext semCtx(ctx);
  DiagContext diag(ctx);
  JSParser parser(
      ctx, "label1: { break label1; } label1: for(;;) break label1;");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());
  ASSERT_TRUE(resolveAST(ctx, semCtx, *parsed));
}

TEST(ResolverTest, ScopeNamedLabelTest) {
  Context ctx;
  sema::SemContext semCtx(ctx);
  DiagContext diag(ctx);
  JSParser parser(ctx, "label1: ; for(;;) break label1;");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_FALSE(resolveAST(ctx, semCtx, *parsed));
  ASSERT_EQ(1, diag.getErrCountClear());
}

TEST(ResolverTest, NamedBreakLabelTest) {
  Context ctx;
  sema::SemContext semCtx(ctx);
  DiagContext diag(ctx);
  JSParser parser(
      ctx, "break exitLoop; exitLoop: for(;;) break exitLoop; break exitLoop;");
  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  ASSERT_FALSE(resolveAST(ctx, semCtx, *parsed));
  ASSERT_EQ(2, diag.getErrCountClear());
}

void assertFunctionLikeSourceVisibility(
    llvh::Optional<ESTree::FunctionLikeNode *> funcLikeNode,
    SourceVisibility sourceVisibility) {
  ASSERT_TRUE(funcLikeNode.hasValue());
  ASSERT_EQ(
      (*funcLikeNode)->getSemInfo()->customDirectives.sourceVisibility,
      sourceVisibility);
}

void assertFirstNodeAsFunctionLikeWithSourceVisibility(
    llvh::Optional<ESTree::ProgramNode *> parsed,
    SourceVisibility sourceVisibility) {
  ASSERT_TRUE(parsed.hasValue());
  auto *programNode = llvh::cast<ESTree::ProgramNode>(parsed.getValue());
  ASSERT_TRUE(llvh::isa<ESTree::FunctionLikeNode>(programNode->_body.front()));
  auto *funcLikeNode =
      llvh::cast<ESTree::FunctionLikeNode>(&programNode->_body.front());

  ASSERT_EQ(
      funcLikeNode->getSemInfo()->customDirectives.sourceVisibility,
      sourceVisibility);
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

  ASSERT_EQ(
      funcLikeNode->getSemInfo()->customDirectives.sourceVisibility,
      sourceVisibility);
}

TEST(ResolverTest, SourceVisibilityTest) {
  Context context;
  sema::SemContext semCtx(context);
  // Top-level program node.
  {
    JSParser parser(context, "");
    auto parsed = parser.parse();
    resolveAST(context, semCtx, *parsed);
    assertFunctionLikeSourceVisibility(*parsed, SourceVisibility::Default);
  }
  {
    JSParser parser(context, "'show source'");
    auto parsed = parser.parse();
    resolveAST(context, semCtx, *parsed);
    // source visibility is correctly updated after semantic validation.
    assertFunctionLikeSourceVisibility(*parsed, SourceVisibility::ShowSource);
  }
  // Singleton function node.
  {
    JSParser parser(context, "function func (a, b) { return 10 }");
    auto parsed = parser.parse();
    resolveAST(context, semCtx, *parsed);
    assertFirstNodeAsFunctionLikeWithSourceVisibility(
        parsed, SourceVisibility::Default);
  }
  {
    JSParser parser(context, "function func (a, b) { 'sensitive'; return 10 }");
    auto parsed = parser.parse();
    resolveAST(context, semCtx, *parsed);
    assertFirstNodeAsFunctionLikeWithSourceVisibility(
        parsed, SourceVisibility::Sensitive);
  }
  {
    JSParser parser(
        context, "function func (a, b) { 'hide source'; return 10 }");
    auto parsed = parser.parse();
    resolveAST(context, semCtx, *parsed);
    assertFirstNodeAsFunctionLikeWithSourceVisibility(
        parsed, SourceVisibility::HideSource);
  }
  {
    JSParser parser(
        context, "function func (a, b) { 'show source'; return 10 }");
    auto parsed = parser.parse();
    resolveAST(context, semCtx, *parsed);
    assertFirstNodeAsFunctionLikeWithSourceVisibility(
        parsed, SourceVisibility::ShowSource);
  }
  // Visibility is correctly restored.
  {
    JSParser parser(context, "function foo(x) { 'sensitive' }function bar(){}");
    auto parsed = parser.parse();
    resolveAST(context, semCtx, *parsed);
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
    resolveAST(context, semCtx, *parsed);
    assertSecondNodeAsFunctionLikeWithSourceVisibility(
        parsed, SourceVisibility::ShowSource);
  }
  {
    // HideSource > ShowSource
    JSParser parser(
        context,
        "'show source'; function func (a, b) { 'hide source'; return 10 }");
    auto parsed = parser.parse();
    resolveAST(context, semCtx, *parsed);
    assertSecondNodeAsFunctionLikeWithSourceVisibility(
        parsed, SourceVisibility::HideSource);
  }
  {
    // ShowSource < HideSource
    JSParser parser(
        context,
        "'hide source'; function func (a, b) { 'show source'; return 10 }");
    auto parsed = parser.parse();
    resolveAST(context, semCtx, *parsed);
    assertSecondNodeAsFunctionLikeWithSourceVisibility(
        parsed, SourceVisibility::HideSource);
  }
  {
    // Sensitive > HideSource
    JSParser parser(
        context,
        "'hide source'; function func (a, b) { 'sensitive'; return 10 }");
    auto parsed = parser.parse();
    resolveAST(context, semCtx, *parsed);
    assertSecondNodeAsFunctionLikeWithSourceVisibility(
        parsed, SourceVisibility::Sensitive);
  }
}

} // anonymous namespace
