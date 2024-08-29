/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/ASTUtils.h"

#include "hermes/AST/ESTree.h"

namespace hermes {

/// Wrap the given program in a IIFE, e.g.
/// (function(exports){ ...program body })({});
/// This function cannot fail.
ESTree::ProgramNode *wrapInIIFE(
    std::shared_ptr<Context> &context,
    ESTree::ProgramNode *program) {
  // Function body should be the given program body.
  auto *funcBody =
      new (*context) ESTree::BlockStatementNode(std::move(program->_body));
  funcBody->setSourceRange(program->getSourceRange());
  funcBody->setDebugLoc(program->getDebugLoc());

  // Add an 'exports' parameter to the function.
  ESTree::NodeList argNames{};
  auto *exports = new (*context) ESTree::IdentifierNode(
      context->getIdentifier("exports").getUnderlyingPointer(), nullptr, false);
  argNames.push_back(*exports);
  auto *wrappedFn = new (*context) ESTree::FunctionExpressionNode(
      nullptr,
      std::move(argNames),
      funcBody,
      nullptr,
      nullptr,
      nullptr,
      false,
      false);
  wrappedFn->setSourceRange(program->getSourceRange());
  wrappedFn->setDebugLoc(program->getDebugLoc());

  // Supply an empty object to the call expression.
  auto *emptyObj = new (*context) ESTree::ObjectExpressionNode({});
  ESTree::NodeList suppliedArgs{};
  suppliedArgs.push_back(*emptyObj);

  // Call the function.
  auto *callExpr = new (*context)
      ESTree::CallExpressionNode(wrappedFn, nullptr, std::move(suppliedArgs));
  // Create a top level expression statement of the function call.
  auto *topLevelExpr =
      new (*context) ESTree::ExpressionStatementNode(callExpr, nullptr);

  ESTree::NodeList stmtList;
  stmtList.push_back(*topLevelExpr);
  program->_body = std::move(stmtList);

  return program;
}

} // namespace hermes
