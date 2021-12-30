/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_RECURSIVEVISITOR_H
#define HERMES_AST_RECURSIVEVISITOR_H

#include "hermes/AST/ESTree.h"

#include "llvh/Support/Casting.h"

namespace hermes {
namespace ESTree {

namespace detail {

/// Call the method v.visit(N *node) or v.visit(N *node, Node *parent).
template <typename V, typename N, typename U = void>
struct VisitCaller {
  static void call(V &v, N *node, Node *) {
    v.visit(node);
  }
};

// decltype((void)&...) is either SFINAE, or void.
// So, if SFINAE does not happen for V, then this specialization exists
// for VisitCaller<V, N, void>, and always applies.  If not, only the
// default above exists, and that is used instead.
template <typename V, typename N>
struct VisitCaller<
    V,
    N,
    decltype((void)static_cast<void (V::*)(N *, Node *)>(&V::visit))> {
  static void call(V &v, N *node, Node *parent) {
    v.visit(node, parent);
  }
};

} // namespace detail

using llvh::cast;

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
///
/// The visitor class must at least implement the method `visit(Node *)` and
/// in addition to that any node-specific overloads. Each overload can have one
/// of two possible signatures:
/// \code
///     visit(NodeType *n)
///     visit(NodeType *n, Node *parent)
/// \endcode
/// Additionally the visitor class must implement the following two methods:
/// \code
///     bool incRecursionDepth(Node *);
///     void decRecursionDepth();
/// \endcode
/// If incRecursionDepth() returns false, the current visitor immediately
/// returns. The purpose of these methods is to protect against stack overflow.
///
/// A recommended implementation will increment and decrement a "depth" value
/// until it exceeds a certain threshold. At that point it should generate an
/// error and set a "failure" mode flag and return false. All future invocations
/// of both methods should do nothing and just return false once the "failure"
/// flag is set.
///
/// \param Visitor the visitor class.
template <class Visitor>
struct RecursiveVisitorDispatch {
  /// Invoke Visitor::visit(cast<Type>(node)) with node being cast to its
  /// concrete type, so the visitor can use static overloading to efficiently
  /// dispatch on different types at compile time.
  static void visit(Visitor &v, Node *node, Node *parent) {
    if (!node)
      return;
    if (LLVM_UNLIKELY(!v.incRecursionDepth(node)))
      return;

    switch (node->getKind()) {
      default:
        llvm_unreachable("invalid node kind");

#define VISIT(NAME)                                 \
  case NodeKind::NAME:                              \
    detail::VisitCaller<Visitor, NAME##Node>::call( \
        v, cast<NAME##Node>(node), parent);         \
    break;

#define ESTREE_NODE_0_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_1_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_2_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_3_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_4_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_5_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_6_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_7_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_8_ARGS(NAME, ...) VISIT(NAME)

#include "hermes/AST/ESTree.def"

#undef VISIT
    }
    v.decRecursionDepth();
  }

  static void visit(Visitor &, NodeLabel, Node *) {}
  static void visit(Visitor &, NodeBoolean, Node *) {}
  static void visit(Visitor &, NodeNumber &, Node *) {}

  static void visit(Visitor &v, NodeList &list, Node *parent) {
    for (auto &node : list)
      visit(v, &node, parent);
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

#define ESTREE_NODE_0_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_1_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_2_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_3_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_4_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_5_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_6_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_7_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_8_ARGS(NAME, ...) VISIT(NAME)

#include "hermes/AST/ESTree.def"

#undef VISIT
    }
  }

/// Declare helper functions to recursively visit the children of a node.
#define ESTREE_NODE_0_ARGS(NAME, BASE) \
  static void visitChildren(Visitor &v, NAME##Node *) {}

#define ESTREE_NODE_1_ARGS(NAME, BASE, ARG0TY, ARG0NM, ARG0OPT) \
  static void visitChildren(Visitor &v, NAME##Node *node) {     \
    visit(v, node->_##ARG0NM, node);                            \
  }

#define ESTREE_NODE_2_ARGS(                                       \
    NAME, BASE, ARG0TY, ARG0NM, ARG0OPT, ARG1TY, ARG1NM, ARG1OPT) \
  static void visitChildren(Visitor &v, NAME##Node *node) {       \
    visit(v, node->_##ARG0NM, node);                              \
    visit(v, node->_##ARG1NM, node);                              \
  }

#define ESTREE_NODE_3_ARGS(                                 \
    NAME,                                                   \
    BASE,                                                   \
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
    visit(v, node->_##ARG0NM, node);                        \
    visit(v, node->_##ARG1NM, node);                        \
    visit(v, node->_##ARG2NM, node);                        \
  }

#define ESTREE_NODE_4_ARGS(                                 \
    NAME,                                                   \
    BASE,                                                   \
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
    visit(v, node->_##ARG0NM, node);                        \
    visit(v, node->_##ARG1NM, node);                        \
    visit(v, node->_##ARG2NM, node);                        \
    visit(v, node->_##ARG3NM, node);                        \
  }

#define ESTREE_NODE_5_ARGS(                                 \
    NAME,                                                   \
    BASE,                                                   \
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
    ARG3OPT,                                                \
    ARG4TY,                                                 \
    ARG4NM,                                                 \
    ARG4OPT)                                                \
  static void visitChildren(Visitor &v, NAME##Node *node) { \
    visit(v, node->_##ARG0NM, node);                        \
    visit(v, node->_##ARG1NM, node);                        \
    visit(v, node->_##ARG2NM, node);                        \
    visit(v, node->_##ARG3NM, node);                        \
    visit(v, node->_##ARG4NM, node);                        \
  }

#define ESTREE_NODE_6_ARGS(                                 \
    NAME,                                                   \
    BASE,                                                   \
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
    ARG3OPT,                                                \
    ARG4TY,                                                 \
    ARG4NM,                                                 \
    ARG4OPT,                                                \
    ARG5TY,                                                 \
    ARG5NM,                                                 \
    ARG5OPT)                                                \
  static void visitChildren(Visitor &v, NAME##Node *node) { \
    visit(v, node->_##ARG0NM, node);                        \
    visit(v, node->_##ARG1NM, node);                        \
    visit(v, node->_##ARG2NM, node);                        \
    visit(v, node->_##ARG3NM, node);                        \
    visit(v, node->_##ARG4NM, node);                        \
    visit(v, node->_##ARG5NM, node);                        \
  }

#define ESTREE_NODE_7_ARGS(                                 \
    NAME,                                                   \
    BASE,                                                   \
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
    ARG3OPT,                                                \
    ARG4TY,                                                 \
    ARG4NM,                                                 \
    ARG4OPT,                                                \
    ARG5TY,                                                 \
    ARG5NM,                                                 \
    ARG5OPT,                                                \
    ARG6TY,                                                 \
    ARG6NM,                                                 \
    ARG6OPT)                                                \
  static void visitChildren(Visitor &v, NAME##Node *node) { \
    visit(v, node->_##ARG0NM, node);                        \
    visit(v, node->_##ARG1NM, node);                        \
    visit(v, node->_##ARG2NM, node);                        \
    visit(v, node->_##ARG3NM, node);                        \
    visit(v, node->_##ARG4NM, node);                        \
    visit(v, node->_##ARG5NM, node);                        \
    visit(v, node->_##ARG6NM, node);                        \
  }

#define ESTREE_NODE_8_ARGS(                                 \
    NAME,                                                   \
    BASE,                                                   \
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
    ARG3OPT,                                                \
    ARG4TY,                                                 \
    ARG4NM,                                                 \
    ARG4OPT,                                                \
    ARG5TY,                                                 \
    ARG5NM,                                                 \
    ARG5OPT,                                                \
    ARG6TY,                                                 \
    ARG6NM,                                                 \
    ARG6OPT,                                                \
    ARG7TY,                                                 \
    ARG7NM,                                                 \
    ARG7OPT)                                                \
  static void visitChildren(Visitor &v, NAME##Node *node) { \
    visit(v, node->_##ARG0NM, node);                        \
    visit(v, node->_##ARG1NM, node);                        \
    visit(v, node->_##ARG2NM, node);                        \
    visit(v, node->_##ARG3NM, node);                        \
    visit(v, node->_##ARG4NM, node);                        \
    visit(v, node->_##ARG5NM, node);                        \
    visit(v, node->_##ARG6NM, node);                        \
    visit(v, node->_##ARG7NM, node);                        \
  }

#include "hermes/AST/ESTree.def"
};

/// Invoke Visitor::visit(cast<Type>(node)) with node being cast to its
/// concrete type, so the visitor can use static overloading to efficiently
/// dispatch on different types at compile time.
template <class Visitor>
void visitESTreeNode(Visitor &v, Node *node, Node *parent = nullptr) {
  RecursiveVisitorDispatch<Visitor>::visit(v, node, parent);
}

/// Recursively visit the children of the node.
template <class Visitor, class Node>
void visitESTreeChildren(Visitor &v, Node *node) {
  RecursiveVisitorDispatch<Visitor>::visitChildren(v, node);
}

} // namespace ESTree
} // namespace hermes

#endif // HERMES_AST_RECURSIVEVISITOR_H
