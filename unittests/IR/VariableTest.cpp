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

/// \return true if the variables in the scope are correctly sorted so that the
/// hidden vars are at the end.
static bool isSorted(VariableScope *vs) {
  size_t i = 0;
  size_t e = vs->getVariables().size();
  for (; i < e; ++i) {
    Variable *v = vs->getVariables()[i];
    if (v->getHidden())
      break;
  }
  for (; i < e; ++i) {
    Variable *v = vs->getVariables()[i];
    if (!v->getHidden())
      return false;
  }
  return true;
}

TEST(VariableTest, ReorderHidden) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);

  {
    auto *vs = Builder.createVariableScope(nullptr);
    auto *v1 = Builder.createVariable(vs, "v1", Type::createAnyType(), true);
    Builder.createVariable(vs, "v2", Type::createAnyType(), false);
    Builder.createVariable(vs, "v3", Type::createAnyType(), false);
    Builder.createVariable(vs, "v4", Type::createAnyType(), false);
    vs->assignIndexToVariables();

    ASSERT_EQ(4, vs->getVariables().size());
    EXPECT_TRUE(isSorted(vs));
    // Make sure the hidden variable gets moved to the end.
    EXPECT_EQ(v1, vs->getVariables()[3]);
  }

  {
    auto *vs = Builder.createVariableScope(nullptr);
    Builder.createVariable(vs, "v2", Type::createAnyType(), false);
    Builder.createVariable(vs, "v3", Type::createAnyType(), false);
    Builder.createVariable(vs, "v4", Type::createAnyType(), false);
    auto *v1 = Builder.createVariable(vs, "v1", Type::createAnyType(), true);
    vs->assignIndexToVariables();

    ASSERT_EQ(4, vs->getVariables().size());
    EXPECT_TRUE(isSorted(vs));
    // Make sure the hidden variable stays at the end.
    EXPECT_EQ(vs->getVariables()[3], v1);
  }

  {
    auto *vs = Builder.createVariableScope(nullptr);
    auto *v1 = Builder.createVariable(vs, "v1", Type::createAnyType(), true);
    // Make sure this doesn't crash or assert.
    vs->assignIndexToVariables();

    ASSERT_EQ(1, vs->getVariables().size());
    // Make sure the hidden variable stays at the end.
    EXPECT_EQ(vs->getVariables()[0], v1);
  }

  {
    auto *vs = Builder.createVariableScope(nullptr);
    // Make sure this doesn't crash or assert.
    vs->assignIndexToVariables();

    ASSERT_TRUE(vs->getVariables().empty());
  }
}

} // end anonymous namespace
