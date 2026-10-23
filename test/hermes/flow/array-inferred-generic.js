/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

// An inferred type argument that is an Array<T> is still treated as an array.

'use strict';

function apply<T, U>(x: T, f: (T) => U): U {
  return f(x);
}
function mapArr<T, U>(a: T[], f: (T) => U): U[] {
  const r: U[] = [];
  for (let i = 0; i < a.length; ++i) r.push(f(a[i]));
  return r;
}

const arr: number[] = [1, 2];
print(apply(arr, a => a[0] + a[1]));
// CHECK: 3

const nested: number[][] = [[3, 4]];
print(mapArr(nested, a => a[0] * a[1])[0]);
// CHECK-NEXT: 12

type NumArr = number[];
const na: NumArr = [5, 6];
print(na[1]);
// CHECK-NEXT: 6
