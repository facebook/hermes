/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <emscripten/emscripten.h>

/// Emscripten defines getentropy as a weak symbol that aborts in the
/// implementation. Override it with a call to an imported function.
extern "C" int getentropy_impl(void *buffer, size_t length)
    __attribute__((import_module("hermes_import"), import_name("getentropy")));
extern "C" int getentropy(void *buffer, size_t length) {
  return getentropy_impl(buffer, length);
}

extern "C" bool test_timeout_impl() __attribute__((
    import_module("hermes_import"),
    import_name("test_timeout")));
extern "C" bool test_wasm_host_timeout() {
  return test_timeout_impl();
}

extern "C" bool test_and_clear_timeout_impl() __attribute__((
    import_module("hermes_import"),
    import_name("test_and_clear_timeout")));
extern "C" bool test_and_clear_wasm_host_timeout() {
  return test_and_clear_timeout_impl();
}
