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

} // namespace ESTree
} // namespace hermes
