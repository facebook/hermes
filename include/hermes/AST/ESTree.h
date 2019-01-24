/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_AST_ESTREE_H
#define HERMES_AST_ESTREE_H

#include "hermes/AST/Context.h"
#include "hermes/Support/StringTable.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/ilist.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/SMLoc.h"

namespace hermes {

using llvm::ArrayRef;
using llvm::StringRef;

namespace ESTree {

using llvm::cast;
using llvm::SMLoc;
using llvm::SMRange;

class Node;
using NodeLabel = UniqueString *;
using NodeBoolean = bool;
using NodeNumber = double;
using NodePtr = Node *;
using NodeList = llvm::simple_ilist<Node>;

enum class NodeKind {
#define ESTREE_FIRST(NAME, ...) _##NAME##_First,
#define ESTREE_LAST(NAME) _##NAME##_Last,
#define ESTREE_NODE_0_ARGS(NAME, ...) NAME,
#define ESTREE_NODE_1_ARGS(NAME, ...) NAME,
#define ESTREE_NODE_2_ARGS(NAME, ...) NAME,
#define ESTREE_NODE_3_ARGS(NAME, ...) NAME,
#define ESTREE_NODE_4_ARGS(NAME, ...) NAME,
#include "ESTree.def"
};

/// This is the base class of all ESTree nodes.
class Node : public llvm::ilist_node<Node> {
  Node(const Node &) = delete;
  void operator=(const Node &) = delete;
  NodeKind kind_;
  SMRange sourceRange_{};
  SMLoc debugLoc_{};

 public:
  explicit Node(NodeKind kind) : kind_(kind) {}

  void setSourceRange(SMRange rng) {
    sourceRange_ = rng;
  }
  SMRange getSourceRange() const {
    return sourceRange_;
  }
  void setStartLoc(SMLoc loc) {
    sourceRange_.Start = loc;
  }
  SMLoc getStartLoc() const {
    return sourceRange_.Start;
  }
  void setEndLoc(SMLoc loc) {
    sourceRange_.End = loc;
  }
  SMLoc getEndLoc() const {
    return sourceRange_.End;
  }
  void setDebugLoc(SMLoc loc) {
    debugLoc_ = loc;
  }
  SMLoc getDebugLoc() const {
    return debugLoc_;
  }

  /// \returns the textual name of the node.
  StringRef getNodeName() {
    switch (getKind()) {
      default:
        llvm_unreachable("invalid node kind");

#define ESTREE_NODE_0_ARGS(NAME, ...) \
  case NodeKind::NAME:                \
    return #NAME;
#define ESTREE_NODE_1_ARGS(NAME, ...) \
  case NodeKind::NAME:                \
    return #NAME;
#define ESTREE_NODE_2_ARGS(NAME, ...) \
  case NodeKind::NAME:                \
    return #NAME;
#define ESTREE_NODE_3_ARGS(NAME, ...) \
  case NodeKind::NAME:                \
    return #NAME;
#define ESTREE_NODE_4_ARGS(NAME, ...) \
  case NodeKind::NAME:                \
    return #NAME;

#include "ESTree.def"
    }
  }

  template <class Visitor>
  void visit(Visitor &V) {
    ESTreeVisit(V, this);
  }

  /// \returns the kind of the value.
  NodeKind getKind() const {
    return kind_;
  }
  static bool classof(const NodePtr) {
    return true;
  }

  // Allow allocation of AST nodes by using the Context allocator or by a
  // placement new.

  void *
  operator new(size_t size, Context &ctx, size_t alignment = alignof(double)) {
    return ctx.allocateNode(size, alignment);
  }
  void *operator new(size_t, void *mem) {
    return mem;
  }

  void operator delete(void *, const Context &, size_t) {}
  void operator delete(void *, size_t) {}

 private:
  // Make new/delete illegal for AST nodes.

  void *operator new(size_t) {
    llvm_unreachable("AST nodes cannot be allocated with regular new");
  }
  void operator delete(void *) {
    llvm_unreachable("AST nodes cannot be released with regular delete");
  }
};

} // namespace ESTree
} // namespace hermes

//===----------------------------------------------------------------------===//
// ilist_traits for Node
//===----------------------------------------------------------------------===//

namespace llvm {

template <>
struct ilist_traits<::hermes::ESTree::Node>
    : public ilist_default_traits<::hermes::ESTree::Node> {
  using Node = ::hermes::ESTree::Node;
};

} // namespace llvm

namespace hermes {
namespace ESTree {

// Skip lables.
template <class Visitor>
void ESTreeVisit(Visitor &V, const NodeLabel &label) {}
template <class Visitor>
void ESTreeVisit(Visitor &V, const NodeBoolean &label) {}
template <class Visitor>
void ESTreeVisit(Visitor &V, const NodeNumber &label) {}

// Visit all nodes in a list.
template <class Visitor>
void ESTreeVisit(Visitor &V, NodeList &Lst) {
  for (auto &Elem : Lst) {
    ESTreeVisit(V, &Elem);
  }
}

// Forward declarations of all nodes.
#define ESTREE_FIRST(NAME, ...) class NAME##Node;
#define ESTREE_NODE_0_ARGS(NAME, ...) class NAME##Node;
#define ESTREE_NODE_1_ARGS(NAME, ...) class NAME##Node;
#define ESTREE_NODE_2_ARGS(NAME, ...) class NAME##Node;
#define ESTREE_NODE_3_ARGS(NAME, ...) class NAME##Node;
#define ESTREE_NODE_4_ARGS(NAME, ...) class NAME##Node;

#include "ESTree.def"

/// An enum to track the "strictness" of a function and whether it has been
/// initialized.
enum class Strictness {
  NotSet,
  NonStrictMode,
  StrictMode,
};

/// \return true if the strictness is set to \c StrictMode. Assert if it hasn't
/// been set.
inline bool isStrict(Strictness strictness) {
  assert(strictness != Strictness::NotSet && "strictness hasn't been set");
  return strictness == Strictness::StrictMode;
}

/// \return \c Strictness::StrictNode or \c Strictness::NonStrictMode depending
///   on the value of \p strictMode.
inline Strictness makeStrictness(bool strictMode) {
  return strictMode ? Strictness::StrictMode : Strictness::NonStrictMode;
}

namespace detail {
/// We need to to be able customize some ESTree types when passing them through
/// a constructor, so we create a simple template type mapper. Specifically, a
/// NodeList has to be passed by RValue-reference and moved into place.
/// In the default case, the type is unmodified.
template <class T>
struct ParamTrait {
  using Type = T;
};

/// NodeList is mapped to NodeList &&.
template <>
struct ParamTrait<NodeList> {
  using Type = NodeList &&;
};

/// All nodes derrive from this empty decorator class by default, unless
/// explicitly overwritten by specializing DecoratorTrait for the node class.
class EmptyDecoration {};

/// This template determines the decorator for each node. It must be specialized
/// as necessary for each node.
template <class T>
struct DecoratorTrait {
  using Type = EmptyDecoration;
};

/// Decoration for all function-like nodes.
class FunctionLikeDecoration {
 public:
  Strictness strictness{Strictness::NotSet};
};

/// Decoration for all statements.
/// NOTE: This decoration is required by the Statement base node, so we need to
/// provide it even if it is empty.
class StatementDecoration {};

/// Decoration for all loop statements.
/// NOTE: This decoration is required by the LoopStatement base node, so we need
/// to  provide it even if it is empty.
class LoopStatementDecoration {};

class BlockStatementDecoration {
 public:
  /// True if this is a function body that was pruned while pre-parsing.
  bool isLazyFunctionBody{false};
  /// The source buffer id in which this block was found (see \p SourceMgr ).
  uint32_t bufferId;
};

class StringLiteralDecoration {
 public:
  /// Indicates whether the string literal originally contained any escapes
  /// or new line continuations. We need this in order to detect directives
  /// (ES5.1. 14.1).
  bool potentialDirective = false;

  /// Was this recognised as a directive.
  bool directive = false;
};

template <>
struct DecoratorTrait<StringLiteralNode> {
  using Type = StringLiteralDecoration;
};
template <>
struct DecoratorTrait<BlockStatementNode> {
  using Type = BlockStatementDecoration;
};

} // namespace detail

/// A convenince alias for the base node.
using BaseNode = Node;

#define ESTREE_FIRST(NAME, BASE)                                          \
  class NAME##Node : public BASE##Node, public detail::NAME##Decoration { \
   public:                                                                \
    explicit NAME##Node(NodeKind kind) : BASE##Node(kind) {}              \
    static bool classof(const Node *V) {                                  \
      auto kind = V->getKind();                                           \
      return NodeKind::_##NAME##_First < kind &&                          \
          kind < NodeKind::_##NAME##_Last;                                \
    }                                                                     \
  };

#define ESTREE_NODE_0_ARGS(NAME, BASE)                                 \
  class NAME##Node : public BASE##Node,                                \
                     public detail::DecoratorTrait<NAME##Node>::Type { \
   public:                                                             \
    explicit NAME##Node() : BASE##Node(NodeKind::NAME) {}              \
    static bool classof(const Node *V) {                               \
      return V->getKind() == NodeKind::NAME;                           \
    }                                                                  \
    template <class Visitor>                                           \
    void visit(Visitor &V) {                                           \
      if (!V.shouldVisit(this)) {                                      \
        return;                                                        \
      }                                                                \
      V.enter(this);                                                   \
      V.leave(this);                                                   \
    }                                                                  \
  };

#define ESTREE_NODE_1_ARGS(NAME, BASE, ARG0TY, ARG0NM, ARG0OPT)        \
  class NAME##Node : public BASE##Node,                                \
                     public detail::DecoratorTrait<NAME##Node>::Type { \
   public:                                                             \
    ARG0TY _##ARG0NM;                                                  \
    explicit NAME##Node(detail::ParamTrait<ARG0TY>::Type ARG0NM_)      \
        : BASE##Node(NodeKind::NAME), _##ARG0NM(std::move(ARG0NM_)) {} \
    template <class Visitor>                                           \
    void visit(Visitor &V) {                                           \
      if (!V.shouldVisit(this)) {                                      \
        return;                                                        \
      }                                                                \
      V.enter(this);                                                   \
      ESTreeVisit(V, _##ARG0NM);                                       \
      V.leave(this);                                                   \
    }                                                                  \
    static bool classof(const Node *V) {                               \
      return V->getKind() == NodeKind::NAME;                           \
    }                                                                  \
  };

#define ESTREE_NODE_2_ARGS(                                            \
    NAME, BASE, ARG0TY, ARG0NM, ARG0OPT, ARG1TY, ARG1NM, ARG1OPT)      \
  class NAME##Node : public BASE##Node,                                \
                     public detail::DecoratorTrait<NAME##Node>::Type { \
   public:                                                             \
    ARG0TY _##ARG0NM;                                                  \
    ARG1TY _##ARG1NM;                                                  \
    explicit NAME##Node(                                               \
        ARG0TY ARG0NM_,                                                \
        detail::ParamTrait<ARG1TY>::Type ARG1NM_)                      \
        : BASE##Node(NodeKind::NAME),                                  \
          _##ARG0NM(ARG0NM_),                                          \
          _##ARG1NM(std::move(ARG1NM_)) {}                             \
    template <class Visitor>                                           \
    void visit(Visitor &V) {                                           \
      if (!V.shouldVisit(this)) {                                      \
        return;                                                        \
      }                                                                \
      V.enter(this);                                                   \
      ESTreeVisit(V, _##ARG0NM);                                       \
      ESTreeVisit(V, _##ARG1NM);                                       \
      V.leave(this);                                                   \
    }                                                                  \
    static bool classof(const Node *V) {                               \
      return V->getKind() == NodeKind::NAME;                           \
    }                                                                  \
  };

#define ESTREE_NODE_3_ARGS(                                            \
    NAME,                                                              \
    BASE,                                                              \
    ARG0TY,                                                            \
    ARG0NM,                                                            \
    ARG0OPT,                                                           \
    ARG1TY,                                                            \
    ARG1NM,                                                            \
    ARG1OPT,                                                           \
    ARG2TY,                                                            \
    ARG2NM,                                                            \
    ARG2OPT)                                                           \
  class NAME##Node : public BASE##Node,                                \
                     public detail::DecoratorTrait<NAME##Node>::Type { \
   public:                                                             \
    ARG0TY _##ARG0NM;                                                  \
    ARG1TY _##ARG1NM;                                                  \
    ARG2TY _##ARG2NM;                                                  \
    explicit NAME##Node(                                               \
        ARG0TY ARG0NM_,                                                \
        detail::ParamTrait<ARG1TY>::Type ARG1NM_,                      \
        ARG2TY ARG2NM_)                                                \
        : BASE##Node(NodeKind::NAME),                                  \
          _##ARG0NM(ARG0NM_),                                          \
          _##ARG1NM(std::move(ARG1NM_)),                               \
          _##ARG2NM(ARG2NM_) {}                                        \
    template <class Visitor>                                           \
    void visit(Visitor &V) {                                           \
      if (!V.shouldVisit(this)) {                                      \
        return;                                                        \
      }                                                                \
      V.enter(this);                                                   \
      ESTreeVisit(V, _##ARG0NM);                                       \
      ESTreeVisit(V, _##ARG1NM);                                       \
      ESTreeVisit(V, _##ARG2NM);                                       \
      V.leave(this);                                                   \
    }                                                                  \
    static bool classof(const Node *V) {                               \
      return V->getKind() == NodeKind::NAME;                           \
    }                                                                  \
  };

#define ESTREE_NODE_4_ARGS(                                            \
    NAME,                                                              \
    BASE,                                                              \
    ARG0TY,                                                            \
    ARG0NM,                                                            \
    ARG0OPT,                                                           \
    ARG1TY,                                                            \
    ARG1NM,                                                            \
    ARG1OPT,                                                           \
    ARG2TY,                                                            \
    ARG2NM,                                                            \
    ARG2OPT,                                                           \
    ARG3TY,                                                            \
    ARG3NM,                                                            \
    ARG3OPT)                                                           \
  class NAME##Node : public BASE##Node,                                \
                     public detail::DecoratorTrait<NAME##Node>::Type { \
   public:                                                             \
    ARG0TY _##ARG0NM;                                                  \
    ARG1TY _##ARG1NM;                                                  \
    ARG2TY _##ARG2NM;                                                  \
    ARG3TY _##ARG3NM;                                                  \
    explicit NAME##Node(                                               \
        ARG0TY ARG0NM_,                                                \
        ARG1TY ARG1NM_,                                                \
        detail::ParamTrait<ARG2TY>::Type ARG2NM_,                      \
        ARG3TY ARG3NM_)                                                \
        : BASE##Node(NodeKind::NAME),                                  \
          _##ARG0NM(ARG0NM_),                                          \
          _##ARG1NM(ARG1NM_),                                          \
          _##ARG2NM(std::move(ARG2NM_)),                               \
          _##ARG3NM(ARG3NM_) {}                                        \
    template <class Visitor>                                           \
    void visit(Visitor &V) {                                           \
      if (!V.shouldVisit(this)) {                                      \
        return;                                                        \
      }                                                                \
      V.enter(this);                                                   \
      ESTreeVisit(V, _##ARG0NM);                                       \
      ESTreeVisit(V, _##ARG1NM);                                       \
      ESTreeVisit(V, _##ARG2NM);                                       \
      ESTreeVisit(V, _##ARG3NM);                                       \
      V.leave(this);                                                   \
    }                                                                  \
                                                                       \
    static bool classof(const Node *V) {                               \
      return V->getKind() == NodeKind::NAME;                           \
    }                                                                  \
  };

#include "ESTree.def"

// Visit nodes.
template <class Visitor>
void ESTreeVisit(Visitor &V, NodePtr Node) {
  if (!Node) {
    return;
  }

  switch (Node->getKind()) {
    default:
      llvm_unreachable("invalid node kind");

#define ESTREE_NODE_0_ARGS(NAME, ...) \
  case NodeKind::NAME:                \
    return cast<NAME##Node>(Node)->visit(V);
#define ESTREE_NODE_1_ARGS(NAME, ...) \
  case NodeKind::NAME:                \
    return cast<NAME##Node>(Node)->visit(V);
#define ESTREE_NODE_2_ARGS(NAME, ...) \
  case NodeKind::NAME:                \
    return cast<NAME##Node>(Node)->visit(V);
#define ESTREE_NODE_3_ARGS(NAME, ...) \
  case NodeKind::NAME:                \
    return cast<NAME##Node>(Node)->visit(V);
#define ESTREE_NODE_4_ARGS(NAME, ...) \
  case NodeKind::NAME:                \
    return cast<NAME##Node>(Node)->visit(V);

#include "ESTree.def"
  }
}

/// Check whether a node is a directive (a statement consisting of a single
/// string) and if so extract the directive string.
llvm::Optional<NodeLabel> matchDirective(const ESTree::Node *node);

} // namespace ESTree
} // namespace hermes

#endif
