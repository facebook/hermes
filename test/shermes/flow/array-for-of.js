/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -O0 -exec %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s
// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s

print('for of');
// CHECK-LABEL: for of

function simple(x: number[]) {
  for (const i of x) {
    if (i === 0) break;
    if (i === 1) continue;
    print(i);
  }
}

simple([20, 1, 2, 3])
// CHECK-NEXT: 20
// CHECK-NEXT: 2
// CHECK-NEXT: 3

simple([10, 11, 0, 20])
// CHECK-NEXT: 10
// CHECK-NEXT: 11

function changeSize(x: number[]) {
  for (const i of x) {
    if (i === 0) x.push(100);
    print(i);
  }
}

changeSize([0]);
// CHECK-NEXT: 0
// CHECK-NEXT: 100
