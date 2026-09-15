/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "llvh/Support/Casting.h"
#include "llvh/Support/SourceMgr.h"

#include "hermes/AST/ASTBuilder.h"
#include "hermes/Parser/JSONParser.h"

#include "gtest/gtest.h"
using llvh::cast;
using llvh::dyn_cast;

using namespace hermes;

namespace {

const char *JSONExample =
    "{"
    "    \"type\": \"Program\","
    "    \"sourceType\": \"script\","
    "    \"body\": [], "
    "    \"directives\": []"
    "}";

TEST(JSONParserTest, SimpleParserTest) {
  Context context;
  parser::JSONFactory factory(context.getAllocator());
  parser::JSONParser parser(
      factory, JSONExample, context.getSourceErrorManager());

  auto parsed = parser.parse();
  ASSERT_TRUE(parsed.hasValue());
  auto Node = hermes::ESTree::buildAST(context, parsed.getValue());
  EXPECT_TRUE(Node.hasValue());
}

/// Regression test: deeply nested input used to overflow the stack; it must
/// be rejected with an error instead.
TEST(JSONParserTest, DeepNestingTest) {
  Context context;
  parser::JSONFactory factory(context.getAllocator());
  std::string deep(100000, '[');
  parser::JSONParser parser(factory, deep, context.getSourceErrorManager());

  auto parsed = parser.parse();
  EXPECT_FALSE(parsed.hasValue());
  EXPECT_NE(0u, context.getSourceErrorManager().getErrorCount());
}

} // end anonymous namespace
