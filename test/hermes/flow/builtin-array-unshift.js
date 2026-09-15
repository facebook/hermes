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

/// Test Array.prototype.unshift with typed arrays.

'use strict';

(function () {

// No args: returns current length, array unchanged.
var a: number[] = [1, 2, 3];
print(a.unshift());
// CHECK: 3
print(a.length, a[0], a[1], a[2]);
// CHECK-NEXT: 3 1 2 3

// Single arg.
a = [2, 3];
print(a.unshift(1));
// CHECK-NEXT: 3
print(a.length, a[0], a[1], a[2]);
// CHECK-NEXT: 3 1 2 3

// Multiple args, preserve order.
a = [4, 5];
print(a.unshift(1, 2, 3));
// CHECK-NEXT: 5
print(a.length, a[0], a[1], a[2], a[3], a[4]);
// CHECK-NEXT: 5 1 2 3 4 5

// Empty array, multiple args.
a = [];
print(a.unshift(1, 2, 3));
// CHECK-NEXT: 3
print(a.length, a[0], a[1], a[2]);
// CHECK-NEXT: 3 1 2 3

// Empty array, no args.
a = [];
print(a.unshift());
// CHECK-NEXT: 0
print(a.length);
// CHECK-NEXT: 0

// More args than original length.
a = [10];
print(a.unshift(1, 2, 3, 4, 5));
// CHECK-NEXT: 6
print(a.length, a[0], a[1], a[2], a[3], a[4], a[5]);
// CHECK-NEXT: 6 1 2 3 4 5 10

// Single-element array, single arg.
a = [2];
print(a.unshift(1));
// CHECK-NEXT: 2
print(a.length, a[0], a[1]);
// CHECK-NEXT: 2 1 2

// String elements.
var s: string[] = ['c', 'd'];
print(s.unshift('a', 'b'));
// CHECK-NEXT: 4
print(s.length, s[0], s[1], s[2], s[3]);
// CHECK-NEXT: 4 a b c d

})();
