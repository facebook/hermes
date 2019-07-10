/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#define DEBUG_TYPE "closureanalysis"

#include "hermes/Optimizer/Scalar/ClosureAnalysis.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Scalar/BundlerUtils.h"
#include "hermes/Optimizer/Scalar/SetConstraintAnalysisProblem.h"
#include "hermes/Support/Statistic.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/Support/Debug.h"

using namespace hermes;
using llvm::dbgs;

/// Goals of ClosureAnalysis:
///
/// 1. For each Function, identify the call sites it may be invoked from.
///    If that set is known comprehensively, we can do a better job of
///    propagating type information from actual arguments to formals.
///
/// 2. For each CallInstruction, identify the set of all possible callees.
///    If that set is known comprehensively, then we can do a better job of
///    propagating type information from function returns to call sites.
///
/// In the first phase, we will focus on flow of closures, as long as they
/// are not stored in objects.  In the second phase, we will do flow of
/// objects and closures simultaneously.

/// Generate constraints. These are solved in SetConstraintAnalysisProblem.

using Function2Num = llvm::DenseMap<Function *, unsigned>;

STATISTIC(NumAnalysisRoots, "Number of functions taken as roots of analysis");
STATISTIC(NumNested, "Number of functions nested within analysis roots");
STATISTIC(NumOutside, "Number of functions outside analysis scope");
STATISTIC(NumInlinedRequires, "Number of requires treated inline");

/// Test if the VariableScope VS is a Function and in this analysis nest (set
/// represented by fs.)
static bool inScope(VariableScope *VS, FunctionSet &fs) {
  Function *F = dyn_cast<Function>(VS);
  return F && fs.count(F);
}

/// Collect the own properties that are stored in the object allocated by I
static void collectOwnProperties(
    Instruction &I,
    llvm::DenseSet<Literal *> &props) {
  for (auto *J : I.getUsers()) {
    if (auto *SOPI = dyn_cast<StoreOwnPropertyInst>(J)) {
      if (SOPI->getObject() == &I) {
        if (auto *propName = dyn_cast<Literal>(SOPI->getProperty()))
          props.insert(propName);
      }
    }
    if (auto *SGSI = dyn_cast<StoreGetterSetterInst>(J)) {
      if (SGSI->getObject() == &I)
        if (auto *propName = dyn_cast<Literal>(SGSI->getProperty()))
          props.insert(propName);
    }
  }
}

/// Generate a directional constraint representing the flow of information.
/// In cases where information comes from the outside, or goes outside, we
/// generate the appropriate "unknown" constraints.
static void generateInstructionConstraints(
    Instruction &I,
    SetConstraintAnalysisProblem *ap,
    FunctionSet &nested) {
  // Potential optimization: if the result of an instruction cannot carry a
  // relevant type then we do not need to make an edge.
  switch (I.getKind()) {
      // Cases appear in the order in which they appear in Instrs.def

    case ValueKind::AddEmptyStringInstKind:
      break;
    case ValueKind::AsNumberInstKind:
      break;
    case ValueKind::LoadStackInstKind: {
      auto LSI = cast<LoadStackInst>(&I);
      // Operand is essentially a memory address.
      ap->addNormalEdge(LSI->getSingleOperand(), &I);
      break;
    }
    case ValueKind::MovInstKind:
      llvm_unreachable("Lower-level instruction in Optimizer phase.");
    case ValueKind::UnaryOperatorInstKind:
      break;
#ifdef INCLUDE_HBC_BACKEND
    case ValueKind::HBCLoadConstInstKind:
    case ValueKind::HBCLoadParamInstKind:
      llvm_unreachable("Target specific instructions in Optimizer phase.");
#endif
    case ValueKind::LoadFrameInstKind: {
      auto LFI = cast<LoadFrameInst>(&I);
      Variable *V = LFI->getLoadVariable();
      if (!inScope(V->getParent(), nested)) {
        ap->addUnknownSrc(&I);
      } else {
        ap->addNormalEdge(V, &I);
      }
      break;
    }
    case ValueKind::PhiInstKind: {
      PhiInst *P = cast<PhiInst>(&I);
      for (unsigned i = 0, e = P->getNumEntries(); i < e; i++) {
        auto E = P->getEntry(i);
        ap->addNormalEdge(E.first, &I);
      }
      break;
    }
    case ValueKind::BinaryOperatorInstKind:
      break;
    case ValueKind::TryStoreGlobalPropertyInstKind:
    case ValueKind::StorePropertyInstKind: {
      auto SPI = cast<StorePropertyInst>(&I);
      if (isa<Literal>(SPI->getObject())) {
        // escape-out the value
        ap->addUnknownDst(SPI->getStoredValue());
        // escape-out the object also
        ap->addUnknownDst(SPI->getObject());
      } else {
        ap->addStoreEdge(SPI->getObject(), SPI);
      }
      break;
    }
    case ValueKind::StoreNewOwnPropertyInstKind:
    case ValueKind::StoreOwnPropertyInstKind: {
      auto SPI = cast<StoreOwnPropertyInst>(&I);
      ap->addStoreOwnEdge(SPI->getObject(), SPI);
      break;
    }
    case ValueKind::StoreGetterSetterInstKind: {
      // A function goes to unknown destination?
      auto *SGSI = cast<StoreGetterSetterInst>(&I);
      // escape-out the value
      ap->addUnknownDst(SGSI->getOperand(0));
      // escape-out the object also
      ap->addUnknownDst(SGSI->getObject());
      break;
    }
    case ValueKind::DeletePropertyInstKind: {
      auto *DPI = cast<DeletePropertyInst>(&I);
      ap->addUnknownDst(DPI->getObject());
      break;
    }
    case ValueKind::TryLoadGlobalPropertyInstKind:
    case ValueKind::LoadPropertyInstKind: {
      auto LPI = cast<LoadPropertyInst>(&I);
      // Loads of methods on literals are not supported. Note that the global
      // object is also a literal.
      // TODO Isn't it better to have this check elsewhere?
      if (isa<Literal>(LPI->getObject())) {
        ap->addUnknownSrc(LPI);
      } else {
        ap->addLoadEdge(LPI->getObject(), LPI);
      }
      break;
    }
    case ValueKind::StoreStackInstKind: {
      auto SSI = cast<StoreStackInst>(&I);
      // Operand is essentially a memory address.
      ap->addNormalEdge(SSI->getValue(), SSI->getPtr());
      break;
    }
    case ValueKind::StoreFrameInstKind: {
      auto SFI = cast<StoreFrameInst>(&I);
      Variable *V = SFI->getVariable();
      Value *W = SFI->getValue();
      if (!inScope(V->getParent(), nested)) {
        ap->addUnknownDst(W);
      } else {
        ap->addNormalEdge(W, V);
      }
      break;
    }
    case ValueKind::AllocStackInstKind:
      break;
    case ValueKind::AllocObjectInstKind: {
      auto *AI = cast<AllocObjectInst>(&I);
      llvm::DenseSet<Literal *> props;
      collectOwnProperties(I, props);
      ap->addObjectEdge(AI, props);
      if (!isa<Literal>(AI->getParentObject()) &&
          !isa<EmptySentinel>(AI->getParentObject())) {
        ap->addUnknownDst(AI->getParentObject());
      }
      break;
    }
    case ValueKind::AllocArrayInstKind: {
      // TODO: Better handling of arrays. Currently we give up.
      auto *AI = cast<AllocArrayInst>(&I);
      ap->addUnknownSrc(AI);
      // Arguments given in the initialization should doom too.
      unsigned int numElems = AI->getElementCount();
      for (unsigned int i = 0; i < numElems; i++) {
        ap->addUnknownDst(AI->getArrayElement(i), AI);
      }
      break;
    }
    case ValueKind::CreateArgumentsInstKind: {
      // Retrieve the containing function.
      Function *F = I.getParent()->getParent();
      // This causes the function and its parameters to escape.
      ap->markFunctionRoot(F);
      break;
    }
    case ValueKind::HBCGetGlobalObjectInstKind:
      break;
    case ValueKind::CatchInstKind:
      break;
    case ValueKind::DebuggerInstKind:
      break;
    case ValueKind::CreateRegExpInstKind:
      break;
    case ValueKind::TryEndInstKind:
      break;
    case ValueKind::HBCResolveEnvironmentKind:
      break;
    case ValueKind::HBCStoreToEnvironmentInstKind:
      break;
    case ValueKind::HBCLoadFromEnvironmentInstKind:
      break;
    case ValueKind::CreateFunctionInstKind: {
      CreateFunctionInst *CFI = cast<CreateFunctionInst>(&I);
      Function *F = CFI->getFunctionCode();
      ap->addFunctionEdge(F, &I);
      break;
    }
#ifdef INCLUDE_HBC_BACKEND
    case ValueKind::HBCCreateFunctionInstKind:
      llvm_unreachable("Target specific instructions in Optimizer phase.");
#endif
    case ValueKind::BranchInstKind:
      break;
    case ValueKind::ReturnInstKind: {
      auto RI = cast<ReturnInst>(&I);
      Value *retVal = RI->getOperand(0);
      Function *F = I.getParent()->getParent();
      ap->addNormalEdge(retVal, F);
      break;
    }
    case ValueKind::ThrowInstKind:
      break;
    case ValueKind::SwitchInstKind:
      break;
    case ValueKind::CondBranchInstKind:
      break;
    case ValueKind::GetPNamesInstKind:
      break;
    case ValueKind::GetNextPNameInstKind:
      break;
    case ValueKind::CheckHasInstanceInstKind:
      break;
    case ValueKind::TryStartInstKind:
      break;
    case ValueKind::CompareBranchInstKind:
      break;
    case ValueKind::SwitchImmInstKind:
      break;
    case ValueKind::CallInstKind: {
      auto CI = cast<CallInst>(&I);
      Value *C = CI->getOperand(0);
      ap->addCallEdge(C, CI);
      break;
    }
    case ValueKind::ConstructInstKind: {
      auto CI = cast<CallInst>(&I);
      Value *C = CI->getOperand(0);
      ap->addCallEdge(C, CI);
      // Currently this only handles the case in which the function
      // returns an
      // object literal, whose instances are merged (conservatively)
      // to a single one. Proper handling is future work.
      break;
    }
#ifdef INCLUDE_HBC_BACKEND
    case ValueKind::HSCConstructInstKind:
      llvm_unreachable("Target specific instructions in Optimizer phase.");
#endif

#ifdef INCLUDE_HBC_BACKEND
    case ValueKind::HBCCreateEnvironmentInstKind:
    case ValueKind::HBCGetThisInstKind:
    case ValueKind::HBCCreateThisInstKind:
    case ValueKind::HBCGetArgumentsPropByValInstKind:
    case ValueKind::HBCGetArgumentsLengthInstKind:
    case ValueKind::HBCReifyArgumentsInstKind:
    case ValueKind::HBCGetConstructedObjectInstKind:
    case ValueKind::HBCSpillMovInstKind:
      llvm_unreachable("Target specific instructions in Optimizer phase.");
#endif
    default:
      llvm_unreachable("Unhandled instruction kind.");
  }
}

/// Given an analysis root, generate constraint for the root
/// and syntactically nested functions.
static void generateConstraints(
    Function &R,
    SetConstraintAnalysisProblem *ap,
    FunctionSet &nested) {
  LLVM_DEBUG(
      dbgs() << "Generating constraints with root " << R.getInternalName().str()
             << " ...\n");

  // Root function escapes. Note that the solver will automatically ensure
  // that all parameters get unknown values, and the return escapes to unknown.
  ap->markFunctionRoot(&R);

  // Generate constraints for the nested functions (inclusive of root.)
  for (auto F : nested) {
    for (auto &bbit : *F) {
      for (auto &it : bbit) {
        Instruction *I = &it;
        generateInstructionConstraints(*I, ap, nested);
      }
    }
  }
  LLVM_DEBUG(
      dbgs() << "... finished generating constraints with root "
             << R.getInternalName().str() << "\n");
  return;
}

/// Given an analysis root, generate constraints for the root of the
/// JSmodule and nested functions.
/// If this is a dependent module, then, module.exports is captured.
/// If require'ing a dependent module, then require(...) is captured.
void ClosureAnalysis::generateConstraintsJSModule(
    Function &R,
    SetConstraintAnalysisProblem *ap,
    FunctionSet &nested,
    unsigned int modId,
    llvm::SetVector<unsigned int> &deps,
    bool isIndependent) {
  LLVM_DEBUG(
      dbgs() << "Generating constraints with JSModule "
             << R.getInternalName().str() << " independent = " << isIndependent
             << "...\n");
  ap->markFunctionRoot(&R);

  // Generate constraints for the nested functions (inclusive of root.)
  for (auto F : nested) {
    for (auto &bbit : *F) {
      for (auto &it : bbit) {
        Instruction *I = &it;

        // Special processing for requires / module.exports.

        unsigned int fmodId;
        if (bundlerUtils_.isJSModuleRequires(I, fmodId)) {
          // Is the required module marked as a dependent?
          if (deps.count(fmodId) != 0) {
            LLVM_DEBUG(
                dbgs() << "Intercepting requires in "
                       << F->getInternalName().str() << ", requiring module id "
                       << fmodId << "\n");
            NumInlinedRequires++;
            ap->addRequireModuleEdge(cast<CallInst>(I), fmodId);
            continue;
          }
        }

        Value *SV;
        // Is the module.exports from a non-independent module?
        if (!isIndependent && bundlerUtils_.isJSModuleExports(I, SV)) {
          LLVM_DEBUG(
              dbgs() << "Intercepting module.exports in "
                     << F->getInternalName().str() << ", module id " << modId
                     << "\n");
          ap->addExportModuleEdge(SV, modId);
          continue;
        }

        // Regular processing.
        generateInstructionConstraints(*I, ap, nested);
      }
    }
  }
  LLVM_DEBUG(
      dbgs() << "... finished generating constraints for "
             << R.getInternalName().str() << "\n");
  return;
}

static Function *getEnclosingFunction(Function *F) {
  // Currently we require each Function to be syntactically nested inside
  // at most one other Function.
  assert(F->getNumUsers() <= 1 && "More the one user of Function!");
  if (!F->hasUsers()) {
    // May be the CreateFunctionInst was previously eliminated for
    // some reason?
    return nullptr;
  }
  auto *CFI = dyn_cast<CreateFunctionInst>(F->getUsers().front());
  assert(CFI && "User of Function is not CreateFunctionInst");
  return CFI->getParent()->getParent();
}

static bool isAnalysisRoot(Function *F, Function2Num &cmap) {
  LLVM_DEBUG(
      dbgs() << "Checking isAnalysisRoot on " << F->getInternalName().c_str()
             << "\n");

  // If not in cmap, it is an unreachable function. We don't care.
  auto it = cmap.find(F);
  if (it == cmap.end()) {
    return false;
  }

  // Skip top level function
  if (F->isGlobalScope())
    return false;

  // We pick up every function nest in the top level!
  Function *enF = getEnclosingFunction(F);
  if (enF->isGlobalScope())
    return true;

  return false;
}

/// Traverse each function body, starting from the top-level function.
/// Count the number of createFunctionInst in each Function.
/// Recurse into the bodies of those Functions as well.
/// counts holds the counts of CreateFunctions thus nested syntactically.
static int countCreateFunctions(Function &F, Function2Num &cmap) {
  unsigned int count = 0;
  for (BasicBlock &BB : F) {
    for (Instruction &II : BB) {
      CreateFunctionInst *CFI = dyn_cast<CreateFunctionInst>(&II);
      if (!CFI)
        continue;

      count++;

      Function *CF = CFI->getFunctionCode();
      count += countCreateFunctions(*CF, cmap);
    }
  }
  cmap[&F] = count;
  return count;
}

/// Starting with an analysis root, descend down and collect the nested
/// Functions.
static void
collectNestedFunctions(Function &R, Function &F, FunctionSet &nested) {
  for (BasicBlock &BB : F) {
    for (Instruction &II : BB) {
      CreateFunctionInst *CFI = dyn_cast<CreateFunctionInst>(&II);
      if (!CFI)
        continue;

      Function *CF = CFI->getFunctionCode();

      LLVM_DEBUG(
          dbgs() << "Nested " << CF->getInternalName().str() << " in root "
                 << R.getInternalName().str() << "\n");

      nested.insert(CF);
      collectNestedFunctions(R, *CF, nested);
    }
  }
}

static void dumpNestingStats(Module *M, Function2Num &cmap) {
  unsigned int size = cmap.size();
  std::vector<int> hist;
  hist.resize(size);
  std::vector<int> cumhist;
  cumhist.resize(size);

  std::fill(hist.begin(), hist.end(), 0);
  std::fill(cumhist.begin(), cumhist.end(), 0);

  unsigned int functions = 0;
  unsigned int unreachables = 0;

  for (auto &F : *M) {
    auto it = cmap.find(&F);
    if (it == cmap.end()) {
      unreachables++;
      LLVM_DEBUG(
          dbgs() << "Unreachable function: " << F.getInternalName().str()
                 << "\n");
      continue;
    }
    auto count = it->second;
    hist[count]++;
    functions++;
  }

  LLVM_DEBUG(dbgs() << "Number of functions: " << functions << "\n");
  LLVM_DEBUG(
      dbgs() << "Number of unreachable functions: " << unreachables << "\n");

  unsigned int cumsum = 0;
  for (unsigned int i = 0; i < size; i++) {
    cumsum += hist[i];
    cumhist[i] = cumsum;
  }

  assert(
      cumsum == functions &&
      "Mismatch in the count of cumulative sums in "
      "the histogram and total number of functions.");

  for (unsigned int i = 0; i < size; i++) {
    unsigned int perc = (100 * cumhist[i]) / cumsum;
    LLVM_DEBUG(dbgs() << i << ", " << hist[i] << ", " << perc << "\n");
    if (perc >= 99) {
      break;
    }
  }
}

void ClosureAnalysis::analyzeJSModuleWithDependents(
    Function *F,
    unsigned int modId,
    SetConstraintAnalysisProblem *ap,
    bool isIndependent) {
  llvm::SetVector<unsigned int> dependents_by_id;
  llvm::DenseSet<unsigned int> imports_of_id;

  if (bundlerUtils_.getImportsOfModule(modId, imports_of_id)) {
    for (unsigned int r : imports_of_id) {
      if (bundlerUtils_.isInlineableJSModule(r)) {
        LLVM_DEBUG(
            dbgs() << "JSModule# " << modId << " carrying along JSModule# " << r
                   << "\n");
        bundlerUtils_.modId2Function(
            r); // ignore the return value, only exercise the assert
        dependents_by_id.insert(r);
      }
    }
  }

  // A flag to know if the independent JSmodule was not processed with CLA.
  bool leader_bailed_out = false;

  auto nm_it = nestedMap_.find(F);
  assert(nm_it != nestedMap_.end());
  FunctionSet &nested = nm_it->second;

  if (nested.size() >= MAX_NEST_SIZE) {
    LLVM_DEBUG(
        dbgs() << "Skipping CLA on " << F->getInternalName().str() << ", "
               << nested.size() << " nested functions is too big!\n");

    // SetConstraint analysis result is a null for these.
    analysisMap_.insert(std::make_pair(F, nullptr));

    leader_bailed_out = true;

  } else {
    LLVM_DEBUG(
        dbgs() << "Performing CLA on " << F->getInternalName().str() << ", "
               << nested.size() << "\n");

    generateConstraintsJSModule(
        *F, ap, nested, modId, dependents_by_id, isIndependent);

    analysisMap_.insert(std::make_pair(F, ap));
  }

  // Recurse into dependent JSModules.
  for (unsigned int fmodId : dependents_by_id) {
    Function *d = bundlerUtils_.modId2Function(fmodId);
    // Treat them as independent if the flag leader_bailed_out is set.
    analyzeJSModuleWithDependents(d, fmodId, ap, leader_bailed_out);
  }
}

bool ClosureAnalysis::analyzeModule(Module *M) {
  bool changed = false;
  Function2Num cmap;
  unsigned int rcount = 0;
  unsigned int ncount = 0;
  FunctionSet nested;

  Function *topF = M->getTopLevelFunction();
  countCreateFunctions(*topF, cmap);
  dumpNestingStats(M, cmap);

  for (auto &F : *M) {
    if (isAnalysisRoot(&F, cmap)) {
      LLVM_DEBUG(
          dbgs() << "\nProcessing analysis root: " << F.getInternalName().str()
                 << "\n");
      rcount++;
      analysisRoots_.insert(&F);

      nested.clear();
      collectNestedFunctions(F, F, nested);
      for (auto a : nested) {
        rootMap_.insert(std::make_pair(a, &F));
      }
      ncount += nested.size();

      nested.insert(&F);
      nestedMap_.insert(std::make_pair(&F, nested));
      // Defer the processing of analysis roots to the next loop
      // (see below.)
    }
  }

  LLVM_DEBUG(dbgs() << "\n");
  LLVM_DEBUG(dbgs() << "root count = " << rcount << "\n");
  LLVM_DEBUG(dbgs() << "nested count = " << ncount << "\n");

  NumAnalysisRoots = rcount;
  NumNested = ncount;
  NumOutside = M->getFunctionList().size() - rcount - ncount;

  BundlerKind xmform =
      M->getContext().getOptimizationSettings().crossModuleClosureAnalysis;

  bool doXM = xmform != hermes::BundlerKind::none;

  if (doXM) {
    // do this only once per M because it costs to set up the cross-module data
    // structures
    bundlerUtils_.initialize(M, nestedMap_, xmform);
  }

  for (auto F : analysisRoots_) {
    // analysisRoots_ contain all the Functions that are in the "top level".
    // Some of these are identified as "JSModules", and others are just the
    // usual function nested immediately in the "program"/"top-level function".

    unsigned int modId;
    // Is F a JSModule?
    if (doXM && bundlerUtils_.isJSModule(F, modId)) {
      // If F is an inlineable JSmodule, it will be processed along with the
      // JSmodule that requires it.
      if (bundlerUtils_.isInlineableJSModule(modId)) {
        LLVM_DEBUG(
            dbgs() << "JSModule# " << modId << " is inlineable .. skipping!\n");
        continue;
      }

      SetConstraintAnalysisProblem *ap = new SetConstraintAnalysisProblem();
      analyses_.insert(ap);

      // Generate constraints for this JSModule, as well as recurse
      // down the dependent JSmodules.  The last argument here being
      // 'true' indicates we're processing a top-level module.
      analyzeJSModuleWithDependents(F, modId, ap, true);
      ap->saturate();
      continue;
    }

    // F is NOT one of those JSModules. This is business as usual:
    // 'require' and 'module.exports' do not mean anything special.
    auto nm_it = nestedMap_.find(F);
    assert(nm_it != nestedMap_.end());
    FunctionSet &nestedForF = nm_it->second;

    if (nestedForF.size() >= MAX_NEST_SIZE) {
      LLVM_DEBUG(
          dbgs() << "Skipping CLA on " << F->getInternalName().str() << ", "
                 << nestedForF.size() << " nested functions is too big!\n");
      // SetConstraint analysis result is a null for these.
      analysisMap_.insert(std::make_pair(F, nullptr));
      continue;
    }

    LLVM_DEBUG(
        dbgs() << "Performing CLA on " << F->getInternalName().str() << ", "
               << nestedForF.size() << "\n");
    SetConstraintAnalysisProblem *ap = new SetConstraintAnalysisProblem();
    analyses_.insert(ap);
    generateConstraints(*F, ap, nestedForF);
    analysisMap_.insert(std::make_pair(F, ap));
    ap->saturate();
  }

  return changed;
}
