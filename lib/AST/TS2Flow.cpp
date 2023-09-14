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
class TS2FlowConverter
    : public ESTree::RecursionDepthTracker<TS2FlowConverter> {
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

    if (sm_.getErrorCount())
      return nullptr;

    if (!TS2FlowValidator(context_).validate(program)) {
      return nullptr;
    }

    return program;
  }

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

  /// Iterate over the given list and convert any TS nodes in the list.
  void visit(ESTree::NodeList &list, ESTree::Node *parent) {
    for (auto iter = list.begin(), end = list.end(); iter != end; ++iter) {
      auto &node = *iter;

      if (!llvh::isa<ESTree::TSNode>(node)) {
        ESTree::visitESTreeNode(*this, &node, parent);
        continue;
      }

      // Convert a TSNode to a matching Flow node and replace the TSNode with
      // the result node.
      if (auto *result = convertTSNode(&node)) {
        iter = list.insert(iter, *result);
        list.remove(node);
      }
    }
  }

  void recursionDepthExceeded(ESTree::Node *n) {
    sm_.error(
        n->getEndLoc(),
        "ts2flow: too many nested expressions/statements/declarations");
  }

 private:
  /// Converts the given node to a matching Flow node, when the given
  /// node is a TS node. Otherwise, returns the given node as is. It returns
  /// nullptr after reporting an error via the context's source manager when
  /// the conversion failed.
  ESTree::Node *convertTSNode(ESTree::Node *node) {
    if (!node || !llvh::isa<ESTree::TSNode>(node)) {
      return node;
    }

    switch (node->getKind()) {
#define CASE(KIND)             \
  case ESTree::NodeKind::KIND: \
    return convertTSNode(llvh::cast<ESTree::KIND##Node>(node));
      CASE(TSTypeAnnotation)
      CASE(TSAnyKeyword)
      CASE(TSNumberKeyword)
      CASE(TSBooleanKeyword)
      CASE(TSStringKeyword)
      CASE(TSSymbolKeyword)
      CASE(TSVoidKeyword)
      CASE(TSUndefinedKeyword)
      CASE(TSUnknownKeyword)
      CASE(TSNeverKeyword)
      CASE(TSBigIntKeyword)
      CASE(TSLiteralType)
      CASE(TSArrayType)
      CASE(TSTypeReference)
      CASE(TSTypeAliasDeclaration)
#undef CASE
      default:
        sm_.error(
            node->getSourceRange(),
            "ts2flow: unsupported node conversion for " + node->getNodeName());
        return nullptr;
    }
  }

#define DEFINE_CONVERT_TSNODE_SIMPLE(TS, FLOW)          \
  ESTree::Node *convertTSNode(ESTree::TS##Node *node) { \
    return createNode<ESTree::FLOW##Node>(node);        \
  }
  DEFINE_CONVERT_TSNODE_SIMPLE(TSAnyKeyword, AnyTypeAnnotation)
  DEFINE_CONVERT_TSNODE_SIMPLE(TSNumberKeyword, NumberTypeAnnotation)
  DEFINE_CONVERT_TSNODE_SIMPLE(TSBooleanKeyword, BooleanTypeAnnotation)
  DEFINE_CONVERT_TSNODE_SIMPLE(TSStringKeyword, StringTypeAnnotation)
  DEFINE_CONVERT_TSNODE_SIMPLE(TSSymbolKeyword, SymbolTypeAnnotation)
  DEFINE_CONVERT_TSNODE_SIMPLE(TSVoidKeyword, VoidTypeAnnotation)
  DEFINE_CONVERT_TSNODE_SIMPLE(TSUndefinedKeyword, VoidTypeAnnotation)
  DEFINE_CONVERT_TSNODE_SIMPLE(TSUnknownKeyword, MixedTypeAnnotation)
  DEFINE_CONVERT_TSNODE_SIMPLE(TSNeverKeyword, EmptyTypeAnnotation)
  DEFINE_CONVERT_TSNODE_SIMPLE(TSBigIntKeyword, BigIntTypeAnnotation)
#undef DEFINE_CONVERT_TSNODE_SIMPLE

  ESTree::Node *convertTSNode(ESTree::TSTypeAnnotationNode *node) {
    return createNode<ESTree::TypeAnnotationNode>(
        node, convertTSNode(node->_typeAnnotation));
  }

  ESTree::Node *convertTSNode(ESTree::TSLiteralTypeNode *node) {
    switch (node->_literal->getKind()) {
      case ESTree::NodeKind::NullLiteral:
        return createNode<ESTree::NullLiteralTypeAnnotationNode>(node);
      case ESTree::NodeKind::NumericLiteral: {
        auto *lit = llvh::cast<ESTree::NumericLiteralNode>(node->_literal);
        return createNode<ESTree::NumberLiteralTypeAnnotationNode>(
            node, lit->_value, getRawString(lit->getSourceRange()));
      }
      case ESTree::NodeKind::StringLiteral: {
        auto *lit = llvh::cast<ESTree::StringLiteralNode>(node->_literal);
        return createNode<ESTree::StringLiteralTypeAnnotationNode>(
            node, lit->_value, getRawString(lit->getSourceRange()));
      }
      case ESTree::NodeKind::BooleanLiteral: {
        auto *lit = llvh::cast<ESTree::BooleanLiteralNode>(node->_literal);
        return createNode<ESTree::BooleanLiteralTypeAnnotationNode>(
            node, lit->_value, getRawString(lit->getSourceRange()));
      }
      case ESTree::NodeKind::BigIntLiteral: {
        auto *lit = llvh::cast<ESTree::BigIntLiteralNode>(node->_literal);
        return createNode<ESTree::BigIntLiteralTypeAnnotationNode>(
            node, lit->_bigint);
      }
      default:
        sm_.error(
            node->getSourceRange(),
            "ts2flow: unsupported literal type " +
                node->_literal->getNodeName());
        return nullptr;
    }
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

  ESTree::Node *convertTSNode(ESTree::TSTypeAliasDeclarationNode *node) {
    if (node->_typeParameters) {
      sm_.error(node->getSourceRange(), "ts2flow: unimplemented type params");
    }
    return createNode<ESTree::TypeAliasNode>(
        node,
        node->_id,
        node->_typeParameters,
        convertTSNode(node->_typeAnnotation));
  }

  template <typename T, typename... Args>
  T *createNode(const ESTree::Node *src, Args &&...args) {
    auto *node = new (context_) T(std::forward<Args>(args)...);
    node->copyLocationFrom(src);
    return node;
  }

  UniqueString *getRawString(llvh::SMRange range) {
    return context_.getStringTable().getString(llvh::StringRef{
        range.Start.getPointer(),
        (size_t)(range.End.getPointer() - range.Start.getPointer())});
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
