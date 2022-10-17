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

/// Add a tiny test to make sure that we can construct a tiny program using the
/// IRBuilder.
TEST(BuilderTest, SimpleSmokeTest) {
  std::string Result;
  llvh::raw_string_ostream OS(Result);

  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "main",
      Function::DefinitionKind::ES5Function,
      true);
  auto BB = Builder.createBasicBlock(F);
  Builder.createParameter(F, "argc");
  Builder.createParameter(F, "argv");
  Builder.setInsertionBlock(BB);
  Builder.createBranchInst(BB);

  EXPECT_TRUE(F);
  EXPECT_TRUE(BB);

  IRPrinter D(*Ctx, OS);
  D.visit(M);

  std::string Res = OS.str();
  std::size_t found = Res.find("main");
  EXPECT_TRUE(found != std::string::npos);
}

TEST(BuilderTest, BuildCFG) {
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
}

TEST(BuilderTest, ReplaceAllUsesWith) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "testRAUW",
      Function::DefinitionKind::ES5Function,
      true);
  auto Cond = Builder.createParameter(F, "cond");
  auto Arg2 = Builder.createParameter(F, "two");
  auto Arg3 = Builder.createParameter(F, "three");

  auto Entry = Builder.createBasicBlock(F);
  auto Left = Builder.createBasicBlock(F);
  auto Right = Builder.createBasicBlock(F);

  Builder.setInsertionBlock(Entry);
  auto A0 = Builder.createAllocStackInst("some_variable");
  auto A1 = Builder.createAllocStackInst("another_variable");
  Builder.createCondBranchInst(Cond, Left, Right);

  Builder.setInsertionBlock(Left);
  auto RetL = Builder.createReturnInst(Arg2);

  Builder.setInsertionBlock(Right);
  auto RetR = Builder.createReturnInst(Arg3);

  // The left BB now returns A0.
  Arg2->replaceAllUsesWith(A0);

  // The right BB now returns A0.
  Arg3->replaceAllUsesWith(A0);

  // This is ok, Arg1 has no users.
  Arg3->replaceAllUsesWith(A0);

  // The left return value returns nothing.
  RetL->setOperand(nullptr, 0);

  // The left return now returns A1.
  RetL->setOperand(A1, 0);

  // Return values have no users.
  EXPECT_EQ((int)RetL->getNumUsers(), 0);
  EXPECT_EQ((int)RetR->getNumUsers(), 0);

  // The allocas have one user each (the return instructions).
  EXPECT_EQ((int)A0->getNumUsers(), 1);
  EXPECT_EQ((int)A1->getNumUsers(), 1);

  // The condition variable has a single user (the branch).
  EXPECT_EQ((int)Cond->getNumUsers(), 1);

  // Arg2 and Arg3 have no users now.
  EXPECT_EQ((int)Arg2->getNumUsers(), 0);
  EXPECT_EQ((int)Arg3->getNumUsers(), 0);

  // Validate successors and predecessors.
  EXPECT_TRUE(pred_empty(Entry));
  EXPECT_EQ((int)succ_count(Entry), 2);

  EXPECT_TRUE(succ_empty(Left));
  EXPECT_TRUE(succ_empty(Right));

  EXPECT_EQ((int)pred_count(Left), 1);
  EXPECT_EQ((int)pred_count(Right), 1);
}

TEST(BuilderTest, Identifiers) {
  Context Ctx;

  Identifier First = Ctx.getIdentifier("first");
  Identifier Second = Ctx.getIdentifier("second");
  Identifier FirstAgain = Ctx.getIdentifier("first");

  // Make sure that the size of the identifier remains small.
  static_assert(sizeof(Identifier) <= sizeof(size_t), "Identifier is too big!");

  EXPECT_NE(First, Second);
  EXPECT_EQ(First, FirstAgain);

  EXPECT_EQ(Ctx.toString(First), "first");
  EXPECT_EQ(Ctx.toString(Second), "second");
}

TEST(BuilderTest, TestValueTypes) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "a_function_with_tests",
      Function::DefinitionKind::ES5Function,
      true);

  auto Cond = Builder.createParameter(F, "cond");
  Cond->setType(Type::createBoolean());

  auto X = Builder.createParameter(F, "x");
  X->setType(Type::createNumber());

  Builder.setInsertionBlock(Builder.createBasicBlock(F));
  Builder.createReturnInst(X);

  EXPECT_EQ(Type::createBoolean(), Cond->getType());
  EXPECT_EQ(Type::createNumber(), X->getType());

  EXPECT_NE(Type::createNumber(), Type::createBoolean());
  EXPECT_EQ(Type::createNumber(), Type::createNumber());

  EXPECT_TRUE(Cond->getType().isBooleanType());
  EXPECT_FALSE(Cond->getType().isNumberType());
  EXPECT_FALSE(X->getType().isBooleanType());

  EXPECT_TRUE(X->getType().isNumberType());

  X->setType(Type::createBoolean());
  EXPECT_FALSE(X->getType().isNumberType());
  EXPECT_TRUE(X->getType().isBooleanType());
  EXPECT_TRUE(X->getType().isPrimitive());
  EXPECT_FALSE(X->getType().isNullType());

  X->setType(Type::createClosure());
  EXPECT_FALSE(X->getType().isNumberType());
  EXPECT_TRUE(X->getType().isClosureType());
  EXPECT_TRUE(X->getType().isObjectType());

  X->setType(Type::createBoolean());
  EXPECT_TRUE(X->getType().isBooleanType());
  EXPECT_FALSE(X->getType().isNoType());
}

TEST(BuilderTest, Types) {
  Type T = Type::createAnyType();
  EXPECT_FALSE(T.isNoType());
  EXPECT_TRUE(T.isAnyType());

  Type W = Type::unionTy(Type::createNumber(), Type::createBoolean());
  EXPECT_FALSE(W.isAnyType());
  EXPECT_TRUE(W.isPrimitive());
  EXPECT_FALSE(W.isStringType());
  EXPECT_FALSE(W.isObjectType());

  EXPECT_EQ(
      Type::createAnyType(),
      Type::unionTy(Type::createAnyType(), Type::createBoolean()));

  EXPECT_EQ(
      Type::createAnyType(),
      Type::unionTy(Type::createAnyType(), Type::createBoolean()));

  Type U = Type::unionTy(Type::createUndefined(), Type::createClosure());
  EXPECT_FALSE(W.isObjectType());
  EXPECT_FALSE(W.isBooleanType());
  Type J = Type::intersectTy(U, Type::createClosure());
  EXPECT_TRUE(J.isObjectType());
  EXPECT_TRUE(J.isClosureType());
  EXPECT_FALSE(J.isPrimitive());

  Type R = Type::unionTy(Type::createNumber(), Type::createObject());
  EXPECT_FALSE(R.isAnyType());
  EXPECT_FALSE(R.isPrimitive());
  EXPECT_FALSE(R.isNumberType());
  EXPECT_FALSE(R.isObjectType());
}

TEST(BuilderTest, CreateAndManipulateFrameTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "a_function_with_a_scope",
      Function::DefinitionKind::ES5Function,
      true);

  Builder.createParameter(F, "cond");

  auto X = Builder.createParameter(F, "x");
  Builder.setInsertionBlock(Builder.createBasicBlock(F));

  Identifier SomeName0 = Ctx->getIdentifier("some_name");
  Identifier SomeName1 = Ctx->getIdentifier("another_name");

  auto A0 = Builder.createAllocStackInst(SomeName0);
  auto A1 = Builder.createAllocStackInst(SomeName1);

  auto Val0 = Builder.createLoadStackInst(A0);
  auto Val1 = Builder.createLoadStackInst(A1);

  Builder.createStoreStackInst(X, A1);
  Builder.createStoreStackInst(Val1, A0);

  Builder.createReturnInst(Val0);
}

TEST(BuilderTest, NestedFunctionFrameTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  auto Caller = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "caller",
      Function::DefinitionKind::ES5Function,
      true);
  auto Callee = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "callee",
      Function::DefinitionKind::ES5Function,
      true);

  Builder.setInsertionBlock(Builder.createBasicBlock(Caller));
  auto A0 = Builder.createAllocStackInst("one");
  auto A1 = Builder.createAllocStackInst("two");
  Builder.createReturnInst(Builder.createLoadStackInst(A0));

  Builder.setInsertionBlock(Builder.createBasicBlock(Callee));
  auto Val0 = Builder.createLoadStackInst(A0);
  auto Val1 = Builder.createLoadStackInst(A1);

  // Swap
  Builder.createStoreStackInst(Val0, A1);
  Builder.createStoreStackInst(Val1, A0);

  // Return something.
  Builder.createReturnInst(Builder.createLoadStackInst(A0));
}

TEST(BuilderTest, LiteralsTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "testRAUW",
      Function::DefinitionKind::ES5Function,
      true);
  Builder.createParameter(F, "cond");
  Builder.createParameter(F, "two");
  Builder.createParameter(F, "three");

  auto Entry = Builder.createBasicBlock(F);
  auto Left = Builder.createBasicBlock(F);
  auto Right = Builder.createBasicBlock(F);

  auto RL = Builder.createBasicBlock(F);
  auto RR = Builder.createBasicBlock(F);

  Builder.setInsertionBlock(Entry);

  auto BoolCond = Builder.getLiteralBool(true);
  Builder.createCondBranchInst(BoolCond, Left, Right);

  Builder.setInsertionBlock(Left);
  Builder.createReturnInst(Builder.getLiteralString("Left Side"));

  Builder.setInsertionBlock(Right);
  Builder.createCondBranchInst(Builder.getLiteralBool(true), RR, RL);

  Builder.setInsertionBlock(RL);
  Builder.createReturnInst(Builder.getLiteralNumber(3.1415926));
  Builder.setInsertionBlock(RR);
  Builder.createReturnInst(Builder.getLiteralNull());
}

TEST(BuilderTest, LiteralConstructionTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);

  auto First = Builder.getLiteralString("first");
  auto Second = Builder.getLiteralString("second");
  EXPECT_NE(First, Second);

  // Make sure that new string literals return the same value.
  for (int i = 0; i < 100; i++) {
    EXPECT_EQ(First, Builder.getLiteralString("first"));
  }

  // Make sure that our literals return the stored value.
  double initialValue = 10.1213;
  LiteralNumber *Num = Builder.getLiteralNumber(initialValue);
  EXPECT_NEAR(Num->getValue(), initialValue, 0.01);

  double sum = 0;
  // Make sure that we are able to load and store a wide range of values.
  for (int i = 1; i < 1000; i++) {
    sum += i + (1 / i);
    EXPECT_NEAR(
        Builder.getLiteralNumber(sum)->getValue(),
        static_cast<double>(sum),
        0.01);
  }

  // Make sure that casting works.
  EXPECT_TRUE(llvh::dyn_cast<LiteralNull>(Builder.getLiteralNull()));
  EXPECT_TRUE(llvh::dyn_cast<LiteralUndefined>(Builder.getLiteralUndefined()));
  EXPECT_TRUE(llvh::dyn_cast<LiteralBool>(Builder.getLiteralBool(true)));
  EXPECT_TRUE(llvh::dyn_cast<LiteralBool>(Builder.getLiteralBool(false)));
  EXPECT_TRUE(
      llvh::dyn_cast<LiteralString>(Builder.getLiteralString("hello world")));
  EXPECT_TRUE(llvh::dyn_cast<LiteralNumber>(Builder.getLiteralNumber(10.23)));
}

TEST(BuilderTest, PropertyTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);

  auto *F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "testProperties",
      Function::DefinitionKind::ES5Function,
      true);
  auto *O = Builder.createParameter(F, "object");
  auto *P = Builder.createParameter(F, "property");

  auto *Entry = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(Entry);

  auto *V = Builder.createLoadPropertyInst(O, P);

  auto *T = Builder.createLoadPropertyInst(O, "some");

  Builder.createStorePropertyInst(Builder.getLiteralNumber(10), O, P);
  auto *G =
      Builder.createStorePropertyInst(Builder.getLiteralNumber(10), O, "ten");
  Builder.createStorePropertyInst(T, O, "ten");

  T->replaceAllUsesWith(Builder.getLiteralUndefined());
  T->eraseFromParent();

  // Remove G once we leave the scope.
  IRBuilder::InstructionDestroyer destroyer;
  destroyer.add(G);

  Builder.createReturnInst(V);
}

} // end anonymous namespace
