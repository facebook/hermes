/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "llvh/Support/Casting.h"
#include "llvh/Support/ErrorHandling.h"

#include "hermes/AST/Context.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"

using namespace hermes;

BasicBlock *IRBuilder::createBasicBlock(Function *Parent) {
  assert(Parent && "Invalid insertion point");
  return new BasicBlock(Parent);
}

Function *IRBuilder::createFunction(
    Identifier OriginalName,
    Function::DefinitionKind definitionKind,
    bool strictMode,
    SourceVisibility sourceVisibility,
    SMRange sourceRange,
    bool isGlobal,
    Function *insertBefore) {
  // Function must have a name. If the source doesn't provide the function name,
  // Hermes will try to infer the name from the name of a variable or property
  // it is assigned to. If there was no inference, an empty string is used.
  if (!OriginalName.isValid()) {
    OriginalName = createIdentifier("");
  }
  return new Function(
      M,
      OriginalName,
      definitionKind,
      strictMode,
      sourceVisibility,
      isGlobal,
      sourceRange,
      insertBefore);
}

GeneratorFunction *IRBuilder::createGeneratorFunction(
    Identifier OriginalName,
    Function::DefinitionKind definitionKind,
    bool strictMode,
    SourceVisibility sourceVisibility,
    SMRange sourceRange,
    Function *insertBefore) {
  if (!OriginalName.isValid()) {
    // Function must have a name, even it's empty.
    // Eventually we will give it a properly inferred name.
    OriginalName = createIdentifier("");
  }
  return new GeneratorFunction(
      M,
      OriginalName,
      definitionKind,
      strictMode,
      sourceVisibility,
      /* isGlobal */ false,
      sourceRange,
      insertBefore);
}

GeneratorInnerFunction *IRBuilder::createGeneratorInnerFunction(
    Identifier OriginalName,
    Function::DefinitionKind definitionKind,
    bool strictMode,
    SMRange sourceRange,
    Function *insertBefore) {
  if (!OriginalName.isValid()) {
    // Function must have a name, even it's empty.
    // Eventually we will give it a properly inferred name.
    OriginalName = createIdentifier("");
  }
  return new GeneratorInnerFunction(
      M,
      OriginalName,
      definitionKind,
      strictMode,
      /* isGlobal */ false,
      sourceRange,
      insertBefore);
}

ExternalScope *IRBuilder::createExternalScope(
    Function *function,
    int32_t depth) {
  return new ExternalScope(function, depth);
}

Function *IRBuilder::createTopLevelFunction(
    bool strictMode,
    SourceVisibility sourceVisibility,
    SMRange sourceRange) {
  // Notice that this synthesized name is not a legal javascript name and
  // can't collide with functions in the processed program.
  return createFunction(
      "global",
      Function::DefinitionKind::ES5Function,
      strictMode,
      sourceVisibility,
      sourceRange,
      true);
}

Function *IRBuilder::createFunction(
    StringRef OriginalName,
    Function::DefinitionKind definitionKind,
    bool strictMode,
    SourceVisibility sourceVisibility,
    SMRange sourceRange,
    bool isGlobal,
    Function *insertBefore) {
  Identifier OrigIden =
      OriginalName.empty() ? Identifier{} : createIdentifier(OriginalName);
  return createFunction(
      OrigIden,
      definitionKind,
      strictMode,
      sourceVisibility,
      sourceRange,
      isGlobal,
      insertBefore);
}

AsyncFunction *IRBuilder::createAsyncFunction(
    Identifier OriginalName,
    Function::DefinitionKind definitionKind,
    bool strictMode,
    SourceVisibility sourceVisibility,
    SMRange sourceRange,
    Function *insertBefore) {
  if (!OriginalName.isValid()) {
    // Function must have a name, even it's empty.
    // Eventually we will give it a properly inferred name.
    OriginalName = createIdentifier("");
  }
  return new AsyncFunction(
      M,
      OriginalName,
      definitionKind,
      strictMode,
      sourceVisibility,
      /* isGlobal */ false,
      sourceRange,
      insertBefore);
}

GlobalObjectProperty *IRBuilder::createGlobalObjectProperty(
    Identifier name,
    bool declared) {
  return M->addGlobalProperty(name, declared);
}
GlobalObjectProperty *IRBuilder::createGlobalObjectProperty(
    StringRef name,
    bool declared) {
  return createGlobalObjectProperty(
      M->getContext().getIdentifier(name), declared);
}

Parameter *IRBuilder::createParameter(Function *Parent, Identifier Name) {
  return new Parameter(Parent, Name);
}

Parameter *IRBuilder::createParameter(Function *Parent, StringRef Name) {
  return createParameter(Parent, createIdentifier(Name));
}

Variable *IRBuilder::createVariable(
    VariableScope *Parent,
    Variable::DeclKind declKind,
    Identifier Name) {
  return new Variable(Parent, declKind, Name);
}

Variable *IRBuilder::createVariable(
    VariableScope *Parent,
    Variable::DeclKind declKind,
    StringRef Name) {
  return createVariable(Parent, declKind, createIdentifier(Name));
}

LiteralNumber *IRBuilder::getLiteralNumber(double value) {
  return M->getLiteralNumber(value);
}

// FIXME: use proper language semantics.
LiteralNumber *IRBuilder::getLiteralPositiveZero() {
  return M->getLiteralNumber(+0.0);
}

LiteralNumber *IRBuilder::getLiteralNegativeZero() {
  return M->getLiteralNumber(-0.0);
}

LiteralNumber *IRBuilder::getLiteralNaN() {
  return M->getLiteralNumber(std::numeric_limits<double>::quiet_NaN());
}

LiteralBigInt *IRBuilder::getLiteralBigInt(UniqueString *value) {
  return M->getLiteralBigInt(value);
}

LiteralString *IRBuilder::getLiteralString(StringRef value) {
  Identifier Iden = createIdentifier(value);
  return getLiteralString(Iden);
}

LiteralString *IRBuilder::getLiteralString(Identifier value) {
  return M->getLiteralString(value);
}

LiteralBool *IRBuilder::getLiteralBool(bool value) {
  return M->getLiteralBool(value);
}

LiteralEmpty *IRBuilder::getLiteralEmpty() {
  return M->getLiteralEmpty();
}

LiteralUndefined *IRBuilder::getLiteralUndefined() {
  return M->getLiteralUndefined();
}

LiteralNull *IRBuilder::getLiteralNull() {
  return M->getLiteralNull();
}

GlobalObject *IRBuilder::getGlobalObject() {
  return M->getGlobalObject();
}

EmptySentinel *IRBuilder::getEmptySentinel() {
  return M->getEmptySentinel();
}

Identifier IRBuilder::createIdentifier(StringRef str) {
  return M->getContext().getIdentifier(str);
}

BranchInst *IRBuilder::createBranchInst(BasicBlock *Destination) {
  auto *BI = new BranchInst(getInsertionBlock(), Destination);
  insert(BI);
  return BI;
}

CondBranchInst *
IRBuilder::createCondBranchInst(Value *Cond, BasicBlock *T, BasicBlock *F) {
  auto *CBI = new CondBranchInst(getInsertionBlock(), Cond, T, F);
  insert(CBI);
  return CBI;
}

ReturnInst *IRBuilder::createReturnInst(Value *Val) {
  auto *RI = new ReturnInst(Val);
  insert(RI);
  return RI;
}

CatchInst *IRBuilder::createCatchInst() {
  auto *CI = new CatchInst();
  insert(CI);
  return CI;
}

ThrowInst *IRBuilder::createThrowInst(Value *thrownValue) {
  auto *TI = new ThrowInst(thrownValue);
  insert(TI);
  return TI;
}

CheckHasInstanceInst *IRBuilder::createCheckHasInstanceInst(
    AllocStackInst *result,
    Value *left,
    Value *right,
    BasicBlock *onTrue,
    BasicBlock *onFalse) {
  auto *TI = new CheckHasInstanceInst(result, left, right, onTrue, onFalse);
  insert(TI);
  return TI;
}

TryStartInst *IRBuilder::createTryStartInst(
    BasicBlock *tryBodyBlock,
    BasicBlock *catchTargetBlock) {
  auto *I = new TryStartInst(tryBodyBlock, catchTargetBlock);
  insert(I);
  return I;
}

TryEndInst *IRBuilder::createTryEndInst() {
  auto *I = new TryEndInst();
  insert(I);
  return I;
}

AllocStackInst *IRBuilder::createAllocStackInst(StringRef varName) {
  Identifier Iden = createIdentifier(varName);
  return createAllocStackInst(Iden);
}

AllocStackInst *IRBuilder::createAllocStackInst(Identifier varName) {
  auto *AHI = new AllocStackInst(varName);
  insert(AHI);
  return AHI;
}

AsNumberInst *IRBuilder::createAsNumberInst(Value *val) {
  auto *ANI = new AsNumberInst(val);
  insert(ANI);
  return ANI;
}

AsNumericInst *IRBuilder::createAsNumericInst(Value *val) {
  auto *ANI = new AsNumericInst(val);
  insert(ANI);
  return ANI;
}

AsInt32Inst *IRBuilder::createAsInt32Inst(Value *val) {
  auto *AII = new AsInt32Inst(val);
  insert(AII);
  return AII;
}

AddEmptyStringInst *IRBuilder::createAddEmptyStringInst(Value *val) {
  auto *I = new AddEmptyStringInst(val);
  insert(I);
  return I;
}

CreateFunctionInst *IRBuilder::createCreateFunctionInst(Function *code) {
  auto CFI = new CreateFunctionInst(code);
  insert(CFI);
  return CFI;
}

LoadFrameInst *IRBuilder::createLoadFrameInst(Variable *ptr) {
  auto LI = new LoadFrameInst(ptr);
  insert(LI);
  return LI;
}

LoadStackInst *IRBuilder::createLoadStackInst(AllocStackInst *ptr) {
  auto LI = new LoadStackInst(ptr);
  insert(LI);
  return LI;
}

StoreFrameInst *IRBuilder::createStoreFrameInst(
    Value *storedValue,
    Variable *ptr) {
  auto SI = new StoreFrameInst(storedValue, ptr);
  insert(SI);
  return SI;
}

StoreStackInst *IRBuilder::createStoreStackInst(
    Value *storedValue,
    AllocStackInst *ptr) {
  auto SI = new StoreStackInst(storedValue, ptr);
  insert(SI);
  return SI;
}

CallInst *IRBuilder::createCallInst(
    Value *callee,
    Value *thisValue,
    ArrayRef<Value *> args) {
  auto CI = new CallInst(ValueKind::CallInstKind, callee, thisValue, args);
  insert(CI);
  return CI;
}

HBCCallNInst *IRBuilder::createHBCCallNInst(
    Value *callee,
    Value *thisValue,
    ArrayRef<Value *> args) {
  auto CI = new HBCCallNInst(callee, thisValue, args);
  insert(CI);
  return CI;
}

ConstructInst *IRBuilder::createConstructInst(
    Value *constructor,
    ArrayRef<Value *> args) {
  auto *inst = new ConstructInst(constructor, getLiteralUndefined(), args);
  insert(inst);
  return inst;
}

LoadPropertyInst *IRBuilder::createLoadPropertyInst(
    Value *object,
    Value *property) {
  auto LPI = new LoadPropertyInst(object, property);
  insert(LPI);
  return LPI;
}

TryLoadGlobalPropertyInst *IRBuilder::createTryLoadGlobalPropertyInst(
    LiteralString *property) {
  auto *inst = new TryLoadGlobalPropertyInst(getGlobalObject(), property);
  insert(inst);
  return inst;
}

TryLoadGlobalPropertyInst *IRBuilder::createTryLoadGlobalPropertyInst(
    GlobalObjectProperty *property) {
  return createTryLoadGlobalPropertyInst(property->getName());
}

DeletePropertyInst *IRBuilder::createDeletePropertyInst(
    Value *object,
    Value *property) {
  auto DPI = new DeletePropertyInst(object, property);
  insert(DPI);
  return DPI;
}

StorePropertyInst *IRBuilder::createStorePropertyInst(
    Value *storedValue,
    Value *object,
    Value *property) {
  auto SPI = new StorePropertyInst(storedValue, object, property);
  insert(SPI);
  return SPI;
}
TryStoreGlobalPropertyInst *IRBuilder::createTryStoreGlobalPropertyInst(
    Value *storedValue,
    LiteralString *property) {
  auto *inst =
      new TryStoreGlobalPropertyInst(storedValue, getGlobalObject(), property);
  insert(inst);
  return inst;
}
TryStoreGlobalPropertyInst *IRBuilder::createTryStoreGlobalPropertyInst(
    Value *storedValue,
    GlobalObjectProperty *property) {
  return createTryStoreGlobalPropertyInst(storedValue, property->getName());
}

StoreOwnPropertyInst *IRBuilder::createStoreOwnPropertyInst(
    Value *storedValue,
    Value *object,
    Value *property,
    PropEnumerable isEnumerable) {
  auto SPI = new StoreOwnPropertyInst(
      storedValue,
      object,
      property,
      getLiteralBool(isEnumerable == PropEnumerable::Yes));
  insert(SPI);
  return SPI;
}
StoreNewOwnPropertyInst *IRBuilder::createStoreNewOwnPropertyInst(
    Value *storedValue,
    Value *object,
    Literal *property,
    PropEnumerable isEnumerable) {
  auto *inst = new StoreNewOwnPropertyInst(
      storedValue,
      object,
      property,
      getLiteralBool(isEnumerable == PropEnumerable::Yes));
  insert(inst);
  return inst;
}

StoreGetterSetterInst *IRBuilder::createStoreGetterSetterInst(
    Value *storedGetter,
    Value *storedSetter,
    Value *object,
    Value *property,
    PropEnumerable isEnumerable) {
  auto *SGSI = new StoreGetterSetterInst(
      storedGetter,
      storedSetter,
      object,
      property,
      getLiteralBool(isEnumerable == PropEnumerable::Yes));
  insert(SGSI);
  return SGSI;
}

DeletePropertyInst *IRBuilder::createDeletePropertyInst(
    Value *object,
    StringRef property) {
  Identifier Iden = createIdentifier(property);
  return createDeletePropertyInst(object, Iden);
}

LoadPropertyInst *IRBuilder::createLoadPropertyInst(
    Value *object,
    StringRef property) {
  Identifier Iden = createIdentifier(property);
  return createLoadPropertyInst(object, Iden);
}

TryLoadGlobalPropertyInst *IRBuilder::createTryLoadGlobalPropertyInst(
    StringRef property) {
  return createTryLoadGlobalPropertyInst(createIdentifier(property));
}

StorePropertyInst *IRBuilder::createStorePropertyInst(
    Value *storedValue,
    Value *object,
    StringRef property) {
  Identifier Iden = createIdentifier(property);
  return createStorePropertyInst(storedValue, object, Iden);
}

LoadPropertyInst *IRBuilder::createLoadPropertyInst(
    Value *object,
    Identifier property) {
  auto L = getLiteralString(property);
  return createLoadPropertyInst(object, L);
}

TryLoadGlobalPropertyInst *IRBuilder::createTryLoadGlobalPropertyInst(
    Identifier property) {
  auto *inst = new TryLoadGlobalPropertyInst(
      getGlobalObject(), getLiteralString(property));
  insert(inst);
  return inst;
}

DeletePropertyInst *IRBuilder::createDeletePropertyInst(
    Value *object,
    Identifier property) {
  auto L = getLiteralString(property);
  return createDeletePropertyInst(object, L);
}

StorePropertyInst *IRBuilder::createStorePropertyInst(
    Value *storedValue,
    Value *object,
    Identifier property) {
  auto L = getLiteralString(property);
  return createStorePropertyInst(storedValue, object, L);
}

TryStoreGlobalPropertyInst *IRBuilder::createTryStoreGlobalPropertyInst(
    Value *storedValue,
    Identifier property) {
  auto *inst = new TryStoreGlobalPropertyInst(
      storedValue, getGlobalObject(), getLiteralString(property));
  insert(inst);
  return inst;
}

AllocObjectInst *IRBuilder::createAllocObjectInst(
    uint32_t size,
    Value *parent) {
  auto AOI = new AllocObjectInst(
      M->getLiteralNumber(size), parent ? parent : getEmptySentinel());
  insert(AOI);
  return AOI;
}

AllocArrayInst *IRBuilder::createAllocArrayInst(
    LiteralNumber *sizeHint,
    AllocArrayInst::ArrayValueList val_list) {
  auto AAI = new AllocArrayInst(val_list, sizeHint);
  insert(AAI);
  return AAI;
}

AllocArrayInst *IRBuilder::createAllocArrayInst(
    AllocArrayInst::ArrayValueList val_list,
    unsigned sizeHint) {
  return createAllocArrayInst(this->getLiteralNumber(sizeHint), val_list);
}

CreateArgumentsInst *IRBuilder::createCreateArgumentsInst() {
  auto CAI = new CreateArgumentsInst();
  insert(CAI);
  return CAI;
}

GetNewTargetInst *IRBuilder::createGetNewTargetInst() {
  auto *inst = new GetNewTargetInst();
  insert(inst);
  return inst;
}

ThrowIfEmptyInst *IRBuilder::createThrowIfEmptyInst(Value *checkedValue) {
  auto *inst = new ThrowIfEmptyInst(checkedValue);
  insert(inst);
  return inst;
}

HBCGetGlobalObjectInst *IRBuilder::createHBCGetGlobalObjectInst() {
  auto inst = new HBCGetGlobalObjectInst();
  insert(inst);
  return inst;
}

CreateRegExpInst *IRBuilder::createRegExpInst(
    Identifier pattern,
    Identifier flags) {
  auto res =
      new CreateRegExpInst(getLiteralString(pattern), getLiteralString(flags));
  insert(res);
  return res;
}

UnaryOperatorInst *IRBuilder::createUnaryOperatorInst(
    Value *value,
    UnaryOperatorInst::OpKind opKind) {
  auto UOI = new UnaryOperatorInst(value, opKind);
  insert(UOI);
  return UOI;
}

BinaryOperatorInst *IRBuilder::createBinaryOperatorInst(
    Value *left,
    Value *right,
    BinaryOperatorInst::OpKind opKind) {
  auto BOI = new BinaryOperatorInst(left, right, opKind);
  insert(BOI);
  return BOI;
}

SwitchInst *IRBuilder::createSwitchInst(
    Value *input,
    BasicBlock *defaultBlock,
    const SwitchInst::ValueListType &values,
    const SwitchInst::BasicBlockListType &blocks) {
  auto SI = new SwitchInst(input, defaultBlock, values, blocks);
  insert(SI);
  return SI;
}

PhiInst *IRBuilder::createPhiInst() {
  PhiInst::ValueListType values;
  PhiInst::BasicBlockListType blocks;

  return createPhiInst(values, blocks);
}

PhiInst *IRBuilder::createPhiInst(
    const PhiInst::ValueListType &values,
    const PhiInst::BasicBlockListType &blocks) {
  auto PI = new PhiInst(values, blocks);
  insert(PI);
  return PI;
}

GetPNamesInst *IRBuilder::createGetPNamesInst(
    Value *iteratorAddr,
    Value *baseAddr,
    Value *indexAddr,
    Value *sizeAddr,
    BasicBlock *onEmpty,
    BasicBlock *onSome) {
  auto GP = new GetPNamesInst(
      getInsertionBlock(),
      iteratorAddr,
      baseAddr,
      indexAddr,
      sizeAddr,
      onEmpty,
      onSome);
  insert(GP);
  return GP;
}

GetNextPNameInst *IRBuilder::createGetNextPNameInst(
    Value *propertyAddr,
    Value *baseAddr,
    Value *indexAddr,
    Value *sizeAddr,
    Value *iteratorAddr,
    BasicBlock *onLast,
    BasicBlock *onSome) {
  auto GNP = new GetNextPNameInst(
      getInsertionBlock(),
      propertyAddr,
      baseAddr,
      indexAddr,
      sizeAddr,
      iteratorAddr,
      onLast,
      onSome);
  insert(GNP);
  return GNP;
}

MovInst *IRBuilder::createMovInst(Value *input) {
  auto MI = new MovInst(input);
  insert(MI);
  return MI;
}

ImplicitMovInst *IRBuilder::createImplicitMovInst(Value *input) {
  auto IMI = new ImplicitMovInst(input);
  insert(IMI);
  return IMI;
}

CoerceThisNSInst *IRBuilder::createCoerceThisNSInst(Value *input) {
  auto *inst = new CoerceThisNSInst(input);
  insert(inst);
  return inst;
}

DebuggerInst *IRBuilder::createDebuggerInst() {
  auto DI = new DebuggerInst();
  insert(DI);
  return DI;
}

SaveAndYieldInst *IRBuilder::createSaveAndYieldInst(
    Value *result,
    BasicBlock *nextBlock) {
  auto *I = new SaveAndYieldInst(result, nextBlock);
  insert(I);
  return I;
}

CreateGeneratorInst *IRBuilder::createCreateGeneratorInst(Function *innerFn) {
  auto *I = new CreateGeneratorInst(innerFn);
  insert(I);
  return I;
}

StartGeneratorInst *IRBuilder::createStartGeneratorInst() {
  auto *I = new StartGeneratorInst();
  insert(I);
  return I;
}

ResumeGeneratorInst *IRBuilder::createResumeGeneratorInst(Value *isReturn) {
  auto *I = new ResumeGeneratorInst(isReturn);
  insert(I);
  return I;
}

HBCResolveEnvironment *IRBuilder::createHBCResolveEnvironment(
    VariableScope *scope) {
  auto RSC = new HBCResolveEnvironment(scope);
  insert(RSC);
  return RSC;
}

HBCStoreToEnvironmentInst *IRBuilder::createHBCStoreToEnvironmentInst(
    Value *env,
    Value *toPut,
    Variable *var) {
  auto PSI = new HBCStoreToEnvironmentInst(env, toPut, var);
  insert(PSI);
  return PSI;
}

HBCLoadFromEnvironmentInst *IRBuilder::createHBCLoadFromEnvironmentInst(
    Value *env,
    Variable *var) {
  auto GSI = new HBCLoadFromEnvironmentInst(env, var);
  insert(GSI);
  return GSI;
}

SwitchImmInst *IRBuilder::createSwitchImmInst(
    Value *input,
    BasicBlock *defaultBlock,
    LiteralNumber *minValue,
    LiteralNumber *size,
    const SwitchImmInst::ValueListType &values,
    const SwitchImmInst::BasicBlockListType &blocks) {
  auto inst =
      new SwitchImmInst(input, defaultBlock, minValue, size, values, blocks);
  insert(inst);
  return inst;
}

DirectEvalInst *IRBuilder::createDirectEvalInst(Value *operand) {
  auto *inst = new DirectEvalInst(operand);
  insert(inst);
  return inst;
}

HBCLoadConstInst *IRBuilder::createHBCLoadConstInst(Literal *value) {
  auto inst = new HBCLoadConstInst(value);
  insert(inst);
  return inst;
}

HBCLoadParamInst *IRBuilder::createHBCLoadParamInst(LiteralNumber *value) {
  auto inst = new HBCLoadParamInst(value);
  insert(inst);
  return inst;
}

HBCCreateEnvironmentInst *IRBuilder::createHBCCreateEnvironmentInst() {
  auto inst = new HBCCreateEnvironmentInst();
  insert(inst);
  return inst;
}

HBCGetThisNSInst *IRBuilder::createHBCGetThisNSInst() {
  auto inst = new HBCGetThisNSInst();
  insert(inst);
  return inst;
}
HBCGetArgumentsPropByValInst *IRBuilder::createHBCGetArgumentsPropByValInst(
    Value *index,
    AllocStackInst *lazyReg) {
  auto inst = new HBCGetArgumentsPropByValInst(index, lazyReg);
  insert(inst);
  return inst;
}
HBCGetArgumentsLengthInst *IRBuilder::createHBCGetArgumentsLengthInst(
    AllocStackInst *lazyReg) {
  auto inst = new HBCGetArgumentsLengthInst(lazyReg);
  insert(inst);
  return inst;
}
HBCReifyArgumentsInst *IRBuilder::createHBCReifyArgumentsInst(
    AllocStackInst *lazyReg) {
  auto inst = new HBCReifyArgumentsInst(lazyReg);
  insert(inst);
  return inst;
}
HBCCreateThisInst *IRBuilder::createHBCCreateThisInst(
    Value *prototype,
    Value *closure) {
  auto inst = new HBCCreateThisInst(prototype, closure);
  insert(inst);
  return inst;
}
HBCConstructInst *IRBuilder::createHBCConstructInst(
    Value *closure,
    Value *thisValue,
    ArrayRef<Value *> arguments) {
  auto inst = new HBCConstructInst(closure, thisValue, arguments);
  insert(inst);
  return inst;
}
HBCGetConstructedObjectInst *IRBuilder::createHBCGetConstructedObjectInst(
    HBCCreateThisInst *thisValue,
    HBCConstructInst *constructorReturnValue) {
  auto inst =
      new HBCGetConstructedObjectInst(thisValue, constructorReturnValue);
  insert(inst);
  return inst;
}

HBCProfilePointInst *IRBuilder::createHBCProfilePointInst(uint16_t pointIndex) {
  auto inst = new HBCProfilePointInst(pointIndex);
  insert(inst);
  return inst;
}

CallBuiltinInst *IRBuilder::createCallBuiltinInst(
    BuiltinMethod::Enum builtinIndex,
    ArrayRef<Value *> arguments) {
  auto *inst = new CallBuiltinInst(
      getLiteralNumber(builtinIndex), getLiteralUndefined(), arguments);
  insert(inst);
  return inst;
}

GetBuiltinClosureInst *IRBuilder::createGetBuiltinClosureInst(
    BuiltinMethod::Enum builtinIndex) {
  auto *inst = new GetBuiltinClosureInst(getLiteralNumber(builtinIndex));
  insert(inst);
  return inst;
}

#ifdef HERMES_RUN_WASM
CallIntrinsicInst *IRBuilder::createCallIntrinsicInst(
    WasmIntrinsics::Enum intrinsicsIndex,
    ArrayRef<Value *> arguments) {
  auto *inst =
      new CallIntrinsicInst(getLiteralNumber(intrinsicsIndex), arguments);
  insert(inst);
  return inst;
}
#endif

HBCCallDirectInst *IRBuilder::createHBCCallDirectInst(
    Function *callee,
    Value *thisValue,
    ArrayRef<Value *> arguments) {
  auto *inst = new HBCCallDirectInst(callee, thisValue, arguments);
  insert(inst);
  return inst;
}

HBCCreateFunctionInst *IRBuilder::createHBCCreateFunctionInst(
    Function *function,
    Value *env) {
  auto inst = new HBCCreateFunctionInst(function, env);
  insert(inst);
  return inst;
}

HBCSpillMovInst *IRBuilder::createHBCSpillMovInst(Instruction *value) {
  auto *inst = new HBCSpillMovInst(value);
  insert(inst);
  return inst;
}

HBCCreateGeneratorInst *IRBuilder::createHBCCreateGeneratorInst(
    Function *function,
    Value *env) {
  auto *inst = new HBCCreateGeneratorInst(function, env);
  insert(inst);
  return inst;
}

HBCAllocObjectFromBufferInst *IRBuilder::createHBCAllocObjectFromBufferInst(
    HBCAllocObjectFromBufferInst::ObjectPropertyMap prop_map,
    uint32_t size) {
  auto *inst =
      new HBCAllocObjectFromBufferInst(M->getLiteralNumber(size), prop_map);
  insert(inst);
  return inst;
}

AllocObjectLiteralInst *IRBuilder::createAllocObjectLiteralInst(
    const AllocObjectLiteralInst::ObjectPropertyMap &propMap) {
  auto *inst = new AllocObjectLiteralInst(propMap);
  insert(inst);
  return inst;
}

CompareBranchInst *IRBuilder::createCompareBranchInst(
    Value *left,
    Value *right,
    BinaryOperatorInst::OpKind opKind,
    BasicBlock *trueBlock,
    BasicBlock *falseBlock) {
  auto *inst =
      new CompareBranchInst(left, right, opKind, trueBlock, falseBlock);
  insert(inst);
  return inst;
}

IteratorBeginInst *IRBuilder::createIteratorBeginInst(
    AllocStackInst *sourceOrNext) {
  auto *I = new IteratorBeginInst(sourceOrNext);
  insert(I);
  return I;
}

IteratorNextInst *IRBuilder::createIteratorNextInst(
    AllocStackInst *iterator,
    AllocStackInst *sourceOrNext) {
  auto *I = new IteratorNextInst(iterator, sourceOrNext);
  insert(I);
  return I;
}

IteratorCloseInst *IRBuilder::createIteratorCloseInst(
    AllocStackInst *iterator,
    bool ignoreInnerException) {
  auto *I =
      new IteratorCloseInst(iterator, getLiteralBool(ignoreInnerException));
  insert(I);
  return I;
}

UnreachableInst *IRBuilder::createUnreachableInst() {
  auto *I = new UnreachableInst();
  insert(I);
  return I;
}

inline void IRBuilder::justInsert(Instruction *Inst) {
  assert(!Inst->getParent() && "Instr that's already inserted elsewhere");
  Inst->setParent(Block);
  Block->getInstList().insert(InsertionPoint, Inst);
}

void IRBuilder::insert(Instruction *Inst) {
  // Set the statement of the new instruction based on the current function's
  // statement counter.
  OptValue<uint32_t> statementOpt = getFunction()->getStatementCount();
  uint32_t statement;
  if (LLVM_LIKELY(statementOpt.hasValue())) {
    statement = *statementOpt;
  } else {
    // If we've cleared the statement count,
    // then just set the new instruction's statement to the statement of the
    // instruction we're inserting at (0 if it doesn't exist).
    statement = InsertionPoint != Block->getInstList().end()
        ? InsertionPoint->getStatementIndex()
        : 0;
  }
  Inst->setStatementIndex(statement);

  Inst->setLocation(Location);

  return justInsert(Inst);
}

void IRBuilder::setInsertionBlock(BasicBlock *BB) {
  if (BB) {
    InsertionPoint = BB->end();
    Block = BB;
  } else {
    InsertionPoint = BasicBlock::iterator(nullptr);
    Block = nullptr;
  }
}

BasicBlock *IRBuilder::getInsertionBlock() {
  return Block;
}

void IRBuilder::setInsertionPointAfter(Instruction *IP) {
  InsertionPoint = IP->getIterator();
  InsertionPoint++;
  Block = IP->getParent();
}

void IRBuilder::setInsertionPoint(Instruction *IP) {
  InsertionPoint = IP->getIterator();
  Block = IP->getParent();
}

bool IRBuilder::isInsertionPointValid() {
  return InsertionPoint.getNodePtr();
}

void IRBuilder::resetInsertionPoint() {
  InsertionPoint = BasicBlock::iterator();
  Block = nullptr;
}

void IRBuilder::transferInstructionToCurrentBlock(Instruction *inst) {
  auto *oldBlock = inst->getParent();
  inst->removeFromParent();
  inst->setParent(Block);
  Block->getInstList().insert(InsertionPoint, inst);

  if (oldBlock == Block || !llvh::isa<TerminatorInst>(inst))
    return;

  // If we moved the terminator, we want to update all successors' phi-nodes.
  for (auto *use : oldBlock->getUsers()) {
    auto *phi = llvh::dyn_cast<PhiInst>(use);
    if (!phi)
      continue;
    for (unsigned i = 0, e = phi->getNumOperands(); i != e; ++i) {
      if (phi->getOperand(i) == oldBlock)
        phi->setOperand(Block, i);
    }
  }
}

Instruction *IRBuilder::cloneInst(
    const Instruction *source,
    llvh::ArrayRef<Value *> operands) {
  Instruction *inst;
  switch (source->getKind()) {
#define INCLUDE_ALL_INSTRS
#define DEF_VALUE(name, parent)                    \
  case ValueKind::name##Kind:                      \
    inst = new name(cast<name>(source), operands); \
    break;

#include "hermes/IR/Instrs.def"
#undef INCLUDE_ALL_INSTRS
    default:
      llvm_unreachable("invalid kind");
  }

  justInsert(inst);
  return inst;
}
