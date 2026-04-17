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

/// Test Array.prototype.fill with typed arrays.

'use strict';

(function () {

// Fill entire array.
var nums: number[] = [1, 2, 3, 4, 5];
nums.fill(0);
print(nums[0], nums[1], nums[2], nums[3], nums[4]);
// CHECK: 0 0 0 0 0

// Fill returns the array.
var arr: number[] = [1, 2, 3];
var result: number[] = arr.fill(7);
print(result[0], result[1], result[2]);
// CHECK-NEXT: 7 7 7

// Fill single element.
var single: number[] = [1];
single.fill(42);
print(single[0]);
// CHECK-NEXT: 42

// Fill empty array (no-op).
var empty: number[] = [];
empty.fill(99);
print(empty.length);
// CHECK-NEXT: 0

// Fill string array.
var strs: string[] = ['a', 'b', 'c'];
strs.fill('x');
print(strs[0], strs[1], strs[2]);
// CHECK-NEXT: x x x

// Fill with start only.
var a: number[] = [1, 2, 3, 4, 5];
a.fill(9, 2);
print(a[0], a[1], a[2], a[3], a[4]);
// CHECK-NEXT: 1 2 9 9 9

// Fill with start and end.
var b: number[] = [1, 2, 3, 4, 5];
b.fill(7, 1, 3);
print(b[0], b[1], b[2], b[3], b[4]);
// CHECK-NEXT: 1 7 7 4 5

// Fill with negative start and end.
var c: number[] = [1, 2, 3, 4, 5];
c.fill(8, -3, -1);
print(c[0], c[1], c[2], c[3], c[4]);
// CHECK-NEXT: 1 2 8 8 5

})();
