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

#include "gtest/gtest.h"

using namespace hermes;

namespace {

/// When a basic block contains a jump to another, they should be in
/// each others' predecessor and successor lists.
TEST(IRBasicBlockTest, BasicBlockConnectionTest) {
  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      "main", Function::DefinitionKind::ES5Function, true);
  auto BB1 = Builder.createBasicBlock(F);
  auto BB2 = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(BB1);
  Builder.createBranchInst(BB2);

  // BB1 is a predecessor of BB2
  EXPECT_TRUE(pred_contains(BB2, BB1));
  // BB2 is not a predecessor of itself
  EXPECT_FALSE(pred_contains(BB2, BB2));

  // BB2 is a successor of BB1
  EXPECT_TRUE(succ_contains(BB1, BB2));
  // BB1 is not a successor of itself
  EXPECT_FALSE(succ_contains(BB1, BB1));
}

} // end anonymous namespace
