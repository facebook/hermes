/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "llvh/Support/MemoryBuffer.h"

#include "HermesParserJSSerializer.h"

namespace hermes {

using namespace hermes::ESTree;
using namespace parser;

class HermesParserJSSerializer {
  SourceErrorManager *sm_{nullptr};
  ParseResult &result_;
  std::vector<PositionInfo> positions_;

  /// Used to generate unique IDs for each source location.
  uint32_t nextLocId_ = 0;

 public:
  explicit HermesParserJSSerializer(SourceErrorManager *sm, ParseResult &result)
      : sm_(sm), result_(result) {}

  void serializeProgram(ProgramNode *programNode, bool tokens) {
    serializeLoc(programNode->getSourceRange());
    serializeNode(programNode->_body);
    serializeComments();

    if (tokens) {
      serializeTokens();
    }

    serializeSourcePositions(programNode);
  }

 private:
  /// Booleans are serialized as a single 4-byte integer.
  void serializeNode(NodeBoolean val) {
    result_.programBuffer_.emplace_back(val);
  }

  /// Numbers are serialized directly into program buffer, but must be aligned
  /// on 8-byte boundaries.
  void serializeNode(NodeNumber num) {
    // HEAPF64 requires doubles aligned on 8 byte boundaries but we are using a
    // buffer of 4 byte values, so add 4 byte padding if necessary.
    std::uintptr_t ptr = reinterpret_cast<std::uintptr_t>(
        result_.programBuffer_.data() + result_.programBuffer_.size());
    if (ptr % 8) {
      result_.programBuffer_.emplace_back(0);
    }

    // Split up number into two 4-byte sections so it can be written into
    // program buffer.
    uint64_t bytes;
    memcpy(&bytes, &num, sizeof(uint64_t));

    result_.programBuffer_.emplace_back((uint32_t)bytes);
    result_.programBuffer_.emplace_back((uint32_t)(bytes >> 32));
  }

  /// Strings are serialized as a 4-byte pointer into the heap, followed
  /// by their size as a 4-byte integer. The size is only present if the
  /// pointer is non-null.
  void serializeNode(NodeLabel label) {
    if (label == nullptr) {
      result_.programBuffer_.emplace_back(0);
      return;
    }

    auto str = label->str();

    result_.programBuffer_.emplace_back((uint32_t)str.begin());
    result_.programBuffer_.emplace_back((uint32_t)str.size());
  }

  /// Node lists are serialized as a 4-byte integer denoting the number of
  /// elements in the list, followed by the serialized elements.
  void serializeNode(NodeList &list) {
    result_.programBuffer_.emplace_back((uint32_t)list.size());
    for (auto &node : list) {
      serializeNode(&node);
    }
  }

  /// Calculate line, column, and overall offset for every position encountered
  /// in the AST and fill the buffer of calculated positions.
  ///
  /// Note that these source locations are meant to be consumed from JS, so they
  /// are calculated as indices into a JS string. This means that a single
  /// "column" corresponds to a single UTF-16 code unit, so code points of at
  /// most U+FFFF count as one "column" whereas code points above U+FFFF count
  /// as two "columns" since they take two UTF-16 code units to represent.
  void serializeSourcePositions(ProgramNode *programNode) {
    result_.positionBuffer_.reserve(positions_.size());

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

      result_.positionBuffer_.emplace_back(
          positionsIt->locId, positionsIt->kind, line, col, offset);

      ++positionsIt;
    }
  }

  /// Save references to the start and end positions so they can be calculated
  /// later. Write a new 4-byte loc ID to the program buffer to uniquely
  /// identify this loc.
  void serializeLoc(SMRange rng) {
    positions_.emplace_back(
        PositionInfo::Kind::Start, rng.Start.getPointer(), nextLocId_);
    positions_.emplace_back(
        PositionInfo::Kind::End, rng.End.getPointer(), nextLocId_);

    result_.programBuffer_.emplace_back(nextLocId_++);
  }

  /// Comments are serialized as a 4-byte integer denoting comment type,
  /// followed by a 4-byte value denoting the loc ID, followed by a serialized
  /// string for the comment value.
  ///
  /// The list of comments begin with the number of comments in the list as a
  /// 4-byte integer.
  void serializeComments() {
    auto comments = result_.parser_->getStoredComments();
    result_.programBuffer_.emplace_back((uint32_t)comments.size());

    for (auto &comment : comments) {
      result_.programBuffer_.emplace_back((uint32_t)comment.getKind());
      serializeLoc(comment.getSourceRange());

      auto value = comment.getString();
      result_.programBuffer_.emplace_back((uint32_t)value.begin());
      result_.programBuffer_.emplace_back((uint32_t)value.size());
    }
  }

  /// The list of tokens begins with the number of tokens serialized as a 4-byte
  /// integer, followed by a sequence of serialized tokens.
  ///
  /// We exclude the EOF token from this sequence and the total number of
  /// tokens. The EOF token will always appear at the end of a successful parse.
  void serializeTokens() {
    auto tokens = result_.parser_->getStoredTokens();
    result_.programBuffer_.emplace_back((uint32_t)tokens.size() - 1);

    for (auto &token : tokens) {
      TokenKind kind = token.getKind();

      // Special case reserved words that are literals, to avoid treating them
      // as keywords like all other reserved words.
      if (kind == TokenKind::rw_null) {
        serializeToken(token, TokenType::Null);
        continue;
      } else if (kind == TokenKind::rw_true || kind == TokenKind::rw_false) {
        serializeToken(token, TokenType::Boolean);
        continue;
      }

      switch (kind) {
        case TokenKind::identifier:
        case TokenKind::private_identifier:
          serializeToken(token, TokenType::Identifier);
          break;
        case TokenKind::numeric_literal:
          serializeToken(token, TokenType::Numeric);
          break;
        case TokenKind::bigint_literal:
          serializeToken(token, TokenType::BigInt);
          break;
        case TokenKind::string_literal:
          serializeToken(token, TokenType::String);
          break;
        case TokenKind::regexp_literal:
          serializeToken(token, TokenType::RegularExpression);
          break;
        case TokenKind::jsx_text:
          serializeToken(token, TokenType::JSXText);
          break;

#define PUNCTUATOR(NAME, ...)                     \
  case TokenKind::NAME:                           \
    serializeToken(token, TokenType::Punctuator); \
    break;

#define PUNCTUATOR_FLOW(NAME, ...)                \
  case TokenKind::NAME:                           \
    serializeToken(token, TokenType::Punctuator); \
    break;

#define TEMPLATE(NAME, ...)                     \
  case TokenKind::NAME:                         \
    serializeToken(token, TokenType::Template); \
    break;

#define RESWORD(NAME, ...)                     \
  case TokenKind::rw_##NAME:                   \
    serializeToken(token, TokenType::Keyword); \
    break;

#include "hermes/Parser/TokenKinds.def"

        // Exclude EOF token
        case TokenKind::eof:
        // Sigil tokens that are never created
        case TokenKind::none:
        case TokenKind::_first_resword:
        case TokenKind::_last_resword:
        case TokenKind::_first_binary:
        case TokenKind::_last_binary:
        case TokenKind::_last_token:
          break;
      }
    }
  }

  /// Tokens are serialized as a 4-byte integer denoting token type,
  /// followed by a 4-byte value denoting the loc ID, followed by a serialized
  /// string for the token value.
  void serializeToken(const StoredToken &token, TokenType type) {
    result_.programBuffer_.emplace_back((uint32_t)type);

    SMRange range = token.getSourceRange();
    serializeLoc(range);

    const char *start = range.Start.getPointer();
    result_.programBuffer_.emplace_back((uint32_t)start);
    result_.programBuffer_.emplace_back(
        (uint32_t)(range.End.getPointer() - start));
  }

  /// AST nodes are serialized as a 4-byte node kind, followed by a 4-byte
  /// loc ID, followed by the serialized node properties.
  void serializeNode(NodePtr node) {
    if (!node) {
      result_.programBuffer_.emplace_back(0);
      return;
    }

    // Node kinds are offset by one so that zero can denote the null pointer
    result_.programBuffer_.emplace_back((uint32_t)node->getKind() + 1);
    serializeLoc(node->getSourceRange());

    switch (node->getKind()) {
      default:
        llvm_unreachable("invalid node kind");

#define SERIALIZE(NAME)                \
  case NodeKind::NAME:                 \
    serialize(cast<NAME##Node>(node)); \
    return;

#define ESTREE_NODE_0_ARGS(NAME, ...) SERIALIZE(NAME)
#define ESTREE_NODE_1_ARGS(NAME, ...) SERIALIZE(NAME)
#define ESTREE_NODE_2_ARGS(NAME, ...) SERIALIZE(NAME)
#define ESTREE_NODE_3_ARGS(NAME, ...) SERIALIZE(NAME)
#define ESTREE_NODE_4_ARGS(NAME, ...) SERIALIZE(NAME)
#define ESTREE_NODE_5_ARGS(NAME, ...) SERIALIZE(NAME)
#define ESTREE_NODE_6_ARGS(NAME, ...) SERIALIZE(NAME)
#define ESTREE_NODE_7_ARGS(NAME, ...) SERIALIZE(NAME)
#define ESTREE_NODE_8_ARGS(NAME, ...) SERIALIZE(NAME)

#include "hermes/AST/ESTree.def"

#undef SERIALIZE
    }
  }

#define ESTREE_NODE_0_ARGS(NAME, BASE) \
  void serialize(NAME##Node *node) {}

#define ESTREE_NODE_1_ARGS(NAME, BASE, ARG0TY, ARG0NM, ARG0OPT) \
  void serialize(NAME##Node *node) {                            \
    serializeNode(node->_##ARG0NM);                             \
  }

#define ESTREE_NODE_2_ARGS(                                       \
    NAME, BASE, ARG0TY, ARG0NM, ARG0OPT, ARG1TY, ARG1NM, ARG1OPT) \
  void serialize(NAME##Node *node) {                              \
    serializeNode(node->_##ARG0NM);                               \
    serializeNode(node->_##ARG1NM);                               \
  }

#define ESTREE_NODE_3_ARGS(          \
    NAME,                            \
    BASE,                            \
    ARG0TY,                          \
    ARG0NM,                          \
    ARG0OPT,                         \
    ARG1TY,                          \
    ARG1NM,                          \
    ARG1OPT,                         \
    ARG2TY,                          \
    ARG2NM,                          \
    ARG2OPT)                         \
  void serialize(NAME##Node *node) { \
    serializeNode(node->_##ARG0NM);  \
    serializeNode(node->_##ARG1NM);  \
    serializeNode(node->_##ARG2NM);  \
  }

#define ESTREE_NODE_4_ARGS(          \
    NAME,                            \
    BASE,                            \
    ARG0TY,                          \
    ARG0NM,                          \
    ARG0OPT,                         \
    ARG1TY,                          \
    ARG1NM,                          \
    ARG1OPT,                         \
    ARG2TY,                          \
    ARG2NM,                          \
    ARG2OPT,                         \
    ARG3TY,                          \
    ARG3NM,                          \
    ARG3OPT)                         \
  void serialize(NAME##Node *node) { \
    serializeNode(node->_##ARG0NM);  \
    serializeNode(node->_##ARG1NM);  \
    serializeNode(node->_##ARG2NM);  \
    serializeNode(node->_##ARG3NM);  \
  }

#define ESTREE_NODE_5_ARGS(          \
    NAME,                            \
    BASE,                            \
    ARG0TY,                          \
    ARG0NM,                          \
    ARG0OPT,                         \
    ARG1TY,                          \
    ARG1NM,                          \
    ARG1OPT,                         \
    ARG2TY,                          \
    ARG2NM,                          \
    ARG2OPT,                         \
    ARG3TY,                          \
    ARG3NM,                          \
    ARG3OPT,                         \
    ARG4TY,                          \
    ARG4NM,                          \
    ARG4OPT)                         \
  void serialize(NAME##Node *node) { \
    serializeNode(node->_##ARG0NM);  \
    serializeNode(node->_##ARG1NM);  \
    serializeNode(node->_##ARG2NM);  \
    serializeNode(node->_##ARG3NM);  \
    serializeNode(node->_##ARG4NM);  \
  }

#define ESTREE_NODE_6_ARGS(          \
    NAME,                            \
    BASE,                            \
    ARG0TY,                          \
    ARG0NM,                          \
    ARG0OPT,                         \
    ARG1TY,                          \
    ARG1NM,                          \
    ARG1OPT,                         \
    ARG2TY,                          \
    ARG2NM,                          \
    ARG2OPT,                         \
    ARG3TY,                          \
    ARG3NM,                          \
    ARG3OPT,                         \
    ARG4TY,                          \
    ARG4NM,                          \
    ARG4OPT,                         \
    ARG5TY,                          \
    ARG5NM,                          \
    ARG5OPT)                         \
  void serialize(NAME##Node *node) { \
    serializeNode(node->_##ARG0NM);  \
    serializeNode(node->_##ARG1NM);  \
    serializeNode(node->_##ARG2NM);  \
    serializeNode(node->_##ARG3NM);  \
    serializeNode(node->_##ARG4NM);  \
    serializeNode(node->_##ARG5NM);  \
  }

#define ESTREE_NODE_7_ARGS(          \
    NAME,                            \
    BASE,                            \
    ARG0TY,                          \
    ARG0NM,                          \
    ARG0OPT,                         \
    ARG1TY,                          \
    ARG1NM,                          \
    ARG1OPT,                         \
    ARG2TY,                          \
    ARG2NM,                          \
    ARG2OPT,                         \
    ARG3TY,                          \
    ARG3NM,                          \
    ARG3OPT,                         \
    ARG4TY,                          \
    ARG4NM,                          \
    ARG4OPT,                         \
    ARG5TY,                          \
    ARG5NM,                          \
    ARG5OPT,                         \
    ARG6TY,                          \
    ARG6NM,                          \
    ARG6OPT)                         \
  void serialize(NAME##Node *node) { \
    serializeNode(node->_##ARG0NM);  \
    serializeNode(node->_##ARG1NM);  \
    serializeNode(node->_##ARG2NM);  \
    serializeNode(node->_##ARG3NM);  \
    serializeNode(node->_##ARG4NM);  \
    serializeNode(node->_##ARG5NM);  \
    serializeNode(node->_##ARG6NM);  \
  }

#define ESTREE_NODE_8_ARGS(          \
    NAME,                            \
    BASE,                            \
    ARG0TY,                          \
    ARG0NM,                          \
    ARG0OPT,                         \
    ARG1TY,                          \
    ARG1NM,                          \
    ARG1OPT,                         \
    ARG2TY,                          \
    ARG2NM,                          \
    ARG2OPT,                         \
    ARG3TY,                          \
    ARG3NM,                          \
    ARG3OPT,                         \
    ARG4TY,                          \
    ARG4NM,                          \
    ARG4OPT,                         \
    ARG5TY,                          \
    ARG5NM,                          \
    ARG5OPT,                         \
    ARG6TY,                          \
    ARG6NM,                          \
    ARG6OPT,                         \
    ARG7TY,                          \
    ARG7NM,                          \
    ARG7OPT)                         \
  void serialize(NAME##Node *node) { \
    serializeNode(node->_##ARG0NM);  \
    serializeNode(node->_##ARG1NM);  \
    serializeNode(node->_##ARG2NM);  \
    serializeNode(node->_##ARG3NM);  \
    serializeNode(node->_##ARG4NM);  \
    serializeNode(node->_##ARG5NM);  \
    serializeNode(node->_##ARG6NM);  \
    serializeNode(node->_##ARG7NM);  \
  }

#include "hermes/AST/ESTree.def"
};

void serialize(
    ProgramNode *programNode,
    SourceErrorManager *sm,
    ParseResult &result,
    bool tokens) {
  HermesParserJSSerializer(sm, result).serializeProgram(programNode, tokens);
}

} // namespace hermes
