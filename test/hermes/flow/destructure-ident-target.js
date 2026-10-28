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

// Destructuring and for-of stores into a typed variable narrow the value
// against the variable's declared type.

let n: number = 0;
let src: any = {p: 7};
let bad: any = {p: 'str'};
let arr: any[] = [1, 2, 3];

({p: n} = src);
print('any source', n);
// CHECK: any source 7

for (n of arr) {}
print('for-of', n);
// CHECK-NEXT: for-of 3

try {
  ({p: n} = bad);
  print('no throw');
} catch (e) {
  print('caught', e.message, n);
}
// CHECK-NEXT: caught Checked cast failed 3

// The undefined placeholder of an uninitialized let is not narrowed.
function f(): void {
  for (let i = 0; i < 2; ++i) {
    let x: number;
    x = i;
    print('let', x);
  }
}
f();
// CHECK-NEXT: let 0
// CHECK-NEXT: let 1
