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

/// Test Array.prototype.join with typed arrays.

'use strict';

(function () {

const nums: number[] = [1, 2, 3];

// Join with comma.
print(nums.join(','));
// CHECK: 1,2,3

// Join with space.
print(nums.join(' '));
// CHECK-NEXT: 1 2 3

// Join with empty string.
print(nums.join(''));
// CHECK-NEXT: 123

// Join with multi-char separator.
print(nums.join(' - '));
// CHECK-NEXT: 1 - 2 - 3

// Single element.
const single: number[] = [42];
print(single.join(','));
// CHECK-NEXT: 42

// Empty array.
const empty: number[] = [];
print(empty.join(','));
// CHECK-EMPTY:

// String array.
const strs: string[] = ['a', 'b', 'c'];
print(strs.join('|'));
// CHECK-NEXT: a|b|c

// Default separator is comma.
print(nums.join());
// CHECK-NEXT: 1,2,3

// Default separator on empty array.
print(empty.join());
// CHECK-EMPTY:

// Undefined and null elements produce empty strings.
var mixed: (number | void | null)[] = [1, undefined, null, 3];
print(mixed.join(','));
// CHECK-NEXT: 1,,,3

print(mixed.join());
// CHECK-NEXT: 1,,,3

print(mixed.join('-'));
// CHECK-NEXT: 1---3

// All undefined/null.
var allEmpty: (void | null)[] = [undefined, null, undefined];
print(allEmpty.join(','));
// CHECK-NEXT: ,,

// Single undefined.
var singleUndef: (number | void)[] = [undefined];
print(singleUndef.join(','));
// CHECK-EMPTY:

// toString also handles undefined/null as empty strings.
print(mixed.toString());
// CHECK-NEXT: 1,,,3

})();
