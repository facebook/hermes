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

/// Test Array.prototype.copyWithin with typed arrays.

'use strict';

(function () {

// Copy forward (no overlap).
var a: number[] = [1, 2, 3, 4, 5];
a.copyWithin(0, 3, 5);
print(a[0], a[1], a[2], a[3], a[4]);
// CHECK: 4 5 3 4 5

// Copy backward (overlap, target > start).
var b: number[] = [1, 2, 3, 4, 5];
b.copyWithin(2, 0, 3);
print(b[0], b[1], b[2], b[3], b[4]);
// CHECK-NEXT: 1 2 1 2 3

// Negative target.
var c: number[] = [1, 2, 3, 4, 5];
c.copyWithin(-2, 0, 2);
print(c[0], c[1], c[2], c[3], c[4]);
// CHECK-NEXT: 1 2 3 1 2

// Negative start and end.
var d: number[] = [1, 2, 3, 4, 5];
d.copyWithin(0, -3, -1);
print(d[0], d[1], d[2], d[3], d[4]);
// CHECK-NEXT: 3 4 3 4 5

// Count clamped to not exceed array length.
var e: number[] = [1, 2, 3];
e.copyWithin(1, 0, 3);
print(e[0], e[1], e[2]);
// CHECK-NEXT: 1 1 2

// Returns the array.
var f: number[] = [1, 2, 3];
var result: number[] = f.copyWithin(0, 1, 3);
print(result[0], result[1], result[2]);
// CHECK-NEXT: 2 3 3

// End parameter omitted defaults to length.
var g: number[] = [1, 2, 3, 4, 5];
g.copyWithin(0, 3);
print(g[0], g[1], g[2], g[3], g[4]);
// CHECK-NEXT: 4 5 3 4 5

// End omitted with negative start.
var h: number[] = [1, 2, 3, 4, 5];
h.copyWithin(0, -2);
print(h[0], h[1], h[2], h[3], h[4]);
// CHECK-NEXT: 4 5 3 4 5

})();
