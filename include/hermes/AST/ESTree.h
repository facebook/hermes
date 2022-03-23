/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_ESTREE_H
#define HERMES_AST_ESTREE_H

#include "hermes/AST/Context.h"
#include "hermes/Support/StringTable.h"

#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/Optional.h"
#include "llvh/ADT/StringRef.h"
#include "llvh/ADT/ilist.h"
#include "llvh/Support/Casting.h"
#include "llvh/Support/ErrorHandling.h"
#include "llvh/Support/SMLoc.h"

namespace hermes {

using llvh::ArrayRef;
using llvh::StringRef;

namespace sem {
class FunctionInfo;
} // namespace sem

namespace ESTree {

using llvh::cast;
using llvh::SMLoc;
using llvh::SMRange;

class Node;
/// This is a string which is guaranteed to contain only valid Unicode
/// characters when decoded. In particular no mismatched surrogate pairs.
/// It is encoded with our "modified" utf-8 encoding, where parts of surrogate
/// pairs are encoded as separate characters. So, it does NOT represent valid
/// utf-8. To turn it into valid utf-8 it must be reencoded.
using NodeLabel = UniqueString *;
/// This is a JS string, which is a sequence of arbitrary 16-bit values, which
/// may or may not represent a valid utf-16 string.
/// It is encoded with our "modified" utf-8 encoding, where each separate 16-bit
/// value is encoded as a separate character. There are no guarantees about the
/// validity.
using NodeString = UniqueString *;
using NodeBoolean = bool;
using NodeNumber = double;
using NodePtr = Node *;
using NodeList = llvh::simple_ilist<Node>;

enum class NodeKind : uint32_t {
#define ESTREE_FIRST(NAME, ...) _##NAME##_First,
#define ESTREE_LAST(NAME) _##NAME##_Last,
#define ESTREE_NODE_0_ARGS(NAME, ...) NAME,
#define ESTREE_NODE_1_ARGS(NAME, ...) NAME,
#define ESTREE_NODE_2_ARGS(NAME, ...) NAME,
#define ESTREE_NODE_3_ARGS(NAME, ...) NAME,
#define ESTREE_NODE_4_ARGS(NAME, ...) NAME,
#define ESTREE_NODE_5_ARGS(NAME, ...) NAME,
#define ESTREE_NODE_6_ARGS(NAME, ...) NAME,
#define ESTREE_NODE_7_ARGS(NAME, ...) NAME,
#define ESTREE_NODE_8_ARGS(NAME, ...) NAME,
#include "ESTree.def"
};

/// This is the base class of all ESTree nodes.
class Node : public llvh::ilist_node<Node> {
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
#define ESTREE_NODE_6_ARGS(NAME, ...) \
  case NodeKind::NAME:                \
    return #NAME;
#define ESTREE_NODE_7_ARGS(NAME, ...) \
  case NodeKind::NAME:                \
    return #NAME;
#define ESTREE_NODE_8_ARGS(NAME, ...) \
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
#define ESTREE_NODE_6_ARGS(NAME, ...) class NAME##Node;
#define ESTREE_NODE_7_ARGS(NAME, ...) class NAME##Node;
#define ESTREE_NODE_8_ARGS(NAME, ...) class NAME##Node;

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
  SourceVisibility sourceVisibility{SourceVisibility::Default};

  /// Whether this function was a method definition rather than using
  /// 'function'. Note that getters and setters are also considered method
  /// definitions, as they do not use the keyword 'function'.
  /// This is used for lazy reparsing of the function.
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
  /// The source buffer id in which this block was found (see \p SourceMgr ).
  uint32_t bufferId;
  /// True if this is a function body that was pruned while pre-parsing.
  bool isLazyFunctionBody{false};
  /// If this is a lazy block, the Yield param to restore when eagerly parsing.
  bool paramYield{false};
  /// If this is a lazy block, the Await param to restore when eagerly parsing.
  bool paramAwait{false};
};

class PatternDecoration {};
class CoverDecoration {};

class CallExpressionLikeDecoration {};
class MemberExpressionLikeDecoration {};

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

#define ESTREE_NODE_1_ARGS(NAME, BASE, ARG0TY, ARG0NM, ARG0OPT)          \
  class NAME##Node : public BASE##Node,                                  \
                     public detail::DecoratorTrait<NAME##Node>::Type {   \
   public:                                                               \
    ARG0TY _##ARG0NM;                                                    \
    explicit NAME##Node(detail::ParamTrait<ARG0TY>::Type ARG0NM##_)      \
        : BASE##Node(NodeKind::NAME), _##ARG0NM(std::move(ARG0NM##_)) {} \
    template <class Visitor>                                             \
    void visit(Visitor &V) {                                             \
      if (!V.shouldVisit(this)) {                                        \
        return;                                                          \
      }                                                                  \
      V.enter(this);                                                     \
      ESTreeVisit(V, _##ARG0NM);                                         \
      V.leave(this);                                                     \
    }                                                                    \
    static bool classof(const Node *V) {                                 \
      return V->getKind() == NodeKind::NAME;                             \
    }                                                                    \
  };

#define ESTREE_NODE_2_ARGS(                                            \
    NAME, BASE, ARG0TY, ARG0NM, ARG0OPT, ARG1TY, ARG1NM, ARG1OPT)      \
  class NAME##Node : public BASE##Node,                                \
                     public detail::DecoratorTrait<NAME##Node>::Type { \
   public:                                                             \
    ARG0TY _##ARG0NM;                                                  \
    ARG1TY _##ARG1NM;                                                  \
    explicit NAME##Node(                                               \
        detail::ParamTrait<ARG0TY>::Type ARG0NM##_,                    \
        detail::ParamTrait<ARG1TY>::Type ARG1NM##_)                    \
        : BASE##Node(NodeKind::NAME),                                  \
          _##ARG0NM(std::move(ARG0NM##_)),                             \
          _##ARG1NM(std::move(ARG1NM##_)) {}                           \
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
        detail::ParamTrait<ARG0TY>::Type ARG0NM##_,                    \
        detail::ParamTrait<ARG1TY>::Type ARG1NM##_,                    \
        detail::ParamTrait<ARG2TY>::Type ARG2NM##_)                    \
        : BASE##Node(NodeKind::NAME),                                  \
          _##ARG0NM(std::move(ARG0NM##_)),                             \
          _##ARG1NM(std::move(ARG1NM##_)),                             \
          _##ARG2NM(std::move(ARG2NM##_)) {}                           \
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
        detail::ParamTrait<ARG0TY>::Type ARG0NM##_,                    \
        detail::ParamTrait<ARG1TY>::Type ARG1NM##_,                    \
        detail::ParamTrait<ARG2TY>::Type ARG2NM##_,                    \
        detail::ParamTrait<ARG3TY>::Type ARG3NM##_)                    \
        : BASE##Node(NodeKind::NAME),                                  \
          _##ARG0NM(std::move(ARG0NM##_)),                             \
          _##ARG1NM(std::move(ARG1NM##_)),                             \
          _##ARG2NM(std::move(ARG2NM##_)),                             \
          _##ARG3NM(std::move(ARG3NM##_)) {}                           \
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
        detail::ParamTrait<ARG0TY>::Type ARG0NM##_,                    \
        detail::ParamTrait<ARG1TY>::Type ARG1NM##_,                    \
        detail::ParamTrait<ARG2TY>::Type ARG2NM##_,                    \
        detail::ParamTrait<ARG3TY>::Type ARG3NM##_,                    \
        detail::ParamTrait<ARG4TY>::Type ARG4NM##_)                    \
        : BASE##Node(NodeKind::NAME),                                  \
          _##ARG0NM(std::move(ARG0NM##_)),                             \
          _##ARG1NM(std::move(ARG1NM##_)),                             \
          _##ARG2NM(std::move(ARG2NM##_)),                             \
          _##ARG3NM(std::move(ARG3NM##_)),                             \
          _##ARG4NM(std::move(ARG4NM##_)) {}                           \
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

#define ESTREE_NODE_6_ARGS(                                            \
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
    ARG4OPT,                                                           \
    ARG5TY,                                                            \
    ARG5NM,                                                            \
    ARG5OPT)                                                           \
  class NAME##Node : public BASE##Node,                                \
                     public detail::DecoratorTrait<NAME##Node>::Type { \
   public:                                                             \
    ARG0TY _##ARG0NM;                                                  \
    ARG1TY _##ARG1NM;                                                  \
    ARG2TY _##ARG2NM;                                                  \
    ARG3TY _##ARG3NM;                                                  \
    ARG4TY _##ARG4NM;                                                  \
    ARG5TY _##ARG5NM;                                                  \
    explicit NAME##Node(                                               \
        detail::ParamTrait<ARG0TY>::Type ARG0NM##_,                    \
        detail::ParamTrait<ARG1TY>::Type ARG1NM##_,                    \
        detail::ParamTrait<ARG2TY>::Type ARG2NM##_,                    \
        detail::ParamTrait<ARG3TY>::Type ARG3NM##_,                    \
        detail::ParamTrait<ARG4TY>::Type ARG4NM##_,                    \
        detail::ParamTrait<ARG5TY>::Type ARG5NM##_)                    \
        : BASE##Node(NodeKind::NAME),                                  \
          _##ARG0NM(std::move(ARG0NM##_)),                             \
          _##ARG1NM(std::move(ARG1NM##_)),                             \
          _##ARG2NM(std::move(ARG2NM##_)),                             \
          _##ARG3NM(std::move(ARG3NM##_)),                             \
          _##ARG4NM(std::move(ARG4NM##_)),                             \
          _##ARG5NM(std::move(ARG5NM##_)) {}                           \
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
      ESTreeVisit(V, _##ARG5NM);                                       \
      V.leave(this);                                                   \
    }                                                                  \
                                                                       \
    static bool classof(const Node *V) {                               \
      return V->getKind() == NodeKind::NAME;                           \
    }                                                                  \
  };

#define ESTREE_NODE_7_ARGS(                                            \
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
    ARG4OPT,                                                           \
    ARG5TY,                                                            \
    ARG5NM,                                                            \
    ARG5OPT,                                                           \
    ARG6TY,                                                            \
    ARG6NM,                                                            \
    ARG6OPT)                                                           \
  class NAME##Node : public BASE##Node,                                \
                     public detail::DecoratorTrait<NAME##Node>::Type { \
   public:                                                             \
    ARG0TY _##ARG0NM;                                                  \
    ARG1TY _##ARG1NM;                                                  \
    ARG2TY _##ARG2NM;                                                  \
    ARG3TY _##ARG3NM;                                                  \
    ARG4TY _##ARG4NM;                                                  \
    ARG5TY _##ARG5NM;                                                  \
    ARG6TY _##ARG6NM;                                                  \
    explicit NAME##Node(                                               \
        detail::ParamTrait<ARG0TY>::Type ARG0NM##_,                    \
        detail::ParamTrait<ARG1TY>::Type ARG1NM##_,                    \
        detail::ParamTrait<ARG2TY>::Type ARG2NM##_,                    \
        detail::ParamTrait<ARG3TY>::Type ARG3NM##_,                    \
        detail::ParamTrait<ARG4TY>::Type ARG4NM##_,                    \
        detail::ParamTrait<ARG5TY>::Type ARG5NM##_,                    \
        detail::ParamTrait<ARG6TY>::Type ARG6NM##_)                    \
        : BASE##Node(NodeKind::NAME),                                  \
          _##ARG0NM(std::move(ARG0NM##_)),                             \
          _##ARG1NM(std::move(ARG1NM##_)),                             \
          _##ARG2NM(std::move(ARG2NM##_)),                             \
          _##ARG3NM(std::move(ARG3NM##_)),                             \
          _##ARG4NM(std::move(ARG4NM##_)),                             \
          _##ARG5NM(std::move(ARG5NM##_)),                             \
          _##ARG6NM(std::move(ARG6NM##_)) {}                           \
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
      ESTreeVisit(V, _##ARG5NM);                                       \
      ESTreeVisit(V, _##ARG6NM);                                       \
      V.leave(this);                                                   \
    }                                                                  \
                                                                       \
    static bool classof(const Node *V) {                               \
      return V->getKind() == NodeKind::NAME;                           \
    }                                                                  \
  };

#define ESTREE_NODE_8_ARGS(                                            \
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
    ARG4OPT,                                                           \
    ARG5TY,                                                            \
    ARG5NM,                                                            \
    ARG5OPT,                                                           \
    ARG6TY,                                                            \
    ARG6NM,                                                            \
    ARG6OPT,                                                           \
    ARG7TY,                                                            \
    ARG7NM,                                                            \
    ARG7OPT)                                                           \
  class NAME##Node : public BASE##Node,                                \
                     public detail::DecoratorTrait<NAME##Node>::Type { \
   public:                                                             \
    ARG0TY _##ARG0NM;                                                  \
    ARG1TY _##ARG1NM;                                                  \
    ARG2TY _##ARG2NM;                                                  \
    ARG3TY _##ARG3NM;                                                  \
    ARG4TY _##ARG4NM;                                                  \
    ARG5TY _##ARG5NM;                                                  \
    ARG6TY _##ARG6NM;                                                  \
    ARG7TY _##ARG7NM;                                                  \
    explicit NAME##Node(                                               \
        detail::ParamTrait<ARG0TY>::Type ARG0NM##_,                    \
        detail::ParamTrait<ARG1TY>::Type ARG1NM##_,                    \
        detail::ParamTrait<ARG2TY>::Type ARG2NM##_,                    \
        detail::ParamTrait<ARG3TY>::Type ARG3NM##_,                    \
        detail::ParamTrait<ARG4TY>::Type ARG4NM##_,                    \
        detail::ParamTrait<ARG5TY>::Type ARG5NM##_,                    \
        detail::ParamTrait<ARG6TY>::Type ARG6NM##_,                    \
        detail::ParamTrait<ARG7TY>::Type ARG7NM##_)                    \
        : BASE##Node(NodeKind::NAME),                                  \
          _##ARG0NM(std::move(ARG0NM##_)),                             \
          _##ARG1NM(std::move(ARG1NM##_)),                             \
          _##ARG2NM(std::move(ARG2NM##_)),                             \
          _##ARG3NM(std::move(ARG3NM##_)),                             \
          _##ARG4NM(std::move(ARG4NM##_)),                             \
          _##ARG5NM(std::move(ARG5NM##_)),                             \
          _##ARG6NM(std::move(ARG6NM##_)),                             \
          _##ARG7NM(std::move(ARG7NM##_)) {}                           \
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
      ESTreeVisit(V, _##ARG5NM);                                       \
      ESTreeVisit(V, _##ARG6NM);                                       \
      ESTreeVisit(V, _##ARG7NM);                                       \
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
#define ESTREE_NODE_6_ARGS(NAME, ...) \
  case NodeKind::NAME:                \
    return cast<NAME##Node>(Node)->visit(V);
#define ESTREE_NODE_7_ARGS(NAME, ...) \
  case NodeKind::NAME:                \
    return cast<NAME##Node>(Node)->visit(V);
#define ESTREE_NODE_8_ARGS(NAME, ...) \
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

/// \return the object of the member expression node.
Node *getObject(MemberExpressionLikeNode *node);

/// \return the property of the member expression node.
Node *getProperty(MemberExpressionLikeNode *node);

/// \return whether the member expression node is computed.
NodeBoolean getComputed(MemberExpressionLikeNode *node);

/// \return the callee of the call.
Node *getCallee(CallExpressionLikeNode *node);

/// \return the arguments list of the call.
NodeList &getArguments(CallExpressionLikeNode *node);

/// \return true when \p node has simple params, i.e. no destructuring and no
/// initializers.
bool hasSimpleParams(FunctionLikeNode *node);

/// \return true when \p node is a generator function.
bool isGenerator(FunctionLikeNode *node);

/// \return true when \p node is an async function.
bool isAsync(FunctionLikeNode *node);

/// Allow using \p NodeKind in \p llvh::DenseMaps.
struct NodeKindInfo : llvh::DenseMapInfo<NodeKind> {
  static inline NodeKind getEmptyKey() {
    return (NodeKind)(-1);
  }
  static inline NodeKind getTombstoneKey() {
    return (NodeKind)(-2);
  }
  static inline bool isEqual(const NodeKind &a, const NodeKind &b) {
    return a == b;
  }
  static unsigned getHashValue(const NodeKind &Val) {
    return (unsigned)Val;
  }
};

using NodeKindSet = llvh::DenseSet<ESTree::NodeKind, NodeKindInfo>;

/// An arbitrary limit to nested assignments. We handle them non-recursively, so
/// this can be very large, but we don't want to let it consume all our memory.
constexpr unsigned MAX_NESTED_ASSIGNMENTS = 30000;

/// An arbitrary limit to nested "+/-" binary expressions. We handle them
/// non-recursively, so this can be very large, but we don't want to let it
/// consume all our memory.
constexpr unsigned MAX_NESTED_BINARY = 30000;

/// Check if an AST node is of the specified type and its `_operator`
/// attribute is within the set of allowed operators.
template <class N>
static N *checkExprOperator(ESTree::Node *e, llvh::ArrayRef<StringRef> ops) {
  if (auto *n = llvh::dyn_cast<N>(e)) {
    if (std::find(ops.begin(), ops.end(), n->_operator->str()) != ops.end())
      return n;
  }
  return nullptr;
}

/// Convert a recursive expression of the form ((a + b) + c) + d) into a list
/// `a, b, c, d`. This description of the list is for exposition purposes, but
/// the actual list contains pointers to each binop node:
///    `list = [(a + b), (list[0] + c), (list[1] + d)`.
/// Note that the list is only three elements long and the first element is
/// accessible through the `_left` pointer of `list[0]`.
///
/// \param ops - the acceptable values for the `_operator` attribute of the
///     expression. Ideally it should contain all operators with the same
///     precedence: ["+", "-"] or ["*", "/", "%"], etc.
template <class N>
static llvh::SmallVector<N *, 1> linearizeLeft(
    N *e,
    llvh::ArrayRef<StringRef> ops) {
  llvh::SmallVector<N *, 1> vec;

  vec.push_back(e);
  while (auto *left = checkExprOperator<N>(e->_left, ops)) {
    e = left;
    vec.push_back(e);
  }

  std::reverse(vec.begin(), vec.end());
  return vec;
}

/// Convert a recursive expression of the form (a = (b = (c = d))) into a list
/// `a, b, c, d`. This description of the list is for exposition purposes, but
/// the actual list contains pointers to each node:
///    `list = [(a = list[1]), (b = list[2]), (c = d)]`.
/// Note that the list is only three elements long and the last element is
/// accessible through the `_right` pointer of `list[2]`.
///
/// \param ops - the acceptable values for the `_operator` attribute of the
///     expression. Ideally it should contain all operators with the same
///     precedence, but can also be a single operator like ["="], if the caller
///     doesn't want to deal with the complexity.
template <class N>
static llvh::SmallVector<N *, 1> linearizeRight(
    N *e,
    llvh::ArrayRef<StringRef> ops) {
  llvh::SmallVector<N *, 1> vec;

  vec.push_back(e);
  while (auto *right = checkExprOperator<N>(e->_right, ops)) {
    e = right;
    vec.push_back(e);
  }

  return vec;
}

} // namespace ESTree
} // namespace hermes

#endif
