/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

// Check Regex doesn't crash due to recursing too many times in native code.

try {
  var res = new RegExp("(?=".repeat(100000) + "A" + ")".repeat(100000)).exec('b');
} catch (e) {
  print(e);
  // CHECK: RangeError: Maximum regex stack depth reached
}

try {
  // This regex should execute fine.
  new RegExp("(?=A)").exec('b'.repeat(10000));
  print("success");
  // CHECK-NEXT: success
} catch (e) {}
