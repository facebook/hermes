/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/Config.h"

#if HERMES_PARSE_TS

#include "hermes/AST/RecursiveVisitor.h"
#include "hermes/AST/StripTS.h"

namespace hermes {

namespace {

using namespace ESTree;

/// Visitor that strips erasable TypeScript syntax from the AST.
/// Non-erasable constructs (enum, namespace, parameter properties) produce
/// errors.
class StripTSTransformer
    : public ESTree::RecursionDepthTracker<StripTSTransformer> {
 public:
  static constexpr bool kEnableNodeListMutation = true;

  explicit StripTSTransformer(Context &context)
      : context_(context), sm_(context.getSourceErrorManager()) {}

  /// Default visitor: recurse into children.
  void visit(Node *node) {
    visitESTreeChildren(*this, node);
  }

  // --- Erasable: type assertions unwrapped to their expression ---

  void visit(TSAsExpressionNode *node, Node **ppNode) {
    // First recurse into the expression child.
    visitESTreeChildren(*this, node);
    // Replace `x as T` with just `x`.
    *ppNode = node->_expression;
  }

  void visit(TSTypeAssertionNode *node, Node **ppNode) {
    // First recurse into the expression child.
    visitESTreeChildren(*this, node);
    // Replace `<T>x` with just `x`.
    *ppNode = node->_expression;
  }

  // --- Erasable: type-only declarations removed from body ---

  void visit(TSTypeAliasDeclarationNode *node, Node **ppNode) {
    // Remove the entire node from its parent list.
    *ppNode = nullptr;
  }

  void visit(TSInterfaceDeclarationNode *node, Node **ppNode) {
    // Remove the entire node from its parent list.
    *ppNode = nullptr;
  }

  // --- Erasable: type-only imports/exports removed ---

  void visit(ImportDeclarationNode *node, Node **ppNode) {
    if (node->_importKind ==
        context_.getIdentifier("type").getUnderlyingPointer()) {
      *ppNode = nullptr;
      return;
    }
    visitESTreeChildren(*this, node);
  }

  void visit(ExportNamedDeclarationNode *node, Node **ppNode) {
    if (node->_exportKind ==
        context_.getIdentifier("type").getUnderlyingPointer()) {
      *ppNode = nullptr;
      return;
    }
    // Recurse into children first (the declaration may be a TS node).
    visitESTreeChildren(*this, node);
    // If the declaration was a stripped TS node (type alias/interface),
    // it will have been set to nullptr. If there are also no specifiers,
    // remove the entire export.
    if (!node->_declaration && node->_specifiers.empty() && !node->_source) {
      *ppNode = nullptr;
      return;
    }
  }

  void visit(ExportAllDeclarationNode *node, Node **ppNode) {
    if (node->_exportKind ==
        context_.getIdentifier("type").getUnderlyingPointer()) {
      *ppNode = nullptr;
      return;
    }
    visitESTreeChildren(*this, node);
  }

  // --- Erasable: strip type annotations from identifiers ---

  void visit(IdentifierNode *node) {
    node->_typeAnnotation = nullptr;
  }

  // --- Erasable: strip type params/return types from functions ---

  void visit(FunctionDeclarationNode *node) {
    node->_typeParameters = nullptr;
    node->_returnType = nullptr;
    node->_predicate = nullptr;
    visitESTreeChildren(*this, node);
  }

  void visit(FunctionExpressionNode *node) {
    node->_typeParameters = nullptr;
    node->_returnType = nullptr;
    node->_predicate = nullptr;
    visitESTreeChildren(*this, node);
  }

  void visit(ArrowFunctionExpressionNode *node) {
    node->_typeParameters = nullptr;
    node->_returnType = nullptr;
    node->_predicate = nullptr;
    visitESTreeChildren(*this, node);
  }

  // --- Erasable: strip type params from classes ---

  void visit(ClassDeclarationNode *node) {
    node->_typeParameters = nullptr;
    node->_superTypeArguments = nullptr;
    node->_implements.clear();
    visitESTreeChildren(*this, node);
  }

  void visit(ClassExpressionNode *node) {
    node->_typeParameters = nullptr;
    node->_superTypeArguments = nullptr;
    node->_implements.clear();
    visitESTreeChildren(*this, node);
  }

  // --- Erasable: strip type annotations from class properties ---

  void visit(ClassPropertyNode *node) {
    node->_typeAnnotation = nullptr;
    node->_tsModifiers = nullptr;
    visitESTreeChildren(*this, node);
  }

  void visit(ClassPrivatePropertyNode *node) {
    node->_typeAnnotation = nullptr;
    node->_tsModifiers = nullptr;
    visitESTreeChildren(*this, node);
  }

  // --- Erasable: strip type arguments from call/new expressions ---

  void visit(CallExpressionNode *node) {
    node->_typeArguments = nullptr;
    visitESTreeChildren(*this, node);
  }

  void visit(NewExpressionNode *node) {
    node->_typeArguments = nullptr;
    visitESTreeChildren(*this, node);
  }

  void visit(OptionalCallExpressionNode *node) {
    node->_typeArguments = nullptr;
    visitESTreeChildren(*this, node);
  }

  // --- Erasable: strip type annotations from patterns ---

  void visit(ObjectPatternNode *node) {
    node->_typeAnnotation = nullptr;
    visitESTreeChildren(*this, node);
  }

  void visit(ArrayPatternNode *node) {
    node->_typeAnnotation = nullptr;
    visitESTreeChildren(*this, node);
  }

  // --- Non-erasable: produce errors ---

  void visit(TSEnumDeclarationNode *node) {
    sm_.error(
        node->getStartLoc(),
        "TypeScript enums are not supported with --transform-ts");
  }

  void visit(TSModuleDeclarationNode *node) {
    sm_.error(
        node->getStartLoc(),
        "TypeScript namespaces/modules are not supported with --transform-ts");
  }

  void visit(TSModuleMemberNode *node) {
    sm_.error(
        node->getStartLoc(),
        "TypeScript namespaces/modules are not supported with --transform-ts");
  }

  void visit(TSParameterPropertyNode *node) {
    sm_.error(
        node->getStartLoc(),
        "TypeScript parameter properties are not supported "
        "with --transform-ts");
  }

  void recursionDepthExceeded(Node *n) {
    sm_.error(
        n->getEndLoc(), "Too many nested expressions/statements/declarations");
  }

 private:
  Context &context_;
  SourceErrorManager &sm_;
};

} // namespace

Node *stripTS(Context &context, Node *node) {
  StripTSTransformer transformer(context);
  visitESTreeNode(transformer, node, nullptr);
  if (context.getSourceErrorManager().getErrorCount())
    return nullptr;
  return node;
}

} // namespace hermes

#endif // HERMES_PARSE_TS
