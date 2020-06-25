/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/DependencyExtractor/DependencyExtractor.h"
#include "hermes/Parser/JSParser.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/raw_ostream.h"

static llvm::cl::opt<std::string> InputFilename(
    llvm::cl::desc("input file"),
    llvm::cl::Positional);

using namespace hermes;

int main(int argc, char **argv) {
  // Normalize the arg vector.
  llvm::InitLLVM initLLVM(argc, argv);
  llvm::sys::PrintStackTraceOnErrorSignal("dependency-extractor");
  llvm::PrettyStackTraceProgram X(argc, argv);
  llvm::llvm_shutdown_obj Y;
  llvm::cl::ParseCommandLineOptions(
      argc, argv, "Hermes JS dependency extractor\n");

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> fileBufOrErr =
      llvm::MemoryBuffer::getFile(InputFilename);

  if (!fileBufOrErr) {
    llvm::errs() << "Error: fail to open file: " << InputFilename << ": "
                 << fileBufOrErr.getError().message() << "\n";
    return -1;
  }

  auto fileBuf = std::move(fileBufOrErr.get());

  auto context = std::make_shared<Context>();
#if HERMES_PARSE_JSX
  context->setParseJSX(true);
#endif
#if HERMES_PARSE_FLOW
  context->setParseFlow(true);
#endif

  int fileBufId =
      context->getSourceErrorManager().addNewSourceBuffer(std::move(fileBuf));
  auto mode = parser::FullParse;

  parser::JSParser jsParser(*context, fileBufId, mode);
  llvm::Optional<ESTree::ProgramNode *> parsedJs = jsParser.parse();

  if (!parsedJs)
    return -1;

  auto deps = extractDependencies(*context, *parsedJs);
  for (const auto &dep : deps) {
    llvm::outs() << dependencyKindStr(dep.kind) << " | " << dep.name << '\n';
  }

  return 0;
}
