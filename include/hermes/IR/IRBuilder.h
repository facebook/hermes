/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_IR_IRBUILDER_H
#define HERMES_IR_IRBUILDER_H

#include <string>
#include <utility>

#include "llvh/ADT/SmallVector.h"

#include "hermes/AST/Context.h"
#include "hermes/FrontEndDefs/Builtins.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/Wasm/WasmIntrinsics.h"

namespace hermes {

/// The IRBuilder is used for creating IR. The builder APIs is split into two
/// parts. First, APIs for creating blocks and functions. These APIs are
/// stateless and do not affect the second kind of APIs which are used for
/// creating new instructions. The Instruction creation APIS are stateful and
/// follow an insertion point that can be saved, restored and manipulated.
class IRBuilder {
  IRBuilder(const IRBuilder &) = delete;
  void operator=(const IRBuilder &) = delete;

  /// The module is the root of the program that we are building.
  Module *M;
  /// This is where the next instruction will be inserted.
  BasicBlock::iterator InsertionPoint{};
  // The iterator must point into this block:
  BasicBlock *Block{};

  SMLoc Location{};

 public:
  explicit IRBuilder(Module *Mod) : M(Mod), InsertionPoint(nullptr) {}
  explicit IRBuilder(Function *F)
      : M(F->getParent()), InsertionPoint(nullptr) {}

  //--------------------------------------------------------------------------//
  //                          Stateless APIs.                                 //
  //--------------------------------------------------------------------------//

  enum class PropEnumerable { No = 0, Yes = 1 };

  Module *getModule() {
    return M;
  }

  /// Create a new BasicBlock and add it to a function \p Parent.
  BasicBlock *createBasicBlock(Function *Parent);

  /// Create a new Function and add it to the Module.
  /// \param OriginalName the original name specified by the user.
  /// \param insertBefore Another function in the module where this function
  ///   should be inserted before. If null, appends to the end of the module.
  Function *createFunction(
      Identifier OriginalName,
      Function::DefinitionKind definitionKind,
      bool strictMode,
      SourceVisibility sourceVisibility = SourceVisibility::Default,
      SMRange sourceRange = SMRange{},
      bool isGlobal = false,
      Function *insertBefore = nullptr);

  /// Create a new Function and add it to the Module.
  Function *createFunction(
      StringRef OriginalName,
      Function::DefinitionKind definitionKind,
      bool strictMode,
      SourceVisibility sourceVisibility = SourceVisibility::Default,
      SMRange sourceRange = SMRange{},
      bool isGlobal = false,
      Function *insertBefore = nullptr);

  /// Create a new AsyncFunction and add it to the Module.
  /// \param OriginalName the original name specified by the user.
  /// \param insertBefore Another function in the module where this function
  ///   should be inserted before. If null, appends to the end of the module.
  AsyncFunction *createAsyncFunction(
      Identifier OriginalName,
      Function::DefinitionKind definitionKind,
      bool strictMode,
      SourceVisibility sourceVisibility,
      SMRange sourceRange = SMRange{},
      Function *insertBefore = nullptr);

  /// Create a new GeneratorFunction and add it to the Module.
  /// \param OriginalName the original name specified by the user.
  /// \param insertBefore Another function in the module where this function
  ///   should be inserted before. If null, appends to the end of the module.
  GeneratorFunction *createGeneratorFunction(
      Identifier OriginalName,
      Function::DefinitionKind definitionKind,
      bool strictMode,
      SourceVisibility sourceVisibility,
      SMRange sourceRange = SMRange{},
      Function *insertBefore = nullptr);

  /// Create a new GeneratorInnerFunction and add it to the Module.
  /// \param OriginalName the original name specified by the user.
  /// \param insertBefore Another function in the module where this function
  ///   should be inserted before. If null, appends to the end of the module.
  GeneratorInnerFunction *createGeneratorInnerFunction(
      Identifier OriginalName,
      Function::DefinitionKind definitionKind,
      bool strictMode,
      SMRange sourceRange = SMRange{},
      Function *insertBefore = nullptr);

  /// Create the top level function representing the global scope.
  Function *createTopLevelFunction(
      bool strictMode,
      SourceVisibility sourceVisibility = SourceVisibility::Default,
      SMRange sourceRange = SMRange{});

  /// Create a new ExternalScope with the given depth, which must be negative.
  ExternalScope *createExternalScope(Function *function, int32_t depth);

  /// Create a new global object property.
  GlobalObjectProperty *createGlobalObjectProperty(
      Identifier name,
      bool declared);
  /// Create a new global object property.
  GlobalObjectProperty *createGlobalObjectProperty(
      StringRef name,
      bool declared);

  /// Add a new parameter to function \p Parent.
  Parameter *createParameter(Function *Parent, Identifier Name);

  /// Add a new parameter to function \p Parent.
  Parameter *createParameter(Function *Parent, StringRef Name);

  /// Add a new variable to scope \p Parent.
  Variable *createVariable(
      VariableScope *Parent,
      Variable::DeclKind declKind,
      Identifier Name);

  /// Add a new variable to scope \p Parent.
  Variable *createVariable(
      VariableScope *Parent,
      Variable::DeclKind declKind,
      StringRef Name);

  /// Create a new literal number of value \p value.
  LiteralNumber *getLiteralNumber(double value);

  /// Create a new literal positive zero.
  LiteralNumber *getLiteralPositiveZero();

  /// Create a new literal negative zero.
  LiteralNumber *getLiteralNegativeZero();

  /// Create a new literal NaN.
  LiteralNumber *getLiteralNaN();

  /// Create a new literal BitInt of value \p value.
  LiteralBigInt *getLiteralBigInt(UniqueString *value);

  /// Create a new literal string of value \p value.
  LiteralString *getLiteralString(StringRef value);

  /// Create a new literal string of value \p value.
  LiteralString *getLiteralString(Identifier value);

  /// Create a new literal bool of value \p value.
  LiteralBool *getLiteralBool(bool value);

  /// Create a new literal 'empty'.
  LiteralEmpty *getLiteralEmpty();

  /// Create a new literal 'undefined'.
  LiteralUndefined *getLiteralUndefined();

  /// Create a new literal null.
  LiteralNull *getLiteralNull();

  /// Return the GlobalObject value.
  GlobalObject *getGlobalObject();

  /// Return the EmptySentinel value.
  EmptySentinel *getEmptySentinel();

  /// Convert StringRef to Identifier.
  Identifier createIdentifier(StringRef str);

  //--------------------------------------------------------------------------//
  //                          Statefull APIs.                                 //
  //--------------------------------------------------------------------------//

 private:
  /// Insert the newly created instruction \p Inst into the basic block without
  /// touching its location or statement index.
  void justInsert(Instruction *Inst);

  /// Insert the newly created instruction \p Inst into the basic block and
  /// populate its location and statement index.
  void insert(Instruction *Inst);

 public:
  /// Set the insertion point of the builder. The next instruction will be
  /// inserted at the end of \p BB.
  void setInsertionBlock(BasicBlock *BB);

  /// Return the current insertion block.
  BasicBlock *getInsertionBlock();

  /// Return the current function.
  Function *getFunction() {
    assert(Block && "Builder has no current function");
    return Block->getParent();
  }

  /// Set the insertion point of the builder. The next instruction will be
  /// inserted after \p IP.
  void setInsertionPointAfter(Instruction *IP);

  /// Set the insertion point of the builder. The next instruction will be
  /// inserted where the current \p IP is (thereby pushing it down).
  void setInsertionPoint(Instruction *IP);

  /// \returns true if the insertion point is set.
  bool isInsertionPointValid();

  /// Clear the insertion point. New instructions must not be created until the
  /// insertion point it set.
  void resetInsertionPoint();

  void setLocation(SMLoc loc) {
    Location = loc;
  }
  void clearLocation() {
    Location = SMLoc{};
  }
  SMLoc getLocation() const {
    return Location;
  }

  /// Move instruction \p inst from its block to the insertion point in the
  /// current block. If the instruction is a terminator, correctly update the
  /// phi-nodes that refer to the old block and point then to the current one.
  void transferInstructionToCurrentBlock(Instruction *inst);

  //--------------------------------------------------------------------------//
  //                    Instruction Creation methods                          //
  //--------------------------------------------------------------------------//

  /// Insert a clone of the instruction at the current insertion point. The
  /// location and statement index are preserved but the operands are recreated.
  /// \param source the original instruction.
  /// \param operands the (updated) operands to use in the new instruction.
  Instruction *cloneInst(
      const Instruction *source,
      llvh::ArrayRef<Value *> operands);

  BranchInst *createBranchInst(BasicBlock *Destination);

  CondBranchInst *
  createCondBranchInst(Value *Cond, BasicBlock *T, BasicBlock *F);

  ReturnInst *createReturnInst(Value *Val);

  AllocStackInst *createAllocStackInst(StringRef varName);

  AllocStackInst *createAllocStackInst(Identifier varName);

  AsNumberInst *createAsNumberInst(Value *val);

  AsNumericInst *createAsNumericInst(Value *val);

  AsInt32Inst *createAsInt32Inst(Value *val);

  AddEmptyStringInst *createAddEmptyStringInst(Value *val);

  CreateFunctionInst *createCreateFunctionInst(Function *code);

  LoadStackInst *createLoadStackInst(AllocStackInst *ptr);

  LoadFrameInst *createLoadFrameInst(Variable *ptr);

  StoreStackInst *createStoreStackInst(Value *storedValue, AllocStackInst *ptr);

  StoreFrameInst *createStoreFrameInst(Value *storedValue, Variable *ptr);

  CallInst *
  createCallInst(Value *callee, Value *thisValue, ArrayRef<Value *> args);

  HBCCallNInst *
  createHBCCallNInst(Value *callee, Value *thisValue, ArrayRef<Value *> args);

  ConstructInst *createConstructInst(
      Value *constructor,
      ArrayRef<Value *> args);

  CatchInst *createCatchInst();

  ThrowInst *createThrowInst(Value *thrownValue);

  CheckHasInstanceInst *createCheckHasInstanceInst(
      AllocStackInst *result,
      Value *left,
      Value *right,
      BasicBlock *onTrue,
      BasicBlock *onFalse);

  TryStartInst *createTryStartInst(
      BasicBlock *tryBodyBlock,
      BasicBlock *catchTargetBlock);

  TryEndInst *createTryEndInst();

  DeletePropertyInst *createDeletePropertyInst(Value *object, Value *property);

  LoadPropertyInst *createLoadPropertyInst(Value *object, Value *property);
  TryLoadGlobalPropertyInst *createTryLoadGlobalPropertyInst(
      LiteralString *property);
  TryLoadGlobalPropertyInst *createTryLoadGlobalPropertyInst(
      GlobalObjectProperty *property);

  StorePropertyInst *
  createStorePropertyInst(Value *storedValue, Value *object, Value *property);
  TryStoreGlobalPropertyInst *createTryStoreGlobalPropertyInst(
      Value *storedValue,
      LiteralString *property);
  TryStoreGlobalPropertyInst *createTryStoreGlobalPropertyInst(
      Value *storedValue,
      GlobalObjectProperty *property);
  StoreOwnPropertyInst *createStoreOwnPropertyInst(
      Value *storedValue,
      Value *object,
      Value *property,
      PropEnumerable isEnumerable);
  StoreNewOwnPropertyInst *createStoreNewOwnPropertyInst(
      Value *storedValue,
      Value *object,
      Literal *property,
      PropEnumerable isEnumerable);

  StoreGetterSetterInst *createStoreGetterSetterInst(
      Value *storedGetter,
      Value *storedSetter,
      Value *object,
      Value *property,
      PropEnumerable isEnumerable);
  DeletePropertyInst *createDeletePropertyInst(
      Value *object,
      StringRef property);

  LoadPropertyInst *createLoadPropertyInst(Value *object, StringRef property);
  TryLoadGlobalPropertyInst *createTryLoadGlobalPropertyInst(
      StringRef property);

  StorePropertyInst *createStorePropertyInst(
      Value *storedValue,
      Value *object,
      StringRef property);

  DeletePropertyInst *createDeletePropertyInst(
      Value *object,
      Identifier property);

  LoadPropertyInst *createLoadPropertyInst(Value *object, Identifier property);
  TryLoadGlobalPropertyInst *createTryLoadGlobalPropertyInst(
      Identifier property);

  StorePropertyInst *createStorePropertyInst(
      Value *storedValue,
      Value *object,
      Identifier property);
  TryStoreGlobalPropertyInst *createTryStoreGlobalPropertyInst(
      Value *storedValue,
      Identifier property);

  AllocObjectInst *createAllocObjectInst(
      uint32_t size,
      Value *parent = nullptr);

  AllocObjectLiteralInst *createAllocObjectLiteralInst(
      const AllocObjectLiteralInst::ObjectPropertyMap &propMap);

  AllocArrayInst *createAllocArrayInst(
      LiteralNumber *sizeHint,
      AllocArrayInst::ArrayValueList val_list);

  AllocArrayInst *createAllocArrayInst(
      AllocArrayInst::ArrayValueList val_list,
      unsigned sizeHint);

  CreateArgumentsInst *createCreateArgumentsInst();

  GetNewTargetInst *createGetNewTargetInst();

  ThrowIfEmptyInst *createThrowIfEmptyInst(Value *checkedValue);

  HBCGetGlobalObjectInst *createHBCGetGlobalObjectInst();

  CreateRegExpInst *createRegExpInst(Identifier pattern, Identifier flags);

  UnaryOperatorInst *createUnaryOperatorInst(
      Value *value,
      UnaryOperatorInst::OpKind opKind);

  DirectEvalInst *createDirectEvalInst(Value *operand);

  SwitchInst *createSwitchInst(
      Value *input,
      BasicBlock *defaultBlock,
      const SwitchInst::ValueListType &values,
      const SwitchInst::BasicBlockListType &blocks);

  PhiInst *createPhiInst(
      const PhiInst::ValueListType &values,
      const PhiInst::BasicBlockListType &blocks);

  PhiInst *createPhiInst();

  BinaryOperatorInst *createBinaryOperatorInst(
      Value *left,
      Value *right,
      BinaryOperatorInst::OpKind opKind);

  GetPNamesInst *createGetPNamesInst(
      Value *iteratorAddr,
      Value *baseAddr,
      Value *indexAddr,
      Value *sizeAddr,
      BasicBlock *onEmpty,
      BasicBlock *onSome);

  GetNextPNameInst *createGetNextPNameInst(
      Value *propertyAddr,
      Value *baseAddr,
      Value *indexAddr,
      Value *sizeAddr,
      Value *iteratorAddr,
      BasicBlock *onLast,
      BasicBlock *onSome);

  MovInst *createMovInst(Value *input);

  ImplicitMovInst *createImplicitMovInst(Value *input);

  CoerceThisNSInst *createCoerceThisNSInst(Value *input);

  DebuggerInst *createDebuggerInst();

  SaveAndYieldInst *createSaveAndYieldInst(
      Value *result,
      BasicBlock *nextBlock);

  CreateGeneratorInst *createCreateGeneratorInst(Function *innerFn);

  StartGeneratorInst *createStartGeneratorInst();

  ResumeGeneratorInst *createResumeGeneratorInst(Value *isReturn);

  //--------------------------------------------------------------------------//
  //                  Target specific insertions                              //
  //--------------------------------------------------------------------------//

  HBCResolveEnvironment *createHBCResolveEnvironment(VariableScope *scope);
  HBCStoreToEnvironmentInst *
  createHBCStoreToEnvironmentInst(Value *env, Value *toPut, Variable *var);
  HBCLoadFromEnvironmentInst *createHBCLoadFromEnvironmentInst(
      Value *env,
      Variable *var);

  SwitchImmInst *createSwitchImmInst(
      Value *input,
      BasicBlock *defaultBlock,
      LiteralNumber *minValue,
      LiteralNumber *size,
      const SwitchImmInst::ValueListType &values,
      const SwitchImmInst::BasicBlockListType &blocks);

  HBCLoadConstInst *createHBCLoadConstInst(Literal *value);

  HBCLoadParamInst *createHBCLoadParamInst(LiteralNumber *value);

  HBCCreateEnvironmentInst *createHBCCreateEnvironmentInst();

  HBCGetThisNSInst *createHBCGetThisNSInst();

  HBCGetArgumentsPropByValInst *createHBCGetArgumentsPropByValInst(
      Value *index,
      AllocStackInst *lazyReg);

  HBCGetArgumentsLengthInst *createHBCGetArgumentsLengthInst(
      AllocStackInst *lazyReg);

  HBCReifyArgumentsInst *createHBCReifyArgumentsInst(AllocStackInst *lazyReg);

  HBCCreateThisInst *createHBCCreateThisInst(Value *prototype, Value *closure);

  HBCConstructInst *createHBCConstructInst(
      Value *closure,
      Value *thisValue,
      ArrayRef<Value *> arguments);
  HBCGetConstructedObjectInst *createHBCGetConstructedObjectInst(
      HBCCreateThisInst *thisValue,
      HBCConstructInst *constructorReturnValue);
  HBCProfilePointInst *createHBCProfilePointInst(uint16_t pointIndex);

  CallBuiltinInst *createCallBuiltinInst(
      BuiltinMethod::Enum builtinIndex,
      ArrayRef<Value *> arguments);

  GetBuiltinClosureInst *createGetBuiltinClosureInst(
      BuiltinMethod::Enum builtinIndex);

#ifdef HERMES_RUN_WASM
  CallIntrinsicInst *createCallIntrinsicInst(
      WasmIntrinsics::Enum intrinsicsIndex,
      ArrayRef<Value *> arguments);
#endif

  HBCCallDirectInst *createHBCCallDirectInst(
      Function *callee,
      Value *thisValue,
      ArrayRef<Value *> arguments);

  HBCCreateFunctionInst *createHBCCreateFunctionInst(
      Function *function,
      Value *env);
  HBCSpillMovInst *createHBCSpillMovInst(Instruction *value);

  HBCCreateGeneratorInst *createHBCCreateGeneratorInst(
      Function *function,
      Value *env);

  HBCAllocObjectFromBufferInst *createHBCAllocObjectFromBufferInst(
      HBCAllocObjectFromBufferInst::ObjectPropertyMap prop_map,
      uint32_t size);

  CompareBranchInst *createCompareBranchInst(
      Value *left,
      Value *right,
      BinaryOperatorInst::OpKind opKind,
      BasicBlock *trueBlock,
      BasicBlock *falseBlock);

  IteratorBeginInst *createIteratorBeginInst(AllocStackInst *sourceOrNext);

  IteratorNextInst *createIteratorNextInst(
      AllocStackInst *iterator,
      AllocStackInst *sourceOrNext);

  IteratorCloseInst *createIteratorCloseInst(
      AllocStackInst *iterator,
      bool ignoreInnerException);

  UnreachableInst *createUnreachableInst();

  /// This is an RAII object that saves and restores the source location of the
  /// IRBuilder.
  class ScopedLocationChange {
    ScopedLocationChange(const ScopedLocationChange &) = delete;
    void operator=(const ScopedLocationChange &) = delete;

    IRBuilder &builder_;
    SMLoc oldLocation_;

   public:
    /// Save the builder source location when constructing the object.
    explicit ScopedLocationChange(IRBuilder &builder, SMLoc location)
        : builder_(builder), oldLocation_(builder.getLocation()) {
      builder_.setLocation(location);
    }

    /// Resotre source location when the object is destroyed.
    ~ScopedLocationChange() {
      builder_.setLocation(oldLocation_);
    }
  };

  //--------------------------------------------------------------------------//
  //                  Insertion point Save and Restore                        //
  //--------------------------------------------------------------------------//

  /// This is an RAII object that saves and restores the insertion point of the
  /// IRBuilder.
  class SaveRestore {
    SaveRestore(const SaveRestore &) = delete;
    void operator=(const SaveRestore &) = delete;

    IRBuilder &Builder;
    BasicBlock *BB;
    SMLoc Location;

   public:
    // Save the builder insertion point when constructing the object.
    explicit SaveRestore(IRBuilder &builder)
        : Builder(builder),
          BB(builder.getInsertionBlock()),
          Location(builder.getLocation()) {
      builder.clearLocation();
    }

    // Restore insertion point when the object is destroyed.
    ~SaveRestore() {
      Builder.setInsertionBlock(BB);
      Builder.setLocation(Location);
    }
  };

  /// This is an RAII object that destroys instructions when it is destroyed.
  class InstructionDestroyer {
    InstructionDestroyer(const InstructionDestroyer &) = delete;
    void operator=(const InstructionDestroyer &) = delete;

    llvh::SmallVector<Instruction *, 8> list{};

   public:
    explicit InstructionDestroyer() = default;

    /// \returns true if the instruction \p A is already in the destruction
    /// queue. Notice that this is an O(n) search and should only be used for
    /// debugging.
    bool hasInstruction(Instruction *A) {
      return std::find(list.begin(), list.end(), A) != list.end();
    }

    /// Add the instruction \p  A to the list of instructions to delete.
    void add(Instruction *A) {
#ifndef NDEBUG
      assert(!hasInstruction(A) && "Instruction already in list!");
#endif
      list.push_back(A);
    }

    ~InstructionDestroyer() {
      for (auto *I : list) {
        I->eraseFromParent();
      }
    }
  };
};

} // end namespace hermes

#endif
