/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers.h"

#include "gtest/gtest.h"

using namespace hermes::vm;
using namespace hermes::hbc;

namespace {

TEST(RuntimeTest, configRegisterCountOverflow) {
  EXPECT_DEATH_IF_SUPPORTED(
      {
        Runtime::create(
            RuntimeConfig::Builder().withMaxNumRegisters(UINT32_MAX).build());
      },
      "RuntimeConfig maxNumRegisters too big");
  EXPECT_DEATH_IF_SUPPORTED(
      {
        Runtime::create(RuntimeConfig::Builder()
                            .withMaxNumRegisters(UINT32_MAX - 1)
                            .build());
      },
      "RuntimeConfig maxNumRegisters too big");
}

} // anonymous namespace
