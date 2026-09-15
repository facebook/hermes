/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes_napi.h"

#include "gtest/gtest.h"

namespace hermes {
namespace napi {

/// Test that napi_module_register stores the module and
/// hermes_napi_get_last_registered_module retrieves it.
TEST(NapiModuleTest, ModuleRegister) {
  napi_module mod = {};
  mod.nm_version = NAPI_MODULE_VERSION;
  mod.nm_modname = "test_module";
  mod.nm_register_func = nullptr;

  napi_module_register(&mod);

  const napi_module *retrieved = hermes_napi_get_last_registered_module();
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved, &mod);
  EXPECT_STREQ(retrieved->nm_modname, "test_module");
}

/// Test that a second call overwrites the first.
TEST(NapiModuleTest, ModuleRegisterOverwrites) {
  napi_module mod1 = {};
  mod1.nm_modname = "first";
  napi_module mod2 = {};
  mod2.nm_modname = "second";

  napi_module_register(&mod1);
  napi_module_register(&mod2);

  const napi_module *retrieved = hermes_napi_get_last_registered_module();
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved, &mod2);
  EXPECT_STREQ(retrieved->nm_modname, "second");
}

/// Test that NAPI_MODULE_INIT() compiles and produces a callable function.
/// We define a test module init function using the macro, then verify we
/// can look it up by name and call it.
///
/// Note: NAPI_MODULE_INIT() expands to both
///   node_api_module_get_api_version_v1() and
///   napi_register_module_v1(env, exports)
/// Because the macro both declares and begins the function body, we
/// define the function here and call it from the test below.

// This must be at file scope because the macro generates extern "C"
// function declarations.
NAPI_MODULE_INIT() {
  // A trivial init function that just returns exports unchanged.
  return exports;
}

TEST(NapiModuleTest, ModuleInitMacroVersion) {
  // Verify the version query function returns NAPI_VERSION.
  int32_t version = node_api_module_get_api_version_v1();
  EXPECT_EQ(version, NAPI_VERSION);
}

TEST(NapiModuleTest, ModuleInitMacroFunction) {
  // Verify the init function is callable. We pass nullptr for both
  // env and exports since our trivial init just returns exports.
  napi_value result = napi_register_module_v1(nullptr, nullptr);
  EXPECT_EQ(result, nullptr);
}

/// Test the NAPI_MODULE() shorthand macro. We can't use it in the same
/// translation unit as NAPI_MODULE_INIT() because both define
/// napi_register_module_v1. Instead, we verify it's a valid macro by
/// checking that the NAPI_MODULE_INIT-generated function works (which
/// NAPI_MODULE delegates to internally).

} // namespace napi
} // namespace hermes
