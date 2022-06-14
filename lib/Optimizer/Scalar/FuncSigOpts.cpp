/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "funcsigopts"

#include "hermes/Optimizer/Scalar/FuncSigOpts.h"
#include "hermes/Optimizer/Scalar/Utils.h"

#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/DenseSet.h"
#include "llvh/Support/Debug.h"

STATISTIC(NumParamOpt, "Number of parameters optimized");
STATISTIC(NumArgsOpt, "Number of arguments optimized");

using namespace hermes;
using llvh::dbgs;

/// \returns True if the function access arguments not through the formal
/// parameters.
static bool capturesArgumentVector(Function *F) {
  for (auto &BB : *F) {
    for (auto &I : BB) {
      if (llvh::isa<CreateArgumentsInst>(I))
        return true;
      if (auto *CB = llvh::dyn_cast<CallBuiltinInst>(&I)) {
        if (CB->getBuiltinIndex() == BuiltinMethod::HermesBuiltin_copyRestArgs)
          return true;
      }
    }
  }

  return false;
}

/// Optimize the calls to F, and the use of literals in F.
/// \returns true if some code changed.
static bool performFSO(Function *F, std::vector<Function *> &worklist) {
  LLVM_DEBUG(dbgs() << "-- Inspecting " << F->getInternalNameStr() << "\n");

  if (capturesArgumentVector(F))
    return false;

  // Generators and async functions should be treated as using all their
  // parameters. CreateGenerator is considered a user of all the
  // arguments because it stores them. The function that is called and the
  // function that actually uses the parameters are different.
  if (llvh::isa<GeneratorFunction>(F) || llvh::isa<AsyncFunction>(F))
    return false;

  IRBuilder builder(F);

  llvh::SmallVector<CallInst *, 8> callsites;
  if (!getCallSites(F, callsites))
    return false;

  LLVM_DEBUG(dbgs() << "-- Has " << callsites.size() << " call sites\n");

  unsigned numFormalParam = F->getParameters().size();

  // This vector saves the union of all callees for each parameter.
  // Null is used to mark non-literal or diverged parameters. False means
  // that the value has not been set before.
  Literal *undef = builder.getLiteralUndefined();
  llvh::SmallVector<std::pair<Literal *, bool>, 8> args(
      numFormalParam, {undef, false});

  // A list of unused arguments passed by the caller.
  llvh::SmallVector<std::pair<CallInst *, unsigned>, 8> unusedParams;

  // For each call site:
  for (CallInst *caller : callsites) {
    // For each parameter in the callee:
    for (unsigned i = 0; i < numFormalParam; i++) {
      // Get the arg that matches the i'th parameter. Unpassed parameters are
      // converted into undefs.
      Value *arg = undef;
      if (i < caller->getNumArguments() - 1)
        arg = caller->getArgument(i + 1);

      auto *L = llvh::dyn_cast<Literal>(arg);
      if (L) {
        LLVM_DEBUG(
            dbgs() << "-- Found literal for argument " << i << ": "
                   << L->getKindStr() << ".\n");
      }

      // Check if this is the first time we initialize the argument.
      if (!args[i].second) {
        args[i] = {L, true};
        continue;
      }

      // If the i'th argument from different call sites pass different values
      // then mark this argument as invalid.
      Literal *prev = args[i].first;
      if (prev != L) {
        LLVM_DEBUG(
            dbgs() << "-- Found disagreement for argument " << i << "\n");
        LLVM_DEBUG(
            dbgs() << "-- Previous value is "
                   << (prev ? prev->getKindStr() : "nullptr") << ".\n");
        args[i] = {nullptr, true};
      }
    }

    // Find the unused arguments in the call site.
    // For each argument in the call site(excluding the 'this' argument).
    for (unsigned i = 1, e = caller->getNumArguments(); i < e; i++) {
      // Remember which arguments are unused by the callee (parameters with no
      // users and undeclared parameters).
      if (i > numFormalParam || !F->getParameters()[i - 1]->hasUsers()) {
        unusedParams.push_back({caller, i});
      }
    }
  }

  LLVM_DEBUG(dbgs() << "-- Found " << unusedParams.size() << " unused args.\n");

  bool changed = false;

  unsigned paramIdx = 0;
  for (auto *P : F->getParameters()) {
    LLVM_DEBUG(dbgs() << "-- Inspecting param " << P->getName().str() << ".\n");

    if (Literal *L = args[paramIdx].first) {
      LLVM_DEBUG(
          dbgs() << "-- Found literal " << L->getKindStr() << " for param "
                 << P->getName().str() << ".\n");
      P->replaceAllUsesWith(L);
      changed = true;
      NumParamOpt++;
    }

    paramIdx++;
  }

  llvh::DenseSet<Function *> toRedo;

  // Replace all unused arguments with undef.
  for (auto &arg : unusedParams) {
    auto *prevArg = arg.first->getOperand(arg.second + 1);
    if (!llvh::isa<Literal>(prevArg))
      toRedo.insert(arg.first->getParent()->getParent());

    arg.first->setOperand(undef, arg.second + 1);
    NumArgsOpt++;
  }

  for (auto *fRedo : toRedo) {
    worklist.push_back(fRedo);
  }

  return changed;
}

bool FuncSigOpts::runOnModule(Module *M) {
  bool changed = false;

  std::vector<Function *> worklist;

  for (auto &F : *M) {
    if (M->findCJSModule(&F)) {
      // If the function is a top-level CommonJS module, skip it.
      continue;
    }

    worklist.push_back(&F);
  }

  // Process all functions.
  while (worklist.size()) {
    Function *F = worklist.back();
    worklist.pop_back();
    changed |= performFSO(F, worklist);
  }

  return changed;
}

Pass *hermes::createFuncSigOpts() {
  return new FuncSigOpts();
}

#undef DEBUG_TYPE
