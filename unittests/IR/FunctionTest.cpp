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

TEST(FunctionTest, AttributesTest) {
  Function::Attributes attr;
  EXPECT_EQ("", attr.getDescriptionStr());
  EXPECT_TRUE(attr.isEmpty());
  attr.allCallsitesKnown = true;
  EXPECT_EQ("[allCallsitesKnown]", attr.getDescriptionStr());
  EXPECT_FALSE(attr.isEmpty());
  attr.pure = true;
  EXPECT_EQ("[allCallsitesKnown,pure]", attr.getDescriptionStr());
  EXPECT_FALSE(attr.isEmpty());
  attr.clear();
  EXPECT_EQ("", attr.getDescriptionStr());
  EXPECT_TRUE(attr.isEmpty());
}

} // end anonymous namespace
