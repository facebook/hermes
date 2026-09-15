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

/// Test Array.prototype.pop with typed arrays.

'use strict';

(function () {

const nums: number[] = [10, 20, 30];

// Pop returns last element.
print(nums.pop());
// CHECK: 30

// Pop again.
print(nums.pop());
// CHECK-NEXT: 20

// Pop last element.
print(nums.pop());
// CHECK-NEXT: 10

// Pop from empty array returns undefined.
print(nums.pop());
// CHECK-NEXT: undefined

// Verify length is 0 after popping all elements.
print(nums.length);
// CHECK-NEXT: 0

// Pop from an initially empty array.
const empty: number[] = [];
print(empty.pop());
// CHECK-NEXT: undefined

// Direct $SHBuiltin.fastArrayPop with N argument.
const bulk: number[] = [1, 2, 3, 4, 5];

// Popping 0 elements is a no-op and returns undefined.
print($SHBuiltin.fastArrayPop(bulk, 0));
// CHECK-NEXT: undefined
print(bulk.length);
// CHECK-NEXT: 5

// Popping N>1 returns the topmost popped element.
print($SHBuiltin.fastArrayPop(bulk, 2));
// CHECK-NEXT: 5
print(bulk.length);
// CHECK-NEXT: 3

// Popping more than length clamps and returns the topmost popped element.
print($SHBuiltin.fastArrayPop(bulk, 10));
// CHECK-NEXT: 3
print(bulk.length);
// CHECK-NEXT: 0

// Popping from an empty array still returns undefined.
print($SHBuiltin.fastArrayPop(bulk, 5));
// CHECK-NEXT: undefined

})();
