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

/// Test Array.prototype.indexOf and lastIndexOf with typed arrays.

'use strict';

(function () {

const nums: number[] = [1, 2, 3, 2, 1];

// Basic indexOf.
print(nums.indexOf(2));
// CHECK: 1

// indexOf not found.
print(nums.indexOf(9));
// CHECK-NEXT: -1

// Basic lastIndexOf.
print(nums.lastIndexOf(2));
// CHECK-NEXT: 3

// lastIndexOf not found.
print(nums.lastIndexOf(9));
// CHECK-NEXT: -1

// Empty array.
const empty: number[] = [];
print(empty.indexOf(1));
// CHECK-NEXT: -1
print(empty.lastIndexOf(1));
// CHECK-NEXT: -1

// String array.
const strs: string[] = ['a', 'b', 'c', 'b'];
print(strs.indexOf('b'));
// CHECK-NEXT: 1
print(strs.lastIndexOf('b'));
// CHECK-NEXT: 3

// indexOf with fromIndex.
print(nums.indexOf(2, 2));
// CHECK-NEXT: 3
print(nums.indexOf(2, -2));
// CHECK-NEXT: 3
print(nums.indexOf(2, -1));
// CHECK-NEXT: -1
// Negative fromIndex that normalizes below 0 starts from 0.
print(nums.indexOf(1, -100));
// CHECK-NEXT: 0

// lastIndexOf with fromIndex.
print(nums.lastIndexOf(2, 2));
// CHECK-NEXT: 1
print(nums.lastIndexOf(2, 0));
// CHECK-NEXT: -1
print(nums.lastIndexOf(2, -2));
// CHECK-NEXT: 3
print(nums.lastIndexOf(2, -4));
// CHECK-NEXT: 1
// fromIndex >= length clamps to length - 1.
print(nums.lastIndexOf(1, 100));
// CHECK-NEXT: 4

// NaN is not found by indexOf (uses strict equality, not SameValueZero).
var withNaN: number[] = [1, NaN, 3];
print(withNaN.indexOf(NaN));
// CHECK-NEXT: -1

// NaN is not found by lastIndexOf.
print(withNaN.lastIndexOf(NaN));
// CHECK-NEXT: -1

})();
