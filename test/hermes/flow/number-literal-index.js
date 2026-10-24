/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

'use strict';

// A union of number literals can index arrays and strings.
const arr: number[] = [10, 20, 30];
let i: 0 | 1 = 1;
print(arr[i]);
// CHECK: 20
let u: 1 | 2 = 2;
print('abc'[u]);
// CHECK-NEXT: c

// 0 and -0 are one literal type, so the default value doesn't depend on which
// specialization is created first.
class Box<T> {
  v: T;
  constructor() {}
}
const b = new Box<-0>();
const a = new Box<0>();
print(Object.is(a.v, 0), Object.is(b.v, 0));
// CHECK-NEXT: true true
