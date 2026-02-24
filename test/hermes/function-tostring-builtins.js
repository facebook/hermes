/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

// Test that JS-implemented builtin functions return "[native code]" from toString().
// This is required by ECMAScript for all builtin functions.

print("Function toString builtins test");
// CHECK-LABEL: Function toString builtins test

// Promise is implemented in InternalJavaScript
print(Promise.toString());
// CHECK-NEXT: function Promise() { [native code] }

// Promise methods (note: names may be empty due to internal bytecode optimizations)
print(Promise.prototype.then.toString().includes("[native code]"));
// CHECK-NEXT: true

print(Promise.prototype.catch.toString().includes("[native code]"));
// CHECK-NEXT: true

print(Promise.prototype.finally.toString().includes("[native code]"));
// CHECK-NEXT: true

print(Promise.all.toString().includes("[native code]"));
// CHECK-NEXT: true

print(Promise.race.toString().includes("[native code]"));
// CHECK-NEXT: true

print(Promise.resolve.toString().includes("[native code]"));
// CHECK-NEXT: true

print(Promise.reject.toString().includes("[native code]"));
// CHECK-NEXT: true

// Native builtins should also show [native code]
print(Array.isArray.toString());
// CHECK-NEXT: function isArray() { [native code] }

print(Object.keys.toString());
// CHECK-NEXT: function keys() { [native code] }

print(Math.abs.toString());
// CHECK-NEXT: function abs() { [native code] }

// TextEncoder is implemented as an extension
print(TextEncoder.toString().includes("[native code]"));
// CHECK-NEXT: true

print(TextEncoder.prototype.encode.toString().includes("[native code]"));
// CHECK-NEXT: true

print(TextEncoder.prototype.encodeInto.toString().includes("[native code]"));
// CHECK-NEXT: true

print("Done");
// CHECK-NEXT: Done
