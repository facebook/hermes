/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <hermes/cdp/DomainState.h>
#include <hermes/hermes.h>

using namespace facebook::hermes::cdp;
using namespace facebook::hermes;
using namespace facebook;

struct TestStateValue : public StateValue {
  ~TestStateValue() override = default;
  std::unique_ptr<StateValue> copy() const override;
  bool boolValue;
  std::shared_ptr<int> sharedInt;
};

std::unique_ptr<StateValue> TestStateValue::copy() const {
  auto copy = std::make_unique<TestStateValue>();
  copy->boolValue = boolValue;
  if (sharedInt) {
    copy->sharedInt = std::make_shared<int>(*sharedInt);
  }
  return copy;
}

class DomainStateTest : public ::testing::Test {};

TEST_F(DomainStateTest, TestCreation) {
  DomainState state;

  // Default construction makes an empty dictionary
  auto value = state.getCopy({"key"});
  EXPECT_TRUE(value == nullptr);
}

TEST_F(DomainStateTest, TestModifications) {
  DomainState state;

  auto sharedInt = std::make_shared<int>(123);
  {
    auto stateValue1 = std::make_unique<TestStateValue>();
    stateValue1->boolValue = true;
    stateValue1->sharedInt = sharedInt;
    DomainState::Transaction transaction = state.transaction();
    transaction.add({"level1", "level2", "key"}, *stateValue1);

    auto stateValue2 = std::make_unique<TestStateValue>();
    transaction.add({"level1", "level2Key"}, *stateValue2);
  }

  // Change the value pointed to by shared_ptr so we can validate that it
  // doesn't affect what's stored
  *sharedInt = 321;

  // Verify level1 dictionary got created
  auto lvl1Dict = state.getCopy({"level1"});
  EXPECT_TRUE(dynamic_cast<DictionaryStateValue *>(lvl1Dict.get()) != nullptr);

  // Verify level2 dictionary got created
  auto lvl2Dict = state.getCopy({"level1", "level2"});
  EXPECT_TRUE(dynamic_cast<DictionaryStateValue *>(lvl2Dict.get()) != nullptr);

  // Verify stored value
  auto leafValue = state.getCopy({"level1", "level2", "key"});
  TestStateValue *testValue = dynamic_cast<TestStateValue *>(leafValue.get());
  EXPECT_TRUE(testValue != nullptr);
  EXPECT_TRUE(testValue->boolValue);
  EXPECT_EQ(*(testValue->sharedInt), 123);

  // Verify removal works
  auto lvl2Value = state.getCopy({"level1", "level2Key"});
  EXPECT_TRUE(dynamic_cast<TestStateValue *>(lvl2Value.get()) != nullptr);
  { state.transaction().remove({"level1", "level2Key"}); }
  lvl2Value = state.getCopy({"level1", "level2Key"});
  EXPECT_FALSE(dynamic_cast<TestStateValue *>(lvl2Value.get()) != nullptr);

  // Verify dictionary works too
  { state.transaction().remove({"level1", "level2"}); }
  leafValue = state.getCopy({"level1", "level2", "key"});
  EXPECT_FALSE(dynamic_cast<TestStateValue *>(leafValue.get()) != nullptr);
}

TEST_F(DomainStateTest, TestCopy) {
  DomainState state;

  auto sharedInt = std::make_shared<int>(123);
  {
    auto stateValue1 = std::make_unique<TestStateValue>();
    stateValue1->boolValue = true;
    stateValue1->sharedInt = sharedInt;
    DomainState::Transaction transaction = state.transaction();
    transaction.add({"level1", "level2", "key"}, *stateValue1);

    auto stateValue2 = std::make_unique<TestStateValue>();
    stateValue2->sharedInt = std::make_shared<int>(333);
    transaction.add({"level1", "level2Key"}, *stateValue2);
  }

  auto copy = state.copy();

  // Change the value pointed to by shared_ptr so we can validate that it
  // doesn't affect what's copied
  *sharedInt = 321;
  // Delete something to validate it doesn't affect what's copied
  { state.transaction().remove({"level1", "level2"}); }

  auto leafValue = copy->getCopy({"level1", "level2", "key"});
  TestStateValue *testValue = dynamic_cast<TestStateValue *>(leafValue.get());
  EXPECT_TRUE(testValue != nullptr);
  EXPECT_TRUE(testValue->boolValue);
  EXPECT_EQ(*(testValue->sharedInt), 123);

  auto lvl2Value = copy->getCopy({"level1", "level2Key"});
  testValue = dynamic_cast<TestStateValue *>(lvl2Value.get());
  EXPECT_TRUE(testValue != nullptr);
  EXPECT_EQ(*(testValue->sharedInt), 333);
}
