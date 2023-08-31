/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SEMA_DECLCOLLECTOR_H
#define HERMES_SEMA_DECLCOLLECTOR_H

#include "hermes/AST/RecursiveVisitor.h"
#include "hermes/Sema/Keywords.h"
#include "hermes/Sema/SemContext.h"

namespace hermes {
namespace sema {

class DeclCollector;
class SemanticResolver;

/// All the declarations in a scope.
using ScopeDecls = std::vector<ESTree::Node *>;

/// A map storing DeclCollector instances associated with every function.
using DeclCollectorMapTy =
    llvh::DenseMap<ESTree::FunctionLikeNode *, std::unique_ptr<DeclCollector>>;

/// Collect all declarations in every scope of a function.
/// All declarations will have to be hoisted to the top of a function or scope,
/// so store them all up front in a single pass.
/// Do not descend into nested functions.
class DeclCollector
    : public ESTree::NestedRecursionDepthTracker<DeclCollector> {
  using Base = ESTree::NestedRecursionDepthTracker<DeclCollector>;
  friend Base;

 public:
  /// \param recursionDepth remaining recursion depth
  /// \param recursionDepthExceeded handler to invoke when we transition fron
  ///     non-zero to zero remaining recursion depth.
  /// \return a DeclCollector which has collected all declarations in \p root.
  static std::unique_ptr<DeclCollector> run(
      ESTree::FunctionLikeNode *root,
      const sema::Keywords &kw,
      unsigned recursionDepth,
      const std::function<void(ESTree::Node *)> &recursionDepthExceeded);

  void dump(llvh::raw_ostream &os, unsigned indent = 0);

  /// \param node the AST node which could have created a scope.
  ///   The only nodes which can are decorated by `ScopeDecorationBase`.
  /// \return the ScopeDecls if the AST node did create a scope,
  ///   Nullptr if it didn't.
  const ScopeDecls *getScopeDeclsForNode(ESTree::Node *node) const {
    auto it = scopes_.find(node);
    if (it == scopes_.end())
      return nullptr;
    return &it->second;
  }
  /// Set the ScopeDecls for an AST node.
  /// Replaces ScopeDecls if it already exists.
  void setScopeDeclsForNode(ESTree::Node *node, ScopeDecls decls);
  /// Add a single node to the list of declarations in the root AST node of the
  /// function.
  /// Used for promoting a function declaration out of block scope,
  /// because that requires moving the declaration into the function's root
  /// scope.
  void addScopeDeclForFunc(ESTree::Node *node);

  /// Return a list of all scoped function declarations in the function.
  llvh::ArrayRef<ESTree::Node *> getScopedFuncDecls() const {
    return scopedFuncDecls_;
  }

  void visit(ESTree::Node *node) {
    visitESTreeChildren(*this, node);
  }

  void visit(ESTree::VariableDeclarationNode *node);
  void visit(ESTree::ClassDeclarationNode *node);
  void visit(ESTree::ImportDeclarationNode *node);
#if HERMES_PARSE_FLOW
  void visit(ESTree::TypeAliasNode *node);
#endif
#if HERMES_PARSE_TS
  void visit(ESTree::TSTypeAliasDeclarationNode *node);
#endif

  void visit(ESTree::FunctionDeclarationNode *node);

  /// Don't descend.
  void visit(ESTree::FunctionExpressionNode *node) {}
  /// Don't descend.
  void visit(ESTree::ArrowFunctionExpressionNode *node) {}

  void visit(ESTree::BlockStatementNode *node);
  void visit(ESTree::ForStatementNode *node);
  void visit(ESTree::ForInStatementNode *node);
  void visit(ESTree::ForOfStatementNode *node);
  void visit(ESTree::SwitchStatementNode *node);

  void visit(ESTree::CatchClauseNode *node);

  /// Don't descend, to avoid recursion overflow.
  void visit(ESTree::BinaryExpressionNode *) {}
  /// Don't descend, to avoid recursion overflow.
  void visit(ESTree::AssignmentExpressionNode *) {}

 private:
  explicit DeclCollector(
      ESTree::Node *root,
      const sema::Keywords &kw,
      unsigned recursionDepth,
      const std::function<void(ESTree::Node *)> &recursionDepthExceeded)
      : Base(recursionDepth, recursionDepthExceeded), root_(root), kw_(kw) {}

  /// Actually run the root node.
  void runImpl();

  /// Add a declaration to the function itself.
  void addToFunc(ESTree::Node *node) {
    assert(!scopeStack_.empty() && "missing function scope");
    scopeStack_.front().push_back(node);
  }
  /// Add a declaration to the current lexical scope.
  void addToCur(ESTree::Node *node) {
    assert(!scopeStack_.empty() && "no current scope");
    scopeStack_.back().push_back(node);
  }

  /// Push a scope onto the stack.
  void newScope() {
    scopeStack_.emplace_back();
  }
  /// Pop a scope from the stack.
  /// If it contains declarations, add them to \c scopes_.
  void closeScope(ESTree::Node *node);

  /// Root node for the collection.
  ESTree::Node *root_;

  const sema::Keywords &kw_;

  /// Associate a ScopeDecls structure with the node that creates the scope.
  llvh::DenseMap<ESTree::Node *, ScopeDecls> scopes_{};

  /// Function declarations in a block scope.
  /// They have special rules described in Annex B 3.3.
  std::vector<ESTree::Node *> scopedFuncDecls_{};

  /// Stack of active scopes.
  /// Once a scope is closed, it is attached to an AST node.
  llvh::SmallVector<ScopeDecls, 4> scopeStack_{};
};

} // namespace sema
} // namespace hermes

#endif
