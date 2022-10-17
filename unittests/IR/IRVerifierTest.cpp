/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/IR/IRVerifier.h"
#include "hermes/AST/Context.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Utils/Dumper.h"

#include "gtest/gtest.h"

using llvh::errs;

using namespace hermes;

namespace {

// The verifier is only enabled if HERMES_SLOW_DEBUG is enabled
#ifdef HERMES_SLOW_DEBUG

TEST(IRVerifierTest, BasicBlockTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "forEach",
      Function::DefinitionKind::ES5Function,
      true);
  auto Arg1 = Builder.createParameter(F, "num");
  auto Arg2 = Builder.createParameter(F, "value");

  auto Entry = Builder.createBasicBlock(F);
  auto Loop = Builder.createBasicBlock(F);
  auto Body = Builder.createBasicBlock(F);
  auto Exit = Builder.createBasicBlock(F);

  Builder.setInsertionBlock(Entry);
  Builder.createBranchInst(Loop);

  Builder.setInsertionBlock(Loop);
  Builder.createCondBranchInst(Arg1, Body, Exit);

  Builder.setInsertionBlock(Body);
  Builder.createBranchInst(Loop);

  Builder.setInsertionBlock(Exit);
  Builder.createReturnInst(Arg2);

  // So far so good, this will pass
  EXPECT_FALSE(verifyModule(M));

  auto Bad = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(Bad);
  Builder.createReturnInst(Arg2);

  // A dead basic block was added, and hence will fail to verify
  EXPECT_TRUE(verifyModule(M, &errs(), VerificationMode::IR_OPTIMIZED));
}

TEST(IRVerifierTest, ReturnInstTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "testReturn",
      Function::DefinitionKind::ES5Function,
      true);
  auto Arg1 = Builder.createParameter(F, "num");
  Arg1->setType(Type::createNumber());

  auto Body = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(Body);
  auto Return = Builder.createReturnInst(Arg1);
  Return->setType(Type::createNumber());

  // Everything should pass so far
  EXPECT_FALSE(verifyModule(M));

  Return->setType(Type::createNumber());
  Builder.createReturnInst(Arg1);
  // This will also fail as there are now multiple return instrs in the BB
  EXPECT_TRUE(verifyModule(M));
}

TEST(IRVerifierTest, BranchInstTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "testBranch",
      Function::DefinitionKind::ES5Function,
      true);

  auto BB1 = Builder.createBasicBlock(F);
  auto BB2 = Builder.createBasicBlock(F);

  Builder.setInsertionBlock(BB1);
  Builder.createBranchInst(BB2);

  Builder.setInsertionBlock(BB2);
  Builder.createBranchInst(BB1);

  // Everything should pass
  EXPECT_FALSE(verifyModule(M));

  Builder.createBranchInst(BB2);

  // This will fail as there are now multple branch instrs in the same BB
  EXPECT_TRUE(verifyModule(M));
}

TEST(IRVerifierTest, DominanceTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "testBranch",
      Function::DefinitionKind::ES5Function,
      true);
  auto Arg1 = Builder.createParameter(F, "num");

  auto Body = Builder.createBasicBlock(F);

  Builder.setInsertionBlock(Body);
  auto AsString = Builder.createAddEmptyStringInst(Arg1);
  AsString->setType(Type::createString());
  Builder.createReturnInst(AsString);

  // This tries to verify that if an instruction A is an operand of another
  // instruction B, A should dominate B.
  EXPECT_FALSE(verifyModule(M, &errs()));
}

TEST(IRVerifierTest, ScopeWithoutVariableScope) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "testScopeWithoutVariableScope",
      Function::DefinitionKind::ES5Function,
      true);
  Builder.setInsertionBlock(Builder.createBasicBlock(F));
  Builder.createUnreachableInst();

  F->getFunctionScopeDesc()->createInnerScope();
  EXPECT_TRUE(verifyModule(M, &errs()));
}
#endif // HERMES_SLOW_DEBUG

} // end anonymous namespace
