/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/Context.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/IRVerifier.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Utils/Dumper.h"

#include "gtest/gtest.h"

using llvh::errs;

using namespace hermes;

namespace {

/// Assumes that M is freshly constructed.
/// Constructs a function in it.  Sets F to that function.
void makeSimpleModule0(Module &M, Function *&F) {
  IRBuilder Builder(&M);
  F = Builder.createFunction(
      "foo", Function::DefinitionKind::ES5Function, true);
  auto BB0 = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(BB0);
  auto sum = Builder.createBinaryOperatorInst(
      Builder.getLiteralNumber(1.0),
      Builder.getLiteralNumber(2.0),
      ValueKind::BinaryAddInstKind);
  Builder.createReturnInst(sum);
}

TEST(IRHashTest, SameModuleTest) {
  Function *FIgnored;
  auto Ctx0 = std::make_shared<Context>();
  Module M0{Ctx0};
  makeSimpleModule0(M0, FIgnored);

  auto Ctx1 = std::make_shared<Context>();
  Module M1{Ctx1};
  makeSimpleModule0(M1, FIgnored);

  EXPECT_EQ(M0.hash(), M1.hash());
}

/// All of these tests for different hashes are actually probabilistic:
/// there's a tiny, but non-zero, chance that the hash function might
/// have a collision.  With 64-hashes, that chance is small enough to
/// ignore.  It would be deterministic if it happened.

TEST(IRHashTest, AddDelInstructionTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  Function *F;
  makeSimpleModule0(M, F);
  IRBuilder Builder(&M);
  auto Arg0 = Builder.createJSDynamicParam(F, "o");
  auto hash0 = M.hash();

  auto &BB = *F->begin();
  Instruction *I = &(*BB.begin());
  Builder.setInsertionPointAfter(I);
  Instruction *storeProp =
      Builder.createStorePropertyInst(I, Arg0, Builder.createIdentifier("p"));

  EXPECT_NE(M.hash(), hash0);

  {
    IRBuilder::InstructionDestroyer destroyer;
    destroyer.add(storeProp);
  }
  EXPECT_EQ(M.hash(), hash0);
}

// A tree:
// sum0 = 1 + 2
// sum1 = 10 + 20;
// sum2 = sum0 + sum1
// return sum2
void makeSimpleModule1(Module &M, Function *&F) {
  IRBuilder Builder(&M);
  F = Builder.createFunction(
      "foo", Function::DefinitionKind::ES5Function, true);
  auto BB0 = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(BB0);
  auto sum0 = Builder.createBinaryOperatorInst(
      Builder.getLiteralNumber(1.0),
      Builder.getLiteralNumber(2.0),
      ValueKind::BinaryAddInstKind);
  auto sum1 = Builder.createBinaryOperatorInst(
      Builder.getLiteralNumber(10.0),
      Builder.getLiteralNumber(20.0),
      ValueKind::BinaryAddInstKind);
  auto sum2 = Builder.createBinaryOperatorInst(
      sum0, sum1, ValueKind::BinaryAddInstKind);
  Builder.createReturnInst(sum2);
}

// Different instruction order yield different hash.
TEST(IRHashTest, SwapInstructionTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  Function *F;
  makeSimpleModule1(M, F);
  IRBuilder Builder(&M);
  auto hash0 = M.hash();

  auto &BB = *F->begin();
  auto bbIter = BB.begin();
  Instruction *sum0 = &(*bbIter);
  bbIter++;
  bbIter++;
  Instruction *ret = &(*bbIter);
  sum0->moveBefore(ret);

  EXPECT_NE(M.hash(), hash0);
}

TEST(IRHashTest, SwapArgumentsTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  Function *F;
  makeSimpleModule1(M, F);
  IRBuilder Builder(&M);
  auto hash0 = M.hash();

  auto &BB = *F->begin();
  auto bbIter = BB.begin();
  bbIter++;
  bbIter++;
  Instruction *ret = &(*bbIter);
  EXPECT_EQ(2, ret->getNumOperands());
  Value *arg0 = ret->getOperand(0);
  Value *arg1 = ret->getOperand(0);
  ret->setOperand(arg1, 0);
  ret->setOperand(arg0, 1);

  EXPECT_NE(M.hash(), hash0);
}

TEST(IRHashTest, JSSpecialParamTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  Function *F = Builder.createFunction(
      "foo", Function::DefinitionKind::ES5Function, true);
  auto BB0 = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(BB0);
  LoadPropertyInst *loadProp = Builder.createLoadPropertyInst(
      F->getParentScopeParam(), Builder.createIdentifier("p"));
  Builder.createReturnInst(loadProp);

  auto hash0 = M.hash();

  loadProp->setOperand(F->getNewTargetParam(), 0);

  EXPECT_NE(hash0, M.hash());
}

// Make sure hash of a function as a value works.
TEST(IRHashTest, JSFunctionAsValueTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  Function *F0 =
      Builder.createFunction("f0", Function::DefinitionKind::ES5Function, true);
  {
    auto BB0 = Builder.createBasicBlock(F0);
    Builder.setInsertionBlock(BB0);
    Builder.createReturnInst(Builder.getLiteralNumber(1.0));
  }
  Function *F1 =
      Builder.createFunction("f1", Function::DefinitionKind::ES5Function, true);
  {
    auto BB0 = Builder.createBasicBlock(F0);
    Builder.setInsertionBlock(BB0);
    Builder.createReturnInst(Builder.getLiteralNumber(2.0));
  }

  Function *F2 =
      Builder.createFunction("f2", Function::DefinitionKind::ES5Function, true);
  Instruction *callInst = nullptr;
  {
    auto BB0 = Builder.createBasicBlock(F2);
    Builder.setInsertionBlock(BB0);
    llvh::SmallVector<Value *, 1> args;
    callInst = Builder.createCallInst(
        F0,
        /* newTarget */ Builder.getLiteralUndefined(),
        /* thisValue */ Builder.getLiteralUndefined(),
        args);
    Builder.createReturnInst(callInst);
  }

  auto hash0 = M.hash();

  // Change the target of the call.
  callInst->setOperand(F1, 0);

  // Should change the hash.
  ASSERT_NE(hash0, M.hash());
}

// Just check that llvh::hash_value(double) works, and yields different
// vals for a couple of different inputs.
TEST(IRHashTest, DoubleHash) {
  const auto hc0 = llvh::hash_value(0.0);
  const auto hc10pt5 = llvh::hash_value(10.5);
  EXPECT_NE(hc0, hc10pt5);
}

} // end anonymous namespace
