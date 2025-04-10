/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <hermes/hermes.h>
#include <cstdint>

#include "js_native_api.h"

using namespace facebook::hermes;
using namespace facebook::jsi;

namespace {

TEST(NodeAPITest, CreateNodeApiEnv) {
  auto rt = makeHermesRuntime();
  rt->createNodeApiEnv(8);
}

TEST(NodeAPITest, WriteAndReadInt64) {
  auto rt = makeHermesRuntime();
  napi_env env = static_cast<napi_env>(rt->createNodeApiEnv(8));

  // Write a global directly via Hermes
  rt->global().setProperty(*rt, "foo", 42);

  // Read it back via Node-API
  napi_value global;
  EXPECT_EQ(napi_get_global(env, &global), napi_ok);
  std::string foo_name = "foo";
  napi_value foo_name_value;
  EXPECT_EQ(
      napi_create_string_utf8(
          env, foo_name.c_str(), foo_name.size(), &foo_name_value),
      napi_ok);
  napi_value foo;
  {
    EXPECT_EQ(napi_get_property(env, global, foo_name_value, &foo), napi_ok);
    int64_t result;
    EXPECT_EQ(napi_get_value_int64(env, foo, &result), napi_ok);
    EXPECT_EQ(result, 42);
  }

  // Write it via Node-API
  {
    EXPECT_EQ(napi_create_int64(env, 1337, &foo), napi_ok);
    EXPECT_EQ(napi_set_property(env, global, foo_name_value, foo), napi_ok);
  }

  // Read it back via Hermes
  EXPECT_EQ(rt->global().getProperty(*rt, "foo").asNumber(), 1337);
}

} // namespace
