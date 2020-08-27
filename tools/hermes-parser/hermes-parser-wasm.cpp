/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/Context.h"
#include "hermes/AST/ESTreeJSONDumper.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Support/Algorithms.h"

#include "llvh/ADT/StringRef.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/raw_ostream.h"

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
  std::string jsonAst_;
};

EMSCRIPTEN_KEEPALIVE
extern "C" ParseResult *hermesParse(const char *source, size_t sourceSize) {
  std::unique_ptr<ParseResult> result = hermes::make_unique<ParseResult>();
  if (source[sourceSize - 1] != 0) {
    result->error_ = "Input source must be zero-terminated";
    return result.release();
  }

  auto context = std::make_shared<Context>();
  context->setParseJSX(true);
  context->setParseFlow(true);

  auto fileBuf =
      llvh::MemoryBuffer::getMemBuffer(llvh::StringRef{source, sourceSize - 1});
  int fileBufId =
      context->getSourceErrorManager().addNewSourceBuffer(std::move(fileBuf));

  parser::JSParser jsParser(*context, fileBufId, parser::FullParse);
  llvh::Optional<ESTree::ProgramNode *> parsedJs = jsParser.parse();

  if (!parsedJs) {
    result->error_ = "Failed to parse JS source";
    return result.release();
  }

  llvh::raw_string_ostream os{result->jsonAst_};
  dumpESTreeJSON(
      os,
      *parsedJs,
      false,
      ESTreeDumpMode::DumpAll,
      &context->getSourceErrorManager());

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
extern "C" const char *hermesParseResult_getJsonAst(const ParseResult *result) {
  if (!result || !result->error_.empty()) {
    return nullptr;
  }

  return result->jsonAst_.c_str();
}

// Dummy main routine which won't actually be called by JS.
int main() {}
