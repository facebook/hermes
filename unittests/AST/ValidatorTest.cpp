/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
} // anonymous namespace
