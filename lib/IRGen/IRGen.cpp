/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/IRGen/IRGen.h"
#include "hermes/BCGen/HBC/Bytecode.h"

#include "ESTreeIRGen.h"

#include "hermes/Parser/JSParser.h"
#include "hermes/Support/SimpleDiagHandler.h"

namespace hermes {

using namespace hermes::irgen;
using llvh::dbgs;

bool generateIRFromESTree(
    ESTree::NodePtr node,
    Module *M,
    const DeclarationFileListTy &declFileList,
    const ScopeChain &scopeChain) {
  // Generate IR into the module M.
  ESTreeIRGen Generator(node, declFileList, M, scopeChain);
  Generator.doIt();

  LLVM_DEBUG(dbgs() << "Finished IRGen.\n");
  return false;
}

void generateIRForCJSModule(
    ESTree::FunctionExpressionNode *node,
    uint32_t id,
    llvh::StringRef filename,
    Module *M,
    Function *topLevelFunction,
    const DeclarationFileListTy &declFileList) {
  // Generate IR into the module M.
  ESTreeIRGen generator(node, declFileList, M, {});
  return generator.doCJSModule(
      topLevelFunction, node->getSemInfo(), id, filename);
}

std::pair<Function *, Function *> generateLazyFunctionIR(
    hbc::BytecodeFunction *bcFunction,
    Module *M,
    llvh::SMRange sourceRange) {
  auto &context = M->getContext();
  auto lazyData = bcFunction->getLazyCompilationData();
  SimpleDiagHandlerRAII diagHandler{context.getSourceErrorManager()};

  AllocationScope alloc(context.getAllocator());
  sem::SemContext semCtx{};
  hermes::parser::JSParser parser(
      context, lazyData->bufferId, parser::LazyParse);

  // Note: we don't know the parent's strictness, which we need to pass, but
  // we can just use the child's strictness, which is always stricter or equal
  // to the parent's.
  parser.setStrictMode(lazyData->strictMode);

  auto parsed = parser.parseLazyFunction(
      (ESTree::NodeKind)lazyData->nodeKind, sourceRange.Start);

  // In case of error, generate a function that just throws a SyntaxError.
  if (!parsed ||
      !sem::validateFunctionAST(
          context, semCtx, *parsed, lazyData->strictMode)) {
    LLVM_DEBUG(
        llvh::dbgs() << "Lazy AST parsing/validation failed with error: "
                     << diagHandler.getErrorString());

    auto *error = ESTreeIRGen::genSyntaxErrorFunction(
        M, lazyData->originalName, sourceRange, diagHandler.getErrorString());

    return {error, error};
  }

  ESTreeIRGen generator{parsed.getValue(), {}, M, {}};
  return generator.doLazyFunction(lazyData);
}

} // namespace hermes
