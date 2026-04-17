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

/// Test Array.prototype.slice with typed arrays.

'use strict';

(function () {

const nums: number[] = [1, 2, 3, 4, 5];

// Basic slice.
var s: number[] = nums.slice(1, 4);
print(s.length, s[0], s[1], s[2]);
// CHECK: 3 2 3 4

// Slice from start.
s = nums.slice(0, 2);
print(s.length, s[0], s[1]);
// CHECK-NEXT: 2 1 2

// Slice entire array.
s = nums.slice(0, 5);
print(s.length);
// CHECK-NEXT: 5

// Negative start.
s = nums.slice(-2, 5);
print(s.length, s[0], s[1]);
// CHECK-NEXT: 2 4 5

// Negative end.
s = nums.slice(1, -1);
print(s.length, s[0], s[1], s[2]);
// CHECK-NEXT: 3 2 3 4

// Empty result when start >= end.
s = nums.slice(3, 2);
print(s.length);
// CHECK-NEXT: 0

// Empty array.
const empty: number[] = [];
s = empty.slice(0, 0);
print(s.length);
// CHECK-NEXT: 0

// No arguments copies entire array.
s = nums.slice();
print(s.length, s[0], s[1], s[2], s[3], s[4]);
// CHECK-NEXT: 5 1 2 3 4 5

// Only start argument.
s = nums.slice(2);
print(s.length, s[0], s[1], s[2]);
// CHECK-NEXT: 3 3 4 5

// Negative start, no end.
s = nums.slice(-2);
print(s.length, s[0], s[1]);
// CHECK-NEXT: 2 4 5

})();
