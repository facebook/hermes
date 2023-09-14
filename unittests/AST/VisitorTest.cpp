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

TEST(VisitorTest, VisitNodeListTest) {
  Context context;

  // Create a program from the following code. There are total 5 nulls, 3
  // are in NodeList and 2 are not in NodeList.
  //   null;
  //   null;
  //   [null, null, null];

  ESTree::NodeList body;

  ESTree::NodeList elements;
  elements.push_back(*new (context) ESTree::NullLiteralNode());
  elements.push_back(*new (context) ESTree::NullLiteralNode());
  elements.push_back(*new (context) ESTree::NullLiteralNode());
  auto *array =
      new (context) ESTree::ArrayExpressionNode(std::move(elements), false);

  body.push_back(*new (context) ESTree::ExpressionStatementNode(
      new (context) ESTree::NullLiteralNode(), nullptr));
  body.push_back(*new (context) ESTree::ExpressionStatementNode(
      new (context) ESTree::NullLiteralNode(), nullptr));
  body.push_back(*new (context)
                     ESTree::ExpressionStatementNode(array, nullptr));

  auto *program = new (context) ESTree::ProgramNode(std::move(body));

  // Count the number of NullLiteralNode
  class Visitor {
   public:
    size_t countNull{0};

    bool incRecursionDepth(ESTree::Node *) {
      return true;
    }
    void decRecursionDepth() {}

    void visit(ESTree::Node *node) {
      visitESTreeChildren(*this, node);
    }

    void visit(ESTree::NullLiteralNode *node) {
      ++countNull;
    }
  };

  Visitor v0{};
  visitESTreeNode(v0, program);
  ASSERT_EQ(v0.countNull, 5);

  // Count the number of NullLiteralNode inside NodeList and outside NodeList
  // separately.
  class VisitorWithVisitNodeList {
   public:
    size_t countNullInNodeList{0};
    size_t countNullOutsideNodeList{0};

    bool incRecursionDepth(ESTree::Node *) {
      return true;
    }
    void decRecursionDepth() {}

    void visit(ESTree::Node *node) {
      visitESTreeChildren(*this, node);
    }

    void visit(ESTree::NullLiteralNode *node) {
      ++countNullOutsideNodeList;
    }

    void visit(ESTree::NodeList &list, ESTree::Node *parent) {
      for (auto &node : list) {
        if (llvh::isa<ESTree::NullLiteralNode>(node)) {
          ++countNullInNodeList;
          continue; // skip calling visit(ESTree::NullLiteralNode *node).
        }

        ESTree::visitESTreeNode(*this, &node);
      }
    }
  };

  VisitorWithVisitNodeList vv{};
  visitESTreeNode(vv, program);
  ASSERT_EQ(vv.countNullInNodeList, 3);
  ASSERT_EQ(vv.countNullOutsideNodeList, 2);
}

} // end anonymous namespace
