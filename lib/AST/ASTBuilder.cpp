/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/AST/ASTBuilder.h"
#include "hermes/Support/UTF8.h"

// Use this value to enable debug logging from the command line.
#define DEBUG_TYPE "ast"

using namespace hermes;

using llvm::cast;
using llvm::dyn_cast;
using llvm::dyn_cast_or_null;
using llvm::isa;
using llvm::None;
using llvm::StringRef;
using parser::JSONArray;
using parser::JSONBoolean;
using parser::JSONNumber;
using parser::JSONObject;
using parser::JSONScalar;
using parser::JSONString;
using parser::JSONValue;

namespace hermes {
namespace ESTree {

namespace {

class ASTBuilder {
  ASTBuilder(const ASTBuilder &) = delete;
  void operator=(const ASTBuilder &) = delete;

  Context &context_;
  SourceErrorManager &sm_;
  /// The optional JavaScript source of the ESTree.
  const llvm::MemoryBuffer *jsSource_;

 public:
  explicit ASTBuilder(Context &context, const llvm::MemoryBuffer *jsSource)
      : context_(context),
        sm_(context.getSourceErrorManager()),
        jsSource_(jsSource) {}

  /// Deserialize the ESTree from the input JSON.
  /// \returns the deserialized data structure. The program asserts and crashes
  /// if the data structure is malformed.
  llvm::Optional<Node *> build(const parser::JSONValue *node);

 private:
  // Extractors for the different types of ESTree fields that we support.
  // Each extractor checks if a value of the specified name and type exists,
  // in which case it stores it in 'result' and returns true. Otherwise it
  // returns false.

  bool extractNodeLabel(
      const parser::JSONObject *jsObj,
      StringRef name,
      UniqueString *&result);
  bool extractNodeBoolean(
      const parser::JSONObject *jsObj,
      StringRef name,
      bool &result);
  bool extractNodeNumber(
      const parser::JSONObject *jsObj,
      StringRef name,
      double &result);
  bool extractNodePtr(
      const parser::JSONObject *jsObj,
      StringRef name,
      NodePtr &result);
  bool extractNodeList(
      const parser::JSONObject *jsObj,
      StringRef name,
      NodeList &result);

  /// Convert an ESTree Literal node into a type-specific tttLiteral node.
  /// Report an error and return None on error.
  llvm::Optional<ESTree::Node *> convertLiteral(const parser::JSONObject *lit);
};

bool ASTBuilder::extractNodeLabel(
    const JSONObject *jsObj,
    StringRef name,
    UniqueString *&result) {
  auto strObj = dyn_cast<JSONString>(jsObj->get(name));
  if (!strObj)
    return false;

  result = strObj->stringBase();
  return true;
}

bool ASTBuilder::extractNodeBoolean(
    const JSONObject *jsObj,
    StringRef name,
    bool &result) {
  auto boolObj = dyn_cast<JSONBoolean>(jsObj->get(name));
  if (!boolObj)
    return false;

  result = boolObj->getValue();
  return true;
}

bool ASTBuilder::extractNodeNumber(
    const JSONObject *jsObj,
    StringRef name,
    double &result) {
  auto numberObj = dyn_cast<JSONNumber>(jsObj->get(name));
  if (!numberObj)
    return false;

  result = numberObj->getValue();
  return true;
}

bool ASTBuilder::extractNodePtr(
    const JSONObject *jsObj,
    StringRef name,
    NodePtr &result) {
  auto obj = dyn_cast_or_null<JSONObject>(jsObj->get(name));
  if (!obj)
    return false;

  auto tmp = build(obj);
  if (!tmp)
    return false;

  result = *tmp;
  return true;
}

bool ASTBuilder::extractNodeList(
    const JSONObject *jsObj,
    StringRef name,
    NodeList &result) {
  auto arr = dyn_cast<JSONArray>(jsObj->get(name));
  if (!arr)
    return false;

  for (const JSONValue *val : *arr) {
    auto elem = build(val);
    if (!elem)
      return false;

    result.push_back(**elem);
  }

  return true;
}

llvm::Optional<Node *> ASTBuilder::convertLiteral(
    const parser::JSONObject *lit) {
  // First check if it is a RegExpLiteral, which is determined by the presence
  // of the 'regex' value.
  if (auto *regex = dyn_cast_or_null<JSONObject>(lit->get("regex"))) {
    NodeLabel pattern{};
    NodeLabel flags{};
    if (!extractNodeLabel(regex, "pattern", pattern)) {
      sm_.error(SMLoc{}, "Invalid field 'pattern' in Literal/regex");
      return None;
    }
    if (!extractNodeLabel(regex, "flags", flags)) {
      sm_.error(SMLoc{}, "Invalid field 'flags' in Literal/regex");
      return None;
    }

    return new (context_) RegExpLiteralNode(pattern, flags);
  };

  // No classify the 'value'.
  JSONValue *value = lit->get("value");
  if (!value) {
    sm_.error(SMLoc{}, "Invalid field 'value' in Literal");
    return None;
  }

  switch (value->getKind()) {
    case parser::JSONKind::String: {
      auto *jsonString = cast<JSONString>(value);
      auto *node = new (context_) StringLiteralNode(jsonString->stringBase());
      // Determine whether the string literal contained any escapes or line
      // continuations so we can detect directives.
      // The check for size 2 is just a sanity check - the raw string must at
      // least have quotation marks.
      UniqueString *raw;
      if (extractNodeLabel(lit, "raw", raw) && raw->str().size() >= 2) {
        StringRef str = raw->str();
        if (str[0] == '\\' && str.size() >= 4) {
          // Remove the escaped quotes.
          str = str.slice(2, str.size() - 2);
        } else {
          // Remove the quotes.
          str = str.slice(1, str.size() - 1);
        }

        if (!str.find('\\'))
          node->potentialDirective = true;
      }
      return node;
    }
    case parser::JSONKind::Boolean:
      return new (context_)
          BooleanLiteralNode(cast<JSONBoolean>(value)->getValue());
    case parser::JSONKind::Null:
      return new (context_) NullLiteralNode();
    case parser::JSONKind::Number:
      return new (context_)
          NumericLiteralNode(cast<JSONNumber>(value)->getValue());
    default:
      sm_.error(SMLoc{}, "Invalid field 'value' in Literal");
      return None;
  }
}

/// Constructs an ESTree object from the node \p Node and sets the parent of the
/// node to \p Parent, that may be null.
llvm::Optional<Node *> ASTBuilder::build(const JSONValue *node) {
  if (isa<parser::JSONNull>(node))
    return new (context_) EmptyNode();

  auto jsObj = dyn_cast<JSONObject>(node);
  if (!jsObj) {
    sm_.error(SMLoc{}, "node is not an object");
    return None;
  }
  assert(jsObj && "invalid node type");

  // Get the type of the serialized object.
  auto TypeItr = jsObj->find("type");

  // If the 'type' field is unspecified then this is a metadata object.
  if (TypeItr == jsObj->end())
    return new (context_) ESTree::MetadataNode();

  // Read the name of the property.
  assert(isa<JSONString>((*TypeItr).second) && "invalid 'type' property");
  StringRef Typename = dyn_cast<JSONString>((*TypeItr).second)->str();

  /*  // Ignore the following nodes as they are not supported.
    if (Typename == "CommentLine" || Typename == "CommentBlock") {
      return nullptr;
    }*/

  // Optionally extract location information.
  SMRange sourceRng;
  if (jsSource_) {
    const auto *start = dyn_cast_or_null<JSONNumber>(jsObj->get("start"));
    const auto *end = dyn_cast_or_null<JSONNumber>(jsObj->get("end"));

    // If start or end are missing, check for range array.
    if (!start || !end) {
      if (auto *range = dyn_cast_or_null<JSONArray>(jsObj->get("range"))) {
        if (range->size() == 2) {
          start = dyn_cast_or_null<JSONNumber>(range->at(0));
          end = dyn_cast_or_null<JSONNumber>(range->at(1));
        }
      }
    }

    if (start && end) {
      auto startN = (int)start->getValue();
      auto endN = (int)end->getValue();

      if (startN >= 0 && startN < (int)jsSource_->getBufferSize() &&
          endN >= startN && endN <= (int)jsSource_->getBufferSize()) {
        sourceRng.Start =
            SMLoc::getFromPointer(jsSource_->getBufferStart() + startN);
        sourceRng.End =
            SMLoc::getFromPointer(jsSource_->getBufferStart() + endN);
      }
    }
  }

  Node *result;

  // Special-case ESTree Literal nodes, which we have to convert to a type-
  // specific literal.
  if (Typename == "Literal") {
    auto lit = convertLiteral(jsObj);
    if (!lit)
      return None;
    result = *lit;
  } else
#define ESTREE_NODE_0_ARGS(NAME)          \
  if (Typename == #NAME) {                \
    result = new (context_) NAME##Node(); \
  } else

#define ESTREE_NODE_1_ARGS(NAME, ARG0TY, ARG0NM, ARG0OPT)          \
  if (Typename == #NAME) {                                         \
    ARG0TY arg0{};                                                 \
    if (!extract##ARG0TY(jsObj, #ARG0NM, arg0) && !(ARG0OPT)) {    \
      sm_.error(SMLoc{}, "Invalid field '" #ARG0NM "' in " #NAME); \
      return None;                                                 \
    }                                                              \
    result = new (context_) NAME##Node(std::move(arg0));           \
  } else

#define ESTREE_NODE_2_ARGS(                                               \
    NAME, ARG0TY, ARG0NM, ARG0OPT, ARG1TY, ARG1NM, ARG1OPT)               \
  if (Typename == #NAME) {                                                \
    ARG0TY arg0{};                                                        \
    ARG1TY arg1{};                                                        \
    if (!extract##ARG0TY(jsObj, #ARG0NM, arg0) && !(ARG0OPT)) {           \
      sm_.error(SMLoc{}, "Invalid field '" #ARG0NM "' in " #NAME);        \
      return None;                                                        \
    }                                                                     \
    if (!extract##ARG1TY(jsObj, #ARG1NM, arg1) && !(ARG1OPT)) {           \
      sm_.error(SMLoc{}, "Invalid field '" #ARG1NM "' in " #NAME);        \
      return None;                                                        \
    }                                                                     \
    result = new (context_) NAME##Node(std::move(arg0), std::move(arg1)); \
  } else

#define ESTREE_NODE_3_ARGS(                                            \
    NAME,                                                              \
    ARG0TY,                                                            \
    ARG0NM,                                                            \
    ARG0OPT,                                                           \
    ARG1TY,                                                            \
    ARG1NM,                                                            \
    ARG1OPT,                                                           \
    ARG2TY,                                                            \
    ARG2NM,                                                            \
    ARG2OPT)                                                           \
  if (Typename == #NAME) {                                             \
    ARG0TY arg0{};                                                     \
    ARG1TY arg1{};                                                     \
    ARG2TY arg2{};                                                     \
    if (!extract##ARG0TY(jsObj, #ARG0NM, arg0) && !(ARG0OPT)) {        \
      sm_.error(SMLoc{}, "Invalid field '" #ARG0NM "' in " #NAME);     \
      return None;                                                     \
    }                                                                  \
    if (!extract##ARG1TY(jsObj, #ARG1NM, arg1) && !(ARG1OPT)) {        \
      sm_.error(SMLoc{}, "Invalid field '" #ARG1NM "' in " #NAME);     \
      return None;                                                     \
    }                                                                  \
    if (!extract##ARG2TY(jsObj, #ARG2NM, arg2) && !(ARG2OPT)) {        \
      sm_.error(SMLoc{}, "Invalid field '" #ARG2NM "' in " #NAME);     \
      return None;                                                     \
    }                                                                  \
    result = new (context_)                                            \
        NAME##Node(std::move(arg0), std::move(arg1), std::move(arg2)); \
  } else

#define ESTREE_NODE_4_ARGS(                                                  \
    NAME,                                                                    \
    ARG0TY,                                                                  \
    ARG0NM,                                                                  \
    ARG0OPT,                                                                 \
    ARG1TY,                                                                  \
    ARG1NM,                                                                  \
    ARG1OPT,                                                                 \
    ARG2TY,                                                                  \
    ARG2NM,                                                                  \
    ARG2OPT,                                                                 \
    ARG3TY,                                                                  \
    ARG3NM,                                                                  \
    ARG3OPT)                                                                 \
  if (Typename == #NAME) {                                                   \
    ARG0TY arg0{};                                                           \
    ARG1TY arg1{};                                                           \
    ARG2TY arg2{};                                                           \
    ARG3TY arg3{};                                                           \
    if (!extract##ARG0TY(jsObj, #ARG0NM, arg0) && !(ARG0OPT)) {              \
      sm_.error(SMLoc{}, "Invalid field '" #ARG0NM "' in " #NAME);           \
      return None;                                                           \
    }                                                                        \
    if (!extract##ARG1TY(jsObj, #ARG1NM, arg1) && !(ARG1OPT)) {              \
      sm_.error(SMLoc{}, "Invalid field '" #ARG1NM "' in " #NAME);           \
      return None;                                                           \
    }                                                                        \
    if (!extract##ARG2TY(jsObj, #ARG2NM, arg2) && !(ARG2OPT)) {              \
      sm_.error(SMLoc{}, "Invalid field '" #ARG2NM "' in " #NAME);           \
      return None;                                                           \
    }                                                                        \
    if (!extract##ARG3TY(jsObj, #ARG3NM, arg3) && !(ARG3OPT)) {              \
      sm_.error(SMLoc{}, "Invalid field '" #ARG3NM "' in " #NAME);           \
      return None;                                                           \
    }                                                                        \
    result = new (context_) NAME##Node(                                      \
        std::move(arg0), std::move(arg1), std::move(arg2), std::move(arg3)); \
  } else

#include "hermes/AST/ESTree.def"

  {
    sm_.error(SMLoc{}, Twine("Unknown node type '") + Typename + "'");
    return None;
  }

  // Convert Property into ObjectProperty or ObjectMethod for compatibility
  // between ESTree and our internal representation.
  if (auto *prop = dyn_cast<PropertyNode>(result)) {
    if (prop->_kind->str() == "init") {
      result = new (context_) ObjectPropertyNode(prop->_key, prop->_value);
    } else {
      auto *funcExpr = dyn_cast<FunctionExpressionNode>(prop->_value);
      if (!funcExpr) {
        sm_.error(
            SMLoc{},
            Twine("Invalid field 'value' in Property(") + prop->_kind->str() +
                ")");
        return None;
      }
      result = new (context_) ObjectMethodNode(
          prop->_kind,
          prop->_key,
          std::move(funcExpr->_params),
          funcExpr->_body);
    }
  }

  result->setSourceRange(sourceRng);
  return result;
}

/// Visits all the ast nodes and synthesizes debug info for each one.
struct DebugLocationSynthesizer {
  /// Should visit all the nodes
  bool shouldVisit(Node *node) {
    return true;
  }

  void enter(Node *node) {}

  void leave(UpdateExpressionNode *node) {
    node->setDebugLoc(
        node->_prefix ? node->getStartLoc() : locBefore(node->getEndLoc()));
  }

  void leave(Node *node) {
    auto kind = node->getKind();
    if (kind == NodeKind::CallExpression ||
        kind == NodeKind::MemberExpression) {
      node->setDebugLoc(locBefore(node->getEndLoc()));
    } else {
      node->setDebugLoc(node->getStartLoc());
    }
  }

  static SMLoc locBefore(SMLoc loc) {
    return loc.isValid()
        ? SMLoc::getFromPointer(previousUTF8Start(loc.getPointer()))
        : loc;
  }
};

} // namespace

llvm::Optional<Node *> buildAST(
    Context &context,
    const parser::JSONValue *node,
    const llvm::MemoryBuffer *jsSource) {
  auto result = ASTBuilder(context, jsSource).build(node);
  DebugLocationSynthesizer synthesizer;
  ESTreeVisit(synthesizer, result.getValue());
  return result;
}

} // namespace ESTree
} // namespace hermes
