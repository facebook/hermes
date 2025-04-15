/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline %s | %FileCheck --match-full-lines %s
// RUN: %hermes -fno-inline -Xforce-jit -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function check(x) {
  return [x === 1.5, x !== 1.5];
}

print(check(1.5));
// CHECK: true,false
print(check(null));
// CHECK-NEXT: false,true
print(check(0));
// CHECK-NEXT: false,true
print(check(1));
// CHECK-NEXT: false,true
print(check('a'));
// CHECK-NEXT: false,true
print(check(1n));
// CHECK-NEXT: false,true
print(check({}));
// CHECK-NEXT: false,true
print(check(NaN));
// CHECK-NEXT: false,true
print(check(1.12984675));
// CHECK-NEXT: false,true
print(check(undefined));
// CHECK-NEXT: false,true
print(check(true));
// CHECK-NEXT: false,true
