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

/// Test Array.prototype.toSpliced with typed arrays.

'use strict';

(function () {

const orig: number[] = [1, 2, 3, 4, 5];

// No args: returns a copy.
var r: number[] = orig.toSpliced();
print(r.length, r[0], r[1], r[2], r[3], r[4]);
// CHECK: 5 1 2 3 4 5
print(orig.length, orig[0], orig[1], orig[2], orig[3], orig[4]);
// CHECK-NEXT: 5 1 2 3 4 5

// Only start: drops everything from start.
r = orig.toSpliced(2);
print(r.length, r[0], r[1]);
// CHECK-NEXT: 2 1 2

// start + skipCount, no items.
r = orig.toSpliced(1, 2);
print(r.length, r[0], r[1], r[2]);
// CHECK-NEXT: 3 1 4 5

// Replace same count.
r = orig.toSpliced(1, 2, 20, 30);
print(r.length, r[0], r[1], r[2], r[3], r[4]);
// CHECK-NEXT: 5 1 20 30 4 5

// Insert more than removed.
r = orig.toSpliced(2, 1, 100, 200, 300);
print(r.length, r[0], r[1], r[2], r[3], r[4], r[5], r[6]);
// CHECK-NEXT: 7 1 2 100 200 300 4 5

// Insert without removing.
r = orig.toSpliced(2, 0, 99);
print(r.length, r[0], r[1], r[2], r[3], r[4], r[5]);
// CHECK-NEXT: 6 1 2 99 3 4 5

// Negative start.
r = orig.toSpliced(-2, 1, 77);
print(r.length, r[0], r[1], r[2], r[3], r[4]);
// CHECK-NEXT: 5 1 2 3 77 5

// start > len clamps to len, with insert.
r = orig.toSpliced(100, 5, 9);
print(r.length, r[0], r[1], r[2], r[3], r[4], r[5]);
// CHECK-NEXT: 6 1 2 3 4 5 9

// skipCount > remaining clamps.
r = orig.toSpliced(3, 100);
print(r.length, r[0], r[1], r[2]);
// CHECK-NEXT: 3 1 2 3

// Negative skipCount treated as 0.
r = orig.toSpliced(2, -1, 8);
print(r.length, r[0], r[1], r[2], r[3], r[4], r[5]);
// CHECK-NEXT: 6 1 2 8 3 4 5

// Empty array.
const empty: number[] = [];
r = empty.toSpliced(0, 0, 1, 2, 3);
print(r.length, r[0], r[1], r[2]);
// CHECK-NEXT: 3 1 2 3
print(empty.length);
// CHECK-NEXT: 0

// Original still untouched after all calls.
print(orig.length, orig[0], orig[1], orig[2], orig[3], orig[4]);
// CHECK-NEXT: 5 1 2 3 4 5

})();
