/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/IRGen/IRGen.h"

#include "ESTreeIRGen.h"

#include "hermes/Parser/JSParser.h"
#include "hermes/Sema/SemResolve.h"
#include "hermes/Support/SimpleDiagHandler.h"

namespace hermes {

using namespace hermes::irgen;
using llvh::dbgs;

void generateIRFromESTree(
    Module *M,
    sema::SemContext &semCtx,
    flow::FlowContext &flowContext,
    ESTree::NodePtr node,
    llvh::StringRef topLevelFunctionName) {
  // Generate IR into the module M.
  ESTreeIRGen generator(M, semCtx, flowContext, node);
  generator.doIt(topLevelFunctionName);
  LLVM_DEBUG(dbgs() << "Finished IRGen.\n");
}

void generateIRFromESTree(
    Module *M,
    sema::SemContext &semCtx,
    ESTree::NodePtr node,
    llvh::StringRef topLevelFunctionName) {
  flow::FlowContext flowContext{};
  generateIRFromESTree(M, semCtx, flowContext, node, topLevelFunctionName);
}

void generateIRForCJSModule(
    sema::SemContext &semContext,
    ESTree::FunctionExpressionNode *node,
    uint32_t segmentID,
    uint32_t id,
    llvh::StringRef filename,
    Module *M) {
  // Generate IR into the module M.
  flow::FlowContext flowContext{};
  ESTreeIRGen generator(M, semContext, flowContext, node);
  return generator.doCJSModule(semContext, segmentID, id, filename);
}

Function *generateLazyFunctionIR(
    Function *F,
    ESTree::FunctionLikeNode *node,
    sema::SemContext &semCtx) {
  Module *M = F->getParent();
  flow::FlowContext flowContext{};
  return ESTreeIRGen{M, semCtx, flowContext, node}.doLazyFunction(F);
}

Function *generateEvalIR(
    Module *M,
    EvalCompilationDataInst *evalDataInst,
    ESTree::FunctionLikeNode *node,
    sema::SemContext &semCtx) {
  flow::FlowContext flowContext{};
  return ESTreeIRGen{M, semCtx, flowContext, node}.doItInScope(evalDataInst);
}

} // namespace hermes
