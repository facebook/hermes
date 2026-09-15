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

/// Test Array.prototype.reduce and reduceRight with typed arrays.

'use strict';

(function () {

const nums: number[] = [1, 2, 3, 4];

// reduce: sum with initial value 0.
print(nums.reduce<number>(
  (acc: number, n: number): number => acc + n, 0));
// CHECK: 10

// reduce: sum with non-zero initial value.
print(nums.reduce<number>(
  (acc: number, n: number): number => acc + n, 100));
// CHECK-NEXT: 110

// reduce: product.
print(nums.reduce<number>(
  (acc: number, n: number): number => acc * n, 1));
// CHECK-NEXT: 24

// reduce: string concatenation.
const strs: string[] = ['a', 'b', 'c'];
print(strs.reduce<string>(
  (acc: string, s: string): string => acc + s, ''));
// CHECK-NEXT: abc

// reduce: empty array with initial value.
const empty: number[] = [];
print(empty.reduce<number>(
  (acc: number, n: number): number => acc + n, 42));
// CHECK-NEXT: 42

// reduceRight: sum with initial value 0.
print(nums.reduceRight<number>(
  (acc: number, n: number): number => acc + n, 0));
// CHECK-NEXT: 10

// reduceRight: sum with non-zero initial value.
print(nums.reduceRight<number>(
  (acc: number, n: number): number => acc + n, 100));
// CHECK-NEXT: 110

// reduceRight: string concatenation (reversed).
print(strs.reduceRight<string>(
  (acc: string, s: string): string => acc + s, ''));
// CHECK-NEXT: cba

// reduceRight: empty array with initial value.
print(empty.reduceRight<number>(
  (acc: number, n: number): number => acc + n, 42));
// CHECK-NEXT: 42

// reduce with index parameter.
print(nums.reduce<number>(
  (acc: number, n: number, i: number): number => acc + i, 0));
// CHECK-NEXT: 6

// reduce: no initial value uses first element.
print(nums.reduce(
  (acc: number, n: number): number => acc + n));
// CHECK-NEXT: 10

// reduce: empty array with no initial value throws TypeError.
try {
  empty.reduce((acc: number, n: number): number => acc + n);
} catch (e) {
  print(e.constructor.name);
}
// CHECK-NEXT: TypeError

// reduceRight: no initial value uses last element.
print(nums.reduceRight(
  (acc: number, n: number): number => acc + n));
// CHECK-NEXT: 10

// reduceRight: empty array with no initial value throws TypeError.
try {
  empty.reduceRight((acc: number, n: number): number => acc + n);
} catch (e) {
  print(e.constructor.name);
}
// CHECK-NEXT: TypeError

// reduce: single-element array with no initial value returns sole element.
const single: number[] = [42];
print(single.reduce(
  (acc: number, n: number): number => acc + n));
// CHECK-NEXT: 42

// reduceRight: single-element array with no initial value returns sole element.
print(single.reduceRight(
  (acc: number, n: number): number => acc + n));
// CHECK-NEXT: 42

})();
