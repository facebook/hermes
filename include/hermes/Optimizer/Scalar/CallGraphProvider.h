/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_SCALAR_CALLGRAPH_PROVIDER_H
#define HERMES_OPTIMIZER_SCALAR_CALLGRAPH_PROVIDER_H

#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"
#include "llvh/ADT/DenseSet.h"

namespace hermes {

class CallInst;
class Function;

/// An interface to call graph. The purpose of defining this interface is
/// that analysis clients, such as type inference, can rely on this
/// interface, where multiple implementations can exist.  Different
/// implementations can offer different cost-benefit tradeoffs.
class CallGraphProvider {
 public:
  /// Map of CallInst to (full) set of Functions that may be called
  /// from this CallInst.  If complete set is not known, the mapping
  /// does not exist.  The information contained is for CallInst
  /// in the scope of the analysis.
  llvh::DenseMap<CallInst *, llvh::DenseSet<Function *>> callees_;

  /// Map of Function to (full) set of CallInst's from where this
  /// Function may be called.  If complete set is not known, the
  /// mapping does not exist.  Note that when the scope of the analysis
  /// is one function, the mapping will contain information only for
  /// that one function; but when we move to module-level analysis
  /// such a map would carry information for multiple functions.
  llvh::DenseMap<Function *, llvh::DenseSet<CallInst *>> callsites_;

  /// Given a CallInst, are there callees that we do not know?
  /// For example, a callee could be read off of an object property
  /// which we could not analyze.
  bool hasUnknownCallees(CallInst *CI) {
    return callees_.count(CI) == 0;
  }

  /// Given a Function, are there call sites from which it may be
  /// invoked that we do not know about?  This can happen if the
  /// function escapes.
  bool hasUnknownCallsites(Function *F) {
    return callsites_.count(F) == 0;
  }

  /// Retrieve the full list of Functions that are potentially called
  /// from a call site.  If that information is not available,
  /// then there may have been unknown callees. The protocol is to
  /// call hasUnknownCallees(CI) before this function.
  llvh::DenseSet<Function *> &getKnownCallees(CallInst *CI) {
    auto a = callees_.find(CI);
    assert(a != callees_.end() && "Did not find CallInst in callees map.");
    return a->second;
  }

  /// Retrieve the full list of CallInst (call sites) from where F
  /// may be invoked.  If that information is not available, then
  /// there may have been unknown call sites. The protocol is to
  /// call hasUnknownCallsites(F) before this function.
  llvh::DenseSet<CallInst *> &getKnownCallsites(Function *F) {
    auto a = callsites_.find(F);
    assert(a != callsites_.end() && "Did not find Function in callsites map.");
    return a->second;
  }

  /// At this point, this structure holds more than just a call graph.
  /// The next two maps are about objects/arrays and loads of properties.

  /// A mapping from LoadPropertyInstruction to the set of Object (or Array)
  /// allocation instructions that may be the receiver of the LPI.
  llvh::DenseMap<LoadPropertyInst *, llvh::DenseSet<Instruction *>> receivers_;

  bool hasUnknownReceivers(LoadPropertyInst *LPI) {
    return receivers_.count(LPI) == 0;
  }

  llvh::DenseSet<Instruction *> &getKnownReceivers(LoadPropertyInst *LPI) {
    auto a = receivers_.find(LPI);
    assert(a != receivers_.end() && "Did not find LPI in receivers map.");
    return a->second;
  }

  /// A mapping from an Object (or Array) allocation instruction to the
  /// set of store instructions that store a property into the object.
  llvh::DenseMap<Instruction *, llvh::DenseSet<Instruction *>> stores_;

  bool hasUnknownStores(Instruction *I) {
    return stores_.count(I) == 0;
  }

  llvh::DenseSet<Instruction *> &getKnownStores(Instruction *I) {
    assert(llvh::isa<AllocObjectInst>(I) || llvh::isa<AllocArrayInst>(I));
    auto a = stores_.find(I);
    assert(a != stores_.end() && "Did not find I in stores map");
    return a->second;
  }
};
} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_CALLGRAPH_PROVIDER_H
