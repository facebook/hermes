/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_IR_INSTRS_H
#define HERMES_IR_INSTRS_H

#include <string>
#include <utility>

#include "hermes/FrontEndDefs/Builtins.h"
#include "hermes/IR/IR.h"

#include "llvh/ADT/SmallVector.h"
#include "llvh/ADT/ilist_node.h"
#include "llvh/IR/CFG.h"
#include "llvh/IR/SymbolTableListTraits.h"

using llvh::ArrayRef;
using llvh::cast;

namespace hermes {

/// Base class for instructions that have exactly one operand. It guarantees
/// that only one operand is pushed and it provides getSingleOperand().
class SingleOperandInst : public Instruction {
  SingleOperandInst(const SingleOperandInst &) = delete;
  void operator=(const SingleOperandInst &) = delete;

  // Make pushOperand private to ensure derived classes don't use it.
  using Instruction::pushOperand;

 protected:
  explicit SingleOperandInst(ValueKind K, Value *Op) : Instruction(K) {
    pushOperand(Op);
  }

 public:
  enum { SingleOperandIdx };

  explicit SingleOperandInst(
      const SingleOperandInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {
    assert(operands.size() == 1 && "SingleOperandInst must have 1 operand!");
  }

  Value *getSingleOperand() const {
    return getOperand(0);
  }

  static bool hasOutput() {
    llvm_unreachable("SingleOperandInst must be inherited.");
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    llvm_unreachable("SingleOperandInst must be inherited.");
  }

 private:
  static bool classof(const Value *V);
};

/// Subclasses of this class are all able to terminate a basic
/// block. Thus, these are all the flow control type of operations.
class TerminatorInst : public Instruction {
  TerminatorInst(const TerminatorInst &) = delete;
  void operator=(const TerminatorInst &) = delete;

 protected:
  explicit TerminatorInst(ValueKind K) : Instruction(K) {}

 public:
  explicit TerminatorInst(
      const TerminatorInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  unsigned getNumSuccessors() const;
  BasicBlock *getSuccessor(unsigned idx) const;
  void setSuccessor(unsigned idx, BasicBlock *B);

  static bool hasOutput() {
    llvm_unreachable("TerminatorInst must be inherited.");
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    llvm_unreachable("TerminatorInst must be inherited.");
  }

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), TerminatorInst);
  }

  using succ_iterator = llvh::SuccIterator<TerminatorInst, BasicBlock>;
  using succ_const_iterator =
      llvh::SuccIterator<const TerminatorInst, const BasicBlock>;
  using succ_range = llvh::iterator_range<succ_iterator>;
  using succ_const_range = llvh::iterator_range<succ_const_iterator>;

 private:
  inline succ_iterator succ_begin() {
    return succ_iterator(this);
  }
  inline succ_const_iterator succ_begin() const {
    return succ_const_iterator(this);
  }
  inline succ_iterator succ_end() {
    return succ_iterator(this, true);
  }
  inline succ_const_iterator succ_end() const {
    return succ_const_iterator(this, true);
  }

 public:
  inline succ_range successors() {
    return succ_range(succ_begin(), succ_end());
  }
  inline succ_const_range successors() const {
    return succ_const_range(succ_begin(), succ_end());
  }
};

class BranchInst : public TerminatorInst {
  BranchInst(const BranchInst &) = delete;
  void operator=(const BranchInst &) = delete;

 public:
  enum { BranchDestIdx };

  BasicBlock *getBranchDest() const {
    return cast<BasicBlock>(getOperand(BranchDestIdx));
  }

  explicit BranchInst(BasicBlock *parent, BasicBlock *dest)
      : TerminatorInst(ValueKind::BranchInstKind) {
    pushOperand(dest);
  }
  explicit BranchInst(const BranchInst *src, llvh::ArrayRef<Value *> operands)
      : TerminatorInst(src, operands) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::BranchInstKind;
  }

  unsigned getNumSuccessorsImpl() const {
    return 1;
  }
  BasicBlock *getSuccessorImpl(unsigned idx) const {
    assert(idx == 0 && "BranchInst only have 1 successor!");
    return getBranchDest();
  }
  void setSuccessorImpl(unsigned idx, BasicBlock *B) {
    assert(idx == 0 && "BranchInst only have 1 successor!");
    setOperand(B, idx);
  }
};

class AddEmptyStringInst : public SingleOperandInst {
  AddEmptyStringInst(const AddEmptyStringInst &) = delete;
  void operator=(const AddEmptyStringInst &) = delete;

 public:
  explicit AddEmptyStringInst(Value *value)
      : SingleOperandInst(ValueKind::AddEmptyStringInstKind, value) {
    setType(*getInherentTypeImpl());
  }
  explicit AddEmptyStringInst(
      const AddEmptyStringInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createString();
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::AddEmptyStringInstKind;
  }
};

class AsNumberInst : public SingleOperandInst {
  AsNumberInst(const AsNumberInst &) = delete;
  void operator=(const AsNumberInst &) = delete;

 public:
  explicit AsNumberInst(Value *value)
      : SingleOperandInst(ValueKind::AsNumberInstKind, value) {
    setType(*getInherentTypeImpl());
  }
  explicit AsNumberInst(
      const AsNumberInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createNumber();
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::AsNumberInstKind;
  }
};

class AsNumericInst : public SingleOperandInst {
  AsNumericInst(const AsNumericInst &) = delete;
  void operator=(const AsNumericInst &) = delete;

 public:
  explicit AsNumericInst(Value *value)
      : SingleOperandInst(ValueKind::AsNumericInstKind, value) {
    if (value->getType().isNumberType()) {
      setType(Type::createNumber());
    } else if (value->getType().isBigIntType()) {
      setType(Type::createBigInt());
    } else {
      setType(Type::createNumeric());
    }
  }
  explicit AsNumericInst(
      const AsNumericInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::AsNumericInstKind;
  }
};

class AsInt32Inst : public SingleOperandInst {
  AsInt32Inst(const AsInt32Inst &) = delete;
  void operator=(const AsInt32Inst &) = delete;

 public:
  explicit AsInt32Inst(Value *value)
      : SingleOperandInst(ValueKind::AsInt32InstKind, value) {
    setType(*getInherentTypeImpl());
  }
  explicit AsInt32Inst(const AsInt32Inst *src, llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createInt32();
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::AsInt32InstKind;
  }
};

class CondBranchInst : public TerminatorInst {
  CondBranchInst(const CondBranchInst &) = delete;
  void operator=(const CondBranchInst &) = delete;

 public:
  enum { ConditionIdx, TrueBlockIdx, FalseBlockIdx };

  Value *getCondition() const {
    return getOperand(ConditionIdx);
  }
  BasicBlock *getTrueDest() const {
    return cast<BasicBlock>(getOperand(TrueBlockIdx));
  }
  BasicBlock *getFalseDest() const {
    return cast<BasicBlock>(getOperand(FalseBlockIdx));
  }

  explicit CondBranchInst(
      BasicBlock *parent,
      Value *cond,
      BasicBlock *trueBlock,
      BasicBlock *falseBlock)
      : TerminatorInst(ValueKind::CondBranchInstKind) {
    pushOperand(cond);
    pushOperand(trueBlock);
    pushOperand(falseBlock);
  }
  explicit CondBranchInst(
      const CondBranchInst *src,
      llvh::ArrayRef<Value *> operands)
      : TerminatorInst(src, operands) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::CondBranchInstKind;
  }

  unsigned getNumSuccessorsImpl() const {
    return 2;
  }
  BasicBlock *getSuccessorImpl(unsigned idx) const {
    if (idx == 0)
      return getTrueDest();
    if (idx == 1)
      return getFalseDest();
    llvm_unreachable("CondBranchInst only have 2 successors!");
  }
  void setSuccessorImpl(unsigned idx, BasicBlock *B) {
    assert(idx <= 1 && "CondBranchInst only have 2 successors!");
    setOperand(B, idx + TrueBlockIdx);
  }
};

class ReturnInst : public TerminatorInst {
  ReturnInst(const ReturnInst &) = delete;
  void operator=(const ReturnInst &) = delete;

 public:
  enum { ReturnValueIdx };

  Value *getValue() const {
    return getOperand(ReturnValueIdx);
  }

  explicit ReturnInst(Value *val) : TerminatorInst(ValueKind::ReturnInstKind) {
    pushOperand(val);
  }
  explicit ReturnInst(const ReturnInst *src, llvh::ArrayRef<Value *> operands)
      : TerminatorInst(src, operands) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::ReturnInstKind;
  }

  unsigned getNumSuccessorsImpl() const {
    return 0;
  }
  BasicBlock *getSuccessorImpl(unsigned idx) const {
    llvm_unreachable("ReturnInst has no successor!");
  }
  void setSuccessorImpl(unsigned idx, BasicBlock *B) {
    llvm_unreachable("ReturnInst has no successor!");
  }
};

class AllocStackInst : public Instruction {
  AllocStackInst(const AllocStackInst &) = delete;
  void operator=(const AllocStackInst &) = delete;

  Label variableName;

 public:
  enum { VariableNameIdx };

  explicit AllocStackInst(Identifier varName)
      : Instruction(ValueKind::AllocStackInstKind), variableName(varName) {
    pushOperand(&variableName);
  }
  explicit AllocStackInst(
      const AllocStackInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, {}), variableName(src->getVariableName()) {
    // NOTE: we are playing a little trick here since the Label is not heap
    // allocated. We can't use the normal machinery to copy the operands.
    pushOperand(&variableName);
  }

  Identifier getVariableName() const {
    return variableName.get();
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  bool acceptsEmptyTypeImpl() const {
    return true;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::AllocStackInstKind;
  }
};

class LoadStackInst : public SingleOperandInst {
  LoadStackInst(const LoadStackInst &) = delete;
  void operator=(const LoadStackInst &) = delete;

 public:
  explicit LoadStackInst(AllocStackInst *alloc)
      : SingleOperandInst(ValueKind::LoadStackInstKind, alloc) {
    setType(alloc->getType());
  }
  explicit LoadStackInst(
      const LoadStackInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  AllocStackInst *getPtr() const {
    return cast<AllocStackInst>(getSingleOperand());
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  bool acceptsEmptyTypeImpl() const {
    return true;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setReadStack().setIdempotent();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::LoadStackInstKind;
  }
};

class StoreStackInst : public Instruction {
  StoreStackInst(const StoreStackInst &) = delete;
  void operator=(const StoreStackInst &) = delete;

 public:
  enum { StoredValueIdx, PtrIdx };

  Value *getValue() const {
    return getOperand(StoredValueIdx);
  }
  AllocStackInst *getPtr() const {
    return cast<AllocStackInst>(getOperand(PtrIdx));
  }

  explicit StoreStackInst(Value *storedValue, AllocStackInst *ptr)
      : Instruction(ValueKind::StoreStackInstKind) {
    pushOperand(storedValue);
    pushOperand(ptr);
  }
  explicit StoreStackInst(
      const StoreStackInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  bool acceptsEmptyTypeImpl() const {
    return true;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setWriteStack().setIdempotent();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::StoreStackInstKind;
  }
};

/// Base class for instructions that always result in a scope. Its operand can
/// be used to determine the VariableScope associated with the resulting scope.
/// This is different from other base classes in that subclasses may perform
/// very different functionality, but it makes it convenient to traverse,
/// optimize, and verify the structure of scopes.
/// Any operand that represents a scope must be a BaseScopeInst until register
/// allocation, at which point inserting Movs will result in the loss of this
/// invariant.
class BaseScopeInst : public Instruction {
  BaseScopeInst(const BaseScopeInst &) = delete;
  void operator=(const BaseScopeInst &) = delete;

 protected:
  enum { VariableScopeIdx, LAST_IDX };

  explicit BaseScopeInst(ValueKind kind, Value *variableScope)
      : Instruction(kind) {
    setType(*getInherentTypeImpl());
    pushOperand(variableScope);
  }

 public:
  explicit BaseScopeInst(
      const BaseScopeInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {
    setType(*getInherentTypeImpl());
  }

  VariableScope *getVariableScope() const {
    return llvh::cast<VariableScope>(getOperand(VariableScopeIdx));
  }

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createEnvironment();
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), BaseScopeInst);
  }
};

class LoadFrameInst : public Instruction {
  LoadFrameInst(const LoadFrameInst &) = delete;
  void operator=(const LoadFrameInst &) = delete;

 public:
  enum { VariableIdx };

  explicit LoadFrameInst(Variable *alloc)
      : Instruction(ValueKind::LoadFrameInstKind) {
    setType(alloc->getType());
    pushOperand(alloc);
  }
  explicit LoadFrameInst(
      const LoadFrameInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Variable *getLoadVariable() const {
    return cast<Variable>(getOperand(VariableIdx));
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  bool acceptsEmptyTypeImpl() const {
    return true;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setReadFrame().setIdempotent();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::LoadFrameInstKind;
  }
};

class StoreFrameInst : public Instruction {
  StoreFrameInst(const StoreFrameInst &) = delete;
  void operator=(const StoreFrameInst &) = delete;

 public:
  enum { StoredValueIdx, VariableIdx };

  Value *getValue() const {
    return getOperand(StoredValueIdx);
  }
  Variable *getVariable() const {
    return cast<Variable>(getOperand(VariableIdx));
  }

  explicit StoreFrameInst(Value *storedValue, Variable *ptr)
      : Instruction(ValueKind::StoreFrameInstKind) {
    pushOperand(storedValue);
    pushOperand(ptr);
  }
  explicit StoreFrameInst(
      const StoreFrameInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  bool acceptsEmptyTypeImpl() const {
    return true;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setWriteFrame().setIdempotent();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::StoreFrameInstKind;
  }
};

/// A base class for all instructions that create a lexical child. Some of them
/// don't return a callable.
class BaseCreateLexicalChildInst : public Instruction {
  BaseCreateLexicalChildInst(const BaseCreateLexicalChildInst &) = delete;
  void operator=(const BaseCreateLexicalChildInst &) = delete;

 protected:
  explicit BaseCreateLexicalChildInst(ValueKind kind, Function *code)
      : Instruction(kind) {
    pushOperand(code);
  }

 public:
  enum { FunctionCodeIdx, LAST_IDX };

  explicit BaseCreateLexicalChildInst(
      const BaseCreateLexicalChildInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Function *getFunctionCode() const {
    return cast<Function>(getOperand(FunctionCodeIdx));
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), BaseCreateLexicalChildInst);
  }
};

/// Base class for all instruction creating and returning a callable.
class BaseCreateCallableInst : public BaseCreateLexicalChildInst {
  BaseCreateCallableInst(const BaseCreateCallableInst &) = delete;
  void operator=(const BaseCreateCallableInst &) = delete;

 protected:
  explicit BaseCreateCallableInst(ValueKind kind, Function *code)
      : BaseCreateLexicalChildInst(kind, code) {
    setType(*getInherentTypeImpl());
  }

 public:
  explicit BaseCreateCallableInst(
      const BaseCreateCallableInst *src,
      llvh::ArrayRef<Value *> operands)
      : BaseCreateLexicalChildInst(src, operands) {}

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createObject();
  }

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), BaseCreateCallableInst);
  }
};

/// Create a normal function, or an outer generator function.
class CreateFunctionInst : public BaseCreateCallableInst {
  CreateFunctionInst(const CreateFunctionInst &) = delete;
  void operator=(const CreateFunctionInst &) = delete;

 public:
  explicit CreateFunctionInst(Function *code)
      : BaseCreateCallableInst(ValueKind::CreateFunctionInstKind, code) {
    assert(
        (llvh::isa<NormalFunction>(code) ||
         llvh::isa<GeneratorFunction>(code) ||
         llvh::isa<AsyncFunction>(code)) &&
        "Only NormalFunction/GeneratorFunction supported by CreateFunctionInst");
  }
  explicit CreateFunctionInst(
      const CreateFunctionInst *src,
      llvh::ArrayRef<Value *> operands)
      : BaseCreateCallableInst(src, operands) {}

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::CreateFunctionInstKind;
  }
};

class BaseCallInst : public Instruction {
  BaseCallInst(const BaseCallInst &) = delete;
  void operator=(const BaseCallInst &) = delete;

 protected:
  /// Operand index of the first arg, starting at 'this'. The arguments are
  /// assumed to be found in [thisIdx_, numOperands).
  size_t thisIdx_;
  explicit BaseCallInst(
      ValueKind kind,
      Value *callee,
      Value *target,
      Value *env,
      Value *newTarget,
      Value *thisValue,
      ArrayRef<Value *> args)
      : Instruction(kind) {
    pushOperand(callee);
    pushOperand(target);
    pushOperand(env);
    pushOperand(newTarget);
    thisIdx_ = getNumOperands();
    pushOperand(thisValue);
    for (const auto &arg : args) {
      pushOperand(arg);
    }
  }
  /// This constructor is used when there are more operands subclasses need to
  /// add before the list of function arguments. It's required that subclass
  /// constructors perform the following in order:
  /// - Push the additional operands they need in the space from NewTargetIdx to
  ///   \p thisIdx.
  /// - Push the function arguments including 'this', which will be starting
  ///   from \p thisIdx.
  explicit BaseCallInst(
      ValueKind kind,
      Value *callee,
      Value *target,
      Value *env,
      Value *newTarget,
      size_t thisIdx)
      : Instruction(kind) {
    pushOperand(callee);
    pushOperand(target);
    pushOperand(env);
    pushOperand(newTarget);
    thisIdx_ = thisIdx;
  }
  explicit BaseCallInst(
      const BaseCallInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {
    thisIdx_ = src->thisIdx_;
  }

  // Forces the code to use the appropriate getters instead of relying on
  // hard-coded offsets when accessing the arguments.
  using Instruction::getOperand;

 public:
  enum { CalleeIdx, TargetIdx, EnvIdx, NewTargetIdx, _LastIdx };

  using ArgumentList = llvh::SmallVector<Value *, 2>;

  Value *getCallee() const {
    return getOperand(CalleeIdx);
  }
  void setCallee(Value *v) {
    setOperand(v, CalleeIdx);
  }
  /// \return the IR Function that the instruction is calling,
  ///   or EmptySentinel if it's not known at this time.
  Value *getTarget() const {
    return getOperand(TargetIdx);
  }
  /// Set the target to \p func.
  void setTarget(Function *func) {
    setOperand(func, TargetIdx);
  }
  /// \return the Environment that the target is being called with,
  ///   or EmptySentinel if it's not known at this time.
  Value *getEnvironment() const {
    return getOperand(EnvIdx);
  }
  /// Set the environment to \p env.
  void setEnvironment(Value *env) {
    setOperand(env, EnvIdx);
  }
  Value *getNewTarget() const {
    return getOperand(NewTargetIdx);
  }
  void setNewTarget(Value *newTarget) {
    return setOperand(newTarget, NewTargetIdx);
  }
  /// Get argument 0, the value for 'this'.
  Value *getThis() const {
    return getOperand(thisIdx_);
  }
  /// Get argument by index. 'this' is argument 0.
  Value *getArgument(unsigned idx) {
    return getOperand(thisIdx_ + idx);
  }
  /// Set the value \p v at index \p idx. 'this' is argument 0.
  void setArgument(Value *V, unsigned idx) {
    setOperand(V, thisIdx_ + idx);
  }

  unsigned getNumArguments() const {
    return getNumOperands() - thisIdx_;
  }

  unsigned getThisIdx() const {
    return thisIdx_;
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute();
  }

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), BaseCallInst);
  }
};

class CallInst : public BaseCallInst {
  CallInst(const CallInst &) = delete;
  void operator=(const CallInst &) = delete;

 public:
  enum { ThisIdx = BaseCallInst::_LastIdx };
  explicit CallInst(
      Value *callee,
      Value *target,
      Value *env,
      Value *newTarget,
      Value *thisValue,
      ArrayRef<Value *> args)
      : BaseCallInst(
            ValueKind::CallInstKind,
            callee,
            target,
            env,
            newTarget,
            thisValue,
            args) {}
  explicit CallInst(const CallInst *src, llvh::ArrayRef<Value *> operands)
      : BaseCallInst(src, operands) {}

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::CallInstKind;
  }
};

class HBCCallWithArgCountInst : public BaseCallInst {
  HBCCallWithArgCountInst(const HBCCallWithArgCountInst &) = delete;
  void operator=(const HBCCallWithArgCountInst &) = delete;

 public:
  enum { NumArgLiteralIdx = BaseCallInst::_LastIdx, ThisIdx };
  explicit HBCCallWithArgCountInst(
      Value *callee,
      Value *target,
      Value *env,
      Value *newTarget,
      LiteralNumber *argCount,
      Value *thisValue,
      ArrayRef<Value *> args)
      : BaseCallInst(
            ValueKind::HBCCallWithArgCountInstKind,
            callee,
            target,
            env,
            newTarget,
            ThisIdx) {
    pushOperand(argCount);
    pushOperand(thisValue);
    for (const auto &arg : args) {
      pushOperand(arg);
    }
  }
  explicit HBCCallWithArgCountInst(
      const HBCCallWithArgCountInst *src,
      llvh::ArrayRef<Value *> operands)
      : BaseCallInst(src, operands) {}

  /// \return the number of arguments, including 'this'.
  /// This should always match the value in \c getNumArguments.
  Value *getNumArgumentsLiteral() const {
    return getOperand(NumArgLiteralIdx);
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::HBCCallWithArgCountInstKind;
  }
};

/// Call a VM builtin with the specified number and undefined as the "this"
/// parameter.
class CallBuiltinInst : public BaseCallInst {
  CallBuiltinInst(const CallBuiltinInst &) = delete;
  void operator=(const CallBuiltinInst &) = delete;

 public:
  enum { ThisIdx = BaseCallInst::_LastIdx };
  explicit CallBuiltinInst(
      LiteralNumber *callee,
      EmptySentinel *target,
      EmptySentinel *env,
      LiteralUndefined *undefined,
      ArrayRef<Value *> args)
      : BaseCallInst(
            ValueKind::CallBuiltinInstKind,
            callee,
            target,
            env,
            /* newTarget */ undefined,
            /* thisValue */ undefined,
            args) {
    assert(
        callee->getValue() == (int)callee->getValue() &&
        callee->getValue() < (double)BuiltinMethod::_count &&
        "invalid builtin call");
  }
  explicit CallBuiltinInst(
      const CallBuiltinInst *src,
      llvh::ArrayRef<Value *> operands)
      : BaseCallInst(src, operands) {}

  BuiltinMethod::Enum getBuiltinIndex() const {
    return (BuiltinMethod::Enum)cast<LiteralNumber>(getCallee())->asInt32();
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::CallBuiltinInstKind;
  }
};

class GetBuiltinClosureInst : public Instruction {
  GetBuiltinClosureInst(const GetBuiltinClosureInst &) = delete;
  void operator=(const GetBuiltinClosureInst &) = delete;

 public:
  enum { BuiltinIndexIdx };

  explicit GetBuiltinClosureInst(LiteralNumber *builtinIndex)
      : Instruction(ValueKind::GetBuiltinClosureInstKind) {
    assert(
        builtinIndex->asUInt32() < BuiltinMethod::_count &&
        "invalid builtin call");
    pushOperand(builtinIndex);
    setType(*getInherentTypeImpl());
  }
  explicit GetBuiltinClosureInst(
      const GetBuiltinClosureInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createObject();
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setIdempotent();
  }

  BuiltinMethod::Enum getBuiltinIndex() const {
    return static_cast<BuiltinMethod::Enum>(
        cast<LiteralNumber>(getOperand(BuiltinIndexIdx))->asInt32());
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::GetBuiltinClosureInstKind;
  }
};

class HBCCallNInst : public BaseCallInst {
 public:
  /// The minimum number of args supported by a CallN instruction, including
  /// 'this'.
  static constexpr uint32_t kMinArgs = 1;

  /// The maximum (inclusive) number of args supported by a CallN instruction,
  /// including 'this'.
  static constexpr uint32_t kMaxArgs = 4;

  explicit HBCCallNInst(
      Value *callee,
      Value *target,
      Value *env,
      Value *newTarget,
      Value *thisValue,
      ArrayRef<Value *> args)
      : BaseCallInst(
            ValueKind::HBCCallNInstKind,
            callee,
            target,
            env,
            newTarget,
            thisValue,
            args) {
    // +1 for 'this'.
    assert(
        kMinArgs <= args.size() + 1 && args.size() + 1 <= kMaxArgs &&
        "Invalid arg count for HBCCallNInst");
  }

  explicit HBCCallNInst(
      const HBCCallNInst *src,
      llvh::ArrayRef<Value *> operands)
      : BaseCallInst(src, operands) {}

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::HBCCallNInstKind;
  }
};

class HBCGetGlobalObjectInst : public Instruction {
  HBCGetGlobalObjectInst(const HBCGetGlobalObjectInst &) = delete;
  void operator=(const HBCGetGlobalObjectInst &) = delete;

 public:
  explicit HBCGetGlobalObjectInst()
      : Instruction(ValueKind::HBCGetGlobalObjectInstKind) {
    setType(*getInherentTypeImpl());
  }
  explicit HBCGetGlobalObjectInst(
      const HBCGetGlobalObjectInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createObject();
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setIdempotent();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::HBCGetGlobalObjectInstKind;
  }
};

class BaseStorePropertyInst : public Instruction {
  BaseStorePropertyInst(const BaseStorePropertyInst &) = delete;
  void operator=(const BaseStorePropertyInst &) = delete;

 protected:
  explicit BaseStorePropertyInst(
      ValueKind kind,
      Value *storedValue,
      Value *object,
      Value *property)
      : Instruction(kind) {
    pushOperand(storedValue);
    pushOperand(object);
    pushOperand(property);
  }

 public:
  enum { StoredValueIdx, ObjectIdx, PropertyIdx };

  explicit BaseStorePropertyInst(
      const BaseStorePropertyInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Value *getStoredValue() const {
    return getOperand(StoredValueIdx);
  }
  Value *getObject() const {
    return getOperand(ObjectIdx);
  };
  Value *getProperty() const {
    return getOperand(PropertyIdx);
  }

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute();
  }

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), BaseStorePropertyInst);
  }
};

class StorePropertyInst : public BaseStorePropertyInst {
  StorePropertyInst(const StorePropertyInst &) = delete;
  void operator=(const StorePropertyInst &) = delete;

 protected:
  explicit StorePropertyInst(
      ValueKind kind,
      Value *storedValue,
      Value *object,
      Value *property)
      : BaseStorePropertyInst(kind, storedValue, object, property) {}
  explicit StorePropertyInst(
      const StorePropertyInst *src,
      llvh::ArrayRef<Value *> operands)
      : BaseStorePropertyInst(src, operands) {}

 public:
  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), StorePropertyInst);
  }
};

class StorePropertyLooseInst : public StorePropertyInst {
  StorePropertyLooseInst(const StorePropertyLooseInst &) = delete;
  void operator=(const StorePropertyLooseInst &) = delete;

 public:
  explicit StorePropertyLooseInst(
      Value *storedValue,
      Value *object,
      Value *property)
      : StorePropertyInst(
            ValueKind::StorePropertyLooseInstKind,
            storedValue,
            object,
            property) {}
  explicit StorePropertyLooseInst(
      const StorePropertyLooseInst *src,
      llvh::ArrayRef<Value *> operands)
      : StorePropertyInst(src, operands) {}

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::StorePropertyLooseInstKind;
  }
};

class StorePropertyStrictInst : public StorePropertyInst {
  StorePropertyStrictInst(const StorePropertyStrictInst &) = delete;
  void operator=(const StorePropertyStrictInst &) = delete;

 public:
  explicit StorePropertyStrictInst(
      Value *storedValue,
      Value *object,
      Value *property)
      : StorePropertyInst(
            ValueKind::StorePropertyStrictInstKind,
            storedValue,
            object,
            property) {}
  explicit StorePropertyStrictInst(
      const StorePropertyStrictInst *src,
      llvh::ArrayRef<Value *> operands)
      : StorePropertyInst(src, operands) {}

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::StorePropertyStrictInstKind;
  }
};

class TryStoreGlobalPropertyInst : public BaseStorePropertyInst {
  TryStoreGlobalPropertyInst(const TryStoreGlobalPropertyInst &) = delete;
  void operator=(const TryStoreGlobalPropertyInst &) = delete;

 protected:
  explicit TryStoreGlobalPropertyInst(
      ValueKind kind,
      Value *storedValue,
      Value *globalObject,
      LiteralString *property)
      : BaseStorePropertyInst(kind, storedValue, globalObject, property) {
    assert(
        (llvh::isa<GlobalObject>(globalObject) ||
         llvh::isa<HBCGetGlobalObjectInst>(globalObject)) &&
        "globalObject must refer to the global object");
  }
  explicit TryStoreGlobalPropertyInst(
      const TryStoreGlobalPropertyInst *src,
      llvh::ArrayRef<Value *> operands)
      : BaseStorePropertyInst(src, operands) {}

 public:
  LiteralString *getProperty() const {
    return cast<LiteralString>(BaseStorePropertyInst::getProperty());
  }

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), TryStoreGlobalPropertyInst);
  }
};

class TryStoreGlobalPropertyLooseInst : public TryStoreGlobalPropertyInst {
  TryStoreGlobalPropertyLooseInst(const TryStoreGlobalPropertyLooseInst &) =
      delete;
  void operator=(const TryStoreGlobalPropertyLooseInst &) = delete;

 public:
  explicit TryStoreGlobalPropertyLooseInst(
      Value *storedValue,
      Value *globalObject,
      LiteralString *property)
      : TryStoreGlobalPropertyInst(
            ValueKind::TryStoreGlobalPropertyLooseInstKind,
            storedValue,
            globalObject,
            property) {}
  explicit TryStoreGlobalPropertyLooseInst(
      const TryStoreGlobalPropertyLooseInst *src,
      llvh::ArrayRef<Value *> operands)
      : TryStoreGlobalPropertyInst(src, operands) {}

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::TryStoreGlobalPropertyLooseInstKind;
  }
};

class TryStoreGlobalPropertyStrictInst : public TryStoreGlobalPropertyInst {
  TryStoreGlobalPropertyStrictInst(const TryStoreGlobalPropertyStrictInst &) =
      delete;
  void operator=(const TryStoreGlobalPropertyStrictInst &) = delete;

 public:
  explicit TryStoreGlobalPropertyStrictInst(
      Value *storedValue,
      Value *globalObject,
      LiteralString *property)
      : TryStoreGlobalPropertyInst(
            ValueKind::TryStoreGlobalPropertyStrictInstKind,
            storedValue,
            globalObject,
            property) {}
  explicit TryStoreGlobalPropertyStrictInst(
      const TryStoreGlobalPropertyStrictInst *src,
      llvh::ArrayRef<Value *> operands)
      : TryStoreGlobalPropertyInst(src, operands) {}

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::TryStoreGlobalPropertyStrictInstKind;
  }
};

class BaseStoreOwnPropertyInst : public Instruction {
  BaseStoreOwnPropertyInst(const BaseStoreOwnPropertyInst &) = delete;
  void operator=(const BaseStoreOwnPropertyInst &) = delete;

 protected:
  explicit BaseStoreOwnPropertyInst(
      ValueKind kind,
      Value *storedValue,
      Value *object,
      Value *property,
      LiteralBool *isEnumerable)
      : Instruction(kind) {
    pushOperand(storedValue);
    pushOperand(object);
    pushOperand(property);
    pushOperand(isEnumerable);
  }

 public:
  enum { StoredValueIdx, ObjectIdx, PropertyIdx, IsEnumerableIdx };

  Value *getStoredValue() const {
    return getOperand(StoredValueIdx);
  }
  Value *getObject() const {
    return getOperand(ObjectIdx);
  }
  Value *getProperty() const {
    return getOperand(PropertyIdx);
  }
  bool getIsEnumerable() const {
    return cast<LiteralBool>(getOperand(IsEnumerableIdx))->getValue();
  }

  explicit BaseStoreOwnPropertyInst(
      const BaseStoreOwnPropertyInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute();
  }

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), BaseStoreOwnPropertyInst);
  }
};

class StoreOwnPropertyInst : public BaseStoreOwnPropertyInst {
  StoreOwnPropertyInst(const StoreOwnPropertyInst &) = delete;
  void operator=(const StoreOwnPropertyInst &) = delete;

 public:
  explicit StoreOwnPropertyInst(
      Value *storedValue,
      Value *object,
      Value *property,
      LiteralBool *isEnumerable)
      : BaseStoreOwnPropertyInst(
            ValueKind::StoreOwnPropertyInstKind,
            storedValue,
            object,
            property,
            isEnumerable) {}

  explicit StoreOwnPropertyInst(
      const StoreOwnPropertyInst *src,
      llvh::ArrayRef<Value *> operands)
      : BaseStoreOwnPropertyInst(src, operands) {}

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::StoreOwnPropertyInstKind;
  }
};

class StoreNewOwnPropertyInst : public BaseStoreOwnPropertyInst {
  StoreNewOwnPropertyInst(const StoreNewOwnPropertyInst &) = delete;
  void operator=(const StoreNewOwnPropertyInst &) = delete;

 public:
  explicit StoreNewOwnPropertyInst(
      Value *storedValue,
      Value *object,
      Literal *property,
      LiteralBool *isEnumerable)
      : BaseStoreOwnPropertyInst(
            ValueKind::StoreNewOwnPropertyInstKind,
            storedValue,
            object,
            property,
            isEnumerable) {
    assert(
        (llvh::isa<LiteralString>(property) ||
         llvh::isa<LiteralNumber>(property)) &&
        "Invalid property literal.");
    assert(
        object->getType().isObjectType() &&
        "object operand must be known to be an object");
  }

  explicit StoreNewOwnPropertyInst(
      const StoreNewOwnPropertyInst *src,
      llvh::ArrayRef<Value *> operands)
      : BaseStoreOwnPropertyInst(src, operands) {}

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::StoreNewOwnPropertyInstKind;
  }
};

class StoreGetterSetterInst : public Instruction {
  StoreGetterSetterInst(const StoreGetterSetterInst &) = delete;
  void operator=(const StoreGetterSetterInst &) = delete;

 public:
  enum {
    StoredGetterIdx,
    StoredSetterIdx,
    ObjectIdx,
    PropertyIdx,
    IsEnumerableIdx
  };

  Value *getStoredGetter() const {
    return getOperand(StoredGetterIdx);
  }
  Value *getStoredSetter() const {
    return getOperand(StoredSetterIdx);
  }
  Value *getObject() const {
    return getOperand(ObjectIdx);
  }
  Value *getProperty() const {
    return getOperand(PropertyIdx);
  }
  bool getIsEnumerable() const {
    return cast<LiteralBool>(getOperand(IsEnumerableIdx))->getValue();
  }

  explicit StoreGetterSetterInst(
      Value *storedGetter,
      Value *storedSetter,
      Value *object,
      Value *property,
      LiteralBool *isEnumerable)
      : Instruction(ValueKind::StoreGetterSetterInstKind) {
    pushOperand(storedGetter);
    pushOperand(storedSetter);
    pushOperand(object);
    pushOperand(property);
    pushOperand(isEnumerable);
  }
  explicit StoreGetterSetterInst(
      const StoreGetterSetterInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::StoreGetterSetterInstKind;
  }
};

class DeletePropertyInst : public Instruction {
  DeletePropertyInst(const DeletePropertyInst &) = delete;
  void operator=(const DeletePropertyInst &) = delete;

 protected:
  explicit DeletePropertyInst(ValueKind kind, Value *object, Value *property)
      : Instruction(kind) {
    pushOperand(object);
    pushOperand(property);
  }
  explicit DeletePropertyInst(
      const DeletePropertyInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

 public:
  enum { ObjectIdx, PropertyIdx };

  Value *getObject() const {
    return getOperand(ObjectIdx);
  }
  Value *getProperty() const {
    return getOperand(PropertyIdx);
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute();
  }

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), DeletePropertyInst);
  }
};

class DeletePropertyLooseInst : public DeletePropertyInst {
  DeletePropertyLooseInst(const DeletePropertyLooseInst &) = delete;
  void operator=(const DeletePropertyLooseInst &) = delete;

 public:
  explicit DeletePropertyLooseInst(Value *object, Value *property)
      : DeletePropertyInst(
            ValueKind::DeletePropertyLooseInstKind,
            object,
            property) {}
  explicit DeletePropertyLooseInst(
      const DeletePropertyLooseInst *src,
      llvh::ArrayRef<Value *> operands)
      : DeletePropertyInst(src, operands) {}

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::DeletePropertyLooseInstKind;
  }
};

class DeletePropertyStrictInst : public DeletePropertyInst {
  DeletePropertyStrictInst(const DeletePropertyStrictInst &) = delete;
  void operator=(const DeletePropertyStrictInst &) = delete;

 public:
  explicit DeletePropertyStrictInst(Value *object, Value *property)
      : DeletePropertyInst(
            ValueKind::DeletePropertyStrictInstKind,
            object,
            property) {}
  explicit DeletePropertyStrictInst(
      const DeletePropertyStrictInst *src,
      llvh::ArrayRef<Value *> operands)
      : DeletePropertyInst(src, operands) {}

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::DeletePropertyStrictInstKind;
  }
};

class BaseLoadPropertyInst : public Instruction {
  BaseLoadPropertyInst(const BaseLoadPropertyInst &) = delete;
  void operator=(const BaseLoadPropertyInst &) = delete;

 protected:
  explicit BaseLoadPropertyInst(ValueKind kind, Value *object, Value *property)
      : Instruction(kind) {
    pushOperand(object);
    pushOperand(property);
  }

 public:
  enum { ObjectIdx, PropertyIdx };

  Value *getObject() const {
    return getOperand(ObjectIdx);
  };
  Value *getProperty() const {
    return getOperand(PropertyIdx);
  }

  explicit BaseLoadPropertyInst(
      const BaseLoadPropertyInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute();
  }

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), BaseLoadPropertyInst);
  }
};

class LoadPropertyInst : public BaseLoadPropertyInst {
  LoadPropertyInst(const LoadPropertyInst &) = delete;
  void operator=(const LoadPropertyInst &) = delete;

 public:
  explicit LoadPropertyInst(Value *object, Value *property)
      : BaseLoadPropertyInst(
            ValueKind::LoadPropertyInstKind,
            object,
            property) {}
  explicit LoadPropertyInst(
      const LoadPropertyInst *src,
      llvh::ArrayRef<Value *> operands)
      : BaseLoadPropertyInst(src, operands) {}

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::LoadPropertyInstKind;
  }
};

class TryLoadGlobalPropertyInst : public BaseLoadPropertyInst {
  TryLoadGlobalPropertyInst(const TryLoadGlobalPropertyInst &) = delete;
  void operator=(const TryLoadGlobalPropertyInst &) = delete;

 public:
  LiteralString *getProperty() const {
    return cast<LiteralString>(BaseLoadPropertyInst::getProperty());
  }

  explicit TryLoadGlobalPropertyInst(
      Value *globalObject,
      LiteralString *property)
      : BaseLoadPropertyInst(
            ValueKind::TryLoadGlobalPropertyInstKind,
            globalObject,
            property) {
    assert(
        (llvh::isa<GlobalObject>(globalObject) ||
         llvh::isa<HBCGetGlobalObjectInst>(globalObject)) &&
        "globalObject must refer to the global object");
  }
  explicit TryLoadGlobalPropertyInst(
      const TryLoadGlobalPropertyInst *src,
      llvh::ArrayRef<Value *> operands)
      : BaseLoadPropertyInst(src, operands) {}

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::TryLoadGlobalPropertyInstKind;
  }
};

class AllocObjectInst : public Instruction {
  AllocObjectInst(const AllocObjectInst &) = delete;
  void operator=(const AllocObjectInst &) = delete;

 public:
  enum { SizeIdx, ParentObjectIdx };

  uint32_t getSize() const {
    return cast<LiteralNumber>(getOperand(SizeIdx))->asUInt32();
  }

  Value *getParentObject() const {
    return getOperand(ParentObjectIdx);
  }

  explicit AllocObjectInst(LiteralNumber *size, Value *parentObject)
      : Instruction(ValueKind::AllocObjectInstKind) {
    setType(*getInherentTypeImpl());
    assert(size->isUInt32Representible() && "size must be uint32");
    pushOperand(size);
    pushOperand(parentObject);
  }
  explicit AllocObjectInst(
      const AllocObjectInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createObject();
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::AllocObjectInstKind;
  }
};

class HBCAllocObjectFromBufferInst : public Instruction {
  HBCAllocObjectFromBufferInst(const HBCAllocObjectFromBufferInst &) = delete;
  void operator=(const HBCAllocObjectFromBufferInst &) = delete;

 public:
  enum { SizeIdx, FirstKeyIdx };

  using ObjectPropertyMap =
      llvh::SmallVector<std::pair<Literal *, Literal *>, 4>;

  /// \sizeHint is a hint for the VM regarding the final size of this object.
  /// It is the number of entries in the object declaration including
  /// non-literal ones. \prop_map is all the literal key/value entries.
  explicit HBCAllocObjectFromBufferInst(
      LiteralNumber *sizeHint,
      const ObjectPropertyMap &prop_map)
      : Instruction(ValueKind::HBCAllocObjectFromBufferInstKind) {
    setType(*getInherentTypeImpl());
    assert(sizeHint->isUInt32Representible() && "size hint must be uint32");
    pushOperand(sizeHint);
    for (size_t i = 0; i < prop_map.size(); i++) {
      pushOperand(prop_map[i].first);
      pushOperand(prop_map[i].second);
    }
  }
  explicit HBCAllocObjectFromBufferInst(
      const HBCAllocObjectFromBufferInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createObject();
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  /// Number of consecutive literal key/value pairs in the object.
  unsigned getKeyValuePairCount() const {
    return (getNumOperands() - FirstKeyIdx) / 2;
  }

  /// Return the \index 'd sequential literal key/value pair.
  std::pair<Literal *, Literal *> getKeyValuePair(unsigned index) const {
    return std::pair<Literal *, Literal *>{
        cast<Literal>(getOperand(FirstKeyIdx + 2 * index)),
        cast<Literal>(getOperand(FirstKeyIdx + 1 + 2 * index))};
  }

  LiteralNumber *getSizeHint() const {
    return cast<LiteralNumber>(getOperand(SizeIdx));
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::HBCAllocObjectFromBufferInstKind;
  }
};

class AllocObjectLiteralInst : public Instruction {
  AllocObjectLiteralInst(const AllocObjectLiteralInst &) = delete;
  void operator=(const AllocObjectLiteralInst &) = delete;

 public:
  using ObjectPropertyMap =
      llvh::SmallVector<std::pair<LiteralString *, Value *>, 4>;

  explicit AllocObjectLiteralInst(const ObjectPropertyMap &propMap)
      : Instruction(ValueKind::AllocObjectLiteralInstKind) {
    setType(*getInherentTypeImpl());
    for (size_t i = 0; i < propMap.size(); i++) {
      pushOperand(propMap[i].first);
      pushOperand(propMap[i].second);
    }
  }

  explicit AllocObjectLiteralInst(
      const AllocObjectLiteralInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createObject();
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  unsigned getKeyValuePairCount() const {
    return getNumOperands() / 2;
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::AllocObjectLiteralInstKind;
  }

  Literal *getKey(unsigned index) const {
    return cast<Literal>(getOperand(2 * index));
  }

  Value *getValue(unsigned index) const {
    return getOperand(2 * index + 1);
  }
};

class GetTemplateObjectInst : public Instruction {
  GetTemplateObjectInst(const GetTemplateObjectInst &) = delete;
  void operator=(const GetTemplateObjectInst &) = delete;

 public:
  enum { TemplateObjIDIdx, DupIdx, RawStartIdx };

  // Note: Operands are set up using the same layout as
  // HermesBuiltin_getTemplateObject.
  /// \param cookedStrings can contain either LiteralString or Undefined.
  explicit GetTemplateObjectInst(
      LiteralNumber *templateObjID,
      LiteralBool *dup,
      llvh::ArrayRef<LiteralString *> rawStrings,
      llvh::ArrayRef<Value *> cookedStrings)
      : Instruction(ValueKind::GetTemplateObjectInstKind) {
    if (dup->getValue()) {
      assert(
          cookedStrings.empty() &&
          "no cooked strings allowed if they are the same as raw");
    } else {
      assert(
          rawStrings.size() == cookedStrings.size() &&
          "same number of raw and cooked strings required");
    }
    pushOperand(templateObjID);
    pushOperand(dup);
    for (Value *v : rawStrings)
      pushOperand(v);
    for (Value *v : cookedStrings)
      pushOperand(v);
  }
  explicit GetTemplateObjectInst(
      const GetTemplateObjectInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createObject();
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    // Reads/writes the template cache, but only allocates new objects.
    return {};
  }

  /// \return the ID for the template object in cache.
  uint32_t getTemplateObjID() const {
    return cast<LiteralNumber>(getOperand(TemplateObjIDIdx))->asUInt32();
  }

  /// \return whether the cooked strings are the same as the raw strings.
  bool isDup() const {
    return cast<LiteralBool>(getOperand(DupIdx))->getValue();
  }

  /// \return how many raw strings are being stored (same as the number of
  /// cooked strings if they are not duped).
  uint32_t getNumStrings() const {
    return isDup() ? getNumOperands() - RawStartIdx
                   : (getNumOperands() - RawStartIdx) / 2;
  }

  /// \return the \p idx raw string.
  Value *getRawString(uint32_t idx) const {
    return getOperand(RawStartIdx + idx);
  }

  /// For template objects that aren't reusing raw strings as cooked strings,
  /// read the \p idx cooked string.
  Value *getCookedString(uint32_t idx) const {
    assert(!isDup() && "cooked strings are the same as raw strings");
    auto cookedStrIdx = RawStartIdx + getNumStrings();
    return getOperand(cookedStrIdx + idx);
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::GetTemplateObjectInstKind;
  }
};

class AllocArrayInst : public Instruction {
  AllocArrayInst(const AllocArrayInst &) = delete;
  void operator=(const AllocArrayInst &) = delete;

 public:
  enum { SizeHintIdx, ElementStartIdx };

  using ArrayValueList = llvh::SmallVector<Value *, 4>;

  explicit AllocArrayInst(ArrayValueList &val_list, LiteralNumber *sizeHint)
      : Instruction(ValueKind::AllocArrayInstKind) {
    // TODO: refine this type annotation to "array" ?.
    setType(*getInherentTypeImpl());
    pushOperand(sizeHint);
    for (auto val : val_list) {
      pushOperand(val);
    }
  }
  explicit AllocArrayInst(
      const AllocArrayInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  /// Return the size of array to be allocated.
  LiteralNumber *getSizeHint() const {
    return cast<LiteralNumber>(getOperand(SizeHintIdx));
  }

  /// Number of consecutive literal elements in the array.
  unsigned getElementCount() const {
    return getNumOperands() - 1;
  }

  /// Return the real position and value of the \p index's non-elision
  /// element.
  Value *getArrayElement(unsigned index) const {
    return getOperand(ElementStartIdx + index);
  }

  /// Returns the index of the first non-literal element.
  /// Returns -1 if every element is a literal.
  int getFirstNonLiteralIndex() const {
    for (unsigned i = 0, e = getElementCount(); i < e; ++i) {
      if (!llvh::isa<Literal>(getArrayElement(i)))
        return i;
    }
    return -1;
  }

  /// Returns true if ths is an array with only literal elements.
  bool isLiteralArray() const {
    return getFirstNonLiteralIndex() == -1;
  }

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createObject();
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::AllocArrayInstKind;
  }
};

class AllocFastArrayInst : public Instruction {
  AllocFastArrayInst(const AllocFastArrayInst &) = delete;
  void operator=(const AllocFastArrayInst &) = delete;

 public:
  enum { CapacityIdx };

  explicit AllocFastArrayInst(LiteralNumber *sizeHint)
      : Instruction(ValueKind::AllocFastArrayInstKind) {
    setType(*getInherentTypeImpl());
    pushOperand(sizeHint);
  }
  explicit AllocFastArrayInst(
      const AllocFastArrayInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  /// \return the capacity to allocate the array with.
  LiteralNumber *getCapacity() const {
    return cast<LiteralNumber>(getOperand(CapacityIdx));
  }

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createObject();
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::AllocFastArrayInstKind;
  }
};

class CreateArgumentsInst : public Instruction {
  CreateArgumentsInst(const CreateArgumentsInst &) = delete;
  void operator=(const CreateArgumentsInst &) = delete;

 public:
  explicit CreateArgumentsInst(ValueKind kind) : Instruction(kind) {
    setType(*getInherentTypeImpl());
  }
  explicit CreateArgumentsInst(
      const CreateArgumentsInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createObject();
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), CreateArgumentsInst);
  }
};

class CreateArgumentsLooseInst : public CreateArgumentsInst {
  CreateArgumentsLooseInst(const CreateArgumentsLooseInst &) = delete;
  void operator=(const CreateArgumentsLooseInst &) = delete;

 public:
  explicit CreateArgumentsLooseInst()
      : CreateArgumentsInst(ValueKind::CreateArgumentsLooseInstKind) {}
  explicit CreateArgumentsLooseInst(
      const CreateArgumentsLooseInst *src,
      llvh::ArrayRef<Value *> operands)
      : CreateArgumentsInst(src, operands) {}

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::CreateArgumentsLooseInstKind;
  }
};

class CreateArgumentsStrictInst : public CreateArgumentsInst {
  CreateArgumentsStrictInst(const CreateArgumentsStrictInst &) = delete;
  void operator=(const CreateArgumentsStrictInst &) = delete;

 public:
  explicit CreateArgumentsStrictInst()
      : CreateArgumentsInst(ValueKind::CreateArgumentsStrictInstKind) {}
  explicit CreateArgumentsStrictInst(
      const CreateArgumentsStrictInst *src,
      llvh::ArrayRef<Value *> operands)
      : CreateArgumentsInst(src, operands) {}

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::CreateArgumentsStrictInstKind;
  }
};

class CreateRegExpInst : public Instruction {
  CreateRegExpInst(const CreateRegExpInst &) = delete;
  void operator=(const CreateRegExpInst &) = delete;

 public:
  enum { PatternIdx, FlagsIdx };

  LiteralString *getPattern() const {
    return cast<LiteralString>(getOperand(PatternIdx));
  }
  LiteralString *getFlags() const {
    return cast<LiteralString>(getOperand(FlagsIdx));
  }

  explicit CreateRegExpInst(LiteralString *pattern, LiteralString *flags)
      : Instruction(ValueKind::CreateRegExpInstKind) {
    setType(*getInherentTypeImpl());
    pushOperand(pattern);
    pushOperand(flags);
  }
  explicit CreateRegExpInst(
      const CreateRegExpInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createObject();
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::CreateRegExpInstKind;
  }
};

class UnaryOperatorInst : public SingleOperandInst {
 public:
 private:
  UnaryOperatorInst(const UnaryOperatorInst &) = delete;
  void operator=(const UnaryOperatorInst &) = delete;

  // A list of textual representation of the operators above.
  static const char *opStringRepr[HERMES_IR_CLASS_LENGTH(UnaryOperatorInst)];

 public:
  // Convert the operator string \p into the enum representation or assert
  // fail if the string is invalud.
  static ValueKind parseOperator(llvh::StringRef op);

  /// \return the string representation of the operator.
  llvh::StringRef getOperatorStr() {
    return opStringRepr[HERMES_IR_KIND_TO_OFFSET(UnaryOperatorInst, getKind())];
  }

  explicit UnaryOperatorInst(ValueKind kind, Value *value, Type type)
      : SingleOperandInst(kind, value) {
    assert(HERMES_IR_KIND_IN_CLASS(kind, UnaryOperatorInst));
    setType(type);
  }
  explicit UnaryOperatorInst(
      const UnaryOperatorInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const;

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), UnaryOperatorInst);
  }
};

class BinaryOperatorInst : public Instruction {
 public:
  // A list of textual representation of the operators above.
  static const char *opStringRepr[HERMES_IR_CLASS_LENGTH(BinaryOperatorInst)];

  // A list of textual representation of the assignment operators that match the
  // operators above.
  static const char
      *assignmentOpStringRepr[HERMES_IR_CLASS_LENGTH(BinaryOperatorInst)];

 private:
  BinaryOperatorInst(const BinaryOperatorInst &) = delete;
  void operator=(const BinaryOperatorInst &) = delete;

 public:
  enum { LeftHandSideIdx, RightHandSideIdx };

  Value *getLeftHandSide() const {
    return getOperand(LeftHandSideIdx);
  }
  Value *getRightHandSide() const {
    return getOperand(RightHandSideIdx);
  }

  // Convert the operator string \p into the enum representation or assert
  // fail if the string is invalud.
  static ValueKind parseOperator(llvh::StringRef op);

  // Convert the assignment operator string \p into the enum representation or
  // assert fail if the string is invalud.
  static ValueKind parseAssignmentOperator(llvh::StringRef op);

  /// \return the string representation of the operator.
  llvh::StringRef getOperatorStr() {
    return opStringRepr[HERMES_IR_KIND_TO_OFFSET(
        BinaryOperatorInst, getKind())];
  }

  explicit BinaryOperatorInst(ValueKind kind, Value *left, Value *right)
      : Instruction(kind) {
    pushOperand(left);
    pushOperand(right);
  }
  explicit BinaryOperatorInst(
      const BinaryOperatorInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return getBinarySideEffect(
        getLeftHandSide()->getType(), getRightHandSide()->getType(), getKind());
  }

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), BinaryOperatorInst);
  }

  /// Calculate the side effect of a binary operator, given inferred types of
  /// its arguments.
  static SideEffect
  getBinarySideEffect(Type leftTy, Type rightTy, ValueKind op);
};

class CatchInst : public Instruction {
  CatchInst(const CatchInst &) = delete;
  void operator=(const CatchInst &) = delete;

 public:
  explicit CatchInst() : Instruction(ValueKind::CatchInstKind) {}
  explicit CatchInst(const CatchInst *src, llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createUnknown().setFirstInBlock();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::CatchInstKind;
  }
};

class ThrowInst : public TerminatorInst {
  ThrowInst(const ThrowInst &) = delete;
  void operator=(const ThrowInst &) = delete;

 public:
  enum { ThrownValueIdx };

  explicit ThrowInst(Value *thrownValue)
      : TerminatorInst(ValueKind::ThrowInstKind) {
    pushOperand(thrownValue);
  }
  explicit ThrowInst(const ThrowInst *src, llvh::ArrayRef<Value *> operands)
      : TerminatorInst(src, operands) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setThrow();
  }

  Value *getThrownValue() const {
    return getOperand(ThrownValueIdx);
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::ThrowInstKind;
  }

  unsigned getNumSuccessorsImpl() const {
    return 0;
  }
  BasicBlock *getSuccessorImpl(unsigned idx) const {
    llvm_unreachable("ThrowInst has no successor!");
  }
  void setSuccessorImpl(unsigned idx, BasicBlock *B) {
    llvm_unreachable("ThrowInst has no successor!");
  }
};

class ThrowTypeErrorInst : public TerminatorInst {
  ThrowTypeErrorInst(const ThrowTypeErrorInst &) = delete;
  void operator=(const ThrowTypeErrorInst &) = delete;

 public:
  enum { MessageIdx };

  explicit ThrowTypeErrorInst(Value *message)
      : TerminatorInst(ValueKind::ThrowTypeErrorInstKind) {
    pushOperand(message);
  }
  explicit ThrowTypeErrorInst(
      const ThrowTypeErrorInst *src,
      llvh::ArrayRef<Value *> operands)
      : TerminatorInst(src, operands) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setThrow();
  }

  Value *getMessage() const {
    return getOperand(MessageIdx);
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::ThrowTypeErrorInstKind;
  }

  unsigned getNumSuccessorsImpl() const {
    return 0;
  }
  BasicBlock *getSuccessorImpl(unsigned idx) const {
    llvm_unreachable("ThrowInst has no successor!");
  }
  void setSuccessorImpl(unsigned idx, BasicBlock *B) {
    llvm_unreachable("ThrowInst has no successor!");
  }
};

class SwitchInst : public TerminatorInst {
  SwitchInst(const SwitchInst &) = delete;
  void operator=(const SwitchInst &) = delete;

 public:
  enum { InputIdx, DefaultBlockIdx, FirstCaseIdx };

  using ValueListType = llvh::SmallVector<Literal *, 8>;
  using BasicBlockListType = llvh::SmallVector<BasicBlock *, 8>;

  /// \returns the number of switch case values.
  unsigned getNumCasePair() const;

  /// Returns the n'th pair of value-basicblock that represent a case
  /// destination.
  std::pair<Literal *, BasicBlock *> getCasePair(unsigned i) const;

  /// \returns the destination of the default target.
  BasicBlock *getDefaultDestination() const;

  /// \returns the input value. This is the value we switch on.
  Value *getInputValue() const;

  explicit SwitchInst(
      Value *input,
      BasicBlock *defaultBlock,
      const ValueListType &values,
      const BasicBlockListType &blocks);
  explicit SwitchInst(const SwitchInst *src, llvh::ArrayRef<Value *> operands)
      : TerminatorInst(src, operands) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::SwitchInstKind;
  }

  unsigned getNumSuccessorsImpl() const {
    return getNumCasePair() + 1;
  }
  BasicBlock *getSuccessorImpl(unsigned idx) const;
  void setSuccessorImpl(unsigned idx, BasicBlock *B);
};

class GetPNamesInst : public TerminatorInst {
  GetPNamesInst(const GetPNamesInst &) = delete;
  void operator=(const GetPNamesInst &) = delete;

 public:
  enum { IteratorIdx, BaseIdx, IndexIdx, SizeIdx, OnEmptyIdx, OnSomeIdx };

  explicit GetPNamesInst(
      BasicBlock *parent,
      AllocStackInst *iteratorAddr,
      AllocStackInst *baseAddr,
      AllocStackInst *indexAddr,
      AllocStackInst *sizeAddr,
      BasicBlock *onEmpty,
      BasicBlock *onSome);
  explicit GetPNamesInst(
      const GetPNamesInst *src,
      llvh::ArrayRef<Value *> operands)
      : TerminatorInst(src, operands) {}

  Value *getIterator() const {
    return getOperand(IteratorIdx);
  }
  Value *getBase() const {
    return getOperand(BaseIdx);
  }
  Value *getIndex() const {
    return getOperand(IndexIdx);
  }
  Value *getSize() const {
    return getOperand(SizeIdx);
  }
  BasicBlock *getOnEmptyDest() const {
    return cast<BasicBlock>(getOperand(OnEmptyIdx));
  }
  BasicBlock *getOnSomeDest() const {
    return cast<BasicBlock>(getOperand(OnSomeIdx));
  }

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute().setReadStack().setWriteStack();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::GetPNamesInstKind;
  }

  unsigned getNumSuccessorsImpl() const {
    return 2;
  }
  BasicBlock *getSuccessorImpl(unsigned idx) const {
    if (idx == 0)
      return getOnEmptyDest();
    if (idx == 1)
      return getOnSomeDest();
    llvm_unreachable("GetPNamesInst only have 2 successors!");
  }
  void setSuccessorImpl(unsigned idx, BasicBlock *B) {
    if (idx == 0)
      return setOperand(B, OnEmptyIdx);
    if (idx == 1)
      return setOperand(B, OnSomeIdx);
    llvm_unreachable("GetPNamesInst only have 2 successors!");
  }
};

class GetNextPNameInst : public TerminatorInst {
  GetNextPNameInst(const GetNextPNameInst &) = delete;
  void operator=(const GetNextPNameInst &) = delete;

 public:
  enum {
    PropertyIdx,
    BaseIdx,
    IndexIdx,
    SizeIdx,
    IteratorIdx,
    OnLastIdx,
    OnSomeIdx
  };

  explicit GetNextPNameInst(
      BasicBlock *parent,
      AllocStackInst *propertyAddr,
      AllocStackInst *baseAddr,
      AllocStackInst *indexAddr,
      AllocStackInst *sizeAddr,
      AllocStackInst *iteratorAddr,
      BasicBlock *onLast,
      BasicBlock *onSome);
  explicit GetNextPNameInst(
      const GetNextPNameInst *src,
      llvh::ArrayRef<Value *> operands)
      : TerminatorInst(src, operands) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute().setReadStack().setWriteStack();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::GetNextPNameInstKind;
  }

  Value *getPropertyAddr() const {
    return getOperand(PropertyIdx);
  }
  Value *getBaseAddr() const {
    return getOperand(BaseIdx);
  }
  Value *getIndexAddr() const {
    return getOperand(IndexIdx);
  }
  Value *getSizeAddr() const {
    return getOperand(SizeIdx);
  }
  Value *getIteratorAddr() const {
    return getOperand(IteratorIdx);
  }
  BasicBlock *getOnLastDest() const {
    return cast<BasicBlock>(getOperand(OnLastIdx));
  }
  BasicBlock *getOnSomeDest() const {
    return cast<BasicBlock>(getOperand(OnSomeIdx));
  }

  unsigned getNumSuccessorsImpl() const {
    return 2;
  }
  BasicBlock *getSuccessorImpl(unsigned idx) const {
    if (idx == 0)
      return getOnLastDest();
    if (idx == 1)
      return getOnSomeDest();
    llvm_unreachable("GetNextPNameInst only have 2 successors!");
  }
  void setSuccessorImpl(unsigned idx, BasicBlock *B) {
    if (idx == 0)
      return setOperand(B, OnLastIdx);
    if (idx == 1)
      return setOperand(B, OnSomeIdx);
    llvm_unreachable("GetNextPNameInst only have 2 successors!");
  }
};

class TryStartInst : public TerminatorInst {
  TryStartInst(const TryStartInst &) = delete;
  void operator=(const TryStartInst &) = delete;

  // CatchTarget is positioned before TryBody in the successor list,
  // so that during Post Order Scan, we lay out TryBody first.
 public:
  enum { CatchTargetBlockIdx, TryBodyBlockIdx };

  explicit TryStartInst(BasicBlock *tryBodyBlock, BasicBlock *catchTargetBlock)
      : TerminatorInst(ValueKind::TryStartInstKind) {
    pushOperand(catchTargetBlock);
    pushOperand(tryBodyBlock);
  }
  explicit TryStartInst(
      const TryStartInst *src,
      llvh::ArrayRef<Value *> operands)
      : TerminatorInst(src, operands) {}

  BasicBlock *getTryBody() const {
    return cast<BasicBlock>(getOperand(TryBodyBlockIdx));
  }
  BasicBlock *getCatchTarget() const {
    return cast<BasicBlock>(getOperand(CatchTargetBlockIdx));
  }

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createUnknown();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::TryStartInstKind;
  }

  unsigned getNumSuccessorsImpl() const {
    return 2;
  }
  BasicBlock *getSuccessorImpl(unsigned idx) const {
    return cast<BasicBlock>(getOperand(idx));
  }
  void setSuccessorImpl(unsigned idx, BasicBlock *B) {
    setOperand(B, idx);
  }
};

class TryEndInst : public Instruction {
  TryEndInst(const TryEndInst &) = delete;
  void operator=(const TryEndInst &) = delete;

 public:
  explicit TryEndInst() : Instruction(ValueKind::TryEndInstKind) {}
  explicit TryEndInst(const TryEndInst *src, llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createUnknown().setFirstInBlock();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::TryEndInstKind;
  }
};

class PhiInst : public Instruction {
  PhiInst(const PhiInst &) = delete;
  void operator=(const PhiInst &) = delete;

 public:
  using ValueListType = llvh::SmallVector<Value *, 8>;
  using BasicBlockListType = llvh::SmallVector<BasicBlock *, 8>;

  /// \returns the number of phi incoming values.
  unsigned getNumEntries() const;

  /// Returns the n'th pair of value-basicblock that represent a case
  /// destination.
  std::pair<Value *, BasicBlock *> getEntry(unsigned i) const;

  /// Update the n'th pair of value-basicblock that represent a case
  /// destination.
  void updateEntry(unsigned i, Value *val, BasicBlock *BB);

  /// Add a pair of value-basicblock that represent a case destination.
  void addEntry(Value *val, BasicBlock *BB);

  /// Remove an entry pair (incoming basic block and value) at index \p index.
  void removeEntry(unsigned index);

  /// Remove an entry pair (incoming basic block and value) for incoming block
  // \p BB.
  void removeEntry(BasicBlock *BB);

  explicit PhiInst(
      const ValueListType &values,
      const BasicBlockListType &blocks);
  explicit PhiInst(const PhiInst *src, llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }
  bool acceptsEmptyTypeImpl() const {
    return true;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setFirstInBlock();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::PhiInstKind;
  }

 private:
  /// Remove an entry without recalculating the result type.
  void removeEntryHelper(unsigned index);

  /// Calculate the result type as a union of all incoming types and update it.
  void recalculateResultType();
};

class MovInst : public SingleOperandInst {
  MovInst(const MovInst &) = delete;
  void operator=(const MovInst &) = delete;

 public:
  explicit MovInst(Value *input)
      : SingleOperandInst(ValueKind::MovInstKind, input) {
    setType(input->getType());
  }
  explicit MovInst(const MovInst *src, llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }
  bool acceptsEmptyTypeImpl() const {
    return true;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setIdempotent();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::MovInstKind;
  }
};

/// ImplicitMovInst may be emitted as part of lowering to express instructions
/// which perform a Mov as part of their implementation, to registers not
/// explicitly marked as a destination. They serve as IR optimization barriers
/// but emit no bytecode.
class ImplicitMovInst : public SingleOperandInst {
 public:
  explicit ImplicitMovInst(Value *input)
      : SingleOperandInst(ValueKind::ImplicitMovInstKind, input) {
    setType(input->getType());
  }

  explicit ImplicitMovInst(
      const ImplicitMovInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::ImplicitMovInstKind;
  }
};

class CoerceThisNSInst : public SingleOperandInst {
  CoerceThisNSInst(const MovInst &) = delete;
  void operator=(const CoerceThisNSInst &) = delete;

 public:
  explicit CoerceThisNSInst(Value *input)
      : SingleOperandInst(ValueKind::CoerceThisNSInstKind, input) {
    setType(*getInherentTypeImpl());
  }
  explicit CoerceThisNSInst(
      const CoerceThisNSInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createObject();
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::CoerceThisNSInstKind;
  }
};

class DebuggerInst : public Instruction {
  DebuggerInst(const DebuggerInst &) = delete;
  void operator=(const DebuggerInst &) = delete;

 public:
  explicit DebuggerInst() : Instruction(ValueKind::DebuggerInstKind) {}
  explicit DebuggerInst(
      const DebuggerInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::DebuggerInstKind;
  }
};

class GetNewTargetInst : public Instruction {
  GetNewTargetInst(const GetNewTargetInst &) = delete;
  void operator=(const GetNewTargetInst &) = delete;

 public:
  enum { GetNewTargetParamIdx };

  explicit GetNewTargetInst(Value *param)
      : Instruction(ValueKind::GetNewTargetInstKind) {
    pushOperand(param);
    setType(param->getType());
  }
  explicit GetNewTargetInst(
      const GetNewTargetInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setIdempotent();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::GetNewTargetInstKind;
  }
};

/// Throw if the operand is "empty", otherwise return it.
class ThrowIfInst : public Instruction {
  ThrowIfInst(const ThrowIfInst &) = delete;
  void operator=(const ThrowIfInst &) = delete;

  /// Record the result type when the instruction was created. The recorded type
  /// must be correct with respect to the instructions that consume it, which
  /// is an invariant enforced by IRGen.
  /// If through optimization we get to a point where the input type is Empty,
  /// meaning that the TDZ is always going to throw, the result type of
  /// ThrowIfInst would theoretically need to be something that represents
  /// an "unreachable" type. Such a type would violate the type requirements of
  /// all instructions consuming the result of ThrowIf, even though they
  /// will never execute. Handling this "unreachable" type everywhere would
  /// require a lot of complexity.
  /// Instead, when we get to that point, we simply return the recorded type.
  Type savedResultType_;

 public:
  enum { CheckedValueIdx, InvalidTypesIdx };

  explicit ThrowIfInst(Value *checkedValue, LiteralIRType *invalidTypes)
      : Instruction(ValueKind::ThrowIfInstKind),
        savedResultType_(Type::createAnyType()) {
    assert(
        !invalidTypes->getData().isNoType() &&
        invalidTypes->getData().isSubsetOf(
            Type::unionTy(Type::createEmpty(), Type::createUninit())) &&
        "invalidTypes set can only contain Empty or Uninit");
    pushOperand(checkedValue);
    pushOperand(invalidTypes);

    // Calculate the correct result type, to preserve invariants for
    // instructions that use this result.
    setType(Type::subtractTy(checkedValue->getType(), invalidTypes->getData()));
    updateSavedResultType();
  }
  explicit ThrowIfInst(const ThrowIfInst *src, llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands), savedResultType_(src->savedResultType_) {}

  /// For use only by TypeInference. Returns the result type that was recorded
  /// before type inference. Please see doc for \c savedResultType_.
  Type getSavedResultType() const {
    return savedResultType_;
  }
  /// Record the current result type in \c savedResultType_ . This method only
  /// needs to be called externally (by IRGen) if the operand type wasn't
  /// correct during construction and was changed afterwards.
  void updateSavedResultType() {
    savedResultType_ = getType();
  }

  Value *getCheckedValue() const {
    return getOperand(CheckedValueIdx);
  }
  void setCheckedValue(Value *value) {
    setOperand(value, CheckedValueIdx);
  }

  LiteralIRType *getInvalidTypes() const {
    return cast<LiteralIRType>(getOperand(InvalidTypesIdx));
  }
  void setInvalidTypes(LiteralIRType *invalidTypes) {
    assert(
        !invalidTypes->getData().isNoType() &&
        invalidTypes->getData().isSubsetOf(
            Type::unionTy(Type::createEmpty(), Type::createUninit())) &&
        "invalidTypes set can only contain Empty or Uninit");
    setOperand(invalidTypes, InvalidTypesIdx);
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  bool acceptsEmptyTypeImpl() const {
    return true;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setThrow();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::ThrowIfInstKind;
  }
};

class HBCResolveParentEnvironmentInst : public BaseScopeInst {
  HBCResolveParentEnvironmentInst(const HBCResolveParentEnvironmentInst &) =
      delete;
  void operator=(const HBCResolveParentEnvironmentInst &) = delete;

  enum { ParentScopeParamIdx = BaseScopeInst::LAST_IDX };

 public:
  explicit HBCResolveParentEnvironmentInst(
      VariableScope *scope,
      JSDynamicParam *parentScopeParam)
      : BaseScopeInst(ValueKind::HBCResolveParentEnvironmentInstKind, scope) {
    pushOperand(parentScopeParam);
  }
  explicit HBCResolveParentEnvironmentInst(
      const HBCResolveParentEnvironmentInst *src,
      llvh::ArrayRef<Value *> operands)
      : BaseScopeInst(src, operands) {}

  JSDynamicParam *getParentScopeParam() const {
    return cast<JSDynamicParam>(getOperand(ParentScopeParamIdx));
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setIdempotent();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::HBCResolveParentEnvironmentInstKind;
  }
};

class HBCStoreToEnvironmentInst : public Instruction {
  HBCStoreToEnvironmentInst(const HBCStoreToEnvironmentInst &) = delete;
  void operator=(const HBCStoreToEnvironmentInst &) = delete;

 public:
  enum { EnvIdx, ValueIdx, NameIdx };

  explicit HBCStoreToEnvironmentInst(Value *env, Value *toPut, Variable *var)
      : Instruction(ValueKind::HBCStoreToEnvironmentInstKind) {
    pushOperand(env);
    pushOperand(toPut);
    pushOperand(var);
  }
  explicit HBCStoreToEnvironmentInst(
      const HBCStoreToEnvironmentInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Variable *getResolvedName() const {
    return cast<Variable>(getOperand(NameIdx));
  }
  Value *getEnvironment() const {
    return getOperand(EnvIdx);
  }
  Value *getStoredValue() const {
    return getOperand(ValueIdx);
  }

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setWriteFrame().setIdempotent();
  }

  bool acceptsEmptyTypeImpl() const {
    return true;
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::HBCStoreToEnvironmentInstKind;
  }
};

class HBCLoadFromEnvironmentInst : public Instruction {
  HBCLoadFromEnvironmentInst(const HBCLoadFromEnvironmentInst &) = delete;
  void operator=(const HBCLoadFromEnvironmentInst &) = delete;

 public:
  enum { EnvIdx, NameIdx };

  explicit HBCLoadFromEnvironmentInst(Value *env, Variable *var)
      : Instruction(ValueKind::HBCLoadFromEnvironmentInstKind) {
    setType(var->getType());
    pushOperand(env);
    pushOperand(var);
  }
  explicit HBCLoadFromEnvironmentInst(
      const HBCLoadFromEnvironmentInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Variable *getResolvedName() const {
    return cast<Variable>(getOperand(NameIdx));
  }
  Value *getEnvironment() const {
    return getOperand(EnvIdx);
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setReadFrame().setIdempotent();
  }

  bool acceptsEmptyTypeImpl() const {
    return true;
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::HBCLoadFromEnvironmentInstKind;
  }
};

class SwitchImmInst : public TerminatorInst {
  SwitchImmInst(const SwitchImmInst &) = delete;
  void operator=(const SwitchImmInst &) = delete;

 public:
  enum { InputIdx, DefaultBlockIdx, MinValueIdx, SizeIdx, FirstCaseIdx };

  using ValueListType = llvh::SmallVector<LiteralNumber *, 8>;
  using BasicBlockListType = llvh::SmallVector<BasicBlock *, 8>;

  /// \returns the number of switch case values.
  unsigned getNumCasePair() const {
    // The number of cases is computed as the total number of operands, minus
    // the input value and the default basic block. Take this number and divide
    // it in two, because we are counting pairs.
    return (getNumOperands() - FirstCaseIdx) / 2;
  }

  /// Returns the n'th pair of value-basicblock that represent a case
  /// destination.
  std::pair<LiteralNumber *, BasicBlock *> getCasePair(unsigned i) const {
    // The values and lables are twined together. Find the index of the pair
    // that we are fetching and return the two values.
    unsigned base = i * 2 + FirstCaseIdx;
    return std::make_pair(
        cast<LiteralNumber>(getOperand(base)),
        cast<BasicBlock>(getOperand(base + 1)));
  }

  /// \returns the destination of the default target.
  BasicBlock *getDefaultDestination() const {
    return cast<BasicBlock>(getOperand(DefaultBlockIdx));
  }

  /// \returns the input value. This is the value we switch on.
  Value *getInputValue() const {
    return getOperand(InputIdx);
  }

  uint32_t getMinValue() const {
    return cast<LiteralNumber>(getOperand(MinValueIdx))->asUInt32();
  }

  uint32_t getSize() const {
    return cast<LiteralNumber>(getOperand(SizeIdx))->asUInt32();
  }

  /// \p input is the discriminator value.
  /// \p defaultBlock is the block to jump to if nothing matches.
  /// \p minValue the smallest (integer) value of all switch cases.
  /// \p size     the difference between minValue and the largest value + 1.
  explicit SwitchImmInst(
      Value *input,
      BasicBlock *defaultBlock,
      LiteralNumber *minValue,
      LiteralNumber *size,
      const ValueListType &values,
      const BasicBlockListType &blocks);
  explicit SwitchImmInst(
      const SwitchImmInst *src,
      llvh::ArrayRef<Value *> operands)
      : TerminatorInst(src, operands) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::SwitchImmInstKind;
  }

  unsigned getNumSuccessorsImpl() const {
    return getNumCasePair() + 1;
  }
  BasicBlock *getSuccessorImpl(unsigned idx) const;
  void setSuccessorImpl(unsigned idx, BasicBlock *B);
};

class SaveAndYieldInst : public TerminatorInst {
  SaveAndYieldInst(const SaveAndYieldInst &) = delete;
  void operator=(const SaveAndYieldInst &) = delete;

 public:
  enum { ResultIdx, NextBlockIdx };

  Value *getResult() const {
    return getOperand(ResultIdx);
  }

  BasicBlock *getNextBlock() const {
    return cast<BasicBlock>(getOperand(NextBlockIdx));
  }

  explicit SaveAndYieldInst(Value *result, BasicBlock *nextBlock)
      : TerminatorInst(ValueKind::SaveAndYieldInstKind) {
    pushOperand(result);
    pushOperand(nextBlock);
  }
  explicit SaveAndYieldInst(
      const SaveAndYieldInst *src,
      llvh::ArrayRef<Value *> operands)
      : TerminatorInst(src, operands) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute();
  }

  unsigned getNumSuccessorsImpl() const {
    return 1;
  }
  BasicBlock *getSuccessorImpl(unsigned idx) const {
    assert(idx == 0 && "SaveAndYieldInst should only have 1 successor");
    return getNextBlock();
  }
  void setSuccessorImpl(unsigned idx, BasicBlock *B) {
    assert(idx == 0 && "CompareBranchInst only have 2 successors!");
    setOperand(B, NextBlockIdx);
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::SaveAndYieldInstKind;
  }
};

class DirectEvalInst : public Instruction {
  DirectEvalInst(const DirectEvalInst &) = delete;
  void operator=(const DirectEvalInst &) = delete;

 public:
  enum { EvalTextIdx = 0, StrictCallerIdx = 1 };

  explicit DirectEvalInst(Value *evalText, LiteralBool *strictCaller)
      : Instruction(ValueKind::DirectEvalInstKind) {
    pushOperand(evalText);
    pushOperand(strictCaller);
  }
  explicit DirectEvalInst(
      const DirectEvalInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Value *getEvalText() const {
    return getOperand(EvalTextIdx);
  }
  bool getStrictCaller() const {
    return llvh::cast<LiteralBool>(getOperand(StrictCallerIdx))->getValue();
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    // This is equivalent to executing an inner function.
    return SideEffect::createExecute();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::DirectEvalInstKind;
  }
};

class DeclareGlobalVarInst : public SingleOperandInst {
  DeclareGlobalVarInst(const DeclareGlobalVarInst &) = delete;
  void operator=(const DeclareGlobalVarInst &) = delete;

 public:
  enum { NameIdx = 0 };

  explicit DeclareGlobalVarInst(LiteralString *value)
      : SingleOperandInst(ValueKind::DeclareGlobalVarInstKind, value) {}
  explicit DeclareGlobalVarInst(
      const DeclareGlobalVarInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  LiteralString *getName() const {
    return llvh::cast<LiteralString>(getSingleOperand());
  }

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::DeclareGlobalVarInstKind;
  }
};

class HBCCreateFunctionEnvironmentInst : public BaseScopeInst {
  HBCCreateFunctionEnvironmentInst(const HBCCreateFunctionEnvironmentInst &) =
      delete;
  void operator=(const HBCCreateFunctionEnvironmentInst &) = delete;

  enum { ParentScopeParamIdx = BaseScopeInst::LAST_IDX };

 public:
  explicit HBCCreateFunctionEnvironmentInst(
      VariableScope *scope,
      JSDynamicParam *parentScopeParam)
      : BaseScopeInst(ValueKind::HBCCreateFunctionEnvironmentInstKind, scope) {
    pushOperand(parentScopeParam);
  }
  explicit HBCCreateFunctionEnvironmentInst(
      const HBCCreateFunctionEnvironmentInst *src,
      llvh::ArrayRef<Value *> operands)
      : BaseScopeInst(src, operands) {}

  JSDynamicParam *getParentScopeParam() const {
    return cast<JSDynamicParam>(getOperand(ParentScopeParamIdx));
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::HBCCreateFunctionEnvironmentInstKind;
  }
};

class HBCProfilePointInst : public Instruction {
  HBCProfilePointInst(const HBCProfilePointInst &) = delete;
  void operator=(const HBCProfilePointInst &) = delete;

  uint16_t pointIndex_{0};

 public:
  explicit HBCProfilePointInst(uint16_t pointIndex)
      : Instruction(ValueKind::HBCProfilePointInstKind),
        pointIndex_(pointIndex) {}

  explicit HBCProfilePointInst(
      const HBCProfilePointInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands), pointIndex_(src->pointIndex_) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::HBCProfilePointInstKind;
  }

  uint16_t getPointIndex() const {
    return pointIndex_;
  }
};

class HBCLoadConstInst : public SingleOperandInst {
  HBCLoadConstInst(const HBCLoadConstInst &) = delete;
  void operator=(const HBCLoadConstInst &) = delete;

 public:
  explicit HBCLoadConstInst(Literal *input)
      : SingleOperandInst(ValueKind::HBCLoadConstInstKind, input) {
    setType(input->getType());
  }
  explicit HBCLoadConstInst(
      const HBCLoadConstInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  Literal *getConst() const {
    return cast<Literal>(getSingleOperand());
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setIdempotent();
  }

  bool acceptsEmptyTypeImpl() const {
    return true;
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::HBCLoadConstInstKind;
  }
};

/// Load a formal parameter. Parameter 0 is "this", the rest of the params start
/// at index 1.
class LoadParamInst : public SingleOperandInst {
  LoadParamInst(const LoadParamInst &) = delete;
  void operator=(const LoadParamInst &) = delete;

 public:
  explicit LoadParamInst(JSDynamicParam *param)
      : SingleOperandInst(ValueKind::LoadParamInstKind, param) {
    setType(param->getType());
  }
  explicit LoadParamInst(
      const LoadParamInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  JSDynamicParam *getParam() const {
    return cast<JSDynamicParam>(getSingleOperand());
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::LoadParamInstKind;
  }
};

/// Load the "this" parameter in non-strict mode, which coerces it to an object,
/// boxing primitives and converting "null" and "undefined" to the global
/// object.
class LIRGetThisNSInst : public Instruction {
  LIRGetThisNSInst(const LIRGetThisNSInst &) = delete;
  void operator=(const LIRGetThisNSInst &) = delete;

 public:
  explicit LIRGetThisNSInst() : Instruction(ValueKind::LIRGetThisNSInstKind) {}
  explicit LIRGetThisNSInst(
      const LIRGetThisNSInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::LIRGetThisNSInstKind;
  }
};

// Get `arguments.length`, without having to create a real array.
class HBCGetArgumentsLengthInst : public SingleOperandInst {
  HBCGetArgumentsLengthInst(const HBCGetArgumentsLengthInst &) = delete;
  void operator=(const HBCGetArgumentsLengthInst &) = delete;

 public:
  explicit HBCGetArgumentsLengthInst(Value *lazyRegValue)
      : SingleOperandInst(
            ValueKind::HBCGetArgumentsLengthInstKind,
            lazyRegValue) {}
  explicit HBCGetArgumentsLengthInst(
      const HBCGetArgumentsLengthInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  Value *getLazyRegister() const {
    return getSingleOperand();
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::HBCGetArgumentsLengthInstKind;
  }
};

// Get `arguments[i]` without having to create a real array.
// This is the base class for the strict and loose variants defined below.
class HBCGetArgumentsPropByValInst : public Instruction {
  HBCGetArgumentsPropByValInst(const HBCGetArgumentsPropByValInst &) = delete;
  void operator=(const HBCGetArgumentsPropByValInst &) = delete;

 protected:
  explicit HBCGetArgumentsPropByValInst(
      ValueKind kind,
      Value *index,
      AllocStackInst *reg)
      : Instruction(kind) {
    pushOperand(index);
    pushOperand(reg);
  }
  explicit HBCGetArgumentsPropByValInst(
      const HBCGetArgumentsPropByValInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

 public:
  enum { IndexIdx, LazyRegisterIdx };

  Value *getIndex() const {
    return getOperand(IndexIdx);
  }
  Value *getLazyRegister() const {
    return getOperand(LazyRegisterIdx);
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute().setReadStack().setWriteStack();
  }

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), HBCGetArgumentsPropByValInst);
  }
};

class HBCGetArgumentsPropByValLooseInst : public HBCGetArgumentsPropByValInst {
  HBCGetArgumentsPropByValLooseInst(const HBCGetArgumentsPropByValLooseInst &) =
      delete;
  void operator=(const HBCGetArgumentsPropByValLooseInst &) = delete;

 public:
  explicit HBCGetArgumentsPropByValLooseInst(Value *index, AllocStackInst *reg)
      : HBCGetArgumentsPropByValInst(
            ValueKind::HBCGetArgumentsPropByValLooseInstKind,
            index,
            reg) {}
  explicit HBCGetArgumentsPropByValLooseInst(
      const HBCGetArgumentsPropByValLooseInst *src,
      llvh::ArrayRef<Value *> operands)
      : HBCGetArgumentsPropByValInst(src, operands) {}

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::HBCGetArgumentsPropByValLooseInstKind;
  }
};

class HBCGetArgumentsPropByValStrictInst : public HBCGetArgumentsPropByValInst {
  HBCGetArgumentsPropByValStrictInst(
      const HBCGetArgumentsPropByValStrictInst &) = delete;
  void operator=(const HBCGetArgumentsPropByValStrictInst &) = delete;

 public:
  explicit HBCGetArgumentsPropByValStrictInst(Value *index, AllocStackInst *reg)
      : HBCGetArgumentsPropByValInst(
            ValueKind::HBCGetArgumentsPropByValStrictInstKind,
            index,
            reg) {}
  explicit HBCGetArgumentsPropByValStrictInst(
      const HBCGetArgumentsPropByValStrictInst *src,
      llvh::ArrayRef<Value *> operands)
      : HBCGetArgumentsPropByValInst(src, operands) {}

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::HBCGetArgumentsPropByValStrictInstKind;
  }
};

// Create a real array for `arguments` for when getting the length and elements
// by index isn't enough.
// This is the base class for the strict and loose variants defined below.
class HBCReifyArgumentsInst : public SingleOperandInst {
  HBCReifyArgumentsInst(const HBCReifyArgumentsInst &) = delete;
  void operator=(const HBCReifyArgumentsInst &) = delete;

 protected:
  explicit HBCReifyArgumentsInst(ValueKind kind, AllocStackInst *reg)
      : SingleOperandInst(kind, reg) {}
  explicit HBCReifyArgumentsInst(
      const HBCReifyArgumentsInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

 public:
  Value *getLazyRegister() const {
    return getSingleOperand();
  }

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    // This instruction could throw in theory, if there are too many arguments.
    // However, this is not a meaningful side-effect, since this instruction
    // should be freely reordered or deleted.
    return SideEffect{}.setReadStack().setWriteStack().setIdempotent();
  }

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), HBCReifyArgumentsInst);
  }
};

class HBCReifyArgumentsStrictInst : public HBCReifyArgumentsInst {
  HBCReifyArgumentsStrictInst(const HBCReifyArgumentsStrictInst &) = delete;
  void operator=(const HBCReifyArgumentsStrictInst &) = delete;

 public:
  explicit HBCReifyArgumentsStrictInst(AllocStackInst *reg)
      : HBCReifyArgumentsInst(ValueKind::HBCReifyArgumentsStrictInstKind, reg) {
  }
  explicit HBCReifyArgumentsStrictInst(
      const HBCReifyArgumentsStrictInst *src,
      llvh::ArrayRef<Value *> operands)
      : HBCReifyArgumentsInst(src, operands) {}

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::HBCReifyArgumentsStrictInstKind;
  }
};

class HBCReifyArgumentsLooseInst : public HBCReifyArgumentsInst {
  HBCReifyArgumentsLooseInst(const HBCReifyArgumentsLooseInst &) = delete;
  void operator=(const HBCReifyArgumentsLooseInst &) = delete;

 public:
  explicit HBCReifyArgumentsLooseInst(AllocStackInst *reg)
      : HBCReifyArgumentsInst(ValueKind::HBCReifyArgumentsLooseInstKind, reg) {}
  explicit HBCReifyArgumentsLooseInst(
      const HBCReifyArgumentsLooseInst *src,
      llvh::ArrayRef<Value *> operands)
      : HBCReifyArgumentsInst(src, operands) {}

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::HBCReifyArgumentsLooseInstKind;
  }
};

/// Create a 'this' object to be filled in by a constructor.
class CreateThisInst : public Instruction {
  CreateThisInst(const CreateThisInst &) = delete;
  void operator=(const CreateThisInst &) = delete;

 public:
  enum { PrototypeIdx, ClosureIdx };

  explicit CreateThisInst(Value *prototype, Value *closure)
      : Instruction(ValueKind::CreateThisInstKind) {
    pushOperand(prototype);
    pushOperand(closure);
  }
  explicit CreateThisInst(
      const CreateThisInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Value *getPrototype() const {
    return getOperand(PrototypeIdx);
  }
  Value *getClosure() const {
    return getOperand(ClosureIdx);
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::CreateThisInstKind;
  }
};

/// Choose between 'this' and the object returned by a constructor.
class GetConstructedObjectInst : public Instruction {
  GetConstructedObjectInst(const GetConstructedObjectInst &) = delete;
  void operator=(const GetConstructedObjectInst &) = delete;

 public:
  enum { ThisValueIdx, ConstructorReturnValueIdx };

  explicit GetConstructedObjectInst(
      CreateThisInst *thisValue,
      CallInst *constructorReturnValue)
      : Instruction(ValueKind::GetConstructedObjectInstKind) {
    pushOperand(thisValue);
    pushOperand(constructorReturnValue);
  }
  explicit GetConstructedObjectInst(
      const GetConstructedObjectInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Value *getThisValue() const {
    // While originally a CreateThisInst, it may have been replaced by a mov
    // or similar.
    return getOperand(ThisValueIdx);
  }
  Value *getConstructorReturnValue() const {
    return getOperand(ConstructorReturnValueIdx);
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setIdempotent();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::GetConstructedObjectInstKind;
  }
};

/// Creating a closure in HBC requires an explicit environment.
class HBCCreateFunctionInst : public BaseCreateCallableInst {
  HBCCreateFunctionInst(const HBCCreateFunctionInst &) = delete;
  void operator=(const HBCCreateFunctionInst &) = delete;

 public:
  enum { EnvIdx = CreateFunctionInst::LAST_IDX };

  explicit HBCCreateFunctionInst(Function *code, BaseScopeInst *env)
      : BaseCreateCallableInst(ValueKind::HBCCreateFunctionInstKind, code) {
    setType(*getInherentTypeImpl());
    pushOperand(env);
  }
  explicit HBCCreateFunctionInst(
      const HBCCreateFunctionInst *src,
      llvh::ArrayRef<Value *> operands)
      : BaseCreateCallableInst(src, operands) {}

  Value *getEnvironment() const {
    return getOperand(EnvIdx);
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::HBCCreateFunctionInstKind;
  }
};

/// Identical to a Mov, except it should never be eliminated.
/// Elimination will undo spilling and cause failures during bc gen.
class HBCSpillMovInst : public SingleOperandInst {
  HBCSpillMovInst(const HBCSpillMovInst &) = delete;
  void operator=(const HBCSpillMovInst &) = delete;

 public:
  explicit HBCSpillMovInst(Instruction *value)
      : SingleOperandInst(ValueKind::HBCSpillMovInstKind, value) {
    setType(value->getType());
  }
  explicit HBCSpillMovInst(
      const HBCSpillMovInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  Instruction *getValue() const {
    return cast<Instruction>(getSingleOperand());
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::HBCSpillMovInstKind;
  }
};

class CompareBranchInst : public TerminatorInst {
  CompareBranchInst(const CompareBranchInst &) = delete;
  void operator=(const CompareBranchInst &) = delete;

 public:
  enum { LeftHandSideIdx, RightHandSideIdx, TrueBlockIdx, FalseBlockIdx };

  Value *getLeftHandSide() const {
    return getOperand(LeftHandSideIdx);
  }
  Value *getRightHandSide() const {
    return getOperand(RightHandSideIdx);
  }
  BasicBlock *getTrueDest() const {
    return cast<BasicBlock>(getOperand(TrueBlockIdx));
  }
  BasicBlock *getFalseDest() const {
    return cast<BasicBlock>(getOperand(FalseBlockIdx));
  }

  /// \return the string representation of the operator.
  llvh::StringRef getOperatorStr() {
    return BinaryOperatorInst::opStringRepr[HERMES_IR_KIND_TO_OFFSET(
        CompareBranchInst, getKind())];
  }

  explicit CompareBranchInst(
      ValueKind kind,
      Value *left,
      Value *right,
      BasicBlock *trueBlock,
      BasicBlock *falseBlock)
      : TerminatorInst(kind) {
    assert(HERMES_IR_KIND_IN_CLASS(kind, CompareBranchInst));
    pushOperand(left);
    pushOperand(right);
    pushOperand(trueBlock);
    pushOperand(falseBlock);
  }
  explicit CompareBranchInst(
      const CompareBranchInst *src,
      llvh::ArrayRef<Value *> operands)
      : TerminatorInst(src, operands) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  /// Return the BinaryOperatorInst ValueKind corresponding to this comparison.
  ValueKind toBinaryOperatorValueKind() const {
    return HERMES_IR_OFFSET_TO_KIND(
        BinaryOperatorInst,
        HERMES_IR_KIND_TO_OFFSET(CompareBranchInst, getKind()));
  }

  /// Convert a BinaryOperatorInst ValueKind into a CompareBranchInst one.
  static ValueKind fromBinaryOperatorValueKind(ValueKind kind) {
    int ofs = HERMES_IR_KIND_TO_OFFSET(BinaryOperatorInst, kind);
    assert(
        ofs >= 0 && ofs < HERMES_IR_CLASS_LENGTH(CompareBranchInst) &&
        "Invalid CmpBr ValueKind");
    return HERMES_IR_OFFSET_TO_KIND(CompareBranchInst, ofs);
  }

  // static ValueKind kindFromBinaryOperatorValueKind

  SideEffect getSideEffectImpl() const {
    return BinaryOperatorInst::getBinarySideEffect(
        getLeftHandSide()->getType(),
        getRightHandSide()->getType(),
        toBinaryOperatorValueKind());
  }

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), CompareBranchInst);
  }

  unsigned getNumSuccessorsImpl() const {
    return 2;
  }
  BasicBlock *getSuccessorImpl(unsigned idx) const {
    if (idx == 0)
      return getTrueDest();
    if (idx == 1)
      return getFalseDest();
    llvm_unreachable("CompareBranchInst only have 2 successors!");
  }
  void setSuccessorImpl(unsigned idx, BasicBlock *B) {
    assert(idx <= 1 && "CompareBranchInst only have 2 successors!");
    setOperand(B, idx + TrueBlockIdx);
  }
};

/// Create a generator instance with an inner function. Note that the result is
/// not a callable.
class CreateGeneratorInst : public BaseCreateLexicalChildInst {
  CreateGeneratorInst(const CreateGeneratorInst &) = delete;
  void operator=(const CreateGeneratorInst &) = delete;

 public:
  explicit CreateGeneratorInst(GeneratorInnerFunction *genFunction)
      : BaseCreateLexicalChildInst(
            ValueKind::CreateGeneratorInstKind,
            genFunction) {
    setType(*getInherentTypeImpl());
  }
  explicit CreateGeneratorInst(
      const CreateGeneratorInst *src,
      llvh::ArrayRef<Value *> operands)
      : BaseCreateLexicalChildInst(src, operands) {}

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createObject();
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::CreateGeneratorInstKind;
  }
};

/// Creating a closure in HBC requires an explicit environment.
class HBCCreateGeneratorInst : public BaseCreateLexicalChildInst {
  HBCCreateGeneratorInst(const HBCCreateGeneratorInst &) = delete;
  void operator=(const HBCCreateGeneratorInst &) = delete;

 public:
  enum { EnvIdx = CreateGeneratorInst::LAST_IDX };

  explicit HBCCreateGeneratorInst(Function *code, BaseScopeInst *env)
      : BaseCreateLexicalChildInst(
            ValueKind::HBCCreateGeneratorInstKind,
            code) {
    pushOperand(env);
    setType(*getInherentTypeImpl());
  }
  explicit HBCCreateGeneratorInst(
      const HBCCreateGeneratorInst *src,
      llvh::ArrayRef<Value *> operands)
      : BaseCreateLexicalChildInst(src, operands) {}

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createObject();
  }

  Value *getEnvironment() const {
    return getOperand(EnvIdx);
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::HBCCreateGeneratorInstKind;
  }
};

class StartGeneratorInst : public Instruction {
  StartGeneratorInst(const StartGeneratorInst &) = delete;
  void operator=(const StartGeneratorInst &) = delete;

 public:
  explicit StartGeneratorInst()
      : Instruction(ValueKind::StartGeneratorInstKind) {}
  explicit StartGeneratorInst(
      const StartGeneratorInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createUnknown().setFirstInBlock();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::StartGeneratorInstKind;
  }
};

class ResumeGeneratorInst : public Instruction {
  ResumeGeneratorInst(const ResumeGeneratorInst &) = delete;
  void operator=(const ResumeGeneratorInst &) = delete;

 public:
  enum { IsReturnIdx };

  explicit ResumeGeneratorInst(AllocStackInst *isReturn)
      : Instruction(ValueKind::ResumeGeneratorInstKind) {
    pushOperand(isReturn);
  }
  explicit ResumeGeneratorInst(
      const ResumeGeneratorInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createUnknown().setWriteStack();
  }

  Value *getIsReturn() {
    return getOperand(IsReturnIdx);
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::ResumeGeneratorInstKind;
  }
};

class IteratorBeginInst : public Instruction {
  IteratorBeginInst(const IteratorBeginInst &) = delete;
  void operator=(const IteratorBeginInst &) = delete;

 public:
  enum { SourceOrNextIdx };

  explicit IteratorBeginInst(AllocStackInst *sourceOrNext)
      : Instruction(ValueKind::IteratorBeginInstKind) {
    pushOperand(sourceOrNext);
  }
  explicit IteratorBeginInst(
      const IteratorBeginInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute().setReadStack().setWriteStack();
  }

  Value *getSourceOrNext() const {
    return getOperand(SourceOrNextIdx);
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::IteratorBeginInstKind;
  }
};

class IteratorNextInst : public Instruction {
  IteratorNextInst(const IteratorNextInst &) = delete;
  void operator=(const IteratorNextInst &) = delete;

 public:
  enum { IteratorIdx, SourceOrNextIdx };

  explicit IteratorNextInst(AllocStackInst *iterator, Value *sourceOrNext)
      : Instruction(ValueKind::IteratorNextInstKind) {
    pushOperand(iterator);
    pushOperand(sourceOrNext);
  }
  explicit IteratorNextInst(
      const IteratorNextInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute().setReadStack().setWriteStack();
  }

  Value *getIterator() const {
    return getOperand(IteratorIdx);
  }
  Value *getSourceOrNext() const {
    return getOperand(SourceOrNextIdx);
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::IteratorNextInstKind;
  }
};

class IteratorCloseInst : public Instruction {
  IteratorCloseInst(const IteratorCloseInst &) = delete;
  void operator=(const IteratorCloseInst &) = delete;

 public:
  enum { IteratorIdx, IgnoreInnerExceptionIdx };

  using TargetList = llvh::SmallVector<Value *, 2>;

  explicit IteratorCloseInst(Value *iterator, LiteralBool *ignoreInnerException)
      : Instruction(ValueKind::IteratorCloseInstKind) {
    pushOperand(iterator);
    pushOperand(ignoreInnerException);
  }
  explicit IteratorCloseInst(
      const IteratorCloseInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute();
  }

  Value *getIterator() const {
    return getOperand(IteratorIdx);
  }

  bool getIgnoreInnerException() const {
    return cast<LiteralBool>(getOperand(IgnoreInnerExceptionIdx))->getValue();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::IteratorCloseInstKind;
  }
};

/// A bytecode version of llvm_unreachable, for use in stubs and similar.
class UnreachableInst : public TerminatorInst {
  UnreachableInst(const UnreachableInst &) = delete;
  void operator=(const UnreachableInst &) = delete;

 public:
  explicit UnreachableInst() : TerminatorInst(ValueKind::UnreachableInstKind) {}
  explicit UnreachableInst(
      const UnreachableInst *src,
      llvh::ArrayRef<Value *> operands)
      : TerminatorInst(src, operands) {}

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return false;
  }

  unsigned getNumSuccessorsImpl() const {
    return 0;
  }
  BasicBlock *getSuccessorImpl(unsigned idx) const {
    llvm_unreachable("unreachable has no successors");
  }
  void setSuccessorImpl(unsigned idx, BasicBlock *B) {
    llvm_unreachable("unreachable has no successors");
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createExecute();
  }

  static bool classof(const Value *V) {
    ValueKind kind = V->getKind();
    return kind == ValueKind::UnreachableInstKind;
  }
};

class PrLoadInst : public Instruction {
 public:
  enum { ObjectIdx, PropIndexIdx, PropNameIdx };

  explicit PrLoadInst(
      Value *object,
      LiteralNumber *propIndex,
      LiteralString *propName,
      Type checkedType)
      : Instruction(ValueKind::PrLoadInstKind) {
    setType(checkedType);
    pushOperand(object);
    pushOperand(propIndex);
    pushOperand(propName);
  }
  explicit PrLoadInst(const PrLoadInst *src, llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::PrLoadInstKind;
  }
  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return true;
  }
  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setReadHeap().setIdempotent();
  }

  Value *getObject() const {
    return getOperand(ObjectIdx);
  }
  size_t getPropIndex() const {
    return (size_t)llvh::cast<LiteralNumber>(getOperand(PropIndexIdx))
        ->getValue();
  }
  LiteralString *getPropName() const {
    return llvh::cast<LiteralString>(getOperand(PropNameIdx));
  }
};

class PrStoreInst : public Instruction {
 public:
  enum { StoredValueIdx, ObjectIdx, PropIndexIdx, PropNameIdx, NonPointerIdx };

  /// \param nonPointer set to true when we know that both the old and new
  ///     value are non-pointers.
  explicit PrStoreInst(
      Value *storedValue,
      Value *object,
      LiteralNumber *propIndex,
      LiteralString *propName,
      LiteralBool *nonPointer)
      : Instruction(ValueKind::PrStoreInstKind) {
    setType(Type::createNoType());
    pushOperand(storedValue);
    pushOperand(object);
    pushOperand(propIndex);
    pushOperand(propName);
    pushOperand(nonPointer);
  }
  explicit PrStoreInst(const PrStoreInst *src, llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::PrStoreInstKind;
  }
  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return true;
  }
  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setWriteHeap().setIdempotent();
  }

  Value *getStoredValue() const {
    return getOperand(StoredValueIdx);
  }
  Value *getObject() const {
    return getOperand(ObjectIdx);
  }
  size_t getPropIndex() const {
    return (size_t)llvh::cast<LiteralNumber>(getOperand(PropIndexIdx))
        ->getValue();
  }
  LiteralString *getPropName() const {
    return llvh::cast<LiteralString>(getOperand(PropNameIdx));
  }
  bool getNonPointer() const {
    return llvh::cast<LiteralBool>(getOperand(NonPointerIdx))->getValue();
  }
};

class FastArrayLoadInst : public Instruction {
 public:
  enum { ArrayIdx, IndexIdx };

  explicit FastArrayLoadInst(Value *array, Value *index, Type checkedType)
      : Instruction(ValueKind::FastArrayLoadInstKind) {
    setType(checkedType);
    pushOperand(array);
    pushOperand(index);
  }
  explicit FastArrayLoadInst(
      const FastArrayLoadInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::FastArrayLoadInstKind;
  }
  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return true;
  }
  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setReadHeap().setThrow();
  }

  Value *getArray() const {
    return getOperand(ArrayIdx);
  }
  Value *getIndex() const {
    return getOperand(IndexIdx);
  }
};

class FastArrayStoreInst : public Instruction {
 public:
  enum { StoredValueIdx, ArrayIdx, IndexIdx };

  explicit FastArrayStoreInst(Value *storedValue, Value *array, Value *index)
      : Instruction(ValueKind::FastArrayStoreInstKind) {
    setType(Type::createNoType());
    pushOperand(storedValue);
    pushOperand(array);
    pushOperand(index);
  }
  explicit FastArrayStoreInst(
      const FastArrayStoreInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::FastArrayStoreInstKind;
  }
  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return true;
  }
  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setWriteHeap().setThrow();
  }

  Value *getStoredValue() const {
    return getOperand(StoredValueIdx);
  }
  Value *getArray() const {
    return getOperand(ArrayIdx);
  }
  Value *getIndex() const {
    return getOperand(IndexIdx);
  }
};

class FastArrayPushInst : public Instruction {
 public:
  enum { PushedValueIdx, ArrayIdx };

  explicit FastArrayPushInst(Value *pushedValue, Value *array)
      : Instruction(ValueKind::FastArrayPushInstKind) {
    setType(Type::createNoType());
    pushOperand(pushedValue);
    pushOperand(array);
  }
  explicit FastArrayPushInst(
      const FastArrayPushInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::FastArrayPushInstKind;
  }
  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return true;
  }
  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setWriteHeap().setThrow();
  }

  Value *getArray() const {
    return getOperand(ArrayIdx);
  }
  Value *getPushedValue() const {
    return getOperand(PushedValueIdx);
  }
};

class FastArrayAppendInst : public Instruction {
 public:
  enum { OtherIdx, ArrayIdx };

  explicit FastArrayAppendInst(Value *other, Value *array)
      : Instruction(ValueKind::FastArrayAppendInstKind) {
    setType(Type::createNoType());
    pushOperand(other);
    pushOperand(array);
  }
  explicit FastArrayAppendInst(
      const FastArrayAppendInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::FastArrayAppendInstKind;
  }
  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return true;
  }
  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setReadHeap().setWriteHeap().setThrow();
  }

  Value *getArray() const {
    return getOperand(ArrayIdx);
  }
  Value *getOther() const {
    return getOperand(OtherIdx);
  }
};

class FastArrayLengthInst : public Instruction {
 public:
  enum { ArrayIdx };

  explicit FastArrayLengthInst(Value *array)
      : Instruction(ValueKind::FastArrayLengthInstKind) {
    setType(*getInherentTypeImpl());
    pushOperand(array);
  }
  explicit FastArrayLengthInst(
      const FastArrayLengthInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::FastArrayLengthInstKind;
  }
  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return true;
  }
  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createUint32();
  }
  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setReadHeap();
  }

  Value *getArray() const {
    return getOperand(ArrayIdx);
  }
};

class LoadParentInst : public Instruction {
  LoadParentInst(const LoadParentInst &) = delete;
  void operator=(const LoadParentInst &) = delete;

 public:
  enum { ObjectIdx };

  explicit LoadParentInst(Value *object)
      : Instruction(ValueKind::LoadParentInstKind) {
    setType(*getInherentTypeImpl());
    pushOperand(object);
  }
  explicit LoadParentInst(
      const LoadParentInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static llvh::Optional<Type> getInherentTypeImpl() {
    return Type::createObject();
  }

  Value *getObject() {
    return getOperand(ObjectIdx);
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return true;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setReadHeap().setIdempotent();
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::LoadParentInstKind;
  }
};

class StoreParentInst : public Instruction {
  StoreParentInst(const StoreParentInst &) = delete;
  void operator=(const StoreParentInst &) = delete;

 public:
  enum { StoredValueIdx, ObjectIdx };

  explicit StoreParentInst(Value *storedValue, Value *object)
      : Instruction(ValueKind::StoreParentInstKind) {
    setType(Type::createNoType());
    pushOperand(storedValue);
    pushOperand(object);
  }
  explicit StoreParentInst(
      const StoreParentInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Value *getStoredValue() {
    return getOperand(StoredValueIdx);
  }
  Value *getObject() {
    return getOperand(ObjectIdx);
  }

  static bool hasOutput() {
    return false;
  }
  static bool isTyped() {
    return true;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setWriteHeap().setIdempotent();
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::StoreParentInstKind;
  }
};

class FUnaryMathInst : public Instruction {
  FUnaryMathInst(const FUnaryMathInst &) = delete;
  void operator=(const FUnaryMathInst &) = delete;

 public:
  enum { ArgIdx };

  explicit FUnaryMathInst(ValueKind kind, Value *arg) : Instruction(kind) {
    assert(arg->getType().isNumberType() && "invalid input FUnaryMathInst");
    setType(Type::createNumber());
    pushOperand(arg);
  }
  explicit FUnaryMathInst(
      const FUnaryMathInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Value *getArg() {
    return getOperand(ArgIdx);
  }
  const Value *getArg() const {
    return getOperand(ArgIdx);
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return true;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setIdempotent();
  }

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), FUnaryMathInst);
  }
};

class FBinaryMathInst : public Instruction {
  FBinaryMathInst(const FBinaryMathInst &) = delete;
  void operator=(const FBinaryMathInst &) = delete;

 public:
  enum { LeftIdx, RightIdx };

  explicit FBinaryMathInst(ValueKind kind, Value *left, Value *right)
      : Instruction(kind) {
    assert(left->getType().isNumberType() && "invalid input FBinaryMathInst");
    assert(right->getType().isNumberType() && "invalid input FBinaryMathInst");
    setType(Type::createNumber());
    pushOperand(left);
    pushOperand(right);
  }
  explicit FBinaryMathInst(
      const FBinaryMathInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Value *getLeft() {
    return getOperand(LeftIdx);
  }
  const Value *getLeft() const {
    return getOperand(LeftIdx);
  }

  Value *getRight() {
    return getOperand(RightIdx);
  }
  const Value *getRight() const {
    return getOperand(RightIdx);
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return true;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setIdempotent();
  }

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), FBinaryMathInst);
  }
};

class FCompareInst : public Instruction {
  FCompareInst(const FCompareInst &) = delete;
  void operator=(const FCompareInst &) = delete;

 public:
  enum { LeftIdx, RightIdx };

  explicit FCompareInst(ValueKind kind, Value *left, Value *right)
      : Instruction(kind) {
    assert(left->getType().isNumberType() && "invalid input FCompareInst");
    assert(right->getType().isNumberType() && "invalid input FCompareInst");
    setType(Type::createBoolean());
    pushOperand(left);
    pushOperand(right);
  }
  explicit FCompareInst(
      const FCompareInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Value *getLeft() {
    return getOperand(LeftIdx);
  }
  const Value *getLeft() const {
    return getOperand(LeftIdx);
  }

  Value *getRight() {
    return getOperand(RightIdx);
  }
  const Value *getRight() const {
    return getOperand(RightIdx);
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return true;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setIdempotent();
  }

  static bool classof(const Value *V) {
    return HERMES_IR_KIND_IN_CLASS(V->getKind(), FCompareInst);
  }
};

class StringConcatInst : public Instruction {
  StringConcatInst(const StringConcatInst &) = delete;
  void operator=(const StringConcatInst &) = delete;

 public:
  explicit StringConcatInst(llvh::ArrayRef<Value *> operands)
      : Instruction(ValueKind::StringConcatInstKind) {
    setType(Type::createString());
    assert(!operands.empty() && "no strings to concatenate");
    for (Value *op : operands) {
      assert(op->getType().isStringType() && "invalid input StringConcatInst");
      pushOperand(op);
    }
  }
  explicit StringConcatInst(
      const StringConcatInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return true;
  }

  SideEffect getSideEffectImpl() const {
    return {};
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::StringConcatInstKind;
  }
};

class UnionNarrowTrustedInst : public SingleOperandInst {
  UnionNarrowTrustedInst(const UnionNarrowTrustedInst &) = delete;
  void operator=(const UnionNarrowTrustedInst &) = delete;

  /// The original result type of the instruction, when it was created.
  /// This is used to intersect with the type of the operand during type
  /// inference, and also to avoid corner cases where the type of the operand is
  /// narrowed so that the intersection is empty.
  /// Also see the comment in ThrowIfInst.
  Type savedResultType_;

 public:
  explicit UnionNarrowTrustedInst(Value *src, Type type)
      : SingleOperandInst(ValueKind::UnionNarrowTrustedInstKind, src),
        savedResultType_(type) {
    setType(type);
  }
  explicit UnionNarrowTrustedInst(
      const UnionNarrowTrustedInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands),
        savedResultType_(src->savedResultType_) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    // UnionNarrowTrusted participates in TypeInference because its type is
    // based on the input type when possible.
    return false;
  }

  bool acceptsEmptyTypeImpl() const {
    return true;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setIdempotent();
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::UnionNarrowTrustedInstKind;
  }

  /// \return the original result type that was set before TypeInference.
  Type getSavedResultType() const {
    return savedResultType_;
  }
};

class CheckedTypeCastInst : public Instruction {
  CheckedTypeCastInst(const CheckedTypeCastInst &) = delete;
  void operator=(const CheckedTypeCastInst &) = delete;

 public:
  enum { CheckedValueIdx, SpecifiedTypeIdx };

  explicit CheckedTypeCastInst(Value *src, LiteralIRType *type)
      : Instruction(ValueKind::CheckedTypeCastInstKind) {
    setType(type->getData());
    pushOperand(src);
    pushOperand(type);
  }
  explicit CheckedTypeCastInst(
      const CheckedTypeCastInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Value *getCheckedValue() const {
    return getOperand(CheckedValueIdx);
  }

  LiteralIRType *getSpecifiedType() const {
    return llvh::cast<LiteralIRType>(getOperand(SpecifiedTypeIdx));
  }

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setThrow();
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::CheckedTypeCastInstKind;
  }
};

class LIRDeadValueInst : public Instruction {
  LIRDeadValueInst(const LIRDeadValueInst &) = delete;
  void operator=(const LIRDeadValueInst &) = delete;

  /// This type is returned during type inference.
  Type savedResultType_;

 public:
  explicit LIRDeadValueInst(Type type)
      : Instruction(ValueKind::LIRDeadValueInstKind), savedResultType_(type) {
    setType(type);
  }
  explicit LIRDeadValueInst(
      const LIRDeadValueInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands), savedResultType_(src->savedResultType_) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return false;
  }

  SideEffect getSideEffectImpl() const {
    return SideEffect::createUnknown();
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::LIRDeadValueInstKind;
  }

  /// \return the original result type that was set.
  Type getSavedResultType() const {
    return savedResultType_;
  }
};

class NativeCallInst : public Instruction {
  NativeCallInst(const NativeCallInst &) = delete;
  void operator=(const NativeCallInst &) = delete;

 public:
  enum { CalleeIdx, SignatureIdx, FirstArgIdx };

  explicit NativeCallInst(
      Type type,
      Value *callee,
      LiteralNativeSignature *signature,
      llvh::ArrayRef<Value *> args)
      : Instruction(ValueKind::NativeCallInstKind) {
    setType(type);
    // FIXME: this should be added when types are propagated.
    // assert(callee->getType().isNumberType() && "callee must be a number");
    pushOperand(callee);
    pushOperand(signature);
    for (auto arg : args)
      pushOperand(arg);
  }
  explicit NativeCallInst(
      const NativeCallInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return true;
  }
  SideEffect getSideEffectImpl() const {
    return SideEffect::createUnknown();
  }
  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::NativeCallInstKind;
  }

  Value *getCallee() const {
    return getOperand(CalleeIdx);
  }
  LiteralNativeSignature *getSignature() const {
    return llvh::cast<LiteralNativeSignature>(getOperand(SignatureIdx));
  }
  unsigned getNumArgs() const {
    return getNumOperands() - FirstArgIdx;
  }
  Value *getArg(unsigned i) const {
    assert(i < getNumArgs() && "invalid arg index");
    return getOperand(FirstArgIdx + i);
  }
};

class GetNativeRuntimeInst : public Instruction {
  GetNativeRuntimeInst(const GetNativeRuntimeInst &) = delete;
  void operator=(const GetNativeRuntimeInst &) = delete;

 public:
  explicit GetNativeRuntimeInst()
      : Instruction(ValueKind::GetNativeRuntimeInstKind) {
    setType(Type::createNumber());
  }
  explicit GetNativeRuntimeInst(
      const GetNativeRuntimeInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  static bool hasOutput() {
    return true;
  }
  static bool isTyped() {
    return true;
  }
  SideEffect getSideEffectImpl() const {
    return SideEffect{}.setIdempotent();
  }
  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::GetNativeRuntimeInstKind;
  }
};

} // end namespace hermes

#endif
