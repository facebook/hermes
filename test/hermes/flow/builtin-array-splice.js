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

/// Test Array.prototype.splice with typed arrays.

'use strict';

(function () {

// No args: no-op, returns [].
var a: number[] = [1, 2, 3, 4, 5];
var d: number[] = a.splice();
print(d.length);
// CHECK: 0
print(a.length, a[0], a[1], a[2], a[3], a[4]);
// CHECK-NEXT: 5 1 2 3 4 5

// Only start: removes everything from start.
a = [1, 2, 3, 4, 5];
d = a.splice(2);
print(d.length, d[0], d[1], d[2]);
// CHECK-NEXT: 3 3 4 5
print(a.length, a[0], a[1]);
// CHECK-NEXT: 2 1 2

// start + deleteCount, no items.
a = [1, 2, 3, 4, 5];
d = a.splice(1, 2);
print(d.length, d[0], d[1]);
// CHECK-NEXT: 2 2 3
print(a.length, a[0], a[1], a[2]);
// CHECK-NEXT: 3 1 4 5

// Replace same count (ic == dc).
a = [1, 2, 3, 4, 5];
d = a.splice(1, 2, 20, 30);
print(d.length, d[0], d[1]);
// CHECK-NEXT: 2 2 3
print(a.length, a[0], a[1], a[2], a[3], a[4]);
// CHECK-NEXT: 5 1 20 30 4 5

// Insert more than removed (ic > dc).
a = [1, 2, 3, 4, 5];
d = a.splice(2, 1, 100, 200, 300);
print(d.length, d[0]);
// CHECK-NEXT: 1 3
print(a.length, a[0], a[1], a[2], a[3], a[4], a[5], a[6]);
// CHECK-NEXT: 7 1 2 100 200 300 4 5

// Insert without removing (ic > 0, dc == 0).
a = [1, 2, 3, 4, 5];
d = a.splice(2, 0, 99);
print(d.length);
// CHECK-NEXT: 0
print(a.length, a[0], a[1], a[2], a[3], a[4], a[5]);
// CHECK-NEXT: 6 1 2 99 3 4 5

// Insert at end with no remove.
a = [1, 2, 3];
d = a.splice(3, 0, 4, 5);
print(d.length);
// CHECK-NEXT: 0
print(a.length, a[0], a[1], a[2], a[3], a[4]);
// CHECK-NEXT: 5 1 2 3 4 5

// Insert at start.
a = [3, 4];
d = a.splice(0, 0, 1, 2);
print(d.length);
// CHECK-NEXT: 0
print(a.length, a[0], a[1], a[2], a[3]);
// CHECK-NEXT: 4 1 2 3 4

// Remove from start.
a = [1, 2, 3, 4];
d = a.splice(0, 2);
print(d.length, d[0], d[1]);
// CHECK-NEXT: 2 1 2
print(a.length, a[0], a[1]);
// CHECK-NEXT: 2 3 4

// Negative start.
a = [1, 2, 3, 4, 5];
d = a.splice(-2, 1, 77);
print(d.length, d[0]);
// CHECK-NEXT: 1 4
print(a.length, a[0], a[1], a[2], a[3], a[4]);
// CHECK-NEXT: 5 1 2 3 77 5

// start > len clamps to len, with insert.
a = [1, 2, 3, 4, 5];
d = a.splice(100, 5, 9);
print(d.length);
// CHECK-NEXT: 0
print(a.length, a[0], a[1], a[2], a[3], a[4], a[5]);
// CHECK-NEXT: 6 1 2 3 4 5 9

// deleteCount > remaining clamps.
a = [1, 2, 3, 4, 5];
d = a.splice(3, 100);
print(d.length, d[0], d[1]);
// CHECK-NEXT: 2 4 5
print(a.length, a[0], a[1], a[2]);
// CHECK-NEXT: 3 1 2 3

// Negative deleteCount treated as 0.
a = [1, 2, 3];
d = a.splice(1, -1, 8);
print(d.length);
// CHECK-NEXT: 0
print(a.length, a[0], a[1], a[2], a[3]);
// CHECK-NEXT: 4 1 8 2 3

// Empty array, insert.
a = [];
d = a.splice(0, 0, 1, 2, 3);
print(d.length);
// CHECK-NEXT: 0
print(a.length, a[0], a[1], a[2]);
// CHECK-NEXT: 3 1 2 3

// Empty array, remove (no-op).
a = [];
d = a.splice(0, 5);
print(d.length);
// CHECK-NEXT: 0
print(a.length);
// CHECK-NEXT: 0

// Remove all then insert.
a = [1, 2, 3];
d = a.splice(0, 3, 9, 8, 7);
print(d.length, d[0], d[1], d[2]);
// CHECK-NEXT: 3 1 2 3
print(a.length, a[0], a[1], a[2]);
// CHECK-NEXT: 3 9 8 7

})();
