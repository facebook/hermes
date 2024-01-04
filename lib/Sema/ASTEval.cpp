/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ASTEval.h"

#include "hermes/Sema/Keywords.h"
#include "hermes/Support/Conversions.h"

namespace hermes::sema {

bool astFoldBinaryExpression(
    const Keywords &kw,
    ESTree::BinaryExpressionNode *BE,
    ESTree::Node **ppNode) {
  // For now only fold numeric constants.
  auto *leftNum = llvh::dyn_cast<ESTree::NumericLiteralNode>(BE->_left);
  auto *rightNum = llvh::dyn_cast<ESTree::NumericLiteralNode>(BE->_right);
  if (!leftNum || !rightNum)
    return false;
  double left = leftNum->_value;
  double right = rightNum->_value;

  // Check for common operators.
  double res;
  if (BE->_operator == kw.identPlus) {
    res = left + right;
  } else if (BE->_operator == kw.identMinus) {
    res = left - right;
  } else if (BE->_operator == kw.identStar) {
    res = left * right;
  } else if (BE->_operator == kw.identSlash) {
    res = left / right;
  } else if (BE->_operator == kw.identPercent) {
    res = std::fmod(left, right);
  } else if (BE->_operator == kw.identAmp) {
    res = hermes::truncateToInt32(left) & hermes::truncateToInt32(right);
  } else if (BE->_operator == kw.identCaret) {
    res = hermes::truncateToInt32(left) ^ hermes::truncateToInt32(right);
  } else if (BE->_operator == kw.identPipe) {
    res = hermes::truncateToInt32(left) | hermes::truncateToInt32(right);
  } else if (BE->_operator == kw.identLessLess) {
    res =
        (int32_t)(hermes::truncateToUInt32(left) << (hermes::truncateToUInt32(right) & 0x1f));
  } else if (BE->_operator == kw.identGreaterGreater) {
    res = hermes::truncateToInt32(left) >>
        (hermes::truncateToUInt32(right) & 0x1f);
  } else if (BE->_operator == kw.identGreaterGreaterGreater) {
    res = hermes::truncateToUInt32(left) >>
        (hermes::truncateToUInt32(right) & 0x1f);
  } else {
    return false;
  }

  // Reuse the left node.
  BE->_left = BE->_right = nullptr;
  leftNum->_value = res;
  leftNum->setSourceRange(BE->getSourceRange());
  *ppNode = leftNum;
  return true;
}

bool astFoldUnaryExpression(
    const Keywords &kw,
    ESTree::UnaryExpressionNode *UE,
    ESTree::Node **ppNode) {
  // For now only fold numeric constants.
  auto *num = llvh::dyn_cast<ESTree::NumericLiteralNode>(UE->_argument);
  if (!num)
    return false;
  double val = num->_value;

  // Check for common operators.
  double res;
  if (UE->_operator == kw.identPlus)
    res = val;
  else if (UE->_operator == kw.identMinus)
    res = -val;
  else if (UE->_operator == kw.identTilde)
    res = ~hermes::truncateToInt32(val);
  else
    return false;

  // Reuse the argument node.
  UE->_argument = nullptr;
  num->_value = res;
  num->setSourceRange(UE->getSourceRange());
  *ppNode = num;
  return true;
}

} // namespace hermes::sema
