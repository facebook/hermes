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

/// Test Array.prototype.includes with typed arrays.

'use strict';

(function () {

const nums: number[] = [1, 2, 3, 4, 5];

print(nums.includes(3));
// CHECK: true

print(nums.includes(6));
// CHECK-NEXT: false

// Empty array.
const empty: number[] = [];
print(empty.includes(1));
// CHECK-NEXT: false

// String array.
const strs: string[] = ['a', 'b', 'c'];
print(strs.includes('b'));
// CHECK-NEXT: true

print(strs.includes('d'));
// CHECK-NEXT: false

print([NaN].includes(NaN));
// CHECK-NEXT: true

// includes with fromIndex.
const fromNums: number[] = [1, 2, 3, 4, 5];
print(fromNums.includes(1, 1));
// CHECK-NEXT: false
print(fromNums.includes(3, 2));
// CHECK-NEXT: true
print(fromNums.includes(3, 3));
// CHECK-NEXT: false

// Negative fromIndex.
print(fromNums.includes(4, -2));
// CHECK-NEXT: true
print(fromNums.includes(1, -2));
// CHECK-NEXT: false

// Negative fromIndex that normalizes below 0 starts from 0.
print(fromNums.includes(1, -100));
// CHECK-NEXT: true

// fromIndex beyond length.
print(fromNums.includes(1, 10));
// CHECK-NEXT: false

// NaN with fromIndex.
var nans: number[] = [1, NaN, 3, NaN, 5];
print(nans.includes(NaN, 2));
// CHECK-NEXT: true
print(nans.includes(NaN, 4));
// CHECK-NEXT: false

})();
