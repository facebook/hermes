/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

/// Test Array.prototype.shift with typed arrays.

'use strict';

(function () {

// Shift returns first element.
const nums: number[] = [10, 20, 30];
print(nums.shift());
// CHECK: 10
print(nums.length, nums[0], nums[1]);
// CHECK-NEXT: 2 20 30

// Shift again.
print(nums.shift());
// CHECK-NEXT: 20
print(nums.length, nums[0]);
// CHECK-NEXT: 1 30

// Shift last element.
print(nums.shift());
// CHECK-NEXT: 30
print(nums.length);
// CHECK-NEXT: 0

// Shift from empty array returns undefined.
print(nums.shift());
// CHECK-NEXT: undefined
print(nums.length);
// CHECK-NEXT: 0

// Shift from initially empty array.
const empty: number[] = [];
print(empty.shift());
// CHECK-NEXT: undefined
print(empty.length);
// CHECK-NEXT: 0

// Single-element array.
const one: number[] = [42];
print(one.shift());
// CHECK-NEXT: 42
print(one.length);
// CHECK-NEXT: 0

// String elements.
const s: string[] = ['a', 'b', 'c'];
print(s.shift());
// CHECK-NEXT: a
print(s.length, s[0], s[1]);
// CHECK-NEXT: 2 b c

})();
