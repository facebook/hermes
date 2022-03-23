/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/SemValidate.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Parser/JSParser.h"

#include "gtest/gtest.h"

using namespace hermes;

namespace {

/// Verify that scope chains are respected in IRGen.
TEST(IRScopeChainTest, BasicScopeChainTest) {
  SourceErrorManager sm;
  CodeGenerationSettings codeGenOpts;
  codeGenOpts.unlimitedRegisters = false;
  auto context = std::make_shared<Context>(sm, codeGenOpts);

  // This simulates local eval. We have an incoming nested scope with a variable
  // "alpha", inside a scope containing variables "beta" and "delta". "gamma" is
  // not part of our scope chain and should be global.
  ScopeChain scopeChain{};
  scopeChain.functions.emplace_back();
  scopeChain.functions.back().variables = {"alpha"};
  scopeChain.functions.emplace_back();
  scopeChain.functions.back().variables = {"beta", "delta"};

  hermes::parser::JSParser jsParser(
      *context, "print(alpha, beta, gamma, delta);");
  auto parsed = jsParser.parse();
  ASSERT_TRUE(parsed);
  sem::SemContext semCtx{};
  auto validated = validateAST(*context, semCtx, *parsed);
  ASSERT_TRUE(validated);

  auto *ast = parsed.getValue();
  Module M(context);

  DeclarationFileListTy declFileList;
  hermes::generateIRFromESTree(ast, &M, declFileList, scopeChain);

  // Count how many frame loads there are.
  int loadsFromGlobals = 0;
  int loadsFromNonglobals = 0;
  for (const auto &F : M) {
    for (const auto &BB : F.getBasicBlockList()) {
      for (const auto &Ins : BB) {
        if (llvh::isa<LoadFrameInst>(&Ins)) {
          loadsFromNonglobals++;
        } else if (const auto *LP = llvh::dyn_cast<LoadPropertyInst>(&Ins)) {
          if (llvh::isa<GlobalObject>(LP->getObject()))
            loadsFromGlobals++;
        }
      }
    }
  }
  EXPECT_EQ(2u, M.size()); // one global function and one inner function.
  EXPECT_EQ(2, loadsFromGlobals); // "gamma", plus print function.
  EXPECT_EQ(3, loadsFromNonglobals); // "alpha", "beta", "delta"
}

} // end anonymous namespace
