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

/// Test Array.prototype.at with typed arrays.

'use strict';

(function () {

const nums: number[] = [10, 20, 30, 40, 50];

// Positive index.
print(nums.at(0));
// CHECK: 10
print(nums.at(2));
// CHECK-NEXT: 30

// Negative index.
print(nums.at(-1));
// CHECK-NEXT: 50
print(nums.at(-3));
// CHECK-NEXT: 30

// Out of bounds.
print(nums.at(5));
// CHECK-NEXT: undefined
print(nums.at(-6));
// CHECK-NEXT: undefined

// Empty array.
const empty: number[] = [];
print(empty.at(0));
// CHECK-NEXT: undefined

})();
