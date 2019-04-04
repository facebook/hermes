/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/AST/ESTree.h"

#include "llvm/Support/raw_ostream.h"

using llvm::dyn_cast;

namespace hermes {
namespace ESTree {

llvm::Optional<NodeLabel> matchDirective(const ESTree::Node *node) {
  auto *exprSt = dyn_cast<ESTree::ExpressionStatementNode>(node);
  if (!exprSt)
    return llvm::None;

  if (exprSt->_directive)
    return exprSt->_directive;

  auto *strLit = dyn_cast<ESTree::StringLiteralNode>(exprSt->_expression);
  if (!strLit || !strLit->potentialDirective)
    return llvm::None;

  return strLit->_value;
}

NodeList &getParams(FunctionLikeNode *node) {
  switch (node->getKind()) {
    default:
      assert(
          node->getKind() == NodeKind::Program && "invalid FunctionLikeNode");
      return cast<ProgramNode>(node)->dummyParamList;
    case NodeKind::FunctionExpression:
      return cast<FunctionExpressionNode>(node)->_params;
    case NodeKind::ArrowFunctionExpression:
      return cast<ArrowFunctionExpressionNode>(node)->_params;
    case NodeKind::FunctionDeclaration:
      return cast<FunctionDeclarationNode>(node)->_params;
  }
}

BlockStatementNode *getBlockStatement(FunctionLikeNode *node) {
  switch (node->getKind()) {
    default:
      assert(
          node->getKind() == NodeKind::Program && "invalid FunctionLikeNode");
      return nullptr;
    case NodeKind::FunctionExpression:
      return cast<BlockStatementNode>(
          cast<FunctionExpressionNode>(node)->_body);
    case NodeKind::FunctionDeclaration:
      return cast<BlockStatementNode>(
          cast<FunctionDeclarationNode>(node)->_body);
    case NodeKind::ArrowFunctionExpression: {
      return dyn_cast<BlockStatementNode>(
          cast<FunctionDeclarationNode>(node)->_body);
    }
  }
}

} // namespace ESTree
} // namespace hermes
