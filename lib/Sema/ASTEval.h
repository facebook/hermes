/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SEMA_ASTEVAL_H
#define HERMES_SEMA_ASTEVAL_H

#include "hermes/AST/ESTree.h"

namespace hermes::sema {
class Keywords;

/// Evaluate a binary expression if possible and fold it into a single literal
/// node.
/// \param kw keywords table
/// \param BE the expression to evaluate.
/// \param ppNode output pointer for the folded node.
/// \return true if the expression was folded.
bool astFoldBinaryExpression(
    const Keywords &kw,
    ESTree::BinaryExpressionNode *BE,
    ESTree::Node **ppNode);

/// Evaluate a unary expression if possible and fold it into a single literal
/// node.
/// \param kw keywords table
/// \param UE the expression to evaluate.
/// \param ppNode output pointer for the folded node.
/// \return true if the expression was folded.
bool astFoldUnaryExpression(
    const Keywords &kw,
    ESTree::UnaryExpressionNode *UE,
    ESTree::Node **ppNode);

} // namespace hermes::sema

#endif
