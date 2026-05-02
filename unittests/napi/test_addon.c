/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
 * A minimal test NAPI addon built as a shared library for testing
 * hermes_napi_load_module. It exports a single property:
 *   exports.hello = "world"
 */

#include "hermes/napi/js_native_api.h"
#include "hermes/napi/node_api.h"

NAPI_MODULE_INIT() {
  napi_value world;
  napi_create_string_utf8(env, "world", 5, &world);
  napi_set_named_property(env, exports, "hello", world);
  return exports;
}
