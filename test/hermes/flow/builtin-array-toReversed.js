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

/// Test Array.prototype.toReversed with typed arrays.

'use strict';

(function () {

const nums: number[] = [1, 2, 3, 4, 5];

// Basic toReversed.
var r: number[] = nums.toReversed();
print(r.length, r[0], r[1], r[2], r[3], r[4]);
// CHECK: 5 5 4 3 2 1

// Original unchanged.
print(nums[0], nums[4]);
// CHECK-NEXT: 1 5

// Single element.
const single: number[] = [42];
r = single.toReversed();
print(r.length, r[0]);
// CHECK-NEXT: 1 42

// Empty array.
const empty: number[] = [];
r = empty.toReversed();
print(r.length);
// CHECK-NEXT: 0

})();
