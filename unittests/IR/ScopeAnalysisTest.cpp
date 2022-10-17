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

  auto createFunction = [&](const char *name) {
    return Builder.createFunction(
        name, Function::DefinitionKind::ES5Function, true);
  };

  auto addEdges = [&](Function *parent, auto... children) {
    Builder.setInsertionBlock(Builder.createBasicBlock(parent));
    (Builder.createCreateFunctionInst(children), ...);
  };

  auto ES5 = createFunction("es5");
  auto ES4 = createFunction("es4");
  auto ES3 = createFunction("es3");
  auto ES2 = createFunction("es2");
  auto ES1 = createFunction("es1");
  auto G = Builder.createTopLevelFunction(true);
  auto F1 = createFunction("f1");
  auto F2 = createFunction("f2");
  auto F11 = createFunction("f11");
  auto orphan = createFunction("orphan");
  auto orphan1 = createFunction("orphan1");
  auto orphan2 = createFunction("orphan2");
  auto orphan21 = createFunction("orphan21");
  auto ES = Builder.createExternalScope(G, -5);

  addEdges(ES5, ES4);
  addEdges(ES4, ES3);
  addEdges(ES3, ES2);
  addEdges(ES2, ES1);
  addEdges(ES1, G);
  addEdges(G, F1, F2);
  addEdges(F1, F11);
  addEdges(orphan, orphan1);
  addEdges(orphan, orphan2);
  addEdges(orphan2, orphan21);

  FunctionScopeAnalysis fsa{G};
  EXPECT_EQ(-5, fsa.getScopeDepth(ES5->getFunctionScope()));
  EXPECT_EQ(-4, fsa.getScopeDepth(ES4->getFunctionScope()));
  EXPECT_EQ(-3, fsa.getScopeDepth(ES3->getFunctionScope()));
  EXPECT_EQ(-2, fsa.getScopeDepth(ES2->getFunctionScope()));
  EXPECT_EQ(-1, fsa.getScopeDepth(ES1->getFunctionScope()));
  EXPECT_EQ(0, fsa.getScopeDepth(G->getFunctionScope()));
  EXPECT_EQ(1, fsa.getScopeDepth(F1->getFunctionScope()));
  EXPECT_EQ(1, fsa.getScopeDepth(F2->getFunctionScope()));
  EXPECT_EQ(2, fsa.getScopeDepth(F11->getFunctionScope()));
  EXPECT_EQ(-5, fsa.getScopeDepth(ES));
  EXPECT_EQ(llvh::None, fsa.getScopeDepth(orphan->getFunctionScope()));
  EXPECT_EQ(llvh::None, fsa.getScopeDepth(orphan1->getFunctionScope()));
  EXPECT_EQ(llvh::None, fsa.getScopeDepth(orphan2->getFunctionScope()));
  EXPECT_EQ(llvh::None, fsa.getScopeDepth(orphan21->getFunctionScope()));

  EXPECT_EQ(nullptr, fsa.getLexicalParent(ES5));
  EXPECT_EQ(ES5, fsa.getLexicalParent(ES4));
  EXPECT_EQ(ES4, fsa.getLexicalParent(ES3));
  EXPECT_EQ(ES3, fsa.getLexicalParent(ES2));
  EXPECT_EQ(ES2, fsa.getLexicalParent(ES1));
  EXPECT_EQ(ES1, fsa.getLexicalParent(G));
  EXPECT_EQ(G, fsa.getLexicalParent(F1));
  EXPECT_EQ(G, fsa.getLexicalParent(F2));
  EXPECT_EQ(F1, fsa.getLexicalParent(F11));
  EXPECT_EQ(llvh::None, fsa.getScopeDepth(orphan->getFunctionScope()));
  EXPECT_EQ(llvh::None, fsa.getScopeDepth(orphan1->getFunctionScope()));
  EXPECT_EQ(llvh::None, fsa.getScopeDepth(orphan2->getFunctionScope()));
  EXPECT_EQ(llvh::None, fsa.getScopeDepth(orphan21->getFunctionScope()));
}

} // end anonymous namespace
