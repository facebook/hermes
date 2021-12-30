/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Optimizer/Scalar/InstructionEscapeAnalysis.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"

#include "gtest/gtest.h"

#include <iterator>
#include <ostream>
#include <vector>

using namespace hermes;

using Prefix = InstructionEscapeAnalysis::Prefix;

namespace hermes {

/// Custom gtest printer for \p Prefix to make test failures more readable.
static void PrintTo(const Prefix &prefix, std::ostream *os) {
  *os << "Prefix(" << prefix.length;
  if (prefix.offset.hasValue()) {
    *os << ", " << prefix.offset.getValue();
  }
  *os << ")";
}

} // namespace hermes

namespace {

class InstructionEscapeAnalysisTest : public ::testing::Test {
 public:
  InstructionEscapeAnalysisTest()
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
  IRBuilder builder_;
  Function *function_;
  BasicBlock *block_;
  InstructionEscapeAnalysis analysis_;

 protected:
  // DSL for InstructionEscapeAnalysis tests:
  //
  // createInstruction()
  //   Append a standalone instruction to the block.
  // createInstruction(X)
  //   An instruction that depends on X.
  // createInstruction(X, Y)
  //   An instruction that depends on X and Y.
  // createInstructions(N)
  //   Shortcut for calling createInstruction() N times.
  //
  // addRange()
  //   Add all instructions created so far. Return longestPrefix().
  // addRange(V)
  //   Add only the range of instructions contained in vector V (they must be
  //   contiguous). Return longestPrefix().
  // removeLastRange(), longestPrefix()
  //   Just calls the InstructionEscapeAnalysis function.

  Instruction *createInstruction() {
    return builder_.createAllocObjectInst(1);
  }
  Instruction *createInstruction(Value *dependency) {
    return builder_.createAsNumberInst(dependency);
  }
  Instruction *createInstruction(Value *dependency1, Value *dependency2) {
    return builder_.createBinaryOperatorInst(
        dependency1, dependency2, BinaryOperatorInst::OpKind::AddKind);
  }
  std::vector<Instruction *> createInstructions(unsigned n) {
    std::vector<Instruction *> vec;
    vec.reserve(n);
    for (unsigned i = 0; i < n; ++i) {
      vec.push_back(createInstruction());
    }
    return vec;
  }

  Prefix addRange() {
    analysis_.addRange(*block_);
    return analysis_.longestPrefix();
  }
  Prefix addRange(const std::vector<Instruction *> &vec) {
    analysis_.addRange(
        {vec.front()->getIterator(), std::next(vec.back()->getIterator())});
    return analysis_.longestPrefix();
  }
  void removeLastRange() {
    analysis_.removeLastRange();
  }
  const Prefix &longestPrefix() {
    return analysis_.longestPrefix();
  }
};

TEST_F(InstructionEscapeAnalysisTest, SingleInstructionTest) {
  createInstruction();
  EXPECT_EQ(Prefix(1), addRange());
}

TEST_F(InstructionEscapeAnalysisTest, ZeroEscapeTest) {
  // Prefixes: Prefix(1), ..., Prefix(5).
  createInstructions(5);
  EXPECT_EQ(Prefix(5), addRange());
}

TEST_F(InstructionEscapeAnalysisTest, OneEscapeTest) {
  // Prefixes: Prefix(1, 0), ..., Prefix(5, 0).
  auto vec = createInstructions(5);
  createInstruction(vec[0]);

  EXPECT_EQ(Prefix(5, 0), addRange(vec));
}

TEST_F(InstructionEscapeAnalysisTest, TwoEscapeTest) {
  // Prefixes: Prefix(1, 0), ..., Prefix(4, 0).
  auto vec = createInstructions(5);
  createInstruction(vec[0]);
  createInstruction(vec[4]);

  EXPECT_EQ(Prefix(4, 0), addRange(vec));
}

TEST_F(InstructionEscapeAnalysisTest, OneEscapeUsedTwiceTest) {
  // Prefixes: Prefix(1), Prefix(2, 1), ..., Prefix(5, 1).
  auto vec = createInstructions(5);
  createInstruction(vec[1]);
  createInstruction(vec[1]);

  EXPECT_EQ(Prefix(5, 1), addRange(vec));
}

TEST_F(InstructionEscapeAnalysisTest, OneEscapeUsedTwiceInOneInstructionTest) {
  // Prefixes: Prefix(1), Prefix(2), Prefix(3, 2), ..., Prefix(5, 2).
  auto vec = createInstructions(5);
  createInstruction(vec[2], vec[2]);

  EXPECT_EQ(Prefix(5, 2), addRange(vec));
}

TEST_F(InstructionEscapeAnalysisTest, TwoEscapesUsedInOneInstructionTest) {
  // Prefixes: Prefix(1), Prefix(2, 1).
  auto vec = createInstructions(5);
  createInstruction(vec[1], vec[2]);

  EXPECT_EQ(Prefix(2, 1), addRange(vec));
}

TEST_F(InstructionEscapeAnalysisTest, AddRangeIdempotentTest) {
  // Prefixes: Prefix(1, 0), ..., Prefix(4, 0).
  auto vec = createInstructions(5);
  createInstruction(vec[0], vec[4]);

  for (unsigned i = 0; i < 5; ++i) {
    EXPECT_EQ(Prefix(4, 0), addRange(vec));
  }
}

TEST_F(InstructionEscapeAnalysisTest, CallLongestPrefixMultipleTimesTest) {
  // Prefixes: Prefix(1, 0), ..., Prefix(4, 0).
  auto vec = createInstructions(5);
  createInstruction(vec[0], vec[4]);

  addRange(vec);
  for (unsigned i = 0; i < 5; ++i) {
    EXPECT_EQ(Prefix(4, 0), longestPrefix());
  }
}

TEST_F(InstructionEscapeAnalysisTest, AddTwoRangesTest) {
  auto vec1 = createInstructions(5);
  auto vec2 = createInstructions(5);

  EXPECT_EQ(Prefix(5), addRange(vec1));
  EXPECT_EQ(Prefix(5), addRange(vec2));
}

TEST_F(InstructionEscapeAnalysisTest, AddUnequalSizeRangesTest) {
  auto vec1 = createInstructions(5);
  auto vec2 = createInstructions(3);

  EXPECT_EQ(Prefix(5), addRange(vec1));
  EXPECT_EQ(Prefix(3), addRange(vec2));
}

TEST_F(InstructionEscapeAnalysisTest, ZeroEscapeAndOneEscapeTest) {
  // Prefixes: Prefix(1, 0), Prefix(3).
  auto vec1 = createInstructions(2);
  vec1.push_back(createInstruction(vec1[0], vec1[1]));

  // Prefixes: Prefix(1), Prefix(2), Prefix(3, 2), Prefix(6).
  auto vec2 = createInstructions(5);
  vec2.push_back(createInstruction(vec2[2], vec2[3]));

  EXPECT_EQ(Prefix(3), addRange(vec1));
  EXPECT_EQ(Prefix(3, 2), addRange(vec2));
}

TEST_F(InstructionEscapeAnalysisTest, OneEscapeAndZeroEscapeTest) {
  // Prefixes: Prefix(1), Prefix(2), Prefix(3, 2).
  auto vec1 = createInstructions(4);
  createInstruction(vec1[2], vec1[3]);

  // Prefixes: Prefix(1, 0), Prefix(3).
  auto vec2 = createInstructions(2);
  vec2.push_back(createInstruction(vec2[0], vec2[1]));

  EXPECT_EQ(Prefix(3, 2), addRange(vec1));
  EXPECT_EQ(Prefix(3, 2), addRange(vec2));
}

TEST_F(InstructionEscapeAnalysisTest, ConflictingOffsetTest) {
  // Prefixes: Prefix(1), Prefix(2, 1).
  auto vec1 = createInstructions(5);
  createInstruction(vec1[1], vec1[2]);

  // Prefixes: Prefix(1, 0), Prefix(2, 0).
  auto vec2 = createInstructions(5);
  createInstruction(vec2[0], vec2[2]);

  EXPECT_EQ(Prefix(2, 1), addRange(vec1));
  EXPECT_EQ(Prefix(1, 0), addRange(vec2));
}

TEST_F(InstructionEscapeAnalysisTest, AddThreeRangesTest) {
  auto vec1 = createInstructions(5);
  auto vec2 = createInstructions(4);
  auto vec3 = createInstructions(3);

  EXPECT_EQ(Prefix(5), addRange(vec1));
  EXPECT_EQ(Prefix(4), addRange(vec2));
  EXPECT_EQ(Prefix(3), addRange(vec3));
}

TEST_F(InstructionEscapeAnalysisTest, RemoveLastRangeTest) {
  auto vec1 = createInstructions(3);
  auto vec2 = createInstructions(5);

  EXPECT_EQ(Prefix(3), addRange(vec1));
  removeLastRange();
  EXPECT_EQ(Prefix(5), addRange(vec2));
}

TEST_F(InstructionEscapeAnalysisTest, RemoveLastRangeWithOneLeftTest) {
  auto vec1 = createInstructions(4);
  auto vec2 = createInstructions(3);

  EXPECT_EQ(Prefix(4), addRange(vec1));
  EXPECT_EQ(Prefix(3), addRange(vec2));
  removeLastRange();
  EXPECT_EQ(Prefix(4), longestPrefix());
}

TEST_F(InstructionEscapeAnalysisTest, AddRemoveInverseTest) {
  // Prefixes: Prefix(1, 0), ..., Prefix(4, 0).
  auto vec = createInstructions(5);
  createInstruction(vec[0], vec[4]);

  addRange(vec);
  for (unsigned i = 1; i <= 5; ++i) {
    auto newVec = createInstructions(i);
    addRange(newVec);
    removeLastRange();
  }
  EXPECT_EQ(Prefix(4, 0), longestPrefix());
}

// Only run death tests when assertions are enabled.
#ifndef NDEBUG

using InstructionEscapeAnalysisDeathTest = InstructionEscapeAnalysisTest;

TEST_F(InstructionEscapeAnalysisDeathTest, CannotAnalyzeEmptyRangeTest) {
  EXPECT_DEATH_IF_SUPPORTED(addRange(), "Assertion.*nonempty");
}

TEST_F(InstructionEscapeAnalysisDeathTest, CannotRemoveBeforeAddTest) {
  EXPECT_DEATH_IF_SUPPORTED(removeLastRange(), "Assertion.*add");
}

TEST_F(InstructionEscapeAnalysisDeathTest, CannotRemoveTwiceInARowTest) {
  createInstruction();
  addRange();
  addRange();
  removeLastRange();
  EXPECT_DEATH_IF_SUPPORTED(removeLastRange(), "Assertion.*once");
}

TEST_F(InstructionEscapeAnalysisDeathTest, CannotGetPrefixBeforeAddRangeTest) {
  EXPECT_DEATH_IF_SUPPORTED(longestPrefix(), "Assertion.*add");
}

#endif // !NDEBUG

} // anonymous namespace
