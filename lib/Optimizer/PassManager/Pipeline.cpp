/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Optimizer/PassManager/Pipeline.h"

#include "hermes/Optimizer/PassManager/PassManager.h"
#include "hermes/Optimizer/Scalar/Auditor.h"
#include "hermes/Optimizer/Scalar/DCE.h"
#include "hermes/Optimizer/Scalar/TypeInference.h"

#include "llvh/Support/Debug.h"
#include "llvh/Support/raw_ostream.h"

#define DEBUG_TYPE "pipeline"

using llvh::dbgs;

bool hermes::runCustomOptimizationPasses(
    Module &M,
    const std::vector<std::string> &Opts) {
  LLVM_DEBUG(dbgs() << "Optimizing with custom pipeline...\n");
  PassManager PM("Custom Opts");

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
  PassManager PM("Full opts");

  auto addMem2Reg = [&PM, &M]() {
    if (M.getContext().getOptimizationSettings().useLegacyMem2Reg)
      PM.addMem2Reg();
    else
      PM.addSimpleMem2Reg();
  };

  // Add the optimization passes.

  PM.addLowerGeneratorFunction();
  // CacheNewObject benefits from running early because it needs new.target,
  // which may be eliminated by later passes. It also currently only works on
  // the `this` parameter, so it should run before inlining.
  PM.addCacheNewObject();
  // We need to fold constant strings before staticrequire.
  PM.addInstSimplify();
  PM.addResolveStaticRequire();
  // staticrequire creates some dead instructions (namely frame loads) which
  // need to be eliminated now, or the "require" parameter cannot be promoted.
  PM.addDCE();

  // Only run LowerBuiltinCallsOptimized once to avoid emitting
  // multiple fast paths for the same call.
  PM.addLowerBuiltinCallsOptimized();
  PM.addSimplifyCFG();
  PM.addSimpleStackPromotion();
  PM.addFrameLoadStoreOpts();
  addMem2Reg();
  PM.addSimpleStackPromotion();
  PM.addScopeElimination();
  PM.addFunctionAnalysis();
  PM.addInlining();
  PM.addDCE();
  PM.addObjectMergeNewStores();
  PM.addObjectStackPromotion();
  PM.addTypeInference();
  PM.addSimpleStackPromotion();
  PM.addInstSimplify();
  PM.addDCE();
  addMem2Reg();
  PM.addFunctionAnalysis();
  PM.addMetroRequire();
  PM.addInlining();
  PM.addDCE();
  // SimpleStackPromotion doesn't remove unused functions, so run it after DCE
  // to ensure unused functions aren't capturing vars.
  PM.addSimpleStackPromotion();
  PM.addFrameLoadStoreOpts();
  addMem2Reg();
  PM.addScopeElimination();
  PM.addFunctionAnalysis();
  PM.addScopeHoisting();
  PM.addObjectStackPromotion();

  // Run type inference before CSE so that we can better reason about binopt.
  PM.addTypeInference();
  PM.addCSE();
  PM.addTDZDedup();
  PM.addSimplifyCFG();

  PM.addInstSimplify();
  PM.addFuncSigOpts();
  PM.addDCE();
  PM.addSimplifyCFG();
  PM.addFrameLoadStoreOpts();
  addMem2Reg();
  PM.addAuditor();

  PM.addTypeInference();

  // Run the optimizations.
  PM.run(&M);
}

void hermes::runOptimizationPassesToFixedPoint(Module &M) {
  LLVM_DEBUG(dbgs() << "Running -O4 optimizations...\n");
  PassManager PM("Opt to fixed point");

  auto addMem2Reg = [&PM, &M]() {
    if (M.getContext().getOptimizationSettings().useLegacyMem2Reg)
      PM.addMem2Reg();
    else
      PM.addSimpleMem2Reg();
  };

  // Add the optimization passes.

  PM.addLowerGeneratorFunction();

  PM.beginFixedPointLoop("outer type inference loop");
  PM.addTypeInference();

  PM.beginFixedPointLoop("inner loop");

  // We need to fold constant strings before staticrequire.
  PM.addInstSimplify();
  PM.addResolveStaticRequire();
  // staticrequire creates some dead instructions (namely frame loads) which
  // need to be eliminated now, or the "require" parameter cannot be promoted.
  PM.addDCE();

  PM.addSimplifyCFG();
  PM.addSimpleStackPromotion();
  PM.addFrameLoadStoreOpts();
  addMem2Reg();
  PM.addScopeElimination();
  PM.addFunctionAnalysis();
  PM.addMetroRequire();
  PM.addInlining();
  PM.addObjectMergeNewStores();
  PM.addObjectStackPromotion();
  PM.addCSE();
  PM.addTDZDedup();
  PM.addFuncSigOpts();

  PM.addMetroRequire();

  PM.endFixedPointLoop(); // inner loop.
  PM.endFixedPointLoop(); // outer type inference loop

  // Auditor must always be run last -- it audits the final state of the IR.
  PM.addAuditor();

  // Run the optimizations.
  PM.run(&M);
}

void hermes::runDebugOptimizationPasses(Module &M) {
  LLVM_DEBUG(dbgs() << "Running -Og optimizations...\n");
  PassManager PM("Debug opts");

  PM.addLowerGeneratorFunction();

  PM.addInstSimplify();
  PM.addResolveStaticRequire();

  // Run the optimizations.
  PM.run(&M);
}

void hermes::runNoOptimizationPasses(Module &) {
  LLVM_DEBUG(dbgs() << "Running -O0 optimizations...\n");
}

void hermes::runNativeBackendOptimizationPasses(Module &M) {
  LLVM_DEBUG(dbgs() << "Running native backend optimizations...\n");
  PassManager PM("Native backend opts");

  PM.addStripDebugInsts();

  PM.run(&M);
}

#undef DEBUG_TYPE
