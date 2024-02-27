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

NormalFunction *IRBuilder::createFunction(
    Identifier OriginalName,
    Function::DefinitionKind definitionKind,
    bool strictMode,
    CustomDirectives customDirectives,
    SMRange sourceRange,
    Function *insertBefore) {
  // Function must have a name. If the source doesn't provide the function name,
  // Hermes will try to infer the name from the name of a variable or property
  // it is assigned to. If there was no inference, an empty string is used.
  if (!OriginalName.isValid()) {
    OriginalName = createIdentifier("");
  }
  return new NormalFunction(
      M,
      OriginalName,
      definitionKind,
      strictMode,
      customDirectives,
      sourceRange,
      insertBefore);
}

GeneratorFunction *IRBuilder::createGeneratorFunction(
    Identifier OriginalName,
    Function::DefinitionKind definitionKind,
    bool strictMode,
    CustomDirectives customDirectives,
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
      customDirectives,
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
      M, OriginalName, definitionKind, strictMode, sourceRange, insertBefore);
}

ExternalScope *IRBuilder::createExternalScope(
    Function *function,
    int32_t depth) {
  return new ExternalScope(function, depth);
}

Function *IRBuilder::createTopLevelFunction(
    bool strictMode,
    CustomDirectives customDirectives,
    SMRange sourceRange) {
  // Notice that this synthesized name is not a legal javascript name and
  // can't collide with functions in the processed program.
  auto *F = createFunction(
      "global",
      Function::DefinitionKind::ES5Function,
      strictMode,
      customDirectives,
      sourceRange);
  M->setTopLevelFunction(F);
  return F;
}

NormalFunction *IRBuilder::createFunction(
    llvh::StringRef OriginalName,
    Function::DefinitionKind definitionKind,
    bool strictMode,
    CustomDirectives customDirectives,
    SMRange sourceRange,
    Function *insertBefore) {
  Identifier OrigIden =
      OriginalName.empty() ? Identifier{} : createIdentifier(OriginalName);
  return createFunction(
      OrigIden,
      definitionKind,
      strictMode,
      customDirectives,
      sourceRange,
      insertBefore);
}

AsyncFunction *IRBuilder::createAsyncFunction(
    Identifier OriginalName,
    Function::DefinitionKind definitionKind,
    bool strictMode,
    CustomDirectives customDirectives,
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
      customDirectives,
      sourceRange,
      insertBefore);
}

GlobalObjectProperty *IRBuilder::createGlobalObjectProperty(
    Identifier name,
    bool declared) {
  return M->addGlobalProperty(name, declared);
}
GlobalObjectProperty *IRBuilder::createGlobalObjectProperty(
    llvh::StringRef name,
    bool declared) {
  return createGlobalObjectProperty(
      M->getContext().getIdentifier(name), declared);
}

/// Add a new JS parameter to function \p Parent.
JSDynamicParam *IRBuilder::createJSDynamicParam(
    Function *parent,
    Identifier name) {
  return parent->addJSDynamicParam(name);
}

/// Add a new JS parameter to function \p Parent.
JSDynamicParam *IRBuilder::createJSDynamicParam(
    Function *parent,
    llvh::StringRef name) {
  return parent->addJSDynamicParam(createIdentifier(name));
}

Variable *
IRBuilder::createVariable(VariableScope *Parent, Identifier Name, Type type) {
  auto *var = new Variable(Parent, Name);
  var->setType(type);
  return var;
}

Variable *IRBuilder::createVariable(
    VariableScope *Parent,
    const llvh::Twine &Name,
    Type type) {
  return createVariable(Parent, createIdentifier(Name), type);
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

LiteralNumber *IRBuilder::getLiteralInfinity() {
  return M->getLiteralNumber(std::numeric_limits<double>::infinity());
}

LiteralNumber *IRBuilder::getLiteralNaN() {
  return M->getLiteralNumber(std::numeric_limits<double>::quiet_NaN());
}

LiteralBigInt *IRBuilder::getLiteralBigInt(UniqueString *value) {
  return M->getLiteralBigInt(value);
}

LiteralString *IRBuilder::getLiteralString(const llvh::Twine &value) {
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

Identifier IRBuilder::createIdentifier(const llvh::Twine &str) {
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

ThrowTypeErrorInst *IRBuilder::createThrowTypeErrorInst(Value *message) {
  auto *TI = new ThrowTypeErrorInst(message);
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

AllocStackInst *IRBuilder::createAllocStackInst(
    const llvh::Twine &varName,
    Type type) {
  Identifier Iden = createIdentifier(varName);
  return createAllocStackInst(Iden, type);
}

AllocStackInst *IRBuilder::createAllocStackInst(Identifier varName, Type type) {
  auto *AHI = new AllocStackInst(varName);
  AHI->setType(type);
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

GetParentScopeInst *IRBuilder::createGetParentScopeInst(
    VariableScope *scope,
    JSDynamicParam *parentScopeParam) {
  auto GPS = new GetParentScopeInst(scope, parentScopeParam);
  insert(GPS);
  return GPS;
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
    Value *target,
    Value *env,
    Value *newTarget,
    Value *thisValue,
    ArrayRef<Value *> args) {
  auto CI = new CallInst(callee, target, env, newTarget, thisValue, args);
  insert(CI);
  return CI;
}

HBCCallWithArgCountInst *IRBuilder::createHBCCallWithArgCount(
    Value *callee,
    Value *target,
    Value *env,
    Value *newTarget,
    LiteralNumber *argCount,
    Value *thisValue,
    ArrayRef<Value *> args) {
  auto CI = new HBCCallWithArgCountInst(
      callee, target, env, newTarget, argCount, thisValue, args);
  insert(CI);
  return CI;
}

HBCCallNInst *IRBuilder::createHBCCallNInst(
    Value *callee,
    Value *target,
    Value *env,
    Value *newTarget,
    Value *thisValue,
    ArrayRef<Value *> args) {
  auto CI = new HBCCallNInst(callee, target, env, newTarget, thisValue, args);
  insert(CI);
  return CI;
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
  if (Block->getParent()->isStrictMode())
    return createDeletePropertyStrictInst(object, property);
  else
    return createDeletePropertyLooseInst(object, property);
}
DeletePropertyLooseInst *IRBuilder::createDeletePropertyLooseInst(
    Value *object,
    Value *property) {
  auto DPI = new DeletePropertyLooseInst(object, property);
  insert(DPI);
  return DPI;
}
DeletePropertyStrictInst *IRBuilder::createDeletePropertyStrictInst(
    Value *object,
    Value *property) {
  auto DPI = new DeletePropertyStrictInst(object, property);
  insert(DPI);
  return DPI;
}

StorePropertyInst *IRBuilder::createStorePropertyInst(
    Value *storedValue,
    Value *object,
    Value *property) {
  if (Block->getParent()->isStrictMode())
    return createStorePropertyStrictInst(storedValue, object, property);
  else
    return createStorePropertyLooseInst(storedValue, object, property);
}
StorePropertyLooseInst *IRBuilder::createStorePropertyLooseInst(
    Value *storedValue,
    Value *object,
    Value *property) {
  auto SPI = new StorePropertyLooseInst(storedValue, object, property);
  insert(SPI);
  return SPI;
}
StorePropertyStrictInst *IRBuilder::createStorePropertyStrictInst(
    Value *storedValue,
    Value *object,
    Value *property) {
  auto SPI = new StorePropertyStrictInst(storedValue, object, property);
  insert(SPI);
  return SPI;
}

TryStoreGlobalPropertyInst *IRBuilder::createTryStoreGlobalPropertyInst(
    Value *storedValue,
    LiteralString *property) {
  if (Block->getParent()->isStrictMode())
    return createTryStoreGlobalPropertyStrictInst(storedValue, property);
  else
    return createTryStoreGlobalPropertyLooseInst(storedValue, property);
}
TryStoreGlobalPropertyLooseInst *
IRBuilder::createTryStoreGlobalPropertyLooseInst(
    Value *storedValue,
    LiteralString *property) {
  auto *inst = new TryStoreGlobalPropertyLooseInst(
      storedValue, getGlobalObject(), property);
  insert(inst);
  return inst;
}
TryStoreGlobalPropertyStrictInst *
IRBuilder::createTryStoreGlobalPropertyStrictInst(
    Value *storedValue,
    LiteralString *property) {
  auto *inst = new TryStoreGlobalPropertyStrictInst(
      storedValue, getGlobalObject(), property);
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
    llvh::StringRef property) {
  Identifier Iden = createIdentifier(property);
  return createDeletePropertyInst(object, Iden);
}

LoadPropertyInst *IRBuilder::createLoadPropertyInst(
    Value *object,
    llvh::StringRef property) {
  Identifier Iden = createIdentifier(property);
  return createLoadPropertyInst(object, Iden);
}

TryLoadGlobalPropertyInst *IRBuilder::createTryLoadGlobalPropertyInst(
    llvh::StringRef property) {
  return createTryLoadGlobalPropertyInst(createIdentifier(property));
}

StorePropertyInst *IRBuilder::createStorePropertyInst(
    Value *storedValue,
    Value *object,
    llvh::StringRef property) {
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
  return createTryStoreGlobalPropertyInst(
      storedValue, getLiteralString(property));
}

AllocObjectInst *IRBuilder::createAllocObjectInst(
    uint32_t size,
    Value *parent) {
  auto AOI = new AllocObjectInst(
      M->getLiteralNumber(size), parent ? parent : getEmptySentinel());
  insert(AOI);
  return AOI;
}

AllocFastArrayInst *IRBuilder::createAllocFastArrayInst(
    LiteralNumber *sizeHint) {
  auto A = new AllocFastArrayInst(sizeHint);
  insert(A);
  return A;
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

GetTemplateObjectInst *IRBuilder::createGetTemplateObjectInst(
    uint32_t templateObjID,
    bool dup,
    llvh::ArrayRef<LiteralString *> rawStrings,
    llvh::ArrayRef<Value *> cookedStrings) {
  auto *inst = new GetTemplateObjectInst(
      getLiteralNumber(templateObjID),
      getLiteralBool(dup),
      rawStrings,
      cookedStrings);
  insert(inst);
  return inst;
}

CreateArgumentsLooseInst *IRBuilder::createCreateArgumentsLooseInst() {
  auto *CAI = new CreateArgumentsLooseInst();
  insert(CAI);
  return CAI;
}
CreateArgumentsStrictInst *IRBuilder::createCreateArgumentsStrictInst() {
  auto *CAI = new CreateArgumentsStrictInst();
  insert(CAI);
  return CAI;
}

GetNewTargetInst *IRBuilder::createGetNewTargetInst(Value *newTargetParam) {
  auto *inst = new GetNewTargetInst(newTargetParam);
  insert(inst);
  return inst;
}

ThrowIfInst *IRBuilder::createThrowIfInst(
    Value *checkedValue,
    Type invalidTypes) {
  auto *inst = new ThrowIfInst(checkedValue, getLiteralIRType(invalidTypes));
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

UnaryOperatorInst *
IRBuilder::createUnaryOperatorInst(Value *value, ValueKind kind, Type type) {
  auto UOI = new UnaryOperatorInst(kind, value, type);
  insert(UOI);
  return UOI;
}

BinaryOperatorInst *
IRBuilder::createBinaryOperatorInst(Value *left, Value *right, ValueKind kind) {
  auto BOI = new BinaryOperatorInst(kind, left, right);
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
    AllocStackInst *iteratorAddr,
    AllocStackInst *baseAddr,
    AllocStackInst *indexAddr,
    AllocStackInst *sizeAddr,
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
    AllocStackInst *propertyAddr,
    AllocStackInst *baseAddr,
    AllocStackInst *indexAddr,
    AllocStackInst *sizeAddr,
    AllocStackInst *iteratorAddr,
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

CreateGeneratorInst *IRBuilder::createCreateGeneratorInst(
    GeneratorInnerFunction *innerFn) {
  auto *I = new CreateGeneratorInst(innerFn);
  insert(I);
  return I;
}

StartGeneratorInst *IRBuilder::createStartGeneratorInst() {
  auto *I = new StartGeneratorInst();
  insert(I);
  return I;
}

ResumeGeneratorInst *IRBuilder::createResumeGeneratorInst(
    AllocStackInst *isReturn) {
  auto *I = new ResumeGeneratorInst(isReturn);
  insert(I);
  return I;
}

HBCResolveParentEnvironmentInst *
IRBuilder::createHBCResolveParentEnvironmentInst(
    VariableScope *scope,
    LiteralNumber *numLevels,
    JSDynamicParam *parentScopeParam) {
  auto *inst =
      new HBCResolveParentEnvironmentInst(scope, numLevels, parentScopeParam);
  insert(inst);
  return inst;
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

DirectEvalInst *IRBuilder::createDirectEvalInst(
    Value *evalText,
    bool strictCaller) {
  auto *inst = new DirectEvalInst(evalText, getLiteralBool(strictCaller));
  insert(inst);
  return inst;
}

DeclareGlobalVarInst *IRBuilder::createDeclareGlobalVarInst(
    LiteralString *name) {
  auto *inst = new DeclareGlobalVarInst(name);
  insert(inst);
  return inst;
}

HBCLoadConstInst *IRBuilder::createHBCLoadConstInst(Literal *value) {
  auto inst = new HBCLoadConstInst(value);
  insert(inst);
  return inst;
}

LoadParamInst *IRBuilder::createLoadParamInst(JSDynamicParam *param) {
  auto inst = new LoadParamInst(param);
  insert(inst);
  return inst;
}

HBCCreateFunctionEnvironmentInst *
IRBuilder::createHBCCreateFunctionEnvironmentInst(
    VariableScope *scope,
    JSDynamicParam *parentScopeParam) {
  auto *inst = new HBCCreateFunctionEnvironmentInst(scope, parentScopeParam);
  insert(inst);
  return inst;
}

LIRGetThisNSInst *IRBuilder::createLIRGetThisNSInst() {
  auto inst = new LIRGetThisNSInst();
  insert(inst);
  return inst;
}
HBCGetArgumentsPropByValLooseInst *
IRBuilder::createHBCGetArgumentsPropByValLooseInst(
    Value *index,
    AllocStackInst *lazyReg) {
  auto inst = new HBCGetArgumentsPropByValLooseInst(index, lazyReg);
  insert(inst);
  return inst;
}
HBCGetArgumentsPropByValStrictInst *
IRBuilder::createHBCGetArgumentsPropByValStrictInst(
    Value *index,
    AllocStackInst *lazyReg) {
  auto inst = new HBCGetArgumentsPropByValStrictInst(index, lazyReg);
  insert(inst);
  return inst;
}
HBCGetArgumentsLengthInst *IRBuilder::createHBCGetArgumentsLengthInst(
    Value *lazyRegValue) {
  auto inst = new HBCGetArgumentsLengthInst(lazyRegValue);
  insert(inst);
  return inst;
}
HBCReifyArgumentsLooseInst *IRBuilder::createHBCReifyArgumentsLooseInst(
    AllocStackInst *lazyReg) {
  auto inst = new HBCReifyArgumentsLooseInst(lazyReg);
  insert(inst);
  return inst;
}
HBCReifyArgumentsStrictInst *IRBuilder::createHBCReifyArgumentsStrictInst(
    AllocStackInst *lazyReg) {
  auto inst = new HBCReifyArgumentsStrictInst(lazyReg);
  insert(inst);
  return inst;
}
CreateThisInst *IRBuilder::createCreateThisInst(
    Value *prototype,
    Value *closure) {
  auto inst = new CreateThisInst(prototype, closure);
  insert(inst);
  return inst;
}
GetConstructedObjectInst *IRBuilder::createGetConstructedObjectInst(
    CreateThisInst *thisValue,
    CallInst *constructorReturnValue) {
  auto inst = new GetConstructedObjectInst(thisValue, constructorReturnValue);
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
      getLiteralNumber(builtinIndex),
      getEmptySentinel(),
      getEmptySentinel(),
      getLiteralUndefined(),
      arguments);
  insert(inst);
  return inst;
}

GetBuiltinClosureInst *IRBuilder::createGetBuiltinClosureInst(
    BuiltinMethod::Enum builtinIndex) {
  auto *inst = new GetBuiltinClosureInst(getLiteralNumber(builtinIndex));
  insert(inst);
  return inst;
}

HBCCreateFunctionInst *IRBuilder::createHBCCreateFunctionInst(
    Function *function,
    BaseScopeInst *env) {
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
    BaseScopeInst *env) {
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
    ValueKind kind,
    BasicBlock *trueBlock,
    BasicBlock *falseBlock) {
  auto *inst = new CompareBranchInst(kind, left, right, trueBlock, falseBlock);
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
    Value *sourceOrNext) {
  auto *I = new IteratorNextInst(iterator, sourceOrNext);
  insert(I);
  return I;
}

IteratorCloseInst *IRBuilder::createIteratorCloseInst(
    Value *iterator,
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

PrLoadInst *IRBuilder::createPrLoadInst(
    Value *object,
    size_t propIndex,
    LiteralString *propName,
    Type checkedType) {
  auto *I = new PrLoadInst(
      object, getLiteralNumber((double)propIndex), propName, checkedType);
  insert(I);
  return I;
}

PrStoreInst *IRBuilder::createPrStoreInst(
    Value *storedValue,
    Value *object,
    size_t propIndex,
    LiteralString *propName,
    bool nonPointer) {
  auto *I = new PrStoreInst(
      storedValue,
      object,
      getLiteralNumber((double)propIndex),
      propName,
      getLiteralBool(nonPointer));
  insert(I);
  return I;
}

FastArrayLoadInst *IRBuilder::createFastArrayLoadInst(
    Value *array,
    Value *index,
    Type checkedType) {
  auto *I = new FastArrayLoadInst(array, index, checkedType);
  insert(I);
  return I;
}

FastArrayStoreInst *IRBuilder::createFastArrayStoreInst(
    Value *storedValue,
    Value *array,
    Value *index) {
  auto *I = new FastArrayStoreInst(storedValue, array, index);
  insert(I);
  return I;
}

FastArrayPushInst *IRBuilder::createFastArrayPushInst(
    Value *pushedValue,
    Value *array) {
  auto *I = new FastArrayPushInst(pushedValue, array);
  insert(I);
  return I;
}
FastArrayAppendInst *IRBuilder::createFastArrayAppendInst(
    Value *other,
    Value *array) {
  auto *I = new FastArrayAppendInst(other, array);
  insert(I);
  return I;
}

FastArrayLengthInst *IRBuilder::createFastArrayLengthInst(Value *array) {
  auto *I = new FastArrayLengthInst(array);
  insert(I);
  return I;
}

LoadParentInst *IRBuilder::createLoadParentInst(Value *object) {
  auto *inst = new LoadParentInst(object);
  insert(inst);
  return inst;
}

StoreParentInst *IRBuilder::createStoreParentInst(
    Value *storedValue,
    Value *object) {
  auto *inst = new StoreParentInst(storedValue, object);
  insert(inst);
  return inst;
}

FUnaryMathInst *IRBuilder::createFUnaryMathInst(ValueKind kind, Value *arg) {
  auto *inst = new FUnaryMathInst(kind, arg);
  insert(inst);
  return inst;
}
FBinaryMathInst *
IRBuilder::createFBinaryMathInst(ValueKind kind, Value *left, Value *right) {
  auto *inst = new FBinaryMathInst(kind, left, right);
  insert(inst);
  return inst;
}

FCompareInst *
IRBuilder::createFCompareInst(ValueKind kind, Value *left, Value *right) {
  auto *inst = new FCompareInst(kind, left, right);
  insert(inst);
  return inst;
}

StringConcatInst *IRBuilder::createStringConcatInst(
    llvh::ArrayRef<Value *> operands) {
  auto *inst = new StringConcatInst(operands);
  insert(inst);
  return inst;
}

UnionNarrowTrustedInst *IRBuilder::createUnionNarrowTrustedInst(
    Value *value,
    Type type) {
  auto *inst = new UnionNarrowTrustedInst(value, type);
  insert(inst);
  return inst;
}

CheckedTypeCastInst *IRBuilder::createCheckedTypeCastInst(
    Value *value,
    Type type) {
  auto *inst = new CheckedTypeCastInst(value, getLiteralIRType(type));
  insert(inst);
  return inst;
}

LIRDeadValueInst *IRBuilder::createLIRDeadValueInst(Type type) {
  auto *inst = new LIRDeadValueInst(type);
  insert(inst);
  return inst;
}

LiteralNativeSignature *IRBuilder::getLiteralNativeSignature(
    NativeSignature *sig) {
  return M->getLiteralNativeSignature(sig);
}
LiteralNativeExtern *IRBuilder::getLiteralNativeExtern(
    NativeExtern *nativeExtern) {
  return M->getLiteralNativeExtern(nativeExtern);
}

NativeCallInst *IRBuilder::createNativeCallInst(
    Type type,
    Value *callee,
    NativeSignature *sig,
    ArrayRef<Value *> args) {
  auto *inst =
      new NativeCallInst(type, callee, getLiteralNativeSignature(sig), args);
  insert(inst);
  return inst;
}

GetNativeRuntimeInst *IRBuilder::createGetNativeRuntimeInst() {
  auto *inst = new GetNativeRuntimeInst();
  insert(inst);
  return inst;
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
#define DEF_TAG(name, parent)                          \
  case ValueKind::name##Kind:                          \
    inst = new parent(cast<parent>(source), operands); \
    break;

#include "hermes/IR/Instrs.def"
#undef INCLUDE_ALL_INSTRS
    default:
      llvm_unreachable("invalid kind");
  }

  justInsert(inst);
  return inst;
}
