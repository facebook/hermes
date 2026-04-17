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

/// Test Array.prototype.every and some with typed arrays.

'use strict';

(function () {

const nums: number[] = [2, 4, 6, 8];

// every: all even.
print(nums.every((n: number): boolean => n % 2 === 0));
// CHECK: true

// every: not all > 5.
print(nums.every((n: number): boolean => n > 5));
// CHECK-NEXT: false

// every on empty array is true.
const empty: number[] = [];
print(empty.every((n: number): boolean => n > 0));
// CHECK-NEXT: true

// some: has element > 5.
print(nums.some((n: number): boolean => n > 5));
// CHECK-NEXT: true

// some: no element > 100.
print(nums.some((n: number): boolean => n > 100));
// CHECK-NEXT: false

// some on empty array is false.
print(empty.some((n: number): boolean => n > 0));
// CHECK-NEXT: false

// every with index parameter.
const arr: number[] = [0, 1, 2, 3];
print(arr.every((n: number, i: number): boolean => n === i));
// CHECK-NEXT: true

// every with thisArg.
const threshold: number[] = [5];
print(nums.every(function(this: number[], n: number): boolean {
  return n <= this[0] * 2;
}, threshold));
// CHECK-NEXT: true

// some with thisArg.
print(nums.some(function(this: number[], n: number): boolean {
  return n > this[0];
}, threshold));
// CHECK-NEXT: true

})();
