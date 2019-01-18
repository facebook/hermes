/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_AST_RECURSIVEVISITOR_H
#define HERMES_AST_RECURSIVEVISITOR_H

#include "hermes/AST/ESTree.h"

#include "llvm/Support/Casting.h"

namespace hermes {
namespace ESTree {

using llvm::cast;

/// A different kind of dispatcher allowing the visitor to visit the child
/// nodes recursively from its stack frame. This allows usage of commoan RAII
/// patterns.
///
/// There are two important methods:
/// - visit(Visitor,Node):
///     dispatches to the apropriate visitor based on type
/// - visitChildren(Visitor,Node):
///     recursively visits all children of the node.
///
/// This class is not intended to be used directly. Instead two global wrapper
/// functions: visitESTreeNode() and visitESTreeChildren() have been defined.
template <class Visitor>
struct RecursiveVisitorDispatch {
  /// Invoke Visitor::visit(cast<Type>(node)) with node being cast to its
  /// concrete type, so the visitor can use static overloading to efficiently
  /// dispatch on different types at compile time.
  static void visit(Visitor &v, Node *node) {
    if (!node)
      return;

    switch (node->getKind()) {
      default:
        llvm_unreachable("invalid node kind");

#define VISIT(NAME)    \
  case NodeKind::NAME: \
    return v.visit(cast<NAME##Node>(node));

#define ESTREE_NODE_0_ARGS(NAME) VISIT(NAME)
#define ESTREE_NODE_1_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_2_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_3_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_4_ARGS(NAME, ...) VISIT(NAME)

#include "hermes/AST/ESTree.def"

#undef VISIT
    }
  }

  static void visit(Visitor &, NodeLabel) {}
  static void visit(Visitor &, NodeBoolean) {}
  static void visit(Visitor &, NodeNumber &) {}

  static void visit(Visitor &v, NodeList &list) {
    for (auto &node : list)
      visit(v, &node);
  }

  /// Recursively visit the children of the node.
  static void visitChildren(Visitor &v, Node *node) {
    if (!node)
      return;

    switch (node->getKind()) {
      default:
        llvm_unreachable("invalid node kind");

#define VISIT(NAME)    \
  case NodeKind::NAME: \
    return visitChildren(v, cast<NAME##Node>(node));

#define ESTREE_NODE_0_ARGS(NAME) VISIT(NAME)
#define ESTREE_NODE_1_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_2_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_3_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_4_ARGS(NAME, ...) VISIT(NAME)

#include "hermes/AST/ESTree.def"

#undef VISIT
    }
  }

/// Declare helper functions to recursively visit the children of a node.
#define ESTREE_NODE_0_ARGS(NAME) \
  static void visitChildren(Visitor &v, NAME##Node *) {}

#define ESTREE_NODE_1_ARGS(NAME, ARG0TY, ARG0NM, ARG0OPT)   \
  static void visitChildren(Visitor &v, NAME##Node *node) { \
    visit(v, node->_##ARG0NM);                              \
  }

#define ESTREE_NODE_2_ARGS(                                 \
    NAME, ARG0TY, ARG0NM, ARG0OPT, ARG1TY, ARG1NM, ARG1OPT) \
  static void visitChildren(Visitor &v, NAME##Node *node) { \
    visit(v, node->_##ARG0NM);                              \
    visit(v, node->_##ARG1NM);                              \
  }

#define ESTREE_NODE_3_ARGS(                                 \
    NAME,                                                   \
    ARG0TY,                                                 \
    ARG0NM,                                                 \
    ARG0OPT,                                                \
    ARG1TY,                                                 \
    ARG1NM,                                                 \
    ARG1OPT,                                                \
    ARG2TY,                                                 \
    ARG2NM,                                                 \
    ARG2OPT)                                                \
  static void visitChildren(Visitor &v, NAME##Node *node) { \
    visit(v, node->_##ARG0NM);                              \
    visit(v, node->_##ARG1NM);                              \
    visit(v, node->_##ARG2NM);                              \
  }

#define ESTREE_NODE_4_ARGS(                                 \
    NAME,                                                   \
    ARG0TY,                                                 \
    ARG0NM,                                                 \
    ARG0OPT,                                                \
    ARG1TY,                                                 \
    ARG1NM,                                                 \
    ARG1OPT,                                                \
    ARG2TY,                                                 \
    ARG2NM,                                                 \
    ARG2OPT,                                                \
    ARG3TY,                                                 \
    ARG3NM,                                                 \
    ARG3OPT)                                                \
  static void visitChildren(Visitor &v, NAME##Node *node) { \
    visit(v, node->_##ARG0NM);                              \
    visit(v, node->_##ARG1NM);                              \
    visit(v, node->_##ARG2NM);                              \
    visit(v, node->_##ARG3NM);                              \
  }

#include "hermes/AST/ESTree.def"
};

/// Invoke Visitor::visit(cast<Type>(node)) with node being cast to its
/// concrete type, so the visitor can use static overloading to efficiently
/// dispatch on different types at compile time.
template <class Visitor>
void visitESTreeNode(Visitor &v, Node *node) {
  RecursiveVisitorDispatch<Visitor>::visit(v, node);
}

/// Recursively visit the children of the node.
template <class Visitor, class Node>
void visitESTreeChildren(Visitor &v, Node *node) {
  RecursiveVisitorDispatch<Visitor>::visitChildren(v, node);
}

} // namespace ESTree
} // namespace hermes

#endif // HERMES_AST_RECURSIVEVISITOR_H
