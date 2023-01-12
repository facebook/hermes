/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/IRGen/IRGen.h"

#include "ESTreeIRGen.h"

#include "hermes/Parser/JSParser.h"
#include "hermes/Support/SimpleDiagHandler.h"

namespace hermes {

using namespace hermes::irgen;
using llvh::dbgs;

void generateIRFromESTree(
    flow::FlowContext &flowContext,
    ESTree::NodePtr node,
    Module *M) {
  // Generate IR into the module M.
  ESTreeIRGen generator(flowContext, node, M);
  generator.doIt();
  LLVM_DEBUG(dbgs() << "Finished IRGen.\n");
}

void generateIRFromESTree(ESTree::NodePtr node, Module *M) {
  flow::FlowContext flowContext{};
  generateIRFromESTree(flowContext, node, M);
}

void generateIRForCJSModule(
    ESTree::FunctionExpressionNode *node,
    uint32_t segmentID,
    uint32_t id,
    llvh::StringRef filename,
    Module *M,
    Function *topLevelFunction) {
  // Generate IR into the module M.
  flow::FlowContext flowContext{};
  ESTreeIRGen generator(flowContext, node, M);
  return generator.doCJSModule(
      topLevelFunction, node->getSemInfo(), segmentID, id, filename);
}

} // namespace hermes
