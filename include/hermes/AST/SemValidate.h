/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
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
  struct VarDecl {
    enum class Kind { Var, Let, Const };

    Kind kind;
    ESTree::IdentifierNode *identifier;
  };

  /// Parameter names.
  llvh::SmallVector<VarDecl, 4> paramNames{};

  /// The list of hoisted variable declarations.
  llvh::SmallVector<VarDecl, 4> varDecls{};

  /// A list of functions that need to be hoisted and materialized before we
  /// can generate the rest of the function.
  llvh::SmallVector<ESTree::FunctionDeclarationNode *, 2> closures{};

  /// A list of imports that need to be hoisted and materialized before we
  /// can generate the rest of the function.
  /// Any line of the file may use the imported values.
  llvh::SmallVector<ESTree::ImportDeclarationNode *, 2> imports{};

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

  /// Number of labels allocated so far. We use this counter to assign
  /// consecutive index values to labels.
  unsigned labelCount = 0;

  /// Allocate a new label and return its index.
  unsigned allocateLabel() {
    return labelCount++;
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

/// Perform semantic validation of the entire AST, without preparing the AST for
/// compilation. This will not error on features we can parse but not compile,
/// transform the AST, or perform compilation specific validation.
bool validateASTForParser(
    Context &astContext,
    SemContext &semCtx,
    ESTree::NodePtr root);

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
