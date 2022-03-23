/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_SCALAR_TERM_H
#define HERMES_OPTIMIZER_SCALAR_TERM_H

#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"
#include "llvh/ADT/DenseSet.h"

using llvh::isa;

namespace hermes {

enum TermKind {
  NormalTermKind, // None of the below categories
  LambdaTermKind, // Functions
  CallTermKind, // Call sites
  EscapeTermKind, // Terms invented to represent values coming from or going to
                  // outside the scope of analysis
  ObjectTermKind, // Objects
  LoadPropertyTermKind, // Load property (currently does not deal with inherited
                        // props)
  StoreOwnPropertyTermKind, // Store own property
  StorePropertyTermKind, // Store property
  PropertyTermKind, // Property and a value
  ArrayTermKind, // Arrays
  ModuleTermKind // Modules
};

/// Terms are needed in the constraint graph to capture different kinds of
/// values.  The same Hermes "Value" could be in different roles.  For example
/// a Function value acts as a lambda, but we also use it to mean the return
/// value of the Function.  For CallInst, in one role we use it as a receptor
/// for lambda terms that arrive as callees; in another role, we use CallInst
/// to mean the value produced by the call.
/// Here is the abstract base class to implement the Term functionality.
class Term {
 public:
  Term(Value *v, unsigned id) : value_id_(id), value_(v) {}

  virtual ~Term() = default;

  /// Unique numeric id for each term, used in printing.
  unsigned value_id_{0};

  /// Value associated with the term.
  Value *value_{nullptr};

  /// The kind of this Term, used in RTTI.
  TermKind kind_{TermKind::NormalTermKind};
  static bool classof(const Term *) {
    return true;
  }

  /// The set of terms that flow in into this term.
  llvh::DenseSet<Term *> ins_;

  /// The set of terms to which this term flows into.
  llvh::DenseSet<Term *> outs_;
#ifndef NDEBUG
  virtual std::string getPrintStr() = 0;
#endif
};

/// Term representing an ordinary value in the constraint graph.
class NormalTerm : public Term {
 public:
  NormalTerm(Value *v, unsigned id) : Term(v, id) {
    kind_ = TermKind::NormalTermKind;
  }
#ifndef NDEBUG
  std::string getPrintStr() {
    Value *v = (Value *)value_;
    switch (v->getKind()) {
      case ValueKind::VariableKind: {
        Variable *Var = cast<Variable>(v);
        VariableScope *VS = Var->getParent();
        std::string part1("Var(");
        std::string part2(Var->getName().c_str());
        std::string part3("):");
        std::string part4(VS->getFunction()->getInternalName().c_str());
        return part1 + part2 + part3 + part4;
      }
      case ValueKind::FunctionKind: {
        Function *F = cast<Function>(v);
        return F->getInternalName().c_str();
      }
      case ValueKind::ParameterKind: {
        Parameter *P = cast<Parameter>(v);
        std::string part1("Param(");
        std::string part2(P->getName().c_str());
        std::string part3("):");
        std::string part4(P->getParent()->getInternalName().c_str());
        return part1 + part2 + part3 + part4;
      }
      case ValueKind::CreateFunctionInstKind: {
        CreateFunctionInst *I = cast<CreateFunctionInst>(v);
        return I->getName();
      }
      default: {
        return v->getKindStr();
      }
    }
  }
#endif
  static bool classof(const Term *t) {
    return t->kind_ == TermKind::NormalTermKind;
  }
};

/// Term representing a Function in the constraint graph.
class LambdaTerm : public Term {
 public:
  LambdaTerm(Function *f, unsigned id) : Term(f, id) {
    kind_ = TermKind::LambdaTermKind;
  }
#ifndef NDEBUG
  std::string getPrintStr() {
    Function *f = (Function *)value_;

    std::string part1("FUN(");
    std::string part2(f->getInternalName().c_str());
    std::string part3(")");

    std::string result = part1 + part2 + part3;
    return result;
  }
#endif
  static bool classof(const Term *t) {
    return t->kind_ == TermKind::LambdaTermKind;
  }
};

/// Term representing a Call site in the constraint graph
class CallTerm : public Term {
 public:
  CallTerm(CallInst *c, unsigned id) : Term(c, id) {
    kind_ = TermKind::CallTermKind;
  }
#ifndef NDEBUG
  std::string getPrintStr() {
    CallInst *c = (CallInst *)value_;
    std::string part1("CALL(");
    std::string part2(c->getKindStr());
    std::string part3(")");
    std::string result = part1 + part2 + part3;
    return result;
  }
#endif
  static bool classof(const Term *t) {
    return t->kind_ == TermKind::CallTermKind;
  }
};

/// Term representing values that either escape in or out.
/// In EscapeTerm, the value_ field represents the reason
/// we introduced this escaped value. This is necessary so as
/// to bound the distinct EscapeTerms generated.
class EscapeTerm : public Term {
 public:
  EscapeTerm(Value *v, unsigned id) : Term(v, id) {
    kind_ = TermKind::EscapeTermKind;
  }
#ifndef NDEBUG
  std::string getPrintStr() {
    Value *via = (Value *)value_;
    if (!via)
      return "Unknown";
    std::string part1("Unknown(");
    std::string part2(via->getKindStr());
    std::string part3(")");
    std::string result = part1 + part2 + part3;
    return result;
  }
#endif
  static bool classof(const Term *t) {
    return t->kind_ == TermKind::EscapeTermKind;
  }
};

/// Term representing an allocated object in the constraint graph.
/// The reference to this term (ai below) is the unaliased handled to
/// the object which is the handle we use for matching stores and loads.
class ObjectTerm : public Term {
 public:
  /// The set of properties created at Object creation time.
  llvh::DenseSet<Literal *> owns_;

  // We use ObjectTerm also for allocations using ConstructInst, and
  // in future, plan to use it for Arrays as well.
  ObjectTerm(Instruction *ai, llvh::DenseSet<Literal *> &owns, unsigned id)
      : Term(ai, id), owns_(owns) {
    kind_ = TermKind::ObjectTermKind;
  }
#ifndef NDEBUG
  std::string getPrintStr() {
    std::string part1("Object(");
    std::string part2(value_->getKindStr());
    std::string part3(")");
    std::string result = part1 + part2 + part3;
    return result;
  }
#endif
  static bool classof(const Term *t) {
    return t->kind_ == TermKind::ObjectTermKind;
  }
};

class ArrayTerm : public Term {
 public:
  ArrayTerm(AllocArrayInst *ai, unsigned id) : Term(ai, id) {
    kind_ = TermKind::ArrayTermKind;
  }
#ifndef NDEBUG
  std::string getPrintStr() {
    std::string part1("Array(");
    std::string part2(value_->getKindStr());
    std::string part3(")");
    std::string result = part1 + part2 + part3;
    return result;
  }
#endif
  static bool classof(const Term *t) {
    return t->kind_ == TermKind::ArrayTermKind;
  }
};

class LoadPropertyTerm : public Term {
 public:
  Value *prop_;

  LoadPropertyTerm(LoadPropertyInst *li, Value *prop, unsigned id)
      : Term(li, id), prop_(prop) {
    kind_ = TermKind::LoadPropertyTermKind;
  }

#ifndef NDEBUG
  std::string getPrintStr() {
    std::string part1("Load(");
    StringRef printProp;
    if (llvh::isa<LiteralString>(prop_))
      printProp = ((LiteralString *)prop_)->getValue().str();
    else
      printProp = prop_->getKindStr();
    std::string part2(printProp);
    std::string part3(", ");
    std::string part4(value_->getKindStr());
    std::string part5(")");
    return part1 + part2 + part3 + part4 + part5;
  }
#endif
  static bool classof(const Term *t) {
    return t->kind_ == TermKind::LoadPropertyTermKind;
  }
};

class StoreOwnPropertyTerm : public Term {
 public:
  Value *prop_;
  StoreOwnPropertyTerm(StoreOwnPropertyInst *si, Value *prop, unsigned id)
      : Term(si, id), prop_(prop) {
    kind_ = TermKind::StoreOwnPropertyTermKind;
  }
#ifndef NDEBUG
  std::string getPrintStr() {
    std::string part1("StoreOwn(");
    StringRef printProp;
    if (llvh::isa<LiteralString>(prop_))
      printProp = ((LiteralString *)prop_)->getValue().str();
    else
      printProp = prop_->getKindStr();
    std::string part2(printProp);
    std::string part3(", ");
    std::string part4(value_->getKindStr());
    std::string part5(")");
    return part1 + part2 + part3 + part4 + part5;
  }
#endif
  static bool classof(const Term *t) {
    return t->kind_ == TermKind::StoreOwnPropertyTermKind;
  }
};

class StorePropertyTerm : public Term {
 public:
  Value *prop_;
  StorePropertyTerm(StorePropertyInst *si, Value *prop, unsigned id)
      : Term(si, id), prop_(prop) {
    kind_ = TermKind::StorePropertyTermKind;
  }
#ifndef NDEBUG
  std::string getPrintStr() {
    std::string part1("Store(");
    StringRef tmp;
    if (llvh::isa<LiteralString>(prop_))
      tmp = ((LiteralString *)prop_)->getValue().str();
    else
      tmp = prop_->getKindStr();
    std::string part2(tmp);
    std::string part3(", ");
    std::string part4(value_->getKindStr());
    std::string part5(")");
    return part1 + part2 + part3 + part4 + part5;
  }
#endif
  static bool classof(const Term *t) {
    return t->kind_ == TermKind::StorePropertyTermKind;
  }
};

class PropertyTerm : public Term {
 public:
  Value *prop_;
  Value *obj_; // do we need this?

  PropertyTerm(Value *v, Value *prop, Value *obj, unsigned id)
      : Term(v, id), prop_(prop), obj_(obj) {
    kind_ = TermKind::PropertyTermKind;
  }
#ifndef NDEBUG
  std::string getPrintStr() {
    std::string part1("Prop(");
    StringRef tmp;
    if (llvh::isa<LiteralString>(prop_))
      tmp = ((LiteralString *)prop_)->getValue().str();
    else
      tmp = prop_->getKindStr();
    std::string part2(tmp);
    std::string part3(", ");
    std::string part4(value_->getKindStr());
    std::string part5(")");
    return part1 + part2 + part3 + part4 + part5;
  }
#endif
  static bool classof(const Term *t) {
    return t->kind_ == TermKind::PropertyTermKind;
  }
};

class ModuleTerm : public Term {
  unsigned int modId_;

 public:
  ModuleTerm(Value *v, unsigned int modId, unsigned id)
      : Term(v, id), modId_(modId) {
    kind_ = TermKind::ModuleTermKind;
  }
#ifndef NDEBUG
  std::string getPrintStr() {
    std::string part1("Module(");
    std::string part2 = std::to_string(modId_);
    std::string part3(")");
    return part1 + part2 + part3;
  }
#else
  std::string getPrintStr() {
    return std::to_string(modId_);
  }
#endif
  static bool classof(const ModuleTerm *) {
    return true;
  }
  static bool classof(const Term *t) {
    return t->kind_ == TermKind::ModuleTermKind;
  }
};
} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_TERM_H
