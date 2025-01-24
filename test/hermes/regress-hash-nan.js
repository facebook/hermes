/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// Test that different NaN representations get the same hash.
globalThis.zero = 0;
var m = new Map([[NaN, 42]]);
var dynamicNaN = 0 / globalThis.zero;
print(m.get(dynamicNaN));
// CHECK: 42
print(m.has(dynamicNaN));
// CHECK: true
print(m.get(NaN));
// CHECK: 42
print(m.has(NaN));
// CHECK: true

m = new Map([[dynamicNaN, 24]]);
print(m.get(dynamicNaN));
// CHECK: 24
print(m.has(dynamicNaN));
// CHECK: true
print(m.get(NaN));
// CHECK: 24
print(m.has(NaN));
// CHECK: true
