/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -typed -O %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -O0 -exec %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -O -exec %s | %FileCheck --match-full-lines %s

print('equality');
// CHECK-LABEL: equality

print(0 === (-0: number | null));
// CHECK-NEXT: true
print(NaN === (NaN: number | null));
// CHECK-NEXT: false
print(0n === (0n: bigint | null));
// CHECK-NEXT: true
print(('a' + 'bc': string) === ('abc': string | null));
// CHECK-NEXT: true
var obj = {};
print(obj === obj);
// CHECK-NEXT: true
print(obj === undefined, obj !== undefined);
// CHECK-NEXT: false true
var nul = (() => null)();
print(nul === null, nul !== null);
// CHECK-NEXT: true false
