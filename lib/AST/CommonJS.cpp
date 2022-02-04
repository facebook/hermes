/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/CommonJS.h"

namespace hermes {

ESTree::FunctionExpressionNode *wrapCJSModule(
    std::shared_ptr<Context> &context,
    ESTree::ProgramNode *program) {
  auto *moduleBlock =
      new (*context) ESTree::BlockStatementNode(std::move(program->_body));
  moduleBlock->setSourceRange(program->getSourceRange());
  moduleBlock->setDebugLoc(program->getDebugLoc());

  ESTree::NodeList argNames{};

  // Identifiers for function arguments.
  auto *exports = new (*context) ESTree::IdentifierNode(
      context->getIdentifier("exports").getUnderlyingPointer(), nullptr, false);
  auto *require = new (*context) ESTree::IdentifierNode(
      context->getIdentifier("require").getUnderlyingPointer(), nullptr, false);
  auto *module = new (*context) ESTree::IdentifierNode(
      context->getIdentifier("module").getUnderlyingPointer(), nullptr, false);
  argNames.push_back(*exports);
  argNames.push_back(*require);
  argNames.push_back(*module);

  auto *wrappedFn = new (*context) ESTree::FunctionExpressionNode(
      nullptr,
      std::move(argNames),
      moduleBlock,
      nullptr,
      nullptr,
      nullptr,
      false,
      false);
  wrappedFn->strictness = ESTree::Strictness::NonStrictMode;
  wrappedFn->setSourceRange(program->getSourceRange());
  wrappedFn->setDebugLoc(program->getDebugLoc());

  return wrappedFn;
}

} // namespace hermes
