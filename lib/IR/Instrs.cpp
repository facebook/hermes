/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "llvh/ADT/Hashing.h"
#include "llvh/ADT/SmallString.h"
#include "llvh/Support/Casting.h"
#include "llvh/Support/ErrorHandling.h"

#include "hermes/IR/CFG.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRVisitor.h"
#include "hermes/IR/Instrs.h"

#define INCLUDE_ALL_INSTRS

using namespace hermes;

unsigned TerminatorInst::getNumSuccessors() const {
#undef TERMINATOR
#define TERMINATOR(CLASS, PARENT)           \
  if (auto I = llvh::dyn_cast<CLASS>(this)) \
    return I->getNumSuccessors();
#include "hermes/IR/Instrs.def"
  llvm_unreachable("not a terminator?!");
}

BasicBlock *TerminatorInst::getSuccessor(unsigned idx) const {
#undef TERMINATOR
#define TERMINATOR(CLASS, PARENT)           \
  if (auto I = llvh::dyn_cast<CLASS>(this)) \
    return I->getSuccessor(idx);
#include "hermes/IR/Instrs.def"
  llvm_unreachable("not a terminator?!");
}

void TerminatorInst::setSuccessor(unsigned idx, BasicBlock *B) {
#undef TERMINATOR
#define TERMINATOR(CLASS, PARENT)           \
  if (auto I = llvh::dyn_cast<CLASS>(this)) \
    return I->setSuccessor(idx, B);
#include "hermes/IR/Instrs.def"
  llvm_unreachable("not a terminator?!");
}

bool hermes::isSideEffectFree(Type T) {
  return T.isPrimitive();
}

const char *UnaryOperatorInst::opStringRepr[] =
    {"delete", "void", "typeof", "+", "-", "~", "!", "++", "--"};

const char *BinaryOperatorInst::opStringRepr[] = {
    "",   "==", "!=",  "===", "!==", "<", "<=", ">",         ">=",
    "<<", ">>", ">>>", "+",   "-",   "*", "/",  "%",         "|",
    "^",  "&",  "**",  "",    "",    "",  "in", "instanceof"};

const char *BinaryOperatorInst::assignmentOpStringRepr[] = {
    "=",   "",    "",     "",    "",    "",      "",   "",   "",
    "<<=", ">>=", ">>>=", "+=",  "-=",  "*=",    "/=", "%=", "|=",
    "^=",  "&=",  "**=",  "||=", "&&=", "\?\?=", "",   ""};

UnaryOperatorInst::OpKind UnaryOperatorInst::parseOperator(StringRef op) {
  for (int i = 0; i < static_cast<int>(BinaryOperatorInst::OpKind::LAST_OPCODE);
       i++) {
    if (op == UnaryOperatorInst::opStringRepr[i]) {
      return static_cast<UnaryOperatorInst::OpKind>(i);
    }
  }

  llvm_unreachable("invalid operator string");
}

SideEffectKind UnaryOperatorInst::getSideEffect() {
  if (getOperatorKind() == OpKind::DeleteKind) {
    return SideEffectKind::Unknown;
  }

  if (isSideEffectFree(getSingleOperand()->getType())) {
    return SideEffectKind::None;
  }

  switch (getOperatorKind()) {
    case OpKind::VoidKind: // void
    case OpKind::TypeofKind: // typeof
      return SideEffectKind::None;

    default:
      break;
  }

  return SideEffectKind::Unknown;
}

static BinaryOperatorInst::OpKind parseOperator_impl(
    StringRef op,
    const char **lookup_table) {
  for (int i = 0; i < static_cast<int>(BinaryOperatorInst::OpKind::LAST_OPCODE);
       i++) {
    if (op == lookup_table[i]) {
      return static_cast<BinaryOperatorInst::OpKind>(i);
    }
  }

  llvm_unreachable("invalid operator string");
}

BinaryOperatorInst::OpKind BinaryOperatorInst::parseOperator(StringRef op) {
  return parseOperator_impl(op, opStringRepr);
}

BinaryOperatorInst::OpKind BinaryOperatorInst::parseAssignmentOperator(
    StringRef op) {
  return parseOperator_impl(op, assignmentOpStringRepr);
}

llvh::Optional<BinaryOperatorInst::OpKind>
BinaryOperatorInst::tryGetReverseOperator(BinaryOperatorInst::OpKind op) {
  switch (op) {
    // Commutative operators
    case OpKind::EqualKind:
    case OpKind::NotEqualKind:
    case OpKind::StrictlyEqualKind:
    case OpKind::StrictlyNotEqualKind:
    case OpKind::AddKind:
    case OpKind::MultiplyKind:
    case OpKind::OrKind:
    case OpKind::XorKind:
    case OpKind::AndKind:
      return op;

    // Rewritable operators
    case OpKind::LessThanKind:
      return OpKind::GreaterThanKind;
    case OpKind::LessThanOrEqualKind:
      return OpKind::GreaterThanOrEqualKind;
    case OpKind::GreaterThanKind:
      return OpKind::LessThanKind;
    case OpKind::GreaterThanOrEqualKind:
      return OpKind::LessThanOrEqualKind;

    default:
      return llvh::None;
  }
}

SideEffectKind
BinaryOperatorInst::getBinarySideEffect(Type leftTy, Type rightTy, OpKind op) {
  // The 'in' and 'instanceof' operators may throw:
  if (op == OpKind::InKind || op == OpKind::InstanceOfKind)
    return SideEffectKind::Unknown;

  // Strict equality does not throw or have other side effects (per ES5 11.9.6).
  if (op == OpKind::StrictlyNotEqualKind || op == OpKind::StrictlyEqualKind)
    return SideEffectKind::None;

  // This instruction does not read/write memory if the LHS and RHS types are
  // known to be safe (number, string, null, etc).
  if (isSideEffectFree(leftTy) && isSideEffectFree(rightTy))
    return SideEffectKind::None;

  // This binary operation may execute arbitrary code.
  return SideEffectKind::Unknown;
}

SwitchInst::SwitchInst(
    Value *input,
    BasicBlock *defaultBlock,
    const ValueListType &values,
    const BasicBlockListType &blocks)
    : TerminatorInst(ValueKind::SwitchInstKind) {
  pushOperand(input);
  pushOperand(defaultBlock);

  assert(blocks.size() && "Empty switch statement (no cases?)");
  assert(values.size() == blocks.size() && "Block-value pairs mismatch");

  // Push the switch targets.
  for (int i = 0, e = values.size(); i < e; ++i) {
    pushOperand(values[i]);
    pushOperand(blocks[i]);
  }
}

unsigned SwitchInst::getNumCasePair() const {
  // The number of cases is computed as the total number of operands, minus
  // the input value and the default basic block. Take this number and divide
  // it in two, because we are counting pairs.
  return (getNumOperands() - FirstCaseIdx) / 2;
}

std::pair<Literal *, BasicBlock *> SwitchInst::getCasePair(unsigned i) const {
  // The values and lables are twined together. Find the index of the pair
  // that we are fetching and return the two values.
  unsigned base = i * 2 + FirstCaseIdx;
  return std::make_pair(
      cast<Literal>(getOperand(base)), cast<BasicBlock>(getOperand(base + 1)));
}

BasicBlock *SwitchInst::getSuccessor(unsigned idx) const {
  assert(idx < getNumSuccessors() && "getSuccessor out of bound!");
  if (idx == 0)
    return getDefaultDestination();
  return getCasePair(idx - 1).second;
}

void SwitchInst::setSuccessor(unsigned idx, BasicBlock *B) {
  assert(idx < getNumSuccessors() && "setSuccessor out of bound!");
  if (idx == 0) {
    setOperand(B, DefaultBlockIdx);
    return;
  }
  setOperand(B, FirstCaseIdx + (idx - 1) * 2 + 1);
}

BasicBlock *SwitchInst::getDefaultDestination() const {
  return cast<BasicBlock>(getOperand(DefaultBlockIdx));
}

Value *SwitchInst::getInputValue() const {
  return getOperand(InputIdx);
}

PhiInst::PhiInst(const ValueListType &values, const BasicBlockListType &blocks)
    : Instruction(ValueKind::PhiInstKind) {
  assert(values.size() == blocks.size() && "Block-value pairs mismatch");
  // Push the incoming values.
  for (int i = 0, e = values.size(); i < e; ++i) {
    pushOperand(values[i]);
    pushOperand(blocks[i]);
  }
}

unsigned PhiInst::getNumEntries() const {
  // The PHI operands are just pairs of values and basic blocks.
  return getNumOperands() / 2;
}

/// Returns the index of the first operand for phi entry pair.
static unsigned indexOfPhiEntry(unsigned index) {
  return index * 2;
}

std::pair<Value *, BasicBlock *> PhiInst::getEntry(unsigned i) const {
  return std::make_pair(
      getOperand(indexOfPhiEntry(i)),
      cast<BasicBlock>(getOperand(indexOfPhiEntry(i) + 1)));
}

void PhiInst::updateEntry(unsigned i, Value *val, BasicBlock *BB) {
  setOperand(val, indexOfPhiEntry(i));
  setOperand(BB, indexOfPhiEntry(i) + 1);
}

void PhiInst::addEntry(Value *val, BasicBlock *BB) {
  pushOperand(val);
  pushOperand(BB);
}

void PhiInst::removeEntry(unsigned index) {
  // Remove the pair at the right offset. See calculation of getEntry above.
  unsigned startIdx = indexOfPhiEntry(index);
  // Remove the value:
  removeOperand(startIdx);
  // Remove the basic block. Notice that we use the same index because the
  // list is shifted.
  removeOperand(startIdx);
}

void PhiInst::removeEntry(BasicBlock *BB) {
  unsigned i = 0;
  // For each one of the entries:
  while (i < getNumEntries()) {
    // If this entry is from the BB we want to remove, then remove it.
    if (getEntry(i).second == BB) {
      removeEntry(i);
      // keep the current iteration index.
      continue;
    }
    // Else, move to the next entry.
    i++;
  }
}

GetPNamesInst::GetPNamesInst(
    BasicBlock *parent,
    Value *iteratorAddr,
    Value *baseAddr,
    Value *indexAddr,
    Value *sizeAddr,
    BasicBlock *onEmpty,
    BasicBlock *onSome)
    : TerminatorInst(ValueKind::GetPNamesInstKind) {
  pushOperand(iteratorAddr);
  pushOperand(baseAddr);
  pushOperand(indexAddr);
  pushOperand(sizeAddr);
  pushOperand(onEmpty);
  pushOperand(onSome);
}

GetNextPNameInst::GetNextPNameInst(
    BasicBlock *parent,
    Value *propertyAddr,
    Value *baseAddr,
    Value *indexAddr,
    Value *sizeAddr,
    Value *iteratorAddr,
    BasicBlock *onLast,
    BasicBlock *onSome)
    : TerminatorInst(ValueKind::GetNextPNameInstKind) {
  pushOperand(propertyAddr);
  pushOperand(baseAddr);
  pushOperand(indexAddr);
  pushOperand(sizeAddr);
  pushOperand(iteratorAddr);
  pushOperand(onLast);
  pushOperand(onSome);
}

SwitchImmInst::SwitchImmInst(
    Value *input,
    BasicBlock *defaultBlock,
    LiteralNumber *minValue,
    LiteralNumber *size,
    const ValueListType &values,
    const BasicBlockListType &blocks)
    : TerminatorInst(ValueKind::SwitchImmInstKind) {
  pushOperand(input);
  pushOperand(defaultBlock);

  assert(minValue->isUInt32Representible() && "minValue must be uint32_t");
  pushOperand(minValue);
  assert(size->isUInt32Representible() && "size must be uint32_t");
  pushOperand(size);
  assert(
      minValue->asUInt32() + size->asUInt32() >= minValue->asUInt32() &&
      "minValue + size must not overflow");

  assert(blocks.size() && "Empty switch statement (no cases?)");
  assert(values.size() == blocks.size() && "Block-value pairs mismatch");

  // Push the switch targets.
  for (size_t i = 0, e = values.size(); i < e; ++i) {
    assert(values[i]->isUInt32Representible() && "case value must be uint32_t");
    pushOperand(values[i]);
    pushOperand(blocks[i]);
  }
}

BasicBlock *SwitchImmInst::getSuccessor(unsigned idx) const {
  assert(idx < getNumSuccessors() && "getSuccessor out of bound!");
  if (idx == 0)
    return getDefaultDestination();
  return getCasePair(idx - 1).second;
}

void SwitchImmInst::setSuccessor(unsigned idx, BasicBlock *B) {
  assert(idx < getNumSuccessors() && "setSuccessor out of bound!");
  if (idx == 0) {
    setOperand(B, DefaultBlockIdx);
    return;
  }
  setOperand(B, FirstCaseIdx + (idx - 1) * 2 + 1);
}

Instruction::Variety Instruction::getVariety() const {
  const ValueKind kind = getKind();

  // Get the operator kind, if the instruction has one.
  unsigned operatorKind;
  switch (kind) {
    case ValueKind::BinaryOperatorInstKind:
      operatorKind = static_cast<unsigned>(
          cast<BinaryOperatorInst>(this)->getOperatorKind());
      break;

    case ValueKind::UnaryOperatorInstKind:
      operatorKind = static_cast<unsigned>(
          cast<UnaryOperatorInst>(this)->getOperatorKind());
      break;

    case ValueKind::CompareBranchInstKind:
      operatorKind = static_cast<unsigned>(
          cast<CompareBranchInst>(this)->getOperatorKind());
      break;

    default:
      operatorKind = 0;
      break;
  }

  // Combine the two kinds into a single value.
  return Variety(std::make_pair(static_cast<unsigned>(kind), operatorKind));
}

bool Instruction::isIdenticalTo(const Instruction *RHS) const {
  // Check if both instructions have the same variety and number of operands.
  // This should filter out most cases.
  if (getVariety() != RHS->getVariety() ||
      getNumOperands() != RHS->getNumOperands())
    return false;

  // Check operands.
  for (unsigned i = 0, e = getNumOperands(); i != e; ++i)
    if (getOperand(i) != RHS->getOperand(i))
      return false;

  return true;
}

namespace {
/// Return the hash code of the visited instruction.
class InstructionHashConstructor
    : public InstructionVisitor<InstructionHashConstructor, llvh::hash_code> {
 public:
  /// Return default hash code.
  llvh::hash_code visitInstruction(const Instruction &V) {
    return llvh::hash_code();
  }

  /// IMPLEMENT anything that needs special handling here. visitInstruction will
  /// be called for Instructions we do not know how to handle speically.
};
} // namespace

llvh::hash_code Instruction::getHashCode() const {
  llvh::hash_code hc = llvh::hash_combine(getVariety(), getNumOperands());

  // Check operands.
  for (unsigned i = 0, e = getNumOperands(); i != e; ++i)
    hc = llvh::hash_combine(hc, getOperand(i));

  // Hash in any special attributes for an instruction.
  return llvh::hash_combine(hc, InstructionHashConstructor().visit(*this));
}
