/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Public/CtorConfig.h"

#include "gtest/gtest.h"

namespace {

#define TEST_FIELDS(F)           \
  F(constexpr, int, AAA, 0)      \
                                 \
  F(constexpr, bool, BBB, false) \
                                 \
  F(, const char *, CCC, "abc")  \
  /* TEST_FIELDS END */

_HERMES_CTORCONFIG_STRUCT(TestConfig, TEST_FIELDS, {})

TEST(CtorConfigTest, testDefaults) {
  static_assert(TestConfig::getDefaultAAA() == 0, "unexpected default");
  static_assert(TestConfig::getDefaultBBB() == false, "unexpected default");
  // Not constexpr.
  EXPECT_STREQ("abc", TestConfig::getDefaultCCC());
}

TEST(CtorConfigTest, testBasic) {
  TestConfig testConfig =
      TestConfig::Builder().withAAA(1).withBBB(true).build();
  EXPECT_EQ(1, testConfig.getAAA());
  EXPECT_TRUE(testConfig.getBBB());
  EXPECT_STREQ("abc", testConfig.getCCC());
}

TEST(CtorConfigTest, testOverride) {
  TestConfig testConfig =
      TestConfig::Builder().withAAA(1).withBBB(true).build();
  testConfig = testConfig.rebuild()
                   .update(TestConfig::Builder().withAAA(2).withCCC("xyz"))
                   .build();
  EXPECT_EQ(2, testConfig.getAAA());
  EXPECT_TRUE(testConfig.getBBB());
  EXPECT_STREQ("xyz", testConfig.getCCC());
}

#define TEST_FIELDS2(F)         \
  F(constexpr, int, MMM, 100)   \
                                \
  F(, TestConfig, NestedConfig) \
  /* TEST_FIELDS2 END */

_HERMES_CTORCONFIG_STRUCT(TestConfigWithNesting, TEST_FIELDS2, {})

TEST(CtorConfigTest, testBasicNested) {
  TestConfigWithNesting testConfig =
      TestConfigWithNesting::Builder()
          .withMMM(200)
          .withNestedConfig(TestConfig::Builder().withCCC("pqr").build())
          .build();
  EXPECT_EQ(200, testConfig.getMMM());
  EXPECT_FALSE(testConfig.getNestedConfig().getBBB());
  EXPECT_STREQ("pqr", testConfig.getNestedConfig().getCCC());
}

TEST(CtorConfigTest, testOverrideNested) {
  TestConfigWithNesting testConfig =
      TestConfigWithNesting::Builder()
          .withMMM(200)
          .withNestedConfig(TestConfig::Builder().withCCC("pqr").build())
          .build();
  testConfig =
      testConfig.rebuild()
          .update(
              TestConfigWithNesting::Builder().withMMM(300).withNestedConfig(
                  testConfig.getNestedConfig()
                      .rebuild()
                      .update(TestConfig::Builder().withCCC("def"))
                      .build()))
          .build();
  EXPECT_EQ(300, testConfig.getMMM());
  EXPECT_FALSE(testConfig.getNestedConfig().getBBB());
  EXPECT_STREQ("def", testConfig.getNestedConfig().getCCC());
}

} // end anonymous namespace
