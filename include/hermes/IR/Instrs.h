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
#include "hermes/Optimizer/Wasm/WasmIntrinsics.h"

#include "llvh/ADT/SmallVector.h"
#include "llvh/ADT/ilist_node.h"
#include "llvh/IR/CFG.h"
#include "llvh/IR/SymbolTableListTraits.h"

using llvh::ArrayRef;
using llvh::cast;

namespace hermes {

/// \returns true if the type \p T is side effect free in the context of
/// binary operations.
bool isSideEffectFree(Type T);

/// Base class for instructions that are used for scope creation (e.g.,
/// HBCCreateEnvironment, CreateScopeInst, etc). All these operands have the
/// descriptor for the scope they are creating as the last operand.
class ScopeCreationInst : public Instruction {
  ScopeCreationInst(const ScopeCreationInst &) = delete;
  void operator=(const ScopeCreationInst &) = delete;

  // Make pushOperand private to ensure derived classes only add operands via
  // the constructor.
  using Instruction::pushOperand;

 protected:
  enum { CreatedScopeIdx, FirstAvailableIdx };

  explicit ScopeCreationInst(ValueKind kind, ScopeDesc *scopeDesc)
      : Instruction(kind) {
    pushOperand(scopeDesc);
  }

  template <uint32_t which>
  void pushOperand(Value *value) {
    static_assert(
        which >= FirstAvailableIdx,
        "Use FirstAvailableIndex to offset the created ScopeDesc.");
    pushOperand(value);
  }

 public:
  explicit ScopeCreationInst(
      const ScopeCreationInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  ScopeDesc *getCreatedScopeDesc() const {
    return cast<ScopeDesc>(getOperand(CreatedScopeIdx));
  }

  SideEffectKind getSideEffect() {
    llvm_unreachable("ScopeCreationInst must be inherited.");
  }

  WordBitSet<> getChangedOperandsImpl() {
    llvm_unreachable("ScopeCreationInst must be inherited.");
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::ScopeCreationInstKind);
  }
};

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

  SideEffectKind getSideEffect() {
    llvm_unreachable("SingleOperandInst must be inherited.");
  }

  WordBitSet<> getChangedOperandsImpl() {
    llvm_unreachable("SingleOperandInst must be inherited.");
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::SingleOperandInstKind);
  }
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

  SideEffectKind getSideEffect() {
    llvm_unreachable("TerminatorInst must be inherited.");
  }

  WordBitSet<> getChangedOperandsImpl() {
    llvm_unreachable("TerminatorInst must be inherited.");
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::TerminatorInstKind);
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::BranchInstKind);
  }

  unsigned getNumSuccessors() const {
    return 1;
  }
  BasicBlock *getSuccessor(unsigned idx) const {
    assert(idx == 0 && "BranchInst only have 1 successor!");
    return getBranchDest();
  }
  void setSuccessor(unsigned idx, BasicBlock *B) {
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
    setType(Type::createString());
  }
  explicit AddEmptyStringInst(
      const AddEmptyStringInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::AddEmptyStringInstKind);
  }
};

class AsNumberInst : public SingleOperandInst {
  AsNumberInst(const AsNumberInst &) = delete;
  void operator=(const AsNumberInst &) = delete;

 public:
  explicit AsNumberInst(Value *value)
      : SingleOperandInst(ValueKind::AsNumberInstKind, value) {
    setType(Type::createNumber());
  }
  explicit AsNumberInst(
      const AsNumberInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::AsNumberInstKind);
  }
};

class AsNumericInst : public SingleOperandInst {
  AsNumericInst(const AsNumericInst &) = delete;
  void operator=(const AsNumericInst &) = delete;

 public:
  explicit AsNumericInst(Value *value)
      : SingleOperandInst(ValueKind::AsNumericInstKind, value) {
    setType(Type::createNumeric());
  }
  explicit AsNumericInst(
      const AsNumericInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::AsNumericInstKind);
  }
};

class AsInt32Inst : public SingleOperandInst {
  AsInt32Inst(const AsInt32Inst &) = delete;
  void operator=(const AsInt32Inst &) = delete;

 public:
  explicit AsInt32Inst(Value *value)
      : SingleOperandInst(ValueKind::AsInt32InstKind, value) {
    setType(Type::createInt32());
  }
  explicit AsInt32Inst(const AsInt32Inst *src, llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::AsInt32InstKind);
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::CondBranchInstKind);
  }

  unsigned getNumSuccessors() const {
    return 2;
  }
  BasicBlock *getSuccessor(unsigned idx) const {
    if (idx == 0)
      return getTrueDest();
    if (idx == 1)
      return getFalseDest();
    llvm_unreachable("CondBranchInst only have 2 successors!");
  }
  void setSuccessor(unsigned idx, BasicBlock *B) {
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::ReturnInstKind);
  }

  unsigned getNumSuccessors() const {
    return 0;
  }
  BasicBlock *getSuccessor(unsigned idx) const {
    llvm_unreachable("ReturnInst has no successor!");
  }
  void setSuccessor(unsigned idx, BasicBlock *B) {
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
      : AllocStackInst(cast<Label>(operands[0])->get()) {
    // NOTE: we are playing a little trick here since the Label is not heap
    // allocated.
  }

  Identifier getVariableName() const {
    return variableName.get();
  }

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::AllocStackInstKind);
  }
};

class LoadStackInst : public SingleOperandInst {
  LoadStackInst(const LoadStackInst &) = delete;
  void operator=(const LoadStackInst &) = delete;

 public:
  explicit LoadStackInst(AllocStackInst *alloc)
      : SingleOperandInst(ValueKind::LoadStackInstKind, alloc) {}
  explicit LoadStackInst(
      const LoadStackInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  AllocStackInst *getPtr() const {
    return cast<AllocStackInst>(getSingleOperand());
  }

  SideEffectKind getSideEffect() {
    return SideEffectKind::MayRead;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::LoadStackInstKind);
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
    setType(Type::createNoType());
    pushOperand(storedValue);
    pushOperand(ptr);
  }
  explicit StoreStackInst(
      const StoreStackInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::MayWrite;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::StoreStackInstKind);
  }
};

class LoadFrameInst : public Instruction {
  LoadFrameInst(const LoadFrameInst &) = delete;
  void operator=(const LoadFrameInst &) = delete;

 public:
  enum { LoadVariableIdx, EnvIdx };

  explicit LoadFrameInst(Variable *alloc, ScopeCreationInst *environment)
      : Instruction(ValueKind::LoadFrameInstKind) {
    pushOperand(alloc);
    pushOperand(environment);
  }

  explicit LoadFrameInst(
      const LoadFrameInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Variable *getLoadVariable() const {
    return cast<Variable>(getOperand(LoadVariableIdx));
  }

  ScopeCreationInst *getEnvironment() const {
    return cast<ScopeCreationInst>(getOperand(EnvIdx));
  }

  SideEffectKind getSideEffect() {
    return SideEffectKind::MayRead;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::LoadFrameInstKind);
  }
};

class StoreFrameInst : public Instruction {
  StoreFrameInst(const StoreFrameInst &) = delete;
  void operator=(const StoreFrameInst &) = delete;

 public:
  enum { StoredValueIdx, VariableIdx, EnvIdx };

  Value *getValue() const {
    return getOperand(StoredValueIdx);
  }
  Variable *getVariable() const {
    return cast<Variable>(getOperand(VariableIdx));
  }
  ScopeCreationInst *getEnvironment() const {
    return cast<ScopeCreationInst>(getOperand(EnvIdx));
  }

  explicit StoreFrameInst(
      Value *storedValue,
      Variable *ptr,
      ScopeCreationInst *environment)
      : Instruction(ValueKind::StoreFrameInstKind) {
    pushOperand(storedValue);
    pushOperand(ptr);
    pushOperand(environment);
  }
  explicit StoreFrameInst(
      const StoreFrameInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::MayWrite;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::StoreFrameInstKind);
  }
};

class CreateScopeInst : public ScopeCreationInst {
  CreateScopeInst(const CreateScopeInst &) = delete;
  void operator=(const CreateScopeInst &) = delete;

 public:
  explicit CreateScopeInst(ScopeDesc *scopeDesc)
      : ScopeCreationInst(ValueKind::CreateScopeInstKind, scopeDesc) {}

  explicit CreateScopeInst(
      const CreateScopeInst *src,
      llvh::ArrayRef<Value *> operands)
      : ScopeCreationInst(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::CreateScopeInstKind);
  }
};

class CreateFunctionInst : public Instruction {
  CreateFunctionInst(const CreateFunctionInst &) = delete;
  void operator=(const CreateFunctionInst &) = delete;

 protected:
  explicit CreateFunctionInst(
      ValueKind kind,
      Function *code,
      Value *environment)
      : Instruction(kind) {
    setType(Type::createClosure());
    pushOperand(code);
    // N.B.: All non-HBC CreateFunctionInst have a ScopeCreationInst as the
    // environment, but that is not necessarily true about the HBC variants; the
    // environment could be an HBCSpillMov.
    pushOperand(environment);
  }

 public:
  enum { FunctionCodeIdx, EnvIdx };

  explicit CreateFunctionInst(Function *code, ScopeCreationInst *environment)
      : CreateFunctionInst(
            ValueKind::CreateFunctionInstKind,
            code,
            environment) {}
  explicit CreateFunctionInst(
      const CreateFunctionInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Function *getFunctionCode() const {
    return cast<Function>(getOperand(FunctionCodeIdx));
  }

  Value *getEnvironment() const {
    return getOperand(EnvIdx);
  }

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::CreateFunctionInstKind);
  }
};

class CallInst : public Instruction {
  CallInst(const CallInst &) = delete;
  void operator=(const CallInst &) = delete;

  // Forces the code to use the appropriate getters instead of relying on
  // hard-coded offsets when accessing the arguments.
  using Instruction::getOperand;

 public:
  enum { CalleeIdx, ThisIdx };

  using ArgumentList = llvh::SmallVector<Value *, 2>;

  Value *getCallee() const {
    return getOperand(CalleeIdx);
  }
  /// Get argument 0, the value for 'this'.
  Value *getThis() const {
    return getOperand(ThisIdx);
  }
  /// Get argument by index. 'this' is argument 0.
  Value *getArgument(unsigned idx) {
    return getOperand(ThisIdx + idx);
  }
  /// Set the value \p v at index \p idx. 'this' is argument 0.
  void setArgument(Value *V, unsigned idx) {
    setOperand(V, ThisIdx + idx);
  }

  unsigned getNumArguments() const {
    return getNumOperands() - ThisIdx;
  }

  explicit CallInst(
      ValueKind kind,
      Value *callee,
      Value *thisValue,
      ArrayRef<Value *> args)
      : Instruction(kind) {
    pushOperand(callee);
    pushOperand(thisValue);
    for (const auto &arg : args) {
      pushOperand(arg);
    }
  }
  explicit CallInst(const CallInst *src, llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::CallInstKind);
  }
};

class ConstructInst : public CallInst {
  ConstructInst(const ConstructInst &) = delete;
  void operator=(const ConstructInst &) = delete;

 public:
  Value *getConstructor() const {
    return getCallee();
  }

  explicit ConstructInst(
      Value *constructor,
      LiteralUndefined *undefined,
      ArrayRef<Value *> args)
      : CallInst(ValueKind::ConstructInstKind, constructor, undefined, args) {
    setType(Type::createObject());
  }
  explicit ConstructInst(
      const ConstructInst *src,
      llvh::ArrayRef<Value *> operands)
      : CallInst(src, operands) {}

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::ConstructInstKind);
  }
};

/// Call a VM builtin with the specified number and undefined as the "this"
/// parameter.
class CallBuiltinInst : public CallInst {
  CallBuiltinInst(const CallBuiltinInst &) = delete;
  void operator=(const CallBuiltinInst &) = delete;

 public:
  explicit CallBuiltinInst(
      LiteralNumber *callee,
      LiteralUndefined *thisValue,
      ArrayRef<Value *> args)
      : CallInst(ValueKind::CallBuiltinInstKind, callee, thisValue, args) {
    assert(
        callee->getValue() == (int)callee->getValue() &&
        callee->getValue() < (double)BuiltinMethod::_count &&
        "invalid builtin call");
  }
  explicit CallBuiltinInst(
      const CallBuiltinInst *src,
      llvh::ArrayRef<Value *> operands)
      : CallInst(src, operands) {}

  BuiltinMethod::Enum getBuiltinIndex() const {
    return (BuiltinMethod::Enum)cast<LiteralNumber>(getCallee())->asInt32();
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::CallBuiltinInstKind);
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
        builtinIndex->asInt32() &&
        builtinIndex->getValue() < (double)BuiltinMethod::_count &&
        "invalid builtin call");
    pushOperand(builtinIndex);
    setType(Type::createClosure());
  }
  explicit GetBuiltinClosureInst(
      const GetBuiltinClosureInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  BuiltinMethod::Enum getBuiltinIndex() const {
    return static_cast<BuiltinMethod::Enum>(
        cast<LiteralNumber>(getOperand(BuiltinIndexIdx))->asInt32());
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::GetBuiltinClosureInstKind);
  }
};

#ifdef HERMES_RUN_WASM
/// Call an unsafe compiler intrinsic.
class CallIntrinsicInst : public Instruction {
  CallIntrinsicInst(const CallIntrinsicInst &) = delete;
  void operator=(const CallIntrinsicInst &) = delete;

 public:
  enum { IntrinsicIndexIdx, ArgIdx };
  explicit CallIntrinsicInst(
      LiteralNumber *intrinsicIndex,
      ArrayRef<Value *> args)
      : Instruction(ValueKind::CallIntrinsicInstKind) {
    assert(
        intrinsicIndex->getValue() < WasmIntrinsics::_count &&
        "invalid intrinsics call");
    pushOperand(intrinsicIndex);
    for (const auto &arg : args) {
      pushOperand(arg);
    }
  }
  explicit CallIntrinsicInst(
      const CallIntrinsicInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Value *getArgument(unsigned idx) {
    return getOperand(ArgIdx + idx);
  }

  unsigned getNumArguments() const {
    return getNumOperands() - 1;
  }

  WasmIntrinsics::Enum getIntrinsicsIndex() const {
    return (WasmIntrinsics::Enum)cast<LiteralNumber>(
               getOperand(IntrinsicIndexIdx))
        ->asUInt32();
  }

  SideEffectKind getSideEffect() {
    if (getIntrinsicsIndex() >= WasmIntrinsics::__uasm_store8)
      return SideEffectKind::MayWrite;
    if (getIntrinsicsIndex() >= WasmIntrinsics::__uasm_loadi8)
      return SideEffectKind::MayRead;
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::CallIntrinsicInstKind);
  }
};
#endif

class HBCCallNInst : public CallInst {
 public:
  /// The minimum number of args supported by a CallN instruction, including
  /// 'this'.
  static constexpr uint32_t kMinArgs = 1;

  /// The maximum (inclusive) number of args supported by a CallN instruction,
  /// including 'this'.
  static constexpr uint32_t kMaxArgs = 4;

  explicit HBCCallNInst(Value *callee, Value *thisValue, ArrayRef<Value *> args)
      : CallInst(ValueKind::HBCCallNInstKind, callee, thisValue, args) {
    // +1 for 'this'.
    assert(
        kMinArgs <= args.size() + 1 && args.size() + 1 <= kMaxArgs &&
        "Invalid arg count for HBCCallNInst");
  }

  explicit HBCCallNInst(
      const HBCCallNInst *src,
      llvh::ArrayRef<Value *> operands)
      : CallInst(src, operands) {}

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
    setType(Type::createObject());
  }
  explicit HBCGetGlobalObjectInst(
      const HBCGetGlobalObjectInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::HBCGetGlobalObjectInstKind);
  }
};

class StorePropertyInst : public Instruction {
  StorePropertyInst(const StorePropertyInst &) = delete;
  void operator=(const StorePropertyInst &) = delete;

 protected:
  explicit StorePropertyInst(
      ValueKind kind,
      Value *storedValue,
      Value *object,
      Value *property)
      : Instruction(kind) {
    setType(Type::createNoType());
    pushOperand(storedValue);
    pushOperand(object);
    pushOperand(property);
  }

 public:
  enum { StoredValueIdx, ObjectIdx, PropertyIdx };

  Value *getStoredValue() const {
    return getOperand(StoredValueIdx);
  }
  Value *getObject() const {
    return getOperand(ObjectIdx);
  };
  Value *getProperty() const {
    return getOperand(PropertyIdx);
  }

  explicit StorePropertyInst(Value *storedValue, Value *object, Value *property)
      : StorePropertyInst(
            ValueKind::StorePropertyInstKind,
            storedValue,
            object,
            property) {}
  explicit StorePropertyInst(
      const StorePropertyInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::StorePropertyInstKind);
  }
};

class TryStoreGlobalPropertyInst : public StorePropertyInst {
  TryStoreGlobalPropertyInst(const TryStoreGlobalPropertyInst &) = delete;
  void operator=(const TryStoreGlobalPropertyInst &) = delete;

 public:
  LiteralString *getProperty() const {
    return cast<LiteralString>(StorePropertyInst::getProperty());
  }

  explicit TryStoreGlobalPropertyInst(
      Value *storedValue,
      Value *globalObject,
      LiteralString *property)
      : StorePropertyInst(
            ValueKind::TryStoreGlobalPropertyInstKind,
            storedValue,
            globalObject,
            property) {
    assert(
        (llvh::isa<GlobalObject>(globalObject) ||
         llvh::isa<HBCGetGlobalObjectInst>(globalObject)) &&
        "globalObject must refer to the global object");
  }
  explicit TryStoreGlobalPropertyInst(
      const TryStoreGlobalPropertyInst *src,
      llvh::ArrayRef<Value *> operands)
      : StorePropertyInst(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::TryStoreGlobalPropertyInstKind);
  }
};

class StoreOwnPropertyInst : public Instruction {
  StoreOwnPropertyInst(const StoreOwnPropertyInst &) = delete;
  void operator=(const StoreOwnPropertyInst &) = delete;

 protected:
  explicit StoreOwnPropertyInst(
      ValueKind kind,
      Value *storedValue,
      Value *object,
      Value *property,
      LiteralBool *isEnumerable)
      : Instruction(kind) {
    setType(Type::createNoType());
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

  explicit StoreOwnPropertyInst(
      Value *storedValue,
      Value *object,
      Value *property,
      LiteralBool *isEnumerable)
      : StoreOwnPropertyInst(
            ValueKind::StoreOwnPropertyInstKind,
            storedValue,
            object,
            property,
            isEnumerable) {}

  explicit StoreOwnPropertyInst(
      const StoreOwnPropertyInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::StoreOwnPropertyInstKind);
  }
};

class StoreNewOwnPropertyInst : public StoreOwnPropertyInst {
  StoreNewOwnPropertyInst(const StoreNewOwnPropertyInst &) = delete;
  void operator=(const StoreNewOwnPropertyInst &) = delete;

 public:
  explicit StoreNewOwnPropertyInst(
      Value *storedValue,
      Value *object,
      Literal *property,
      LiteralBool *isEnumerable)
      : StoreOwnPropertyInst(
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
      : StoreOwnPropertyInst(src, operands) {}

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::StoreNewOwnPropertyInstKind);
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
    setType(Type::createNoType());
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::StoreGetterSetterInstKind);
  }
};

class DeletePropertyInst : public Instruction {
  DeletePropertyInst(const DeletePropertyInst &) = delete;
  void operator=(const DeletePropertyInst &) = delete;

 public:
  enum { ObjectIdx, PropertyIdx };

  Value *getObject() const {
    return getOperand(ObjectIdx);
  }
  Value *getProperty() const {
    return getOperand(PropertyIdx);
  }

  explicit DeletePropertyInst(Value *object, Value *property)
      : Instruction(ValueKind::DeletePropertyInstKind) {
    pushOperand(object);
    pushOperand(property);
  }
  explicit DeletePropertyInst(
      const DeletePropertyInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::DeletePropertyInstKind);
  }
};

class LoadPropertyInst : public Instruction {
  LoadPropertyInst(const LoadPropertyInst &) = delete;
  void operator=(const LoadPropertyInst &) = delete;

 protected:
  explicit LoadPropertyInst(ValueKind kind, Value *object, Value *property)
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

  explicit LoadPropertyInst(Value *object, Value *property)
      : LoadPropertyInst(ValueKind::LoadPropertyInstKind, object, property) {}
  explicit LoadPropertyInst(
      const LoadPropertyInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::LoadPropertyInstKind);
  }
};

class TryLoadGlobalPropertyInst : public LoadPropertyInst {
  TryLoadGlobalPropertyInst(const TryLoadGlobalPropertyInst &) = delete;
  void operator=(const TryLoadGlobalPropertyInst &) = delete;

 public:
  LiteralString *getProperty() const {
    return cast<LiteralString>(LoadPropertyInst::getProperty());
  }

  explicit TryLoadGlobalPropertyInst(
      Value *globalObject,
      LiteralString *property)
      : LoadPropertyInst(
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
      : LoadPropertyInst(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::TryLoadGlobalPropertyInstKind);
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
    setType(Type::createObject());
    assert(size->isUInt32Representible() && "size must be uint32");
    pushOperand(size);
    pushOperand(parentObject);
  }
  explicit AllocObjectInst(
      const AllocObjectInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::AllocObjectInstKind);
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
    setType(Type::createObject());
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
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
    return kindIsA(V->getKind(), ValueKind::HBCAllocObjectFromBufferInstKind);
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
    setType(Type::createObject());
    for (size_t i = 0; i < propMap.size(); i++) {
      pushOperand(propMap[i].first);
      pushOperand(propMap[i].second);
    }
  }

  explicit AllocObjectLiteralInst(
      const AllocObjectLiteralInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  unsigned getKeyValuePairCount() const {
    return getNumOperands() / 2;
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::AllocObjectLiteralInstKind);
  }

  Literal *getKey(unsigned index) const {
    return cast<Literal>(getOperand(2 * index));
  }

  Value *getValue(unsigned index) const {
    return getOperand(2 * index + 1);
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
    setType(Type::createObject());
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::AllocArrayInstKind);
  }
};

class CreateArgumentsInst : public Instruction {
  CreateArgumentsInst(const CreateArgumentsInst &) = delete;
  void operator=(const CreateArgumentsInst &) = delete;

 public:
  explicit CreateArgumentsInst()
      : Instruction(ValueKind::CreateArgumentsInstKind) {
    setType(Type::createObject());
  }
  explicit CreateArgumentsInst(
      const CreateArgumentsInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::CreateArgumentsInstKind);
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
    setType(Type::createRegExp());
    pushOperand(pattern);
    pushOperand(flags);
  }
  explicit CreateRegExpInst(
      const CreateRegExpInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::CreateRegExpInstKind);
  }
};

class UnaryOperatorInst : public SingleOperandInst {
 public:
  /// JavaScript Binary operators as defined in the ECMA spec.
  /// http://www.ecma-international.org/ecma-262/7.0/index.html#sec-unary-operators
  enum class OpKind {
    DeleteKind, // delete
    VoidKind, // void
    TypeofKind, // typeof
    PlusKind, // +
    MinusKind, // -
    TildeKind, // ~
    BangKind, // !
    IncKind, // + 1
    DecKind, // - 1
    LAST_OPCODE
  };

 private:
  UnaryOperatorInst(const UnaryOperatorInst &) = delete;
  void operator=(const UnaryOperatorInst &) = delete;

  /// The operator kind.
  OpKind op_;

  // A list of textual representation of the operators above.
  static const char *opStringRepr[(int)OpKind::LAST_OPCODE];

 public:
  /// \return the binary operator kind.
  OpKind getOperatorKind() const {
    return op_;
  }

  // Convert the operator string \p into the enum representation or assert
  // fail if the string is invalud.
  static OpKind parseOperator(llvh::StringRef op);

  /// \return the string representation of the operator.
  llvh::StringRef getOperatorStr() {
    return opStringRepr[static_cast<int>(op_)];
  }

  explicit UnaryOperatorInst(Value *value, OpKind opKind)
      : SingleOperandInst(ValueKind::UnaryOperatorInstKind, value),
        op_(opKind) {}
  explicit UnaryOperatorInst(
      const UnaryOperatorInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands), op_(src->op_) {}

  SideEffectKind getSideEffect();

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::UnaryOperatorInstKind);
  }
};

class BinaryOperatorInst : public Instruction {
 public:
  /// JavaScript Binary operators as defined in the ECMA spec.
  /// http://ecma-international.org/ecma-262/5.1/#sec-11
  enum class OpKind {
    IdentityKind, // nop (assignment operator, no arithmetic op)
    EqualKind, // ==
    NotEqualKind, // !=
    StrictlyEqualKind, // ===
    StrictlyNotEqualKind, // !==
    LessThanKind, // <
    LessThanOrEqualKind, // <=
    GreaterThanKind, // >
    GreaterThanOrEqualKind, // >=
    LeftShiftKind, // <<  (<<=)
    RightShiftKind, // >>  (>>=)
    UnsignedRightShiftKind, // >>> (>>>=)
    AddKind, // +   (+=)
    SubtractKind, // -   (-=)
    MultiplyKind, // *   (*=)
    DivideKind, // /   (/=)
    ModuloKind, // %   (%=)
    OrKind, // |   (|=)
    XorKind, // ^   (^=)
    AndKind, // &   (^=)
    ExponentiationKind, // ** (**=)
    AssignShortCircuitOrKind, // ||= (only for assignment)
    AssignShortCircuitAndKind, // &&= (only for assignment)
    AssignNullishCoalesceKind, // ??= (only for assignment)
    InKind, // "in"
    InstanceOfKind, // instanceof
    LAST_OPCODE
  };

  // A list of textual representation of the operators above.
  static const char *opStringRepr[(int)OpKind::LAST_OPCODE];

  // A list of textual representation of the assignment operators that match the
  // operators above.
  static const char *assignmentOpStringRepr[(int)OpKind::LAST_OPCODE];

 private:
  BinaryOperatorInst(const BinaryOperatorInst &) = delete;
  void operator=(const BinaryOperatorInst &) = delete;

  /// The operator kind.
  OpKind op_;

 public:
  enum { LeftHandSideIdx, RightHandSideIdx };

  /// \return the binary operator kind.
  OpKind getOperatorKind() const {
    return op_;
  }

  Value *getLeftHandSide() const {
    return getOperand(LeftHandSideIdx);
  }
  Value *getRightHandSide() const {
    return getOperand(RightHandSideIdx);
  }

  // Convert the operator string \p into the enum representation or assert
  // fail if the string is invalud.
  static OpKind parseOperator(llvh::StringRef op);

  // Convert the assignment operator string \p into the enum representation or
  // assert fail if the string is invalud.
  static OpKind parseAssignmentOperator(llvh::StringRef op);

  // Get the operator that allows you to swap the operands, if one exists.
  // >= becomes <= and + becomes +.
  static llvh::Optional<OpKind> tryGetReverseOperator(OpKind op);

  /// \return the string representation of the operator.
  llvh::StringRef getOperatorStr() {
    return opStringRepr[static_cast<int>(op_)];
  }

  explicit BinaryOperatorInst(Value *left, Value *right, OpKind opKind)
      : Instruction(ValueKind::BinaryOperatorInstKind), op_(opKind) {
    pushOperand(left);
    pushOperand(right);
  }
  explicit BinaryOperatorInst(
      const BinaryOperatorInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands), op_(src->op_) {}

  SideEffectKind getSideEffect() {
    return getBinarySideEffect(
        getLeftHandSide()->getType(),
        getRightHandSide()->getType(),
        getOperatorKind());
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::BinaryOperatorInstKind);
  }

  /// Calculate the side effect of a binary operator, given inferred types of
  /// its arguments.
  static SideEffectKind
  getBinarySideEffect(Type leftTy, Type rightTy, OpKind op);
};

class CatchInst : public Instruction {
  CatchInst(const CatchInst &) = delete;
  void operator=(const CatchInst &) = delete;

 public:
  explicit CatchInst() : Instruction(ValueKind::CatchInstKind) {}
  explicit CatchInst(const CatchInst *src, llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::CatchInstKind);
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  Value *getThrownValue() const {
    return getOperand(ThrownValueIdx);
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::ThrowInstKind);
  }

  unsigned getNumSuccessors() const {
    return 0;
  }
  BasicBlock *getSuccessor(unsigned idx) const {
    llvm_unreachable("ThrowInst has no successor!");
  }
  void setSuccessor(unsigned idx, BasicBlock *B) {
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::SwitchInstKind);
  }

  unsigned getNumSuccessors() const {
    return getNumCasePair() + 1;
  }
  BasicBlock *getSuccessor(unsigned idx) const;
  void setSuccessor(unsigned idx, BasicBlock *B);
};

class GetPNamesInst : public TerminatorInst {
  GetPNamesInst(const GetPNamesInst &) = delete;
  void operator=(const GetPNamesInst &) = delete;

 public:
  enum { IteratorIdx, BaseIdx, IndexIdx, SizeIdx, OnEmptyIdx, OnSomeIdx };

  explicit GetPNamesInst(
      BasicBlock *parent,
      Value *iteratorAddr,
      Value *baseAddr,
      Value *indexAddr,
      Value *sizeAddr,
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::MayWrite;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return WordBitSet<>{}
        .set(IteratorIdx)
        .set(BaseIdx)
        .set(IndexIdx)
        .set(SizeIdx);
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::GetPNamesInstKind);
  }

  unsigned getNumSuccessors() const {
    return 2;
  }
  BasicBlock *getSuccessor(unsigned idx) const {
    if (idx == 0)
      return getOnEmptyDest();
    if (idx == 1)
      return getOnSomeDest();
    llvm_unreachable("GetPNamesInst only have 2 successors!");
  }
  void setSuccessor(unsigned idx, BasicBlock *B) {
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
      Value *propertyAddr,
      Value *baseAddr,
      Value *indexAddr,
      Value *sizeAddr,
      Value *iteratorAddr,
      BasicBlock *onLast,
      BasicBlock *onSome);
  explicit GetNextPNameInst(
      const GetNextPNameInst *src,
      llvh::ArrayRef<Value *> operands)
      : TerminatorInst(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::MayWrite;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return WordBitSet<>{}
        .set(IteratorIdx)
        .set(BaseIdx)
        .set(IndexIdx)
        .set(SizeIdx)
        .set(PropertyIdx);
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::GetNextPNameInstKind);
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

  unsigned getNumSuccessors() const {
    return 2;
  }
  BasicBlock *getSuccessor(unsigned idx) const {
    if (idx == 0)
      return getOnLastDest();
    if (idx == 1)
      return getOnSomeDest();
    llvm_unreachable("GetNextPNameInst only have 2 successors!");
  }
  void setSuccessor(unsigned idx, BasicBlock *B) {
    if (idx == 0)
      return setOperand(B, OnLastIdx);
    if (idx == 1)
      return setOperand(B, OnSomeIdx);
    llvm_unreachable("GetNextPNameInst only have 2 successors!");
  }
};

class CheckHasInstanceInst : public TerminatorInst {
  CheckHasInstanceInst(const CheckHasInstanceInst &) = delete;
  void operator=(const CheckHasInstanceInst &) = delete;

 public:
  enum { ResultIdx, LeftIdx, RightIdx, OnTrueIdx, OnFalseIdx };

  explicit CheckHasInstanceInst(
      AllocStackInst *result,
      Value *left,
      Value *right,
      BasicBlock *onTrue,
      BasicBlock *onFalse)
      : TerminatorInst(ValueKind::CheckHasInstanceInstKind) {
    pushOperand(result);
    pushOperand(left);
    pushOperand(right);
    pushOperand(onTrue);
    pushOperand(onFalse);
  }
  explicit CheckHasInstanceInst(
      const CheckHasInstanceInst *src,
      llvh::ArrayRef<Value *> operands)
      : TerminatorInst(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::CheckHasInstanceInstKind);
  }

  AllocStackInst *getResult() const {
    return cast<AllocStackInst>(getOperand(ResultIdx));
  }
  Value *getLeft() const {
    return getOperand(LeftIdx);
  }
  Value *getRight() const {
    return getOperand(RightIdx);
  }
  BasicBlock *getOnTrueDest() const {
    return cast<BasicBlock>(getOperand(OnTrueIdx));
  }
  BasicBlock *getOnFalseDest() const {
    return cast<BasicBlock>(getOperand(OnFalseIdx));
  }

  unsigned getNumSuccessors() const {
    return 2;
  }
  BasicBlock *getSuccessor(unsigned idx) const {
    if (idx == 0)
      return getOnTrueDest();
    if (idx == 1)
      return getOnFalseDest();
    llvm_unreachable("CheckHasInstanceInst only have 2 successors!");
  }
  void setSuccessor(unsigned idx, BasicBlock *B) {
    if (idx == 0)
      return setOperand(B, OnTrueIdx);
    if (idx == 1)
      return setOperand(B, OnFalseIdx);
    llvm_unreachable("CheckHasInstanceInst only have 2 successors!");
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::TryStartInstKind);
  }

  unsigned getNumSuccessors() const {
    return 2;
  }
  BasicBlock *getSuccessor(unsigned idx) const {
    return cast<BasicBlock>(getOperand(idx));
  }
  void setSuccessor(unsigned idx, BasicBlock *B) {
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::TryEndInstKind);
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::PhiInstKind);
  }
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::MovInstKind);
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::ImplicitMovInstKind);
  }
};

class CoerceThisNSInst : public SingleOperandInst {
  CoerceThisNSInst(const MovInst &) = delete;
  void operator=(const CoerceThisNSInst &) = delete;

 public:
  explicit CoerceThisNSInst(Value *input)
      : SingleOperandInst(ValueKind::CoerceThisNSInstKind, input) {
    setType(input->getType());
  }
  explicit CoerceThisNSInst(
      const CoerceThisNSInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::CoerceThisNSInstKind);
  }
};

class DebuggerInst : public Instruction {
  DebuggerInst(const DebuggerInst &) = delete;
  void operator=(const DebuggerInst &) = delete;

 public:
  explicit DebuggerInst() : Instruction(ValueKind::DebuggerInstKind) {
    setType(Type::createNoType());
  }
  explicit DebuggerInst(
      const DebuggerInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::DebuggerInstKind);
  }
};

class GetNewTargetInst : public Instruction {
  GetNewTargetInst(const GetNewTargetInst &) = delete;
  void operator=(const GetNewTargetInst &) = delete;

 public:
  explicit GetNewTargetInst() : Instruction(ValueKind::GetNewTargetInstKind) {}
  explicit GetNewTargetInst(
      const GetNewTargetInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::GetNewTargetInstKind);
  }
};

/// Throw if the operand is "empty", otherwise return it.
class ThrowIfEmptyInst : public Instruction {
  ThrowIfEmptyInst(const ThrowIfEmptyInst &) = delete;
  void operator=(const ThrowIfEmptyInst &) = delete;

 public:
  enum { CheckedValueIdx };

  explicit ThrowIfEmptyInst(Value *checkedValue)
      : Instruction(ValueKind::ThrowIfEmptyInstKind) {
    pushOperand(checkedValue);
  }
  explicit ThrowIfEmptyInst(
      const ThrowIfEmptyInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Value *getCheckedValue() {
    return getOperand(CheckedValueIdx);
  }

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::ThrowIfEmptyInstKind);
  }
};

class HBCResolveEnvironment : public ScopeCreationInst {
  HBCResolveEnvironment(const HBCResolveEnvironment &) = delete;
  void operator=(const HBCResolveEnvironment &) = delete;

 public:
  enum { OriginScopeDescIdx = FirstAvailableIdx };
  explicit HBCResolveEnvironment(
      ScopeDesc *srcScopeDesc,
      ScopeDesc *targetScopeDesc)
      : ScopeCreationInst(
            ValueKind::HBCResolveEnvironmentKind,
            targetScopeDesc) {
    pushOperand<OriginScopeDescIdx>(srcScopeDesc);
  }
  explicit HBCResolveEnvironment(
      const HBCResolveEnvironment *src,
      llvh::ArrayRef<Value *> operands)
      : ScopeCreationInst(src, operands) {}

  ScopeDesc *getOriginScopeDesc() const {
    return cast<ScopeDesc>(getOperand(OriginScopeDescIdx));
  }

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::HBCResolveEnvironmentKind);
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::MayWrite;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::HBCStoreToEnvironmentInstKind);
  }
};

class HBCLoadFromEnvironmentInst : public Instruction {
  HBCLoadFromEnvironmentInst(const HBCLoadFromEnvironmentInst &) = delete;
  void operator=(const HBCLoadFromEnvironmentInst &) = delete;

 public:
  enum { EnvIdx, NameIdx };

  explicit HBCLoadFromEnvironmentInst(Value *env, Variable *var)
      : Instruction(ValueKind::HBCLoadFromEnvironmentInstKind) {
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::MayRead;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::HBCLoadFromEnvironmentInstKind);
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::SwitchImmInstKind);
  }

  unsigned getNumSuccessors() const {
    return getNumCasePair() + 1;
  }
  BasicBlock *getSuccessor(unsigned idx) const;
  void setSuccessor(unsigned idx, BasicBlock *B);
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  unsigned getNumSuccessors() const {
    return 1;
  }

  BasicBlock *getSuccessor(unsigned idx) const {
    assert(idx == 0 && "SaveAndYieldInst should only have 1 successor");
    return getNextBlock();
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::SaveAndYieldInstKind);
  }
};

class DirectEvalInst : public SingleOperandInst {
  DirectEvalInst(const DirectEvalInst &) = delete;
  void operator=(const DirectEvalInst &) = delete;

 public:
  explicit DirectEvalInst(Value *value)
      : SingleOperandInst(ValueKind::DirectEvalInstKind, value) {}
  explicit DirectEvalInst(
      const DirectEvalInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  SideEffectKind getSideEffect() const {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::DirectEvalInstKind);
  }
};

class HBCCreateEnvironmentInst : public ScopeCreationInst {
  HBCCreateEnvironmentInst(const HBCCreateEnvironmentInst &) = delete;
  void operator=(const HBCCreateEnvironmentInst &) = delete;

 public:
  explicit HBCCreateEnvironmentInst(ScopeDesc *scopeDesc)
      : ScopeCreationInst(ValueKind::HBCCreateEnvironmentInstKind, scopeDesc) {}
  explicit HBCCreateEnvironmentInst(
      const HBCCreateEnvironmentInst *src,
      llvh::ArrayRef<Value *> operands)
      : ScopeCreationInst(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::HBCCreateEnvironmentInstKind);
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::HBCProfilePointInstKind);
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::HBCLoadConstInstKind);
  }
};

/// Load a formal parameter. Parameter 0 is "this", the rest of the params start
/// at index 1.
class HBCLoadParamInst : public SingleOperandInst {
  HBCLoadParamInst(const HBCLoadParamInst &) = delete;
  void operator=(const HBCLoadParamInst &) = delete;

 public:
  explicit HBCLoadParamInst(LiteralNumber *input)
      : SingleOperandInst(ValueKind::HBCLoadParamInstKind, input) {}
  explicit HBCLoadParamInst(
      const HBCLoadParamInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  LiteralNumber *getIndex() const {
    return cast<LiteralNumber>(getSingleOperand());
  }

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::HBCLoadParamInstKind);
  }
};

/// Load the "this" parameter in non-strict mode, which coerces it to an object,
/// boxing primitives and converting "null" and "undefined" to the global
/// object.
class HBCGetThisNSInst : public Instruction {
  HBCGetThisNSInst(const HBCGetThisNSInst &) = delete;
  void operator=(const HBCGetThisNSInst &) = delete;

 public:
  explicit HBCGetThisNSInst() : Instruction(ValueKind::HBCGetThisNSInstKind) {}
  explicit HBCGetThisNSInst(
      const HBCGetThisNSInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::HBCGetThisNSInstKind);
  }
};

// Get `arguments.length`, without having to create a real array.
class HBCGetArgumentsLengthInst : public SingleOperandInst {
  HBCGetArgumentsLengthInst(const HBCGetArgumentsLengthInst &) = delete;
  void operator=(const HBCGetArgumentsLengthInst &) = delete;

 public:
  explicit HBCGetArgumentsLengthInst(AllocStackInst *reg)
      : SingleOperandInst(ValueKind::HBCGetArgumentsLengthInstKind, reg) {}
  explicit HBCGetArgumentsLengthInst(
      const HBCGetArgumentsLengthInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  Value *getLazyRegister() const {
    return getSingleOperand();
  }

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::HBCGetArgumentsLengthInstKind);
  }
};

// Get `arguments[i]` without having to create a real array.
class HBCGetArgumentsPropByValInst : public Instruction {
  HBCGetArgumentsPropByValInst(const HBCGetArgumentsPropByValInst &) = delete;
  void operator=(const HBCGetArgumentsPropByValInst &) = delete;

 public:
  enum { IndexIdx, LazyRegisterIdx };

  explicit HBCGetArgumentsPropByValInst(Value *index, AllocStackInst *reg)
      : Instruction(ValueKind::HBCGetArgumentsPropByValInstKind) {
    pushOperand(index);
    pushOperand(reg);
  }
  explicit HBCGetArgumentsPropByValInst(
      const HBCGetArgumentsPropByValInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Value *getIndex() const {
    return getOperand(IndexIdx);
  }
  Value *getLazyRegister() const {
    return getOperand(LazyRegisterIdx);
  }

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::HBCGetArgumentsPropByValInstKind);
  }
};

// Create a real array for `arguments` for when getting the length and elements
// by index isn't enough.
class HBCReifyArgumentsInst : public SingleOperandInst {
  HBCReifyArgumentsInst(const HBCReifyArgumentsInst &) = delete;
  void operator=(const HBCReifyArgumentsInst &) = delete;

 public:
  explicit HBCReifyArgumentsInst(AllocStackInst *reg)
      : SingleOperandInst(ValueKind::HBCReifyArgumentsInstKind, reg) {
    setType(Type::createNoType());
  }
  explicit HBCReifyArgumentsInst(
      const HBCReifyArgumentsInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  Value *getLazyRegister() const {
    return getSingleOperand();
  }

  SideEffectKind getSideEffect() {
    return SideEffectKind::MayWrite;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return WordBitSet<>{}.set(0);
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::HBCReifyArgumentsInstKind);
  }
};

/// Create a 'this' object to be filled in by a constructor.
class HBCCreateThisInst : public Instruction {
  HBCCreateThisInst(const HBCCreateThisInst &) = delete;
  void operator=(const HBCCreateThisInst &) = delete;

 public:
  enum { PrototypeIdx, ClosureIdx };

  explicit HBCCreateThisInst(Value *prototype, Value *closure)
      : Instruction(ValueKind::HBCCreateThisInstKind) {
    pushOperand(prototype);
    pushOperand(closure);
  }
  explicit HBCCreateThisInst(
      const HBCCreateThisInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Value *getPrototype() const {
    return getOperand(PrototypeIdx);
  }
  Value *getClosure() const {
    return getOperand(ClosureIdx);
  }

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::HBCCreateThisInstKind);
  }
};

/// Call a constructor. thisValue can be created with HBCCreateThisInst.
class HBCConstructInst : public CallInst {
  HBCConstructInst(const HBCConstructInst &) = delete;
  void operator=(const HBCConstructInst &) = delete;

 public:
  explicit HBCConstructInst(
      Value *callee,
      Value *thisValue,
      ArrayRef<Value *> args)
      : CallInst(ValueKind::HBCConstructInstKind, callee, thisValue, args) {}
  explicit HBCConstructInst(
      const HBCConstructInst *src,
      llvh::ArrayRef<Value *> operands)
      : CallInst(src, operands) {}

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::HBCConstructInstKind);
  }
};

/// Choose between 'this' and the object returned by a constructor.
class HBCGetConstructedObjectInst : public Instruction {
  HBCGetConstructedObjectInst(const HBCGetConstructedObjectInst &) = delete;
  void operator=(const HBCGetConstructedObjectInst &) = delete;

 public:
  enum { ThisValueIdx, ConstructorReturnValueIdx };

  explicit HBCGetConstructedObjectInst(
      HBCCreateThisInst *thisValue,
      HBCConstructInst *constructorReturnValue)
      : Instruction(ValueKind::HBCGetConstructedObjectInstKind) {
    pushOperand(thisValue);
    pushOperand(constructorReturnValue);
  }
  explicit HBCGetConstructedObjectInst(
      const HBCGetConstructedObjectInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  Value *getThisValue() const {
    // While originally a HBCCreateThisInst, it may have been replaced by a mov
    // or similar.
    return getOperand(ThisValueIdx);
  }
  Value *getConstructorReturnValue() const {
    return getOperand(ConstructorReturnValueIdx);
  }

  SideEffectKind getSideEffect() {
    return SideEffectKind::MayRead;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::HBCGetConstructedObjectInstKind);
  }
};

class HBCCallDirectInst : public CallInst {
  HBCCallDirectInst(const HBCCallDirectInst &) = delete;
  void operator=(const HBCCallDirectInst &) = delete;

 public:
  static constexpr unsigned MAX_ARGUMENTS = 255;

  explicit HBCCallDirectInst(
      Function *callee,
      Value *thisValue,
      ArrayRef<Value *> args)
      : CallInst(ValueKind::HBCCallDirectInstKind, callee, thisValue, args) {
    assert(
        getNumArguments() <= MAX_ARGUMENTS &&
        "Too many arguments to HBCCallDirect");
  }
  explicit HBCCallDirectInst(
      const HBCCallDirectInst *src,
      llvh::ArrayRef<Value *> operands)
      : CallInst(src, operands) {}

  Function *getFunctionCode() const {
    return cast<Function>(getCallee());
  }

  static bool classof(const Value *V) {
    return V->getKind() == ValueKind::HBCCallDirectInstKind;
  }
};

/// Creating a closure in HBC requires an explicit environment.
class HBCCreateFunctionInst : public CreateFunctionInst {
  HBCCreateFunctionInst(const HBCCreateFunctionInst &) = delete;
  void operator=(const HBCCreateFunctionInst &) = delete;

 public:
  explicit HBCCreateFunctionInst(Function *code, Value *environment)
      : CreateFunctionInst(
            ValueKind::HBCCreateFunctionInstKind,
            code,
            environment) {}
  explicit HBCCreateFunctionInst(
      const HBCCreateFunctionInst *src,
      llvh::ArrayRef<Value *> operands)
      : CreateFunctionInst(src, operands) {}

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::HBCCreateFunctionInstKind);
  }
};

/// Identical to a Mov, except it should never be eliminated.
/// Elimination will undo spilling and cause failures during bc gen.
class HBCSpillMovInst : public SingleOperandInst {
  HBCSpillMovInst(const HBCSpillMovInst &) = delete;
  void operator=(const HBCSpillMovInst &) = delete;

 public:
  explicit HBCSpillMovInst(Instruction *value)
      : SingleOperandInst(ValueKind::HBCSpillMovInstKind, value) {}
  explicit HBCSpillMovInst(
      const HBCSpillMovInst *src,
      llvh::ArrayRef<Value *> operands)
      : SingleOperandInst(src, operands) {}

  Instruction *getValue() const {
    return cast<Instruction>(getSingleOperand());
  }

  SideEffectKind getSideEffect() {
    return SideEffectKind::None;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::HBCSpillMovInstKind);
  }
};

class CompareBranchInst : public TerminatorInst {
  CompareBranchInst(const CompareBranchInst &) = delete;
  void operator=(const CompareBranchInst &) = delete;

  /// The operator kind.
  BinaryOperatorInst::OpKind op_;

 public:
  enum { LeftHandSideIdx, RightHandSideIdx, TrueBlockIdx, FalseBlockIdx };

  BinaryOperatorInst::OpKind getOperatorKind() const {
    return op_;
  }
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
    return BinaryOperatorInst::opStringRepr[static_cast<int>(op_)];
  }

  explicit CompareBranchInst(
      Value *left,
      Value *right,
      BinaryOperatorInst::OpKind opKind,
      BasicBlock *trueBlock,
      BasicBlock *falseBlock)
      : TerminatorInst(ValueKind::CompareBranchInstKind), op_(opKind) {
    pushOperand(left);
    pushOperand(right);
    pushOperand(trueBlock);
    pushOperand(falseBlock);
  }
  explicit CompareBranchInst(
      const CompareBranchInst *src,
      llvh::ArrayRef<Value *> operands)
      : TerminatorInst(src, operands), op_(src->op_) {}

  SideEffectKind getSideEffect() {
    return BinaryOperatorInst::getBinarySideEffect(
        getLeftHandSide()->getType(),
        getRightHandSide()->getType(),
        getOperatorKind());
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::CompareBranchInstKind);
  }

  unsigned getNumSuccessors() const {
    return 2;
  }
  BasicBlock *getSuccessor(unsigned idx) const {
    if (idx == 0)
      return getTrueDest();
    if (idx == 1)
      return getFalseDest();
    llvm_unreachable("CompareBranchInst only have 2 successors!");
  }
  void setSuccessor(unsigned idx, BasicBlock *B) {
    assert(idx <= 1 && "CompareBranchInst only have 2 successors!");
    setOperand(B, idx + TrueBlockIdx);
  }
};

class CreateGeneratorInst : public CreateFunctionInst {
  CreateGeneratorInst(const CreateGeneratorInst &) = delete;
  void operator=(const CreateGeneratorInst &) = delete;

 protected:
  explicit CreateGeneratorInst(
      ValueKind kind,
      Function *genFunction,
      Value *environment)
      : CreateFunctionInst(kind, genFunction, environment) {
    setType(Type::createObject());
  }

 public:
  explicit CreateGeneratorInst(
      Function *genFunction,
      ScopeCreationInst *environment)
      : CreateGeneratorInst(
            ValueKind::CreateGeneratorInstKind,
            genFunction,
            environment) {}
  explicit CreateGeneratorInst(
      const CreateGeneratorInst *src,
      llvh::ArrayRef<Value *> operands)
      : CreateFunctionInst(src, operands) {}

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::CreateGeneratorInstKind);
  }
};

/// Creating a closure in HBC requires an explicit environment.
class HBCCreateGeneratorInst : public CreateGeneratorInst {
  HBCCreateGeneratorInst(const HBCCreateGeneratorInst &) = delete;
  void operator=(const HBCCreateGeneratorInst &) = delete;

 public:
  explicit HBCCreateGeneratorInst(Function *code, Value *env)
      : CreateGeneratorInst(ValueKind::HBCCreateGeneratorInstKind, code, env) {}
  explicit HBCCreateGeneratorInst(
      const HBCCreateGeneratorInst *src,
      llvh::ArrayRef<Value *> operands)
      : CreateGeneratorInst(src, operands) {}

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::HBCCreateGeneratorInstKind);
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

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::StartGeneratorInstKind);
  }
};

class ResumeGeneratorInst : public Instruction {
  ResumeGeneratorInst(const ResumeGeneratorInst &) = delete;
  void operator=(const ResumeGeneratorInst &) = delete;

 public:
  enum { IsReturnIdx };

  explicit ResumeGeneratorInst(Value *isReturn)
      : Instruction(ValueKind::ResumeGeneratorInstKind) {
    pushOperand(isReturn);
  }
  explicit ResumeGeneratorInst(
      const ResumeGeneratorInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  Value *getIsReturn() {
    return getOperand(IsReturnIdx);
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::ResumeGeneratorInstKind);
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

  SideEffectKind getSideEffect() const {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return WordBitSet<>{}.set(SourceOrNextIdx);
  }

  Value *getSourceOrNext() const {
    return getOperand(SourceOrNextIdx);
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::IteratorBeginInstKind);
  }
};

class IteratorNextInst : public Instruction {
  IteratorNextInst(const IteratorNextInst &) = delete;
  void operator=(const IteratorNextInst &) = delete;

 public:
  enum { IteratorIdx, SourceOrNextIdx };

  explicit IteratorNextInst(
      AllocStackInst *iterator,
      AllocStackInst *sourceOrNext)
      : Instruction(ValueKind::IteratorNextInstKind) {
    pushOperand(iterator);
    pushOperand(sourceOrNext);
  }
  explicit IteratorNextInst(
      const IteratorNextInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() const {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return WordBitSet<>{}.set(IteratorIdx);
  }

  Value *getIterator() const {
    return getOperand(IteratorIdx);
  }
  Value *getSourceOrNext() const {
    return getOperand(SourceOrNextIdx);
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::IteratorNextInstKind);
  }
};

class IteratorCloseInst : public Instruction {
  IteratorCloseInst(const IteratorCloseInst &) = delete;
  void operator=(const IteratorCloseInst &) = delete;

 public:
  enum { IteratorIdx, IgnoreInnerExceptionIdx };

  using TargetList = llvh::SmallVector<Value *, 2>;

  explicit IteratorCloseInst(
      AllocStackInst *iterator,
      LiteralBool *ignoreInnerException)
      : Instruction(ValueKind::IteratorCloseInstKind) {
    pushOperand(iterator);
    pushOperand(ignoreInnerException);
  }
  explicit IteratorCloseInst(
      const IteratorCloseInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() const {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  Value *getIterator() const {
    return getOperand(IteratorIdx);
  }

  bool getIgnoreInnerException() const {
    return cast<LiteralBool>(getOperand(IgnoreInnerExceptionIdx))->getValue();
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::IteratorCloseInstKind);
  }
};

/// A bytecode version of llvm_unreachable, for use in stubs and similar.
class UnreachableInst : public Instruction {
  UnreachableInst(const UnreachableInst &) = delete;
  void operator=(const UnreachableInst &) = delete;

 public:
  explicit UnreachableInst() : Instruction(ValueKind::UnreachableInstKind) {}
  explicit UnreachableInst(
      const UnreachableInst *src,
      llvh::ArrayRef<Value *> operands)
      : Instruction(src, operands) {}

  SideEffectKind getSideEffect() {
    return SideEffectKind::Unknown;
  }

  WordBitSet<> getChangedOperandsImpl() {
    return {};
  }

  static bool classof(const Value *V) {
    return kindIsA(V->getKind(), ValueKind::UnreachableInstKind);
  }
};

} // end namespace hermes

#endif
