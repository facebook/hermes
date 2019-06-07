/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/raw_ostream.h"

#include "hermes/AST/ASTBuilder.h"
#include "hermes/AST/Context.h"
#include "hermes/AST/ESTreeVisitors.h"
#include "hermes/AST/SemValidate.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/IRVerifier.h"
#include "hermes/IR/Instrs.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Optimizer/PassManager/Pipeline.h"
#include "hermes/Parser/JSONParser.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Utils/Dumper.h"
#include "hermes/Utils/Options.h"

#include "Config.h"

#define DEBUG_TYPE "hermes-fuzzer"

using namespace hermes;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  TypeCheckerSettings typeCheckerOpts;
  typeCheckerOpts.flowAnnotations = false;
  typeCheckerOpts.typeVerification = false;
  typeCheckerOpts.closureAnalysis = false;

  CodeGenerationSettings codeGenOpts;
  codeGenOpts.dumpOperandRegisters = false;
  codeGenOpts.dumpUseList = false;
  codeGenOpts.dumpSourceLocation = false;
  codeGenOpts.unlimitedRegisters = false;

  OptimizationSettings optimizationOpts;
  optimizationOpts.constantPropertyOptimizations = false;

  Context context(codeGenOpts, typeCheckerOpts, optimizationOpts);
  context.setStrictMode(true);
  context.setEnableEval(true);
  context.setEmitDebugInfo(true);

  auto input = llvm::StringRef((const char *)data, size);

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> FileBufOrErr =
      llvm::MemoryBuffer::getMemBufferCopy(input);

  parser::JSParser jsParser(context, std::move(FileBufOrErr.get()));
  auto parsedJs = jsParser.parse();
  if (!parsedJs)
    return 0;

  ESTree::NodePtr ast = parsedJs.getValue();

  // Don't continue processing the file if the module is not in a valid state.
  if (!validateAST(context, ast)) {
    return 0;
  }

  Module M(context);

  // A list of parsed global definition files.
  DeclarationFileListTy declFileList;
  generateIRFromESTree(ast, &M, declFileList);

  // Bail out if there were any errors. We can't ensure that the module is in
  // a valid state.
  if (auto N = context.getSourceErrorManager().getErrorCount()) {
    llvm::outs() << "Emitted " << N << " errors. exiting.\n";
    return 0;
  }

  // Optimize the module.
  runOptimizationPasses(M);

  // Print the module.
  M.dump();

  hbc::generateBytecode(&M, llvm::outs(), DumpBytecode, true);
  return 0;
}
