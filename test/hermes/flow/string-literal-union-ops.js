/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

// A union of string literals supports string operators and members.

'use strict';

function f(k: 'a' | 'b'): void {
  const n: number = k.length;
  const s: string = k + 'x';
  const c: boolean = k < 'z';
  const u: string = k.toUpperCase();
  const i: string = k[0];
  print(n, s, c, u, i);
}
f('b');
// CHECK: 1 bx true B b

const arr = ['p', 'q', 'p'];
let first: string = arr[0];
first = 'r';
print(first, arr.length);
// CHECK-NEXT: r 3
