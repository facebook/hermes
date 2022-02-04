/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "llvh/Support/Casting.h"
#include "llvh/Support/SourceMgr.h"
#include "llvh/Support/YAMLParser.h"

#include "hermes/AST/Context.h"
#include "hermes/BCGen/RegAlloc.h"

#include "gtest/gtest.h"

using namespace hermes;

namespace {

TEST(RegisterAllocatorTest, RegisterFileTest) {
  RegisterFile File;

  // We are starting with an empty register file.
  EXPECT_EQ(File.getNumLiveRegisters(), 0u);
  EXPECT_EQ(File.getMaxRegisterUsage(), 0u);

  // Allocate a few registers.
  Register R1 = File.allocateRegister();
  Register R2 = File.allocateRegister();
  Register R3 = File.allocateRegister();
  // Make sure we know which registers are alive.
  EXPECT_EQ(File.getMaxRegisterUsage(), 3u);
  EXPECT_TRUE(File.isUsed(R1));
  EXPECT_TRUE(File.isUsed(R2));
  EXPECT_TRUE(File.isUsed(R3));

  EXPECT_FALSE(File.isFree(R1));
  EXPECT_FALSE(File.isFree(R2));
  EXPECT_FALSE(File.isFree(R3));

  EXPECT_EQ(File.getMaxRegisterUsage(), 3u);

  // Make sure we can kill registers and things keep working.
  File.killRegister(R2);
  EXPECT_TRUE(File.isUsed(R1));
  EXPECT_FALSE(File.isUsed(R2));
  EXPECT_TRUE(File.isUsed(R3));

  EXPECT_TRUE(File.isFree(R2));

  // Make sure we can reuse the freed register.
  Register R4 = File.allocateRegister();
  EXPECT_EQ(File.getMaxRegisterUsage(), 3u);

  File.killRegister(R1);
  File.killRegister(R3);
  File.killRegister(R4);

  // Make sure that all registers have been freed.
  EXPECT_EQ(File.getNumLiveRegisters(), 0u);

  // Make sure we can allocate lots of registers and free them in some order.
  std::vector<Register> regs;
  for (int i = 0; i < 1000; i++) {
    regs.push_back(File.allocateRegister());
  }
  for (auto &R : regs) {
    File.killRegister(R);
  }
  // Make sure that all registers have been freed again.
  EXPECT_EQ(File.getNumLiveRegisters(), 0u);
}

} // end anonymous namespace
