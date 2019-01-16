#include "llvm/Support/Casting.h"
#include "llvm/Support/SourceMgr.h"

#include "hermes/AST/ASTBuilder.h"
#include "hermes/Parser/JSONParser.h"

#include "gtest/gtest.h"
using llvm::cast;
using llvm::dyn_cast;

using namespace hermes;

namespace {

const char *JSONExample =
    "{"
    "  \"type\": \"File\","
    "  \"program\": {"
    "    \"type\": \"Program\","
    "    \"sourceType\": \"script\","
    "    \"body\": [], "
    "    \"directives\": []"
    "  },  "
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

} // end anonymous namespace
