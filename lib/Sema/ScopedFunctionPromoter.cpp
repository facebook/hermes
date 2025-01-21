/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ScopedFunctionPromoter.h"

#include "SemanticResolver.h"
#include "hermes/ADT/ScopedHashTable.h"
#include "hermes/AST/ESTree.h"
#include "hermes/Sema/Keywords.h"
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

  /// \return the list of promoted function declarations.
  std::vector<FunctionDeclarationNode *> acquirePromotedFuncDecls() {
    return std::move(promotedFuncDecls_);
  }

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
  void visit(CatchClauseNode *node) {
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

  /// Add the formal parameters of \p func to the binding table if they have
  /// names we care about, because they must also prevent function promotion.
  /// ES2022 B.3.2.1 29.a.ii
  /// Needed to check "parameterNames does not contain F".
  void processParameters(FunctionLikeNode *funcNode);

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

  /// The result list of promoted function declarations.
  std::vector<ESTree::FunctionDeclarationNode *> promotedFuncDecls_{};

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
  BindingTableScopeTy bindingScope{bindingTable_};
  llvh::ArrayRef<Node *> decls =
      resolver_.functionContext()->decls->getScopedFuncDecls();

  // Populate the sets.
  for (Node *node : decls) {
    auto *funcDecl = cast<FunctionDeclarationNode>(node);
    funcNames_.insert(cast<IdentifierNode>(funcDecl->_id)->_name);
    funcDecls_.insert(funcDecl);
  }

  processParameters(funcNode);
  processDeclarations(funcNode);
  if (auto *programNode = llvh::dyn_cast<ProgramNode>(funcNode)) {
    visitESTreeChildren(*this, programNode);
  } else {
    visitESTreeChildren(*this, getBlockStatement(funcNode));
  }
}

void ScopedFunctionPromoter::visitScope(Node *node) {
  BindingTableScopeTy bindingScope{bindingTable_};
  processDeclarations(node);
  visitESTreeChildren(*this, node);
}

void ScopedFunctionPromoter::processParameters(FunctionLikeNode *funcNode) {
  for (Decl *decl : funcNode->getSemInfo()->getParameterScope()->decls) {
    if (decl->kind == Decl::Kind::Parameter) {
      UniqueString *name = decl->name.getUnderlyingPointer();
      if (funcNames_.count(name)) {
        // Found a parameter with a name we care about, add it to the binding
        // table.
        bindingTable_.try_emplace(name, true);
      }
    }
  }
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

  // New decls with the promoted functions removed.
  // Populated with non-candidate declarations in the first loop,
  // and non-promoted candidate declarations in the second loop.
  ScopeDecls newDecls{};

  for (auto &nodeRef : decls) {
    Node *node = nodeRef;
    if (!node)
      continue;

    if (auto *funcDecl = llvh::dyn_cast<FunctionDeclarationNode>(node)) {
      if (funcDecls_.count(funcDecl)) {
        // We encountered one of the candidate declarations.
        // Add it to the found_decls list and move on.
        foundDecls.push_back(&nodeRef);
      } else {
        newDecls.push_back(node);
      }
      continue;
    }

    newDecls.push_back(node);

    // Extract idents, report errors.
    idents.clear();
    Decl::Kind declKind = extractDeclaredIdents(node, idents);

    // We are only interested in let-like declarations, but not ES5Catch.
    // ES5Catch doesn't conflict with Var declarations.
    // See ES14.0 B.3.4.
    if (!Decl::isKindLetLike(declKind) || declKind == Decl::Kind::ES5Catch)
      continue;

    // Remember only idents matching the set.
    for (IdentifierNode *idNode : idents) {
      if (funcNames_.count(idNode->_name)) {
        bindingTable_.try_emplace(idNode->_name, true);
      }
    }
  }

  if (foundDecls.empty()) {
    // No work to do.
    return;
  }

  // Did we finally encounter one of the scoped function declarations?
  for (Node *const *funcDeclRef : foundDecls) {
    auto *funcDecl = cast<FunctionDeclarationNode>(*funcDeclRef);
    // Remove it from the set, since we are no longer interested in it.
    funcDecls_.erase(funcDecl);

    if (funcDecl->_id &&
        !bindingTable_.lookup(cast<IdentifierNode>(funcDecl->_id)->_name)) {
      // There's no visible let-like declaration with the same name.
      // So this decl can be promoted because it would not shadow a `let`.
      // Add it to the function scope list.
      promotedFuncDecls_.push_back(funcDecl);
    }
  }
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

  if (auto *catchClause = llvh::dyn_cast<CatchClauseNode>(node)) {
    resolver_.extractDeclaredIdentsFromID(catchClause->_param, idents);
    if (auto *id =
            llvh::dyn_cast_or_null<IdentifierNode>(catchClause->_param)) {
      return Decl::Kind::ES5Catch;
    } else {
      return Decl::Kind::Catch;
    }
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

std::vector<ESTree::FunctionDeclarationNode *> getPromotedScopedFuncDecls(
    SemanticResolver &resolver,
    ESTree::FunctionLikeNode *funcNode) {
  if (resolver.functionContext()->decls->getScopedFuncDecls().empty()) {
    // No scoped function declarations, nothing to promote.
    return {};
  }
  ScopedFunctionPromoter impl{resolver};
  impl.run(funcNode);
  return impl.acquirePromotedFuncDecls();
}

} // namespace sema
} // namespace hermes
