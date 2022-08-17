/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_IR_IREVAL_H
#define HERMES_IR_IREVAL_H

#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"

namespace hermes {
class IRBuilder;

/// Tries to evaluate a unary operator using JS semantics.
/// \returns a literal which represents the result of evaluating the unary
/// operator or nullptr if the operator cannot be evaluated.
Literal *evalUnaryOperator(
    UnaryOperatorInst::OpKind kind,
    IRBuilder &builder,
    Literal *operand);

/// Tries to evaluate a binary operator using JS semantics.
/// \returns a literal which represents the result of evaluating the binary
/// operator or nullptr if the operator cannot be evaluated.
Literal *evalBinaryOperator(
    BinaryOperatorInst::OpKind kind,
    IRBuilder &builder,
    Literal *lhs,
    Literal *rhs);

/// Converts a literal to a boolean literal.
/// \returns a bool if the operand can be converted to one, nullptr otherwise.
LiteralBool *evalToBoolean(IRBuilder &builder, Literal *operand);

/// Converts a literal to a string literal.
/// Returns a string if the operand can be converted to one, nullptr otherwise.
LiteralString *evalToString(IRBuilder &builder, Literal *operand);

/// Converts a literal to a number literal.
/// Returns a number if the operand can be converted to one, nullptr otherwise.
LiteralNumber *evalToNumber(IRBuilder &builder, Literal *operand);

/// Converts a literal to a 32-bit signed integer literal.
/// Returns a number if the operand can be converted to one, nullptr otherwise.
LiteralNumber *evalToInt32(IRBuilder &builder, Literal *operand);

/// Converts a value to a boolean literal.
/// Returns a boolean if the operand can be converted to one, nullptr otherwise.
LiteralBool *evalToBoolean(IRBuilder &builder, Value *operand);

/// Checks if the operand evaluates to boolean true.
/// \returns true if ToBoolean(operand) is true.
bool evalIsTrue(IRBuilder &builder, Literal *operand);

/// Checks if the operand evaluates to boolean false.
/// \return true if ToBoolean(operand) is false.
bool evalIsFalse(IRBuilder &builder, Literal *operand);

} // namespace hermes

#endif
