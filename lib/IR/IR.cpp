/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "llvh/ADT/SetVector.h"
#include "llvh/ADT/SmallString.h"
#include "llvh/Support/Casting.h"
#include "llvh/Support/ErrorHandling.h"
#include "llvh/Support/raw_ostream.h"

#include "hermes/AST/Context.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Utils/Dumper.h"

#include <set>
#include <type_traits>

#define INCLUDE_ALL_INSTRS

using namespace hermes;

// Make sure the ValueKinds.def tree is consistent with the class hierarchy.
#define QUOTE(X) #X
#define DEF_VALUE(CLASS, PARENT)                                           \
  static_assert(                                                           \
      std::is_base_of<PARENT, CLASS>::value,                               \
      QUOTE(CLASS) " should directly inherit from " QUOTE(PARENT));        \
  static_assert(                                                           \
      std::is_convertible<CLASS *, PARENT *>::value,                       \
      QUOTE(CLASS) " should publicly inherit from " QUOTE(PARENT));        \
  static_assert(                                                           \
      ValueKind::CLASS##Kind > ValueKind::First_##PARENT##Kind,            \
      QUOTE(CLASS) "Kind should be after First_" QUOTE(PARENT) "Kind");    \
  static_assert(                                                           \
      ValueKind::CLASS##Kind < ValueKind::Last_##PARENT##Kind,             \
      QUOTE(CLASS) "Kind should be before Last_" QUOTE(PARENT) "Kind");    \
  static_assert(                                                           \
      ValueKind::PARENT##Kind ==                                           \
          static_cast<ValueKind>(                                          \
              static_cast<unsigned>(ValueKind::First_##PARENT##Kind) + 1), \
      QUOTE(PARENT) "Kind should be right after First_" QUOTE(PARENT) "Kind");
#include "hermes/IR/ValueKinds.def"
#undef QUOTE

void Value::destroy(Value *V) {
  if (!V)
    return;

  switch (V->Kind) {
    default:
      llvm_unreachable("Invalid kind");
#define DEF_VALUE(XX, PARENT) \
  case ValueKind::XX##Kind:   \
    delete cast<XX>(V);       \
    break;
#include "hermes/IR/ValueKinds.def"
  }
}

StringRef Value::getKindStr() const {
  switch (Kind) {
    default:
      llvm_unreachable("Invalid kind");
#define DEF_VALUE(XX, PARENT) \
  case ValueKind::XX##Kind:   \
    return StringRef(#XX);
#include "hermes/IR/ValueKinds.def"
  }
}

const Value::UseListTy &Value::getUsers() const {
  return Users;
}

unsigned Value::getNumUsers() const {
  return Users.size();
}

bool Value::hasUsers() const {
  return Users.size();
}

bool Value::hasOneUser() const {
  return 1 == Users.size();
}

void Value::removeUse(Use U) {
  assert(Users.size() && "Removing a user from an empty list");
  assert(U.first == this && "Invalid user");

  // We don't care about the order of the operands in the use vector. One cheap
  // way to delete an element is to pop the last element and save it on top of
  // the element that we want to remove. This is faster than moving the whole
  // array.
  Users[U.second] = Users.back();
  Users.pop_back();

  // If we've changed the location of a use in the use list then we need to
  // update the operand in the user.
  if (U.second != Users.size()) {
    Use oldUse = {this, Users.size()};
    auto &operands = Users[U.second]->Operands;
    for (int i = 0, e = operands.size(); i < e; i++) {
      if (operands[i] == oldUse) {
        operands[i] = {this, U.second};
        return;
      }
    }
    llvm_unreachable("Can't find user in operand list");
  }
}

Value::Use Value::addUser(Instruction *Inst) {
  Users.push_back(Inst);
  return {this, Users.size() - 1};
}

void Value::replaceAllUsesWith(Value *Other) {
  if (this == Other) {
    return;
  }

  // Ask the users of this value to unregister themselves. Notice that the users
  // modify and invalidate the iterators of Users.
  while (Users.size()) {
    Users[Users.size() - 1]->replaceFirstOperandWith(this, Other);
  }
}

void Value::removeAllUses() {
  // Ask the users of this value to delete operands of this value. Notice that
  // the users modify and invalidate the iterators of Users.
  while (Users.size()) {
    Users[Users.size() - 1]->eraseOperand(this);
  }
}

bool Value::hasUser(Value *other) {
  return std::find(Users.begin(), Users.end(), other) != Users.end();
}

bool VariableScope::isGlobalScope() const {
  return function_->isGlobalScope() && function_->getFunctionScope() == this;
}

ExternalScope::ExternalScope(Function *function, int32_t depth)
    : VariableScope(ValueKind::ExternalScopeKind, function), depth_(depth) {
  function->addExternalScope(this);
}

Function::Function(
    ValueKind kind,
    Module *parent,
    Identifier originalName,
    DefinitionKind definitionKind,
    bool strictMode,
    SourceVisibility sourceVisibility,
    bool isGlobal,
    SMRange sourceRange,
    Function *insertBefore)
    : Value(kind),
      parent_(parent),
      isGlobal_(isGlobal),
      externalScopes_(),
      functionScope_(this),
      originalOrInferredName_(originalName),
      definitionKind_(definitionKind),
      strictMode_(strictMode),
      SourceRange(sourceRange),
      sourceVisibility_(sourceVisibility),
      internalName_(parent->deriveUniqueInternalName(originalName)) {
  if (insertBefore) {
    assert(insertBefore != this && "Cannot insert a function before itself!");
    assert(
        insertBefore->getParent() == parent &&
        "Function to insert before is from a different module!");
    parent->insert(insertBefore->getIterator(), this);
  } else {
    parent->push_back(this);
  }

  // Derive internalName from originalName.
  assert(originalName.isValid() && "Function originalName must be valid");
}

Function::~Function() {
  // Free all parameters.
  for (auto *p : Parameters) {
    Value::destroy(p);
  }
  Value::destroy(thisParameter);

  // Free all external scopes.
  for (auto *ES : externalScopes_)
    Value::destroy(ES);
}

std::string Function::getDefinitionKindStr(bool isDescriptive) const {
  switch (definitionKind_) {
    case Function::DefinitionKind::ES5Function:
      return "function";
    case Function::DefinitionKind::ES6Constructor:
      return "constructor";
    case Function::DefinitionKind::ES6Arrow:
      return isDescriptive ? "arrow function" : "arrow";
    case Function::DefinitionKind::ES6Method:
      return "method";
  }
  assert(false && "Invalid DefinitionKind");
  return "function";
}

std::string Function::getDescriptiveDefinitionKindStr() const {
  return (isAnonymous() ? "anonymous " : "") + getDefinitionKindStr(true);
}

llvh::Optional<llvh::StringRef> Function::getSourceRepresentationStr() const {
  switch (getSourceVisibility()) {
    case SourceVisibility::ShowSource: {
      // Return the actual source code string.
      auto sourceRange = getSourceRange();
      assert(
          sourceRange.isValid() && "Function does not have source available");
      const auto sourceStart = sourceRange.Start.getPointer();
      llvh::StringRef strBuf{
          sourceStart, (size_t)(sourceRange.End.getPointer() - sourceStart)};
      return strBuf;
    }
    case SourceVisibility::HideSource:
    case SourceVisibility::Sensitive: {
      // For implementation-hiding functions, the spec requires the source code
      // representation of them "must have the syntax of a NativeFunction".
      //
      // Naively adding 'function <name>() { [ native code ] }' to string table
      // for each function would be a huge waste of space in the generated hbc.
      // Instead, we associate all such functions with an empty string to
      // effectively "mark" them. (the assumption is that an real function
      // source can't be empty). This reduced the constant factor of the space
      // cost to only 8 bytes (one FunctionSourceTable entry) per function.
      return llvh::StringRef("");
    }
    case SourceVisibility::Default:
    default: {
      // No need of special source representation for these cases, their
      // 'toString' will return `{ [ bytecode ] }` as the default.
      return llvh::None;
    }
  }
}

BasicBlock::BasicBlock(Function *parent)
    : Value(ValueKind::BasicBlockKind), Parent(parent) {
  assert(Parent && "Invalid parent function");
  Parent->addBlock(this);
}

void BasicBlock::dump() {
  IRPrinter D(getParent()->getContext(), llvh::outs());
  D.visit(*this);
}

void BasicBlock::printAsOperand(llvh::raw_ostream &OS, bool) const {
  // Use the address of the basic block when LLVM prints the CFG.
  size_t Num = (size_t)this;
  OS << "BB#" << std::to_string(Num);
}

void Instruction::dump(llvh::raw_ostream &os) {
  IRPrinter D(getParent()->getContext(), os);
  D.visit(*this);
}

Instruction::Instruction(
    const Instruction *src,
    llvh::ArrayRef<Value *> operands)
    : Instruction(src->getKind()) {
  assert(
      src->getNumOperands() == operands.size() && "invalid number of operands");

  setType(src->getType());

  location_ = src->location_;
  statementIndex_ = src->statementIndex_;

  for (auto val : operands)
    pushOperand(val);
}

void Instruction::pushOperand(Value *Val) {
  Operands.push_back({nullptr, 0});
  setOperand(Val, getNumOperands() - 1);
}

void Instruction::setOperand(Value *Val, unsigned Index) {
  assert(Index < Operands.size() && "Not all operands have been pushed!");

  Value *CurrentValue = Operands[Index].first;

  // If the operand is already set then there is nothing to do. The instruction
  // is already registered in the use-list of the value.
  if (CurrentValue == Val) {
    return;
  }

  // Remove the current instruction from the old value that we are removing.
  if (CurrentValue) {
    CurrentValue->removeUse(Operands[Index]);
  }

  // Register this instruction as a user of the new value and set the operand.
  if (Val) {
    Operands[Index] = Val->addUser(this);
  } else {
    Operands[Index] = {nullptr, 0};
  }
}

Value *Instruction::getOperand(unsigned Index) const {
  return Operands[Index].first;
}

unsigned Instruction::getNumOperands() const {
  return Operands.size();
}

void Instruction::removeOperand(unsigned index) {
  // We call to setOperand before deleting the operand because setOperand
  // un-registers the user from the user list.
  setOperand(nullptr, index);
  Operands.erase(Operands.begin() + index);
}

void Instruction::replaceFirstOperandWith(Value *OldValue, Value *NewValue) {
  for (int i = 0, e = getNumOperands(); i < e; i++) {
    if (OldValue == getOperand(i)) {
      setOperand(NewValue, i);
      return;
    }
  }
  llvm_unreachable("Can't find operand. Invalid use-def chain.");
}

void Instruction::eraseOperand(Value *Value) {
  // Overwrite all of the operands that we are removing with null. This will
  // unregister them from the use list.
  for (int i = 0, e = getNumOperands(); i < e; ++i) {
    if (getOperand(i) == Value)
      setOperand(nullptr, i);
  }

  // Now remove all null operands from the list.
  auto new_end = std::remove_if(
      Operands.begin(), Operands.end(), [](Use U) { return !U.first; });
  Operands.erase(new_end, Operands.end());

  assert(!Value->hasUser(this) && "corrupt uselist");
}

void Instruction::insertBefore(Instruction *InsertPos) {
  InsertPos->getParent()->getInstList().insert(InsertPos->getIterator(), this);
}

void Instruction::insertAfter(Instruction *InsertPos) {
  InsertPos->getParent()->getInstList().insertAfter(
      InsertPos->getIterator(), this);
}

void Instruction::moveBefore(Instruction *Later) {
  if (this == Later)
    return;

  getParent()->getInstList().remove(this);
  Later->getParent()->getInstList().insert(Later->getIterator(), this);
  setParent(Later->getParent());
}

void BasicBlock::remove(Instruction *I) {
  InstList.remove(I);
}
void BasicBlock::erase(Instruction *I) {
  InstList.erase(I);
}

void Instruction::removeFromParent() {
  getParent()->remove(this);
}
void Instruction::eraseFromParent() {
  // Release this instruction from the use-list of other instructions.
  for (unsigned i = 0; i < getNumOperands(); i++)
    setOperand(nullptr, i);

  getParent()->erase(this);
}

void Function::eraseFromParentNoDestroy() {
  // Erase all of the basic blocks before deleting the function.
  while (begin() != end()) {
    begin()->replaceAllUsesWith(nullptr);
    begin()->eraseFromParent();
  }
  assert(!hasUsers() && "Use list is not empty");
  getParent()->getFunctionList().remove(getIterator());
}

StringRef Instruction::getName() {
  switch (getKind()) {
    default:
      llvm_unreachable("Invalid kind");
#define DEF_VALUE(XX, PARENT) \
  case ValueKind::XX##Kind:   \
    return #XX;
#include "hermes/IR/Instrs.def"
  }
}

SideEffectKind Instruction::getDerivedSideEffect() {
  switch (getKind()) {
    default:
      llvm_unreachable("Invalid kind");
#define DEF_VALUE(XX, PARENT) \
  case ValueKind::XX##Kind:   \
    return cast<XX>(this)->getSideEffect();
#include "hermes/IR/Instrs.def"
  }
}

WordBitSet<> Instruction::getChangedOperands() {
  switch (getKind()) {
    default:
      llvm_unreachable("Invalid kind");

#define DEF_VALUE(XX, PARENT)                        \
  case ValueKind::XX##Kind:                          \
    return cast<XX>(this)->getChangedOperandsImpl(); \
    break;
#include "hermes/IR/Instrs.def"
  }
}

Parameter::Parameter(Function *parent, Identifier name)
    : Value(ValueKind::ParameterKind), Parent(parent), Name(name) {
  assert(Parent && "Invalid parent");
  if (name.str() == "this") {
    Parent->setThisParameter(this);
  } else {
    Parent->addParameter(this);
  }
}

Variable::Variable(
    ValueKind k,
    VariableScope *scope,
    DeclKind declKind,
    Identifier txt)
    : Value(k), declKind(declKind), text(txt), parent(scope) {
  scope->addVariable(this);
}

Variable::~Variable() {}

int Variable::getIndexInVariableList() const {
  int index = 0;
  for (auto V : parent->getVariables()) {
    if (V == this)
      return index;
    index++;
  }
  llvm_unreachable("Cannot find variable in the variable list");
}

Identifier Parameter::getName() const {
  return Name;
}

void BasicBlock::push_back(Instruction *I) {
  InstList.push_back(I);
}

TerminatorInst *BasicBlock::getTerminator() {
  if (InstList.empty())
    return nullptr;
  return llvh::dyn_cast<TerminatorInst>(&InstList.back());
}

const TerminatorInst *BasicBlock::getTerminator() const {
  if (InstList.empty())
    return nullptr;
  return llvh::dyn_cast<TerminatorInst>(&InstList.back());
}

void BasicBlock::removeFromParent() {
  getParent()->getBasicBlockList().remove(getIterator());
}

void BasicBlock::eraseFromParent() {
  // Erase all of the instructions in the block before deleting the block.
  // We are starting to delete from the start of the block. Naturally we will
  // have forward dependencies between instructions. To allow safe deletion
  // we replace all uses with the invalid null value. setOperand knows how
  // to deal with null values.
  while (begin() != end()) {
    begin()->replaceAllUsesWith(nullptr);
    begin()->eraseFromParent();
  }

  assert(!hasUsers() && "Use list is not empty");
  // Erase the block itself:
  getParent()->getBasicBlockList().erase(getIterator());
}

Context &Function::getContext() const {
  return parent_->getContext();
}

void Function::addBlock(BasicBlock *BB) {
  BasicBlockList.push_back(BB);
}
void Function::addParameter(Parameter *A) {
  Parameters.push_back(A);
}

Module::~Module() {
  FunctionList.clear();

  // Free global properties.
  globalPropertyMap_.clear();
  for (auto *prop : globalPropertyList_) {
    Value::destroy(prop);
  }

  llvh::SmallVector<Literal *, 32> toDelete;

  // Collect all literals.
  // Note that we cannot delete while iterating due to the implementation
  // of FoldingSet.
  for (auto &L : literalNumbers) {
    toDelete.push_back(&L);
  }
  for (auto &L : literalBigInts) {
    toDelete.push_back(&L);
  }
  for (auto &L : literalStrings) {
    toDelete.push_back(&L);
  }
  // Free the literals.
  for (auto *L : toDelete) {
    Value::destroy(L);
  }
}

void Module::push_back(Function *F) {
  FunctionList.push_back(F);
}

void Module::insert(iterator position, Function *F) {
  FunctionList.insert(position, F);
}

GlobalObjectProperty *Module::findGlobalProperty(Identifier name) {
  auto it = globalPropertyMap_.find(name);
  return it != globalPropertyMap_.end() ? it->second : nullptr;
}

GlobalObjectProperty *Module::addGlobalProperty(
    Identifier name,
    bool declared) {
  auto &ref = globalPropertyMap_[name];

  if (!ref) {
    ref = new GlobalObjectProperty(this, getLiteralString(name), declared);
    globalPropertyList_.push_back(ref);
  } else {
    ref->orDeclared(declared);
  }

  return ref;
}

void Module::eraseGlobalProperty(GlobalObjectProperty *prop) {
  globalPropertyMap_.erase(prop->getName()->getValue());
  auto it =
      std::find(globalPropertyList_.begin(), globalPropertyList_.end(), prop);
  if (it != globalPropertyList_.end()) {
    Value::destroy(*it);
    globalPropertyList_.erase(it);
  }
}

void Module::populateCJSModuleUseGraph() {
  if (!cjsModuleUseGraph_.empty()) {
    return;
  }

  for (Function &f : *this) {
    for (Instruction *user : f.getUsers()) {
      // Add an edge to f, from the function which uses f.
      cjsModuleUseGraph_[user->getParent()->getParent()].insert(&f);
    }
  }
}

llvh::DenseSet<Function *> Module::getFunctionsInSegment(uint32_t segment) {
  populateCJSModuleUseGraph();

  // Final set of functions which must be output when generating this segment.
  llvh::DenseSet<Function *> result{};

  // Current set of functions which we haven't inspected (the frontier).
  // Use this to perform graph search and find all used functions.
  llvh::SetVector<Function *> worklist{};

  // Populate the worklist initially with the wrapper functions for each module
  // in the given segment.
  for (Function *fn : cjsModuleSegmentMap_[segment]) {
    worklist.insert(fn);
  }

  while (!worklist.empty()) {
    Function *cur = worklist.back();
    worklist.pop_back();
    if (result.count(cur)) {
      // We've already visited this function and added its children, so don't do
      // it again.
      continue;
    }
    result.insert(cur);
    // The functions that are used by the function cur.
    const auto targets = cjsModuleUseGraph_[cur];
    worklist.insert(targets.begin(), targets.end());
  }

  return result;
}

uint32_t Module::getTemplateObjectID(RawStringList &&rawStrings) {
  // Try inserting into the map with a dummy value.
  auto res = templateObjectIDMap_.emplace(std::move(rawStrings), 0);
  if (res.second) {
    // Insertion succeeded. Overwrite the value with the correct id.
    res.first->second = templateObjectIDMap_.size() - 1;
  }
  return res.first->second;
}

Context &Instruction::getContext() const {
  return Parent->getContext();
}
Context &BasicBlock::getContext() const {
  return Parent->getContext();
}
Context &Parameter::getContext() const {
  return Parent->getContext();
}

/// \returns true if this parameter is a 'this' parameter.
bool Parameter::isThisParameter() const {
  return Parent->getThisParameter() == this;
}

int Parameter::getIndexInParamList() const {
  int index = 0;
  for (auto P : Parent->getParameters()) {
    if (P == this)
      return index;
    index++;
  }
  llvm_unreachable("Cannot find parameter in the function");
}

void Function::dump() {
  IRPrinter D(getParent()->getContext(), llvh::outs());
  D.visit(*this);
}

void Function::viewGraph() {
  ::viewGraph(this);
}

/// Strip the " #number" suffice of a generated internal name, or a name that
/// just happens to be in that format.
static inline Identifier stripInternalNameSuffix(
    Context &context,
    Identifier originalName) {
  auto originalStr = originalName.str();
  auto e = originalStr.end();

  if (!(e - originalStr.begin() >= 3 && e[-1] == '#' && e[-2] >= '0' &&
        e[-2] <= '9')) {
    return originalName;
  }

  e -= 2;
  while (e != originalStr.begin() && e[-1] >= '0' && e[-1] <= '9') {
    --e;
  }

  if (!(e != originalStr.begin() && e[-1] == ' '))
    return originalName;

  --e;
  return context.getIdentifier(originalStr.slice(0, e - originalStr.begin()));
}

Identifier Module::deriveUniqueInternalName(Identifier originalName) {
  assert(originalName.isValid() && "originalName must be valid");

  // Check whether the original name already is in the form "... number#" and
  // if so, strip the suffix.
  originalName = stripInternalNameSuffix(getContext(), originalName);

  auto insertResult = internalNamesMap_.try_emplace(originalName, 0);

  // If inserted for the first time, there is no need for a suffix.
  if (insertResult.second)
    return originalName;

  // Construct a suffix using the number of internal names derived from this
  // identifier.
  ++insertResult.first->second;
  char itoaBuf[16];
  snprintf(itoaBuf, sizeof(itoaBuf), "%u", insertResult.first->second);

  llvh::SmallString<32> buf;
  buf.append(originalName.str());
  buf.append(" ");
  buf.append(itoaBuf);
  buf.append("#");

  return getContext().getIdentifier(buf);
}

void Module::viewGraph() {
  for (auto &F : *this) {
    ::viewGraph(&F);
  }
}

void Module::dump() {
  for (auto &F : *this) {
    F.dump();
  }
}

LiteralNumber *Module::getLiteralNumber(double value) {
  // Check to see if we've already seen this tuple before.
  llvh::FoldingSetNodeID ID;

  LiteralNumber::Profile(ID, value);

  // If this is not the first time we see this tuple then return the old copy.
  void *InsertPos = nullptr;
  if (LiteralNumber *LN = literalNumbers.FindNodeOrInsertPos(ID, InsertPos))
    return LN;

  auto New = new LiteralNumber(value);
  literalNumbers.InsertNode(New, InsertPos);
  return New;
}

LiteralBigInt *Module::getLiteralBigInt(UniqueString *value) {
  // Check to see if we've already seen this tuple before.
  llvh::FoldingSetNodeID ID;

  LiteralBigInt::Profile(ID, value);

  // If this is not the first time we see this tuple then return the old copy.
  void *InsertPos = nullptr;
  if (LiteralBigInt *LN = literalBigInts.FindNodeOrInsertPos(ID, InsertPos))
    return LN;

  auto New = new LiteralBigInt(value);
  literalBigInts.InsertNode(New, InsertPos);
  return New;
}

LiteralString *Module::getLiteralString(Identifier value) {
  // Check to see if we've already seen this tuple before.
  llvh::FoldingSetNodeID ID;

  LiteralString::Profile(ID, value);

  // If this is not the first time we see this tuple then return the old copy.
  void *InsertPos = nullptr;
  if (LiteralString *LS = literalStrings.FindNodeOrInsertPos(ID, InsertPos))
    return LS;

  auto New = new LiteralString(value);
  literalStrings.InsertNode(New, InsertPos);
  return New;
}

LiteralBool *Module::getLiteralBool(bool value) {
  if (value)
    return &literalTrue;
  return &literalFalse;
}

void Type::print(llvh::raw_ostream &OS) const {
  bool first = true;
  for (unsigned i = 0; i < (unsigned)Type::TypeKind::LAST_TYPE; i++) {
    // Don't print the object type annotations if the type is closure or regex.
    if (i == (unsigned)Type::TypeKind::Object &&
        (isClosureType() || isRegExpType())) {
      continue;
    }

    if (bitmask_ & (1 << i)) {
      if (!first) {
        OS << "|";
      }

      OS << getKindStr((Type::TypeKind)i);
      first = false;
    }
  }
}

raw_ostream &llvh::operator<<(raw_ostream &OS, const hermes::Type &T) {
  T.print(OS);
  return OS;
}
