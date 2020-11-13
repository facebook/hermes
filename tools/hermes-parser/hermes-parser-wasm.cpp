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

/// An opaque object containing the result of parsing
class ParseResult {
 public:
  std::string error_;
  JSReference astReference_;
};

EMSCRIPTEN_KEEPALIVE
extern "C" ParseResult *hermesParse(
    const char *source,
    size_t sourceSize,
    const char *sourceFilename,
    size_t sourceFilenameSize) {
  std::unique_ptr<ParseResult> result = hermes::make_unique<ParseResult>();
  if (source[sourceSize - 1] != 0) {
    result->error_ = "Input source must be zero-terminated";
    return result.release();
  }

  if (sourceFilename != nullptr &&
      sourceFilename[sourceFilenameSize - 1] != 0) {
    result->error_ = "Input source filename must be zero-terminated";
    return result.release();
  }

  auto context = std::make_shared<Context>();
  context->setParseJSX(true);
  context->setParseFlow(true);
  context->setUseCJSModules(true);

  // Set up custom diagnostic handler for error reporting
  auto &sm = context->getSourceErrorManager();
  const auto &diagHandler = HermesParserDiagHandler(sm, sourceFilename);

  auto fileBuf =
      llvh::MemoryBuffer::getMemBuffer(llvh::StringRef{source, sourceSize - 1});
  int fileBufId = sm.addNewSourceBuffer(std::move(fileBuf));

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

// Dummy main routine which won't actually be called by JS.
int main() {}
