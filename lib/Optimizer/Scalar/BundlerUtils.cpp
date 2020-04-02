/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "bundlerutils"

#include "hermes/Optimizer/Scalar/BundlerUtils.h"
#include "hermes/IR/Instrs.h"
#include "llvm/Support/Debug.h"

using namespace hermes;
using llvm::dbgs;

// Metromin format
// II =
// (StorePropertyInstruction
//    (Object (Parameter (And (Position 3) IsGlobal))
//    (Property (LiteralString (Value "exports")))
//    (StoredValue V)
// )
// Note: A bundling closer to Common JS might instead implement the following
// pattern matching:
// II =
// (StorePropertyInstruction
//    (Object (Parameter (Name "module")))
//    (Property (LiteralString (Value "exports")))
//    (StoredValue V)
// )
bool BundlerUtils::isJSModuleExportsMetroMin(Instruction *II, Value *&V) {
  auto *SPI = dyn_cast<StorePropertyInst>(II);
  if (!SPI)
    return false;

  Value *P = SPI->getProperty();
  if (P->getKind() != ValueKind::LiteralStringKind)
    return false;

  LiteralString *LS = cast<LiteralString>(P);
  if (LS->getValue().str().compare("exports") != 0)
    return false;

  Value *O = SPI->getObject();
  if (O->getKind() != ValueKind::ParameterKind)
    return false;

  Parameter *PR = cast<Parameter>(O);
  if (PR->getIndexInParamList() != 2) { // Position 3
    return false;
  }

  if (!isJSModule(PR->getParent()))
    return false;

  V = SPI->getStoredValue();
  return true;
}

bool BundlerUtils::isJSModuleExports(Instruction *II, Value *&V) {
  if (xmform_ == BundlerKind::metromin)
    return isJSModuleExportsMetroMin(II, V);

  return false;
}

// Metromin bundling format
// II =
// (CallInstruction
//    (Callee (OR (Parameter (AND (Position 2) (IsJSModule)))
//                ((LoadFrameInstruction (Variable R))
//                 AND
//                 (StoreFrameInstruction (Parameter (AND (Position 2)
//                 (IsJSModule))) (Variable R))
//    (Argument_1 (LiteralNumber modId))
// )
// Note: a bundling format closer to Common JS module system might
// implement this alternate pattern:
// II =
// (CallInstruction
//    (Callee (OR (Parameter (Name "require"))
//                (LoadFrameInstruction (Variable (Name "require"))))
//    (Argument_1 (LiteralNumber modId))
// )
bool BundlerUtils::isJSModuleRequiresMetroMin(
    Instruction *II,
    unsigned int &modId) { // TODO: Fix the scoping of ModuleID
  auto *CI = dyn_cast<CallInst>(II);
  if (!CI)
    return false;

  auto *CL = CI->getCallee();

  if (CL->getKind() == ValueKind::ParameterKind) {
    Parameter *P = cast<Parameter>(CL);
    if (P->isThisParameter())
      return false;
    if (P->getIndexInParamList() != 1) { // Position 2
      return false;
    }
    if (!isJSModule(P->getParent()))
      return false;
  } else if (CL->getKind() == ValueKind::LoadFrameInstKind) {
    LoadFrameInst *LFI = cast<LoadFrameInst>(CL);
    Variable *R = LFI->getLoadVariable();

    // Find out whether there was a StoreFrame that wrote to that Variable
    enum SFIState { None, One, More };
    SFIState state = None;
    StoreFrameInst *SFI = nullptr;
    for (auto *U : R->getUsers()) {
      switch (U->getKind()) {
        case ValueKind::StoreFrameInstKind: {
          if (state == SFIState::None) {
            SFI = cast<StoreFrameInst>(U);
            state = One;
          } else if (state == SFIState::One) {
            state = More;
          }
        }
        default:
          break;
      }
    }
    if (state != SFIState::One)
      return false;

    // Now check if that SFI gets value from Parameter 1
    Value *V = SFI->getValue();
    if (V->getKind() != ValueKind::ParameterKind)
      return false;

    Parameter *P = cast<Parameter>(V);
    if (P->isThisParameter())
      return false;
    if (P->getIndexInParamList() != 1) // Position 2
      return false;
    if (!isJSModule(P->getParent()))
      return false;
  } else {
    return false;
  }

  auto *V = CI->getArgument(1);
  if (V->getKind() != ValueKind::LiteralNumberKind)
    return false;

  LiteralNumber *N = cast<LiteralNumber>(V);
  modId = N->asUInt32();

  return true;
}

bool BundlerUtils::isJSModuleRequires(Instruction *II, ModuleID &modId) {
  if (xmform_ == BundlerKind::metromin)
    return BundlerUtils::isJSModuleRequiresMetroMin(II, modId);

  return false;
}

void BundlerUtils::findFunctionRequires(
    Function *F,
    llvm::DenseSet<ModuleID> &requires) {
  LLVM_DEBUG(dbgs() << "Function " << F->getInternalName() << " requires:\n");
  for (BasicBlock &BB : *F) {
    for (Instruction &II : BB) {
      ModuleID modId;
      if (!isJSModuleRequires(&II, modId))
        continue;
      LLVM_DEBUG(dbgs() << " " << modId << "\n");
      // Record the required module; the same module could be required multiple
      // times. The set semantics of insert obviates a membership tests.
      requires.insert(modId);
    }
  }
}

// II =
// (CallInst
//    (Callee (LoadPropertyInst (globalObject) (Variable (Name "__d")))
//    (Argument_1 (CreateFunctionInstruction F))
//    (Argument_2 (LiteralNumber modId))
bool BundlerUtils::isJSModuleDefine(
    Instruction *II,
    ModuleID &modId,
    Function *&F) {
  if (xmform_ != BundlerKind::metromin)
    return false;

  auto *CI = dyn_cast<CallInst>(II);
  if (!CI)
    return false;

  auto *LPI = dyn_cast<LoadPropertyInst>(CI->getCallee());
  if (!LPI)
    return false;
  if (!isa<GlobalObject>(LPI->getObject()))
    return false;
  LiteralString *PN = dyn_cast<LiteralString>(LPI->getProperty());
  if (!PN)
    return false;
  if (PN->getValue().str().compare("__d") != 0)
    return false;

  if (CI->getNumOperands() < 4)
    return false;

  Value *modNum = CI->getArgument(2);
  if (modNum->getKind() != ValueKind::LiteralNumberKind)
    return false;

  LiteralNumber *LN = cast<LiteralNumber>(modNum);
  modId = LN->asUInt32();

  Value *anonFun = CI->getArgument(1);
  if (anonFun->getKind() != ValueKind::CreateFunctionInstKind)
    return false;

  CreateFunctionInst *CF = cast<CreateFunctionInst>(anonFun);
  F = CF->getFunctionCode();

  return true;
}

// Identify JSmodules that can be safely processed with the
// JSmodule that requires it.  For now, we only look for
// JSmodules that have out-degree of 1.  (It is possible to
// handle DAG situations too, but we'll leave it for later.)
void BundlerUtils::identifyInlineableJSModules() {
  for (auto p : revImportsMap_) {
    ModuleID modId = p.first;

    // If this JSModule is imported by more than one JSModule,
    // don't mark it.
    if (p.second.size() != 1)
      continue;

    auto it = JSmoduleTable_.find(p.first);
    if (it == JSmoduleTable_.end()) {
      llvm_unreachable("Expected to find module in the JSmoduleTable\n");
    }

    LLVM_DEBUG(
        dbgs() << "JSModule# " << modId << ", "
               << (it->second)->getInternalName().str() << " is inlineable\n");

    inlineableJSmodules_.insert(modId);
  }

  LLVM_DEBUG(dbgs() << "digraph jsmodulesinlined {\n");
  for (auto p : JSmoduleTable_) {
    ModuleID jsModId = p.first;
    auto it = importsMap_.find(jsModId);
    if (it == importsMap_.end())
      continue;
    llvm::DenseSet<ModuleID> &imports = it->second;
    for (ModuleID i : imports) {
      if (inlineableJSmodules_.count(i) != 0) {
        LLVM_DEBUG(
            dbgs() << "m" << i << " -> "
                   << "m" << jsModId << " [color=\"red\"];\n");
      } else {
        LLVM_DEBUG(
            dbgs() << "m" << i << " -> "
                   << "m" << jsModId << ";\n");
      }
    }
  }
  LLVM_DEBUG(dbgs() << "}\n");
}

void BundlerUtils::createJSModuleDependencies(Module *M) {
  // Step 1: Figure out JSmoduleTable_ and its inverse.
  Function *T = M->getTopLevelFunction();
  for (BasicBlock &BB : *T) {
    for (Instruction &II : BB) {
      ModuleID modId;
      Function *F;
      if (!BundlerUtils::isJSModuleDefine(&II, modId, F))
        continue;

      JSmoduleTable_[modId] = F;
      revJSmoduleTable_[F] = modId;

      LLVM_DEBUG(
          dbgs() << "JSModule# " << modId << " is "
                 << F->getInternalName().str() << "\n");
    }
  }

  // Step 2: Figure out importsMap_ and its inverse.
  for (auto mt_it : JSmoduleTable_) {
    ModuleID jsModId = mt_it.first;
    Function *F = mt_it.second;

    auto nm_it = nestedMap_.find(F);
    if (nm_it == nestedMap_.end()) {
      llvm_unreachable("Expected to find function in nestedMap_\n");
    }
    llvm::SetVector<Function *> funcs = nm_it->second;

    llvm::DenseSet<ModuleID> requires;

    for (Function *f : funcs) {
      BundlerUtils::findFunctionRequires(f, requires);
    }

    // Remember this set in the imports map for module jsModId.
    if (requires.size() > 0)
      importsMap_[jsModId] = requires;

    // Create the inverse of importsMap for convenience.
    for (ModuleID r : requires) {
      auto it = revImportsMap_.find(r);
      if (it == revImportsMap_.end()) {
        llvm::DenseSet<ModuleID> exports;
        exports.insert(jsModId);
        revImportsMap_[r] = exports;
      } else {
        llvm::DenseSet<ModuleID> &exports = it->second;
        exports.insert(jsModId);
      }
    }
  }
}
