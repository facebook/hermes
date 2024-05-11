/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/IR/CFG.h"
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

using namespace hermes;

// Make sure the ValueKinds.def tree is consistent with the class hierarchy.
#define QUOTE(X) #X
#define DEF_VALUE(CLASS, PARENT)                                        \
  static_assert(                                                        \
      std::is_base_of<PARENT, CLASS>::value,                            \
      QUOTE(CLASS) " should directly inherit from " QUOTE(PARENT));     \
  static_assert(                                                        \
      std::is_convertible<CLASS *, PARENT *>::value,                    \
      QUOTE(CLASS) " should publicly inherit from " QUOTE(PARENT));     \
  static_assert(                                                        \
      ValueKind::CLASS##Kind > ValueKind::First_##PARENT##Kind,         \
      QUOTE(CLASS) "Kind should be after First_" QUOTE(PARENT) "Kind"); \
  static_assert(                                                        \
      ValueKind::CLASS##Kind < ValueKind::Last_##PARENT##Kind,          \
      QUOTE(CLASS) "Kind should be before Last_" QUOTE(PARENT) "Kind");
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
#define DEF_TAG(XX, PARENT) \
  case ValueKind::XX##Kind: \
    delete cast<PARENT>(V); \
    break;
#include "hermes/IR/ValueKinds.def"
  }
}

llvh::StringRef Value::getKindStr() const {
  switch (Kind) {
    default:
      llvm_unreachable("Invalid kind");
#define DEF_VALUE(XX, PARENT) \
  case ValueKind::XX##Kind:   \
    return llvh::StringLiteral(#XX);
#define DEF_TAG(XX, PARENT) DEF_VALUE(XX, PARENT)
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
    Use oldUse = {this, static_cast<unsigned>(Users.size())};
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
  return {this, static_cast<unsigned>(Users.size() - 1)};
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

std::string Attributes::getDescriptionStr() const {
  if (isEmpty())
    return "";
  std::string result{"["};

  bool comma = false;

  /// Add \p name to the result string.
  const auto addFlag = [&comma, &result](llvh::StringRef name) -> void {
    if (comma)
      result += ',';
    comma = true;
    result += name;
  };

#define ATTRIBUTE(_valueKind, name, string) \
  if (name)                                 \
    addFlag(string);
#include "hermes/IR/Attributes.def"

  result += "]";
  return result;
}

/// Determine the type of a function's \c new.target, based on the function's
/// \c DefinitionKind.
static Type functionNewTargetType(Function::DefinitionKind defKind) {
  switch (defKind) {
    case Function::DefinitionKind::ES5Function:
      return Type::unionTy(Type::createObject(), Type::createUndefined());
    case Function::DefinitionKind::ES6Constructor:
      return Type::createObject();
    case Function::DefinitionKind::ES6Arrow:
      // Arrow functions never access their own new.target.
      return Type::createNoType();
    case Function::DefinitionKind::GeneratorInner:
    case Function::DefinitionKind::ES6Method:
      return Type::createUndefined();
  }
}

Function::Function(
    ValueKind kind,
    Module *parent,
    Identifier originalName,
    DefinitionKind definitionKind,
    bool strictMode,
    CustomDirectives customDirectives,
    SMRange sourceRange,
    Function *insertBefore)
    : Value(kind),
      parent_(parent),
      newTargetParam_(this, parent_->getContext().getIdentifier("new.target")),
      parentScopeParam_(
          this,
          parent_->getContext().getIdentifier("parentScope")),
      originalOrInferredName_(originalName),
      definitionKind_(definitionKind),
      strictMode_(strictMode),
      SourceRange(sourceRange),
      customDirectives_(customDirectives),
      internalName_(parent->deriveUniqueInternalName(originalName)) {
  setType(Type::createFunctionCode());
  // Determine the type of new.target.
  newTargetParam_.setType(functionNewTargetType(definitionKind_));
  parentScopeParam_.setType(Type::createEnvironment());

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
  for (auto *p : jsDynamicParams_) {
    Value::destroy(p);
  }
}

bool Function::isGlobalScope() const {
  return parent_->getTopLevelFunction() == this;
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
    case Function::DefinitionKind::GeneratorInner:
      return "generator inner";
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

Function::ProhibitInvoke Function::getProhibitInvoke() const {
  // ES6 constructors must be invoked as constructors.
  if (definitionKind_ == DefinitionKind::ES6Constructor)
    return ProhibitInvoke::ProhibitCall;

  // Generators, async functions, methods, and arrow functions may not be
  // invoked as constructors.
  if (getKind() == ValueKind::GeneratorFunctionKind ||
      getKind() == ValueKind::AsyncFunctionKind ||
      definitionKind_ == DefinitionKind::ES6Arrow ||
      definitionKind_ == DefinitionKind::ES6Method)
    return ProhibitInvoke::ProhibitConstruct;

  return ProhibitInvoke::ProhibitNone;
}

BasicBlock::BasicBlock(Function *parent)
    : Value(ValueKind::BasicBlockKind), Parent(parent) {
  assert(Parent && "Invalid parent function");
  Parent->addBlock(this);
}

void BasicBlock::dump(llvh::raw_ostream &os) const {
  irdumper::IRPrinter D(getParent()->getContext(), os);
  D.visit(*this);
}

void BasicBlock::printAsOperand(llvh::raw_ostream &OS, bool) const {
  // Use the address of the basic block when LLVM prints the CFG.
  size_t Num = (size_t)this;
  OS << "BB#" << std::to_string(Num);
}

void Instruction::dump(llvh::raw_ostream &os) const {
  irdumper::IRPrinter D(getParent()->getContext(), os);
  D.visit(*this);
}

Instruction::Instruction(
    const Instruction *src,
    llvh::ArrayRef<Value *> operands)
    : Instruction(src->getKind()) {
  setType(src->getType());

  location_ = src->location_;
  statementIndex_ = src->statementIndex_;
  Module *M = src->getModule();
  getAttributesRef(M) = src->getAttributes(M);

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
  getParent()->getFunctionList().remove(getIterator());
}

llvh::StringRef Instruction::getName() {
  switch (getKind()) {
    default:
      llvm_unreachable("Invalid kind");
#define DEF_VALUE(XX, PARENT) \
  case ValueKind::XX##Kind:   \
    return #XX;
#define DEF_TAG(XX, PARENT) \
  case ValueKind::XX##Kind: \
    return #XX;
#include "hermes/IR/Instrs.def"
  }
}

SideEffect Instruction::getSideEffect() const {
  switch (getKind()) {
    default:
      llvm_unreachable("Invalid kind");
#define DEF_VALUE(XX, PARENT) \
  case ValueKind::XX##Kind:   \
    return cast<XX>(this)->getSideEffectImpl();
#define DEF_TAG(XX, PARENT) \
  case ValueKind::XX##Kind: \
    return cast<PARENT>(this)->getSideEffectImpl();
#include "hermes/IR/Instrs.def"
  }
}

llvh::Optional<Type> Instruction::getInherentType() {
  switch (getKind()) {
    default:
      llvm_unreachable("Invalid kind");
#define DEF_VALUE(XX, PARENT) \
  case ValueKind::XX##Kind:   \
    return XX::getInherentTypeImpl();
#define DEF_TAG(XX, PARENT) \
  case ValueKind::XX##Kind: \
    return PARENT::getInherentTypeImpl();
#include "hermes/IR/Instrs.def"
  }
}

bool Instruction::hasOutput() const {
  switch (getKind()) {
    default:
      llvm_unreachable("Invalid kind");
#define DEF_VALUE(XX, PARENT) \
  case ValueKind::XX##Kind:   \
    return XX::hasOutput();
#define DEF_TAG(XX, PARENT) \
  case ValueKind::XX##Kind: \
    return PARENT::hasOutput();
#include "hermes/IR/Instrs.def"
  }
}

bool Instruction::isTyped() const {
  switch (getKind()) {
    default:
      llvm_unreachable("Invalid kind");
#define DEF_VALUE(XX, PARENT) \
  case ValueKind::XX##Kind:   \
    return XX::isTyped();
#define DEF_TAG(XX, PARENT) \
  case ValueKind::XX##Kind: \
    return PARENT::isTyped();
#include "hermes/IR/Instrs.def"
  }
}

bool Instruction::acceptsEmptyType() const {
  switch (getKind()) {
    default:
      llvm_unreachable("Invalid kind");

#define DEF_VALUE(XX, PARENT)                      \
  case ValueKind::XX##Kind:                        \
    return cast<XX>(this)->acceptsEmptyTypeImpl(); \
    break;
#define DEF_TAG(XX, PARENT)                            \
  case ValueKind::XX##Kind:                            \
    return cast<PARENT>(this)->acceptsEmptyTypeImpl(); \
    break;
#include "hermes/IR/Instrs.def"
  }
}

Variable::Variable(VariableScope *scope, Identifier txt)
    : Value(ValueKind::VariableKind), text(txt), parent(scope) {
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

VariableScope::VariableScope(VariableScope *parentScope)
    : Value(ValueKind::VariableScopeKind), parentScope_(parentScope) {
  if (parentScope)
    parentScope->children_.push_back(*this);
}

void VariableScope::removeFromScopeChain() {
  // Update all children to now be children of the parent.
  for (auto &child : children_)
    child.parentScope_ = parentScope_;

  // Remove this scope from the parent's children list (if any), and transfer
  // all children to it.
  if (parentScope_) {
    parentScope_->children_.remove(*this);
    parentScope_->children_.splice(parentScope_->children_.begin(), children_);
  } else {
    // Clear the children list so we don't do any work on subsequent calls to
    // this function.
    children_.clear();
  }

  // Clear the parent scope, so this operation is idempotent.
  parentScope_ = nullptr;
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

void Function::moveBlockToEntry(BasicBlock *newEntry) {
  assert(
      std::find_if(
          BasicBlockList.begin(),
          BasicBlockList.end(),
          [newEntry](auto &BB) { return &BB == newEntry; }) !=
          BasicBlockList.end() &&
      "block not in function");
  assert(pred_count(newEntry) == 0 && "block should not have any predecessors");
  BasicBlockList.remove(iterator(newEntry));
  BasicBlockList.push_front(newEntry);
}

void Function::addJSDynamicParam(JSDynamicParam *param) {
  assert(jsDynamicParams_.size() < UINT32_MAX && "Too many parameters");
  jsDynamicParams_.push_back(param);
}
void Function::addJSThisParam(JSDynamicParam *param) {
  assert(jsDynamicParams_.empty() && "'this' must be the first js parameter");
  jsDynamicParams_.push_back(param);
  jsThisAdded_ = true;
}

Module::~Module() {
  FunctionList.clear();
}

void Module::push_back(Function *F) {
  FunctionList.push_back(F);
}

void Module::insert(iterator position, Function *F) {
  FunctionList.insert(position, F);
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

std::pair<llvh::ArrayRef<LiteralString *>, uint32_t>
Module::emplaceTemplateObject(RawStringList &&rawStrings) {
  // Try inserting into the map with a dummy value.
  auto res = templateObjectIDMap_.emplace(std::move(rawStrings), 0);
  if (res.second) {
    // Insertion succeeded. Overwrite the value with the correct id.
    res.first->second = templateObjectIDMap_.size() - 1;
  }
  return {res.first->first, res.first->second};
}

Context &Instruction::getContext() const {
  return Parent->getContext();
}
Context &BasicBlock::getContext() const {
  return Parent->getContext();
}
Context &JSDynamicParam::getContext() const {
  return parent_->getContext();
}

uint32_t JSDynamicParam::getIndexInParamList() const {
  uint32_t index = 0;
  for (auto P : parent_->getJSDynamicParams()) {
    if (P == this)
      return index;
    index++;
  }
  llvm_unreachable("Cannot find parameter in the function");
}

void Function::dump(llvh::raw_ostream &os) const {
  irdumper::IRPrinter D(getParent()->getContext(), os);
  D.visit(*this);
}

void Function::viewGraph() {
  irdumper::viewGraph(this);
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
    irdumper::viewGraph(&F);
  }
}

void Module::dump(llvh::raw_ostream &os) const {
  irdumper::IRPrinter D(getContext(), os);
  D.visit(*this);
}

LiteralNumber *Module::getLiteralNumber(double value) {
  return literalNumbers_.getOrEmplace(value).first;
}

LiteralBigInt *Module::getLiteralBigInt(UniqueString *value) {
  return literalBigInts_.getOrEmplace(value).first;
}

LiteralString *Module::getLiteralString(Identifier value) {
  return literalStrings_.getOrEmplace(value).first;
}

LiteralBool *Module::getLiteralBool(bool value) {
  if (value)
    return &literalTrue;
  return &literalFalse;
}

GlobalObjectProperty *Module::addGlobalProperty(
    Identifier name,
    bool declared) {
  auto *res = globalProperties_.getOrEmplace(getLiteralString(name)).first;
  res->orDeclared(declared);
  return res;
}

LiteralIRType *Module::getLiteralIRType(Type value) {
  return literalIRTypes_.getOrEmplace(value).first;
}

LiteralNativeSignature *Module::getLiteralNativeSignature(
    NativeSignature *data) {
  return nativeSignatures_.getOrEmplace(data).first;
}

LiteralNativeExtern *Module::getLiteralNativeExtern(NativeExtern *data) {
  return nativeExterns_.getOrEmplace(data).first;
}

void Type::print(llvh::raw_ostream &OS) const {
  bool first = true;
  if (isNoType()) {
    OS << "notype";
    return;
  }
  if (canBeAny()) {
    OS << "any";
    if (canBeEmpty())
      OS << "|empty";
    if (canBeUninit())
      OS << "|uninit";
    return;
  }
  for (unsigned i = 0; i < (unsigned)Type::TypeKind::LAST_TYPE; i++) {
    if (bitmask_ & (1 << i)) {
      if (!first) {
        OS << "|";
      }

      OS << getKindStr((Type::TypeKind)i);
      first = false;
    }
  }
}

llvh::raw_ostream &llvh::operator<<(raw_ostream &OS, const hermes::Type &T) {
  T.print(OS);
  return OS;
}
