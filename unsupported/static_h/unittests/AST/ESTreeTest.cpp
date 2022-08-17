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
#include "hermes/Parser/JSONParser.h"

#include "gtest/gtest.h"
using llvh::cast;
using llvh::dyn_cast;

using namespace hermes;

namespace {

const char *SimpleESTreeProgram =
    "  {"
    "      \"type\": \"Program\","
    "      \"body\": ["
    "        {   "
    "          \"type\": \"FunctionDeclaration\","
    "          \"id\": {"
    "            \"type\": \"Identifier\","
    "            \"name\": \"my_name_is_foo\","
    "            \"optional\": false"
    "          },  "
    "          \"params\": [], "
    "          \"body\": {"
    "            \"type\": \"BlockStatement\","
    "            \"body\": [], "
    "          },  "
    "          \"generator\": false,"
    "          \"async\": false"
    "        }   "
    "      ],  "
    "  }";

TEST(ESTreeTest, EmptyTest) {
  Context context;
  parser::JSONFactory factory(context.getAllocator());
  parser::JSONParser parser(
      factory, SimpleESTreeProgram, context.getSourceErrorManager());

  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());

  auto ast = ESTree::buildAST(context, parsed.getValue());
  ASSERT_TRUE(ast.hasValue());
  auto Node = ast.getValue();

  /// Collect some information about the graph traits.
  ESTree::TreeTrait TT;
  Node->visit(TT);

  EXPECT_EQ((int)TT.CurrDepth, 0);
  EXPECT_EQ((int)TT.MaxDepth, 3);
  EXPECT_EQ((int)TT.Count, 4);

  auto &ProgramBody = cast<ESTree::ProgramNode>(Node)->_body;
  auto FuncDecl = cast<ESTree::FunctionDeclarationNode>(&ProgramBody.front());
  auto Name = cast<ESTree::IdentifierNode>(FuncDecl->_id)->_name;

  EXPECT_STREQ("my_name_is_foo", Name->c_str());
}

} // end anonymous namespace
