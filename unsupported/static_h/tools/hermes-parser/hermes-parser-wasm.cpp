/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/Context.h"
#include "hermes/AST/SemValidate.h"
#include "hermes/Parser/FlowHelpers.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Support/Algorithms.h"

#include "llvh/ADT/StringRef.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/raw_ostream.h"

#include "HermesParserDiagHandler.h"
#include "HermesParserJSSerializer.h"

#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

using namespace hermes;

EMSCRIPTEN_KEEPALIVE
extern "C" ParseResult *hermesParse(
    const char *source,
    size_t sourceSize,
    bool detectFlow,
    bool tokens,
    bool allowReturnOutsideFunction) {
  std::unique_ptr<ParseResult> result = std::make_unique<ParseResult>();
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

  const auto hasFlowPragma = [](Context &context, uint32_t fileBufId) -> bool {
    std::vector<parser::StoredComment> comments =
        parser::getCommentsInDocBlock(context, fileBufId);
    return parser::hasFlowPragma(comments);
  };

  auto parseFlowSetting = detectFlow && !hasFlowPragma(*context, fileBufId)
      ? ParseFlowSetting::UNAMBIGUOUS
      : ParseFlowSetting::ALL;
  context->setParseFlow(parseFlowSetting);
  context->setParseJSX(true);
  context->setUseCJSModules(true);
  context->setAllowReturnOutsideFunction(allowReturnOutsideFunction);

  std::unique_ptr<parser::JSParser> jsParser =
      std::make_unique<parser::JSParser>(
          *context, fileBufId, parser::FullParse);
  jsParser->setStoreComments(true);
  jsParser->setStoreTokens(tokens);

  llvh::Optional<ESTree::ProgramNode *> parsedJs = jsParser->parse();

  // Return first error if there are any errors detected during parsing
  if (diagHandler.hasError()) {
    result->error_ = diagHandler.getErrorString();
    result->errorLine_ = diagHandler.getErrorLine();
    result->errorColumn_ = diagHandler.getErrorColumn();

    return result.release();
  }

  // Return generic error message if no AST was returned, but no specific errors
  // were detected.
  if (!parsedJs) {
    result->error_ = "Failed to parse source";
    return result.release();
  }

  result->context_ = context;
  result->parser_ = std::move(jsParser);
  serialize(*parsedJs, &context->getSourceErrorManager(), *result, tokens);

  // Run semantic validation after AST has been serialized
  sem::SemContext semContext{};
  validateASTForParser(*context, semContext, *parsedJs);

  // Return first error if errors are detected during semantic validation
  if (diagHandler.hasError()) {
    result->error_ = diagHandler.getErrorString();
    result->errorLine_ = diagHandler.getErrorLine();
    result->errorColumn_ = diagHandler.getErrorColumn();

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
extern "C" uint32_t hermesParseResult_getErrorLine(const ParseResult *result) {
  return result->errorLine_;
}

EMSCRIPTEN_KEEPALIVE
extern "C" uint32_t hermesParseResult_getErrorColumn(
    const ParseResult *result) {
  return result->errorColumn_;
}

EMSCRIPTEN_KEEPALIVE
extern "C" const uint32_t *hermesParseResult_getProgramBuffer(
    const ParseResult *result) {
  if (!result || !result->error_.empty()) {
    return 0;
  }

  return result->programBuffer_.data();
}

EMSCRIPTEN_KEEPALIVE
extern "C" const PositionResult *hermesParseResult_getPositionBuffer(
    const ParseResult *result) {
  if (!result || !result->error_.empty()) {
    return 0;
  }

  return result->positionBuffer_.data();
}

EMSCRIPTEN_KEEPALIVE
extern "C" size_t hermesParseResult_getPositionBufferSize(
    const ParseResult *result) {
  if (!result || !result->error_.empty()) {
    return 0;
  }

  return result->positionBuffer_.size();
}

// Dummy main routine which won't actually be called by JS.
int main() {}
