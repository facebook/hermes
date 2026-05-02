/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "NapiTestFixture.h"

namespace {

using hermes::napi::NapiTestFixture;

//===========================================================================
// napi_get_instance_data
//===========================================================================

TEST_F(NapiTestFixture, GetInstanceDataDefaultNull) {
  void *data = reinterpret_cast<void *>(0xDEAD);
  ASSERT_EQ(napi_ok, napi_get_instance_data(env_, &data));
  EXPECT_EQ(nullptr, data);
}

TEST_F(NapiTestFixture, GetInstanceDataNullEnv) {
  void *data = nullptr;
  EXPECT_EQ(napi_invalid_arg, napi_get_instance_data(nullptr, &data));
}

TEST_F(NapiTestFixture, GetInstanceDataNullResult) {
  EXPECT_EQ(napi_invalid_arg, napi_get_instance_data(env_, nullptr));
}

//===========================================================================
// napi_set_instance_data
//===========================================================================

TEST_F(NapiTestFixture, SetInstanceDataNullEnv) {
  int value = 42;
  EXPECT_EQ(
      napi_invalid_arg,
      napi_set_instance_data(nullptr, &value, nullptr, nullptr));
}

TEST_F(NapiTestFixture, SetAndGetInstanceData) {
  int value = 42;
  ASSERT_EQ(napi_ok, napi_set_instance_data(env_, &value, nullptr, nullptr));

  void *data = nullptr;
  ASSERT_EQ(napi_ok, napi_get_instance_data(env_, &data));
  EXPECT_EQ(&value, data);
  EXPECT_EQ(42, *static_cast<int *>(data));
}

TEST_F(NapiTestFixture, SetInstanceDataNoFinalizer) {
  // Setting instance data with null finalizer should work fine.
  int value = 99;
  ASSERT_EQ(napi_ok, napi_set_instance_data(env_, &value, nullptr, nullptr));

  void *data = nullptr;
  ASSERT_EQ(napi_ok, napi_get_instance_data(env_, &data));
  EXPECT_EQ(&value, data);
}

TEST_F(NapiTestFixture, SetInstanceDataFinalizerCalledOnEnvDestroy) {
  // Create a dynamically allocated value so the finalizer can free it.
  bool finalizerCalled = false;
  auto *flag = &finalizerCalled;

  // The data pointer itself doesn't matter; we track via the hint.
  ASSERT_EQ(
      napi_ok,
      napi_set_instance_data(
          env_,
          nullptr,
          [](napi_env, void *, void *hint) {
            *static_cast<bool *>(hint) = true;
          },
          flag));

  EXPECT_FALSE(finalizerCalled);

  // Destroy the env — should call the finalizer.
  hermes_napi_destroy_env(env_);
  env_ = nullptr; // Prevent TearDown from double-destroying.

  EXPECT_TRUE(finalizerCalled);
}

TEST_F(NapiTestFixture, SetInstanceDataFinalizerCalledOnReplace) {
  bool firstFinalizerCalled = false;
  auto *firstFlag = &firstFinalizerCalled;

  ASSERT_EQ(
      napi_ok,
      napi_set_instance_data(
          env_,
          nullptr,
          [](napi_env, void *, void *hint) {
            *static_cast<bool *>(hint) = true;
          },
          firstFlag));

  EXPECT_FALSE(firstFinalizerCalled);

  // Replace with new instance data — old finalizer should be called.
  bool secondFinalizerCalled = false;
  auto *secondFlag = &secondFinalizerCalled;

  ASSERT_EQ(
      napi_ok,
      napi_set_instance_data(
          env_,
          nullptr,
          [](napi_env, void *, void *hint) {
            *static_cast<bool *>(hint) = true;
          },
          secondFlag));

  EXPECT_TRUE(firstFinalizerCalled);
  EXPECT_FALSE(secondFinalizerCalled);

  // Destroy the env — should call the second finalizer.
  hermes_napi_destroy_env(env_);
  env_ = nullptr;

  EXPECT_TRUE(secondFinalizerCalled);
}

TEST_F(NapiTestFixture, SetInstanceDataFinalizerReceivesCorrectArgs) {
  struct FinalizeInfo {
    napi_env receivedEnv = nullptr;
    void *receivedData = nullptr;
    void *receivedHint = nullptr;
  };
  FinalizeInfo info;

  int myData = 42;

  ASSERT_EQ(
      napi_ok,
      napi_set_instance_data(
          env_,
          &myData,
          [](napi_env env, void *data, void *hint) {
            auto *info = static_cast<FinalizeInfo *>(hint);
            info->receivedEnv = env;
            info->receivedData = data;
            // The hint we set points to FinalizeInfo, not the original hint.
            // So we stash the env, data here.
          },
          &info));

  // Destroy env to trigger finalizer.
  hermes_napi_destroy_env(env_);
  env_ = nullptr;

  EXPECT_EQ(&myData, info.receivedData);
  // The env pointer should match what was passed.
  EXPECT_NE(nullptr, info.receivedEnv);
}

TEST_F(NapiTestFixture, ReplaceInstanceDataWithNull) {
  int value = 42;
  ASSERT_EQ(napi_ok, napi_set_instance_data(env_, &value, nullptr, nullptr));

  void *data = nullptr;
  ASSERT_EQ(napi_ok, napi_get_instance_data(env_, &data));
  EXPECT_EQ(&value, data);

  // Replace with null data.
  ASSERT_EQ(napi_ok, napi_set_instance_data(env_, nullptr, nullptr, nullptr));

  ASSERT_EQ(napi_ok, napi_get_instance_data(env_, &data));
  EXPECT_EQ(nullptr, data);
}

} // namespace
