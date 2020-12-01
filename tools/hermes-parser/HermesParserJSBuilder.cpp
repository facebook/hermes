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

/// A reference to the start or end position of a source location along with
/// the handle of the corresponding source location object in JS.
struct PositionInfo {
  enum class Kind { Start, End };

  Kind kind;

  /// The location itself in the source text.
  const char *ptr;

  // Handle of the JS source location object.
  JSReference jsRef;

  PositionInfo(Kind kind, const char *ptr, JSReference jsRef)
      : kind(kind), ptr(ptr), jsRef(jsRef) {}

  /// Order by position in source text.
  bool operator<(const PositionInfo &x) const {
    return ptr < x.ptr;
  }
};

class HermesParserJSBuilder {
  SourceErrorManager *sm_{nullptr};
  parser::JSParser *parser_{nullptr};
  std::vector<PositionInfo> positions_;

 public:
  explicit HermesParserJSBuilder(
      SourceErrorManager *sm,
      parser::JSParser *parser)
      : sm_(sm), parser_(parser) {}

  JSReference buildProgram(ProgramNode *programNode) {
    auto loc = buildLoc(programNode);
    auto body = buildNode(programNode->_body);
    auto comments = buildComments();
    auto program = buildProgramWithComments(loc, body, comments);
    buildSourcePositions(programNode);

    return program;
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

  /// Decode a string within JS and return a reference to it from this function.
  /// We must pass the size along with the pointer as the string may contain
  /// null bytes.
  JSReference buildNode(NodeLabel label) {
    if (label == nullptr) {
      return 0;
    }

    auto str = label->str();

    return buildString(str.begin(), str.size());
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

  /// Calculate line, column, and overall offset for every position encountered
  /// in the AST and use these to fill in JS source location objects.
  ///
  /// Note that these source locations are meant to be consumed from JS, so they
  /// are calculated as indices into a JS string. This means that a single
  /// "column" corresponds to a single UTF-16 code unit, so code points of at
  /// most U+FFFF count as one "column" whereas code points above U+FFFF count
  /// as two "columns" since they take two UTF-16 code units to represent.
  void buildSourcePositions(ProgramNode *programNode) {
    // Sort all positions by their order in the source text
    std::sort(positions_.begin(), positions_.end());
    auto positionsIt = positions_.begin();
    auto positionsEnd = positions_.end();

    const llvh::MemoryBuffer *buffer =
        sm_->findBufferForLoc(programNode->getStartLoc());
    const char *ptr = buffer->getBufferStart();

    unsigned line = 1;
    unsigned col = 0;
    unsigned offset = 0;

    // Iterate through both sorted positions and source text to calculate the
    // line, column, and offset for each position in JS.
    while (positionsIt < positionsEnd) {
      auto nextLoc = positionsIt->ptr;
      while (ptr < nextLoc) {
        char ch = *ptr;
        if (ch == '\n') {
          // Newline character translates to one UTF-16 code unit
          ++offset;
          ++line;
          col = 0;

          ++ptr;
        } else if ((unsigned char)ch < 128) {
          // One byte UTF-8 code point, translates to one UTF-16 code unit
          ++offset;
          ++col;

          ++ptr;
        } else if ((ch & 0xE0) == 0xC0) {
          // Two byte UTF-8 code point, translates to one UTF-16 code unit
          ++offset;
          ++col;

          ptr += 2;
        } else if ((ch & 0xF0) == 0xE0) {
          // Three byte UTF-8 code point, translates to one UTF-16 code unit
          ++offset;
          ++col;

          ptr += 3;
        } else {
          // Four byte UTF-8 code point, corresponds to code points above U+FFFF
          // which translate to two UTF-16 code units.
          offset += 2;
          col += 2;

          ptr += 4;
        }
      }

      // We found the position, so add the newly calcualted position to the JS
      // source location
      if (positionsIt->kind == PositionInfo::Kind::Start) {
        addStartPosition(positionsIt->jsRef, line, col, offset);
      } else {
        addEndPosition(positionsIt->jsRef, line, col, offset);
      }

      ++positionsIt;
    }
  }

  /// Build an empty source location in JS, and save references to the start
  /// and end positions so they can be calculated and filled later.
  JSReference buildLoc(SMRange rng) {
    JSReference locReference = buildSourceLocation();

    positions_.emplace_back(
        PositionInfo::Kind::Start, rng.Start.getPointer(), locReference);
    positions_.emplace_back(
        PositionInfo::Kind::End, rng.End.getPointer(), locReference);

    return locReference;
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
