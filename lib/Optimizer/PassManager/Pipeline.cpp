/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Optimizer/PassManager/Pipeline.h"

#include "hermes/Optimizer/PassManager/PassManager.h"
#include "hermes/Optimizer/Scalar/Auditor.h"
#include "hermes/Optimizer/Scalar/DCE.h"
#include "hermes/Optimizer/Scalar/SimplifyCFG.h"
#include "hermes/Optimizer/Scalar/StackPromotion.h"
#include "hermes/Optimizer/Scalar/TypeInference.h"

#include "llvh/Support/Debug.h"
#include "llvh/Support/raw_ostream.h"

#define DEBUG_TYPE "pipeline"

using namespace hermes;
using llvh::dbgs;
using llvh::raw_ostream;

bool hermes::runCustomOptimizationPasses(
    Module &M,
    const std::vector<std::string> &Opts) {
  LLVM_DEBUG(dbgs() << "Optimizing with custom pipeline...\n");
  PassManager PM;

  // Add the optimization passes.
  for (auto P : Opts) {
    if (!PM.addPassForName(P)) {
      return false;
    }
  }
  // Run the optimizations.
  PM.run(&M);
  return true;
}

void hermes::runFullOptimizationPasses(Module &M) {
  LLVM_DEBUG(dbgs() << "Running -O3 optimizations...\n");
  PassManager PM;

  // Add the optimization passes.

  // We need to fold constant strings before staticrequire.
  PM.addInstSimplify();
  PM.addResolveStaticRequire();
  // staticrequire creates some dead instructions (namely frame loads) which
  // need to be eliminated now, or the "require" parameter cannot be promoted.
  PM.addDCE();

  PM.addTypeInference();
  PM.addSimplifyCFG();
  PM.addStackPromotion();
  PM.addMem2Reg();
  PM.addStackPromotion();
  PM.addInlining();
  PM.addStackPromotion();
  PM.addInstSimplify();
  PM.addDCE();

  // Run type inference before CSE so that we can better reason about binopt.
  PM.addTypeInference();
  PM.addCSE();
  PM.addTDZDedup();
  PM.addSimplifyCFG();

  PM.addInstSimplify();
  PM.addFuncSigOpts();
  PM.addDCE();
  PM.addSimplifyCFG();
  PM.addMem2Reg();
  PM.addAuditor();

  PM.addTypeInference();

  // Move StartGenerator instructions to the start of functions.
  PM.addHoistStartGenerator();

  // Run the optimizations.
  PM.run(&M);
}

void hermes::runDebugOptimizationPasses(Module &M) {
  LLVM_DEBUG(dbgs() << "Running -Og optimizations...\n");
  PassManager PM;

  PM.addInstSimplify();
  PM.addResolveStaticRequire();

  // Move StartGenerator instructions to the start of functions.
  PM.addHoistStartGenerator();

  // Run the optimizations.
  PM.run(&M);
}

void hermes::runNoOptimizationPasses(Module &) {
  LLVM_DEBUG(dbgs() << "Running -O0 optimizations...\n");
}
