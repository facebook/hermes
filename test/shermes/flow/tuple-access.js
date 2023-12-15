/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s
// RUN: %shermes -O0 -typed -exec %s | %FileCheck --match-full-lines %s

let x: [number, bool] = [1, true]

print('tuple');
// CHECK-LABEL: tuple

let y: number = x[0]
print(y);
// CHECK-NEXT: 1
let z: bool = x[1]
print(z);
// CHECK-NEXT: true
x[0] = 2;
print(x[0]);
// CHECK-NEXT: 2
