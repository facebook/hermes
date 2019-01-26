/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/Optimizer/Scalar/InstructionNumbering.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"

#include "gtest/gtest.h"
#include "llvm/ADT/Hashing.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <vector>

using namespace hermes;

using Expression = InstructionNumbering::Expression;
using Operand = InstructionNumbering::Operand;
using OperandKind = InstructionNumbering::OperandKind;

namespace hermes {

/// Custom printer for gtest Expression to make test failures more readable.
static void PrintTo(const Expression &expr, std::ostream *os) {
  using llvm::hash_value;
  const auto variety = hash_value(expr.variety);
  *os << "Expression(" << std::hex << variety << std::dec << ", [";

  bool first = true;
  for (const Operand &op : expr.operands) {
    if (!first) {
      *os << ", ";
    }
    first = false;
    switch (op.kind) {
      case OperandKind::Internal:
        *os << "Internal(" << op.index << ")";
        break;
      case OperandKind::External:
        *os << "External(" << op.index << ")";
        break;
      case OperandKind::Value:
        *os << "Value(" << op.valuePtr << ")";
        break;
    }
  }
  *os << "])";
}

} // namespace hermes

namespace {

class InstructionNumberingTest : public ::testing::Test {
 public:
  InstructionNumberingTest()
      : context_(std::make_shared<Context>()),
        module_(context_),
        builder_(&module_),
        function_(builder_.createFunction(
            "main",
            Function::DefinitionKind::ES5Function,
            true)),
        block_(builder_.createBasicBlock(function_)) {
    builder_.setInsertionBlock(block_);
  }

 private:
  std::shared_ptr<Context> context_;
  Module module_;

 protected:
  IRBuilder builder_;
  Function *function_;
  BasicBlock *block_;
};

Operand internal(unsigned index) {
  return Operand(OperandKind::Internal, index);
}
Operand external(unsigned index) {
  return Operand(OperandKind::External, index);
}
Operand value(Value *valuePtr) {
  return Operand(OperandKind::Value, valuePtr);
}

/// Invoke InstructionNumbering on the given inclusive range of instructions
/// and collect the resulting expressions in a vector.
std::vector<Expression> collectExpressions(
    Instruction *first,
    Instruction *last) {
  BasicBlock::range range(first->getIterator(), std::next(last->getIterator()));
  InstructionNumbering numbering(range);
  std::vector<Expression> exprs;
  std::copy(numbering.begin(), numbering.end(), std::back_inserter(exprs));
  return exprs;
}

TEST_F(InstructionNumberingTest, EmptyRangeTest) {
  InstructionNumbering numbering(*block_);
  EXPECT_EQ(numbering.begin(), numbering.end());
}

TEST_F(InstructionNumberingTest, SingleInstructionTest) {
  auto *inst = builder_.createAllocObjectInst(1);
  Expression expr(inst, {value(builder_.getLiteralNumber(1))});

  InstructionNumbering numbering(*block_);
  auto iter = numbering.begin();
  EXPECT_EQ(inst, iter.getInstruction());
  EXPECT_EQ(expr, *iter);
  EXPECT_EQ(numbering.end(), ++iter);
}

TEST_F(InstructionNumberingTest, ArrowOperatorTest) {
  auto *inst = builder_.createAllocObjectInst(1);
  Expression expr(inst, {value(builder_.getLiteralNumber(1))});

  InstructionNumbering numbering(*block_);
  auto iter = numbering.begin();
  EXPECT_EQ(1, iter->operands.size());
}

// %0 = AllocObjectInst 8
// %1 = LoadPropertyInst %0, "foo"
// %2 = BinaryOperatorInst '*', %1, %1
// %3 = AsNumberInst %2
TEST_F(InstructionNumberingTest, InternalOperandTest) {
  auto *foo = builder_.getLiteralString("foo");

  auto *inst0 = builder_.createAllocObjectInst(8);
  auto *inst1 = builder_.createLoadPropertyInst(inst0, foo);
  auto *inst2 = builder_.createBinaryOperatorInst(
      inst1, inst1, BinaryOperatorInst::OpKind::MultiplyKind);
  auto *inst3 = builder_.createAsNumberInst(inst2);

  std::vector<Expression> expressions{
      Expression(inst0, {value(builder_.getLiteralNumber(8))}),
      Expression(inst1, {internal(0), value(foo)}),
      Expression(inst2, {internal(1), internal(1)}),
      Expression(inst3, {internal(2)}),
  };

  EXPECT_EQ(expressions, collectExpressions(inst0, inst3));
}

// %ext0 = HBCLoadConstInst 7
// %ext1 = HBCLoadConstInst 42
// ...
// %0 = UnaryOperatorInst '-', %ext0
// %1 = BinaryOperatorInst '/', %ext1, %ext0
TEST_F(InstructionNumberingTest, ExternalOperandTest) {
  auto *ext0 = builder_.createHBCLoadConstInst(builder_.getLiteralNumber(7));
  auto *ext1 = builder_.createHBCLoadConstInst(builder_.getLiteralNumber(42));

  auto *inst0 = builder_.createUnaryOperatorInst(
      ext0, UnaryOperatorInst::OpKind::MinusKind);
  auto *inst1 = builder_.createBinaryOperatorInst(
      ext1, ext0, BinaryOperatorInst::OpKind::DivideKind);

  std::vector<Expression> expressions{
      Expression(inst0, {external(0)}),
      Expression(inst1, {external(1), external(0)}),
  };

  EXPECT_EQ(expressions, collectExpressions(inst0, inst1));
}

// %0 = AsNumberInst undefined
// %1 = AsNumberInst null
// %2 = AsNumberInst NaN
// %3 = AsNumberInst false
// %4 = AsNumberInst 1
// %5 = AsNumberInst "foo"
TEST_F(InstructionNumberingTest, LiteralValueOperandTest) {
  auto *inst0 = builder_.createAsNumberInst(builder_.getLiteralUndefined());
  auto *inst1 = builder_.createAsNumberInst(builder_.getLiteralNull());
  auto *inst2 = builder_.createAsNumberInst(builder_.getLiteralNaN());
  auto *inst3 = builder_.createAsNumberInst(builder_.getLiteralBool(false));
  auto *inst4 = builder_.createAsNumberInst(builder_.getLiteralNumber(1));
  auto *inst5 = builder_.createAsNumberInst(builder_.getLiteralString("foo"));

  std::vector<Expression> expressions{
      Expression(inst0, {value(builder_.getLiteralUndefined())}),
      Expression(inst1, {value(builder_.getLiteralNull())}),
      Expression(inst2, {value(builder_.getLiteralNaN())}),
      Expression(inst3, {value(builder_.getLiteralBool(false))}),
      Expression(inst4, {value(builder_.getLiteralNumber(1))}),
      Expression(inst5, {value(builder_.getLiteralString("foo"))}),
  };

  EXPECT_EQ(expressions, collectExpressions(inst0, inst5));
}

// %0 = PhiInst %0, %BB0
TEST_F(InstructionNumberingTest, PhiValueOperandTest) {
  auto *inst0 = builder_.createPhiInst();
  inst0->addEntry(inst0, block_);

  // The first operand is external(0), not internal(0), because Internal
  // operands can only be recognized after they've been defined.
  std::vector<Expression> expressions{
      Expression(inst0, {external(0), value(block_)}),
  };

  EXPECT_EQ(expressions, collectExpressions(inst0, inst0));
}

// %0 = AllocStackInst $foo
// %1 = StoreStackInst 42, %0
TEST_F(InstructionNumberingTest, LabelValueOperandTest) {
  auto *number = builder_.getLiteralNumber(42);

  auto *inst0 = builder_.createAllocStackInst("foo");
  auto *inst1 = builder_.createStoreStackInst(number, inst0);

  std::vector<Expression> expressions{
      // There's no other way of getting the Label pointer because only the
      // Identifier it wraps is interned, not the Label object itself.
      Expression(inst0, {value(inst0->getOperand(0))}),
      Expression(inst1, {value(number), internal(0)}),
  };

  EXPECT_EQ(expressions, collectExpressions(inst0, inst1));
}

// %0 = CreateFunctionInst %main()
// %1 = StoreFrameInst 42, [var]
// %2 = LoadFrameInst [var], false
// %3 = CondBranchInst %param, %BB0, %BB0
TEST_F(InstructionNumberingTest, OtherValueOperandTest) {
  auto *var = builder_.createVariable(function_->getFunctionScope(), "var");
  auto *number = builder_.getLiteralNumber(42);
  auto *param = builder_.createParameter(function_, "param");

  auto *inst0 = builder_.createCreateFunctionInst(function_);
  auto *inst1 = builder_.createStoreFrameInst(number, var);
  auto *inst2 = builder_.createLoadFrameInst(var);
  auto *inst3 = builder_.createCondBranchInst(param, block_, block_);

  auto blockVal = value(block_);
  std::vector<Expression> expressions{
      Expression(inst0, {value(function_)}),
      Expression(inst1, {value(number), value(var)}),
      Expression(inst2, {value(var)}),
      Expression(inst3, {value(param), blockVal, blockVal}),
  };

  EXPECT_EQ(expressions, collectExpressions(inst0, inst3));
}

// %0 = BinaryOperatorInst '+', %param, 42
TEST_F(InstructionNumberingTest, ExternalFlagsParametersTest) {
  auto *param = builder_.createParameter(function_, "param");
  auto *number = builder_.getLiteralNumber(42);
  auto *inst = builder_.createBinaryOperatorInst(
      param, number, BinaryOperatorInst::OpKind::AddKind);
  Expression expr(inst, {external(0), value(number)});

  auto flags =
      InstructionNumbering::Instructions | InstructionNumbering::Parameters;
  InstructionNumbering numbering(*block_, flags);
  auto iter = numbering.begin();
  EXPECT_EQ(expr, *iter);
}

// %0 = BinaryOperatorInst '+', %param, 42
TEST_F(InstructionNumberingTest, ExternalFlagsLiteralsTest) {
  auto *param = builder_.createParameter(function_, "param");
  auto *number = builder_.getLiteralNumber(42);
  auto *inst = builder_.createBinaryOperatorInst(
      param, number, BinaryOperatorInst::OpKind::AddKind);
  Expression expr(inst, {value(param), external(0)});

  auto flags =
      InstructionNumbering::Instructions | InstructionNumbering::Literals;
  InstructionNumbering numbering(*block_, flags);
  auto iter = numbering.begin();
  EXPECT_EQ(expr, *iter);
}

// %0 = AsNumberInst 5
// %1 = BinaryOperatorInst '*', %0, %0
// %2 = BinaryOperatorInst '-', %1, %0
TEST_F(InstructionNumberingTest, EquivalentBlocksTest) {
  BasicBlock *otherBlock = builder_.createBasicBlock(function_);
  for (BasicBlock *block : {block_, otherBlock}) {
    builder_.setInsertionBlock(block);
    Instruction *inst0 =
        builder_.createAsNumberInst(builder_.getLiteralNumber(5));
    Instruction *inst1 = builder_.createBinaryOperatorInst(
        inst0, inst0, BinaryOperatorInst::OpKind::MultiplyKind);
    builder_.createBinaryOperatorInst(
        inst1, inst0, BinaryOperatorInst::OpKind::SubtractKind);
  }

  InstructionNumbering numbering(*block_);
  InstructionNumbering otherNumbering(*otherBlock);

  EXPECT_TRUE(
      std::equal(numbering.begin(), numbering.end(), otherNumbering.begin()));
}

// %0 = AsNumberInst 5
// %1 = BinaryOperatorInst '*', %0, %0
// %2 = BinaryOperatorInst '-', %1, %0    [COPY: %0, %1 instead of %1, %0]
TEST_F(InstructionNumberingTest, InequivalentOperandTest) {
  BasicBlock *otherBlock = builder_.createBasicBlock(function_);
  for (BasicBlock *block : {block_, otherBlock}) {
    builder_.setInsertionBlock(block);
    Instruction *inst0 =
        builder_.createAsNumberInst(builder_.getLiteralNumber(5));
    Instruction *inst1 = builder_.createBinaryOperatorInst(
        inst0, inst0, BinaryOperatorInst::OpKind::MultiplyKind);
    if (block == otherBlock) {
      // Swap operands in the last instruction for otherBlock.
      std::swap(inst0, inst1);
    }
    builder_.createBinaryOperatorInst(
        inst1, inst0, BinaryOperatorInst::OpKind::SubtractKind);
  }

  InstructionNumbering numbering(*block_);
  InstructionNumbering otherNumbering(*otherBlock);

  EXPECT_FALSE(
      std::equal(numbering.begin(), numbering.end(), otherNumbering.begin()));
}

// %0 = AsNumberInst 5
// %1 = BinaryOperatorInst '*', %0, %0
// %2 = BinaryOperatorInst '-', %1, %0    [COPY: '+' instead of '-']
TEST_F(InstructionNumberingTest, InequivalentOpcodeTest) {
  BasicBlock *otherBlock = builder_.createBasicBlock(function_);
  for (BasicBlock *block : {block_, otherBlock}) {
    builder_.setInsertionBlock(block);
    Instruction *inst0 =
        builder_.createAsNumberInst(builder_.getLiteralNumber(5));
    Instruction *inst1 = builder_.createBinaryOperatorInst(
        inst0, inst0, BinaryOperatorInst::OpKind::MultiplyKind);
    auto kind = BinaryOperatorInst::OpKind::SubtractKind;
    if (block == otherBlock) {
      // Use a different opcode in the last instruction for otherBlock.
      kind = BinaryOperatorInst::OpKind::AddKind;
    }
    builder_.createBinaryOperatorInst(inst1, inst0, kind);
  }

  InstructionNumbering numbering(*block_);
  InstructionNumbering otherNumbering(*otherBlock);

  EXPECT_FALSE(
      std::equal(numbering.begin(), numbering.end(), otherNumbering.begin()));
}

// Only run death tests when assertions are enabled.
#ifndef NDEBUG

using InstructionNumberingDeathTest = InstructionNumberingTest;

TEST_F(InstructionNumberingDeathTest, CannotCallBeginTwiceTest) {
  InstructionNumbering numbering(*block_);
  numbering.begin();
  EXPECT_DEATH(numbering.begin(), "Assertion.*begin");
}

TEST_F(InstructionNumberingDeathTest, CannotDereferenceEndTest) {
  InstructionNumbering numbering(*block_);
  auto iter = numbering.begin();
  EXPECT_DEATH(iter.getInstruction(), "Assertion.*dereference");
  EXPECT_DEATH(*iter, "Assertion.*dereference");
}

TEST_F(InstructionNumberingDeathTest, CannotIncrementEndTests) {
  InstructionNumbering numbering(*block_);
  auto iter = numbering.begin();
  EXPECT_DEATH(++iter, "Assertion.*increment");
}

#endif // !NDEBUG

} // anonymous namespace
