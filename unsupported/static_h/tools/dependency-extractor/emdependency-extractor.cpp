/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
 * This is a driver of the dependency extractor intended to be compiled to
 * WebAssembly with Emscripten and invoked from JavaScript.
 *
 * When configuring CMake, don't specify CMAKE_EXE_LINKER_FLAGS, because the
 * correct flags are already set for this target.
 *
 * DependencyExtractor.js is a module exposing the compiler interface to JS.
 */

#include "hermes/AST/Context.h"
#include "hermes/DependencyExtractor/DependencyExtractor.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Support/Algorithms.h"
#include "hermes/Support/JSONEmitter.h"

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

// Forward declaration.
class Dependencies;

extern "C" {
/// Parse the supplied source and return a Dependencies, which is an opaque
/// structure containing the extracted dependencies or an error message. The
/// result must be freed with \c hermesDependencies_free().
///
/// \param source utf-8 encoded input string. It must be zero terminated.
/// \param sourceSize the length of \c source in bytes, including the
///     terminating zero.
/// \return a new instance of Dependencies.
Dependencies *hermesExtractDependencies(const char *source, size_t sourceSize);

/// Free the Dependencies allocated by \c hermesExtractDependencies().
void hermesDependencies_free(Dependencies *res);

/// \return nullptr if compilation was successful, the error string otherwise.
const char *hermesDependencies_getError(const Dependencies *res);

/// \return a JSON representation of the dependencies as an array of objects,
/// each with a 'name' and 'kind' field.
/// [
///   {
///     "name": "foo.js",
///     "kind": "Type",
///   },
///   {
///     "name": "bar.js",
///     "kind": "Require",
///   },
/// ]
const char *hermesDependencies_getDeps(const Dependencies *res);

} // extern "C"

/// An opaque object containing the result of an extraction.
class Dependencies {
 public:
  std::string error_;
  std::string deps_;
};

EMSCRIPTEN_KEEPALIVE
extern "C" Dependencies *hermesExtractDependencies(
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

  auto deps = extractDependencies(*context, *parsedJs);
  llvh::raw_string_ostream os{result->deps_};
  JSONEmitter json{os, false};

  json.openArray();
  for (const Dependency &dep : deps) {
    json.openDict();
    json.emitKeyValue("name", dep.name);
    json.emitKeyValue("kind", dependencyKindStr(dep.kind));
    json.closeDict();
  }
  json.closeArray();

  return result.release();
}

EMSCRIPTEN_KEEPALIVE
extern "C" void hermesDependencies_free(Dependencies *res) {
  delete res;
}

EMSCRIPTEN_KEEPALIVE
extern "C" const char *hermesDependencies_getError(const Dependencies *res) {
  if (!res || res->error_.empty())
    return nullptr;
  return res->error_.c_str();
}

EMSCRIPTEN_KEEPALIVE
extern "C" const char *hermesDependencies_getDeps(const Dependencies *res) {
  if (!res || !res->error_.empty())
    return nullptr;
  return res->deps_.c_str();
}

// This is just a dummy main routine to exercise the code. It won't actually
// be called by JS.
int main() {
  static const char src1[] = "require('foo');";
  auto *res1 = hermesExtractDependencies(src1, sizeof(src1));
  assert(!hermesDependencies_getError(res1) && "success expected");
  hermesDependencies_free(res1);
  return 0;
}
