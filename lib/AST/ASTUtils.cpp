/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/ASTUtils.h"

#include "hermes/AST/ESTree.h"

namespace hermes {

/// Wrap the given program in an IIFE.
/// Without prelude: (function(exports){ ...program body... })({})
/// With prelude:
///   (function(){ ...prelude... (function(exports){ ...program... })({}) })()
/// \return {wrappedAST, originalFunc}.
std::pair<ESTree::ProgramNode *, ESTree::FunctionExpressionNode *> wrapInIIFE(
    std::shared_ptr<Context> &context,
    ESTree::ProgramNode *program,
    ESTree::ProgramNode *prelude) {
  // Build function body from program body statements.
  ESTree::NodeList innerBodyStmts;
  innerBodyStmts.splice(innerBodyStmts.end(), program->_body);

  auto *innerBody = new (*context)
      ESTree::BlockStatementNode(std::move(innerBodyStmts), false);
  innerBody->setSourceRange(program->getSourceRange());
  innerBody->setDebugLoc(program->getDebugLoc());

  // Add an 'exports' parameter to the inner function.
  ESTree::NodeList argNames{};
  auto *exports = new (*context) ESTree::IdentifierNode(
      context->getIdentifier("exports").getUnderlyingPointer(), nullptr, false);
  argNames.push_back(*exports);
  auto *originalFunc = new (*context) ESTree::FunctionExpressionNode(
      nullptr,
      std::move(argNames),
      innerBody,
      nullptr,
      nullptr,
      nullptr,
      false,
      false);
  originalFunc->setSourceRange(program->getSourceRange());
  originalFunc->setDebugLoc(program->getDebugLoc());

  // Supply an empty object to the inner call expression.
  auto *emptyObj = new (*context) ESTree::ObjectExpressionNode({});
  ESTree::NodeList innerArgs{};
  innerArgs.push_back(*emptyObj);

  // Call the inner function: (function(exports){...})({})
  auto *innerCallExpr = new (*context)
      ESTree::CallExpressionNode(originalFunc, nullptr, std::move(innerArgs));
  innerCallExpr->setSourceRange(program->getSourceRange());
  innerCallExpr->setDebugLoc(program->getDebugLoc());

  if (!prelude) {
    // No prelude: just wrap in a single IIFE.
    auto *topLevelExpr =
        new (*context) ESTree::ExpressionStatementNode(innerCallExpr, nullptr);

    ESTree::NodeList stmtList;
    stmtList.push_back(*topLevelExpr);
    program->_body = std::move(stmtList);

    return {program, originalFunc};
  }

  // With prelude: wrap in nested IIFEs.
  // (function(){ ...prelude... ; return (function(exports){...})({}) })()

  // Wrap inner call in a return statement.
  auto *innerCallStmt =
      new (*context) ESTree::ReturnStatementNode(innerCallExpr);

  // Build outer function body: prelude statements + inner call statement.
  ESTree::NodeList outerBodyStmts;
  outerBodyStmts.splice(outerBodyStmts.end(), prelude->_body);
  outerBodyStmts.push_back(*innerCallStmt);

  auto *outerBody = new (*context)
      ESTree::BlockStatementNode(std::move(outerBodyStmts), false);
  outerBody->setSourceRange(program->getSourceRange());
  outerBody->setDebugLoc(program->getDebugLoc());

  // Create outer function with no params.
  ESTree::NodeList outerParams{};
  auto *outerFunc = new (*context) ESTree::FunctionExpressionNode(
      nullptr,
      std::move(outerParams),
      outerBody,
      nullptr,
      nullptr,
      nullptr,
      false,
      false);
  outerFunc->setSourceRange(program->getSourceRange());
  outerFunc->setDebugLoc(program->getDebugLoc());

  // Call the outer function with no args: (function(){...})()
  ESTree::NodeList outerArgs{};
  auto *outerCallExpr = new (*context)
      ESTree::CallExpressionNode(outerFunc, nullptr, std::move(outerArgs));
  outerCallExpr->setSourceRange(program->getSourceRange());
  outerCallExpr->setDebugLoc(program->getDebugLoc());

  auto *topLevelExpr =
      new (*context) ESTree::ExpressionStatementNode(outerCallExpr, nullptr);

  ESTree::NodeList stmtList;
  stmtList.push_back(*topLevelExpr);
  program->_body = std::move(stmtList);

  return {program, originalFunc};
}

ESTree::DecoratorNode *findDecorator(
    ESTree::NodeList &decorators,
    llvh::ArrayRef<ESTree::NodeLabel> names) {
  assert(!names.empty() && "can't search for no names");

  // Special case: no member expressions.
  if (names.size() == 1) {
    for (ESTree::Node &dec : decorators) {
      auto *decNode = llvh::cast<ESTree::DecoratorNode>(&dec);
      auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(decNode->_expression);
      if (id && id->_name == names.front())
        return decNode;
    }
    return nullptr;
  }

  /// \return true when the \p decorator matches the names in the \c names list.
  auto matchMemberExpression = [&names](ESTree::DecoratorNode *decorator) {
    ESTree::Node *cur = decorator->_expression;
    // Iterate backwards because nested member expressions have the final
    // element in the chain higher in the AST.
    for (size_t nameIdx = names.size() - 1; nameIdx > 0; --nameIdx) {
      auto *me = llvh::dyn_cast<ESTree::MemberExpressionNode>(cur);
      if (!me)
        return false;
      auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(me->_property);
      if (!id)
        return false;

      // Check that the property name is correct.
      if (id->_name != names[nameIdx])
        return false;

      if (auto *idObj = llvh::dyn_cast<ESTree::IdentifierNode>(me->_object)) {
        // If the object is also an ID, then we should be done.
        // Make sure we're done iterating and check the name.
        if (nameIdx != 1)
          return false;
        return idObj->_name == names[0];
      } else {
        cur = me->_object;
      }
    }

    return false;
  };

  for (ESTree::Node &dec : decorators) {
    auto *decNode = llvh::cast<ESTree::DecoratorNode>(&dec);
    if (matchMemberExpression(decNode))
      return decNode;
  }
  return nullptr;
}

} // namespace hermes
