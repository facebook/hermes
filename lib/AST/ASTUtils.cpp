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
  auto *funcBody = new (*context)
      ESTree::BlockStatementNode(std::move(program->_body), false);
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
