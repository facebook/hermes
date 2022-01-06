/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_TOOLS_HERMESPARSER_HERMESPARSERJSSERIALIZER_H
#define HERMES_TOOLS_HERMESPARSER_HERMESPARSERJSSERIALIZER_H

#include "hermes/AST/ESTree.h"
#include "hermes/Parser/JSParser.h"

namespace hermes {

/// General category for token, based off esprima's token types.
enum class TokenType {
  Boolean,
  Identifier,
  Keyword,
  Null,
  Numeric,
  BigInt,
  Punctuator,
  String,
  RegularExpression,
  Template,
  JSXText
};

/// A reference to the start or end position of a source location.
struct PositionInfo {
  enum class Kind { Start, End };

  Kind kind;

  /// The location itself in the source text.
  const char *ptr;

  /// ID of the source location this position is associated with.
  uint32_t locId;

  PositionInfo(Kind kind, const char *ptr, uint32_t locId)
      : kind(kind), ptr(ptr), locId(locId) {}

  /// Order by position in source text.
  bool operator<(const PositionInfo &x) const {
    return ptr < x.ptr;
  }
};

struct PositionResult {
  /// ID of the source location this position is associated with.
  uint32_t locId;
  /// 0 if this is a start position, 1 if this is an end position.
  uint32_t kind;

  uint32_t line;
  uint32_t column;
  uint32_t offset;

  PositionResult(
      uint32_t locId,
      PositionInfo::Kind kind,
      uint32_t line,
      uint32_t column,
      uint32_t offset)
      : locId(locId),
        kind(kind == PositionInfo::Kind::Start ? 0 : 1),
        line(line),
        column(column),
        offset(offset) {}
};

/// An opaque object containing the result of parsing
class ParseResult {
 public:
  std::string error_;
  uint32_t errorLine_ = 0;
  uint32_t errorColumn_ = 0;
  /// Buffer containing serialized AST
  std::vector<uint32_t> programBuffer_;
  /// Buffer containing serialized source positions
  std::vector<PositionResult> positionBuffer_;

  // Keep references to parser and context as they should last until
  // parse result is freed.
  std::shared_ptr<Context> context_{nullptr};
  std::unique_ptr<parser::JSParser> parser_{nullptr};
};

void serialize(
    ESTree::ProgramNode *programNode,
    SourceErrorManager *sm,
    ParseResult &result,
    bool tokens);

} // namespace hermes

#endif
