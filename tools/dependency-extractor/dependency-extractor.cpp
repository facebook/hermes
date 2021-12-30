/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/DependencyExtractor/DependencyExtractor.h"
#include "hermes/Parser/JSParser.h"

#include "llvh/ADT/SmallVector.h"
#include "llvh/Support/CommandLine.h"
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/InitLLVM.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/PrettyStackTrace.h"
#include "llvh/Support/Signals.h"
#include "llvh/Support/raw_ostream.h"

static llvh::cl::opt<std::string> InputFilename(
    llvh::cl::desc("input file"),
    llvh::cl::Positional);

using namespace hermes;

int main(int argc, char **argv) {
  // Normalize the arg vector.
  llvh::InitLLVM initLLVM(argc, argv);
  llvh::sys::PrintStackTraceOnErrorSignal("dependency-extractor");
  llvh::PrettyStackTraceProgram X(argc, argv);
  llvh::llvm_shutdown_obj Y;
  llvh::cl::ParseCommandLineOptions(
      argc, argv, "Hermes JS dependency extractor\n");

  llvh::ErrorOr<std::unique_ptr<llvh::MemoryBuffer>> fileBufOrErr =
      llvh::MemoryBuffer::getFile(InputFilename);

  if (!fileBufOrErr) {
    llvh::errs() << "Error: fail to open file: " << InputFilename << ": "
                 << fileBufOrErr.getError().message() << "\n";
    return -1;
  }

  auto fileBuf = std::move(fileBufOrErr.get());

  auto context = std::make_shared<Context>();
#if HERMES_PARSE_JSX
  context->setParseJSX(true);
#endif
#if HERMES_PARSE_FLOW
  context->setParseFlow(ParseFlowSetting::ALL);
#endif

  int fileBufId =
      context->getSourceErrorManager().addNewSourceBuffer(std::move(fileBuf));
  auto mode = parser::FullParse;

  parser::JSParser jsParser(*context, fileBufId, mode);
  llvh::Optional<ESTree::ProgramNode *> parsedJs = jsParser.parse();

  if (!parsedJs)
    return -1;

  auto deps = extractDependencies(*context, *parsedJs);
  for (const auto &dep : deps) {
    llvh::outs() << dependencyKindStr(dep.kind) << " | " << dep.name << '\n';
  }

  return 0;
}
