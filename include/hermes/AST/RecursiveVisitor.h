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

#include <type_traits>
#include <utility>

/// A helper template structure used to determine if a method 'visit' exists in
/// a given class 'T' and can be called with a set of arguments 'Args...'.
/// It has three template parameters:
/// - T: The type of the class to check.
/// - A dummy parameter (unnamed), used for SFINAE.
/// - Args...: The types of the arguments that we want to check if they can be
///   passed to the 'visit'.
/// The base case inherits from std::false_type, assuming 'visit' doesn't exist.
template <typename T, typename, typename... Args>
struct has_callable_visit_helper : std::false_type {};

/// This is a partial specialization of has_callable_visit_helper. It gets
/// instantiated if the expression inside std::void_t is valid.
/// std::void_t<decltype(std::declval<T>().method(std::declval<Args>()...))>
/// checks if 'visit' can be called with arguments of types Args...
/// If it can, decltype extracts the return type of the method call, and
/// std::void_t transforms it into 'void', making this specialization valid.
template <typename T, typename... Args>
struct has_callable_visit_helper<
    T,
    std::void_t<decltype(std::declval<T>().visit(std::declval<Args>()...))>,
    Args...> : std::true_type {};

/// A convenience alias template that sets the second template parameter of
/// has_callable_visit_helper to 'void'. This simplifies usage by automatically
/// filling in the non-deduced context (the dummy parameter).
template <typename T, typename... Args>
using has_callable_visit = has_callable_visit_helper<T, void, Args...>;

/// Call the visit() method, detecting the correct signature to use.
template <typename N, typename V>
void visitCaller(V &v, Node **node, Node *parent) {
  if constexpr (has_callable_visit<V, N *, Node **, Node *>::value)
    v.visit(static_cast<N *>(*node), node, parent);
  else if constexpr (has_callable_visit<V, N *, Node **>::value)
    v.visit(static_cast<N *>(*node), node);
  else if constexpr (has_callable_visit<V, N *, Node *>::value)
    v.visit(static_cast<N *>(*node), parent);
  else
    v.visit(static_cast<N *>(*node));
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
/// This class is not intended to be used directly. Instead, two global wrapper
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
/// \section AST Mutation
///
/// The dispatcher supports optional AST mutation. To enable mutation, the
/// visitor needs to implement one of two additional signatures:
/// \code
///     visit(NodeType *n, Node **ppNode)
///     visit(NodeType *n, Node **ppNode, Node *parent)
/// \endcode
/// In either case, writing to \c *ppNode allows the visitor to replace the
/// current node. In most cases \c *ppNode is literally the node pointer in the
/// AST.
///
/// This technique allows easy mutation of AST nodes referenced directly by a
/// parent node (i.e. expression nodes). However modifying nodes in a
/// \c NodeList, which is needed for almost all mutation, presents additional
/// challenges.
///
/// \subsection NodeList Mutation
///
/// \c NodeList works differently because there is no pointer to the node that
/// can be modified. The node has been inserted in an intrusive doubly-linked
/// list. This also means that the node could be moved into another list by the
/// visitor, before we know it. Handling this requires a bit of care after the
/// visitor has been invoked, and is not free.
///
/// The dispatcher does not support mutating \c NodeList by default. Enabling
/// mutation and the additional cost requires an explicit opt in. The visitor
/// class must declare the following public constant:
/// \code
///     static constexpr bool kEnableNodeListMutation = true;
/// \endcode
///
/// NOTE: AST mutation affects only the AST. If there are side tables, etc.,
/// maintained by other compiler modules, which need to be kept synchronized
/// with the AST, they have to be taken care of explicitly by the user of this
/// API.
///
/// \param Visitor the visitor class.
template <class Visitor>
struct RecursiveVisitorDispatch {
  /// Invoke Visitor::visit(cast<Type>(node)) with node being cast to its
  /// concrete type, so the visitor can use static overloading to efficiently
  /// dispatch on different types at compile time.
  static void visit(Visitor &v, Node *&nodeRef, Node *parent) {
    auto *node = nodeRef;
    if (!node)
      return;
    if (LLVM_UNLIKELY(!v.incRecursionDepth(node)))
      return;

    switch (node->getKind()) {
      default:
        llvm_unreachable("invalid node kind");

#define VISIT(NAME)                                       \
  case NodeKind::NAME:                                    \
    detail::visitCaller<NAME##Node>(v, &nodeRef, parent); \
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
#define ESTREE_NODE_9_ARGS(NAME, ...) VISIT(NAME)

#include "hermes/AST/ESTree.def"

#undef VISIT
    }
    v.decRecursionDepth();
  }

  static void visit(Visitor &, NodeLabel, Node *) {}
  static void visit(Visitor &, NodeBoolean, Node *) {}
  static void visit(Visitor &, NodeNumber &, Node *) {}

  template <typename V>
  static constexpr auto has_node_list_override(V *v)
      -> decltype(v->visit(*static_cast<NodeList *>(nullptr), nullptr), true) {
    return true;
  }
  template <typename V>
  static constexpr bool has_node_list_override(...) {
    return false;
  }

  template <typename V>
  static constexpr auto has_node_list_mutation(V *)
      -> decltype(V::kEnableNodeListMutation, true) {
    return V::kEnableNodeListMutation;
  }
  template <typename V>
  static constexpr bool has_node_list_mutation(...) {
    return false;
  }

  static void visit(Visitor &v, NodeList &list, Node *parent) {
    if constexpr (has_node_list_override<Visitor>(nullptr))
      return v.visit(list, parent);

    if constexpr (has_node_list_mutation<Visitor>(nullptr)) {
      // Iterator to the previous element, initialized with the sentinel.
      NodeList::iterator prevIt = list._before_begin();
      for (auto it = list.begin(), end = list.end(); it != end;) {
        auto curr = it;
        Node *pNode = &*it++;
        visit(v, pNode, parent);

        // If the node was replaced, we need to erase it from the list without
        // touching the node itself, and optionally insert a replacement node.
        if (pNode != &*curr) {
          list._erase_between(prevIt, it);
          if (pNode)
            prevIt = list.insert(it, *pNode);
        } else {
          prevIt = curr;
        }
      }
    } else {
      for (Node &node : list) {
        Node *pNode = &node;
        visit(v, pNode, parent);
        assert(
            pNode == &node &&
            "NodeList mutation is disabled. Declare kEnableNodeListMutation=true to enable it.");
      }
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
#define ESTREE_NODE_9_ARGS(NAME, ...) VISIT(NAME)

#include "hermes/AST/ESTree.def"

#undef VISIT
    }
  }

/// Declare helper functions to recursively visit the children of a node.
#define ESTREE_NODE_0_ARGS(NAME, BASE)                   \
  HERMES_ALWAYS_INLINE static inline void visitChildren( \
      Visitor &v, NAME##Node *) {}

#define ESTREE_NODE_1_ARGS(NAME, BASE, ARG0TY, ARG0NM, ARG0OPT) \
  HERMES_ALWAYS_INLINE static inline void visitChildren(        \
      Visitor &v, NAME##Node *node) {                           \
    visit(v, node->_##ARG0NM, node);                            \
  }

#define ESTREE_NODE_2_ARGS(                                       \
    NAME, BASE, ARG0TY, ARG0NM, ARG0OPT, ARG1TY, ARG1NM, ARG1OPT) \
  HERMES_ALWAYS_INLINE static inline void visitChildren(          \
      Visitor &v, NAME##Node *node) {                             \
    visit(v, node->_##ARG0NM, node);                              \
    visit(v, node->_##ARG1NM, node);                              \
  }

#define ESTREE_NODE_3_ARGS(                              \
    NAME,                                                \
    BASE,                                                \
    ARG0TY,                                              \
    ARG0NM,                                              \
    ARG0OPT,                                             \
    ARG1TY,                                              \
    ARG1NM,                                              \
    ARG1OPT,                                             \
    ARG2TY,                                              \
    ARG2NM,                                              \
    ARG2OPT)                                             \
  HERMES_ALWAYS_INLINE static inline void visitChildren( \
      Visitor &v, NAME##Node *node) {                    \
    visit(v, node->_##ARG0NM, node);                     \
    visit(v, node->_##ARG1NM, node);                     \
    visit(v, node->_##ARG2NM, node);                     \
  }

#define ESTREE_NODE_4_ARGS(                              \
    NAME,                                                \
    BASE,                                                \
    ARG0TY,                                              \
    ARG0NM,                                              \
    ARG0OPT,                                             \
    ARG1TY,                                              \
    ARG1NM,                                              \
    ARG1OPT,                                             \
    ARG2TY,                                              \
    ARG2NM,                                              \
    ARG2OPT,                                             \
    ARG3TY,                                              \
    ARG3NM,                                              \
    ARG3OPT)                                             \
  HERMES_ALWAYS_INLINE static inline void visitChildren( \
      Visitor &v, NAME##Node *node) {                    \
    visit(v, node->_##ARG0NM, node);                     \
    visit(v, node->_##ARG1NM, node);                     \
    visit(v, node->_##ARG2NM, node);                     \
    visit(v, node->_##ARG3NM, node);                     \
  }

#define ESTREE_NODE_5_ARGS(                              \
    NAME,                                                \
    BASE,                                                \
    ARG0TY,                                              \
    ARG0NM,                                              \
    ARG0OPT,                                             \
    ARG1TY,                                              \
    ARG1NM,                                              \
    ARG1OPT,                                             \
    ARG2TY,                                              \
    ARG2NM,                                              \
    ARG2OPT,                                             \
    ARG3TY,                                              \
    ARG3NM,                                              \
    ARG3OPT,                                             \
    ARG4TY,                                              \
    ARG4NM,                                              \
    ARG4OPT)                                             \
  HERMES_ALWAYS_INLINE static inline void visitChildren( \
      Visitor &v, NAME##Node *node) {                    \
    visit(v, node->_##ARG0NM, node);                     \
    visit(v, node->_##ARG1NM, node);                     \
    visit(v, node->_##ARG2NM, node);                     \
    visit(v, node->_##ARG3NM, node);                     \
    visit(v, node->_##ARG4NM, node);                     \
  }

#define ESTREE_NODE_6_ARGS(                              \
    NAME,                                                \
    BASE,                                                \
    ARG0TY,                                              \
    ARG0NM,                                              \
    ARG0OPT,                                             \
    ARG1TY,                                              \
    ARG1NM,                                              \
    ARG1OPT,                                             \
    ARG2TY,                                              \
    ARG2NM,                                              \
    ARG2OPT,                                             \
    ARG3TY,                                              \
    ARG3NM,                                              \
    ARG3OPT,                                             \
    ARG4TY,                                              \
    ARG4NM,                                              \
    ARG4OPT,                                             \
    ARG5TY,                                              \
    ARG5NM,                                              \
    ARG5OPT)                                             \
  HERMES_ALWAYS_INLINE static inline void visitChildren( \
      Visitor &v, NAME##Node *node) {                    \
    visit(v, node->_##ARG0NM, node);                     \
    visit(v, node->_##ARG1NM, node);                     \
    visit(v, node->_##ARG2NM, node);                     \
    visit(v, node->_##ARG3NM, node);                     \
    visit(v, node->_##ARG4NM, node);                     \
    visit(v, node->_##ARG5NM, node);                     \
  }

#define ESTREE_NODE_7_ARGS(                              \
    NAME,                                                \
    BASE,                                                \
    ARG0TY,                                              \
    ARG0NM,                                              \
    ARG0OPT,                                             \
    ARG1TY,                                              \
    ARG1NM,                                              \
    ARG1OPT,                                             \
    ARG2TY,                                              \
    ARG2NM,                                              \
    ARG2OPT,                                             \
    ARG3TY,                                              \
    ARG3NM,                                              \
    ARG3OPT,                                             \
    ARG4TY,                                              \
    ARG4NM,                                              \
    ARG4OPT,                                             \
    ARG5TY,                                              \
    ARG5NM,                                              \
    ARG5OPT,                                             \
    ARG6TY,                                              \
    ARG6NM,                                              \
    ARG6OPT)                                             \
  HERMES_ALWAYS_INLINE static inline void visitChildren( \
      Visitor &v, NAME##Node *node) {                    \
    visit(v, node->_##ARG0NM, node);                     \
    visit(v, node->_##ARG1NM, node);                     \
    visit(v, node->_##ARG2NM, node);                     \
    visit(v, node->_##ARG3NM, node);                     \
    visit(v, node->_##ARG4NM, node);                     \
    visit(v, node->_##ARG5NM, node);                     \
    visit(v, node->_##ARG6NM, node);                     \
  }

#define ESTREE_NODE_8_ARGS(                              \
    NAME,                                                \
    BASE,                                                \
    ARG0TY,                                              \
    ARG0NM,                                              \
    ARG0OPT,                                             \
    ARG1TY,                                              \
    ARG1NM,                                              \
    ARG1OPT,                                             \
    ARG2TY,                                              \
    ARG2NM,                                              \
    ARG2OPT,                                             \
    ARG3TY,                                              \
    ARG3NM,                                              \
    ARG3OPT,                                             \
    ARG4TY,                                              \
    ARG4NM,                                              \
    ARG4OPT,                                             \
    ARG5TY,                                              \
    ARG5NM,                                              \
    ARG5OPT,                                             \
    ARG6TY,                                              \
    ARG6NM,                                              \
    ARG6OPT,                                             \
    ARG7TY,                                              \
    ARG7NM,                                              \
    ARG7OPT)                                             \
  HERMES_ALWAYS_INLINE static inline void visitChildren( \
      Visitor &v, NAME##Node *node) {                    \
    visit(v, node->_##ARG0NM, node);                     \
    visit(v, node->_##ARG1NM, node);                     \
    visit(v, node->_##ARG2NM, node);                     \
    visit(v, node->_##ARG3NM, node);                     \
    visit(v, node->_##ARG4NM, node);                     \
    visit(v, node->_##ARG5NM, node);                     \
    visit(v, node->_##ARG6NM, node);                     \
    visit(v, node->_##ARG7NM, node);                     \
  }

#define ESTREE_NODE_9_ARGS(                              \
    NAME,                                                \
    BASE,                                                \
    ARG0TY,                                              \
    ARG0NM,                                              \
    ARG0OPT,                                             \
    ARG1TY,                                              \
    ARG1NM,                                              \
    ARG1OPT,                                             \
    ARG2TY,                                              \
    ARG2NM,                                              \
    ARG2OPT,                                             \
    ARG3TY,                                              \
    ARG3NM,                                              \
    ARG3OPT,                                             \
    ARG4TY,                                              \
    ARG4NM,                                              \
    ARG4OPT,                                             \
    ARG5TY,                                              \
    ARG5NM,                                              \
    ARG5OPT,                                             \
    ARG6TY,                                              \
    ARG6NM,                                              \
    ARG6OPT,                                             \
    ARG7TY,                                              \
    ARG7NM,                                              \
    ARG7OPT,                                             \
    ARG8TY,                                              \
    ARG8NM,                                              \
    ARG8OPT)                                             \
  HERMES_ALWAYS_INLINE static inline void visitChildren( \
      Visitor &v, NAME##Node *node) {                    \
    visit(v, node->_##ARG0NM, node);                     \
    visit(v, node->_##ARG1NM, node);                     \
    visit(v, node->_##ARG2NM, node);                     \
    visit(v, node->_##ARG3NM, node);                     \
    visit(v, node->_##ARG4NM, node);                     \
    visit(v, node->_##ARG5NM, node);                     \
    visit(v, node->_##ARG6NM, node);                     \
    visit(v, node->_##ARG7NM, node);                     \
    visit(v, node->_##ARG8NM, node);                     \
  }

#include "hermes/AST/ESTree.def"
};

/// Invoke Visitor::visit(cast<Type>(node)) with node being cast to its
/// concrete type, so the visitor can use static overloading to efficiently
/// dispatch on different types at compile time.
/// The node pointer could be modified by the visitor.
template <class Visitor>
void visitESTreeNode(Visitor &v, Node *&node, Node *parent) {
  RecursiveVisitorDispatch<Visitor>::visit(v, node, parent);
}

template <class Visitor>
void visitESTreeNodeList(Visitor &v, NodeList &list, Node *parent) {
  RecursiveVisitorDispatch<Visitor>::visit(v, list, parent);
}

/// Invoke Visitor::visit(cast<Type>(node)) with node being cast to its
/// concrete type, so the visitor can use static overloading to efficiently
/// dispatch on different types at compile time.
/// \p node must not be replaced by the visitor, because there is no way to pass
/// that information back to the caller of this function. This restriction does
/// not apply to child nodes - if it is a mutating visitor, it could still
/// change the AST, it is just not allowed to replace the specific node passed
/// as a parameter here.
template <class Visitor>
void visitESTreeNodeNoReplace(Visitor &v, Node *node, Node *parent) {
  Node *saveNode = node;
  (void)saveNode;
  RecursiveVisitorDispatch<Visitor>::visit(v, node, parent);
  assert(
      saveNode == node &&
      "node must not be replaced in-place with a no-replace visitor");
}

/// Invoke Visitor::visit(cast<Type>(node)) with node being cast to its
/// concrete type, so the visitor can use static overloading to efficiently
/// dispatch on different types at compile time.
/// The node pointer must not be modified by the visitor.
template <class Visitor>
void visitESTreeNodeNoReplace(Visitor &v, Node *node) {
  visitESTreeNodeNoReplace(v, node, nullptr);
}

/// Recursively visit the children of the node.
template <class Visitor, class Node>
void visitESTreeChildren(Visitor &v, Node *node) {
  RecursiveVisitorDispatch<Visitor>::visitChildren(v, node);
}

/// The maximum AST nesting level. Once we reach it, we report an error and
/// stop.
static constexpr unsigned kASTMaxRecursionDepth =
#if defined(HERMES_LIMIT_STACK_DEPTH) || defined(_MSC_VER)
    512
#else
    1024
#endif
    ;

/// This class implements the recursion depth control protocol defined by
/// RecursiveVisitor. It is intended to be subclassed by visitors that need
/// to enforce a maximum AST nesting level. They need to provide a method
/// with the following signature:
/// \code
///     void recursionDepthExceeded(Node *);
/// \endcode
template <class Derived>
class RecursionDepthTracker {
 protected:
  /// \c ESTree::kASTMaxRecursionDepth minus the current AST nesting level. Once
  /// it reaches 0, we report an error and stop modifying it.
  unsigned recursionDepth_;

  explicit RecursionDepthTracker(unsigned recursionDepth)
      : recursionDepth_(recursionDepth) {}

 public:
  explicit RecursionDepthTracker()
      : RecursionDepthTracker(kASTMaxRecursionDepth) {}

  /// This method implements the first part of the protocol defined by
  /// RecursiveVisitor. It is supposed to return true if everything is normal,
  /// and false if we should not visit the current node.
  /// It maintains the current AST nesting level, and generates an error the
  /// first time it exceeds the maximum nesting level. Once that happens, it
  /// always returns false.
  bool incRecursionDepth(ESTree::Node *n) {
    if (LLVM_UNLIKELY(recursionDepth_ == 0))
      return false;
    --recursionDepth_;
    if (LLVM_UNLIKELY(recursionDepth_ == 0)) {
      static_cast<Derived *>(this)->recursionDepthExceeded(n);
      return false;
    }
    return true;
  }

  /// This is the second part of the protocol defined by RecursiveVisitor.
  /// Once we have reached the maximum nesting level, it does nothing. Otherwise
  /// it decrements the nesting level.
  void decRecursionDepth() {
    if (LLVM_LIKELY(recursionDepth_ != 0))
      ++recursionDepth_;
  }
};

/// This class implements the recursion depth control protocol for visitors that
/// execute nested in a parent visitor. To decrease the coupling, it takes a
/// current recursion depth and a callback to invoke when we exceed the maximum.
template <class Derived>
class NestedRecursionDepthTracker
    : public RecursionDepthTracker<NestedRecursionDepthTracker<Derived>> {
  /// We call this when we exceed the maximum recursion depth.
  const std::function<void(ESTree::Node *)> &recursionDepthExceeded_;

 public:
  /// \param recursionDepth remaining recursion depth
  /// \param recursionDepthExceeded handler to invoke when we transition fron
  ///     non-zero to zero remaining recursion depth.
  explicit NestedRecursionDepthTracker(
      unsigned recursionDepth,
      const std::function<void(ESTree::Node *)> &recursionDepthExceeded)
      : RecursionDepthTracker<NestedRecursionDepthTracker<Derived>>(
            recursionDepth),
        recursionDepthExceeded_(recursionDepthExceeded) {}

  /// We call this when we exceed the maximum recursion depth.
  void recursionDepthExceeded(ESTree::Node *n) {
    recursionDepthExceeded_(n);
  }
};

}; // namespace ESTree
} // namespace hermes

#endif // HERMES_AST_RECURSIVEVISITOR_H
