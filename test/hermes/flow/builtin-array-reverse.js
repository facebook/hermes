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

/// Test Array.prototype.reverse with typed arrays.

'use strict';

(function () {

// Reverse odd-length array.
var nums: number[] = [1, 2, 3, 4, 5];
nums.reverse();
print(nums[0], nums[1], nums[2], nums[3], nums[4]);
// CHECK: 5 4 3 2 1

// Reverse even-length array.
var even: number[] = [1, 2, 3, 4];
even.reverse();
print(even[0], even[1], even[2], even[3]);
// CHECK-NEXT: 4 3 2 1

// Reverse returns the array.
var arr: number[] = [10, 20];
var result: number[] = arr.reverse();
print(result[0], result[1]);
// CHECK-NEXT: 20 10

// Single element.
var single: number[] = [42];
single.reverse();
print(single[0]);
// CHECK-NEXT: 42

// Empty array.
var empty: number[] = [];
empty.reverse();
print(empty.length);
// CHECK-NEXT: 0

})();
