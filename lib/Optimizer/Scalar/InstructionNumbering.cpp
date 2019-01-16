#define DEBUG_TYPE "instructionnumbering"

#include "hermes/Optimizer/Scalar/InstructionNumbering.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Support/OptValue.h"

#include "llvm/Support/Casting.h"

using llvm::dyn_cast;
using llvm::isa;

namespace hermes {

InstructionNumbering::InstructionNumbering(
    BasicBlock::range range,
    ExternalFlags flags)
    : currentIter_(range.begin()),
      endIter_(range.end()),
      externalFlags_(flags) {
  processInstruction();
}

void InstructionNumbering::processInstruction() {
  if (currentIter_ == endIter_) {
    expression_ = llvm::None;
    return;
  }

  // Build the list of operands.
  Instruction *const currentInst = &*currentIter_;
  Expression::OperandVec operandVec;
  for (unsigned i = 0, e = currentInst->getNumOperands(); i < e; ++i) {
    Value *value = currentInst->getOperand(i);
    OptValue<unsigned> earlierInstructionIdx;
    if (auto *inst = dyn_cast<Instruction>(value)) {
      auto iter = internalMap_.find(inst);
      if (iter != internalMap_.end()) {
        earlierInstructionIdx = iter->second;
      }
    }

    // Decide whether it's an Internal, External, or Value operand.
    if (earlierInstructionIdx.hasValue()) {
      // A reference to an instruction defined earlier in the range.
      operandVec.emplace_back(
          OperandKind::Internal, earlierInstructionIdx.getValue());
    } else if (isExternalKind(value)) {
      auto iter = externalMap_.find(value);
      if (iter != externalMap_.end()) {
        // A reference to an external input that we've seen before.
        operandVec.emplace_back(OperandKind::External, iter->second);
      } else {
        // A reference to an external input we're seeing for the first time.
        const auto index = externalMap_.size();
        operandVec.emplace_back(OperandKind::External, index);
        auto insert = externalMap_.try_emplace(value, index);
        (void)insert;
        assert(insert.second && "Failed to insert in externalMap_!");
      }
    } else {
      // For other kinds of values, store the value directly.
      operandVec.emplace_back(OperandKind::Value, value);
    }
  }

  // Insert this instruction into the map so that it can be recognized when it's
  // used as an operand of subsequent instructions.
  auto insert = internalMap_.try_emplace(currentInst, internalMap_.size());
  (void)insert;
  assert(insert.second && "Same instruction encountered twice!");

  expression_ = Expression(currentInst, std::move(operandVec));
}

bool InstructionNumbering::isExternalKind(Value *value) const {
  return ((externalFlags_ & Instructions) && isa<Instruction>(value)) ||
      ((externalFlags_ & Parameters) && isa<Parameter>(value)) ||
      ((externalFlags_ & Literals) && isa<Literal>(value));
}

} // namespace hermes
