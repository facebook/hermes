/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_IRGEN_IR_INSTRUMENT_H
#define HERMES_IRGEN_IR_INSTRUMENT_H

#include "hermes/IR/IRBuilder.h"

namespace hermes {

/// A class to optionally instrument the generated IR. For each supported IR
/// expression, it supports a pre and post "hook". The pre-hook receives the
/// calculated operands and the AST node and returns an optional "cookie" value.
/// The post-hook receives the AST node, the result of the operation and the
/// cookie value. The post-hook returns the result value for convenience.
class IRInstrument {
 public:
#ifdef HERMES_ENABLE_IR_INSTRUMENTATION
  IRInstrument(Module *M, IRBuilder &builder);
  ~IRInstrument();

  /// Generate a unique source location identifier for a node.
  Value *getIID(ESTree::Node *node);

  /// Conveniently invoke a JS instrumentation hook by name.
  Value *invokeHook(llvh::StringRef name, llvh::ArrayRef<Value *> args);

  Value *undefinedIfNull(Value *v) {
    return v ? v : builder_.getLiteralUndefined();
  }

  /// Called before a binary expression is computed.
  /// \return an arbitrary value passed to the post-hook
  Value *preBinaryExpression(
      ESTree::BinaryExpressionNode *,
      Value *left,
      Value *right);

  /// Called after a binary expression has been computed.
  /// \return a Value* that overrides the result of the expression
  Value *postBinaryExpression(
      ESTree::BinaryExpressionNode *,
      Value *cookie,
      Value *result,
      Value *left,
      Value *right);

  /// Called before a unary expression is computed.
  /// \return an arbitrary value passed to the post-hook
  Value *preUnaryExpression(ESTree::UnaryExpressionNode *, Value *operand);

  /// Called after a unary expression has been computed.
  /// \return a Value* that overrides the result of the expression
  Value *postUnaryExpression(
      ESTree::UnaryExpressionNode *,
      Value *cookie,
      Value *result,
      Value *operand);

  /// Called before an assignment expression is computed.
  /// \param left the LHS operand for += etc, or nullptr for =
  /// \param right the RHS of the += or = assignment
  /// \return an arbitrary value passed to the post-hook
  Value *
  preAssignment(ESTree::AssignmentExpressionNode *, Value *left, Value *right);

  /// Called after an assignment expression has been computed.
  /// \return the Value* to assign
  Value *postAssignment(
      ESTree::AssignmentExpressionNode *,
      Value *cookie,
      Value *result,
      Value *left,
      Value *right);

 private:
  IRBuilder &builder_;
  /// Name of the global to find hooks on ("__instrument").
  LiteralString *globalName_;
  /// The module we're instrumenting.
  Module *M_;
  // Whether we should instrument.
  bool enabled_;

#else
  // IR Instrumentation is compiled out, so all these members are just no-ops.
  IRInstrument(Module *M, IRBuilder &builder) {}
  ~IRInstrument() = default;

  inline Value *preBinaryExpression(
      ESTree::BinaryExpressionNode *,
      Value *left,
      Value *right) {
    return nullptr;
  }
  inline Value *postBinaryExpression(
      ESTree::BinaryExpressionNode *,
      Value *cookie,
      Value *result,
      Value *left,
      Value *right) {
    return result;
  }

  inline Value *preUnaryExpression(
      ESTree::UnaryExpressionNode *,
      Value *operand) {
    return nullptr;
  }
  inline Value *postUnaryExpression(
      ESTree::UnaryExpressionNode *,
      Value *cookie,
      Value *result,
      Value *operand) {
    return result;
  }

  inline Value *preAssignment(
      ESTree::AssignmentExpressionNode *node,
      Value *left,
      Value *right) {
    return nullptr;
  }
  inline Value *postAssignment(
      ESTree::AssignmentExpressionNode *node,
      Value *cookie,
      Value *result,
      Value *left,
      Value *right) {
    return result;
  }
#endif
};

} // namespace hermes

#endif // HERMES_IRGEN_IR_INSTRUMENT_H
