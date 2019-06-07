/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#define DEBUG_TYPE "setconstraintanalysis"

#include "hermes/Optimizer/Scalar/SetConstraintAnalysisProblem.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/Support/Statistic.h"

#include "llvm/Support/Debug.h"

using namespace hermes;
using llvm::dbgs;
using llvm::isa;

STATISTIC(DoesNotEscape, "Number of non-escaping functions");
STATISTIC(DoesEscape, "Number of escaping functions (analysis roots escape)");
STATISTIC(NoInfoFunction, "Number of functions that appear to be un-used");
STATISTIC(
    CoveredCallSite,
    "Number of call sites in analysis scope whose callees are fully known");
STATISTIC(
    UncoveredCallSite,
    "Number of call sites in analysis scope with escaped callees");
STATISTIC(
    NoInfoCallSite,
    "Number of call sites in analysis scope with no information on callees");

STATISTIC(
    KnownLoad,
    "Number of property loads for which all reaching objects are known");
STATISTIC(
    UnknownLoad,
    "Number of property loads for which escaped objects reach");
STATISTIC(
    NoInfoLoad,
    "Number of property loads for which there is no info on reaching objects");

STATISTIC(
    UnknownAlloc,
    "Number of objects/arrays for which there are some escaped stores");
STATISTIC(
    KnownAlloc,
    "Number of object/arrays for which all owned stores are known");
STATISTIC(
    NoInfoAlloc,
    "Number of object/array for which there is no info on reached stores");

STATISTIC(
    EscapeCauseSPI,
    "Number of unknowns where cause is StorePropertyInstruction");
STATISTIC(EscapeCauseFunction, "Number of unknowns where cause is Function");
STATISTIC(
    EscapeCauseLPI,
    "Number of unknowns where cause is LoadPropertyInstruction");
STATISTIC(EscapeCauseCall, "Number of unknowns where cause is CallInst");
STATISTIC(EscapeCauseLoadFrame, "Number of unknowns where cause is LoadFrame");
STATISTIC(EscapeCauseArrayAlloc, "Number of unknowns where the cause is Array");
STATISTIC(EscapeCauseOther, "Number of unknowns where cause is Other");

void SetConstraintAnalysisProblem::addEdge(Term *src, Term *dst) {
  terms_.insert(src);
  terms_.insert(dst);

  LLVM_DEBUG(
      dbgs() << "Adding edge: "
             << "%" << src->value_id_ << ":" << src->getPrintStr() << " -> "
             << "%" << dst->value_id_ << ":" << dst->getPrintStr());

  auto it1 = src->outs_.find(dst);
  auto it2 = dst->ins_.find(src);
  if (it1 != src->outs_.end() && it2 != dst->ins_.end()) {
    LLVM_DEBUG(dbgs() << "... already exists.\n");
    return;
  }

  LLVM_DEBUG(dbgs() << "... adding to work list.\n");
  src->outs_.insert(dst);
  dst->ins_.insert(src);
  worklist_.push_back(std::make_pair(src, dst));
}

void SetConstraintAnalysisProblem::matchLambdaToCall(
    LambdaTerm *lt,
    CallTerm *ct) {
  Function *F = cast<Function>(lt->value_);
  CallInst *CI = cast<CallInst>(ct->value_);

  // Deal with the case that the CallInst is a ConstructInst
  // TODO: Is there any way we can identify owned properties?
  if (auto *CSI = dyn_cast<ConstructInst>(CI)) {
    // We pipe through a new object term as an additional value of 'this'
    // Note that there are no owned properties.
    ObjectTerm *ot = objectTermFactory(CSI);
    // Connect the object to the instruction.
    // This respects the convention that object-term always has a variable
    // associated with it in the normal-term
    NormalTerm *nt = normalTermFactory(CSI);
    addEdge(ot, nt);
    // Send the term to 'this'.
    addEdge(nt, normalTermFactory(F->getThisParameter()));
    // Also return the term as a return value.
    addEdge(nt, normalTermFactory(F));
  }

  // Propagate return, though it could be undefined.
  addEdge(normalTermFactory(F), normalTermFactory(CI));

  // Propagate 'this', though it could be undefined.
  addEdge(
      normalTermFactory(CI->getArgument(0)),
      normalTermFactory(F->getThisParameter()));

  // Propagate arguments.
  for (int i = 0, e = F->getParameters().size(); i < e; i++) {
    auto *P = F->getParameters()[i];

    // Skip the 'this' argument.
    unsigned argIdx = i + 1;

    // Load the argument that's passed in.
    if (argIdx < CI->getNumArguments()) {
      auto *A = CI->getArgument(argIdx);
      addEdge(normalTermFactory(A), normalTermFactory(P));
    }
    // If there are fewer actual arguments than formals, the remaining
    // should get undefined value, but NOT escaped value. Right?
  }

  // If there are more actual arguments than formals, the remaining actual
  // args go into arguments array.  Conservatively, we consider them escaped,
  // as they are no longer tracked precisely.
  // The -1 here is to account for the 'this'
  int extraArgs = CI->getNumArguments() - 1 - F->getParameters().size();

  while (extraArgs > 0) {
    unsigned argIdx = F->getParameters().size() + extraArgs;
    addEdge(normalTermFactory(CI->getArgument(argIdx)), escapeTermFactory(CI));
    extraArgs--;
  }
}

void SetConstraintAnalysisProblem::matchEscapeToCall(
    EscapeTerm *et,
    CallTerm *ct) {
  CallInst *CI = cast<CallInst>(ct->value_);
  int numArgs = CI->getNumArguments();

  // Escape all the arguments, including 'this'
  for (int i = 0; i < numArgs; i++) {
    auto *A = CI->getArgument(i);
    addEdge(normalTermFactory(A), et);
  }

  // Return should get value unknown as well.
  addEdge(et, normalTermFactory(CI));
}

void SetConstraintAnalysisProblem::matchLambdaToEscape(
    LambdaTerm *lt,
    EscapeTerm *et) {
  Function *F = cast<Function>(lt->value_);
  for (int i = 0, e = F->getParameters().size(); i < e; i++) {
    auto *P = F->getParameters()[i];
    addEdge(et, normalTermFactory(P));
  }
  // Escape the 'this'
  addEdge(et, normalTermFactory(F->getThisParameter()));
  // Return value should escape out.
  addEdge(normalTermFactory(F), et);
}

void SetConstraintAnalysisProblem::matchObjectToStoreOwn(
    ObjectTerm *ot,
    StoreOwnPropertyTerm *st) {
  addEdge(st, normalTermFactory(ot->value_));
}

void SetConstraintAnalysisProblem::matchObjectToStore(
    ObjectTerm *ot,
    StorePropertyTerm *st) {
  StorePropertyInst *si = (StorePropertyInst *)st->value_;
  Literal *lit = dyn_cast<Literal>(si->getProperty());
  if (!lit) {
    // havoc the object.
    addEdge(escapeTermFactory(si), normalTermFactory(ot->value_));
    // The value being stored will escape via escape -> ot.value_ -> store
    return;
  }

  if (LiteralString *l = dyn_cast<LiteralString>(lit)) {
    if (l->getValue().str().equals("prototype") ||
        l->getValue().str().equals("__proto__")) {
      // escape the value of the load
      // Revisit when we can track values through prototypes
      // keeping in mind Object.setPrototypeOf
      addEdge(escapeTermFactory(si), normalTermFactory(si));
      // store of .prototype and __proto__ do not kill the object
      return;
    }
  }

  // Revisit: What if we let un-owned stores not havoc the object?
  if (ot->owns_.count(lit) == 0) {
    // We're adding an un-owned prop. The corresponding load will
    // not be able to take advantage of this anyway.
    addEdge(escapeTermFactory(si), normalTermFactory(ot->value_));
    // Escape the value that was being stored. (Redundant?)
    addEdge(normalTermFactory(si->getStoredValue()), escapeTermFactory(si));
    return;
  }
  // The property being stored was in the owned set of the receiver.
  addEdge(st, normalTermFactory(ot->value_));
}

void SetConstraintAnalysisProblem::matchObjectToLoad(
    ObjectTerm *ot,
    LoadPropertyTerm *lt) {
  LoadPropertyInst *LI = (LoadPropertyInst *)lt->value_;
  Literal *lit = dyn_cast<Literal>(LI->getProperty());

  if (lit) {
    if (LiteralString *l = dyn_cast<LiteralString>(lit)) {
      if (l->getValue().str().equals("prototype") ||
          l->getValue().str().equals("__proto__")) {
        // escape the value of the load
        // Revisit when we can track values through prototypes
        addEdge(escapeTermFactory(LI), normalTermFactory(LI));
        // loads of .prototype and __proto__ do not kill the object
        return;
      }
    }
  }

  // If it was not an owned property, then we should return an
  // unknown value.
  if (!lit || ot->owns_.count(lit) == 0) {
    // escape the value of the load
    addEdge(escapeTermFactory(LI), normalTermFactory(LI));
    // Assuming the prototype can have anything,
    // this should havoc the object.
    addEdge(escapeTermFactory(LI), normalTermFactory(ot->value_));
    return;
  }
}

void SetConstraintAnalysisProblem::matchStoreToLoad(
    StorePropertyTerm *pt,
    LoadPropertyTerm *lt) {
  LoadPropertyInst *LI = (LoadPropertyInst *)lt->value_;
  // Need to revisit this for prototypes.
  // if (pt->prop_ == LI->getProperty()) {
  if (pt->prop_ == lt->prop_) {
    StorePropertyInst *si = (StorePropertyInst *)pt->value_;
    addEdge(normalTermFactory(si->getStoredValue()), normalTermFactory(LI));
    return;
  }
}

void SetConstraintAnalysisProblem::matchStoreOwnToLoad(
    StoreOwnPropertyTerm *pt,
    LoadPropertyTerm *lt) {
  LoadPropertyInst *LI = (LoadPropertyInst *)lt->value_;
  // Need to revisit this for prototypes.
  if (pt->prop_ == LI->getProperty()) {
    StoreOwnPropertyInst *si = (StoreOwnPropertyInst *)pt->value_;
    addEdge(normalTermFactory(si->getStoredValue()), normalTermFactory(LI));
    return;
  }
}

void SetConstraintAnalysisProblem::matchObjectToEscape(
    ObjectTerm *ot,
    EscapeTerm *et) {
  Value *V = ot->value_;
  addEdge(et, normalTermFactory(V));
}

void SetConstraintAnalysisProblem::matchStoreToEscape(
    StorePropertyTerm *st,
    EscapeTerm *et) {
  Value *V = st->value_;
  addEdge(normalTermFactory(V), et);
}

void SetConstraintAnalysisProblem::matchStoreOwnToEscape(
    StoreOwnPropertyTerm *st,
    EscapeTerm *et) {
  Value *V = st->value_;
  addEdge(normalTermFactory(V), et);
}

void SetConstraintAnalysisProblem::matchEscapeToLoad(
    EscapeTerm *et,
    LoadPropertyTerm *lpt) {
  addEdge(et, normalTermFactory(lpt->value_));
}

void SetConstraintAnalysisProblem::matchEscapeToStore(
    EscapeTerm *et,
    StorePropertyTerm *spt) {
  StorePropertyInst *si = (StorePropertyInst *)spt->value_;
  addEdge(normalTermFactory(si->getStoredValue()), et);
}

void SetConstraintAnalysisProblem::matchEscapeToStoreOwn(
    EscapeTerm *et,
    StoreOwnPropertyTerm *spt) {
  StoreOwnPropertyInst *si = (StoreOwnPropertyInst *)spt->value_;
  addEdge(normalTermFactory(si->getStoredValue()), et);
}

// TODO: The call to factory in the two methods below does not create a new
// Term.
// Mutating the prop_ could be a problem if the receiver could be either an
// array or
// an object. The right idea here would be to use IRBuilder to whip out a new
// StorePropertyInst and work with it.

void SetConstraintAnalysisProblem::matchArrayToStore(
    ArrayTerm *at,
    StorePropertyTerm *st) {
  StorePropertyInst *si = (StorePropertyInst *)st->value_;
  Function *F = si->getParent()->getParent();
  IRBuilder b(F);
  StorePropertyTerm *spt = storePropertyTermFactory(si);
  spt->prop_ = b.getLiteralString("[]");
  addEdge(spt, normalTermFactory(at->value_));
}

void SetConstraintAnalysisProblem::matchArrayToLoad(
    ArrayTerm *at,
    LoadPropertyTerm *lt) {
  LoadPropertyInst *LI = (LoadPropertyInst *)lt->value_;
  Function *F = LI->getParent()->getParent();
  IRBuilder b(F);
  LoadPropertyTerm *lpt = loadPropertyTermFactory(LI);
  lpt->prop_ = b.getLiteralString("[]");
  addEdge(lpt, normalTermFactory(at->value_));
}

void SetConstraintAnalysisProblem::matchArrayToEscape(
    ArrayTerm *at,
    EscapeTerm *et) {
  Value *V = at->value_;
  addEdge(et, normalTermFactory(V));
}

void SetConstraintAnalysisProblem::matchLambdaToLoad(
    LambdaTerm *lt,
    LoadPropertyTerm *lpt) {
  Value *v = lpt->prop_;
  if (LiteralString *l = dyn_cast<LiteralString>(v)) {
    // loads of .prototype and __proto__ do not bother us
    if ((l->getValue().str().equals("prototype")) ||
        (l->getValue().str().equals("__proto__")))
      return;
  }

  // The function escapes.
  addEdge(lt, escapeTermFactory(lpt->value_));
  // LPI gets an escaped value.
  addEdge(escapeTermFactory(lpt->value_), normalTermFactory(lpt->value_));
}

void SetConstraintAnalysisProblem::matchLambdaToStore(
    LambdaTerm *lt,
    StorePropertyTerm *spt) {
  StorePropertyInst *SPI = (StorePropertyInst *)spt->value_;
  // Stored value escapes.
  addEdge(normalTermFactory(SPI->getStoredValue()), escapeTermFactory(SPI));
  // Function also escapes, because there could be a setter on Function?
  addEdge(lt, escapeTermFactory(spt->value_));
}

bool SetConstraintAnalysisProblem::saturate() {
  LLVM_DEBUG(
      dbgs() << "Saturating ..."
             << "\n");

  while (!worklist_.empty()) {
    auto &p = worklist_.back();

    Term *src = p.first;
    Term *dst = p.second;

    LLVM_DEBUG(
        dbgs() << "Removing from worklist: "
               << "%" << src->value_id_ << ":" << src->getPrintStr() << " -> "
               << "%" << dst->value_id_ << ":" << dst->getPrintStr() << "\n");

    worklist_.pop_back();

    /// Flow via parameter passing and return

    if (LambdaTerm *lt = dyn_cast<LambdaTerm>(src)) {
      if (CallTerm *ct = dyn_cast<CallTerm>(dst)) {
        matchLambdaToCall(lt, ct);
        continue;
      }
    }

    if (CallTerm *ct = dyn_cast<CallTerm>(dst)) {
      if (EscapeTerm *et = dyn_cast<EscapeTerm>(src)) {
        matchEscapeToCall(et, ct);
        continue;
      }
    }

    if (LambdaTerm *lt = dyn_cast<LambdaTerm>(src)) {
      if (EscapeTerm *et = dyn_cast<EscapeTerm>(dst)) {
        matchLambdaToEscape(lt, et);
        continue;
      }
    }

    /// Flow via object properties

    // Flowing an object into where it is used as a receiver in StoreOwn.
    if (ObjectTerm *ot = dyn_cast<ObjectTerm>(src)) {
      if (StoreOwnPropertyTerm *st = dyn_cast<StoreOwnPropertyTerm>(dst)) {
        matchObjectToStoreOwn(ot, st);
        continue;
      }
    }

    // Flowing an object into where it is used as a receiver in Store.
    if (ObjectTerm *ot = dyn_cast<ObjectTerm>(src)) {
      if (StorePropertyTerm *st = dyn_cast<StorePropertyTerm>(dst)) {
        matchObjectToStore(ot, st);
        continue;
      }
    }

    // Flowing an object into where it is used as a receiver in Load.
    if (ObjectTerm *ot = dyn_cast<ObjectTerm>(src)) {
      if (LoadPropertyTerm *lt = dyn_cast<LoadPropertyTerm>(dst)) {
        matchObjectToLoad(ot, lt);
        continue;
      }
    }

    // The work-horse of matching stores with loads.
    if (StoreOwnPropertyTerm *pt = dyn_cast<StoreOwnPropertyTerm>(src)) {
      if (LoadPropertyTerm *lt = dyn_cast<LoadPropertyTerm>(dst)) {
        matchStoreOwnToLoad(pt, lt);
        continue;
      }
    }

    // The work-horse of matching stores with loads.
    if (StorePropertyTerm *pt = dyn_cast<StorePropertyTerm>(src)) {
      if (LoadPropertyTerm *lt = dyn_cast<LoadPropertyTerm>(dst)) {
        matchStoreToLoad(pt, lt);
        continue;
      }
    }

    // When the unique reference to an object escapes out, we intentionally
    // make the reference also get an escaped-in value, so accesses where
    // this object is a receiver see an escaped value.
    if (ObjectTerm *ot = dyn_cast<ObjectTerm>(src)) {
      if (EscapeTerm *et = dyn_cast<EscapeTerm>(dst)) {
        matchObjectToEscape(ot, et);
        continue;
      }
    }

    // Storeown of a property goes into an escaping object: stored value
    // escapes.
    if (StoreOwnPropertyTerm *pt = dyn_cast<StoreOwnPropertyTerm>(src)) {
      if (EscapeTerm *et = dyn_cast<EscapeTerm>(dst)) {
        matchStoreOwnToEscape(pt, et);
        continue;
      }
    }

    // Store of a property goes into an escaping object: stored value escapes.
    if (StorePropertyTerm *pt = dyn_cast<StorePropertyTerm>(src)) {
      if (EscapeTerm *et = dyn_cast<EscapeTerm>(dst)) {
        matchStoreToEscape(pt, et);
        continue;
      }
    }

    // Receiver of a load escapes: we get an escaped value from LPI.
    if (EscapeTerm *et = dyn_cast<EscapeTerm>(src)) {
      if (LoadPropertyTerm *lpt = dyn_cast<LoadPropertyTerm>(dst)) {
        matchEscapeToLoad(et, lpt);
        continue;
      }
    }

    // Receiver of a store escapes: stored value must escape.
    if (EscapeTerm *et = dyn_cast<EscapeTerm>(src)) {
      if (StorePropertyTerm *spt = dyn_cast<StorePropertyTerm>(dst)) {
        matchEscapeToStore(et, spt);
        continue;
      }
    }

    // Receiver of a storeown escapes: stored value must escape.
    if (EscapeTerm *et = dyn_cast<EscapeTerm>(src)) {
      if (StoreOwnPropertyTerm *spt = dyn_cast<StoreOwnPropertyTerm>(dst)) {
        matchEscapeToStoreOwn(et, spt);
        continue;
      }
    }

    /// Flow via arrays (currently we do not use these because we do not
    /// create Array Terms.

    if (ArrayTerm *at = dyn_cast<ArrayTerm>(src)) {
      if (LoadPropertyTerm *lpt = dyn_cast<LoadPropertyTerm>(dst)) {
        matchArrayToLoad(at, lpt);
        continue;
      }
    }

    if (ArrayTerm *at = dyn_cast<ArrayTerm>(src)) {
      if (StorePropertyTerm *spt = dyn_cast<StorePropertyTerm>(dst)) {
        matchArrayToStore(at, spt);
        continue;
      }
    }

    if (ArrayTerm *at = dyn_cast<ArrayTerm>(src)) {
      if (EscapeTerm *et = dyn_cast<EscapeTerm>(dst)) {
        matchArrayToEscape(at, et);
        continue;
      }
    }

    /// Unusual flows

    // When we see f.bind or f.apply, etc., for now just assume f escapes
    // In future, we can attempt a more careful treatment of these.
    if (LambdaTerm *lt = dyn_cast<LambdaTerm>(src)) {
      if (LoadPropertyTerm *lpt = dyn_cast<LoadPropertyTerm>(dst)) {
        matchLambdaToLoad(lt, lpt);
        continue;
      }
    }

    // When we see a store into a function, just do escapement of the stored
    // value.
    if (LambdaTerm *lt = dyn_cast<LambdaTerm>(src)) {
      if (StorePropertyTerm *spt = dyn_cast<StorePropertyTerm>(dst)) {
        matchLambdaToStore(lt, spt);
        continue;
      }
    }

    /// Usual edge propagation.

    if (isa<NormalTerm>(dst) || isa<ModuleTerm>(dst)) {
      for (auto it1 : dst->outs_) {
        Term *t = it1;
        addEdge(src, t);
      }
    }

    if (isa<NormalTerm>(src) || isa<ModuleTerm>(src)) {
      for (auto it2 : src->ins_) {
        Term *t = it2;
        addEdge(t, dst);
      }
    }
  }
  dumpSaturationResults();

  extractCallGraph();

  LLVM_DEBUG(dbgs() << "... done.\n");
  return false;
}

void SetConstraintAnalysisProblem::dumpSaturationResults() {
  for (auto t : terms_) {
    if (auto *lt = dyn_cast<LambdaTerm>(t)) {
      LLVM_DEBUG(
          dbgs() << "Call sites for "
                 << "%" << lt->value_id_ << ":" << lt->getPrintStr() << "\n");
      for (auto *out : lt->outs_) {
        if (dyn_cast<CallTerm>(out) || dyn_cast<EscapeTerm>(out)) {
          LLVM_DEBUG(
              dbgs() << "  "
                     << "%" << out->value_id_ << ":" << out->getPrintStr()
                     << "\n");
        }
      }
    }
    if (auto *ct = dyn_cast<CallTerm>(t)) {
      LLVM_DEBUG(
          dbgs() << "Callees at "
                 << "%" << ct->value_id_ << ":" << ct->getPrintStr() << "\n");
      for (auto *in : ct->ins_) {
        if (dyn_cast<LambdaTerm>(in) || dyn_cast<EscapeTerm>(in)) {
          LLVM_DEBUG(
              dbgs() << "  "
                     << "%" << in->value_id_ << ":" << in->getPrintStr()
                     << "\n");
        }
      }
    }
    if (auto *lpt = dyn_cast<LoadPropertyTerm>(t)) {
      LLVM_DEBUG(
          dbgs() << "Objects/Arrays reaching at "
                 << "%" << lpt->value_id_ << ":" << lpt->getPrintStr() << "\n");
      for (auto *in : lpt->ins_) {
        if (dyn_cast<ObjectTerm>(in) || dyn_cast<EscapeTerm>(in) ||
            dyn_cast<ArrayTerm>(in)) {
          LLVM_DEBUG(
              dbgs() << "  "
                     << "%" << in->value_id_ << ":" << in->getPrintStr()
                     << "\n");
        }
      }
    }
    if (auto *ot = dyn_cast<ObjectTerm>(t)) {
      auto *normt = normalTermFactory(ot->value_);
      LLVM_DEBUG(
          dbgs() << "Stores reaching at "
                 << "%" << ot->value_id_ << ":" << ot->getPrintStr() << "\n");
      for (auto *in : normt->ins_) {
        if (dyn_cast<EscapeTerm>(in) || dyn_cast<StoreOwnPropertyTerm>(in) ||
            dyn_cast<StorePropertyTerm>(in)) {
          LLVM_DEBUG(
              dbgs() << "  "
                     << "%" << in->value_id_ << ":" << in->getPrintStr()
                     << "\n");
        }
      }
    }
    if (auto *at = dyn_cast<ArrayTerm>(t)) {
      auto *normt = normalTermFactory(at->value_);
      LLVM_DEBUG(
          dbgs() << "Stores reaching at "
                 << "%" << at->value_id_ << ":" << at->getPrintStr() << "\n");
      for (auto *in : normt->ins_) {
        if (dyn_cast<EscapeTerm>(in) || dyn_cast<StorePropertyTerm>(in)) {
          LLVM_DEBUG(
              dbgs() << "  "
                     << "%" << in->value_id_ << ":" << in->getPrintStr()
                     << "\n");
        }
      }
    }
  }
}

namespace {

void profileEscapeTerm(EscapeTerm *et) {
  Value *ev = et->value_;

  if (dyn_cast<StorePropertyInst>(ev)) {
    EscapeCauseSPI++;
  } else if (dyn_cast<Function>(ev)) {
    EscapeCauseFunction++;
#ifndef NDEBUG
    Function *f = cast<Function>(ev);
    StringRef c = f->getInternalNameStr();
    std::string str(c);
    LLVM_DEBUG(dbgs() << "Escape due to Function: " << str << "\n");
#endif
  } else if (dyn_cast<LoadPropertyInst>(ev)) {
    EscapeCauseLPI++;
  } else if (dyn_cast<CallInst>(ev)) {
    EscapeCauseCall++;
  } else if (dyn_cast<LoadFrameInst>(ev)) {
    EscapeCauseLoadFrame++;
#ifndef NDEBUG
    LoadFrameInst *lfi = cast<LoadFrameInst>(ev);
    const char *c = lfi->getLocation().getPointer();
    std::string str(c, 0, 40);
    LLVM_DEBUG(dbgs() << "Escape due to LFI: " << str << "\n");
#endif
  } else if (dyn_cast<AllocArrayInst>(ev)) {
    EscapeCauseArrayAlloc++;
#ifndef NDEBUG
    // Log which array alloc was it, statically
    AllocArrayInst *ai = cast<AllocArrayInst>(ev);
    const char *c = ai->getLocation().getPointer();
    std::string str(c, 0, 40);
    LLVM_DEBUG(dbgs() << "Escape due to array: " << str << "\n");
#endif
  } else
    EscapeCauseOther++;
}

} // namespace

void SetConstraintAnalysisProblem::getCallGraph(
    llvm::DenseMap<CallInst *, llvm::DenseSet<Function *>> &callees,
    llvm::DenseMap<Function *, llvm::DenseSet<CallInst *>> &callsites,
    llvm::DenseMap<LoadPropertyInst *, llvm::DenseSet<Instruction *>>
        &receivers,
    llvm::DenseMap<Instruction *, llvm::DenseSet<Instruction *>> &stores) {
  callees = callees_;
  callsites = callsites_;
  receivers = receivers_;
  stores = stores_;
}

void SetConstraintAnalysisProblem::extractCallGraph() {
  for (auto t : terms_) {
    if (auto *lt = dyn_cast<LambdaTerm>(t)) {
      Function *F = (Function *)lt->value_;
      bool escapes = false;
      llvm::DenseSet<CallInst *> callSites;
      for (auto *out : lt->outs_) {
        if (dyn_cast<EscapeTerm>(out)) {
          escapes = true;
          DoesEscape++;
          LLVM_DEBUG(
              dbgs() << "Escaped Function " << F->getInternalName() << "\n");
          // If F escapes, then we don't care about other outs_.
          break;
        }
        if (auto *ct = dyn_cast<CallTerm>(out)) {
          callSites.insert((CallInst *)(ct->value_));
        }
      }
      if (!escapes) {
        if (callSites.size() > 0)
          DoesNotEscape++;
        else {
          NoInfoFunction++;
          LLVM_DEBUG(
              dbgs() << "Uncalled Function " << F->getInternalName() << "\n");
        }
        callsites_.insert(std::make_pair(F, callSites));
      }
    }
    if (auto *ct = dyn_cast<CallTerm>(t)) {
      CallInst *CI = (CallInst *)ct->value_;
      bool escapes = false;
      llvm::DenseSet<Function *> funcs;
      for (auto *in : ct->ins_) {
        if (dyn_cast<EscapeTerm>(in)) {
          escapes = true;
          UncoveredCallSite++;
          // If CI has escaping-in values, then we don't care about other ins_.
          break;
        }
        if (auto *lt = dyn_cast<LambdaTerm>(in)) {
          funcs.insert((Function *)(lt->value_));
        }
      }
      if (!escapes) {
        if (funcs.size() > 0)
          CoveredCallSite++;
        else
          NoInfoCallSite++;
        callees_.insert(std::make_pair(CI, funcs));
      }
    }
    if (auto *lpt = dyn_cast<LoadPropertyTerm>(t)) {
      LoadPropertyInst *LPI = (LoadPropertyInst *)lpt->value_;
      bool escapes = false;
      llvm::DenseSet<Instruction *> objs;
      for (auto *in : lpt->ins_) {
        if (isa<EscapeTerm>(in)) {
          escapes = true;
          UnknownLoad++;
          profileEscapeTerm(cast<EscapeTerm>(in));
          // If LPI has escaping values, then we don't care about other ins_.
          break;
        }
        if (isa<ObjectTerm>(in) || isa<ArrayTerm>(in)) {
          objs.insert((Instruction *)in->value_);
        }
      }
      if (!escapes) {
        if (objs.size() > 0)
          KnownLoad++;
        else
          NoInfoLoad++;
        receivers_.insert(
            std::make_pair(LPI, objs)); // OK to have objs set empty?
      }
    }
    // Split stores between Objects and Arrays? Currently we lose type info :(
    if (isa<ArrayTerm>(t) || isa<ObjectTerm>(t)) {
      Instruction *I = (Instruction *)t->value_;
      bool escapes = false;
      llvm::DenseSet<Instruction *> storeinsts;
      auto *normt = normalTermFactory(t->value_);
      for (auto *in : normt->ins_) {
        if (isa<EscapeTerm>(in)) {
          escapes = true;
          UnknownAlloc++;
          // If Object is escaping, then we do not care about anything else.
          break;
        }
        if (isa<StorePropertyTerm>(in) || isa<StoreOwnPropertyTerm>(in)) {
          storeinsts.insert((Instruction *)in->value_);
        }
      }
      if (!escapes) {
        if (storeinsts.size() > 0)
          KnownAlloc++;
        else
          NoInfoAlloc++;
        stores_.insert(std::make_pair(I, storeinsts));
      }
    }
  }
}
