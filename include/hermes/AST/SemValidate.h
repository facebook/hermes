/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_AST_SEMVALIDATE_H
#define HERMES_AST_SEMVALIDATE_H

#include "hermes/AST/Context.h"
#include "hermes/AST/ESTree.h"
#include "hermes/Support/SourceErrorManager.h"

#include <deque>

namespace hermes {
namespace sem {

/// Semantic information for a function declaration, expression, method, etc.
class FunctionInfo {
 public:
  /// The list of hoisted variable declarations.
  llvm::SmallVector<ESTree::IdentifierNode *, 4> varDecls{};

  /// A list of functions that need to be hoisted and materialized before we
  /// can generate the rest of the function.
  llvm::SmallVector<ESTree::FunctionDeclarationNode *, 2> closures{};

  /// Whether this function references the "arguments" identifier. This is a
  /// conservative approximation of whether it tries to access the "arguments"
  /// object. Why "conservative"? Because in non-strict mode it is possible to
  /// declare a variable called "arguments" and then access it.
  /// In the future, when we start resolving variables in the validator, we will
  /// be able to be completely accurate, but for now this is good enough.
  bool usesArguments = false;

  /// Whether this function contains arrow functions.
  bool containsArrowFunctions = false;

  /// This is a logical or of the \c usesArguments flags of all contained
  /// arrow functions. This will be used as a conservative estimate of
  /// whether a non-arrow function needs to eagerly create and capture its
  /// Arguments object.
  bool containsArrowFunctionsUsingArguments = false;

  /// Class that holds the target for a break/continue label, as well as the
  /// depth of the nested try/catch/finally blocks where this label is defined.
  class GotoLabel {
   public:
    /// The closest surrounding try statement.
    ESTree::TryStatementNode *const surroundingTry;

    explicit GotoLabel(ESTree::TryStatementNode *aSurroundingTry)
        : surroundingTry(aSurroundingTry) {}
  };

  /// An array of labels defined in the function. The AST nodes (LoopStatement
  /// and LabeledStatement) defining each label are decorated with
  /// LabelDecorationBase refering to the label index in this array.
  /// Break/continue AST nodes contain the index of the target label.
  llvm::SmallVector<GotoLabel, 2> labels{};

  /// Allocate a new label and return its index.
  unsigned allocateLabel(ESTree::TryStatementNode *surroundingTry) {
    labels.emplace_back(surroundingTry);
    return (unsigned)(labels.size() - 1);
  }
};

/// Identifier and label tables, populated by the semantic validator. They need
/// to be stored separately from the AST because they have destructors, while
/// the AST is stored in a pool.
class SemContext {
 public:
  /// Create a new instance of \c FunctionInfo.
  FunctionInfo *createFunction() {
    functions_.emplace_back();
    return &functions_.back();
  }

 private:
  std::deque<FunctionInfo> functions_{};
};

/// Perform semantic validation of the entire AST, starting from the specified
/// root, which should be ProgramNode.
bool validateAST(Context &astContext, SemContext &semCtx, ESTree::NodePtr root);

/// Perform semantic validation of an individual function in the given context
/// \param function must be a function node
/// \param strict specifies parent strictness.
bool validateFunctionAST(
    Context &astContext,
    SemContext &semCtx,
    ESTree::NodePtr function,
    bool strict);

} // namespace sem
} // namespace hermes

#endif // HERMES_AST_SEMVALIDATE_H
