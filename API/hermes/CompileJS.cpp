/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "CompileJS.h"

#include "hermes/AST/Context.h"
#include "hermes/AST/SemanticValidator.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Support/Algorithms.h"

#include "llvm/Support/SHA1.h"

namespace hermes {

bool compileJS(const std::string &str, std::string &bytecode, bool optimize) {
  ::hermes::CodeGenerationSettings codeGenOpts;
  codeGenOpts.unlimitedRegisters = false;
  ::hermes::TypeCheckerSettings typeCheckerOpts;
  ::hermes::OptimizationSettings optimizationOpts;
  optimizationOpts.aggressiveNonStrictModeOptimizations = optimize;
  optimizationOpts.inlining = optimize;
  auto context = std::make_shared<::hermes::Context>(
      codeGenOpts, typeCheckerOpts, optimizationOpts);
  sem::SemContext semCtx{};
  ::hermes::parser::JSParser jsParser(
      *context, llvm::MemoryBuffer::getMemBuffer(str));
  auto parsedJs = jsParser.parse();
  if (!parsedJs || !validateAST(*context, semCtx, *parsedJs)) {
    return false;
  }
  ::hermes::ESTree::NodePtr ast = parsedJs.getValue();
  ::hermes::Module module(context);
  ::hermes::DeclarationFileListTy declFileList;
  generateIRFromESTree(ast, &module, declFileList, {});
  if (context->getSourceErrorManager().getErrorCount() != 0) {
    return false;
  }
  llvm::raw_string_ostream bcstream(bytecode);
  BytecodeGenerationOptions opts(::hermes::EmitBundle);
  opts.optimizationEnabled = optimize;
  ::hermes::hbc::generateBytecode(
      &module,
      bcstream,
      opts,
      llvm::SHA1::hash(llvm::makeArrayRef(
          reinterpret_cast<const uint8_t *>(str.data()), str.size())));
  // Flush to string.
  bcstream.str();
  return true;
}

} // namespace hermes
