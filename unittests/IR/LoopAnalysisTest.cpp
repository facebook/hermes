/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/Context.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"

#include "gtest/gtest.h"

using namespace hermes;

namespace {

/// These tests make sure LoopAnalysis returns the correct loop information.
/// Each test has an ASCII diagram of the control flow graph.

//    Main
//     +
//     |
// +-> v
// |  Loop
// +---+
//     |
//     v
//    Return
TEST(IRVerifierTest, LoopAnalysisTestSimple) {
  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "main",
      Function::DefinitionKind::ES5Function,
      true);
  auto BBMain = Builder.createBasicBlock(F);
  auto BBLoop = Builder.createBasicBlock(F);
  auto BBReturn = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(BBMain);
  Builder.createBranchInst(BBLoop);

  Builder.setInsertionBlock(BBLoop);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBLoop, BBReturn);

  Builder.setInsertionBlock(BBReturn);
  Builder.createReturnInst(M.getLiteralBool(true));

  DominanceInfo dominanceInfo(F);
  LoopAnalysis loopAnalysis(F, dominanceInfo);

  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBMain));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBLoop));
  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBReturn));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBMain));
  EXPECT_EQ(BBLoop, loopAnalysis.getLoopHeader(BBLoop));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBReturn));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBMain));
  EXPECT_EQ(BBMain, loopAnalysis.getLoopPreheader(BBLoop));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBReturn));
}

//              +---------+ Main +-----+
//              |                      |
//              v                      v
//         FirstIfT                FirstIfF
//              +                      +
//              |                      |
//              v                      |
//    +-+ FirstForBegin +---+          |
//    |         ^           |          |
//    |         |           |          |
//    v         |           v          |
// SecondIfT    |     SecondIfF        |
//    +         |           +          |
//    |         |           |          |
//    |         +           |          |
//    +-> FirstForEnd <-----+          |
//              +                      |
//              |                      |
//              +------> FinalReturn <-+
TEST(IRVerifierTest, LoopAnalysisTestBranch) {
  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "main",
      Function::DefinitionKind::ES5Function,
      true);
  auto BBMain = Builder.createBasicBlock(F);
  auto BBFirstIfT = Builder.createBasicBlock(F);
  auto BBFirstIfF = Builder.createBasicBlock(F);
  auto BBFirstForBegin = Builder.createBasicBlock(F);
  auto BBFirstForEnd = Builder.createBasicBlock(F);
  auto BBSecondIfT = Builder.createBasicBlock(F);
  auto BBSecondIfF = Builder.createBasicBlock(F);
  auto BBFinalReturn = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(BBMain);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBFirstIfT, BBFirstIfF);

  Builder.setInsertionBlock(BBFirstIfF);
  Builder.createBranchInst(BBFinalReturn);

  Builder.setInsertionBlock(BBFirstIfT);
  Builder.createBranchInst(BBFirstForBegin);

  Builder.setInsertionBlock(BBFirstForBegin);
  Builder.createCondBranchInst(
      M.getLiteralBool(true), BBSecondIfT, BBSecondIfF);

  Builder.setInsertionBlock(BBSecondIfT);
  Builder.createBranchInst(BBFirstForEnd);

  Builder.setInsertionBlock(BBSecondIfF);
  Builder.createBranchInst(BBFirstForEnd);

  Builder.setInsertionBlock(BBFirstForEnd);
  Builder.createCondBranchInst(
      M.getLiteralBool(true), BBFirstForBegin, BBFinalReturn);

  Builder.setInsertionBlock(BBFinalReturn);
  Builder.createReturnInst(M.getLiteralBool(true));

  DominanceInfo dominanceInfo(F);
  LoopAnalysis loopAnalysis(F, dominanceInfo);

  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBMain));
  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBFirstIfT));
  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBFirstIfF));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBFirstForBegin));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBFirstForEnd));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBSecondIfT));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBSecondIfF));
  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBFinalReturn));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBMain));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBFirstIfT));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBFirstIfF));
  EXPECT_EQ(BBFirstForBegin, loopAnalysis.getLoopHeader(BBFirstForBegin));
  EXPECT_EQ(BBFirstForBegin, loopAnalysis.getLoopHeader(BBFirstForEnd));
  EXPECT_EQ(BBFirstForBegin, loopAnalysis.getLoopHeader(BBSecondIfT));
  EXPECT_EQ(BBFirstForBegin, loopAnalysis.getLoopHeader(BBSecondIfF));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBFinalReturn));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBMain));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBFirstIfT));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBFirstIfF));
  EXPECT_EQ(BBFirstIfT, loopAnalysis.getLoopPreheader(BBFirstForBegin));
  EXPECT_EQ(BBFirstIfT, loopAnalysis.getLoopPreheader(BBFirstForEnd));
  EXPECT_EQ(BBFirstIfT, loopAnalysis.getLoopPreheader(BBSecondIfT));
  EXPECT_EQ(BBFirstIfT, loopAnalysis.getLoopPreheader(BBSecondIfF));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBFinalReturn));
}

//    Main
//     +
//     |
// +-> v
// |  Loop1 <--+
// +---+       |
//     |       |
//     v       |
//    Inside   |
//     +       |
//     |       |
// +-> v       |
// |  Loop2 +--+
// +---+
TEST(IRVerifierTest, LoopAnalysisTestInnerLoops) {
  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "main",
      Function::DefinitionKind::ES5Function,
      true);
  auto BBMain = Builder.createBasicBlock(F);
  auto BBLoop1 = Builder.createBasicBlock(F);
  auto BBInside = Builder.createBasicBlock(F);
  auto BBLoop2 = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(BBMain);
  Builder.createBranchInst(BBLoop1);

  Builder.setInsertionBlock(BBLoop1);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBLoop1, BBInside);

  Builder.setInsertionBlock(BBInside);
  Builder.createBranchInst(BBLoop2);

  Builder.setInsertionBlock(BBLoop2);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBLoop2, BBLoop1);

  DominanceInfo dominanceInfo(F);
  LoopAnalysis loopAnalysis(F, dominanceInfo);

  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBMain));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBLoop1));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBInside));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBLoop2));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBMain));
  EXPECT_EQ(BBLoop1, loopAnalysis.getLoopHeader(BBLoop1));
  EXPECT_EQ(BBLoop1, loopAnalysis.getLoopHeader(BBInside));
  EXPECT_EQ(BBLoop2, loopAnalysis.getLoopHeader(BBLoop2));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBMain));
  EXPECT_EQ(BBMain, loopAnalysis.getLoopPreheader(BBLoop1));
  EXPECT_EQ(BBMain, loopAnalysis.getLoopPreheader(BBInside));
  EXPECT_EQ(BBInside, loopAnalysis.getLoopPreheader(BBLoop2));
}

//          Main
//           +
//           |
//           v
//   +---> Node1 <--+
//   |              |
//   |              |
//   |              |
//   v              v
// Node2 <------> Node3
TEST(IRVerifierTest, LoopAnalysisTestCompleteGraph) {
  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "main",
      Function::DefinitionKind::ES5Function,
      true);
  auto BBMain = Builder.createBasicBlock(F);
  auto BBNode1 = Builder.createBasicBlock(F);
  auto BBNode2 = Builder.createBasicBlock(F);
  auto BBNode3 = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(BBMain);
  Builder.createBranchInst(BBNode1);

  Builder.setInsertionBlock(BBNode1);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBNode2, BBNode3);

  Builder.setInsertionBlock(BBNode2);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBNode1, BBNode3);

  Builder.setInsertionBlock(BBNode3);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBNode1, BBNode2);

  DominanceInfo dominanceInfo(F);
  LoopAnalysis loopAnalysis(F, dominanceInfo);

  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBMain));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBNode1));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBNode2));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBNode3));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBMain));
  EXPECT_EQ(BBNode1, loopAnalysis.getLoopHeader(BBNode1));
  EXPECT_EQ(BBNode1, loopAnalysis.getLoopHeader(BBNode2));
  EXPECT_EQ(BBNode1, loopAnalysis.getLoopHeader(BBNode3));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBMain));
  EXPECT_EQ(BBMain, loopAnalysis.getLoopPreheader(BBNode1));
  EXPECT_EQ(BBMain, loopAnalysis.getLoopPreheader(BBNode2));
  EXPECT_EQ(BBMain, loopAnalysis.getLoopPreheader(BBNode3));
}

//     Main
//      +
//      |
//      v
//     Loop1 <-+
//      +      |
//      |      |
//      v      |
//     Inner1  |
//      +      |
//      |      |
//      v      |
// +-> Loop2 +-+
// |    +
// |    |
// |    v
// |   Inner2
// |    +
// |    |
// |    v
// +-+ Exit
//      +
//      |
//      v
//     Return
TEST(IRVerifierTest, LoopAnalysisTestTwoInARow) {
  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "main",
      Function::DefinitionKind::ES5Function,
      true);
  auto BBMain = Builder.createBasicBlock(F);
  auto BBLoop1 = Builder.createBasicBlock(F);
  auto BBInner1 = Builder.createBasicBlock(F);
  auto BBLoop2 = Builder.createBasicBlock(F);
  auto BBInner2 = Builder.createBasicBlock(F);
  auto BBExit = Builder.createBasicBlock(F);
  auto BBReturn = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(BBMain);
  Builder.createBranchInst(BBLoop1);

  Builder.setInsertionBlock(BBLoop1);
  Builder.createBranchInst(BBInner1);

  Builder.setInsertionBlock(BBInner1);
  Builder.createBranchInst(BBLoop2);

  Builder.setInsertionBlock(BBLoop2);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBLoop1, BBInner2);

  Builder.setInsertionBlock(BBInner2);
  Builder.createBranchInst(BBExit);

  Builder.setInsertionBlock(BBExit);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBLoop2, BBReturn);

  Builder.setInsertionBlock(BBReturn);
  Builder.createReturnInst(M.getLiteralBool(true));

  DominanceInfo dominanceInfo(F);
  LoopAnalysis loopAnalysis(F, dominanceInfo);

  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBMain));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBLoop1));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBInner1));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBLoop2));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBInner2));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBExit));
  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBReturn));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBMain));
  EXPECT_EQ(BBLoop1, loopAnalysis.getLoopHeader(BBLoop1));
  EXPECT_EQ(BBLoop1, loopAnalysis.getLoopHeader(BBInner1));
  EXPECT_EQ(BBLoop2, loopAnalysis.getLoopHeader(BBLoop2));
  EXPECT_EQ(BBLoop2, loopAnalysis.getLoopHeader(BBInner2));
  EXPECT_EQ(BBLoop2, loopAnalysis.getLoopHeader(BBExit));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBReturn));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBMain));
  EXPECT_EQ(BBMain, loopAnalysis.getLoopPreheader(BBLoop1));
  EXPECT_EQ(BBMain, loopAnalysis.getLoopPreheader(BBInner1));
  EXPECT_EQ(BBInner1, loopAnalysis.getLoopPreheader(BBLoop2));
  EXPECT_EQ(BBInner1, loopAnalysis.getLoopPreheader(BBInner2));
  EXPECT_EQ(BBInner1, loopAnalysis.getLoopPreheader(BBExit));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBReturn));
}

//     Main
//      +
//      |
//      v
//     Node1 <-+
//      +      |
//      |      |
//      v      |
// +-> Node2   |
// |    +      |
// |    |      |
// |    v      |
// |   Node3 +-+
// |    +
// |    |
// |    v
// +-+ Node4
//      +
//      |
//      v
//     Return
TEST(IRVerifierTest, LoopAnalysisTestInterleaving) {
  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "main",
      Function::DefinitionKind::ES5Function,
      true);
  auto BBMain = Builder.createBasicBlock(F);
  auto BBNode1 = Builder.createBasicBlock(F);
  auto BBNode2 = Builder.createBasicBlock(F);
  auto BBNode3 = Builder.createBasicBlock(F);
  auto BBNode4 = Builder.createBasicBlock(F);
  auto BBReturn = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(BBMain);
  Builder.createBranchInst(BBNode1);

  Builder.setInsertionBlock(BBNode1);
  Builder.createBranchInst(BBNode2);

  Builder.setInsertionBlock(BBNode2);
  Builder.createBranchInst(BBNode3);

  Builder.setInsertionBlock(BBNode3);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBNode1, BBNode4);

  Builder.setInsertionBlock(BBNode4);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBNode2, BBReturn);

  Builder.setInsertionBlock(BBReturn);
  Builder.createReturnInst(M.getLiteralBool(true));

  DominanceInfo dominanceInfo(F);
  LoopAnalysis loopAnalysis(F, dominanceInfo);

  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBMain));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBNode1));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBNode2));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBNode3));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBNode4));
  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBReturn));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBMain));
  EXPECT_EQ(BBNode1, loopAnalysis.getLoopHeader(BBNode1));
  EXPECT_EQ(BBNode2, loopAnalysis.getLoopHeader(BBNode2));
  EXPECT_EQ(BBNode2, loopAnalysis.getLoopHeader(BBNode3));
  EXPECT_EQ(BBNode2, loopAnalysis.getLoopHeader(BBNode4));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBReturn));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBMain));
  EXPECT_EQ(BBMain, loopAnalysis.getLoopPreheader(BBNode1));
  EXPECT_EQ(BBNode1, loopAnalysis.getLoopPreheader(BBNode2));
  EXPECT_EQ(BBNode1, loopAnalysis.getLoopPreheader(BBNode3));
  EXPECT_EQ(BBNode1, loopAnalysis.getLoopPreheader(BBNode4));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBReturn));
}

//        +-+ Main +--+
//        |           |
//        |           |
//        v           v
// +-> Header +--> Return
// |      +
// |      |
// |      v
// +--+ Loop
TEST(IRVerifierTest, LoopAnalysisTestExitFromHeader) {
  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "main",
      Function::DefinitionKind::ES5Function,
      true);
  auto BBMain = Builder.createBasicBlock(F);
  auto BBHeader = Builder.createBasicBlock(F);
  auto BBLoop = Builder.createBasicBlock(F);
  auto BBReturn = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(BBMain);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBHeader, BBReturn);

  Builder.setInsertionBlock(BBHeader);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBLoop, BBReturn);

  Builder.setInsertionBlock(BBLoop);
  Builder.createBranchInst(BBHeader);

  Builder.setInsertionBlock(BBReturn);
  Builder.createReturnInst(M.getLiteralBool(true));

  DominanceInfo dominanceInfo(F);
  LoopAnalysis loopAnalysis(F, dominanceInfo);

  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBMain));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBHeader));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBLoop));
  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBReturn));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBMain));
  EXPECT_EQ(BBHeader, loopAnalysis.getLoopHeader(BBHeader));
  EXPECT_EQ(BBHeader, loopAnalysis.getLoopHeader(BBLoop));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBReturn));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBMain));
  EXPECT_EQ(BBMain, loopAnalysis.getLoopPreheader(BBHeader));
  EXPECT_EQ(BBMain, loopAnalysis.getLoopPreheader(BBLoop));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBReturn));
}

//       +--+ Main +--+
//       |            |
//       |            |
//       v            v
// +-> Node1        Node2
// |     +            +
// |     |            |
// |     v            |
// +-+ Node3 <--------+
//       +
//       |
//       v
//     Return
TEST(IRVerifierTest, LoopAnalysisTestNoUniqueHeader) {
  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "main",
      Function::DefinitionKind::ES5Function,
      true);
  auto BBMain = Builder.createBasicBlock(F);
  auto BBNode1 = Builder.createBasicBlock(F);
  auto BBNode2 = Builder.createBasicBlock(F);
  auto BBNode3 = Builder.createBasicBlock(F);
  auto BBReturn = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(BBMain);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBNode1, BBNode2);

  Builder.setInsertionBlock(BBNode1);
  Builder.createBranchInst(BBNode3);

  Builder.setInsertionBlock(BBNode3);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBReturn, BBNode1);

  Builder.setInsertionBlock(BBNode2);
  Builder.createBranchInst(BBNode3);

  Builder.setInsertionBlock(BBReturn);
  Builder.createReturnInst(M.getLiteralBool(true));

  DominanceInfo dominanceInfo(F);
  LoopAnalysis loopAnalysis(F, dominanceInfo);

  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBMain));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBNode1));
  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBNode2));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBNode3));
  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBReturn));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBMain));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBNode1));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBNode2));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBNode3));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBReturn));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBMain));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBNode1));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBNode2));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBNode3));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBReturn));
}

//  +---+ Main +---+
//  |              |
//  |              |
//  v              v
// Node1         Node2
//  +              +
//  |              |
//  |              |
//  +---> Node3 <--+
//         + ^
//         | |
//         v +
//        Node4
TEST(IRVerifierTest, LoopAnalysisTestNoUniquePreheader) {
  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "main",
      Function::DefinitionKind::ES5Function,
      true);
  auto BBMain = Builder.createBasicBlock(F);
  auto BBNode1 = Builder.createBasicBlock(F);
  auto BBNode2 = Builder.createBasicBlock(F);
  auto BBNode3 = Builder.createBasicBlock(F);
  auto BBNode4 = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(BBMain);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBNode1, BBNode2);

  Builder.setInsertionBlock(BBNode1);
  Builder.createBranchInst(BBNode3);

  Builder.setInsertionBlock(BBNode2);
  Builder.createBranchInst(BBNode3);

  Builder.setInsertionBlock(BBNode3);
  Builder.createBranchInst(BBNode4);

  Builder.setInsertionBlock(BBNode4);
  Builder.createBranchInst(BBNode3);

  DominanceInfo dominanceInfo(F);
  LoopAnalysis loopAnalysis(F, dominanceInfo);

  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBMain));
  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBNode1));
  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBNode2));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBNode3));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBNode4));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBMain));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBNode1));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBNode2));
  EXPECT_EQ(BBNode3, loopAnalysis.getLoopHeader(BBNode3));
  EXPECT_EQ(BBNode3, loopAnalysis.getLoopHeader(BBNode4));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBMain));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBNode1));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBNode2));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBNode3));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBNode4));
}

//              Main +--------+
//                +           |
//                |           |
//                v           v
// Node1 <----+ Node2 +---> Node3
// + ^                        +
// | |                        |
// | +--------+               |
// +----------> Node4 <-------+
//              ^   +
//              |   |
//              +---+
TEST(IRVerifierTest, LoopAnalysisTestFinishingTime) {
  // This was intended to be a test case demonstrating the difference between
  // selecting the "innermost" loop header by max DFS discovery time vs. min
  // finishing time. But I realized that in cases where there is a difference
  // (like this one), the headers are null anyway because they aren't unique.
  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "main",
      Function::DefinitionKind::ES5Function,
      true);
  auto BBMain = Builder.createBasicBlock(F);
  auto BBNode1 = Builder.createBasicBlock(F);
  auto BBNode2 = Builder.createBasicBlock(F);
  auto BBNode3 = Builder.createBasicBlock(F);
  auto BBNode4 = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(BBMain);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBNode2, BBNode3);

  Builder.setInsertionBlock(BBNode1);
  Builder.createBranchInst(BBNode4);

  Builder.setInsertionBlock(BBNode2);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBNode1, BBNode3);

  Builder.setInsertionBlock(BBNode3);
  Builder.createBranchInst(BBNode4);

  Builder.setInsertionBlock(BBNode4);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBNode1, BBNode4);

  DominanceInfo dominanceInfo(F);
  LoopAnalysis loopAnalysis(F, dominanceInfo);

  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBMain));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBNode1));
  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBNode2));
  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBNode3));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBNode4));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBMain));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBNode1));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBNode2));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBNode3));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBNode4));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBMain));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBNode1));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBNode2));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBNode3));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBNode4));
}

//      Main +--+
//        +     |
//        |     |
//        v     |
// +--> Node1   |
// |      +     |
// |      |     |
// |      v     |
// +--> Node2 <-+
// |      +
// |      |
// |      v
// +--+ Node3
TEST(IRVerifierTest, LoopAnalysisTestDirectlyIntoInner) {
  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      M.getInitialScope()->createInnerScope(),
      "main",
      Function::DefinitionKind::ES5Function,
      true);
  auto BBMain = Builder.createBasicBlock(F);
  auto BBNode1 = Builder.createBasicBlock(F);
  auto BBNode2 = Builder.createBasicBlock(F);
  auto BBNode3 = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(BBMain);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBNode1, BBNode2);

  Builder.setInsertionBlock(BBNode1);
  Builder.createBranchInst(BBNode2);

  Builder.setInsertionBlock(BBNode2);
  Builder.createBranchInst(BBNode3);

  Builder.setInsertionBlock(BBNode3);
  Builder.createCondBranchInst(M.getLiteralBool(true), BBNode2, BBNode1);

  DominanceInfo dominanceInfo(F);
  LoopAnalysis loopAnalysis(F, dominanceInfo);

  EXPECT_FALSE(loopAnalysis.isBlockInLoop(BBMain));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBNode1));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBNode2));
  EXPECT_TRUE(loopAnalysis.isBlockInLoop(BBNode3));

  // In theory it could identify BBNode2 as the header for BBNode2 and BBNode3,
  // but it is too conservative right now.
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBMain));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBNode1));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBNode2));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopHeader(BBNode3));

  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBMain));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBNode1));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBNode2));
  EXPECT_EQ(nullptr, loopAnalysis.getLoopPreheader(BBNode3));
}
} // end anonymous namespace
