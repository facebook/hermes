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

/// Test Array.prototype.concat with typed arrays.

'use strict';

(function () {

const a: number[] = [1, 2, 3];
const b: number[] = [4, 5, 6];

// Basic concat.
const c: number[] = a.concat(b);
print(c.length, c[0], c[1], c[2], c[3], c[4], c[5]);
// CHECK: 6 1 2 3 4 5 6

// Original arrays unchanged.
print(a.length, b.length);
// CHECK-NEXT: 3 3

// Concat with empty array.
const d: number[] = a.concat([]);
print(d.length, d[0], d[1], d[2]);
// CHECK-NEXT: 3 1 2 3

// Empty concat with array.
const empty: number[] = [];
const e: number[] = empty.concat(a);
print(e.length, e[0], e[1], e[2]);
// CHECK-NEXT: 3 1 2 3

// String arrays.
const s1: string[] = ['a', 'b'];
const s2: string[] = ['c', 'd'];
const s3: string[] = s1.concat(s2);
print(s3.length, s3[0], s3[1], s3[2], s3[3]);
// CHECK-NEXT: 4 a b c d

})();
