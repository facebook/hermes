/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xmicrotask-queue=0 %s | %FileCheck --match-full-lines --check-prefix=DISABLED %s
// RUN: %hermes -Xmicrotask-queue=1 %s | %FileCheck --match-full-lines --check-prefix=ENABLED %s

// Test that Function.prototype.toString() behavior for JS-implemented builtins
// depends on the microtask queue setting. This is intentional: some React Native
// versions check Promise.toString().includes("[native code]") to detect whether
// the engine's microtask queue is in use.

print("Function toString microtask test");
// DISABLED-LABEL: Function toString microtask test
// ENABLED-LABEL: Function toString microtask test

// Promise is implemented in InternalJavaScript.
// With microtask queue disabled, it should NOT show [native code].
// With microtask queue enabled, it should show [native code].
print(Promise.toString().includes("[native code]"));
// DISABLED-NEXT: false
// ENABLED-NEXT: true

print(Promise.prototype.then.toString().includes("[native code]"));
// DISABLED-NEXT: false
// ENABLED-NEXT: true

// TextEncoder is implemented as an extension.
// Extensions always show [native code] regardless of microtask queue setting.
print(TextEncoder.toString().includes("[native code]"));
// DISABLED-NEXT: true
// ENABLED-NEXT: true

print(TextEncoder.prototype.encode.toString().includes("[native code]"));
// DISABLED-NEXT: true
// ENABLED-NEXT: true

// Native builtins always show [native code] regardless of microtask queue.
print(Array.isArray.toString().includes("[native code]"));
// DISABLED-NEXT: true
// ENABLED-NEXT: true

print("Done");
// DISABLED-NEXT: Done
// ENABLED-NEXT: Done
