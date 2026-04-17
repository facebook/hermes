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

/// Test Array.prototype.flatMap with typed arrays.

'use strict';

(function () {

const nums: number[] = [1, 2, 3];

// Basic flatMap: duplicate each element.
const doubled: number[] = nums.flatMap(
  (n: number): number[] => [n, n],
);
print(doubled.length, doubled[0], doubled[1], doubled[2], doubled[3], doubled[4], doubled[5]);
// CHECK: 6 1 1 2 2 3 3

// flatMap on empty array.
const empty: number[] = [];
const result: number[] = empty.flatMap(
  (n: number): number[] => [n, n],
);
print(result.length);
// CHECK-NEXT: 0

// flatMap with thisArg.
const offset: number[] = [10];
const shifted: number[] = nums.flatMap(function(
  this: number[],
  n: number,
): number[] {
  return [n + this[0]];
}, offset);
print(shifted.length, shifted[0], shifted[1], shifted[2]);
// CHECK-NEXT: 3 11 12 13

})();
