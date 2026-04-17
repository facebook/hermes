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

/// Test Array.prototype.find and findIndex with typed arrays.

'use strict';

(function () {

const nums: number[] = [1, 2, 3, 4, 5];

// find: first even.
print(nums.find((n: number): boolean => n % 2 === 0));
// CHECK: 2

// find: not found.
print(nums.find((n: number): boolean => n > 100));
// CHECK-NEXT: undefined

// findIndex: first even.
print(nums.findIndex((n: number): boolean => n % 2 === 0));
// CHECK-NEXT: 1

// findIndex: not found.
print(nums.findIndex((n: number): boolean => n > 100));
// CHECK-NEXT: -1

// Empty array.
const empty: number[] = [];
print(empty.find((n: number): boolean => true));
// CHECK-NEXT: undefined
print(empty.findIndex((n: number): boolean => true));
// CHECK-NEXT: -1

// find with index parameter.
print(nums.find((n: number, i: number): boolean => i === 3));
// CHECK-NEXT: 4

// find with thisArg.
const threshold: number[] = [3];
print(nums.find(function(this: number[], n: number): boolean {
  return n > this[0];
}, threshold));
// CHECK-NEXT: 4

// findIndex with thisArg.
print(nums.findIndex(function(this: number[], n: number): boolean {
  return n > this[0];
}, threshold));
// CHECK-NEXT: 3

})();
