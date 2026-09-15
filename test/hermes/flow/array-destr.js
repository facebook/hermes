/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s
// RUN: %shermes -O0 -typed -exec %s | %FileCheck --match-full-lines %s

/// Test runtime behavior of array destructuring against typed Array<T>
/// (FastArray) sources, including the ...rest pattern that allocates a fresh
/// FastArray and copies the trailing elements.

'use strict';

(function () {

print('array-destr');
// CHECK-LABEL: array-destr

var arr: Array<number> = [10, 20, 30, 40, 50];

// Destructuring without rest reads typed elements directly.
let [a, b]: Array<number> = arr;
print(a, b);
// CHECK-NEXT: 10 20

// Rest collects the remaining elements into a fresh Array<number>.
let [c, d, ...rest]: Array<number> = arr;
print(c, d);
// CHECK-NEXT: 10 20
print(rest.length);
// CHECK-NEXT: 3
print(rest[0], rest[1], rest[2]);
// CHECK-NEXT: 30 40 50

// Mutating the rest array does not modify the source.
rest.push(60);
print(rest.length, arr.length);
// CHECK-NEXT: 4 5

// Rest-only pattern copies the entire array.
let [...all]: Array<number> = arr;
print(all.length, all[0], all[4]);
// CHECK-NEXT: 5 10 50
all.push(99);
print(arr.length, all.length);
// CHECK-NEXT: 5 6

// Rest with an empty tail produces an empty Array<T>.
var two: Array<string> = ['x', 'y'];
let [p, q, ...tail]: Array<string> = two;
print(p, q, tail.length);
// CHECK-NEXT: x y 0

// Rest in a function parameter destructuring binds a fresh Array<T>.
function f([h, ...t]: Array<number>): number {
  return h + t.length;
}
print(f([1, 2, 3, 4]));
// CHECK-NEXT: 4
print(f([7]));
// CHECK-NEXT: 7

// T[] syntax works the same as Array<T>.
let nums: number[] = [100, 200, 300];
let [first, ...others]: number[] = nums;
print(first, others.length, others[0], others[1]);
// CHECK-NEXT: 100 2 200 300

// Assignment-expression destructuring against an Array<T> source. The
// pattern must include a rest binding to be typed against Array<T>;
// without a rest the LHS is treated as a tuple of its element types.
let x: number = 0, y: number = 0;
let restA: Array<number> = [];
[x, y, ...restA] = arr;
print(x, y, restA.length, restA[0]);
// CHECK-NEXT: 10 20 3 30

// Rest-only assignment yields a fresh Array<T> independent of the source.
[...restA] = arr;
restA.push(999);
print(restA.length, arr.length);
// CHECK-NEXT: 6 5

// Assignment from an Array<string>.
let p2: string = '', q2: string = '';
let tail2: Array<string> = [];
[p2, q2, ...tail2] = two;
print(p2, q2, tail2.length);
// CHECK-NEXT: x y 0

// Nested array destructuring at declaration.
let nested: Array<Array<number>> = [[1, 2], [3, 4], [5, 6]];
let [[na, nb], [nc, nd]]: Array<Array<number>> = nested;
print(na, nb, nc, nd);
// CHECK-NEXT: 1 2 3 4

// Nested array destructuring with rest binding.
let ya: number = 0, yb: number = 0;
let ytail: Array<Array<number>> = [];
[[ya, yb], ...ytail] = nested;
print(ya, yb, ytail.length, ytail[0][0], ytail[1][1]);
// CHECK-NEXT: 1 2 2 3 6

// Rest target itself a destructuring pattern.
let za: number = 0, zb: number = 0, zc: number = 0;
[za, ...[zb, zc]] = arr;
print(za, zb, zc);
// CHECK-NEXT: 10 20 30

// Destructuring more elements than the source has throws on the first
// out-of-bounds load, since the element type T disallows undefined.
var oob: Array<number> = [1];
try {
  let [oa, ob, oc]: Array<number> = oob;
  print('no throw', oa, ob, oc);
} catch (e) {
  print('threw');
}
// CHECK-NEXT: threw

// Sparse patterns: holes are skipped at both declaration and assignment
// sites, and a trailing rest still collects the remaining elements.
let [, sb, , ...stail]: Array<number> = arr;
print(sb, stail.length, stail[0], stail[1]);
// CHECK-NEXT: 20 2 40 50

let sx: number = 0;
let srest: Array<number> = [];
[, sx, , ...srest] = arr;
print(sx, srest.length, srest[0], srest[1]);
// CHECK-NEXT: 20 2 40 50

})();
