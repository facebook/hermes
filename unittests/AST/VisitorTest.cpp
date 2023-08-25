/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "llvh/Support/Casting.h"
#include "llvh/Support/SourceMgr.h"
#include "llvh/Support/YAMLParser.h"

#include "hermes/AST/ASTBuilder.h"
#include "hermes/AST/ESTreeVisitors.h"
#include "hermes/AST/RecursiveVisitor.h"
#include "hermes/Parser/JSONParser.h"

#include "gtest/gtest.h"
using llvh::cast;
using llvh::dyn_cast;

using namespace hermes;

namespace {

TEST(VisitorTest, NodeParentTest) {
  Context context;

  auto *statement = new (context)
      ESTree::AwaitExpressionNode(new (context) ESTree::NullLiteralNode());

  class Visitor {
   public:
    bool foundNull = false;

    bool incRecursionDepth(ESTree::Node *) {
      return true;
    }
    void decRecursionDepth() {}

    void visit(ESTree::Node *node, ESTree::Node *parent) {
      if (llvh::isa<ESTree::NullLiteralNode>(node)) {
        foundNull = true;
        ASSERT_TRUE(llvh::isa<ESTree::AwaitExpressionNode>(parent));
      }
      visitESTreeChildren(*this, node);
    }
  };

  Visitor v{};
  visitESTreeNode(v, statement);
  ASSERT_TRUE(v.foundNull);
}

} // end anonymous namespace
