/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
 * A reference test NAPI addon that exercises the core NAPI pipeline:
 * module registration, function creation, argument extraction, value
 * creation, and property definition.
 *
 * Exports:
 *   add(a, b) -- returns a + b (as doubles)
 *   hello()   -- returns the string "world"
 */

#include "hermes/napi/js_native_api.h"
#include "hermes/napi/node_api.h"

/// add(a, b): Extract two numbers, return their sum.
static napi_value add(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2];
  napi_get_cb_info(env, info, &argc, argv, NULL, NULL);

  double a = 0;
  double b = 0;
  napi_get_value_double(env, argv[0], &a);
  napi_get_value_double(env, argv[1], &b);

  napi_value result;
  napi_create_double(env, a + b, &result);
  return result;
}

/// hello(): Return the string "world".
static napi_value hello(napi_env env, napi_callback_info info) {
  (void)info;
  napi_value result;
  napi_create_string_utf8(env, "world", 5, &result);
  return result;
}

NAPI_MODULE_INIT() {
  napi_value add_fn;
  napi_create_function(env, "add", NAPI_AUTO_LENGTH, add, NULL, &add_fn);
  napi_set_named_property(env, exports, "add", add_fn);

  napi_value hello_fn;
  napi_create_function(env, "hello", NAPI_AUTO_LENGTH, hello, NULL, &hello_fn);
  napi_set_named_property(env, exports, "hello", hello_fn);

  return exports;
}
