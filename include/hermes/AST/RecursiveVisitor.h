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

#include <variant>

namespace hermes {
namespace ESTree {

/// Unmodified (of type UnmodifiedT) is the value that visitors should return to
/// indicate nothing was changed on the node that was visited. A visitor that
/// returns void is assumed to always return Unmodified.
struct UnmodifiedT {};

inline UnmodifiedT Unmodified;

/// Removed (of type RemovedT) is the value that visitors should return to
/// indicate that the node that's been visited should be removed.
struct RemovedT {};

inline RemovedT Removed;

/// VisitResult is the return that that visitors should have (if they are not
/// void) so they can communicate back what should be done with the node that's
/// just been processed.
using VisitResult = std::variant<UnmodifiedT, RemovedT, Node *>;

namespace detail {

/// Call the method v.visit(N *node) or v.visit(N *node, Node *parent).
template <typename V, typename N, typename U = void>
struct VisitCaller {
  static auto call(V &v, N *node, Node *) {
    // The visitor does not need the parent node.
    if constexpr (std::is_same_v<void, decltype(v.visit(node))>) {
      // Visitor returning void is assumed to preserve the AST.
      v.visit(node);
      return Unmodified;
    } else {
      // Visitor returns a status; pipe it back to the caller.
      return v.visit(node);
    }
  }
};

/// VisitTakesParent<V, N> is either SFINAE, or void. So, if SFINAE does not
/// happen, VisitCaller<V, N, VisitTakesParent<V, N>> exists, and always
/// applies. If not, only the default above exists, and that is used instead.
template <
    typename V,
    typename N,
    typename = decltype(((V *)nullptr)->visit((N *)nullptr, (Node *)nullptr))>
struct VisitTakesParentImpl {
  using Type = void;
};
template <typename V, typename N>
using EnableIfVisitTakesParent = typename VisitTakesParentImpl<V, N>::Type;

template <typename V, typename N>
struct VisitCaller<V, N, EnableIfVisitTakesParent<V, N>> {
  static auto call(V &v, N *node, Node *parent) {
    if constexpr (std::is_same_v<void, decltype(v.visit(node, parent))>) {
      // Visitor returning void is assumed to preserve the AST.
      v.visit(node, parent);
      return Unmodified;
    } else {
      // Visitor returns a status; pipe it back to the caller.
      return v.visit(node, parent);
    }
  }
};

/// isReadOnlyVisit<V, N>() \return  true if V::visit(N*[, Node *]) returns
/// void, and false otherwise.

// The default definition of IsReadOnlyVisitImpl is used when the V::visit does
// not need the parent node.
template <typename V, typename N, typename = void>
struct IsReadOnlyVisitImpl {
  static constexpr bool value =
      std::is_same_v<void, decltype(((V *)nullptr)->visit((N *)nullptr))>;
};

// This definition of IsReadOnlyVisitImpl is used when V::Visit takes the parent
// node.
template <typename V, typename N>
struct IsReadOnlyVisitImpl<V, N, EnableIfVisitTakesParent<V, N>> {
  static constexpr bool value = std::is_same_v<
      void,
      decltype(((V *)nullptr)->visit((N *)nullptr, (Node *)nullptr))>;
};

template <typename V, typename N>
inline constexpr bool isReadOnlyVisit() {
  return IsReadOnlyVisitImpl<V, N>::value;
}

/// \returns whether \p Visitor is a read-only visitor or not. A read-only
/// visitor does not have any visit methods that return VisitResult.
template <typename Visitor>
static constexpr bool isReadOnlyVisitor() {
#define ESTREE_NODE_0_ARGS(NAME, ...) isReadOnlyVisit<Visitor, NAME##Node>() &&
#define ESTREE_NODE_1_ARGS(NAME, ...) isReadOnlyVisit<Visitor, NAME##Node>() &&
#define ESTREE_NODE_2_ARGS(NAME, ...) isReadOnlyVisit<Visitor, NAME##Node>() &&
#define ESTREE_NODE_3_ARGS(NAME, ...) isReadOnlyVisit<Visitor, NAME##Node>() &&
#define ESTREE_NODE_4_ARGS(NAME, ...) isReadOnlyVisit<Visitor, NAME##Node>() &&
#define ESTREE_NODE_5_ARGS(NAME, ...) isReadOnlyVisit<Visitor, NAME##Node>() &&
#define ESTREE_NODE_6_ARGS(NAME, ...) isReadOnlyVisit<Visitor, NAME##Node>() &&
#define ESTREE_NODE_7_ARGS(NAME, ...) isReadOnlyVisit<Visitor, NAME##Node>() &&
#define ESTREE_NODE_8_ARGS(NAME, ...) isReadOnlyVisit<Visitor, NAME##Node>() &&

  return
#include "hermes/AST/ESTree.def"
      true;
}

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
///     X visit(NodeType *n)
///     X visit(NodeType *n, Node *parent)
/// \endcode
/// where X is one of void|VisitResult. Read-only visitors (i.e., those that
/// don't need to modify the AST) should return void, and mutating visitors,
/// VisitResult.
///
/// Additionally the visitor class must implement the
/// following two methods: \code
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
/// \param readOnlyVisitor is defined in terms of Visitor, and should not be
/// specified by the user code; it is a template parameter to ensure proper
/// compile-time evaluation of isReadOnlyVisitor<Visitor>.
template <
    class Visitor,
    bool readOnlyVisitor = detail::isReadOnlyVisitor<Visitor>()>
struct RecursiveVisitorDispatch {
  /// Invoke Visitor::visit(cast<Type>(node)) with node being cast to its
  /// concrete type, so the visitor can use static overloading to efficiently
  /// dispatch on different types at compile time.
  static VisitResult visit(Visitor &v, Node *node, Node *parent) {
    if (!node)
      return Unmodified;
    if (LLVM_UNLIKELY(!v.incRecursionDepth(node)))
      return Unmodified;

    VisitResult result;
    switch (node->getKind()) {
      default:
        llvm_unreachable("invalid node kind");

#define VISIT(NAME)                                          \
  case NodeKind::NAME:                                       \
    result = detail::VisitCaller<Visitor, NAME##Node>::call( \
        v, cast<NAME##Node>(node), parent);                  \
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
    return result;
  }

  static UnmodifiedT visit(Visitor &, NodeLabel, Node *) {
    return Unmodified;
  }
  static UnmodifiedT visit(Visitor &, NodeBoolean, Node *) {
    return Unmodified;
  }
  static UnmodifiedT visit(Visitor &, NodeNumber &, Node *) {
    return Unmodified;
  }

  static UnmodifiedT visit(Visitor &v, NodeList &list, Node *parent) {
    if constexpr (readOnlyVisitor) {
      // Read-only visitors don't need the extra overhead of checking visit's
      // return value.
      for (Node &node : list) {
        visit(v, &node, parent);
      }
    } else {
      for (auto it = list.begin(), end = list.end(); it != end;) {
        auto curr = it++;

        // Temporarily remove curr from list, which is important in case v
        // repurposes the node and inserts it into another list.
        list.erase(curr);

        VisitResult res = visit(v, &*curr, parent);
        if (std::holds_alternative<Node *>(res)) {
          // The visitor decided to return a new node; insert it.
          list.insert(it, *std::get<Node *>(res));
        } else if (std::holds_alternative<UnmodifiedT>(res)) {
          // The visitor didn't do anything to the old node; insert it back.
          list.insert(it, *curr);
        } else {
          // The visitor decided to remove the node; do nothing, as the node's
          // been removed already.
          assert(
              std::holds_alternative<RemovedT>(res) &&
              "Unexpected VisitResult alternative.");
        }
      }
    }
    return Unmodified;
  }

  /// postVisit is invoked during visitChildren below to handle the visitor's
  /// return value. For any type other than Node *, visit() always returns
  /// Unmodified (as the visitor is never invoked for those types).
  static void postVisit(NodeLabel *, UnmodifiedT) {}

  static void postVisit(NodeBoolean *, UnmodifiedT) {}

  static void postVisit(NodeNumber *, UnmodifiedT) {}

  static void postVisit(NodeList *, UnmodifiedT) {}

  static void postVisit(Node **node, VisitResult vr) {
    if (std::holds_alternative<RemovedT>(vr)) {
      // The visitor removed the node.
      *node = nullptr;
    } else if (std::holds_alternative<Node *>(vr)) {
      // The visitor updated the node.
      *node = std::get<Node *>(vr);
    } else {
      assert(
          std::holds_alternative<UnmodifiedT>(vr) &&
          "Unexpected VisitResult alternative.");
    }
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

#define VISIT(visitor, field, parent)                        \
  if constexpr (readOnlyVisitor) {                           \
    /* Read-only visitors don't need post-visit procesing */ \
    visit(visitor, field, parent);                           \
  } else {                                                   \
    postVisit(&(field), visit(visitor, field, parent));      \
  }

/// Declare helper functions to recursively visit the children of a node.
#define ESTREE_NODE_0_ARGS(NAME, BASE) \
  static void visitChildren(Visitor &v, NAME##Node *) {}

#define ESTREE_NODE_1_ARGS(NAME, BASE, ARG0TY, ARG0NM, ARG0OPT) \
  static void visitChildren(Visitor &v, NAME##Node *node) {     \
    VISIT(v, node->_##ARG0NM, node);                            \
  }

#define ESTREE_NODE_2_ARGS(                                       \
    NAME, BASE, ARG0TY, ARG0NM, ARG0OPT, ARG1TY, ARG1NM, ARG1OPT) \
  static void visitChildren(Visitor &v, NAME##Node *node) {       \
    VISIT(v, node->_##ARG0NM, node);                              \
    VISIT(v, node->_##ARG1NM, node);                              \
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
    VISIT(v, node->_##ARG0NM, node);                        \
    VISIT(v, node->_##ARG1NM, node);                        \
    VISIT(v, node->_##ARG2NM, node);                        \
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
    VISIT(v, node->_##ARG0NM, node);                        \
    VISIT(v, node->_##ARG1NM, node);                        \
    VISIT(v, node->_##ARG2NM, node);                        \
    VISIT(v, node->_##ARG3NM, node);                        \
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
    VISIT(v, node->_##ARG0NM, node);                        \
    VISIT(v, node->_##ARG1NM, node);                        \
    VISIT(v, node->_##ARG2NM, node);                        \
    VISIT(v, node->_##ARG3NM, node);                        \
    VISIT(v, node->_##ARG4NM, node);                        \
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
    VISIT(v, node->_##ARG0NM, node);                        \
    VISIT(v, node->_##ARG1NM, node);                        \
    VISIT(v, node->_##ARG2NM, node);                        \
    VISIT(v, node->_##ARG3NM, node);                        \
    VISIT(v, node->_##ARG4NM, node);                        \
    VISIT(v, node->_##ARG5NM, node);                        \
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
    VISIT(v, node->_##ARG0NM, node);                        \
    VISIT(v, node->_##ARG1NM, node);                        \
    VISIT(v, node->_##ARG2NM, node);                        \
    VISIT(v, node->_##ARG3NM, node);                        \
    VISIT(v, node->_##ARG4NM, node);                        \
    VISIT(v, node->_##ARG5NM, node);                        \
    VISIT(v, node->_##ARG6NM, node);                        \
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
    VISIT(v, node->_##ARG0NM, node);                        \
    VISIT(v, node->_##ARG1NM, node);                        \
    VISIT(v, node->_##ARG2NM, node);                        \
    VISIT(v, node->_##ARG3NM, node);                        \
    VISIT(v, node->_##ARG4NM, node);                        \
    VISIT(v, node->_##ARG5NM, node);                        \
    VISIT(v, node->_##ARG6NM, node);                        \
    VISIT(v, node->_##ARG7NM, node);                        \
  }

#include "hermes/AST/ESTree.def"

#undef VISIT
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
