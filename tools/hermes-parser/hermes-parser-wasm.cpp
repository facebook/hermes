/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/Context.h"
#include "hermes/AST/SemValidate.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Support/Algorithms.h"

#include "llvh/ADT/StringRef.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/raw_ostream.h"

#include "HermesParserDiagHandler.h"
#include "HermesParserJSBuilder.h"

#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

using namespace hermes;

bool hasFlowPragma(Context &context, uint32_t bufferId);

/// An opaque object containing the result of parsing
class ParseResult {
 public:
  std::string error_;
  JSReference astReference_;
};

EMSCRIPTEN_KEEPALIVE
extern "C" ParseResult *
hermesParse(const char *source, size_t sourceSize, bool detectFlow) {
  std::unique_ptr<ParseResult> result = hermes::make_unique<ParseResult>();
  if (source[sourceSize - 1] != 0) {
    result->error_ = "Input source must be zero-terminated";
    return result.release();
  }

  // Set up custom diagnostic handler for error reporting
  auto context = std::make_shared<Context>();
  auto &sm = context->getSourceErrorManager();
  const auto &diagHandler = HermesParserDiagHandler(sm);

  auto fileBuf =
      llvh::MemoryBuffer::getMemBuffer(llvh::StringRef{source, sourceSize - 1});
  int fileBufId = sm.addNewSourceBuffer(std::move(fileBuf));

  auto parseFlowSetting = detectFlow && !hasFlowPragma(*context, fileBufId)
      ? ParseFlowSetting::UNAMBIGUOUS
      : ParseFlowSetting::ALL;
  context->setParseFlow(parseFlowSetting);
  context->setParseJSX(true);
  context->setUseCJSModules(true);

  parser::JSParser jsParser(*context, fileBufId, parser::FullParse);
  jsParser.setStoreComments(true);

  llvh::Optional<ESTree::ProgramNode *> parsedJs = jsParser.parse();

  // Return first error if there are any errors detected during parsing
  if (diagHandler.hasError()) {
    result->error_ = diagHandler.getErrorString();
    return result.release();
  }

  // Return generic error message if no AST was returned, but no specific errors
  // were detected.
  if (!parsedJs) {
    result->error_ = "Failed to parse source";
    return result.release();
  }

  result->astReference_ =
      buildProgram(*parsedJs, &context->getSourceErrorManager(), &jsParser);

  // Run semantic validation after AST has been transferred to JS
  sem::SemContext semContext{};
  validateASTForParser(*context, semContext, *parsedJs);

  // Return first error if errors are detected during semantic validation
  if (diagHandler.hasError()) {
    result->error_ = diagHandler.getErrorString();
    return result.release();
  }

  return result.release();
}

EMSCRIPTEN_KEEPALIVE
extern "C" void hermesParseResult_free(ParseResult *result) {
  delete result;
}

EMSCRIPTEN_KEEPALIVE
extern "C" const char *hermesParseResult_getError(const ParseResult *result) {
  if (!result || result->error_.empty()) {
    return nullptr;
  }

  return result->error_.c_str();
}

EMSCRIPTEN_KEEPALIVE
extern "C" JSReference hermesParseResult_getASTReference(
    const ParseResult *result) {
  if (!result || !result->error_.empty()) {
    return 0;
  }

  return result->astReference_;
}

bool hasFlowPragma(Context &context, uint32_t bufferId) {
  parser::JSLexer lexer(
      bufferId,
      context.getSourceErrorManager(),
      context.getAllocator(),
      &context.getStringTable(),
      context.isStrictMode());
  lexer.setStoreComments(true);

  // Determine which comments appear before the first non-directive token
  lexer.advance();
  size_t numComments = lexer.getStoredComments().size();

  while (lexer.isCurrentTokenADirective()) {
    const parser::Token *token = lexer.advance();

    // A directive token can optionally be followed by a semicolon
    if (token->getKind() == parser::TokenKind::semi) {
      lexer.advance();
    }

    numComments = lexer.getStoredComments().size();
  }

  // A flow pragma has the form @flow followed by a word boundary
  auto comments = lexer.getStoredComments().take_front(numComments);
  for (auto comment : comments) {
    if (comment.getKind() == parser::StoredComment::Kind::Hashbang) {
      continue;
    }

    llvh::StringRef value = comment.getString();

    size_t offset = value.find("@flow");
    while (offset < value.size() - 4) {
      const char *ptr = value.data() + offset;
      // Word boundary could be end of the comment
      if (offset == value.size() - 5) {
        return true;
      }

      // Word boundary if @flow is followed by any non
      // alphanumeric/underscore character.
      const char ch = ptr[5];
      if (!(((ch | 32) >= 'a' && (ch | 32) <= 'z') ||
            (ch >= '0' && ch <= '9') || ch == '_')) {
        return true;
      }

      offset = value.find("@flow", offset + 1);
    }
  }

  return false;
}

// Dummy main routine which won't actually be called by JS.
int main() {}
