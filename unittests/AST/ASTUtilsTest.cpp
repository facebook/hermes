/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/ASTUtils.h"
#include "hermes/AST/ESTree.h"

#include "gtest/gtest.h"

using namespace hermes;

namespace {

#define LABEL(name) (ctx.getIdentifier(name).getUnderlyingPointer())

/// Helper to create an IdentifierNode.
ESTree::IdentifierNode *makeId(Context &ctx, llvh::StringRef name) {
  return new (ctx) ESTree::IdentifierNode(LABEL(name), nullptr, false);
}

/// Helper to create a DecoratorNode with an identifier expression.
ESTree::DecoratorNode *makeDecorator(Context &ctx, llvh::StringRef name) {
  return new (ctx) ESTree::DecoratorNode(makeId(ctx, name));
}

/// Helper to create a DecoratorNode with a member expression.
/// For example, makeMemberDecorator(ctx, {"a", "b", "c"}) creates @a.b.c.
ESTree::DecoratorNode *makeMemberDecorator(
    Context &ctx,
    llvh::ArrayRef<llvh::StringRef> names) {
  assert(names.size() >= 2 && "Need at least 2 names for member expression");
  auto it = names.begin();
  ESTree::Node *cur = makeId(ctx, *it++);
  while (it != names.end()) {
    cur =
        new (ctx) ESTree::MemberExpressionNode(cur, makeId(ctx, *it++), false);
  }
  return new (ctx) ESTree::DecoratorNode(cur);
}

TEST(ASTUtilsTest, FindDecoratorSingleNameMatch) {
  Context ctx;
  ESTree::NodeList decorators;
  auto *dec = makeDecorator(ctx, "foo");
  decorators.push_back(*dec);

  auto result = findDecorator(decorators, {LABEL("foo")});
  EXPECT_EQ(result, dec);
}

TEST(ASTUtilsTest, FindDecoratorSingleNameNoMatch) {
  Context ctx;
  ESTree::NodeList decorators;
  decorators.push_back(*makeDecorator(ctx, "foo"));

  auto result = findDecorator(decorators, {LABEL("bar")});
  EXPECT_EQ(result, nullptr);
}

TEST(ASTUtilsTest, FindDecoratorSingleNameMultipleDecorators) {
  Context ctx;
  ESTree::NodeList decorators;
  // @foo
  decorators.push_back(*makeDecorator(ctx, "foo"));
  // @bar
  auto *bar = makeDecorator(ctx, "bar");
  decorators.push_back(*bar);
  // @baz
  decorators.push_back(*makeDecorator(ctx, "baz"));

  auto result = findDecorator(decorators, {LABEL("bar")});
  EXPECT_EQ(result, bar);
}

TEST(ASTUtilsTest, FindDecoratorMemberExpressionMatch) {
  Context ctx;
  ESTree::NodeList decorators;
  // @a.b.c
  auto *dec = makeMemberDecorator(ctx, {"a", "b", "c"});
  decorators.push_back(*dec);

  auto result = findDecorator(decorators, {LABEL("a"), LABEL("b"), LABEL("c")});
  EXPECT_EQ(result, dec);
}

TEST(ASTUtilsTest, FindDecoratorMemberExpressionMultipleDecorators) {
  Context ctx;

  ESTree::NodeList decorators;
  // @x
  decorators.push_back(*makeDecorator(ctx, "x"));
  // @a.b
  auto *ab = makeMemberDecorator(ctx, {"a", "b"});
  decorators.push_back(*ab);
  // @y
  decorators.push_back(*makeDecorator(ctx, "y"));

  auto result = findDecorator(decorators, {LABEL("a"), LABEL("b")});
  EXPECT_EQ(result, ab);
}

} // namespace
