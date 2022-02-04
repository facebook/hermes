/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
 * Implementation of the rust interface for hermes.
 */
#include "hermes/DependencyExtractor/DependencyExtractor.h"

#include "hermes/AST/Context.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Support/Algorithms.h"
#include "hermes/Support/JSONEmitter.h"

#include "hermes/DependencyExtractor/rust-dependency-extractor.h"
#include "llvh/ADT/StringRef.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/raw_ostream.h"

#include <string>
#include <vector>

using namespace hermes;

Dependencies *hermes::hermesExtractDependencies(
    const char *source,
    size_t sourceSize) {
  std::unique_ptr<Dependencies> result = std::make_unique<Dependencies>();

  auto context = std::make_shared<Context>();
#if HERMES_PARSE_JSX
  context->setParseJSX(true);
#endif
#if HERMES_PARSE_FLOW
  context->setParseFlow(ParseFlowSetting::ALL);
#endif

  if (source[sourceSize - 1] != 0) {
    result->error_ = "Input source must be zero-terminated";
    return result.release();
  }

  auto fileBuf =
      llvh::MemoryBuffer::getMemBuffer(llvh::StringRef{source, sourceSize - 1});
  int fileBufId =
      context->getSourceErrorManager().addNewSourceBuffer(std::move(fileBuf));
  auto mode = parser::FullParse;

  parser::JSParser jsParser(*context, fileBufId, mode);
  llvh::Optional<ESTree::ProgramNode *> parsedJs = jsParser.parse();

  if (!parsedJs) {
    result->error_ = "Failed to parse JS source";
    return result.release();
  }

  result->deps_ = extractDependencies(*context, *parsedJs);

  return result.release();
}

void hermes::hermesDependencies_free(Dependencies *res) {
  delete res;
}

const char *hermes::hermesDependencies_getError(const Dependencies *res) {
  if (!res || res->error_.empty())
    return nullptr;
  return res->error_.c_str();
}

const char *hermes::hermesDependencies_getDepKind(
    const Dependencies *res,
    size_t index) {
  if (!res || !res->error_.empty())
    return nullptr;
  return dependencyKindStr(res->deps_[index].kind);
}

const char *hermes::hermesDependencies_getDepName(
    const Dependencies *res,
    size_t index) {
  if (!res || !res->error_.empty())
    return nullptr;
  return res->deps_[index].name.c_str();
}

size_t hermes::hermesDependencies_getDepsLength(const Dependencies *res) {
  if (!res || !res->error_.empty())
    return 0;
  return res->deps_.size();
}
