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
  visitESTreeNodeNoReplace(v, statement);
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
  visitESTreeNodeNoReplace(v0, program);
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

        ESTree::visitESTreeNodeNoReplace(*this, &node);
      }
    }
  };

  VisitorWithVisitNodeList vv{};
  visitESTreeNodeNoReplace(vv, program);
  ASSERT_EQ(vv.countNullInNodeList, 3);
  ASSERT_EQ(vv.countNullOutsideNodeList, 2);
}

TEST(VisitorTest, MutateNodeTest) {
  Context context;

  auto *expr = new (context) ESTree::BinaryExpressionNode(
      new (context) ESTree::NullLiteralNode(),
      new (context) ESTree::NumericLiteralNode(1),
      context.getStringTable().getString("+"));

  class Visitor {
    Context &context_;

   public:
    explicit Visitor(Context &context) : context_(context) {}

    bool incRecursionDepth(ESTree::Node *) {
      return true;
    }
    void decRecursionDepth() {}

    void visit(ESTree::Node *node) {
      visitESTreeChildren(*this, node);
    }

    void visit(ESTree::NullLiteralNode *, ESTree::Node **ppNode) {
      *ppNode = new (context_) ESTree::BooleanLiteralNode(false);
    }
    void visit(
        ESTree::NumericLiteralNode *,
        ESTree::Node **ppNode,
        ESTree::Node *parent) {
      ASSERT_TRUE(llvh::isa<ESTree::BinaryExpressionNode>(parent));
      *ppNode = new (context_) ESTree::BooleanLiteralNode(true);
    }
  };

  Visitor v(context);
  ESTree::visitESTreeNodeNoReplace(v, expr);
  ASSERT_TRUE(llvh::isa<ESTree::BooleanLiteralNode>(expr->_left));
  ASSERT_EQ(false, llvh::cast<ESTree::BooleanLiteralNode>(expr->_left)->_value);
  ASSERT_TRUE(llvh::isa<ESTree::BooleanLiteralNode>(expr->_right));
  ASSERT_EQ(true, llvh::cast<ESTree::BooleanLiteralNode>(expr->_right)->_value);
}

TEST(VisitorTest, MutateNodeList) {
  Context context;

  // Create a program from the following code.
  //   return;
  //   1;
  //   break;
  //   2;
  //   label: continue;
  //   3;

  ESTree::NodeList body;

  body.push_back(*new (context) ESTree::ReturnStatementNode(nullptr));
  body.push_back(*new (context) ESTree::ExpressionStatementNode(
      new (context) ESTree::NumericLiteralNode(1), nullptr));
  body.push_back(*new (context) ESTree::BreakStatementNode(nullptr));
  body.push_back(*new (context) ESTree::ExpressionStatementNode(
      new (context) ESTree::NumericLiteralNode(2), nullptr));
  body.push_back(*new (context) ESTree::LabeledStatementNode(
      new (context) ESTree::IdentifierNode(
          context.getIdentifier("label").getUnderlyingPointer(),
          nullptr,
          false),
      new (context) ESTree::ContinueStatementNode(nullptr)));
  body.push_back(*new (context) ESTree::ExpressionStatementNode(
      new (context) ESTree::NumericLiteralNode(3), nullptr));

  auto *program = new (context) ESTree::ProgramNode(std::move(body));

  class Visitor {
   public:
    // NOTE: have to use an enum because statics are not allowed here.
    enum { kEnableNodeListMutation = 1 };

    bool incRecursionDepth(ESTree::Node *) {
      return true;
    }
    void decRecursionDepth() {}

    void visit(ESTree::Node *node) {
      visitESTreeChildren(*this, node);
    }

    void visit(ESTree::ReturnStatementNode *, ESTree::Node **ppNode) {
      // Delete the "return";
      *ppNode = nullptr;
    }
    void visit(ESTree::BreakStatementNode *, ESTree::Node **ppNode) {
      // Delete the "break";
      *ppNode = nullptr;
    }
    void visit(ESTree::LabeledStatementNode *node, ESTree::Node **ppNode) {
      // Replace the label with its body;
      *ppNode = node->_body;
      node->_body = nullptr;
    }
  };

  Visitor v;
  ESTree::visitESTreeNodeNoReplace(v, program);

  // Transformed program:
  //   1;
  //   2;
  //   label: continue;
  //   3;
  ASSERT_EQ(4, program->_body.size());
  auto it = program->_body.begin();
  // 1.
  ASSERT_TRUE(llvh::isa<ESTree::ExpressionStatementNode>(*it));
  ASSERT_EQ(
      1,
      llvh::cast<ESTree::NumericLiteralNode>(
          llvh::cast<ESTree::ExpressionStatementNode>(it)->_expression)
          ->_value);
  // 2.
  ++it;
  ASSERT_TRUE(llvh::isa<ESTree::ExpressionStatementNode>(*it));
  ASSERT_EQ(
      2,
      llvh::cast<ESTree::NumericLiteralNode>(
          llvh::cast<ESTree::ExpressionStatementNode>(it)->_expression)
          ->_value);
  // continue;
  ++it;
  ASSERT_TRUE(llvh::isa<ESTree::ContinueStatementNode>(*it));
  // 3;
  ++it;
  ASSERT_TRUE(llvh::isa<ESTree::ExpressionStatementNode>(*it));
  ASSERT_EQ(
      3,
      llvh::cast<ESTree::NumericLiteralNode>(
          llvh::cast<ESTree::ExpressionStatementNode>(it)->_expression)
          ->_value);
}

} // end anonymous namespace
