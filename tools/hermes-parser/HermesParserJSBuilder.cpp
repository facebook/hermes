/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "llvh/Support/MemoryBuffer.h"

#include "HermesParserJSBuilder.h"
#include "HermesParserJSLibrary.h"

namespace hermes {

using namespace hermes::ESTree;

class HermesParserJSBuilder {
  SourceErrorManager *sm_{nullptr};
  parser::JSParser *parser_{nullptr};

 public:
  explicit HermesParserJSBuilder(
      SourceErrorManager *sm,
      parser::JSParser *parser)
      : sm_(sm), parser_(parser) {}

  JSReference buildProgram(ProgramNode *programNode) {
    auto loc = buildLoc(programNode);
    auto body = buildNode(programNode->_body);
    auto comments = buildComments();

    return buildProgramWithComments(loc, body, comments);
  }

 private:
  /// Booleans can be passed directly to a JS library call.
  bool buildNode(NodeBoolean val) {
    return val;
  }

  /// Numbers can be passed directly to a JS library call.
  double buildNode(NodeNumber num) {
    return num;
  }

  /// Pointers to null-terminated strings can be passed directly to a JS library
  /// call. The caller in JS must then read the null-terminated string off the
  /// WASM heap.
  const char *buildNode(NodeLabel label) {
    if (label == nullptr) {
      return nullptr;
    }

    return label->c_str();
  }

  /// Arrays cannot be passed directly to a JS library call. Instead, an array
  /// is created in JS and a reference to it is returned from this function.
  JSReference buildNode(NodeList &list) {
    JSReference arrayReference = buildArray();
    for (auto &node : list) {
      appendToArray(arrayReference, buildNode(&node));
    }

    return arrayReference;
  }

  /// Build a source location in JS, returning a JS reference to the source
  /// location object.
  JSReference buildLoc(SMRange rng) {
    SourceErrorManager::SourceCoords start, end;
    if (!sm_->findBufferLineAndLoc(rng.Start, start) ||
        !sm_->findBufferLineAndLoc(rng.End, end))
      return 0;

    const llvh::MemoryBuffer *buffer = sm_->getSourceBuffer(start.bufId);
    assert(buffer && "The buffer must exist");
    const char *bufStart = buffer->getBufferStart();
    assert(
        rng.Start.getPointer() >= bufStart &&
        rng.End.getPointer() <= buffer->getBufferEnd() &&
        "The range must be within the buffer");

    return buildSourceLocation(
        start.line,
        start.col - 1,
        end.line,
        end.col - 1,
        rng.Start.getPointer() - bufStart,
        rng.End.getPointer() - bufStart);
  }

  JSReference buildLoc(Node *node) {
    return buildLoc(node->getSourceRange());
  }

  /// Build an array in JS of all comments in the source file.
  JSReference buildComments() {
    JSReference commentsReference = buildArray();

    for (auto &storedComment : parser_->getStoredComments()) {
      auto loc = buildLoc(storedComment.getSourceRange());

      // Comment values not null terminated so pass both their pointer and size
      auto value = storedComment.getString();
      auto commentReference = buildComment(
          loc, (int)storedComment.getKind(), value.data(), value.size());

      appendToArray(commentsReference, commentReference);
    }

    return commentsReference;
  }

  /// Objects cannot be passed directly to a JS library call. Instead, the
  /// object is created in JS and a reference to it is returned from this
  /// function.
  ///
  /// This function selects the correct create function based on node type.
  JSReference buildNode(NodePtr node) {
    if (!node) {
      return 0;
    }

    switch (node->getKind()) {
      default:
        llvm_unreachable("invalid node kind");

#define BUILD(NAME)    \
  case NodeKind::NAME: \
    return build(cast<NAME##Node>(node));

#define ESTREE_NODE_0_ARGS(NAME, ...) BUILD(NAME)
#define ESTREE_NODE_1_ARGS(NAME, ...) BUILD(NAME)
#define ESTREE_NODE_2_ARGS(NAME, ...) BUILD(NAME)
#define ESTREE_NODE_3_ARGS(NAME, ...) BUILD(NAME)
#define ESTREE_NODE_4_ARGS(NAME, ...) BUILD(NAME)
#define ESTREE_NODE_5_ARGS(NAME, ...) BUILD(NAME)
#define ESTREE_NODE_6_ARGS(NAME, ...) BUILD(NAME)
#define ESTREE_NODE_7_ARGS(NAME, ...) BUILD(NAME)
#define ESTREE_NODE_8_ARGS(NAME, ...) BUILD(NAME)

#include "hermes/AST/ESTree.def"

#undef BUILD
    }
  }

#define ESTREE_NODE_0_ARGS(NAME, BASE)   \
  JSReference build(NAME##Node *node) {  \
    return build_##NAME(buildLoc(node)); \
  }

#define ESTREE_NODE_1_ARGS(NAME, BASE, ARG0TY, ARG0NM, ARG0OPT)      \
  JSReference build(NAME##Node *node) {                              \
    return build_##NAME(buildLoc(node), buildNode(node->_##ARG0NM)); \
  }

#define ESTREE_NODE_2_ARGS(                                       \
    NAME, BASE, ARG0TY, ARG0NM, ARG0OPT, ARG1TY, ARG1NM, ARG1OPT) \
  JSReference build(NAME##Node *node) {                           \
    return build_##NAME(                                          \
        buildLoc(node),                                           \
        buildNode(node->_##ARG0NM),                               \
        buildNode(node->_##ARG1NM));                              \
  }

#define ESTREE_NODE_3_ARGS(             \
    NAME,                               \
    BASE,                               \
    ARG0TY,                             \
    ARG0NM,                             \
    ARG0OPT,                            \
    ARG1TY,                             \
    ARG1NM,                             \
    ARG1OPT,                            \
    ARG2TY,                             \
    ARG2NM,                             \
    ARG2OPT)                            \
  JSReference build(NAME##Node *node) { \
    return build_##NAME(                \
        buildLoc(node),                 \
        buildNode(node->_##ARG0NM),     \
        buildNode(node->_##ARG1NM),     \
        buildNode(node->_##ARG2NM));    \
  }

#define ESTREE_NODE_4_ARGS(             \
    NAME,                               \
    BASE,                               \
    ARG0TY,                             \
    ARG0NM,                             \
    ARG0OPT,                            \
    ARG1TY,                             \
    ARG1NM,                             \
    ARG1OPT,                            \
    ARG2TY,                             \
    ARG2NM,                             \
    ARG2OPT,                            \
    ARG3TY,                             \
    ARG3NM,                             \
    ARG3OPT)                            \
  JSReference build(NAME##Node *node) { \
    return build_##NAME(                \
        buildLoc(node),                 \
        buildNode(node->_##ARG0NM),     \
        buildNode(node->_##ARG1NM),     \
        buildNode(node->_##ARG2NM),     \
        buildNode(node->_##ARG3NM));    \
  }

#define ESTREE_NODE_5_ARGS(             \
    NAME,                               \
    BASE,                               \
    ARG0TY,                             \
    ARG0NM,                             \
    ARG0OPT,                            \
    ARG1TY,                             \
    ARG1NM,                             \
    ARG1OPT,                            \
    ARG2TY,                             \
    ARG2NM,                             \
    ARG2OPT,                            \
    ARG3TY,                             \
    ARG3NM,                             \
    ARG3OPT,                            \
    ARG4TY,                             \
    ARG4NM,                             \
    ARG4OPT)                            \
  JSReference build(NAME##Node *node) { \
    return build_##NAME(                \
        buildLoc(node),                 \
        buildNode(node->_##ARG0NM),     \
        buildNode(node->_##ARG1NM),     \
        buildNode(node->_##ARG2NM),     \
        buildNode(node->_##ARG3NM),     \
        buildNode(node->_##ARG4NM));    \
  }

#define ESTREE_NODE_6_ARGS(             \
    NAME,                               \
    BASE,                               \
    ARG0TY,                             \
    ARG0NM,                             \
    ARG0OPT,                            \
    ARG1TY,                             \
    ARG1NM,                             \
    ARG1OPT,                            \
    ARG2TY,                             \
    ARG2NM,                             \
    ARG2OPT,                            \
    ARG3TY,                             \
    ARG3NM,                             \
    ARG3OPT,                            \
    ARG4TY,                             \
    ARG4NM,                             \
    ARG4OPT,                            \
    ARG5TY,                             \
    ARG5NM,                             \
    ARG5OPT)                            \
  JSReference build(NAME##Node *node) { \
    return build_##NAME(                \
        buildLoc(node),                 \
        buildNode(node->_##ARG0NM),     \
        buildNode(node->_##ARG1NM),     \
        buildNode(node->_##ARG2NM),     \
        buildNode(node->_##ARG3NM),     \
        buildNode(node->_##ARG4NM),     \
        buildNode(node->_##ARG5NM));    \
  }

#define ESTREE_NODE_7_ARGS(             \
    NAME,                               \
    BASE,                               \
    ARG0TY,                             \
    ARG0NM,                             \
    ARG0OPT,                            \
    ARG1TY,                             \
    ARG1NM,                             \
    ARG1OPT,                            \
    ARG2TY,                             \
    ARG2NM,                             \
    ARG2OPT,                            \
    ARG3TY,                             \
    ARG3NM,                             \
    ARG3OPT,                            \
    ARG4TY,                             \
    ARG4NM,                             \
    ARG4OPT,                            \
    ARG5TY,                             \
    ARG5NM,                             \
    ARG5OPT,                            \
    ARG6TY,                             \
    ARG6NM,                             \
    ARG6OPT)                            \
  JSReference build(NAME##Node *node) { \
    return build_##NAME(                \
        buildLoc(node),                 \
        buildNode(node->_##ARG0NM),     \
        buildNode(node->_##ARG1NM),     \
        buildNode(node->_##ARG2NM),     \
        buildNode(node->_##ARG3NM),     \
        buildNode(node->_##ARG4NM),     \
        buildNode(node->_##ARG5NM),     \
        buildNode(node->_##ARG6NM));    \
  }

#define ESTREE_NODE_8_ARGS(             \
    NAME,                               \
    BASE,                               \
    ARG0TY,                             \
    ARG0NM,                             \
    ARG0OPT,                            \
    ARG1TY,                             \
    ARG1NM,                             \
    ARG1OPT,                            \
    ARG2TY,                             \
    ARG2NM,                             \
    ARG2OPT,                            \
    ARG3TY,                             \
    ARG3NM,                             \
    ARG3OPT,                            \
    ARG4TY,                             \
    ARG4NM,                             \
    ARG4OPT,                            \
    ARG5TY,                             \
    ARG5NM,                             \
    ARG5OPT,                            \
    ARG6TY,                             \
    ARG6NM,                             \
    ARG6OPT,                            \
    ARG7TY,                             \
    ARG7NM,                             \
    ARG7OPT)                            \
  JSReference build(NAME##Node *node) { \
    return build_##NAME(                \
        buildLoc(node),                 \
        buildNode(node->_##ARG0NM),     \
        buildNode(node->_##ARG1NM),     \
        buildNode(node->_##ARG2NM),     \
        buildNode(node->_##ARG3NM),     \
        buildNode(node->_##ARG4NM),     \
        buildNode(node->_##ARG5NM),     \
        buildNode(node->_##ARG6NM),     \
        buildNode(node->_##ARG7NM));    \
  }

#include "hermes/AST/ESTree.def"
};

JSReference buildProgram(
    ProgramNode *programNode,
    SourceErrorManager *sm,
    parser::JSParser *parser) {
  return HermesParserJSBuilder(sm, parser).buildProgram(programNode);
}

} // namespace hermes
