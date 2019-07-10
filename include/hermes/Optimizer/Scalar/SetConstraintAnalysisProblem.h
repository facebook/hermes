/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_OPTIMIZER_SCALAR_SET_CONSTRAINT_ANALYSIS_PROBLEM_H
#define HERMES_OPTIMIZER_SCALAR_SET_CONSTRAINT_ANALYSIS_PROBLEM_H

#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Scalar/Term.h"
#include "llvm/ADT/DenseSet.h"

namespace hermes {

/// SetConstraintAnalysisProblem contains the machinery to set up a
/// constraint graph, and saturate it.  The nodes in the constraint
/// graph are various Terms.  The directed edges represent flow of
/// values through the program.  An initial set of nodes and edges
/// are created by syntactic traversal of the IR (see ClosureAnalysis).
///
/// After that, a saturation process computes a dynamic transitive
/// closure of the graph, introducing additional edges (and sometimes
/// nodes) as needed.  The addition of edges during saturation is
/// triggered by match_X_to_Y methods, where X and Y are kinds of
/// terms.  See, for example, comment at matchLambdaToCall below.
///
/// The syntactic constraint generation procedure adds escape terms
/// to indicate unknown sources and unknown destinations. Additional
/// escape terms may be introduced during the saturation process.
class SetConstraintAnalysisProblem {
  /// Mapping from Values to unique IDs, used for printing.
  llvm::DenseMap<Value *, unsigned> valueRegistry_;

  /// A counter to help assign unique ids.
  unsigned seq_{0};

  /// Obtain a unique ID per value
  unsigned getUniqueValueID(Value *V) {
    auto it = valueRegistry_.find(V);
    if (it == valueRegistry_.end()) {
      unsigned val = seq_;
      valueRegistry_[V] = val;
      seq_++;
      return val;
    }
    return it->second;
  }

  /// It is an important property of the analysis that Terms with the
  /// same value in them are equal (by pointer equality.)  So we maintain
  /// a registry of terms. NEVER create a Term without first consulting
  /// the registry.  The total number of terms in the registries, all
  /// together, will be proportional to the number of distinct Values in
  /// the IR.
  /// Registries and factories to help implement the above invariant.
  llvm::DenseMap<Value *, Term *> normalTermRegistry_;
  llvm::DenseMap<Value *, Term *> escapeTermRegistry_;
  llvm::DenseMap<Value *, Term *> callTermRegistry_;
  llvm::DenseMap<Value *, Term *> lambdaTermRegistry_;

  llvm::DenseMap<Value *, Term *> objectTermRegistry_;
  llvm::DenseMap<Value *, Term *> loadPropertyTermRegistry_;
  llvm::DenseMap<Value *, Term *> storeOwnPropertyTermRegistry_;
  llvm::DenseMap<Value *, Term *> storePropertyTermRegistry_;
  llvm::DenseMap<Value *, Term *> propertyTermRegistry_;
  llvm::DenseMap<Value *, Term *> arrayTermRegistry_;

  llvm::DenseMap<unsigned int, Term *> moduleTermRegistry_;

  NormalTerm *normalTermFactory(Value *v) {
    auto it = normalTermRegistry_.find(v);
    if (it == normalTermRegistry_.end()) {
      NormalTerm *newt = new NormalTerm(v, getUniqueValueID(v));
      normalTermRegistry_[v] = newt;
      return newt;
    }
    return (NormalTerm *)it->second;
  }

  LambdaTerm *lambdaTermFactory(Function *f) {
    auto it = lambdaTermRegistry_.find(f);
    if (it == lambdaTermRegistry_.end()) {
      LambdaTerm *newt = new LambdaTerm(f, getUniqueValueID(f));
      lambdaTermRegistry_[f] = newt;
      return newt;
    }
    return (LambdaTerm *)it->second;
  }

  CallTerm *callTermFactory(CallInst *c) {
    auto it = callTermRegistry_.find(c);
    if (it == callTermRegistry_.end()) {
      CallTerm *newt = new CallTerm(c, getUniqueValueID(c));
      callTermRegistry_[c] = newt;
      return newt;
    }
    return (CallTerm *)it->second;
  }

  EscapeTerm *escapeTermFactory(Value *v) {
    auto it = escapeTermRegistry_.find(v);
    if (it == escapeTermRegistry_.end()) {
      EscapeTerm *newt = new EscapeTerm(v, getUniqueValueID(v));
      escapeTermRegistry_[v] = newt;
      return newt;
    }
    return (EscapeTerm *)it->second;
  }

  ObjectTerm *objectTermFactory(
      AllocObjectInst *v,
      llvm::DenseSet<Literal *> &properties) {
    auto it = objectTermRegistry_.find(v);
    if (it == objectTermRegistry_.end()) {
      ObjectTerm *newt = new ObjectTerm(v, properties, getUniqueValueID(v));
      objectTermRegistry_[v] = newt;
      return newt;
    }
    return (ObjectTerm *)it->second;
  }

  ObjectTerm *objectTermFactory(ConstructInst *v) {
    llvm::DenseSet<Literal *> properties;
    auto it = objectTermRegistry_.find(v);
    if (it == objectTermRegistry_.end()) {
      ObjectTerm *newt = new ObjectTerm(v, properties, getUniqueValueID(v));
      objectTermRegistry_[v] = newt;
      return newt;
    }
    return (ObjectTerm *)it->second;
  }

  ArrayTerm *arrayTermFactory(AllocArrayInst *v) {
    auto it = arrayTermRegistry_.find(v);
    if (it == arrayTermRegistry_.end()) {
      ArrayTerm *newt = new ArrayTerm(v, getUniqueValueID(v));
      arrayTermRegistry_[v] = newt;
      return newt;
    }
    return (ArrayTerm *)it->second;
  }

  LoadPropertyTerm *loadPropertyTermFactory(LoadPropertyInst *v) {
    auto it = loadPropertyTermRegistry_.find(v);
    if (it == loadPropertyTermRegistry_.end()) {
      Value *prop = v->getProperty();
      LoadPropertyTerm *newt =
          new LoadPropertyTerm(v, prop, getUniqueValueID(v));
      loadPropertyTermRegistry_[v] = newt;
      return newt;
    }
    return (LoadPropertyTerm *)it->second;
  }

  StoreOwnPropertyTerm *storeOwnPropertyTermFactory(StoreOwnPropertyInst *v) {
    auto it = storeOwnPropertyTermRegistry_.find(v);
    if (it == storeOwnPropertyTermRegistry_.end()) {
      Value *prop = v->getProperty();
      StoreOwnPropertyTerm *newt =
          new StoreOwnPropertyTerm(v, prop, getUniqueValueID(v));
      storeOwnPropertyTermRegistry_[v] = newt;
      return newt;
    }
    return (StoreOwnPropertyTerm *)it->second;
  }

  StorePropertyTerm *storePropertyTermFactory(StorePropertyInst *v) {
    auto it = storePropertyTermRegistry_.find(v);
    if (it == storePropertyTermRegistry_.end()) {
      Value *prop = v->getProperty();
      StorePropertyTerm *newt =
          new StorePropertyTerm(v, prop, getUniqueValueID(v));
      storePropertyTermRegistry_[v] = newt;
      return newt;
    }
    return (StorePropertyTerm *)it->second;
  }

  PropertyTerm *propertyTermFactory(Value *v, Value *prop, Value *obj) {
    auto it = propertyTermRegistry_.find(v);
    if (it == propertyTermRegistry_.end()) {
      PropertyTerm *newt = new PropertyTerm(v, prop, obj, getUniqueValueID(v));
      propertyTermRegistry_[v] = newt;
      return newt;
    }
    return (PropertyTerm *)it->second;
  }

  ModuleTerm *moduleTermFactory(unsigned int modId) {
    auto it = moduleTermRegistry_.find(modId);
    if (it == moduleTermRegistry_.end()) {
      // The third argument is unique within JSmodules, but does not use
      // the getUniqueValueID mechanism because there is no Value associated
      // with the JSModule.
      ModuleTerm *newt = new ModuleTerm(nullptr, modId, modId);
      moduleTermRegistry_[modId] = newt;
      return newt;
    }
    return (ModuleTerm *)it->second;
  }

  /// A helper function to introduce edges into the constraint graph.
  void addEdge(Term *src, Term *dst);

  /// A helper function to debug-output saturation results.
  void dumpSaturationResults();

  void extractCallGraph();

  llvm::DenseMap<CallInst *, llvm::DenseSet<Function *>> callees_;
  llvm::DenseMap<Function *, llvm::DenseSet<CallInst *>> callsites_;
  llvm::DenseMap<LoadPropertyInst *, llvm::DenseSet<Instruction *>> receivers_;
  llvm::DenseMap<Instruction *, llvm::DenseSet<Instruction *>> stores_;

  /// Handle the case in which a LambdaTerm flows into a CallTerm
  /// A Function (represented as a LambdaTerm has flowed into a
  /// CallSite (represented as a CallTerm) as its callee.
  /// In this case, we introduce flow edges from actuals to formals,
  /// and from function return (represented by the Function as
  /// a NormalTerm) to the call site (represented by the CallInst as a
  /// NormalTerm.)
  void matchLambdaToCall(LambdaTerm *lt, CallTerm *ct);

  /// Handle the case in which an unknown Term flows into a CallTerm
  /// Make actuals escape.  CallInst produces unknown value.
  void matchEscapeToCall(EscapeTerm *et, CallTerm *ct);

  /// Handle the case in which a LambdaTerm (Function) escapes to
  /// out of the analysis scope. Make formals escape-in and return
  /// value escapes too.
  void matchLambdaToEscape(LambdaTerm *lt, EscapeTerm *et);

  /// When an object reaches a storeOwnProp instruction (as its base
  /// pointer, we introduce a Prop term flowing into the Object.
  /// Object(L) --> o.Store(val, o, prop)  =>  Prop(prop, val) --> L
  /// Prop stands for the common supertype of StoreOwn and Store
  void matchObjectToStoreOwn(ObjectTerm *ot, StoreOwnPropertyTerm *st);
  void matchObjectToStore(ObjectTerm *ot, StorePropertyTerm *st);

  /// When an object reaches a loadProp as base pointer.
  /// Object(...) ---> Load(_, prop),
  /// we make sure the prop was in the owned set of Object
  /// Actual propagation of values is via PropTerm -> LoadTerm
  void matchObjectToLoad(ObjectTerm *ot, LoadPropertyTerm *lt);

  /// When a property term reaches a loadProperty (as its base pointer),
  /// we make the Value flow into the result of the load instruction.
  /// Prop(prop, val) ---> Load(_, prop) => val -> LI
  void matchStoreToLoad(StorePropertyTerm *pt, LoadPropertyTerm *lt);
  void matchStoreOwnToLoad(StoreOwnPropertyTerm *pt, LoadPropertyTerm *lt);

  /// Object(L) ---> Escape =>  Escape(..) --> L
  void matchObjectToEscape(ObjectTerm *ot, EscapeTerm *et);

  /// Prop(v, foo) --> Escape => v --> Escape
  void matchStoreToEscape(StorePropertyTerm *st, EscapeTerm *et);
  void matchStoreOwnToEscape(StoreOwnPropertyTerm *st, EscapeTerm *et);

  /// Escape --> Load(_, prop)  ==>   Escape -> LI
  void matchEscapeToLoad(EscapeTerm *et, LoadPropertyTerm *lpt);

  /// Escape --> Store(_ , value, prop) ==> value -> Escape
  void matchEscapeToStore(EscapeTerm *et, StorePropertyTerm *spt);
  void matchEscapeToStoreOwn(EscapeTerm *et, StoreOwnPropertyTerm *spt);

  void matchArrayToStore(ArrayTerm *at, StorePropertyTerm *st);
  void matchArrayToLoad(ArrayTerm *at, LoadPropertyTerm *lpt);
  void matchArrayToEscape(ArrayTerm *at, EscapeTerm *et);

  /// Access of any property from a Function causes the Function to escape
  void matchLambdaToLoad(LambdaTerm *lt, LoadPropertyTerm *lpt);

  /// Storing a value into a function (currently) causes the value to escape.
  void matchLambdaToStore(LambdaTerm *lt, StorePropertyTerm *spt);

  /// Keep track of all the terms introduced in the graph. These are
  /// the union of terms in various registries, and could possibly
  /// be retrieved that way. Retire?
  llvm::DenseSet<Term *> terms_;

  /// A worklist used in the saturation process.
  std::vector<std::pair<Term *, Term *>> worklist_;

 public:
  SetConstraintAnalysisProblem() {}

  ~SetConstraintAnalysisProblem() {
    for (auto t : normalTermRegistry_) {
      delete t.second;
    }
    for (auto t : lambdaTermRegistry_) {
      delete t.second;
    }
    for (auto t : callTermRegistry_) {
      delete t.second;
    }
    for (auto t : escapeTermRegistry_) {
      delete t.second;
    }
    for (auto t : objectTermRegistry_) {
      delete t.second;
    }
    for (auto t : loadPropertyTermRegistry_) {
      delete t.second;
    }
    for (auto t : storeOwnPropertyTermRegistry_) {
      delete t.second;
    }
    for (auto t : storePropertyTermRegistry_) {
      delete t.second;
    }
    for (auto t : propertyTermRegistry_) {
      delete t.second;
    }
    for (auto t : arrayTermRegistry_) {
      delete t.second;
    }
    normalTermRegistry_.clear();
    lambdaTermRegistry_.clear();
    callTermRegistry_.clear();
    escapeTermRegistry_.clear();
    objectTermRegistry_.clear();
    loadPropertyTermRegistry_.clear();
    storeOwnPropertyTermRegistry_.clear();
    storePropertyTermRegistry_.clear();
    propertyTermRegistry_.clear();
    arrayTermRegistry_.clear();

    callsites_.clear();
    callees_.clear();
    receivers_.clear();
    stores_.clear();
  }

  /// Read off the call graph implied by the saturated graph.
  void getCallGraph(
      llvm::DenseMap<CallInst *, llvm::DenseSet<Function *>> &callees,
      llvm::DenseMap<Function *, llvm::DenseSet<CallInst *>> &callsites,
      llvm::DenseMap<LoadPropertyInst *, llvm::DenseSet<Instruction *>>
          &receivers,
      llvm::DenseMap<Instruction *, llvm::DenseSet<Instruction *>> &stores);

  /// Saturate computes the dynamic transitive closure of the constraint
  /// graph. This is the main workhorse of the constraint solving.
  bool saturate();

  /// The methods below are used by a syntactic traversal of the IR
  /// to generate constraints.

  /// Adds a simple "flow" edge, saying that whatever values arrive at src,
  /// flow into dst.
  void addNormalEdge(Value *src, Value *dst) {
    addEdge(normalTermFactory(src), normalTermFactory(dst));
  };

  /// At CreateFunction, the Function value (src) flows into the destination
  /// of the CreateFunction instruction (dst)
  void addFunctionEdge(Function *src, Value *dst) {
    addEdge(lambdaTermFactory(src), normalTermFactory(dst));
  };

  /// Callee of a CallInst flows the callee's incoming (Function) values
  /// into the CallInst.
  void addCallEdge(Value *src, CallInst *dst) {
    addEdge(normalTermFactory(src), callTermFactory(dst));
  };

  /// Indicate that the dst may receive unknown values, e.g. from
  /// unknown property reads or parameters from escaped functions.
  void addUnknownSrc(Value *dst) {
    addEdge(escapeTermFactory(dst), normalTermFactory(dst));
  };

  /// Indicate that the src may leak values into unknown contexts,
  /// e.g. by unknown property stores or return from escaped functions.
  void addUnknownDst(Value *src) {
    addEdge(normalTermFactory(src), escapeTermFactory(src));
  };

  /// Indicate that the src may leak values into specifically the
  /// unknown context indicated by the dst. Used for arrays.
  void addUnknownDst(Value *src, Value *dst) {
    addEdge(normalTermFactory(src), escapeTermFactory(dst));
  }

  /// Indicate a Function as the root of the analysis, so we know that
  /// its parameters always escape-in, and return value escapes out.
  // TODO: Perhaps a more appropriate name is needed for this function.
  void markFunctionRoot(Function *src) {
    addEdge(lambdaTermFactory(src), escapeTermFactory(src));
  }

  /// Allocation at the AllocObjectInst flows into the results of the
  /// instruction.
  void addObjectEdge(AllocObjectInst *o, llvm::DenseSet<Literal *> &props) {
    addEdge(objectTermFactory(o, props), normalTermFactory(o));
  }

  /// A reference to an object (src) flows as a base pointer (aka receiver)
  /// to a load property instruction (dst).
  void addLoadEdge(Value *src, LoadPropertyInst *dst) {
    addEdge(normalTermFactory(src), loadPropertyTermFactory(dst));
  }

  /// A reference to an object (src) flows as a base pointer (aka receiver)
  /// to a store own property instruction (dst).
  void addStoreOwnEdge(Value *src, StoreOwnPropertyInst *dst) {
    addEdge(normalTermFactory(src), storeOwnPropertyTermFactory(dst));
  }

  /// A reference to an object (src) flows as a base pointer (aka receiver)
  /// to a store property instruction (dst).
  void addStoreEdge(Value *src, StorePropertyInst *dst) {
    addEdge(normalTermFactory(src), storePropertyTermFactory(dst));
  }

  void addArrayEdge(AllocArrayInst *ai) {
    addEdge(arrayTermFactory(ai), normalTermFactory(ai));
  }

  /// The Value being exported via assignment to module.exports
  /// is captured into a new term, identified by a unique numeric
  /// id.
  void addExportModuleEdge(Value *V, unsigned int fmodId) {
    addEdge(normalTermFactory(V), moduleTermFactory(fmodId));
  }

  /// The result of the require call is the term associated with
  /// the JSmodule being required.
  void addRequireModuleEdge(CallInst *I, unsigned int fmodId) {
    addEdge(moduleTermFactory(fmodId), normalTermFactory(I));
  }
};
} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_SET_CONSTRAINT_ANALYSIS_PROBLEM_H
