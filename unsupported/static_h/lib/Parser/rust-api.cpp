/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/*
 * This file exposes a C API for use by Rust. It cannot actually be used from C
 * as is, because the function declarations themselves require C++ types.
 * Addressing that would require non-trivial effort, which for now we have
 * chosen not to do, given that we don't have a use case for C, just Rust. For
 * that reason we are not exposing a header file - there is no possible
 * consumer for that file.
 */

#include "hermes/Parser/FlowHelpers.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Support/SimpleDiagHandler.h"
#include "llvh/ADT/StringRef.h"

// As explained above, we don't have a header with prototypes for this file.
#if defined(__GNUC__) && defined(__clang__)
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#endif

using namespace hermes;
using namespace hermes::ESTree;

// While we return almost all attributes unmodified by value, that doesn't work
// for NodeList (since it is the head of a circular list) and we need to return
// a pointer to it.
// So some templating to adjust the return type of the getter depending on the
// type of the AST attribute.
namespace {
template <class T>
struct ReturnType {
  using Type = T;
  static inline Type get(T x) {
    return x;
  }
};

template <>
struct ReturnType<NodeList> {
  using Type = const NodeList *;
  static inline Type get(const NodeList &x) {
    return &x;
  }
};
} // namespace

#define ESTREE_FIRST(NAME, ...)
#define ESTREE_NODE_0_ARGS(NAME, ...)

// NOTE: -fomit-frame-pointer helps here

#define ESTREE_NODE_1_ARGS(NAME, BASE, ARG0TY, ARG0NM, ARG0OPT)     \
  extern "C" ReturnType<ARG0TY>::Type hermes_get_##NAME##_##ARG0NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG0TY>::get(node->_##ARG0NM);                \
  }

#define ESTREE_NODE_2_ARGS(                                         \
    NAME, BASE, ARG0TY, ARG0NM, ARG0OPT, ARG1TY, ARG1NM, ARG1OPT)   \
  extern "C" ReturnType<ARG0TY>::Type hermes_get_##NAME##_##ARG0NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG0TY>::get(node->_##ARG0NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG1TY>::Type hermes_get_##NAME##_##ARG1NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG1TY>::get(node->_##ARG1NM);                \
  }

#define ESTREE_NODE_3_ARGS(                                         \
    NAME,                                                           \
    BASE,                                                           \
    ARG0TY,                                                         \
    ARG0NM,                                                         \
    ARG0OPT,                                                        \
    ARG1TY,                                                         \
    ARG1NM,                                                         \
    ARG1OPT,                                                        \
    ARG2TY,                                                         \
    ARG2NM,                                                         \
    ARG2OPT)                                                        \
  extern "C" ReturnType<ARG0TY>::Type hermes_get_##NAME##_##ARG0NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG0TY>::get(node->_##ARG0NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG1TY>::Type hermes_get_##NAME##_##ARG1NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG1TY>::get(node->_##ARG1NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG2TY>::Type hermes_get_##NAME##_##ARG2NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG2TY>::get(node->_##ARG2NM);                \
  }

#define ESTREE_NODE_4_ARGS(                                         \
    NAME,                                                           \
    BASE,                                                           \
    ARG0TY,                                                         \
    ARG0NM,                                                         \
    ARG0OPT,                                                        \
    ARG1TY,                                                         \
    ARG1NM,                                                         \
    ARG1OPT,                                                        \
    ARG2TY,                                                         \
    ARG2NM,                                                         \
    ARG2OPT,                                                        \
    ARG3TY,                                                         \
    ARG3NM,                                                         \
    ARG3OPT)                                                        \
  extern "C" ReturnType<ARG0TY>::Type hermes_get_##NAME##_##ARG0NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG0TY>::get(node->_##ARG0NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG1TY>::Type hermes_get_##NAME##_##ARG1NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG1TY>::get(node->_##ARG1NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG2TY>::Type hermes_get_##NAME##_##ARG2NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG2TY>::get(node->_##ARG2NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG3TY>::Type hermes_get_##NAME##_##ARG3NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG3TY>::get(node->_##ARG3NM);                \
  }

#define ESTREE_NODE_5_ARGS(                                         \
    NAME,                                                           \
    BASE,                                                           \
    ARG0TY,                                                         \
    ARG0NM,                                                         \
    ARG0OPT,                                                        \
    ARG1TY,                                                         \
    ARG1NM,                                                         \
    ARG1OPT,                                                        \
    ARG2TY,                                                         \
    ARG2NM,                                                         \
    ARG2OPT,                                                        \
    ARG3TY,                                                         \
    ARG3NM,                                                         \
    ARG3OPT,                                                        \
    ARG4TY,                                                         \
    ARG4NM,                                                         \
    ARG4OPT)                                                        \
  extern "C" ReturnType<ARG0TY>::Type hermes_get_##NAME##_##ARG0NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG0TY>::get(node->_##ARG0NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG1TY>::Type hermes_get_##NAME##_##ARG1NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG1TY>::get(node->_##ARG1NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG2TY>::Type hermes_get_##NAME##_##ARG2NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG2TY>::get(node->_##ARG2NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG3TY>::Type hermes_get_##NAME##_##ARG3NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG3TY>::get(node->_##ARG3NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG4TY>::Type hermes_get_##NAME##_##ARG4NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG4TY>::get(node->_##ARG4NM);                \
  }

#define ESTREE_NODE_6_ARGS(                                         \
    NAME,                                                           \
    BASE,                                                           \
    ARG0TY,                                                         \
    ARG0NM,                                                         \
    ARG0OPT,                                                        \
    ARG1TY,                                                         \
    ARG1NM,                                                         \
    ARG1OPT,                                                        \
    ARG2TY,                                                         \
    ARG2NM,                                                         \
    ARG2OPT,                                                        \
    ARG3TY,                                                         \
    ARG3NM,                                                         \
    ARG3OPT,                                                        \
    ARG4TY,                                                         \
    ARG4NM,                                                         \
    ARG4OPT,                                                        \
    ARG5TY,                                                         \
    ARG5NM,                                                         \
    ARG5OPT)                                                        \
  extern "C" ReturnType<ARG0TY>::Type hermes_get_##NAME##_##ARG0NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG0TY>::get(node->_##ARG0NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG1TY>::Type hermes_get_##NAME##_##ARG1NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG1TY>::get(node->_##ARG1NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG2TY>::Type hermes_get_##NAME##_##ARG2NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG2TY>::get(node->_##ARG2NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG3TY>::Type hermes_get_##NAME##_##ARG3NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG3TY>::get(node->_##ARG3NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG4TY>::Type hermes_get_##NAME##_##ARG4NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG4TY>::get(node->_##ARG4NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG5TY>::Type hermes_get_##NAME##_##ARG5NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG5TY>::get(node->_##ARG5NM);                \
  }

#define ESTREE_NODE_7_ARGS(                                         \
    NAME,                                                           \
    BASE,                                                           \
    ARG0TY,                                                         \
    ARG0NM,                                                         \
    ARG0OPT,                                                        \
    ARG1TY,                                                         \
    ARG1NM,                                                         \
    ARG1OPT,                                                        \
    ARG2TY,                                                         \
    ARG2NM,                                                         \
    ARG2OPT,                                                        \
    ARG3TY,                                                         \
    ARG3NM,                                                         \
    ARG3OPT,                                                        \
    ARG4TY,                                                         \
    ARG4NM,                                                         \
    ARG4OPT,                                                        \
    ARG5TY,                                                         \
    ARG5NM,                                                         \
    ARG5OPT,                                                        \
    ARG6TY,                                                         \
    ARG6NM,                                                         \
    ARG6OPT)                                                        \
  extern "C" ReturnType<ARG0TY>::Type hermes_get_##NAME##_##ARG0NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG0TY>::get(node->_##ARG0NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG1TY>::Type hermes_get_##NAME##_##ARG1NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG1TY>::get(node->_##ARG1NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG2TY>::Type hermes_get_##NAME##_##ARG2NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG2TY>::get(node->_##ARG2NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG3TY>::Type hermes_get_##NAME##_##ARG3NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG3TY>::get(node->_##ARG3NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG4TY>::Type hermes_get_##NAME##_##ARG4NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG4TY>::get(node->_##ARG4NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG5TY>::Type hermes_get_##NAME##_##ARG5NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG5TY>::get(node->_##ARG5NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG6TY>::Type hermes_get_##NAME##_##ARG6NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG6TY>::get(node->_##ARG6NM);                \
  }

#define ESTREE_NODE_8_ARGS(                                         \
    NAME,                                                           \
    BASE,                                                           \
    ARG0TY,                                                         \
    ARG0NM,                                                         \
    ARG0OPT,                                                        \
    ARG1TY,                                                         \
    ARG1NM,                                                         \
    ARG1OPT,                                                        \
    ARG2TY,                                                         \
    ARG2NM,                                                         \
    ARG2OPT,                                                        \
    ARG3TY,                                                         \
    ARG3NM,                                                         \
    ARG3OPT,                                                        \
    ARG4TY,                                                         \
    ARG4NM,                                                         \
    ARG4OPT,                                                        \
    ARG5TY,                                                         \
    ARG5NM,                                                         \
    ARG5OPT,                                                        \
    ARG6TY,                                                         \
    ARG6NM,                                                         \
    ARG6OPT,                                                        \
    ARG7TY,                                                         \
    ARG7NM,                                                         \
    ARG7OPT)                                                        \
  extern "C" ReturnType<ARG0TY>::Type hermes_get_##NAME##_##ARG0NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG0TY>::get(node->_##ARG0NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG1TY>::Type hermes_get_##NAME##_##ARG1NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG1TY>::get(node->_##ARG1NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG2TY>::Type hermes_get_##NAME##_##ARG2NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG2TY>::get(node->_##ARG2NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG3TY>::Type hermes_get_##NAME##_##ARG3NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG3TY>::get(node->_##ARG3NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG4TY>::Type hermes_get_##NAME##_##ARG4NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG4TY>::get(node->_##ARG4NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG5TY>::Type hermes_get_##NAME##_##ARG5NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG5TY>::get(node->_##ARG5NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG6TY>::Type hermes_get_##NAME##_##ARG6NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG6TY>::get(node->_##ARG6NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG7TY>::Type hermes_get_##NAME##_##ARG7NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG7TY>::get(node->_##ARG7NM);                \
  }

#include "hermes/AST/ESTree.def"

namespace {

enum class ParserDialect : uint8_t {
  /// Good old JS.
  JavaScript,
  /// Parse all Flow type syntax.
  Flow,
  /// Parse all unambiguous Flow type syntax. Syntax that can be intepreted as
  /// either Flow types or standard JavaScript is parsed as if it were standard
  /// JavaScript.
  ///
  /// For example, `foo<T>(x)` is parsed as if it were standard JavaScript
  /// containing two comparisons, even though it could otherwise be interpreted
  /// as a call expression with Flow type arguments.
  FlowUnambiguous,
  /// Look for the '@flow' pragma in the first comment in the JS file.
  /// If it exists, then parse all Flow type syntax, otherwise only parse
  /// Flow type syntax according to the FlowUnambiguous mode.
  FlowDetect,
  /// Parse TypeScript.
  TypeScript,
};

/// Flags controlling the behavior of the parser.
struct ParserFlags {
  /// Start parsing in strict mode.
  bool strictMode = false;
  /// Enable JSX parsing.
  bool enableJSX = false;
  /// Dialect control.
  ParserDialect dialect = ParserDialect::JavaScript;
  /// Store doc-comment block at the top of the file.
  bool storeDocBlock = false;
};

enum class DiagKind : uint32_t {
  Error,
  Warning,
  Remark,
  Note,
};

DiagKind toDiagKind(llvh::SourceMgr::DiagKind k) {
  switch (k) {
    default:
      assert(false);
    case llvh::SourceMgr::DK_Error:
      return DiagKind::Error;
    case llvh::SourceMgr::DK_Warning:
      return DiagKind::Warning;
    case llvh::SourceMgr::DK_Remark:
      return DiagKind::Remark;
    case llvh::SourceMgr::DK_Note:
      return DiagKind::Note;
  }
}

struct DataRef {
  const void *data;
  size_t length;
};

template <typename T>
inline DataRef toDataRef(const T &ref) {
  return {ref.data(), ref.size()};
}

/// A coordinate in the input file identified by a 1-based line number and a
/// 0-based byte offset within that line.
struct Coord {
  /// 1-based.
  unsigned line = 0;
  /// 0-based offset from start of line.
  unsigned offset = 0;

  Coord() = default;
  Coord(int lineNo, int offset) : line(lineNo), offset(offset) {}
};

/// Result from looking for a line in the input buffer. Contains the 1-based
/// line number and a reference to the line itself in the buffer.
struct LineCoord {
  /// 1-based line number.
  unsigned lineNo = 0;
  /// Reference to the line itself, including the EOL, if present.
  DataRef lineRef;
};

/// A temporary struct describing an error message, returned to Rust.
struct DiagMessage {
  /// Location.
  SMLoc loc{};
  /// Source coordinate.
  Coord coord{};
  /// What kind of message.
  DiagKind diagKind = DiagKind::Error;
  /// Error message.
  llvh::StringRef message{};
  /// Contents of the error line.
  llvh::StringRef lineContents{};

  DiagMessage() = default;

  DiagMessage(const llvh::SMDiagnostic &diag)
      : loc(diag.getLoc()),
        coord(diag.getLineNo(), diag.getColumnNo()),
        diagKind(toDiagKind(diag.getKind())),
        message(diag.getMessage()),
        lineContents(diag.getLineContents()) {}
};

enum class MagicCommentKind : uint32_t {
  SourceUrl = 0,
  SourceMappingUrl = 1,
};

/// This object contains the entire parser state.
struct ParserContext {
  /// Parser context with allocators, string table, etc.
  Context context_{};
  /// Source buffer id, generated by SourceErrorManager.
  unsigned bufId_ = ~0u;
  /// Original error messages. We need them because they provide storage for
  /// the strings.
  std::deque<llvh::SMDiagnostic> ourMessages_{};
  /// Messages converted to the external layout.
  std::vector<DiagMessage> convertedMessages_{};

  /// Index of the first error.
  llvh::Optional<size_t> firstError_;
  /// AST.
  ESTree::ProgramNode *ast_ = nullptr;

  /// Doc block at the top of the file.
  std::string docBlock_{};

  explicit ParserContext() {
    context_.getSourceErrorManager().setDiagHandler(
        [](const llvh::SMDiagnostic &diag, void *ctx) {
          static_cast<ParserContext *>(ctx)->addMessage(diag);
        },
        this);
  }

  void setInputBuffer(llvh::StringRef str) {
    assert(!haveBufferId() && "input buffer has already been set");
    assert(str.back() == 0 && "input source must be 0-terminated");
    bufId_ = context_.getSourceErrorManager().addNewSourceBuffer(
        llvh::MemoryBuffer::getMemBuffer(str.drop_back(), "JavaScript", true));
  }

  bool haveBufferId() const {
    return bufId_ != ~0u;
  }

  unsigned getBufferId() const {
    assert(haveBufferId() && "input buffer has not been set");
    return bufId_;
  }

  void addMessage(const llvh::SMDiagnostic &diag) {
    if (diag.getKind() <= llvh::SourceMgr::DK_Error && !firstError_)
      firstError_ = ourMessages_.size();
    ourMessages_.push_back(diag);
    convertedMessages_.push_back(ourMessages_.back());
  }

  void addError(const char *msg) {
    context_.getSourceErrorManager().error(
        SMLoc{}, "Input is not zero terminated");
  }
};

} // namespace

/// source is the zero terminated input. source[len-1] must be \0.
extern "C" ParserContext *
hermes_parser_parse(ParserFlags flags, const char *source, size_t len) {
  std::unique_ptr<ParserContext> parserCtx(new ParserContext());

  parserCtx->context_.setStrictMode(flags.strictMode);
  parserCtx->context_.setParseJSX(flags.enableJSX);

  if (len == 0 || source[len - 1] != 0) {
    parserCtx->addError("Input is not zero terminated");
    return parserCtx.release();
  }

  parserCtx->setInputBuffer(llvh::StringRef(source, len));

  parserCtx->context_.setParseTS(false);
  parserCtx->context_.setParseFlow(hermes::ParseFlowSetting::NONE);

  std::vector<parser::StoredComment> comments;
  if (flags.dialect == ParserDialect::FlowDetect || flags.storeDocBlock) {
    comments = parser::getCommentsInDocBlock(
        parserCtx->context_, parserCtx->getBufferId());
  }

  switch (flags.dialect) {
    case ParserDialect::JavaScript:
      break;
    case ParserDialect::Flow:
      parserCtx->context_.setParseFlow(hermes::ParseFlowSetting::ALL);
      break;
    case ParserDialect::FlowUnambiguous:
      parserCtx->context_.setParseFlow(hermes::ParseFlowSetting::UNAMBIGUOUS);
      break;
    case ParserDialect::FlowDetect:
      parserCtx->context_.setParseFlow(
          parser::hasFlowPragma(comments) ? ParseFlowSetting::ALL
                                          : ParseFlowSetting::UNAMBIGUOUS);
      break;
    case ParserDialect::TypeScript:
      parserCtx->context_.setParseTS(true);
      break;
  }

  if (flags.storeDocBlock) {
    parserCtx->docBlock_ = parser::getDocBlock(comments);
  }

  parser::JSParser parser(
      parserCtx->context_, parserCtx->bufId_, hermes::parser::FullParse);
  auto ast = parser.parse();

  if (!parserCtx->firstError_) {
    if (!ast) {
      // Just in case.
      parserCtx->addError("Internal error");
    } else {
      parserCtx->ast_ = *ast;
    }
  }
  return parserCtx.release();
}

extern "C" void hermes_parser_free(ParserContext *parserCtx) {
  delete parserCtx;
}

/// \return the index of the first error or -1 if no errors.
extern "C" ssize_t hermes_parser_get_first_error(
    const ParserContext *parserCtx) {
  return parserCtx->firstError_ ? (int)*parserCtx->firstError_ : -1;
}

extern "C" DataRef hermes_parser_get_messages(const ParserContext *parserCtx) {
  return toDataRef(parserCtx->convertedMessages_);
}

extern "C" ESTree::ProgramNode *hermes_parser_get_ast(
    const ParserContext *parserCtx) {
  return parserCtx->ast_;
}

extern "C" bool
hermes_parser_find_location(ParserContext *parserCtx, SMLoc loc, Coord *res) {
  SourceErrorManager::SourceCoords coords;
  if (!parserCtx->context_.getSourceErrorManager().findBufferLineAndLoc(
          loc, coords)) {
    res->line = res->offset = 0;
    return false;
  }

  res->line = coords.line;
  res->offset = coords.col - 1;
  return true;
}

/// Return the line surrounding the specified location \p loc. This method
/// allows the caller to calculate the location column taking UTF-8 into
/// consideration and to perform its own location caching.
extern "C" bool hermes_parser_find_line(
    const ParserContext *parserCtx,
    SMLoc loc,
    LineCoord *res) {
  auto coord =
      parserCtx->context_.getSourceErrorManager().findBufferAndLine(loc);
  if (!coord)
    return false;

  res->lineNo = coord->lineNo;
  res->lineRef = toDataRef(coord->lineRef);
  return true;
}

/// Return a reference to the specified (1-based) line.
/// If the line is greater than the last line in the buffer, an empty
/// reference is returned.
extern "C" DataRef hermes_parser_get_line_ref(
    const ParserContext *parserCtx,
    unsigned line) {
  return toDataRef(parserCtx->context_.getSourceErrorManager().getLineRef(
      parserCtx->getBufferId(), line));
}

/// Note that we guarantee that the result is valid UTF-8 because we only
/// return it if there were no parse errors.
extern "C" DataRef hermes_parser_get_magic_comment(
    const ParserContext *parserCtx,
    MagicCommentKind kind) {
  // Make sure that we have successfully parsed the input. (The magic comments
  // could be set even if we didn't, but in that case are not guaranteed to be
  // value utf-8).
  if (!parserCtx->haveBufferId() || !parserCtx->ast_)
    return {nullptr, 0};

  llvh::StringRef res{};
  switch (kind) {
    case MagicCommentKind::SourceUrl:
      res = parserCtx->context_.getSourceErrorManager().getSourceUrl(
          parserCtx->getBufferId());
      break;
    case MagicCommentKind::SourceMappingUrl:
      res = parserCtx->context_.getSourceErrorManager().getSourceMappingUrl(
          parserCtx->getBufferId());
      break;
  }

  return toDataRef(res);
}

extern "C" DataRef hermes_get_node_name(ESTree::Node *n) {
  return toDataRef(n->getNodeName());
}

/// \return the doc block for the file if storeDocBlock was provided at
/// parse time.
extern "C" DataRef hermes_parser_get_doc_block(ParserContext *parserCtx) {
  return toDataRef(parserCtx->docBlock_);
}
