/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/AST/ESTreeDumper.h"

#include "RecursiveVisitor.h"

namespace hermes {

namespace {

using namespace hermes::ESTree;

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, NodePtr &node) {
  return os << "<child>";
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, NodeList &node) {
  return os << "<children>";
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, NodeLabel &label) {
  if (label)
    os << '"' << label->str() << '"';
  return os;
}

class ESTreeDumper {
  llvm::raw_ostream &os_;

  uint32_t indentation_{0};

 public:
  explicit ESTreeDumper(llvm::raw_ostream &os) : os_(os) {}

  void doIt(NodePtr rootNode) {
    visitESTreeNode(*this, rootNode);
  }

#define ESTREE_NODE_0_ARGS(NAME)               \
  void visit(NAME##Node *node) {               \
    os_.indent(indentation_) << #NAME << '\n'; \
    dumpChildren(node);                        \
  }

#define ESTREE_NODE_1_ARGS(NAME, ARG0TY, ARG0NM, ARG0OPT)                \
  void visit(NAME##Node *node) {                                         \
    os_.indent(indentation_) << #NAME << " " << node->_##ARG0NM << '\n'; \
    dumpChildren(node);                                                  \
  }

#define ESTREE_NODE_2_ARGS(                                                   \
    NAME, ARG0TY, ARG0NM, ARG0OPT, ARG1TY, ARG1NM, ARG1OPT)                   \
  void visit(NAME##Node *node) {                                              \
    os_.indent(indentation_)                                                  \
        << #NAME << " " << node->_##ARG0NM << " " << node->_##ARG1NM << '\n'; \
    dumpChildren(node);                                                       \
  }

#define ESTREE_NODE_3_ARGS(                                                 \
    NAME,                                                                   \
    ARG0TY,                                                                 \
    ARG0NM,                                                                 \
    ARG0OPT,                                                                \
    ARG1TY,                                                                 \
    ARG1NM,                                                                 \
    ARG1OPT,                                                                \
    ARG2TY,                                                                 \
    ARG2NM,                                                                 \
    ARG2OPT)                                                                \
  void visit(NAME##Node *node) {                                            \
    os_.indent(indentation_)                                                \
        << #NAME << " " << node->_##ARG0NM << " " << node->_##ARG1NM << " " \
        << node->_##ARG2NM << '\n';                                         \
    dumpChildren(node);                                                     \
  }

#define ESTREE_NODE_4_ARGS(                                                 \
    NAME,                                                                   \
    ARG0TY,                                                                 \
    ARG0NM,                                                                 \
    ARG0OPT,                                                                \
    ARG1TY,                                                                 \
    ARG1NM,                                                                 \
    ARG1OPT,                                                                \
    ARG2TY,                                                                 \
    ARG2NM,                                                                 \
    ARG2OPT,                                                                \
    ARG3TY,                                                                 \
    ARG3NM,                                                                 \
    ARG3OPT)                                                                \
  void visit(NAME##Node *node) {                                            \
    os_.indent(indentation_)                                                \
        << #NAME << " " << node->_##ARG0NM << " " << node->_##ARG1NM << " " \
        << node->_##ARG2NM << '\n';                                         \
    dumpChildren(node);                                                     \
  }

#include "hermes/AST/ESTree.def"

 private:
  /// Dump the children of the given node at a level of indentation
  /// one more than the current level.
  void dumpChildren(NodePtr node) {
    const uint32_t kIndentationWidth = 2;

    indentation_ += kIndentationWidth;
    visitESTreeChildren(*this, node);
    indentation_ -= kIndentationWidth;
  }
};

} // namespace

void dumpESTree(llvm::raw_ostream &os, NodePtr rootNode) {
  return ESTreeDumper(os).doIt(rootNode);
}

} // namespace hermes
