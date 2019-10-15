/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_ESTREE_H
#define HERMES_AST_ESTREE_H

#include "hermes/AST/Context.h"
#include "hermes/Support/StringTable.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/ilist.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/SMLoc.h"

namespace hermes {

using llvm::ArrayRef;
using llvm::StringRef;

namespace sem {
class FunctionInfo;
} // namespace sem

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
#define ESTREE_NODE_5_ARGS(NAME, ...) NAME,
#include "ESTree.def"
};

/// This is the base class of all ESTree nodes.
class Node : public llvm::ilist_node<Node> {
  Node(const Node &) = delete;
  void operator=(const Node &) = delete;

  NodeKind kind_;

  /// How many parens this node was surrounded by.
  /// This value can be 0, 1 and 2 (indicating 2 or more).
  unsigned parens_ = 0;

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

  unsigned getParens() const {
    return parens_;
  }
  void incParens() {
    parens_ = parens_ < 2 ? parens_ + 1 : 2;
  }
  void clearParens() {
    parens_ = 0;
  }

  /// Copy all location data from a different node.
  void copyLocationFrom(const Node *src) {
    setSourceRange(src->getSourceRange());
    setDebugLoc(src->getDebugLoc());
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
#define ESTREE_NODE_5_ARGS(NAME, ...) \
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

// Skip labels.
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
#define ESTREE_NODE_5_ARGS(NAME, ...) class NAME##Node;

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

/// Decoration for all function-like nodes.
class FunctionLikeDecoration {
  sem::FunctionInfo *semInfo_{};

 public:
  Strictness strictness{Strictness::NotSet};
  /// Whether this function was a method definiton rather than using 'function'.
  /// Note that getters and setters are also considered method definitions,
  /// as they do not use the keyword 'function'.
  bool isMethodDefinition{false};

  void setSemInfo(sem::FunctionInfo *semInfo) {
    assert(semInfo && "setting semInfo to null");
    assert(!semInfo_ && "semInfo is already set");
    semInfo_ = semInfo;
  }

  sem::FunctionInfo *getSemInfo() const {
    assert(semInfo_ && "semInfo is not set!");
    return semInfo_;
  }
};

class ProgramDecoration {
 public:
  // An empty parameter list which we need for compatibility with functions.
  NodeList dummyParamList;
};

/// A decoration describing a label.
class LabelDecorationBase {
  static constexpr unsigned INVALID_LABEL = ~0u;
  unsigned labelIndex_ = INVALID_LABEL;

 public:
  bool isLabelIndexSet() const {
    return labelIndex_ != INVALID_LABEL;
  }

  unsigned getLabelIndex() const {
    assert(isLabelIndexSet() && "labelIndex is not set");
    return labelIndex_;
  }

  void setLabelIndex(unsigned labelIndex) {
    assert(labelIndex != INVALID_LABEL && "setting labelIndex to invalid");
    assert(!isLabelIndexSet() && "labelIndex is already set");
    labelIndex_ = labelIndex;
  }
};

/// A decoration for a break/continue statement.
class GotoDecorationBase : public LabelDecorationBase {};

/// Decoration for all statements.
/// NOTE: This decoration is required by the Statement base node, so we need to
/// provide it even if it is empty.
class StatementDecoration {};

/// Decoration for all loop statements.
/// NOTE: This decoration is required by the LoopStatement base node, so we need
/// to  provide it even if it is empty.
/// It contains an optional label, if it has been referenced by a contained
/// break/continue.
class LoopStatementDecoration : public LabelDecorationBase {};

class SwitchStatementDecoration : public LabelDecorationBase {};

class BreakStatementDecoration : public GotoDecorationBase {};
class ContinueStatementDecoration : public GotoDecorationBase {};

class LabeledStatementDecoration : public LabelDecorationBase {};

class BlockStatementDecoration {
 public:
  /// True if this is a function body that was pruned while pre-parsing.
  bool isLazyFunctionBody{false};
  /// The source buffer id in which this block was found (see \p SourceMgr ).
  uint32_t bufferId;
};

class PatternDecoration {};
class CoverDecoration {};

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

template <>
struct DecoratorTrait<BlockStatementNode> {
  using Type = BlockStatementDecoration;
};
template <>
struct DecoratorTrait<BreakStatementNode> {
  using Type = BreakStatementDecoration;
};
template <>
struct DecoratorTrait<ContinueStatementNode> {
  using Type = ContinueStatementDecoration;
};
template <>
struct DecoratorTrait<SwitchStatementNode> {
  using Type = SwitchStatementDecoration;
};
template <>
struct DecoratorTrait<LabeledStatementNode> {
  using Type = LabeledStatementDecoration;
};
template <>
struct DecoratorTrait<ProgramNode> {
  using Type = ProgramDecoration;
};

} // namespace detail

/// A convenince alias for the base node.
using BaseNode = Node;

#define ESTREE_FIRST(NAME, BASE)                                  \
  class NAME##Node : public BASE##Node, public NAME##Decoration { \
   public:                                                        \
    explicit NAME##Node(NodeKind kind) : BASE##Node(kind) {}      \
    static bool classof(const Node *V) {                          \
      auto kind = V->getKind();                                   \
      return NodeKind::_##NAME##_First < kind &&                  \
          kind < NodeKind::_##NAME##_Last;                        \
    }                                                             \
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
        detail::ParamTrait<ARG0TY>::Type ARG0NM_,                      \
        detail::ParamTrait<ARG1TY>::Type ARG1NM_)                      \
        : BASE##Node(NodeKind::NAME),                                  \
          _##ARG0NM(std::move(ARG0NM_)),                               \
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
        detail::ParamTrait<ARG0TY>::Type ARG0NM_,                      \
        detail::ParamTrait<ARG1TY>::Type ARG1NM_,                      \
        detail::ParamTrait<ARG2TY>::Type ARG2NM_)                      \
        : BASE##Node(NodeKind::NAME),                                  \
          _##ARG0NM(std::move(ARG0NM_)),                               \
          _##ARG1NM(std::move(ARG1NM_)),                               \
          _##ARG2NM(std::move(ARG2NM_)) {}                             \
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
        detail::ParamTrait<ARG0TY>::Type ARG0NM_,                      \
        detail::ParamTrait<ARG1TY>::Type ARG1NM_,                      \
        detail::ParamTrait<ARG2TY>::Type ARG2NM_,                      \
        detail::ParamTrait<ARG3TY>::Type ARG3NM_)                      \
        : BASE##Node(NodeKind::NAME),                                  \
          _##ARG0NM(std::move(ARG0NM_)),                               \
          _##ARG1NM(std::move(ARG1NM_)),                               \
          _##ARG2NM(std::move(ARG2NM_)),                               \
          _##ARG3NM(std::move(ARG3NM_)) {}                             \
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

#define ESTREE_NODE_5_ARGS(                                            \
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
    ARG3OPT,                                                           \
    ARG4TY,                                                            \
    ARG4NM,                                                            \
    ARG4OPT)                                                           \
  class NAME##Node : public BASE##Node,                                \
                     public detail::DecoratorTrait<NAME##Node>::Type { \
   public:                                                             \
    ARG0TY _##ARG0NM;                                                  \
    ARG1TY _##ARG1NM;                                                  \
    ARG2TY _##ARG2NM;                                                  \
    ARG3TY _##ARG3NM;                                                  \
    ARG4TY _##ARG4NM;                                                  \
    explicit NAME##Node(                                               \
        detail::ParamTrait<ARG0TY>::Type ARG0NM_,                      \
        detail::ParamTrait<ARG1TY>::Type ARG1NM_,                      \
        detail::ParamTrait<ARG2TY>::Type ARG2NM_,                      \
        detail::ParamTrait<ARG3TY>::Type ARG3NM_,                      \
        detail::ParamTrait<ARG4TY>::Type ARG4NM_)                      \
        : BASE##Node(NodeKind::NAME),                                  \
          _##ARG0NM(std::move(ARG0NM_)),                               \
          _##ARG1NM(std::move(ARG1NM_)),                               \
          _##ARG2NM(std::move(ARG2NM_)),                               \
          _##ARG3NM(std::move(ARG3NM_)),                               \
          _##ARG4NM(std::move(ARG4NM_)) {}                             \
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
      ESTreeVisit(V, _##ARG4NM);                                       \
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
#define ESTREE_NODE_5_ARGS(NAME, ...) \
  case NodeKind::NAME:                \
    return cast<NAME##Node>(Node)->visit(V);

#include "ESTree.def"
  }
}

/// Return a reference to the parameter list of a FunctionLikeNode.
NodeList &getParams(FunctionLikeNode *node);

/// If the body of the function-like node is a block statement, return it,
/// otherwise return nullptr.
/// ProgramNode doesn't have a block statement body, as well as some arrow
/// functions.
BlockStatementNode *getBlockStatement(FunctionLikeNode *node);

} // namespace ESTree
} // namespace hermes

#endif
