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

/// Test Array.prototype.filter with typed arrays.

'use strict';

(function () {

const nums: number[] = [1, 2, 3, 4, 5, 6];

// Filter even numbers.
const evens: number[] = nums.filter((n: number): boolean => n % 2 === 0);
print(evens.length, evens[0], evens[1], evens[2]);
// CHECK: 3 2 4 6

// Filter with index parameter.
const firstThree: number[] = nums.filter(
  (n: number, i: number): boolean => i < 3,
);
print(firstThree.length, firstThree[0], firstThree[1], firstThree[2]);
// CHECK-NEXT: 3 1 2 3

// Filter returns empty array when nothing matches.
const none: number[] = nums.filter((n: number): boolean => n > 100);
print(none.length);
// CHECK-NEXT: 0

})();
