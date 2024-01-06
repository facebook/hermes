/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CompileJS.h"

#include "hermes/BCGen/HBC/BytecodeProviderFromSrc.h"
#include "hermes/Optimizer/PassManager/Pipeline.h"
#include "hermes/SourceMap/SourceMapParser.h"
#include "hermes/Support/Algorithms.h"

#include "llvh/Support/SHA1.h"

namespace hermes {

// Enbles using DiagnosticHandler with SourceErrorManager::DiagHandlerTy's API.
static void diagHandlerAdapter(const llvh::SMDiagnostic &d, void *context) {
  auto diagHandler = static_cast<DiagnosticHandler *>(context);
  DiagnosticHandler::Kind kind = DiagnosticHandler::Note;
  switch (d.getKind()) {
    case llvh::SourceMgr::DK_Error:
      kind = DiagnosticHandler::Error;
      break;
    case llvh::SourceMgr::DK_Warning:
      kind = DiagnosticHandler::Warning;
      break;
    case llvh::SourceMgr::DK_Note:
      kind = DiagnosticHandler::Note;
      break;
    default:
      assert(false && "Hermes compiler produced unexpected diagnostic kind");
  };
  // Use 1-based indexing for column to match what is used in formatted errors.
  diagHandler->handle(
      {kind,
       d.getLineNo(),
       d.getColumnNo() + 1,
       d.getMessage(),
       d.getRanges()});
}

bool compileJS(
    const std::string &str,
    const std::string &sourceURL,
    std::string &bytecode,
    bool optimize,
    bool emitAsyncBreakCheck,
    DiagnosticHandler *diagHandler,
    std::optional<std::string_view> sourceMapBuf,
    bool debug) {
  hbc::CompileFlags flags{};
  flags.debug = debug;
  flags.format = EmitBundle;
  flags.emitAsyncBreakCheck = emitAsyncBreakCheck;

  std::unique_ptr<hermes::SourceMap> sourceMap{};
  // parse the source map if one was provided
  if (sourceMapBuf.has_value()) {
    hermes::SourceErrorManager sm;
    sourceMap = hermes::SourceMapParser::parse(
        llvh::StringRef{sourceMapBuf->data(), sourceMapBuf->size()}, {}, sm);
    if (!sourceMap) {
      return false;
    }
  }

  // Note that we are relying the zero termination provided by str.data(),
  // because the parser requires it.
  auto res = hbc::BCProviderFromSrc::createBCProviderFromSrc(
      std::make_unique<hermes::Buffer>((const uint8_t *)str.data(), str.size()),
      sourceURL,
      std::move(sourceMap),
      flags,
      {} /* scopeChain */,
      diagHandler ? diagHandlerAdapter : nullptr,
      diagHandler,
      optimize ? runFullOptimizationPasses : nullptr);
  if (!res.first)
    return false;

  llvh::raw_string_ostream bcstream(bytecode);

  BytecodeGenerationOptions opts(::hermes::EmitBundle);
  opts.optimizationEnabled = optimize;

  hbc::BytecodeSerializer BS{bcstream, opts};
  BS.serialize(
      *res.first->getBytecodeModule(),
      llvh::SHA1::hash(llvh::makeArrayRef(
          reinterpret_cast<const uint8_t *>(str.data()), str.size())));

  // Flush to string.
  bcstream.flush();
  return true;
}

bool compileJS(const std::string &str, std::string &bytecode, bool optimize) {
  return compileJS(str, "", bytecode, optimize, false, nullptr);
}

bool compileJS(
    const std::string &str,
    const std::string &sourceURL,
    std::string &bytecode,
    bool optimize) {
  return compileJS(str, sourceURL, bytecode, optimize, false, nullptr);
}

} // namespace hermes
