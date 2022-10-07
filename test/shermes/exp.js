/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

'use strict'

print('exponentiation');
// CHECK-LABEL: exponentiation

print(2 ** 3);
// CHECK-NEXT: 8
print({valueOf: () => 2} ** 3);
// CHECK-NEXT: 8

print(2 ** 3 ** 2);
// CHECK-NEXT: 512
print(2**3**2);
// CHECK-NEXT: 512
print(2**(3**2));
// CHECK-NEXT: 512
print((2**3)**2);
// CHECK-NEXT: 64

print(1 + 2 ** 3 ** 2);
// CHECK-NEXT: 513
print(2 ** 3 ** 2 + 1);
// CHECK-NEXT: 513
print(Math.random() ** 0);
// CHECK-NEXT: 1

var x = 10;
x **= 2;
print(x);
// CHECK-NEXT: 100
