/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xhermes-internal-test-methods=true %s | %FileCheck --match-full-lines %s
"use strict";

print("HermesInternal reset timezone cache");
// CHECK-LABEL: HermesInternal reset timezone cache

if (
  typeof HermesInternal !== "undefined" &&
  typeof HermesInternal.resetTimezoneCache === "function"
) {
  try {
    // Test: resetTimezoneCache can be called without throwing
    HermesInternal.resetTimezoneCache();
    print("resetTimezoneCache called successfully");
    // CHECK: resetTimezoneCache called successfully

    // Test: resetTimezoneCache returns undefined (expected for void functions)
    var result = HermesInternal.resetTimezoneCache();
    print("resetTimezoneCache return value: " + String(result));
    // CHECK: resetTimezoneCache return value: undefined
  } catch (e) {
    print("resetTimezoneCache threw: " + e);
    // CHECK-NOT: resetTimezoneCache threw:
  }
} else {
  print("HermesInternal.resetTimezoneCache is not available");
  // CHECK-NOT: resetTimezoneCache called successfully
}
