/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/AST/ESTreeJSONDumper.h"

#include "hermes/Support/JSONEmitter.h"

namespace hermes {

namespace {

using namespace hermes::ESTree;

class ESTreeJSONDumper {
  JSONEmitter json_;

 public:
  explicit ESTreeJSONDumper(llvm::raw_ostream &os, bool pretty)
      : json_(os, pretty) {}

  void doIt(NodePtr rootNode) {
    dumpNode(rootNode);
    json_.endJSONL();
  }

 private:
/// Visitor functions for each node type.
#define ESTREE_NODE_0_ARGS(NAME, BASE) \
  void visit(NAME##Node *node) {       \
    json_.openDict();                  \
    json_.emitKeyValue("type", #NAME); \
    dumpChildren(node);                \
    json_.closeDict();                 \
  }

#define ESTREE_NODE_1_ARGS(NAME, BASE, ARG0TY, ARG0NM, ARG0OPT) \
  void visit(NAME##Node *node) {                                \
    json_.openDict();                                           \
    json_.emitKeyValue("type", #NAME);                          \
    dumpChildren(node);                                         \
    json_.closeDict();                                          \
  }

#define ESTREE_NODE_2_ARGS(                                       \
    NAME, BASE, ARG0TY, ARG0NM, ARG0OPT, ARG1TY, ARG1NM, ARG1OPT) \
  void visit(NAME##Node *node) {                                  \
    json_.openDict();                                             \
    json_.emitKeyValue("type", #NAME);                            \
    dumpChildren(node);                                           \
    json_.closeDict();                                            \
  }

#define ESTREE_NODE_3_ARGS(            \
    NAME,                              \
    BASE,                              \
    ARG0TY,                            \
    ARG0NM,                            \
    ARG0OPT,                           \
    ARG1TY,                            \
    ARG1NM,                            \
    ARG1OPT,                           \
    ARG2TY,                            \
    ARG2NM,                            \
    ARG2OPT)                           \
  void visit(NAME##Node *node) {       \
    json_.openDict();                  \
    json_.emitKeyValue("type", #NAME); \
    dumpChildren(node);                \
    json_.closeDict();                 \
  }

#define ESTREE_NODE_4_ARGS(            \
    NAME,                              \
    BASE,                              \
    ARG0TY,                            \
    ARG0NM,                            \
    ARG0OPT,                           \
    ARG1TY,                            \
    ARG1NM,                            \
    ARG1OPT,                           \
    ARG2TY,                            \
    ARG2NM,                            \
    ARG2OPT,                           \
    ARG3TY,                            \
    ARG3NM,                            \
    ARG3OPT)                           \
  void visit(NAME##Node *node) {       \
    json_.openDict();                  \
    json_.emitKeyValue("type", #NAME); \
    dumpChildren(node);                \
    json_.closeDict();                 \
  }

#define ESTREE_NODE_5_ARGS(            \
    NAME,                              \
    BASE,                              \
    ARG0TY,                            \
    ARG0NM,                            \
    ARG0OPT,                           \
    ARG1TY,                            \
    ARG1NM,                            \
    ARG1OPT,                           \
    ARG2TY,                            \
    ARG2NM,                            \
    ARG2OPT,                           \
    ARG3TY,                            \
    ARG3NM,                            \
    ARG3OPT,                           \
    ARG4TY,                            \
    ARG4NM,                            \
    ARG4OPT)                           \
  void visit(NAME##Node *node) {       \
    json_.openDict();                  \
    json_.emitKeyValue("type", #NAME); \
    dumpChildren(node);                \
    json_.closeDict();                 \
  }

#include "hermes/AST/ESTree.def"

  void dumpNode(NodeList &list) {
    json_.openArray();
    for (auto &node : list) {
      dumpNode(&node);
    }
    json_.closeArray();
  }

  void dumpNode(NodeLabel label) {
    if (label) {
      json_.emitValue(label->str());
    } else {
      json_.emitNullValue();
    }
  }

  void dumpNode(NodeBoolean val) {
    json_.emitValue(val);
  }

  void dumpNode(NodeNumber num) {
    json_.emitValue(num);
  }

  /// Selects the correct visit function based on node type.
  void dumpNode(NodePtr node) {
    if (!node) {
      json_.emitNullValue();
      return;
    }

    switch (node->getKind()) {
      default:
        llvm_unreachable("invalid node kind");

#define VISIT(NAME)    \
  case NodeKind::NAME: \
    return visit(cast<NAME##Node>(node));

#define ESTREE_NODE_0_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_1_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_2_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_3_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_4_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_5_ARGS(NAME, ...) VISIT(NAME)

#include "hermes/AST/ESTree.def"

#undef VISIT
    }
  }

  /// Selects the correct function to dump the children nodes of an AST node.
  void dumpChildren(NodePtr node) {
    if (!node)
      return;

    switch (node->getKind()) {
      default:
        llvm_unreachable("invalid node kind");

#define VISIT(NAME)    \
  case NodeKind::NAME: \
    return visitChildren(cast<NAME##Node>(node));

#define ESTREE_NODE_0_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_1_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_2_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_3_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_4_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_5_ARGS(NAME, ...) VISIT(NAME)

#include "hermes/AST/ESTree.def"

#undef VISIT
    }
  }

#define DUMP_KEY_VALUE_PAIR(KEY, NODE) \
  json_.emitKey(KEY);                  \
  dumpNode(NODE);

/// Declare helper functions to recursively visit the children of a node.
#define ESTREE_NODE_0_ARGS(NAME, BASE) \
  void visitChildren(NAME##Node *) {}

#define ESTREE_NODE_1_ARGS(NAME, BASE, ARG0TY, ARG0NM, ARG0OPT) \
  void visitChildren(NAME##Node *node) {                        \
    DUMP_KEY_VALUE_PAIR(#ARG0NM, node->_##ARG0NM)               \
  }

#define ESTREE_NODE_2_ARGS(                                       \
    NAME, BASE, ARG0TY, ARG0NM, ARG0OPT, ARG1TY, ARG1NM, ARG1OPT) \
  void visitChildren(NAME##Node *node) {                          \
    DUMP_KEY_VALUE_PAIR(#ARG0NM, node->_##ARG0NM)                 \
    DUMP_KEY_VALUE_PAIR(#ARG1NM, node->_##ARG1NM)                 \
  }

#define ESTREE_NODE_3_ARGS(                       \
    NAME,                                         \
    BASE,                                         \
    ARG0TY,                                       \
    ARG0NM,                                       \
    ARG0OPT,                                      \
    ARG1TY,                                       \
    ARG1NM,                                       \
    ARG1OPT,                                      \
    ARG2TY,                                       \
    ARG2NM,                                       \
    ARG2OPT)                                      \
  void visitChildren(NAME##Node *node) {          \
    DUMP_KEY_VALUE_PAIR(#ARG0NM, node->_##ARG0NM) \
    DUMP_KEY_VALUE_PAIR(#ARG1NM, node->_##ARG1NM) \
    DUMP_KEY_VALUE_PAIR(#ARG2NM, node->_##ARG2NM) \
  }

#define ESTREE_NODE_4_ARGS(                       \
    NAME,                                         \
    BASE,                                         \
    ARG0TY,                                       \
    ARG0NM,                                       \
    ARG0OPT,                                      \
    ARG1TY,                                       \
    ARG1NM,                                       \
    ARG1OPT,                                      \
    ARG2TY,                                       \
    ARG2NM,                                       \
    ARG2OPT,                                      \
    ARG3TY,                                       \
    ARG3NM,                                       \
    ARG3OPT)                                      \
  void visitChildren(NAME##Node *node) {          \
    DUMP_KEY_VALUE_PAIR(#ARG0NM, node->_##ARG0NM) \
    DUMP_KEY_VALUE_PAIR(#ARG1NM, node->_##ARG1NM) \
    DUMP_KEY_VALUE_PAIR(#ARG2NM, node->_##ARG2NM) \
    DUMP_KEY_VALUE_PAIR(#ARG3NM, node->_##ARG3NM) \
  }

#define ESTREE_NODE_5_ARGS(                       \
    NAME,                                         \
    BASE,                                         \
    ARG0TY,                                       \
    ARG0NM,                                       \
    ARG0OPT,                                      \
    ARG1TY,                                       \
    ARG1NM,                                       \
    ARG1OPT,                                      \
    ARG2TY,                                       \
    ARG2NM,                                       \
    ARG2OPT,                                      \
    ARG3TY,                                       \
    ARG3NM,                                       \
    ARG3OPT,                                      \
    ARG4TY,                                       \
    ARG4NM,                                       \
    ARG4OPT)                                      \
  void visitChildren(NAME##Node *node) {          \
    DUMP_KEY_VALUE_PAIR(#ARG0NM, node->_##ARG0NM) \
    DUMP_KEY_VALUE_PAIR(#ARG1NM, node->_##ARG1NM) \
    DUMP_KEY_VALUE_PAIR(#ARG2NM, node->_##ARG2NM) \
    DUMP_KEY_VALUE_PAIR(#ARG3NM, node->_##ARG3NM) \
    DUMP_KEY_VALUE_PAIR(#ARG4NM, node->_##ARG4NM) \
  }

#include "hermes/AST/ESTree.def"

#undef DUMP_KEY_VALUE_PAIR
};

} // namespace

void dumpESTreeJSON(llvm::raw_ostream &os, NodePtr rootNode, bool pretty) {
  return ESTreeJSONDumper(os, pretty).doIt(rootNode);
}

} // namespace hermes
