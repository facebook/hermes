/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s
// RUN: %shermes -O0 -typed -exec %s | %FileCheck --match-full-lines %s

print('for-of destr');
// CHECK-LABEL: for-of destr

// The element type of a typed array flows into the loop variable, so the
// pattern is destructured by index instead of via the iterator protocol.

let tuples: [number, string][] = [[1, 'a'], [2, 'b']];
for (const [n, s] of tuples) {
  print(n + 1, s + '!');
}
// CHECK-NEXT: 2 a!
// CHECK-NEXT: 3 b!

let nested: [number, [string, boolean]][] = [[1, ['a', true]]];
for (const [n, [s, b]] of nested) {
  print(n, s, b);
}
// CHECK-NEXT: 1 a true

let arrays: number[][] = [[1, 2], [3, 4]];
for (const [a, b] of arrays) {
  print(a, b);
}
// CHECK-NEXT: 1 2
// CHECK-NEXT: 3 4

let objs: {p: number, q: string}[] = [{p: 1, q: 'x'}];
for (const {p, q} of objs) {
  print(p, q);
}
// CHECK-NEXT: 1 x

// A plain identifier gets the element type too.
let nums: number[] = [10, 20];
for (const n of nums) {
  print(n * 2);
}
// CHECK-NEXT: 20
// CHECK-NEXT: 40

// A pattern in the expression (non-declaration) position is checked leaf by
// leaf, not as a whole: destructuring assigns one target at a time, while
// comparing the element type against an aggregate of the targets would be
// invariant and reject this.
let u: any, v: any;
for ([u, v] of tuples) {
  print(u, v);
}
// CHECK-NEXT: 1 a
// CHECK-NEXT: 2 b

// Same for a pattern nested inside an object destructuring target.
let wrapped: {p: [number, string]}[] = [{p: [3, 'c']}];
for ({p: [u, v]} of wrapped) {
  print(u, v);
}
// CHECK-NEXT: 3 c

// An annotated loop variable is narrowed on store just like a member target,
// so an element outside its type throws instead of inhabiting it.
let maybes: (?number)[] = [1, null];
try {
  for (const m: number of maybes) {
    print('narrowed', m);
  }
} catch (e) {
  print('caught', e.message);
}
// CHECK-NEXT: narrowed 1
// CHECK-NEXT: caught Checked cast failed
