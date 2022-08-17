/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/ESTreeJSONDumper.h"

#include "hermes/Support/JSONEmitter.h"
#include "llvh/ADT/StringMap.h"
#include "llvh/ADT/StringSet.h"
#include "llvh/Support/MemoryBuffer.h"

namespace hermes {

namespace {

using namespace hermes::ESTree;

class ESTreeJSONDumper {
  JSONEmitter &json_;
  SourceErrorManager *sm_;
  ESTreeDumpMode mode_;
  LocationDumpMode locMode_;
  /// Whether to include or exclude the "raw" property where available.
  ESTreeRawProp const rawProp_;

  /// A collection of fields to ignore if they are empty (null or []).
  /// Mapping from node name to a set of ignored field names for that node.
  llvh::StringMap<llvh::StringSet<>> ignoredEmptyFields_{};

  /// If null, this is ignored.
  /// If non-null, only print the source locations for kinds in this set.
  const NodeKindSet *includeSourceLocs_;

 public:
  explicit ESTreeJSONDumper(
      JSONEmitter &json,
      SourceErrorManager *sm,
      ESTreeDumpMode mode,
      LocationDumpMode locMode,
      ESTreeRawProp rawProp = ESTreeRawProp::Include,
      const NodeKindSet *includeSourceLocs = nullptr)
      : json_(json),
        sm_(sm),
        mode_(mode),
        locMode_(locMode),
        rawProp_(rawProp),
        includeSourceLocs_(includeSourceLocs) {
    if (locMode != LocationDumpMode::None) {
      assert(sm && "SourceErrorManager required for dumping");
    }

    if (mode == ESTreeDumpMode::HideEmpty) {
#define ESTREE_NODE_0_ARGS(NAME, ...)
#define ESTREE_NODE_1_ARGS(NAME, ...)
#define ESTREE_NODE_2_ARGS(NAME, ...)
#define ESTREE_NODE_3_ARGS(NAME, ...)
#define ESTREE_NODE_4_ARGS(NAME, ...)
#define ESTREE_NODE_5_ARGS(NAME, ...)
#define ESTREE_NODE_6_ARGS(NAME, ...)
#define ESTREE_NODE_7_ARGS(NAME, ...)
#define ESTREE_NODE_8_ARGS(NAME, ...)
#define ESTREE_IGNORE_IF_EMPTY(NAME, FIELD)        \
  assert(                                          \
      !ignoredEmptyFields_[#NAME].count(#FIELD) && \
      "duplicate ignored fields");                 \
  ignoredEmptyFields_[#NAME].insert(#FIELD);
#include "hermes/AST/ESTree.def"
    }
  }

  void doIt(NodePtr rootNode) {
    dumpNode(rootNode);
  }

 private:
  /// Print the source location for the \p node.
  void printSourceLocation(Node *node) {
    if (locMode_ == LocationDumpMode::None)
      return;
    if (includeSourceLocs_ && !includeSourceLocs_->count(node->getKind()))
      return;

    SourceErrorManager::SourceCoords start, end;
    SMRange rng = node->getSourceRange();
    if (!sm_->findBufferLineAndLoc(rng.Start, start) ||
        !sm_->findBufferLineAndLoc(rng.End, end))
      return;

    if (locMode_ == LocationDumpMode::Loc ||
        locMode_ == LocationDumpMode::LocAndRange) {
      json_.emitKey("loc");
      json_.openDict();
      json_.emitKey("start");
      json_.openDict();
      json_.emitKeyValue("line", start.line);
      json_.emitKeyValue("column", start.col);
      json_.closeDict();
      json_.emitKey("end");
      json_.openDict();
      json_.emitKeyValue("line", end.line);
      json_.emitKeyValue("column", end.col);
      json_.closeDict();
      json_.closeDict();
    }

    if (locMode_ == LocationDumpMode::Range ||
        locMode_ == LocationDumpMode::LocAndRange) {
      json_.emitKey("range");
      json_.openArray();
      dumpSMRangeJSON(json_, rng, sm_->findBufferForLoc(rng.Start));
      json_.closeArray();
    }
  }

  void visit(Node *node, llvh::StringRef type) {
    json_.openDict();
    json_.emitKeyValue("type", type);
    dumpChildren(node);
    printSourceLocation(node);
    json_.closeDict();
  }

  void visit(NumericLiteralNode *node, llvh::StringRef type) {
    json_.openDict();
    json_.emitKeyValue("type", type);
    dumpChildren(node);
    SMRange sr = node->getSourceRange();
    if (sr.isValid() && rawProp_ == ESTreeRawProp::Include) {
      json_.emitKeyValue(
          "raw",
          llvh::StringRef{
              sr.Start.getPointer(),
              (size_t)(sr.End.getPointer() - sr.Start.getPointer())});
    }
    printSourceLocation(node);
    json_.closeDict();
  }

  static bool isEmpty(NodeList &list) {
    return list.empty();
  }

  static bool isEmpty(NodeLabel label) {
    return false;
  }

  static bool isEmpty(NodeBoolean val) {
    return !val;
  }

  static bool isEmpty(NodeNumber num) {
    return false;
  }

  static bool isEmpty(NodePtr node) {
    return node == nullptr;
  }

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
    return visit(cast<NAME##Node>(node), #NAME);

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
#define ESTREE_NODE_6_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_7_ARGS(NAME, ...) VISIT(NAME)
#define ESTREE_NODE_8_ARGS(NAME, ...) VISIT(NAME)

#include "hermes/AST/ESTree.def"

#undef VISIT
    }
  }

#define DUMP_KEY_VALUE_PAIR(PARENT, KEY, NODE)       \
  do {                                               \
    if (isEmpty(NODE)) {                             \
      if (mode_ == ESTreeDumpMode::Compact) {        \
        break;                                       \
      }                                              \
      if (mode_ == ESTreeDumpMode::HideEmpty) {      \
        auto it = ignoredEmptyFields_.find(#PARENT); \
        if (it != ignoredEmptyFields_.end()) {       \
          if (it->second.count(KEY)) {               \
            break;                                   \
          }                                          \
        }                                            \
      }                                              \
    }                                                \
    json_.emitKey(KEY);                              \
    dumpNode(NODE);                                  \
  } while (0);

/// Declare helper functions to recursively visit the children of a node.
#define ESTREE_NODE_0_ARGS(NAME, BASE) \
  void visitChildren(NAME##Node *) {}

#define ESTREE_NODE_1_ARGS(NAME, BASE, ARG0TY, ARG0NM, ARG0OPT) \
  void visitChildren(NAME##Node *node) {                        \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG0NM, node->_##ARG0NM)         \
  }

#define ESTREE_NODE_2_ARGS(                                       \
    NAME, BASE, ARG0TY, ARG0NM, ARG0OPT, ARG1TY, ARG1NM, ARG1OPT) \
  void visitChildren(NAME##Node *node) {                          \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG0NM, node->_##ARG0NM)           \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG1NM, node->_##ARG1NM)           \
  }

#define ESTREE_NODE_3_ARGS(                             \
    NAME,                                               \
    BASE,                                               \
    ARG0TY,                                             \
    ARG0NM,                                             \
    ARG0OPT,                                            \
    ARG1TY,                                             \
    ARG1NM,                                             \
    ARG1OPT,                                            \
    ARG2TY,                                             \
    ARG2NM,                                             \
    ARG2OPT)                                            \
  void visitChildren(NAME##Node *node) {                \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG0NM, node->_##ARG0NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG1NM, node->_##ARG1NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG2NM, node->_##ARG2NM) \
  }

#define ESTREE_NODE_4_ARGS(                             \
    NAME,                                               \
    BASE,                                               \
    ARG0TY,                                             \
    ARG0NM,                                             \
    ARG0OPT,                                            \
    ARG1TY,                                             \
    ARG1NM,                                             \
    ARG1OPT,                                            \
    ARG2TY,                                             \
    ARG2NM,                                             \
    ARG2OPT,                                            \
    ARG3TY,                                             \
    ARG3NM,                                             \
    ARG3OPT)                                            \
  void visitChildren(NAME##Node *node) {                \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG0NM, node->_##ARG0NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG1NM, node->_##ARG1NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG2NM, node->_##ARG2NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG3NM, node->_##ARG3NM) \
  }

#define ESTREE_NODE_5_ARGS(                             \
    NAME,                                               \
    BASE,                                               \
    ARG0TY,                                             \
    ARG0NM,                                             \
    ARG0OPT,                                            \
    ARG1TY,                                             \
    ARG1NM,                                             \
    ARG1OPT,                                            \
    ARG2TY,                                             \
    ARG2NM,                                             \
    ARG2OPT,                                            \
    ARG3TY,                                             \
    ARG3NM,                                             \
    ARG3OPT,                                            \
    ARG4TY,                                             \
    ARG4NM,                                             \
    ARG4OPT)                                            \
  void visitChildren(NAME##Node *node) {                \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG0NM, node->_##ARG0NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG1NM, node->_##ARG1NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG2NM, node->_##ARG2NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG3NM, node->_##ARG3NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG4NM, node->_##ARG4NM) \
  }

#define ESTREE_NODE_6_ARGS(                             \
    NAME,                                               \
    BASE,                                               \
    ARG0TY,                                             \
    ARG0NM,                                             \
    ARG0OPT,                                            \
    ARG1TY,                                             \
    ARG1NM,                                             \
    ARG1OPT,                                            \
    ARG2TY,                                             \
    ARG2NM,                                             \
    ARG2OPT,                                            \
    ARG3TY,                                             \
    ARG3NM,                                             \
    ARG3OPT,                                            \
    ARG4TY,                                             \
    ARG4NM,                                             \
    ARG4OPT,                                            \
    ARG5TY,                                             \
    ARG5NM,                                             \
    ARG5OPT)                                            \
  void visitChildren(NAME##Node *node) {                \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG0NM, node->_##ARG0NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG1NM, node->_##ARG1NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG2NM, node->_##ARG2NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG3NM, node->_##ARG3NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG4NM, node->_##ARG4NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG5NM, node->_##ARG5NM) \
  }

#define ESTREE_NODE_7_ARGS(                             \
    NAME,                                               \
    BASE,                                               \
    ARG0TY,                                             \
    ARG0NM,                                             \
    ARG0OPT,                                            \
    ARG1TY,                                             \
    ARG1NM,                                             \
    ARG1OPT,                                            \
    ARG2TY,                                             \
    ARG2NM,                                             \
    ARG2OPT,                                            \
    ARG3TY,                                             \
    ARG3NM,                                             \
    ARG3OPT,                                            \
    ARG4TY,                                             \
    ARG4NM,                                             \
    ARG4OPT,                                            \
    ARG5TY,                                             \
    ARG5NM,                                             \
    ARG5OPT,                                            \
    ARG6TY,                                             \
    ARG6NM,                                             \
    ARG6OPT)                                            \
  void visitChildren(NAME##Node *node) {                \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG0NM, node->_##ARG0NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG1NM, node->_##ARG1NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG2NM, node->_##ARG2NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG3NM, node->_##ARG3NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG4NM, node->_##ARG4NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG5NM, node->_##ARG5NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG6NM, node->_##ARG6NM) \
  }

#define ESTREE_NODE_8_ARGS(                             \
    NAME,                                               \
    BASE,                                               \
    ARG0TY,                                             \
    ARG0NM,                                             \
    ARG0OPT,                                            \
    ARG1TY,                                             \
    ARG1NM,                                             \
    ARG1OPT,                                            \
    ARG2TY,                                             \
    ARG2NM,                                             \
    ARG2OPT,                                            \
    ARG3TY,                                             \
    ARG3NM,                                             \
    ARG3OPT,                                            \
    ARG4TY,                                             \
    ARG4NM,                                             \
    ARG4OPT,                                            \
    ARG5TY,                                             \
    ARG5NM,                                             \
    ARG5OPT,                                            \
    ARG6TY,                                             \
    ARG6NM,                                             \
    ARG6OPT,                                            \
    ARG7TY,                                             \
    ARG7NM,                                             \
    ARG7OPT)                                            \
  void visitChildren(NAME##Node *node) {                \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG0NM, node->_##ARG0NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG1NM, node->_##ARG1NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG2NM, node->_##ARG2NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG3NM, node->_##ARG3NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG4NM, node->_##ARG4NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG5NM, node->_##ARG5NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG6NM, node->_##ARG6NM) \
    DUMP_KEY_VALUE_PAIR(NAME, #ARG7NM, node->_##ARG7NM) \
  }

#include "hermes/AST/ESTree.def"

#undef DUMP_KEY_VALUE_PAIR
}; // namespace

} // namespace

void dumpSMRangeJSON(
    JSONEmitter &json,
    llvh::SMRange rng,
    const llvh::MemoryBuffer *buffer) {
  assert(buffer && "The buffer must exist");
  const char *bufStart = buffer->getBufferStart();
  assert(
      rng.Start.getPointer() >= bufStart &&
      rng.End.getPointer() <= buffer->getBufferEnd() &&
      "The range must be within the buffer");
  json.emitValues(
      {rng.Start.getPointer() - bufStart, rng.End.getPointer() - bufStart});
}

void dumpESTreeJSON(
    llvh::raw_ostream &os,
    NodePtr rootNode,
    bool pretty,
    ESTreeDumpMode mode) {
  JSONEmitter json{os, pretty};
  ESTreeJSONDumper(json, nullptr, mode, LocationDumpMode::None).doIt(rootNode);
  json.endJSONL();
}

void dumpESTreeJSON(
    llvh::raw_ostream &os,
    NodePtr rootNode,
    bool pretty,
    ESTreeDumpMode mode,
    SourceErrorManager &sm,
    LocationDumpMode locMode,
    ESTreeRawProp rawProp) {
  JSONEmitter json{os, pretty};
  ESTreeJSONDumper(json, &sm, mode, locMode, rawProp).doIt(rootNode);
  json.endJSONL();
}

void dumpESTreeJSON(
    JSONEmitter &json,
    NodePtr rootNode,
    ESTreeDumpMode mode,
    SourceErrorManager &sm,
    LocationDumpMode locMode,
    const NodeKindSet *includeSourceLocs,
    ESTreeRawProp rawProp) {
  ESTreeJSONDumper(json, &sm, mode, locMode, rawProp, includeSourceLocs)
      .doIt(rootNode);
}

} // namespace hermes
