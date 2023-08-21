/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/TS2Flow.h"

#include "hermes/AST/RecursiveVisitor.h"

namespace hermes {

namespace {

/// Class to validate the result of TS to Flow AST conversion.
class TS2FlowValidator {
 public:
  explicit TS2FlowValidator(Context &context) : context_(context) {}

  /// Validates the result of TS to Flow AST conversion.
  /// Reports errors via the context's source manager if validation fails.
  /// \param program the program node that went through TS to Flow conversion.
  /// \return true if there was no validation error, false otherwise.
  bool validate(ESTree::ProgramNode *program) {
    auto &sm = context_.getSourceErrorManager();
    if (sm.getErrorCount())
      return false;

    visitESTreeNode(*this, program);

    return sm.getErrorCount() == 0;
  }

  bool incRecursionDepth(ESTree::Node *) {
    return true;
  }

  void decRecursionDepth() {}

  void visit(ESTree::Node *node, ESTree::Node *parent) {
    // Report an error if node is a TSNode.
    if (llvh::isa<ESTree::TSNode>(node)) {
      return reportError(node, parent);
    }

    visitESTreeChildren(*this, node);
  }

 private:
  void reportError(ESTree::Node *node, ESTree::Node *parent) {
    context_.getSourceErrorManager().error(
        node->getSourceRange(),
        "ts2flow: remaining TS node " + node->getNodeName() + " in " +
            (parent ? parent->getNodeName() : "<unknown>"));
  }

  Context &context_;
};

/// Class to convert TS AST to Flow AST.
class TS2FlowConverter {
 public:
  TS2FlowConverter(Context &context)
      : context_(context), sm_(context.getSourceErrorManager()) {}

  /// Convert any TS annotation nodes to Flow annotation nodes, in place.
  /// Reports errors via the context's source manager if conversion fails.
  /// \param program the top-level program node.
  /// \return the converted program node. nullptr on error.
  ESTree::ProgramNode *convert(ESTree::ProgramNode *program) {
    if (sm_.getErrorCount())
      return nullptr;

    visitESTreeNode(*this, program);

    if (!TS2FlowValidator(context_).validate(program)) {
      return nullptr;
    }

    return program;
  }

  bool incRecursionDepth(ESTree::Node *) {
    return true;
  }
  void decRecursionDepth() {}

  /// Base case: visit the node's children.
  void visit(ESTree::Node *node) {
    visitESTreeChildren(*this, node);
  }

  void visit(ESTree::IdentifierNode *node) {
    node->_typeAnnotation = convertTSNode(node->_typeAnnotation);
    visitESTreeChildren(*this, node);
  }

  void visit(ESTree::ClassPropertyNode *node) {
    node->_typeAnnotation = convertTSNode(node->_typeAnnotation);
    if (auto *mods =
            llvh::cast_or_null<ESTree::TSModifiersNode>(node->_tsModifiers)) {
      if (mods->_readonly || mods->_accessibility) {
        sm_.error(
            node->getSourceRange(),
            "ts2flow: accessibility and readonly modifiers are not supported yet.");
      }
      node->_tsModifiers = nullptr;
    }
    visitESTreeChildren(*this, node);
  }

  void visit(ESTree::FunctionDeclarationNode *node) {
    node->_returnType = convertTSNode(node->_returnType);
    visitESTreeChildren(*this, node);
  }

 private:
  /// Converts the given node to a matching Flow node, when the given
  /// node is a TS node. Otherwise, returns the given node as is. It returns
  /// nullptr after reporting an error via the context's source manager when the
  /// conversion failed.
  ESTree::Node *convertTSNode(ESTree::Node *node) {
    if (!node || !llvh::isa<ESTree::TSNode>(node)) {
      return node;
    }

    switch (node->getKind()) {
#define CASE(KIND)             \
  case ESTree::NodeKind::KIND: \
    return convertTSNode(llvh::cast<ESTree::KIND##Node>(node));
      CASE(TSTypeAnnotation)
      CASE(TSNumberKeyword)
      CASE(TSVoidKeyword)
      CASE(TSArrayType)
      CASE(TSTypeReference)
#undef CASE
      default:
        sm_.error(
            node->getSourceRange(),
            "ts2flow: unsupported node conversion for " + node->getNodeName());
        return nullptr;
    }
  }

  ESTree::Node *convertTSNode(ESTree::TSTypeAnnotationNode *node) {
    return createNode<ESTree::TypeAnnotationNode>(
        node, convertTSNode(node->_typeAnnotation));
  }

  ESTree::Node *convertTSNode(ESTree::TSNumberKeywordNode *node) {
    return createNode<ESTree::NumberTypeAnnotationNode>(node);
  }

  ESTree::Node *convertTSNode(ESTree::TSVoidKeywordNode *node) {
    return createNode<ESTree::VoidTypeAnnotationNode>(node);
  }

  ESTree::Node *convertTSNode(ESTree::TSArrayTypeNode *node) {
    return createNode<ESTree::ArrayTypeAnnotationNode>(
        node, convertTSNode(node->_elementType));
  }

  ESTree::Node *convertTSNode(ESTree::TSTypeReferenceNode *node) {
    if (node->_typeParameters) {
      sm_.error(node->getSourceRange(), "ts2flow: unimplemented type params");
    }
    return createNode<ESTree::GenericTypeAnnotationNode>(
        node, node->_typeName, node->_typeParameters);
  }

  template <typename T, typename... Args>
  T *createNode(const ESTree::Node *src, Args &&...args) {
    auto *node = new (context_) T(std::forward<Args>(args)...);
    node->copyLocationFrom(src);
    return node;
  }

  Context &context_;

  /// A reference to Context::getSourceErrorManager() for easier access.
  SourceErrorManager &sm_;
};

} // anonymous namespace

ESTree::ProgramNode *convertTSToFlow(
    Context &context,
    ESTree::ProgramNode *program) {
  return TS2FlowConverter(context).convert(program);
}

} // namespace hermes
