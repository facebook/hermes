/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "DeclCollector.h"

#define DEBUG_TYPE "DeclCollector"

using namespace hermes::ESTree;

namespace hermes {
namespace sema {

/* static */ std::unique_ptr<DeclCollector> DeclCollector::run(
    ESTree::FunctionLikeNode *root,
    const sema::Keywords &kw,
    unsigned recursionDepth,
    const std::function<void(ESTree::Node *)> &recursionDepthExceeded) {
  std::unique_ptr<DeclCollector> dc(
      new DeclCollector(root, kw, recursionDepth, recursionDepthExceeded));
  dc->runImpl();
  return dc;
}

std::unique_ptr<DeclCollector> DeclCollector::clone(
    const DeclCollector &original,
    const llvh::DenseMap<ESTree::Node *, ESTree::Node *> &clonedNodes) {
  std::unique_ptr<DeclCollector> dc(new DeclCollector(
      clonedNodes.lookup(original.root_), original.kw_, 0, [](ESTree::Node *) {
        // Recursion depth won't be exceeded.
        llvm_unreachable("DeclCollector clone is non-recursive");
      }));

  // Clone scopes_.
  for (const auto &[node, decls] : original.scopes_) {
    LLVM_DEBUG(
        llvh::dbgs() << "Cloning DeclCollector for: " << node->getNodeName()
                     << '\n');
    auto *newNode = clonedNodes.lookup(node);
    assert(newNode && "Missing cloned node");
    // Create the new declaration list in the new DeclCollector.
    ScopeDecls &newDecls = dc->scopes_[newNode];
    for (auto *decl : decls) {
      LLVM_DEBUG(
          llvh::dbgs() << "Cloning decl: " << node->getNodeName() << "@" << decl
                       << '\n');
      auto *newDecl = clonedNodes.lookup(decl);
      assert(newDecl && "Missing cloned node");
      newDecls.push_back(newDecl);
    }
  }

  // Clone scopedFuncDecls_.
  for (auto *decl : original.scopedFuncDecls_) {
    dc->scopedFuncDecls_.push_back(clonedNodes.lookup(decl));
  }

  return dc;
}

void DeclCollector::runImpl() {
  if (auto *func = llvh::dyn_cast<FunctionDeclarationNode>(root_)) {
    newScope();
    // Visit the children of the body, since we don't want to associate a
    // scope with it.
    visitESTreeChildren(*this, llvh::cast<BlockStatementNode>(func->_body));
    closeScope(root_);
    return;
  }
  if (auto *func = llvh::dyn_cast<FunctionExpressionNode>(root_)) {
    newScope();
    // Visit the children of the body, since we don't want to associate a
    // scope with it.
    visitESTreeChildren(*this, llvh::cast<BlockStatementNode>(func->_body));
    closeScope(root_);
    return;
  }

  if (auto *func = llvh::dyn_cast<ArrowFunctionExpressionNode>(root_)) {
    newScope();
    // If there is a BlockStatement, don't visit it, just visit its children.
    if (auto *block = llvh::dyn_cast<BlockStatementNode>(func->_body)) {
      visitESTreeChildren(*this, block);
    } else {
      visitESTreeChildren(*this, root_);
    }
    closeScope(root_);
    return;
  }

  newScope();
  visitESTreeChildren(*this, root_);
  closeScope(root_);
}

void DeclCollector::dump(llvh::raw_ostream &os, unsigned indent) const {
#ifndef NDEBUG
  for (const auto &p : scopes_) {
    os << llvh::left_justify("", indent) << p.first->getNodeName() << "["
       << llvh::format("%p", p.first) << "]:";
    for (ESTree::Node *n : p.second) {
      os << " " << n->getNodeName() << "[" << llvh::format("%p", n) << "]";
    }
    os << "\n";
  }
#endif
}

void DeclCollector::visit(ESTree::VariableDeclarationNode *node) {
  if (node->_kind == kw_.identVar) {
    addToFunc(node);
  } else {
    addToCur(node);
  }
  visitESTreeChildren(*this, node);
}
void DeclCollector::visit(ESTree::ClassDeclarationNode *node) {
  addToCur(node);
  // Don't descend into class bodies.
}
void DeclCollector::visit(ESTree::ImportDeclarationNode *node) {
  addToCur(node);
  visitESTreeChildren(*this, node);
}
#if HERMES_PARSE_FLOW
void DeclCollector::visit(ESTree::TypeAliasNode *node) {
  addToCur(node);
  visitESTreeChildren(*this, node);
}
#endif
#if HERMES_PARSE_TS
void DeclCollector::visit(ESTree::TSTypeAliasDeclarationNode *node) {
  addToCur(node);
  visitESTreeChildren(*this, node);
}
#endif

void DeclCollector::visit(ESTree::FunctionDeclarationNode *node) {
  // Record but don't descend.
  addToCur(node);
  if (scopeStack_.size() > 1) {
    scopedFuncDecls_.push_back(node);
  }
}

void DeclCollector::visit(ESTree::BlockStatementNode *node) {
  newScope();
  visitESTreeChildren(*this, node);
  closeScope(node);
}
void DeclCollector::visit(ESTree::ForStatementNode *node) {
  newScope();
  visitESTreeChildren(*this, node);
  closeScope(node);
}
void DeclCollector::visit(ESTree::ForInStatementNode *node) {
  newScope();
  visitESTreeChildren(*this, node);
  closeScope(node);
}
void DeclCollector::visit(ESTree::ForOfStatementNode *node) {
  newScope();
  visitESTreeChildren(*this, node);
  closeScope(node);
}
void DeclCollector::visit(ESTree::CatchClauseNode *node) {
  newScope();
  if (node->_param) {
    addToCur(node);
    visitESTreeNode(*this, node->_param, node);
  }
  // CatchClauseNode is supposed to have separate scopes for the body and the
  // parameters.
  visitESTreeNode(*this, node->_body, node);
  closeScope(node);
}
void DeclCollector::visit(ESTree::SwitchStatementNode *node) {
  newScope();
  visitESTreeChildren(*this, node);
  closeScope(node);
}

void DeclCollector::closeScope(ESTree::Node *node) {
  assert(!scopeStack_.empty() && "no scope to close");

  // Move the popped scope out so we can put it in the `scopes_` map.
  ScopeDecls decls = std::move(scopeStack_.back());
  scopeStack_.pop_back();

  // Only store to the map if there's something to store.
  if (!decls.empty()) {
    auto result = scopes_.try_emplace(node, decls);
    (void)result;
    assert(result.second && "Tried to collect same node twice");
  }
}

} // namespace sema
} // namespace hermes
