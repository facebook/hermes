/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/TransformDecorate.h"

#include "hermes/AST/RecursiveVisitor.h"

namespace hermes {

namespace {

/// Visitor that rewrites top-of-statement-list `Hermes.decorate(name, dec);`
/// calls into entries on the preceding FunctionDeclaration's
/// FunctionLikeDecoration::decorations list, and removes the call from the
/// statement list. Any `Hermes.decorate(...)` call that is not in that exact
/// position is reported as a compile error.
class DecorateRewriter
    : public ESTree::RecursionDepthTracker<DecorateRewriter> {
  Context &astContext_;
  const Keywords &kw_;
  bool error_{false};

 public:
  explicit DecorateRewriter(Context &astContext)
      : astContext_(astContext), kw_(astContext.keywords()) {}

  bool run(ESTree::Node *root) {
    ESTree::visitESTreeNodeNoReplace(*this, root);
    return !error_;
  }

  void recursionDepthExceeded(ESTree::Node *node) {
    error(node, "too many nested expressions/statements/declarations");
  }

  void visit(ESTree::ProgramNode *node) {
    processStatementList(node->_body);
    ESTree::visitESTreeChildren(*this, node);
  }

  void visit(ESTree::BlockStatementNode *node) {
    processStatementList(node->_body);
    ESTree::visitESTreeChildren(*this, node);
  }

  void visit(ESTree::StaticBlockNode *node) {
    processStatementList(node->_body);
    ESTree::visitESTreeChildren(*this, node);
  }

  void visit(ESTree::SwitchCaseNode *node) {
    processStatementList(node->_consequent);
    ESTree::visitESTreeChildren(*this, node);
  }

  void visit(ESTree::CallExpressionNode *call) {
    // Any Hermes.decorate call we reach here was not consumed by
    // processStatementList, so it is in an invalid position.
    if (isHermesDecorateCall(call)) {
      error(
          call,
          "Hermes.decorate must appear as a statement immediately following "
          "a FunctionDeclaration of the same name");
    }
    ESTree::visitESTreeChildren(*this, call);
  }

  void visit(ESTree::Node *node) {
    ESTree::visitESTreeChildren(*this, node);
  }

 private:
  void error(ESTree::Node *node, llvh::Twine msg) {
    astContext_.getSourceErrorManager().error(node->getSourceRange(), msg);
    error_ = true;
  }

  /// \return true if \p call is of the form `Hermes.decorate(...)`.
  bool isHermesDecorateCall(ESTree::CallExpressionNode *call) const {
    auto *member = llvh::dyn_cast<ESTree::MemberExpressionNode>(call->_callee);
    if (!member || member->_computed)
      return false;
    auto *obj = llvh::dyn_cast<ESTree::IdentifierNode>(member->_object);
    auto *prop = llvh::dyn_cast<ESTree::IdentifierNode>(member->_property);
    return obj && prop && obj->_name == kw_.identHermes &&
        prop->_name == kw_.identDecorate;
  }

  /// Walk \p body looking for `Hermes.decorate(name, Hermes.dec);` statements.
  /// When one is found and the immediately preceding statement is a matching
  /// FunctionDeclaration, attach a DecoratorNode wrapping the decoration
  /// expression to that function's FunctionLikeDecoration::decorations and
  /// remove the call statement. Misuse (wrong shape, wrong position, name
  /// mismatch) is reported as a compile error and the offending statement is
  /// removed to keep the AST coherent for downstream passes.
  void processStatementList(ESTree::NodeList &body) {
    for (auto it = body.begin(); it != body.end();) {
      auto *stmt = llvh::dyn_cast<ESTree::ExpressionStatementNode>(&*it);
      auto *call = stmt
          ? llvh::dyn_cast<ESTree::CallExpressionNode>(stmt->_expression)
          : nullptr;
      // Validate the call shape and position; on any error, drop the
      // statement so the default visitor doesn't re-report it, and continue
      // processing the rest of the list so later valid decorations can still
      // be attached.
      if (call && isHermesDecorateCall(call)) {
        processDecorateCall(body, it, call);
        it = body.erase(it);
      } else {
        ++it;
      }
    }
  }

  /// Validate \p call (which must be a `Hermes.decorate(...)` call appearing
  /// as an ExpressionStatement at \p it in \p body) and, on success, attach
  /// the decoration to the preceding FunctionDeclaration.
  /// On failure, report error.
  /// \return false on error.
  bool processDecorateCall(
      ESTree::NodeList &body,
      ESTree::NodeList::iterator it,
      ESTree::CallExpressionNode *call) {
    if (it == body.begin()) {
      error(
          call,
          "Hermes.decorate must immediately follow a FunctionDeclaration");
      return false;
    }
    auto *funcDecl =
        llvh::dyn_cast<ESTree::FunctionDeclarationNode>(&*std::prev(it));
    if (!funcDecl) {
      error(
          call,
          "Hermes.decorate must immediately follow a FunctionDeclaration");
      return false;
    }

    if (call->_arguments.size() != 2) {
      error(call, "Hermes.decorate must be called with exactly 2 arguments");
      return false;
    }

    auto argIt = call->_arguments.begin();
    auto *firstArg = &*argIt++;
    auto *secondArg = &*argIt;

    auto *targetId = llvh::dyn_cast<ESTree::IdentifierNode>(firstArg);
    if (!targetId) {
      error(
          firstArg,
          "Hermes.decorate first argument must be a function identifier");
      return false;
    }

    auto *funcId =
        llvh::dyn_cast_or_null<ESTree::IdentifierNode>(funcDecl->_id);
    if (!funcId || funcId->_name != targetId->_name) {
      error(
          targetId,
          "Hermes.decorate target name does not match the preceding "
          "FunctionDeclaration");
      return false;
    }

    // All checks passed: detach the decoration expression from the call
    // (the intrusive list only allows a node to live in one list at a
    // time), wrap it in a DecoratorNode, and append it to the function's
    // decorations.
    call->_arguments.remove(*secondArg);
    auto *decorator = new (astContext_) ESTree::DecoratorNode(secondArg);
    decorator->copyLocationFrom(secondArg);
    auto *funcDeco =
        ESTree::getDecoration<ESTree::FunctionLikeDecoration>(funcDecl);
    assert(funcDeco && "FunctionDeclaration missing FunctionLikeDecoration");
    funcDeco->decorations.push_back(*decorator);
    return true;
  }
};

} // anonymous namespace

ESTree::Node *transformDecorate(Context &context, ESTree::Node *root) {
  if (!DecorateRewriter(context).run(root))
    return nullptr;
  return root;
}

} // namespace hermes
