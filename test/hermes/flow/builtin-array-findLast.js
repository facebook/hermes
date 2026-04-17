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

/// Test Array.prototype.findLast and findLastIndex with typed arrays.

'use strict';

(function () {

const nums: number[] = [1, 2, 3, 4, 5, 6];

// findLast: last even.
print(nums.findLast((n: number): boolean => n % 2 === 0));
// CHECK: 6

// findLast: not found.
print(nums.findLast((n: number): boolean => n > 100));
// CHECK-NEXT: undefined

// findLastIndex: last even.
print(nums.findLastIndex((n: number): boolean => n % 2 === 0));
// CHECK-NEXT: 5

// findLastIndex: not found.
print(nums.findLastIndex((n: number): boolean => n > 100));
// CHECK-NEXT: -1

// Empty array.
const empty: number[] = [];
print(empty.findLast((n: number): boolean => true));
// CHECK-NEXT: undefined
print(empty.findLastIndex((n: number): boolean => true));
// CHECK-NEXT: -1

// findLast with thisArg.
const threshold: number[] = [3];
print(nums.findLast(function(this: number[], n: number): boolean {
  return n < this[0];
}, threshold));
// CHECK-NEXT: 2

// findLastIndex with thisArg.
print(nums.findLastIndex(function(this: number[], n: number): boolean {
  return n < this[0];
}, threshold));
// CHECK-NEXT: 1

})();
