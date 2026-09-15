/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// REQUIRES: core_extensions
// RUN: %hermes %s | %FileCheck --match-full-lines %s

// Test that Function.prototype.toString() behavior for JS-implemented builtins
// with the microtask queue enabled. This is intentional: some React Native
// versions check Promise.toString().includes("[native code]") to detect whether
// the engine's microtask queue is in use.

print("Function toString microtask test");
// CHECK-LABEL: Function toString microtask test

// Promise is implemented in InternalJavaScript.
// With microtask queue enabled, it should show [native code].
print(Promise.toString().includes("[native code]"));
// CHECK-NEXT: true

print(Promise.prototype.then.toString().includes("[native code]"));
// CHECK-NEXT: true

// TextEncoder is implemented as an extension.
// Extensions always show [native code] regardless of microtask queue setting.
print(TextEncoder.toString().includes("[native code]"));
// CHECK-NEXT: true

print(TextEncoder.prototype.encode.toString().includes("[native code]"));
// CHECK-NEXT: true

// Native builtins always show [native code] regardless of microtask queue.
print(Array.isArray.toString().includes("[native code]"));
// CHECK-NEXT: true

print("Done");
// CHECK-NEXT: Done
