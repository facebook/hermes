/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ScopedFunctionPromoter.h"

#include "SemanticResolver.h"
#include "hermes/AST/ESTree.h"
#include "hermes/AST/Keywords.h"
#include "hermes/Sema/SemContext.h"

using namespace hermes::ESTree;

namespace hermes {
namespace sema {

namespace {

/// Visitor class for promoting scoped function declarations.
class ScopedFunctionPromoter {
 public:
  explicit ScopedFunctionPromoter(SemanticResolver &resolver)
      : resolver_(resolver) {}

  /// Run the AST pass.
  void run(FunctionLikeNode *funcNode);

  /// Handle the default case for all nodes which we ignore, but we still want
  /// to visit their children.
  void visit(Node *node) {
    visitESTreeChildren(*this, node);
  }

  /// Do not descend into nested functions.
  void visit(FunctionLikeNode *) {}

  // All nodes with scopes.

  void visit(SwitchStatementNode *node) {
    visitScope(node);
  }
  void visit(BlockStatementNode *node) {
    visitScope(node);
  }
  void visit(ForStatementNode *node) {
    visitScope(node);
  }
  void visit(ForInStatementNode *node) {
    visitScope(node);
  }
  void visit(ForOfStatementNode *node) {
    visitScope(node);
  }
  void visit(WithStatementNode *node) {
    visitScope(node);
  }

  /// Needed by RecursiveVisitorDispatch. Optionally can protect against too
  /// deep nesting.
  bool incRecursionDepth(ESTree::Node *) {
    return true;
  }
  void decRecursionDepth() {}

 private:
  /// Visit any statement starting a scope.
  void visitScope(Node *node);

  /// Process the declarations in a scope.
  /// This is the core of the algorithm, it updates the binding tables, etc.
  void processDeclarations(Node *scope);

  /// Extract the list of declared identifiers in a declaration node into
  /// `idents`.
  /// \return the declaration kind of the node.
  /// Function declarations are always returned as `ScopedFunction`,
  /// so they can be distinguished.
  Decl::Kind extractDeclaredIdents(
      Node *node,
      llvh::SmallVectorImpl<IdentifierNode *> &idents);

 private:
  SemanticResolver &resolver_;

  /// The names of the scoped functions. We will ignore all other identifiers.
  llvh::SmallDenseSet<UniqueString *> funcNames_{};

  /// The scoped function declarations. We remove each from this set once
  /// we encounter it.
  llvh::SmallDenseSet<FunctionDeclarationNode *> funcDecls_{};

  using BindingTableTy = hermes::ScopedHashTable<UniqueString *, bool>;
  using BindingTableScopeTy =
      hermes::ScopedHashTableScope<UniqueString *, bool>;

  /// The currently lexically visible names.
  BindingTableTy bindingTable_{};
};

void ScopedFunctionPromoter::run(FunctionLikeNode *funcNode) {
  llvh::ArrayRef<Node *> decls =
      resolver_.functionContext()->decls->getScopedFuncDecls();

  // Populate the sets.
  for (Node *node : decls) {
    auto *funcDecl = cast<FunctionDeclarationNode>(node);
    funcNames_.insert(cast<IdentifierNode>(funcDecl->_id)->_name);
    funcDecls_.insert(funcDecl);
  }

  visitESTreeChildren(*this, funcNode);
}

void ScopedFunctionPromoter::visitScope(Node *node) {
  BindingTableScopeTy bindingScope{bindingTable_};
  processDeclarations(node);
}

void ScopedFunctionPromoter::processDeclarations(Node *scope) {
  auto *declsOpt =
      resolver_.functionContext()->decls->getScopeDeclsForNode(scope);
  if (!declsOpt) {
    return;
  }
  llvh::ArrayRef<Node *> decls = *declsOpt;

  llvh::SmallVector<IdentifierNode *, 4> idents{};
  // Whenever we encounter one of the scoped func decls we are trying to
  // promote, we store the address of its list entry here (so we can clear it
  // if we want to).
  llvh::SmallVector<Node *const *, 4> foundDecls{};

  for (auto &nodeRef : decls) {
    Node *node = nodeRef;
    if (!node)
      continue;

    if (auto *funcDecl = llvh::dyn_cast<FunctionDeclarationNode>(node)) {
      if (funcDecls_.count(funcDecl)) {
        // We encountered one of the candidate declarations.
        // Add it to the found_decls list and move on.
        foundDecls.push_back(&nodeRef);
      }
      continue;
    }

    // Extract idents, report errors.
    idents.clear();
    Decl::Kind declKind = extractDeclaredIdents(node, idents);

    // We are only interested in let-like declarations.
    if (!Decl::isKindLetLike(declKind))
      continue;

    // Remember only idents matching the set.
    for (IdentifierNode *idNode : idents) {
      if (funcNames_.count(idNode->_name)) {
        bindingTable_.insert(idNode->_name, true);
      }
    }
  }

  if (foundDecls.empty()) {
    // No work to do.
    return;
  }

  ScopeDecls newDecls{};

  // Did we finally encounter one of the scoped function declarations?
  for (Node *const *funcDeclRef : foundDecls) {
    auto *funcDecl = cast<FunctionDeclarationNode>(*funcDeclRef);
    // Remove it from the set, since we are no longer interested in it.
    funcDecls_.erase(funcDecl);

    if (funcDecl->_id) {
      // Is there a visible let-like declaration with the same name?
      // If so, it can't be promoted because it would shadow a `let`,
      // so keep it in `newDecls` and move on.
      if (bindingTable_.lookup(cast<IdentifierNode>(funcDecl->_id)->_name)) {
        newDecls.push_back(funcDecl);
        continue;
      }
    } else {
      // No name on the declaration, nothing to promote.
      newDecls.push_back(funcDecl);
      continue;
    }

    // This block-scoped function declaration can (and should) be promoted.
    // 1. Don't add it to the new_decls list.
    // 2. Add it to the function scope list.
    resolver_.functionContext()->decls->addScopeDeclForFunc(funcDecl);
  }

  resolver_.functionContext()->decls->setScopeDeclsForNode(
      scope, std::move(newDecls));
}

Decl::Kind ScopedFunctionPromoter::extractDeclaredIdents(
    Node *node,
    llvh::SmallVectorImpl<IdentifierNode *> &idents) {
  if (auto *varDeclaration = llvh::dyn_cast<VariableDeclarationNode>(node)) {
    for (Node &declarator : varDeclaration->_declarations) {
      resolver_.extractDeclaredIdentsFromID(
          cast<VariableDeclaratorNode>(declarator)._id, idents);
    }
    if (varDeclaration->_kind == resolver_.keywords().identLet) {
      return Decl::Kind::Let;
    } else if (varDeclaration->_kind == resolver_.keywords().identConst) {
      return Decl::Kind::Const;
    } else {
      assert(varDeclaration->_kind == resolver_.keywords().identVar);
      return Decl::Kind::Var;
    }
  }

  if (auto *fd = llvh::dyn_cast<FunctionDeclarationNode>(node)) {
    resolver_.extractDeclaredIdentsFromID(fd->_id, idents);
    return Decl::Kind::ScopedFunction;
  }

  if (auto *cd = llvh::dyn_cast<ClassDeclarationNode>(node)) {
    resolver_.extractDeclaredIdentsFromID(cd->_id, idents);
    return Decl::Kind::Class;
  }

  {
    auto *id = llvh::cast<ImportDeclarationNode>(node);
    for (Node &spec : id->_specifiers) {
      if (auto *is = llvh::dyn_cast<ImportSpecifierNode>(&spec)) {
        resolver_.extractDeclaredIdentsFromID(is->_local, idents);
      } else if (
          auto *ids = llvh::dyn_cast<ImportDefaultSpecifierNode>(&spec)) {
        resolver_.extractDeclaredIdentsFromID(ids->_local, idents);
      } else {
        auto *ins = cast<ImportNamespaceSpecifierNode>(&spec);
        resolver_.extractDeclaredIdentsFromID(ins->_local, idents);
      }
    }
    return Decl::Kind::Import;
  }
}

} // anonymous namespace

void promoteScopedFuncDecls(
    SemanticResolver &resolver,
    ESTree::FunctionLikeNode *funcNode) {
  if (resolver.functionContext()->decls->getScopedFuncDecls().empty()) {
    // No scoped function declarations, nothing to promote.
    return;
  }
  ScopedFunctionPromoter{resolver}.run(funcNode);
}

} // namespace sema
} // namespace hermes
