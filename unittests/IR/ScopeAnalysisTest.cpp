/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/Context.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"

#include "gtest/gtest.h"

using namespace hermes;

namespace {

TEST(IRVerifierTest, ScopeAnalysisTest) {
  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder Builder(&M);

  auto G = Builder.createTopLevelFunction(true);
  auto F1 =
      Builder.createFunction("f1", Function::DefinitionKind::ES5Function, true);
  auto F2 =
      Builder.createFunction("f2", Function::DefinitionKind::ES5Function, true);
  auto orphan = Builder.createFunction(
      "orphan", Function::DefinitionKind::ES5Function, true);
  auto F11 = Builder.createFunction(
      "f11", Function::DefinitionKind::ES5Function, true);
  auto ES = Builder.createExternalScope(G, -5);

  auto Gbb = Builder.createBasicBlock(G);
  Builder.setInsertionBlock(Gbb);
  Builder.createCreateFunctionInst(F1);
  Builder.createCreateFunctionInst(F2);

  auto F1bb = Builder.createBasicBlock(F1);
  Builder.setInsertionBlock(F1bb);
  Builder.createCreateFunctionInst(F11);

  FunctionScopeAnalysis fsa{M.getTopLevelFunction()};
  EXPECT_EQ(0, fsa.getScopeDepth(G->getFunctionScope()));
  EXPECT_EQ(1, fsa.getScopeDepth(F1->getFunctionScope()));
  EXPECT_EQ(1, fsa.getScopeDepth(F2->getFunctionScope()));
  EXPECT_EQ(2, fsa.getScopeDepth(F11->getFunctionScope()));
  EXPECT_EQ(-5, fsa.getScopeDepth(ES));
  EXPECT_EQ(llvh::None, fsa.getScopeDepth(orphan->getFunctionScope()));

  EXPECT_EQ(nullptr, fsa.getLexicalParent(G));
  EXPECT_EQ(G, fsa.getLexicalParent(F1));
  EXPECT_EQ(G, fsa.getLexicalParent(F2));
  EXPECT_EQ(F1, fsa.getLexicalParent(F11));
  EXPECT_EQ(llvh::None, fsa.getScopeDepth(orphan->getFunctionScope()));
}

} // end anonymous namespace
