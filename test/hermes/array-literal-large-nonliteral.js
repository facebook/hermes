/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */


// RUN: %hermes %s | %hermes - -O | %FileCheck %s --match-full-lines
// RUN: %hermes %s | %hermes - | %FileCheck %s --match-full-lines
// RUN: %hermes %s | %shermes - -exec | %FileCheck %s --match-full-lines
// REQUIRES: !slow_debug

// Array literal allocation can only handle 16-bits worth of size.
// We need to make sure that nonliteral population works after that.
print("globalThis.a = [")
for (var i = 0; i < 70_000; ++i) {
  print(i + ", ");
}
print("Math];");
print("print(globalThis.a[70_000]);");
// CHECK: [object Math]
