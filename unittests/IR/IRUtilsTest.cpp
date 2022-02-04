/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/Context.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Utils/Dumper.h"

#include <limits>

#include "gtest/gtest.h"

using namespace hermes;

namespace {

TEST(IRUtilsTest, Dominators1) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      "testDominators1", Function::DefinitionKind::ES5Function, true);
  auto Cond = Builder.createParameter(F, "cond");
  auto Arg2 = Builder.createParameter(F, "two");
  auto Arg3 = Builder.createParameter(F, "three");

  auto Entry = Builder.createBasicBlock(F);
  auto Left = Builder.createBasicBlock(F);
  auto Right = Builder.createBasicBlock(F);

  Builder.setInsertionBlock(Entry);
  auto A0 = Builder.createAllocStackInst("A0");
  auto A1 = Builder.createAllocStackInst("A1");
  Builder.createCondBranchInst(Cond, Left, Right);

  Builder.setInsertionBlock(Left);
  auto RetL = Builder.createReturnInst(Arg2);

  Builder.setInsertionBlock(Right);
  auto RetR = Builder.createReturnInst(Arg3);

  DominanceInfo DI(F);
  EXPECT_TRUE(DI.properlyDominates(A0, RetL));
  EXPECT_TRUE(DI.properlyDominates(A1, RetR));

  EXPECT_TRUE(DI.properlyDominates(Entry, Left));
  EXPECT_TRUE(DI.properlyDominates(Entry, Right));

  EXPECT_FALSE(DI.properlyDominates(Left, Entry));
  EXPECT_FALSE(DI.properlyDominates(Right, Entry));
}

TEST(IRUtilsTest, Dominators2) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      "testDominators1", Function::DefinitionKind::ES5Function, true);
  auto Param = Builder.createParameter(F, "param");
  auto Entry = Builder.createBasicBlock(F);

  auto LastBB = Entry;
  for (int i = 0; i < 100; i++) {
    auto NewBB = Builder.createBasicBlock(F);
    Builder.setInsertionBlock(LastBB);
    Builder.createBranchInst(NewBB);
    LastBB = NewBB;
  }

  Builder.setInsertionBlock(LastBB);
  Builder.createReturnInst(Param);

  DominanceInfo DI(F);
  EXPECT_TRUE(DI.properlyDominates(Entry, LastBB));
  EXPECT_FALSE(DI.properlyDominates(LastBB, Entry));
}

TEST(Numerics, IntegerConversion) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);

  double mx = std::numeric_limits<double>::max();
  double inf = std::numeric_limits<double>::infinity();

  // Doubles are round to zero.
  auto *C0 = Builder.getLiteralNumber(0.99);
  // Non integers are converted properly.
  auto *C1 = Builder.getLiteralNumber(0.01);
  // Integers are converted properly.
  auto *C2 = Builder.getLiteralNumber(1);
  // Large Builder are truncated.
  auto *C3 = Builder.getLiteralNumber((1ll << 35) + 1ll);
  // Negative numbers are converted and truncated if needed.
  auto *C4 = Builder.getLiteralNumber(-12);
  // High numbers are truncated.
  auto *C5 = Builder.getLiteralNumber(mx);
  // Non-integral numbers are converted to zero.
  auto *C6 = Builder.getLiteralNumber(inf);

  EXPECT_EQ(0, C0->truncateToInt32());
  EXPECT_EQ(0, C1->truncateToInt32());
  EXPECT_EQ(1, C2->truncateToInt32());
  EXPECT_EQ(1, C3->truncateToInt32());
  EXPECT_EQ(-12, C4->truncateToInt32());
  EXPECT_EQ(0, C5->truncateToInt32());
  EXPECT_EQ(0, C6->truncateToInt32());
}

} // end anonymous namespace
