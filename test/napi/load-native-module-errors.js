/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// REQUIRES: napi
// UNSUPPORTED: windows

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// Test error handling: non-string argument.
try {
  loadNativeModule(42);
} catch (e) {
  print(e.constructor.name);
}
// CHECK: TypeError

// Test error handling: non-existent file.
try {
  loadNativeModule("/nonexistent/path/to/module.node");
} catch (e) {
  print(e.constructor.name);
}
// CHECK: TypeError

// Test error handling: no arguments.
try {
  loadNativeModule();
} catch (e) {
  print(e.constructor.name);
}
// CHECK: TypeError
